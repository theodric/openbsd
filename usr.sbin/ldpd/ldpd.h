/*	$OpenBSD: src/usr.sbin/ldpd/ldpd.h,v 1.7 2009/09/28 09:48:46 michele Exp $ */

/*
 * Copyright (c) 2009 Michele Marchetto <michele@openbsd.org>
 * Copyright (c) 2004 Esben Norby <norby@openbsd.org>
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

#ifndef _LDPD_H_
#define _LDPD_H_

#include <sys/queue.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/tree.h>
#include <md5.h>
#include <net/if.h>
#include <netinet/in.h>
#include <event.h>

#include <imsg.h>
#include "ldp.h"

#define CONF_FILE		"/etc/ldpd.conf"
#define	LDPD_SOCKET		"/var/run/ldpd.sock"
#define LDPD_USER		"_ldpd"

#define NBR_HASHSIZE		128

#define NBR_IDSELF		1
#define NBR_CNTSTART		(NBR_IDSELF + 1)

#define	RT_BUF_SIZE		16384
#define	MAX_RTSOCK_BUF		128 * 1024
#define	LDP_BACKLOG		128

#define	LDPD_FLAG_NO_LFIB_UPDATE	0x0001

#define	F_LDPD_INSERTED		0x0001
#define	F_KERNEL		0x0002
#define	F_BGPD_INSERTED		0x0004
#define	F_CONNECTED		0x0008
#define	F_DOWN			0x0010
#define	F_STATIC		0x0020
#define	F_DYNAMIC		0x0040
#define	F_REDISTRIBUTED		0x0100

struct imsgev {
	struct imsgbuf		 ibuf;
	void			(*handler)(int, short, void *);
	struct event		 ev;
	void			*data;
	short			 events;
};

enum imsg_type {
	IMSG_NONE,
	IMSG_CTL_RELOAD,
	IMSG_CTL_SHOW_INTERFACE,
	IMSG_CTL_SHOW_NBR,
	IMSG_CTL_SHOW_LIB,
	IMSG_CTL_LFIB_COUPLE,
	IMSG_CTL_LFIB_DECOUPLE,
	IMSG_CTL_KROUTE,
	IMSG_CTL_KROUTE_ADDR,
	IMSG_CTL_IFINFO,
	IMSG_CTL_END,
	IMSG_KLABEL_INSERT,
	IMSG_KLABEL_CHANGE,
	IMSG_KLABEL_DELETE,
	IMSG_IFINFO,
	IMSG_LABEL_MAPPING,
	IMSG_LABEL_MAPPING_FULL,
	IMSG_LABEL_REQUEST,
	IMSG_REQUEST_ADD,
	IMSG_REQUEST_ADD_END,
	IMSG_MAPPING_ADD,
	IMSG_MAPPING_ADD_END,
	IMSG_RELEASE_ADD,
	IMSG_RELEASE_ADD_END,
	IMSG_ADDRESS_ADD,
	IMSG_ADDRESS_DEL,
	IMSG_NOTIFICATION_SEND,
	IMSG_NEIGHBOR_UP,
	IMSG_NEIGHBOR_DOWN,
	IMSG_NEIGHBOR_CHANGE,
	IMSG_NETWORK_ADD,
	IMSG_NETWORK_DEL,
	IMSG_RECONF_CONF,
	IMSG_RECONF_AREA,
	IMSG_RECONF_IFACE,
	IMSG_RECONF_END
};

/* interface states */
#define	IF_STA_NEW		0x00	/* dummy state for reload */
#define	IF_STA_DOWN		0x01
#define	IF_STA_LOOPBACK		0x02
#define	IF_STA_POINTTOPOINT	0x04
#define	IF_STA_DROTHER		0x08
#define	IF_STA_MULTI		(IF_STA_DROTHER | IF_STA_BACKUP | IF_STA_DR)
#define	IF_STA_ANY		0x7f
#define	IF_STA_ACTIVE		(~IF_STA_DOWN)

/* interface events */
enum iface_event {
	IF_EVT_NOTHING,
	IF_EVT_UP,
	IF_EVT_DOWN
};

/* interface actions */
enum iface_action {
	IF_ACT_NOTHING,
	IF_ACT_STRT,
	IF_ACT_RST
};

/* interface types */
enum iface_type {
	IF_TYPE_POINTOPOINT,
	IF_TYPE_BROADCAST,
	IF_TYPE_NBMA,
	IF_TYPE_POINTOMULTIPOINT,
	IF_TYPE_VIRTUALLINK
};

