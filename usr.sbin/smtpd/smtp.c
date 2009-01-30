/*	$OpenBSD: src/usr.sbin/smtpd/smtp.c,v 1.20 2009/01/30 17:34:58 gilles Exp $	*/

/*
 * Copyright (c) 2008 Gilles Chehade <gilles@openbsd.org>
 * Copyright (c) 2008 Pierre-Yves Ritschard <pyr@openbsd.org>
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

#include <ctype.h>
#include <event.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "smtpd.h"

__dead void	smtp_shutdown(void);
void		smtp_sig_handler(int, short, void *);
void		smtp_dispatch_parent(int, short, void *);
void		smtp_dispatch_mfa(int, short, void *);
void		smtp_dispatch_lka(int, short, void *);
void		smtp_dispatch_queue(int, short, void *);
void		smtp_dispatch_control(int, short, void *);
void		smtp_setup_events(struct smtpd *);
void		smtp_disable_events(struct smtpd *);
void		smtp_pause(struct smtpd *);
void		smtp_resume(struct smtpd *);
void		smtp_accept(int, short, void *);
void		session_timeout(int, short, void *);
void		session_auth_pickup(struct session *, char *, size_t);

struct s_smtp	s_smtp;

void
smtp_sig_handler(int sig, short event, void *p)
{
	switch (sig) {
	case SIGINT:
	case SIGTERM:
		smtp_shutdown();
		break;
	default:
		fatalx("smtp_sig_handler: unexpected signal");
	}
}

void
smtp_dispatch_parent(int sig, short event, void *p)
{
	struct smtpd		*env = p;
	struct imsgbuf		*ibuf;
	struct imsg		 imsg;
	ssize_t			 n;

	ibuf = env->sc_ibufs[PROC_PARENT];
	switch (event) {
	case EV_READ:
		if ((n = imsg_read(ibuf)) == -1)
			fatal("imsg_read_error");
		if (n == 0) {
			/* this pipe is dead, so remove the event handler */
			event_del(&ibuf->ev);
			event_loopexit(NULL);
			return;
		}
		break;
	case EV_WRITE:
		if (msgbuf_write(&ibuf->w) == -1)
			fatal("msgbuf_write");
		imsg_event_add(ibuf);
		return;
	default:
		fatalx("unknown event");
	}

	for (;;) {
		if ((n = imsg_get(ibuf, &imsg)) == -1)
			fatal("parent_dispatch_smtp: imsg_read error");
		if (n == 0)
			break;

		switch (imsg.hdr.type) {
		case IMSG_CONF_START:
			if (env->sc_flags & SMTPD_CONFIGURING)
				break;
			env->sc_flags |= SMTPD_CONFIGURING;
			smtp_disable_events(env);
			break;
		case IMSG_CONF_SSL: {
			struct ssl	*s;
			struct ssl	*x_ssl;

			if (!(env->sc_flags & SMTPD_CONFIGURING))
				break;

			if ((s = calloc(1, sizeof(*s))) == NULL)
				fatal(NULL);
			x_ssl = imsg.data;
			(void)strlcpy(s->ssl_name, x_ssl->ssl_name,
			    sizeof(s->ssl_name));
			s->ssl_cert_len = x_ssl->ssl_cert_len;
			if ((s->ssl_cert =
			    strdup((char *)imsg.data + sizeof(*s))) == NULL)
				fatal(NULL);
			s->ssl_key_len = x_ssl->ssl_key_len;
			if ((s->ssl_key = strdup((char *)imsg.data +
			    (sizeof(*s) + s->ssl_cert_len))) == NULL)
				fatal(NULL);

			SPLAY_INSERT(ssltree, &env->sc_ssl, s);
			break;
		}
		case IMSG_CONF_LISTENER: {
			struct listener	*l;
			struct ssl	 key;

			if (!(env->sc_flags & SMTPD_CONFIGURING))
				break;

			if ((l = calloc(1, sizeof(*l))) == NULL)
				fatal(NULL);
			memcpy(l, imsg.data, sizeof(*l));
			if ((l->fd = imsg_get_fd(ibuf, &imsg)) == -1)
				fatal("cannot get fd");

			log_debug("smtp_dispatch_parent: "
			    "got fd %d for listener: %p", l->fd, l);

			(void)strlcpy(key.ssl_name, l->ssl_cert_name,
			    sizeof(key.ssl_name));

			if (l->flags & F_SSL)
				if ((l->ssl = SPLAY_FIND(ssltree,
				    &env->sc_ssl, &key)) == NULL)
					fatal("parent and smtp desynchronized");

			TAILQ_INSERT_TAIL(&env->sc_listeners, l, entry);
			break;
		}
		case IMSG_CONF_END:
			if (!(env->sc_flags & SMTPD_CONFIGURING))
				break;
			smtp_setup_events(env);
			env->sc_flags &= ~SMTPD_CONFIGURING;
			break;
		case IMSG_PARENT_AUTHENTICATE: {
			struct session		*s;
			struct session		 key;
			struct session_auth_reply *reply;

			log_debug("smtp_dispatch_parent: parent handled authentication");
			reply = imsg.data;
			key.s_id = reply->session_id;
			key.s_msg.id = reply->session_id;

			s = SPLAY_FIND(sessiontree, &env->sc_sessions, &key);
			if (s == NULL)
				fatal("smtp_dispatch_parent: session is gone");

			if (s->s_flags & F_QUIT) {
				session_destroy(s);
				break;
			}

			if (reply->value)
				s->s_flags |= F_AUTHENTICATED;

			session_auth_pickup(s, NULL, 0);

			break;
		}
		default:
			log_debug("parent_dispatch_smtp: unexpected imsg %d",
			    imsg.hdr.type);
			break;
		}
		imsg_free(&imsg);
	}
	imsg_event_add(ibuf);
}

