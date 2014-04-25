/*	$OpenBSD: src/usr.sbin/snmpd/control.c,v 1.24 2014/04/25 11:12:14 blambert Exp $	*/

/*
 * Copyright (c) 2010-2013 Reyk Floeter <reyk@openbsd.org>
 * Copyright (c) 2003, 2004 Henning Brauer <henning@openbsd.org>
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

#include <sys/queue.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/tree.h>

#include <net/if.h>

#include <errno.h>
#include <event.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "snmpd.h"

#define	CONTROL_BACKLOG	5

struct ctl_connlist ctl_conns;

static int agentx_sessionid = 1;

void	 control_accept(int, short, void *);
void	 control_close(struct ctl_conn *);
void	 control_dispatch_imsg(int, short, void *);
void	 control_dispatch_agentx(int, short, void *);
void	 control_imsg_forward(struct imsg *);
void	 control_event_add(struct ctl_conn *, int, int, struct timeval *);

int
control_init(struct privsep *ps, struct control_sock *cs)
{
	struct snmpd		*env = ps->ps_env;
	struct sockaddr_un	 sun;
	int			 fd;
	mode_t			 old_umask, mode;

	if (cs->cs_name == NULL)
		return (0);

	if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		log_warn("%s: socket", __func__);
		return (-1);
	}

	sun.sun_family = AF_UNIX;
	if (strlcpy(sun.sun_path, cs->cs_name,
	    sizeof(sun.sun_path)) >= sizeof(sun.sun_path)) {
		log_warn("%s: %s name too long", __func__, cs->cs_name);
		close(fd);
		return (-1);
	}

	if (unlink(cs->cs_name) == -1)
		if (errno != ENOENT) {
			log_warn("%s: unlink %s", __func__, cs->cs_name);
			close(fd);
			return (-1);
		}

	if (cs->cs_restricted || cs->cs_agentx) {
		old_umask = umask(S_IXUSR|S_IXGRP|S_IXOTH);
		mode = S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH;
	} else {
		old_umask = umask(S_IXUSR|S_IXGRP|S_IWOTH|S_IROTH|S_IXOTH);
		mode = S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP;
	}

	if (bind(fd, (struct sockaddr *)&sun, sizeof(sun)) == -1) {
		log_warn("%s: bind: %s", __func__, cs->cs_name);
		close(fd);
		(void)umask(old_umask);
		return (-1);
	}
	(void)umask(old_umask);

	if (chmod(cs->cs_name, mode) == -1) {
		log_warn("%s: chmod", __func__);
		close(fd);
		(void)unlink(cs->cs_name);
		return (-1);
	}

	socket_set_blockmode(fd, BM_NONBLOCK);
	cs->cs_fd = fd;
	cs->cs_env = env;

	return (0);
}

int
control_listen(struct control_sock *cs)
{
	if (cs->cs_name == NULL)
		return (0);

	if (listen(cs->cs_fd, CONTROL_BACKLOG) == -1) {
		log_warn("%s: listen", __func__);
		return (-1);
	}

	event_set(&cs->cs_ev, cs->cs_fd, EV_READ,
	    control_accept, cs);
	event_add(&cs->cs_ev, NULL);
	evtimer_set(&cs->cs_evt, control_accept, cs);

	return (0);
}

void
control_cleanup(struct control_sock *cs)
{
	if (cs->cs_name == NULL)
		return;
	event_del(&cs->cs_ev);
	event_del(&cs->cs_evt);
	(void)unlink(cs->cs_name);
}

/* ARGSUSED */
void
control_accept(int listenfd, short event, void *arg)
{
	struct control_sock	*cs = arg;
	int			 connfd;
	socklen_t		 len;
	struct sockaddr_un	 sun;
	struct ctl_conn		*c;

	event_add(&cs->cs_ev, NULL);
	if ((event & EV_TIMEOUT))
		return;

	len = sizeof(sun);
	if ((connfd = accept(listenfd,
	    (struct sockaddr *)&sun, &len)) == -1) {
		/*
		 * Pause accept if we are out of file descriptors, or
		 * libevent will haunt us here too.
		 */
		if (errno == ENFILE || errno == EMFILE) {
			struct timeval evtpause = { 1, 0 };

			event_del(&cs->cs_ev);
			evtimer_add(&cs->cs_evt, &evtpause);
		} else if (errno != EWOULDBLOCK && errno != EINTR &&
		    errno != ECONNABORTED)
			log_warn("%s: accept", __func__);
		return;
	}

	socket_set_blockmode(connfd, BM_NONBLOCK);

	if ((c = calloc(1, sizeof(struct ctl_conn))) == NULL) {
		log_warn("%s", __func__);
		close(connfd);
		return;
	}

	imsg_init(&c->iev.ibuf, connfd);
	if (cs->cs_agentx) {
		c->data = snmp_agentx_alloc(c->iev.ibuf.fd);
		if (c->data == NULL) {
			free(c);
			log_warn("%s: agentx", __func__);
			return;
		}
		c->flags |= CTL_CONN_LOCKED;
		c->iev.handler = control_dispatch_agentx;
	} else
		c->iev.handler = control_dispatch_imsg;
	c->iev.events = EV_READ;
	c->iev.data = c;
	c->cs = cs;
	event_set(&c->iev.ev, c->iev.ibuf.fd, c->iev.events,
	    c->iev.handler, c->iev.data);
	event_add(&c->iev.ev, NULL);

	TAILQ_INSERT_TAIL(&ctl_conns, c, entry);
}