/* neighbor states */
#define	NBR_STA_DOWN		0x0001
#define	NBR_STA_PRESENT		0x0002
#define	NBR_STA_INITIAL		0x0004
#define	NBR_STA_OPENREC		0x0008
#define	NBR_STA_OPENSENT	0x0010
#define	NBR_STA_OPER		0x0020
#define	NBR_STA_ACTIVE		(~NBR_STA_DOWN)
#define	NBR_STA_SESSION		(NBR_STA_PRESENT | NBR_STA_PRESENT | \
				NBR_STA_INITIAL | NBR_STA_OPENREC | \
				NBR_STA_OPER | NBR_STA_OPENSENT | \
				NBR_STA_ACTIVE)
#define	NBR_STA_UP		(NBR_STA_PRESENT | NBR_STA_SESSION)

/* neighbor events */
enum nbr_event {
	NBR_EVT_NOTHING,
	NBR_EVT_HELLO_RCVD,
	NBR_EVT_SESSION_UP,
	NBR_EVT_CLOSE_SESSION,
	NBR_EVT_INIT_RCVD,
	NBR_EVT_KEEPALIVE_RCVD,
	NBR_EVT_PDU_RCVD,
	NBR_EVT_INIT_SENT,
	NBR_EVT_DOWN
};

/* neighbor actions */
enum nbr_action {
	NBR_ACT_NOTHING,
	NBR_ACT_STRT_ITIMER,
	NBR_ACT_RST_ITIMER,
	NBR_ACT_RST_KTIMEOUT,
	NBR_ACT_STRT_KTIMER,
	NBR_ACT_RST_KTIMER,
	NBR_ACT_SESSION_EST,
	NBR_ACT_INIT_SEND,
	NBR_ACT_KEEPALIVE_SEND,
	NBR_ACT_CLOSE_SESSION
};

TAILQ_HEAD(mapping_head, mapping_entry);

struct map {
	u_int32_t	label;
	u_int32_t	prefix;
	u_int8_t	prefixlen;
	u_int32_t	messageid;
};

struct notify_msg {
	u_int32_t	messageid;
	u_int32_t	status;
	u_int32_t	type;
};

struct iface {
	LIST_ENTRY(iface)	 entry;
	struct event		 hello_timer;

	LIST_HEAD(, nbr)	 nbr_list;
	LIST_HEAD(, lde_nbr)	 lde_nbr_list;

	char			 name[IF_NAMESIZE];
	struct in_addr		 addr;
	struct in_addr		 dst;
	struct in_addr		 mask;
	struct nbr		*self;

	u_int16_t		 lspace_id;

	u_int64_t		 baudrate;
	time_t			 uptime;
	unsigned int		 ifindex;
	int			 discovery_fd;
	int			 session_fd;
	int			 state;
	int			 mtu;
	u_int16_t		 holdtime;
	u_int16_t		 keepalive;
	u_int16_t		 hello_interval;
	u_int16_t		 flags;
	enum iface_type		 type;
	u_int8_t		 media_type;
	u_int8_t		 linkstate;
	u_int8_t		 priority;
	u_int8_t		 passive;
};

/* ldp_conf */
enum {
	PROC_MAIN,
	PROC_LDP_ENGINE,
	PROC_LDE_ENGINE
} ldpd_process;

#define	MODE_DIST_INDEPENDENT	0x01
#define	MODE_DIST_ORDERED	0x02
#define	MODE_RET_LIBERAL	0x04
#define	MODE_RET_CONSERVATIVE	0x08
#define	MODE_ADV_ONDEMAND	0x10
#define	MODE_ADV_UNSOLICITED	0x20

struct ldpd_conf {
	struct event		disc_ev, sess_ev;
	struct in_addr		rtr_id;
	LIST_HEAD(, iface)	iface_list;

	u_int32_t		opts;
#define LDPD_OPT_VERBOSE	0x00000001
#define LDPD_OPT_VERBOSE2	0x00000002
#define LDPD_OPT_NOACTION	0x00000004
	time_t			uptime;
	int			ldp_discovery_socket;
	int			ldp_session_socket;
	int			flags;
	u_int8_t		mode;
};

/* kroute */
struct kroute {
	struct in_addr	prefix;
	struct in_addr	nexthop;
	u_int32_t	local_label;
	u_int32_t	remote_label;
	u_int16_t	flags;
	u_int16_t	rtlabel;
	u_int32_t	ext_tag;
	u_short		ifindex;
	u_int8_t	prefixlen;
};

struct rroute {
	struct kroute	kr;
	u_int32_t	metric;
};

struct kif_addr {
	TAILQ_ENTRY(kif_addr)	 entry;
	struct in_addr		 addr;
	struct in_addr		 mask;
	struct in_addr		 dstbrd;
};

struct kif {
	char			 ifname[IF_NAMESIZE];
	u_int64_t		 baudrate;
	int			 flags;
	int			 mtu;
	u_short			 ifindex;
	u_int8_t		 media_type;
	u_int8_t		 link_state;
	u_int8_t		 nh_reachable;	/* for nexthop verification */
};

