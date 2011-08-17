/*	$OpenBSD: src/usr.sbin/smtpd/Attic/ramqueue.c,v 1.14 2011/08/17 20:35:11 gilles Exp $	*/

/*
 * Copyright (c) 2011 Gilles Chehade <gilles@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/types.h>
#include <sys/queue.h>
#include <sys/tree.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <dirent.h>
#include <err.h>
#include <errno.h>
#include <event.h>
#include <fcntl.h>
#include <imsg.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "smtpd.h"
#include "log.h"


void ramqueue_insert(struct ramqueue *, struct envelope *, time_t);
int ramqueue_host_cmp(struct ramqueue_host *, struct ramqueue_host *);
void ramqueue_put_host(struct ramqueue *, struct ramqueue_host *);
void ramqueue_put_batch(struct ramqueue *, struct ramqueue_batch *);
int ramqueue_load_offline(struct ramqueue *);

static int ramqueue_expire(struct envelope *, time_t);
static time_t ramqueue_next_schedule(struct envelope *, time_t);
static struct ramqueue_host *ramqueue_get_host(struct ramqueue *, char *);
static struct ramqueue_batch *ramqueue_get_batch(struct ramqueue *,
    struct ramqueue_host *, struct envelope *);


void
ramqueue_init(struct ramqueue *rqueue)
{
	bzero(rqueue, sizeof (*rqueue));
	TAILQ_INIT(&rqueue->queue);
	RB_INIT(&rqueue->hosttree);
	RB_INIT(&rqueue->msgtree);
	rqueue->current_evp = NULL;
}

int
ramqueue_is_empty(struct ramqueue *rqueue)
{
	return TAILQ_FIRST(&rqueue->queue) == NULL;
}

int
ramqueue_message_is_empty(struct ramqueue_message *rq_message)
{
	return RB_EMPTY(&rq_message->evptree);
}

int
ramqueue_batch_is_empty(struct ramqueue_batch *rq_batch)
{
	return TAILQ_FIRST(&rq_batch->envelope_queue) == NULL;
}

int
ramqueue_host_is_empty(struct ramqueue_host *rq_host)
{
	return TAILQ_FIRST(&rq_host->batch_queue) == NULL;
}

struct ramqueue_envelope *
ramqueue_first_envelope(struct ramqueue *rqueue)
{
	return TAILQ_FIRST(&rqueue->queue);
}

struct ramqueue_envelope *
ramqueue_next_envelope(struct ramqueue *rqueue)
{
	if (rqueue->current_evp == NULL)
		rqueue->current_evp = TAILQ_FIRST(&rqueue->queue);
	else
		rqueue->current_evp = TAILQ_NEXT(rqueue->current_evp, queue_entry);
	return rqueue->current_evp;
}

struct ramqueue_envelope *
ramqueue_batch_first_envelope(struct ramqueue_batch *rq_batch)
{
	return TAILQ_FIRST(&rq_batch->envelope_queue);
}

int
ramqueue_load_offline(struct ramqueue *rqueue)
{
	char		 path[MAXPATHLEN];
	static struct qwalk    *q = NULL;

	log_debug("ramqueue: offline queue loading in progress");
	if (q == NULL)
		q = qwalk_new(PATH_OFFLINE);
	while (qwalk(q, path)) {
		imsg_compose_event(env->sc_ievs[PROC_QUEUE],
		    IMSG_PARENT_ENQUEUE_OFFLINE, PROC_PARENT, 0, -1, path,
		    strlen(path) + 1);
		log_debug("ramqueue: offline queue loading interrupted");
		return 0;
	}
	qwalk_close(q);
	q = NULL;
	log_debug("ramqueue: offline queue loading over");
	return 1;
}

int
ramqueue_load(struct ramqueue *rqueue, time_t *nsched)
{
	char			path[MAXPATHLEN];
	time_t			curtm;
	struct envelope		envelope;
	static struct qwalk    *q = NULL;
	struct ramqueue_envelope *rq_evp;



	log_debug("ramqueue: queue loading in progress");

	if (q == NULL)
		q = qwalk_new(PATH_QUEUE);
	while (qwalk(q, path)) {
		u_int64_t evpid;

		curtm = time(NULL);

		if ((evpid = filename_to_evpid(basename(path))) == 0)
			continue;

		if (! queue_envelope_load(Q_QUEUE, evpid, &envelope))
			continue;
		if (ramqueue_expire(&envelope, curtm))
			continue;
		ramqueue_insert(rqueue, &envelope, curtm);

		rq_evp = TAILQ_FIRST(&rqueue->queue);
		*nsched = rq_evp->sched;

		if (rq_evp->sched <= *nsched) {
			log_debug("ramqueue: loading interrupted");
			return (0);
		}
	}
	qwalk_close(q);
	q = NULL;
	log_debug("ramqueue: loading over");
	return (1);
}

void
ramqueue_insert(struct ramqueue *rqueue, struct envelope *envelope, time_t curtm)
{
	struct ramqueue_envelope *rq_evp;
	struct ramqueue_envelope *evp;
	struct ramqueue_message  *rq_msg, msgkey;

	msgkey.msgid = evpid_to_msgid(envelope->delivery.id);
	rq_msg = RB_FIND(msgtree, &rqueue->msgtree, &msgkey);
	if (rq_msg == NULL) {
		log_debug("inserting new message node in msgtree: %d", msgkey.msgid);
		rq_msg = calloc(1, sizeof (*rq_msg));
		if (rq_msg == NULL)
			fatal("calloc");
		rq_msg->msgid = msgkey.msgid;
		RB_INSERT(msgtree, &rqueue->msgtree, rq_msg);
		RB_INIT(&rq_msg->evptree);
		env->stats->ramqueue.messages++;
		SET_IF_GREATER(env->stats->ramqueue.messages,
		    env->stats->ramqueue.messages_max);
	}
	rq_msg->rq_host = ramqueue_get_host(rqueue, envelope->delivery.rcpt.domain);

	rq_evp = calloc(1, sizeof (*rq_evp));
	if (rq_evp == NULL)
		fatal("calloc");
	rq_evp->evpid = envelope->delivery.id;
	rq_evp->sched = ramqueue_next_schedule(envelope, curtm);
	rq_evp->rq_batch = ramqueue_get_batch(rqueue, rq_msg->rq_host, envelope);
	RB_INSERT(evptree, &rq_msg->evptree, rq_evp);
	rq_evp->rq_msg = rq_msg;

	TAILQ_INSERT_TAIL(&rq_evp->rq_batch->envelope_queue, rq_evp,
	    batchqueue_entry);

	/* sorted insert */
	TAILQ_FOREACH(evp, &rqueue->queue, queue_entry) {
		if (evp->sched >= rq_evp->sched) {
			TAILQ_INSERT_BEFORE(evp, rq_evp, queue_entry);
			break;
		}
	}
	if (evp == NULL)
		TAILQ_INSERT_TAIL(&rqueue->queue, rq_evp, queue_entry);

 	env->stats->ramqueue.envelopes++;
	SET_IF_GREATER(env->stats->ramqueue.envelopes,
	    env->stats->ramqueue.envelopes_max);
}