void
smtp_dispatch_mfa(int sig, short event, void *p)
{
	struct smtpd		*env = p;
	struct imsgbuf		*ibuf;
	struct imsg		 imsg;
	ssize_t			 n;

	ibuf = env->sc_ibufs[PROC_MFA];
	switch (event) {
	case EV_READ:
		if ((n = imsg_read(ibuf)) == -1)
			fatal("imsg_read_error");
		if (n == 0) {
			/* this pipe is dead, so remove the event handler */
			event_del(&ibuf->ev);
			event_loopexit(NULL);
			return;
		}
		break;
	case EV_WRITE:
		if (msgbuf_write(&ibuf->w) == -1)
			fatal("msgbuf_write");
		imsg_event_add(ibuf);
		return;
	default:
		fatalx("unknown event");
	}

	for (;;) {
		if ((n = imsg_get(ibuf, &imsg)) == -1)
			fatal("smtp_dispatch_mfa: imsg_read error");
		if (n == 0)
			break;

		switch (imsg.hdr.type) {
		case IMSG_MFA_MAIL:
		case IMSG_MFA_RCPT: {
			struct submit_status	*ss;
			struct session		*s;
			struct session		 key;

			log_debug("smtp_dispatch_mfa: mfa handled return path");
			ss = imsg.data;
			key.s_id = ss->id;
			key.s_msg.id = ss->id;

			s = SPLAY_FIND(sessiontree, &env->sc_sessions, &key);
			if (s == NULL)
				fatal("smtp_dispatch_mfa: session is gone");

			if (s->s_flags & F_QUIT) {
				session_destroy(s);
				break;
			}

			session_pickup(s, ss);
			break;
		}
		default:
			log_debug("smtp_dispatch_mfa: unexpected imsg %d",
			    imsg.hdr.type);
			break;
		}
		imsg_free(&imsg);
	}
	imsg_event_add(ibuf);
}