/* name2id */
struct n2id_label {
	TAILQ_ENTRY(n2id_label)	 entry;
	char			*name;
	u_int16_t		 id;
	u_int32_t		 ext_tag;
	int			 ref;
};

TAILQ_HEAD(n2id_labels, n2id_label);
extern struct n2id_labels rt_labels;

/* control data structures */
struct ctl_iface {
	char			 name[IF_NAMESIZE];
	struct in_addr		 addr;
	struct in_addr		 mask;
	struct in_addr		 lspace;
	struct in_addr		 rtr_id;
	struct in_addr		 dr_id;
	struct in_addr		 dr_addr;
	struct in_addr		 bdr_id;
	struct in_addr		 bdr_addr;
	time_t			 hello_timer;
	time_t			 uptime;
	u_int64_t		 baudrate;
	unsigned int		 ifindex;
	int			 state;
	int			 mtu;
	int			 nbr_cnt;
	int			 adj_cnt;
	u_int16_t		 flags;
	u_int16_t		 holdtime;
	u_int16_t		 hello_interval;
	enum iface_type		 type;
	u_int8_t		 linkstate;
	u_int8_t		 mediatype;
	u_int8_t		 priority;
	u_int8_t		 passive;
};

struct ctl_nbr {
	char			 name[IF_NAMESIZE];
	struct in_addr		 id;
	struct in_addr		 addr;
	struct in_addr		 dr;
	struct in_addr		 bdr;
	struct in_addr		 lspace;
	time_t			 dead_timer;
	time_t			 uptime;
	u_int32_t		 db_sum_lst_cnt;
	u_int32_t		 ls_req_lst_cnt;
	u_int32_t		 ls_retrans_lst_cnt;
	u_int32_t		 state_chng_cnt;
	int			 nbr_state;
	int			 iface_state;
	u_int8_t		 priority;
	u_int8_t		 options;
};

struct ctl_rt {
	struct in_addr		 prefix;
	struct in_addr		 nexthop;
	struct in_addr		 lspace;
	struct in_addr		 adv_rtr;
	time_t			 uptime;
	u_int32_t		 local_label;
	u_int32_t		 remote_label;
	u_int8_t		 flags;
	u_int8_t		 prefixlen;
	u_int8_t		 connected;
};

struct ctl_sum {
	struct in_addr		 rtr_id;
	u_int32_t		 spf_delay;
	u_int32_t		 spf_hold_time;
	u_int32_t		 num_ext_lsa;
	u_int32_t		 num_lspace;
	time_t			 uptime;
	u_int8_t		 rfc1583compat;
};

struct ctl_sum_lspace {
	struct in_addr		 lspace;
	u_int32_t		 num_iface;
	u_int32_t		 num_adj_nbr;
	u_int32_t		 num_spf_calc;
	u_int32_t		 num_lsa;
};

/* parse.y */
struct ldpd_conf	*parse_config(char *, int);
int			 cmdline_symset(char *);

/* in_cksum.c */
u_int16_t	 in_cksum(void *, size_t);

/* iso_cksum.c */
u_int16_t	 iso_cksum(void *, u_int16_t, u_int16_t);

/* kroute.c */
int		 kif_init(void);
int		 kr_init(int);
int		 kr_change(struct kroute *);
int		 kr_delete(struct kroute *);
void		 kr_shutdown(void);
void		 kr_lfib_couple(void);
void		 kr_lfib_decouple(void);
void		 kr_dispatch_msg(int, short, void *);
void		 kr_show_route(struct imsg *);
void		 kr_ifinfo(char *, pid_t);
struct kif	*kif_findname(char *, struct in_addr, struct kif_addr **);
void		 kr_reload(void);
int		 kroute_insert_label(struct kroute *);

u_int8_t	mask2prefixlen(in_addr_t);
in_addr_t	prefixlen2mask(u_int8_t);

/* log.h */
const char	*nbr_state_name(int);
const char	*if_state_name(int);
const char	*if_type_name(enum iface_type);

/* name2id.c */
u_int16_t	 rtlabel_name2id(const char *);
const char	*rtlabel_id2name(u_int16_t);
void		 rtlabel_unref(u_int16_t);
u_int32_t	 rtlabel_id2tag(u_int16_t);
u_int16_t	 rtlabel_tag2id(u_int32_t);
void		 rtlabel_tag(u_int16_t, u_int32_t);

/* ldpd.c */
void	main_imsg_compose_ldpe(int, pid_t, void *, u_int16_t);
void	main_imsg_compose_lde(int, pid_t, void *, u_int16_t);
void	merge_config(struct ldpd_conf *, struct ldpd_conf *);
int	imsg_compose_event(struct imsgev *, u_int16_t, u_int32_t, pid_t,
	    int, void *, u_int16_t);
void	imsg_event_add(struct imsgev *);


/* printconf.c */
void	print_config(struct ldpd_conf *);

#endif	/* _LDPD_H_ */