static int
ramqueue_expire(struct envelope *envelope, time_t curtm)
{
	struct envelope bounce;

	if (curtm - envelope->delivery.creation >= envelope->delivery.expire) {
		envelope_set_errormsg(envelope,
		    "message expired after sitting in queue for %d days",
		    envelope->delivery.expire / 60 / 60 / 24);
		bounce_record_message(envelope, &bounce);
		ramqueue_insert(&env->sc_rqueue, &bounce, time(NULL));
		queue_envelope_delete(Q_QUEUE, envelope);
		return 1;
	}
	return 0;
}

static time_t
ramqueue_next_schedule(struct envelope *envelope, time_t curtm)
{
	time_t delay;

	if (envelope->delivery.lasttry == 0)
		return curtm;

	delay = SMTPD_QUEUE_MAXINTERVAL;
	
	if (envelope->delivery.type == D_MDA ||
	    envelope->delivery.type == D_BOUNCE) {
		if (envelope->delivery.retry < 5)
			return curtm;
			
		if (envelope->delivery.retry < 15)
			delay = (envelope->delivery.retry * 60) + arc4random_uniform(60);
	}

	if (envelope->delivery.type == D_MTA) {
		if (envelope->delivery.retry < 3)
			delay = SMTPD_QUEUE_INTERVAL;
		else if (envelope->delivery.retry <= 7) {
			delay = SMTPD_QUEUE_INTERVAL * (1 << (envelope->delivery.retry - 3));
			if (delay > SMTPD_QUEUE_MAXINTERVAL)
				delay = SMTPD_QUEUE_MAXINTERVAL;
		}
	}

	if (curtm >= envelope->delivery.lasttry + delay)
		return curtm;

	return curtm + delay;
}