void
smtp_dispatch_lka(int sig, short event, void *p)
{
	struct smtpd		*env = p;
	struct imsgbuf		*ibuf;
	struct imsg		 imsg;
	ssize_t			 n;

	ibuf = env->sc_ibufs[PROC_LKA];
	switch (event) {
	case EV_READ:
		if ((n = imsg_read(ibuf)) == -1)
			fatal("imsg_read_error");
		if (n == 0) {
			/* this pipe is dead, so remove the event handler */
			event_del(&ibuf->ev);
			event_loopexit(NULL);
			return;
		}
		break;
	case EV_WRITE:
		if (msgbuf_write(&ibuf->w) == -1)
			fatal("msgbuf_write");
		imsg_event_add(ibuf);
		return;
	default:
		fatalx("unknown event");
	}

	for (;;) {
		if ((n = imsg_get(ibuf, &imsg)) == -1)
			fatal("smtp_dispatch_lka: imsg_read error");
		if (n == 0)
			break;

		switch (imsg.hdr.type) {
		case IMSG_LKA_HOST: {
			struct session		 key;
			struct session		*s;
			struct session		*ss;

			s = imsg.data;
			key.s_id = s->s_id;

			ss = SPLAY_FIND(sessiontree, &env->sc_sessions, &key);
			if (ss == NULL)
				fatal("smtp_dispatch_lka: session is gone");

			if (ss->s_flags & F_QUIT) {
				session_destroy(s);
				break;
			}

			strlcpy(ss->s_hostname, s->s_hostname, MAXHOSTNAMELEN);
			strlcpy(ss->s_msg.session_hostname, s->s_hostname,
			    MAXHOSTNAMELEN);

			session_pickup(s, NULL);

			break;
		}
		default:
			log_debug("smtp_dispatch_lka: unexpected imsg %d",
			    imsg.hdr.type);
			break;
		}
		imsg_free(&imsg);
	}
	imsg_event_add(ibuf);
}

void
smtp_dispatch_queue(int sig, short event, void *p)
{
	struct smtpd		*env = p;
	struct imsgbuf		*ibuf;
	struct imsg		 imsg;
	ssize_t			 n;

	ibuf = env->sc_ibufs[PROC_QUEUE];
	switch (event) {
	case EV_READ:
		if ((n = imsg_read(ibuf)) == -1)
			fatal("imsg_read_error");
		if (n == 0) {
			/* this pipe is dead, so remove the event handler */
			event_del(&ibuf->ev);
			event_loopexit(NULL);
			return;
		}
		break;
	case EV_WRITE:
		if (msgbuf_write(&ibuf->w) == -1)
			fatal("msgbuf_write");
		imsg_event_add(ibuf);
		return;
	default:
		fatalx("unknown event");
	}

	for (;;) {
		if ((n = imsg_get(ibuf, &imsg)) == -1)
			fatal("smtp_dispatch_queue: imsg_read error");
		if (n == 0)
			break;

		switch (imsg.hdr.type) {
		case IMSG_QUEUE_CREATE_MESSAGE: {
			struct submit_status	*ss;
			struct session		*s;
			struct session		 key;

			log_debug("smtp_dispatch_queue: queue handled message creation");
			ss = imsg.data;

			key.s_id = ss->id;

			s = SPLAY_FIND(sessiontree, &env->sc_sessions, &key);
			if (s == NULL)
				fatal("smtp_dispatch_queue: session is gone");

			if (s->s_flags & F_QUIT) {
				session_destroy(s);
				break;
			}

			(void)strlcpy(s->s_msg.message_id, ss->u.msgid,
			    sizeof(s->s_msg.message_id));
			session_pickup(s, ss);
			break;
		}
		case IMSG_QUEUE_MESSAGE_FILE: {
			struct submit_status	*ss;
			struct session		*s;
			struct session		 key;
			int			 fd;

			log_debug("smtp_dispatch_queue: queue handled message creation");
			ss = imsg.data;

			key.s_id = ss->id;

			s = SPLAY_FIND(sessiontree, &env->sc_sessions, &key);
			if (s == NULL)
				fatal("smtp_dispatch_queue: session is gone");

			fd = imsg_get_fd(ibuf, &imsg);

			if (s->s_flags & F_QUIT) {
				session_destroy(s);
				break;
			}

			if (fd != -1) {
				s->s_msg.datafp = fdopen(fd, "w");
				if (s->s_msg.datafp == NULL) {
					/* no need to handle error, it will be
					 * caught in session_pickup()
					 */
					close(fd);
				}
			}
			session_pickup(s, ss);

			break;
		}
		case IMSG_QUEUE_TEMPFAIL: {
			struct submit_status	*ss;
			struct session		*s;
			struct session		 key;

			log_debug("smtp_dispatch_queue: queue acknownedged a temporary failure");
			ss = imsg.data;
			key.s_id = ss->id;
			key.s_msg.id = ss->id;

			s = SPLAY_FIND(sessiontree, &env->sc_sessions, &key);
			if (s == NULL)
				fatal("smtp_dispatch_queue: session is gone");

			if (s->s_flags & F_QUIT) {
				session_destroy(s);
				break;
			}

			s->s_msg.status |= S_MESSAGE_TEMPFAILURE;
			break;
		}

		case IMSG_QUEUE_COMMIT_ENVELOPES:
		case IMSG_QUEUE_COMMIT_MESSAGE: {
			struct submit_status	*ss;
			struct session		*s;
			struct session		 key;

			log_debug("smtp_dispatch_queue: queue acknowledged message submission");
			ss = imsg.data;
			key.s_id = ss->id;
			key.s_msg.id = ss->id;

			s = SPLAY_FIND(sessiontree, &env->sc_sessions, &key);
			if (s == NULL)
				fatal("smtp_dispatch_queue: session is gone");

			if (imsg.hdr.type == IMSG_QUEUE_COMMIT_MESSAGE) {
				s->s_msg.message_id[0] = '\0';
				s->s_msg.message_uid[0] = '\0';
			}

			if (s->s_flags & F_QUIT) {
				session_destroy(s);
				break;
			}

			session_pickup(s, ss);
			break;
		}
		default:
			log_debug("smtp_dispatch_queue: unexpected imsg %d",
			    imsg.hdr.type);
			break;
		}
		imsg_free(&imsg);
	}
	imsg_event_add(ibuf);
}