void
control_close(struct ctl_conn *c)
{
	struct control_sock *cs = c->cs;

	msgbuf_clear(&c->iev.ibuf.w);
	TAILQ_REMOVE(&ctl_conns, c, entry);

	event_del(&c->iev.ev);
	close(c->iev.ibuf.fd);

	/* Some file descriptors are available again. */
	if (evtimer_pending(&cs->cs_evt, NULL)) {
		evtimer_del(&cs->cs_evt);
		event_add(&cs->cs_ev, NULL);
	}

	free(c);
}

/* ARGSUSED */
void
control_dispatch_imsg(int fd, short event, void *arg)
{
	struct ctl_conn		*c = arg;
	struct control_sock	*cs = c->cs;
	struct snmpd		*env = cs->cs_env;
	struct imsg		 imsg;
	int			 n, v, i;

	switch (event) {
	case EV_READ:
		if ((n = imsg_read(&c->iev.ibuf)) == -1 || n == 0) {
			control_close(c);
			return;
		}
		break;
	case EV_WRITE:
		if (msgbuf_write(&c->iev.ibuf.w) < 0 && errno != EAGAIN) {
			control_close(c);
			return;
		}
		break;
	default:
		fatalx("unknown event");
	}

	for (;;) {
		if ((n = imsg_get(&c->iev.ibuf, &imsg)) == -1) {
			control_close(c);
			return;
		}

		if (n == 0)
			break;

		if (cs->cs_restricted || (c->flags & CTL_CONN_LOCKED)) {
			switch (imsg.hdr.type) {
			case IMSG_SNMP_AGENTX:
			case IMSG_SNMP_ELEMENT:
			case IMSG_SNMP_END:
			case IMSG_SNMP_LOCK:
				break;
			default:
				log_debug("control_dispatch_imsg: "
				    "client requested restricted command");
				imsg_free(&imsg);
				control_close(c);
				return;
			}
		}

		control_imsg_forward(&imsg);

		switch (imsg.hdr.type) {
		case IMSG_CTL_NOTIFY:
			if (c->flags & CTL_CONN_NOTIFY) {
				log_debug("%s: "
				    "client requested notify more than once",
				    __func__);
				imsg_compose_event(&c->iev, IMSG_CTL_FAIL,
				    0, 0, -1, NULL, 0);
				break;
			}
			c->flags |= CTL_CONN_NOTIFY;
			break;

		case IMSG_SNMP_LOCK:
			/* enable restricted control mode */
			c->flags |= CTL_CONN_LOCKED;
			break;

		case IMSG_SNMP_AGENTX:

			/* rendezvous with the client */
			imsg_compose_event(&c->iev, IMSG_CTL_OK, 0, 0, -1, NULL, 0);
			if (imsg_flush(&c->iev.ibuf) == -1) {
				log_debug("control_dispatch_imsg: "
				    "could not rendezvous with client");
				imsg_free(&imsg);
				control_close(c);
				return;
			}

			/* enable AgentX socket */
			c->data = snmp_agentx_alloc(c->iev.ibuf.fd);
			if (c->data == NULL) {
				log_debug("control_dispatch_imsg: "
				    "could not allocate restricted socket");
				imsg_free(&imsg);
				control_close(c);
				return;
			}
			/* disable IMSG notifications */
			c->flags &= ~CTL_CONN_NOTIFY;
			c->flags |= CTL_CONN_LOCKED;
			c->iev.handler = control_dispatch_agentx;
			break;

		case IMSG_CTL_VERBOSE:
			IMSG_SIZE_CHECK(&imsg, &v);

			memcpy(&v, imsg.data, sizeof(v));
			log_verbose(v);

			for (i = 0; i < PROC_MAX; i++) {
				if (privsep_process == PROC_CONTROL)
					continue;
				proc_forward_imsg(&env->sc_ps, &imsg, i, -1);
			}
			break;
		case IMSG_CTL_RELOAD:
			proc_forward_imsg(&env->sc_ps, &imsg, PROC_PARENT, -1);
			break;
		default:
			log_debug("%s: error handling imsg %d",
			    __func__, imsg.hdr.type);
			break;
		}
		imsg_free(&imsg);
	}

	imsg_event_add(&c->iev);
}