static struct ramqueue_host *
ramqueue_get_host(struct ramqueue *rqueue, char *hostname)
{
	struct ramqueue_host *rq_host, key;

	strlcpy(key.hostname, hostname, sizeof(key.hostname));
	rq_host = RB_FIND(hosttree, &rqueue->hosttree, &key);
	if (rq_host == NULL) {
		rq_host = calloc(1, sizeof (*rq_host));
		if (rq_host == NULL)
			fatal("calloc");
		rq_host->h_id = generate_uid();
		strlcpy(rq_host->hostname, hostname, sizeof(rq_host->hostname));
		TAILQ_INIT(&rq_host->batch_queue);
		RB_INSERT(hosttree, &rqueue->hosttree, rq_host);
		env->stats->ramqueue.hosts++;
		SET_IF_GREATER(env->stats->ramqueue.hosts,
		    env->stats->ramqueue.hosts_max);
	}

	return rq_host;
}

void
ramqueue_put_host(struct ramqueue *rqueue, struct ramqueue_host *host)
{
	TAILQ_INIT(&host->batch_queue);
	RB_INSERT(hosttree, &rqueue->hosttree, host);
}

static struct ramqueue_batch *
ramqueue_get_batch(struct ramqueue *rqueue, struct ramqueue_host *host,
    struct envelope *envelope)
{
	struct ramqueue_batch *rq_batch;

	TAILQ_FOREACH(rq_batch, &host->batch_queue, batch_entry) {
		if (rq_batch->msgid == (u_int32_t)(envelope->delivery.id >> 32))
			return rq_batch;
	}

	rq_batch = calloc(1, sizeof (*rq_batch));
	if (rq_batch == NULL)
		fatal("calloc");
	rq_batch->b_id = generate_uid();
	rq_batch->rule = envelope->rule;
	rq_batch->type = envelope->delivery.type;
	rq_batch->msgid = envelope->delivery.id >> 32;

	TAILQ_INIT(&rq_batch->envelope_queue);
	TAILQ_INSERT_TAIL(&host->batch_queue, rq_batch, batch_entry);

	env->stats->ramqueue.batches++;
	SET_IF_GREATER(env->stats->ramqueue.batches,
	    env->stats->ramqueue.batches_max);
	return rq_batch;
}

void
ramqueue_put_batch(struct ramqueue *rqueue, struct ramqueue_batch *rq_batch)
{
	struct ramqueue_host *rq_host;

	TAILQ_INIT(&rq_batch->envelope_queue);
	RB_FOREACH(rq_host, hosttree, &rqueue->hosttree) {
		if (rq_host->h_id == rq_batch->h_id) {
			TAILQ_INSERT_TAIL(&rq_host->batch_queue, rq_batch,
			    batch_entry);
			return;
		}
	}
}

void
ramqueue_remove_batch(struct ramqueue_host *rq_host, struct ramqueue_batch *rq_batch)
{
	TAILQ_REMOVE(&rq_host->batch_queue, rq_batch, batch_entry);
}

void
ramqueue_remove_host(struct ramqueue *rqueue, struct ramqueue_host *rq_host)
{
	RB_REMOVE(hosttree, &rqueue->hosttree, rq_host);
}

void
ramqueue_remove_message(struct ramqueue *rqueue, struct ramqueue_message *rq_msg)
{
	RB_REMOVE(msgtree, &rqueue->msgtree, rq_msg);
}