void
smtp_dispatch_control(int sig, short event, void *p)
{
	struct smtpd		*env = p;
	struct imsgbuf		*ibuf;
	struct imsg		 imsg;
	ssize_t			 n;

	ibuf = env->sc_ibufs[PROC_CONTROL];
	switch (event) {
	case EV_READ:
		if ((n = imsg_read(ibuf)) == -1)
			fatal("imsg_read_error");
		if (n == 0) {
			/* this pipe is dead, so remove the event handler */
			event_del(&ibuf->ev);
			event_loopexit(NULL);
			return;
		}
		break;
	case EV_WRITE:
		if (msgbuf_write(&ibuf->w) == -1)
			fatal("msgbuf_write");
		imsg_event_add(ibuf);
		return;
	default:
		fatalx("unknown event");
	}

	for (;;) {
		if ((n = imsg_get(ibuf, &imsg)) == -1)
			fatal("smtp_dispatch_control: imsg_read error");
		if (n == 0)
			break;

		switch (imsg.hdr.type) {
		case IMSG_SMTP_PAUSE:
			smtp_pause(env);
			break;
		case IMSG_SMTP_RESUME:
			smtp_resume(env);
			break;
		case IMSG_STATS: {
			struct stats *s;

			s = imsg.data;
			s->u.smtp = s_smtp;
			imsg_compose(ibuf, IMSG_STATS, 0, 0, -1, s, sizeof(*s));
			break;
		}
		default:
			log_debug("smtp_dispatch_control: unexpected imsg %d",
			    imsg.hdr.type);
			break;
		}
		imsg_free(&imsg);
	}
	imsg_event_add(ibuf);
}

void
smtp_shutdown(void)
{
	log_info("smtp server exiting");
	_exit(0);
}

pid_t
smtp(struct smtpd *env)
{
	pid_t		 pid;
	struct passwd	*pw;

	struct event	 ev_sigint;
	struct event	 ev_sigterm;

	struct peer peers[] = {
		{ PROC_PARENT,	smtp_dispatch_parent },
		{ PROC_MFA,	smtp_dispatch_mfa },
		{ PROC_QUEUE,	smtp_dispatch_queue },
		{ PROC_LKA,	smtp_dispatch_lka },
		{ PROC_CONTROL,	smtp_dispatch_control }
	};

	switch (pid = fork()) {
	case -1:
		fatal("smtp: cannot fork");
	case 0:
		break;
	default:
		return (pid);
	}

	ssl_init();
	purge_config(env, PURGE_EVERYTHING);

	pw = env->sc_pw;

#ifndef DEBUG
	if (chroot(pw->pw_dir) == -1)
		fatal("smtp: chroot");
	if (chdir("/") == -1)
		fatal("smtp: chdir(\"/\")");
#else
#warning disabling privilege revocation and chroot in DEBUG MODE
#endif

	setproctitle("smtp server");
	smtpd_process = PROC_SMTP;

#ifndef DEBUG
	if (setgroups(1, &pw->pw_gid) ||
	    setresgid(pw->pw_gid, pw->pw_gid, pw->pw_gid) ||
	    setresuid(pw->pw_uid, pw->pw_uid, pw->pw_uid))
		fatal("smtp: cannot drop privileges");
#endif

	event_init();

	signal_set(&ev_sigint, SIGINT, smtp_sig_handler, env);
	signal_set(&ev_sigterm, SIGTERM, smtp_sig_handler, env);
	signal_add(&ev_sigint, NULL);
	signal_add(&ev_sigterm, NULL);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGHUP, SIG_IGN);

	config_peers(env, peers, 5);

	smtp_setup_events(env);
	event_dispatch();
	smtp_shutdown();

	return (0);
}