/* ARGSUSED */
void
control_dispatch_agentx(int fd, short event, void *arg)
{
	struct ctl_conn			*c = arg;
	struct agentx_handle		*h = c->data;
	struct agentx_pdu		*pdu;
	struct timeval			 tv;
	struct agentx_open_timeout	 to;
	struct ber_oid			 oid;
	struct agentx_close_request_data clhdr;
	int				 closing = 0;
	int				 evflags = 0;
	int				 timer = 0;
	int				 error = AGENTX_ERR_NONE;
	int				 idx = 0, vcpylen, dlen, uptime;
	char				*descr, *varcpy;

	varcpy = descr = NULL;
	if (h->timeout != 0)
		tv.tv_sec = h->timeout;
	else
		tv.tv_sec = AGENTX_DEFAULT_TIMEOUT;
	tv.tv_usec = 0;

	if (event & EV_TIMEOUT) {
		log_info("subagent session '%i' timed out after %i seconds",
		    h->sessionid, h->timeout);
		goto teardown;
	}

	if (event & EV_WRITE) {
		if (snmp_agentx_send(h, NULL) == -1) {
			if (errno != EAGAIN)
				goto teardown;

			/* short write */
			evflags |= EV_WRITE;
			timer = 1;
		}
	}

	if (event & EV_READ) {
		if ((pdu = snmp_agentx_recv(h)) == NULL) {
			if (h->error) {
				error = h->error;
				goto respond;
			}
			if (errno != EAGAIN)
				goto teardown;

			/* short read */
			timer = 1;
			goto done;
		}

		switch (pdu->hdr->type) {
		case AGENTX_OPEN:
			if (snmp_agentx_read_raw(pdu, &to, sizeof(to)) == -1 ||
			    snmp_agentx_read_oid(pdu,
			    (struct snmp_oid *)&oid) == -1 ||
			    (descr =
			    snmp_agentx_read_octetstr(pdu, &dlen)) == NULL) {
				error = AGENTX_ERR_PARSE_ERROR;
				break;
			}

			log_info("opening AgentX socket for '%.*s'",
			    dlen, descr);

			h->sessionid = pdu->hdr->sessionid =
			    agentx_sessionid++;
			if (to.timeout != 0)
				h->timeout = to.timeout;
			else
				h->timeout = AGENTX_DEFAULT_TIMEOUT;
			break;

		case AGENTX_CLOSE:
			if (snmp_agentx_read_raw(pdu,
			    &clhdr, sizeof(clhdr)) == -1) {
				error = AGENTX_ERR_PARSE_ERROR;
				break;
			}
			closing = 1;
			break;

		case AGENTX_NOTIFY:
			error = trap_agentx(h, pdu, &idx, &varcpy, &vcpylen);
			break;

		case AGENTX_PING:
			/* no processing, just an empty response */
			break;

		/* unimplemented */
		case AGENTX_ADD_AGENT_CAPS:
		case AGENTX_REMOVE_AGENT_CAPS:
		case AGENTX_RESPONSE:
		case AGENTX_REGISTER:
		case AGENTX_UNREGISTER:
		case AGENTX_GET:
		case AGENTX_GET_NEXT:
		case AGENTX_GET_BULK:
		case AGENTX_TEST_SET:
		case AGENTX_COMMIT_SET:
		case AGENTX_UNDO_SET:
		case AGENTX_CLEANUP_SET:
		case AGENTX_INDEX_ALLOCATE:
		case AGENTX_INDEX_DEALLOCATE:
			error = AGENTX_ERR_REQUEST_DENIED;
			break;

		/* NB: by RFC, this should precede all other checks. */
		default:
			log_info("unknown AgentX type '%i'", pdu->hdr->type);
			error = AGENTX_ERR_PARSE_ERROR;
			break;
		}
 respond:
		if (pdu)
			snmp_agentx_pdu_free(pdu);

		uptime = smi_getticks();
		if ((pdu = snmp_agentx_response_pdu(uptime, error, idx)) == NULL) {
			log_debug("failed to generate response");
			if (varcpy)
				free(varcpy);
			control_event_add(c, fd, EV_WRITE, NULL);	/* XXX -- EV_WRITE? */
			return;
		}

		if (varcpy) {
			snmp_agentx_raw(pdu, varcpy, vcpylen); /* XXX */
			free(varcpy);
		}
		snmp_agentx_send(h, pdu);

		/* Request processed, now write out response */
		evflags |= EV_WRITE;
	}

	if (closing)
		goto teardown;
 done:
	control_event_add(c, fd, evflags, timer ? &tv : NULL);
	return;

 teardown:
	log_debug("subagent session '%i' destroyed", h->sessionid);
	snmp_agentx_free(h);
	if (varcpy)
		free(varcpy);
	control_close(c);
}

void
control_imsg_forward(struct imsg *imsg)
{
	struct ctl_conn *c;

	TAILQ_FOREACH(c, &ctl_conns, entry)
		if (c->flags & CTL_CONN_NOTIFY)
			imsg_compose(&c->iev.ibuf, imsg->hdr.type,
			    0, imsg->hdr.pid, -1, imsg->data,
			    imsg->hdr.len - IMSG_HEADER_SIZE);
}

void
control_event_add(struct ctl_conn *c, int fd, int wflag, struct timeval *tv)
{
	event_del(&c->iev.ev);
	event_set(&c->iev.ev, fd, EV_READ|wflag, control_dispatch_agentx, c);
	event_add(&c->iev.ev, tv);
}