void
ramqueue_schedule(struct ramqueue *rq, u_int64_t id)
{
	struct ramqueue_message *rq_msg;
	struct ramqueue_envelope *rq_evp;

	/* scheduling by evpid */
	if (id > 0xffffffffL) {
		rq_evp = ramqueue_lookup_envelope(rq, id);
		if (rq_evp == NULL)
			return;
		ramqueue_schedule_envelope(rq, rq_evp);
		return;
	}

	rq_msg = ramqueue_lookup_message(rq, id);
	if (rq_msg == NULL)
		return;

	/* scheduling by msgid */
	RB_FOREACH(rq_evp, evptree, &rq_msg->evptree) {
		ramqueue_schedule_envelope(rq, rq_evp);
	}
}

void
ramqueue_schedule_envelope(struct ramqueue *rq, struct ramqueue_envelope *rq_evp)
{
	rq_evp->sched = 0;
	TAILQ_REMOVE(&rq->queue, rq_evp, queue_entry);
	TAILQ_INSERT_HEAD(&rq->queue, rq_evp, queue_entry);
}

struct ramqueue_envelope *
ramqueue_envelope_by_id(struct ramqueue *rqueue, u_int64_t id)
{
	struct ramqueue_envelope *rq_evp;

	TAILQ_FOREACH(rq_evp, &rqueue->queue, queue_entry) {
		if (rq_evp->evpid == id)
			return rq_evp;
	}

	return NULL;
}

int
ramqueue_host_cmp(struct ramqueue_host *h1, struct ramqueue_host *h2)
{
	return strcmp(h1->hostname, h2->hostname);
}


int
ramqueue_msg_cmp(struct ramqueue_message *m1, struct ramqueue_message *m2)
{
	return (m1->msgid < m2->msgid ? -1 : m1->msgid > m2->msgid);
}

int
ramqueue_evp_cmp(struct ramqueue_envelope *e1, struct ramqueue_envelope *e2)
{
	return (e1->evpid < e2->evpid ? -1 : e1->evpid > e2->evpid);
}

struct ramqueue_host *
ramqueue_lookup_host(struct ramqueue *rq, char *hostname)
{
	struct ramqueue_host hostkey;

	if (strlcpy(hostkey.hostname, hostname, sizeof(hostkey.hostname))
	    >= sizeof(hostkey.hostname))
		fatalx("ramqueue_lookup_host: hostname truncated");

	return RB_FIND(hosttree, &rq->hosttree, &hostkey);
}

struct ramqueue_message *
ramqueue_lookup_message(struct ramqueue *rq, u_int32_t msgid)
{
	struct ramqueue_message  msgkey;

	msgkey.msgid = msgid;
	return RB_FIND(msgtree, &rq->msgtree, &msgkey);
}

struct ramqueue_envelope *
ramqueue_lookup_envelope(struct ramqueue *rq, u_int64_t evpid)
{
	struct ramqueue_envelope  evpkey;
	struct ramqueue_message *rq_msg;

	rq_msg = ramqueue_lookup_message(rq, evpid_to_msgid(evpid));
	if (rq_msg == NULL)
		return NULL;

	evpkey.evpid = evpid;
	return RB_FIND(evptree, &rq_msg->evptree, &evpkey);
}

void
ramqueue_remove_envelope(struct ramqueue *rq, struct ramqueue_envelope *rq_evp)
{
	struct ramqueue_batch *rq_batch;
	struct ramqueue_message *rq_msg;

	if (rq_evp == rq->current_evp)
		rq->current_evp = TAILQ_NEXT(rq->current_evp, queue_entry);

	rq_msg = rq_evp->rq_msg;
	rq_batch = rq_evp->rq_batch;

	RB_REMOVE(evptree, &rq_msg->evptree, rq_evp);
	TAILQ_REMOVE(&rq_batch->envelope_queue, rq_evp, batchqueue_entry);
	TAILQ_REMOVE(&rq->queue, rq_evp, queue_entry);

	env->stats->ramqueue.envelopes--;
}


RB_GENERATE(hosttree, ramqueue_host, hosttree_entry, ramqueue_host_cmp);
RB_GENERATE(msgtree, ramqueue_message, msgtree_entry, ramqueue_msg_cmp);
RB_GENERATE(evptree, ramqueue_envelope, evptree_entry, ramqueue_evp_cmp);