void
smtp_setup_events(struct smtpd *env)
{
	struct listener *l;
	struct timeval	 tv;

	TAILQ_FOREACH(l, &env->sc_listeners, entry) {
		log_debug("smtp_setup_events: configuring listener: %p%s.",
		    l, (l->flags & F_SSL)?" (with ssl)":"");

		session_socket_blockmode(l->fd, BM_NONBLOCK);
		if (listen(l->fd, SMTPD_BACKLOG) == -1)
			fatal("listen");
		l->env = env;
		event_set(&l->ev, l->fd, EV_READ, smtp_accept, l);
		event_add(&l->ev, NULL);
		ssl_setup(env, l);
	}

	evtimer_set(&env->sc_ev, session_timeout, env);
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	evtimer_add(&env->sc_ev, &tv);
}

void
smtp_disable_events(struct smtpd *env)
{
	struct listener	*l;

	log_debug("smtp_disable_events: closing listening sockets");
	while ((l = TAILQ_FIRST(&env->sc_listeners)) != NULL) {
		TAILQ_REMOVE(&env->sc_listeners, l, entry);
		event_del(&l->ev);
		close(l->fd);
		free(l);
	}
	TAILQ_INIT(&env->sc_listeners);
}

void
smtp_pause(struct smtpd *env)
{
	log_debug("smtp_pause_listeners: pausing listening sockets");
	smtp_disable_events(env);
	env->sc_opts |= SMTPD_SMTP_PAUSED;
}

void
smtp_resume(struct smtpd *env)
{
	log_debug("smtp_pause_listeners: resuming listening sockets");
	imsg_compose(env->sc_ibufs[PROC_PARENT], IMSG_PARENT_SEND_CONFIG,
	    0, 0, -1, NULL, 0);
	env->sc_opts &= ~SMTPD_SMTP_PAUSED;
}

void
smtp_accept(int fd, short event, void *p)
{
	int			 s_fd;
	struct sockaddr_storage	 ss;
	struct listener		*l = p;
	struct session		*s;
	socklen_t		 len;

	log_debug("smtp_accept: incoming client on listener: %p", l);
	len = sizeof(struct sockaddr_storage);
	if ((s_fd = accept(l->fd, (struct sockaddr *)&ss, &len)) == -1) {
		event_del(&l->ev);
		return;
	}

	log_debug("smtp_accept: accepted client on listener: %p", l);
	if ((s = calloc(1, sizeof(*s))) == NULL)
		fatal(NULL);
	len = sizeof(s->s_ss);

	s->s_fd = s_fd;
	s->s_tm = time(NULL);
	s->s_env = l->env;
	s->s_l = l;

	(void)memcpy(&s->s_ss, &ss, sizeof(s->s_ss));

	session_init(l, s);
	event_add(&l->ev, NULL);

	s_smtp.clients++;

	if (s_smtp.clients == s->s_env->sc_maxconn)
		event_del(&l->ev);
}

void
smtp_listener_setup(struct smtpd *env, struct listener *l)
{
	int opt;

	if ((l->fd = socket(l->ss.ss_family, SOCK_STREAM, 0)) == -1)
		fatal("socket");

	opt = 1;
	setsockopt(l->fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	if (bind(l->fd, (struct sockaddr *)&l->ss, l->ss.ss_len) == -1)
		fatal("bind");
}
