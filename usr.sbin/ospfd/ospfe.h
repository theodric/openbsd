/*	$OpenBSD: src/usr.sbin/ospfd/ospfe.h,v 1.19 2005/12/15 20:29:06 claudio Exp $ */

/*
 * Copyright (c) 2004, 2005 Esben Norby <norby@openbsd.org>
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

#ifndef _OSPFE_H_
#define _OSPFE_H_

#define max(x,y) ((x) > (y) ? (x) : (y))

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>

TAILQ_HEAD(ctl_conns, ctl_conn)	ctl_conns;

/* neighbor states */
#define	NBR_STA_DOWN		0x0001
#define	NBR_STA_ATTEMPT		0x0002
#define	NBR_STA_INIT		0x0004
#define	NBR_STA_2_WAY		0x0008
#define	NBR_STA_XSTRT		0x0010
#define NBR_STA_SNAP		0x0020
#define	NBR_STA_XCHNG		0x0040
#define	NBR_STA_LOAD		0x0080
#define	NBR_STA_FULL		0x0100
#define	NBR_STA_ACTIVE		(~NBR_STA_DOWN)
#define	NBR_STA_FLOOD		(NBR_STA_XCHNG | NBR_STA_LOAD | NBR_STA_FULL)
#define	NBR_STA_ADJFORM		(NBR_STA_XSTRT | NBR_STA_SNAP | NBR_STA_FLOOD)
#define	NBR_STA_BIDIR		(NBR_STA_2_WAY | NBR_STA_ADJFORM)
#define	NBR_STA_PRELIM		(NBR_STA_DOWN | NBR_STA_ATTEMPT | NBR_STA_INIT)
#define	NBR_STA_ANY		0xffff

/* neighbor events */
enum nbr_event {
	NBR_EVT_NOTHING,
	NBR_EVT_HELLO_RCVD,
	NBR_EVT_2_WAY_RCVD,
	NBR_EVT_NEG_DONE,
	NBR_EVT_SNAP_DONE,
	NBR_EVT_XCHNG_DONE,
	NBR_EVT_BAD_LS_REQ,
	NBR_EVT_LOAD_DONE,
	NBR_EVT_ADJ_OK,
	NBR_EVT_SEQ_NUM_MIS,
	NBR_EVT_1_WAY_RCVD,
	NBR_EVT_KILL_NBR,
	NBR_EVT_ITIMER,
	NBR_EVT_LL_DOWN,
	NBR_EVT_ADJTMOUT
};

/* neighbor actions */
enum nbr_action {
	NBR_ACT_NOTHING,
	NBR_ACT_RST_ITIMER,
	NBR_ACT_STRT_ITIMER,
	NBR_ACT_EVAL,
	NBR_ACT_SNAP,
	NBR_ACT_SNAP_DONE,
	NBR_ACT_XCHNG_DONE,
	NBR_ACT_ADJ_OK,
	NBR_ACT_RESTRT_DD,
	NBR_ACT_DEL,
	NBR_ACT_CLR_LST,
	NBR_ACT_HELLO_CHK
};

struct lsa_entry {
	TAILQ_ENTRY(lsa_entry)	 entry;
	union {
		struct lsa_hdr	*lu_lsa;
		struct lsa_ref	*lu_ref;
	}			 le_data;
};
#define	le_lsa	le_data.lu_lsa
#define	le_ref	le_data.lu_ref

struct lsa_ref {
	LIST_ENTRY(lsa_ref)	 entry;
	struct lsa_hdr		 hdr;
	void			*data;
	time_t			 stamp;
	int			 refcnt;
	u_int16_t		 len;
};

struct nbr_stats {
	u_int32_t		 sta_chng;
};

struct nbr {
	LIST_ENTRY(nbr)		 entry, hash;
	struct event		 inactivity_timer;
	struct event		 db_tx_timer;
	struct event		 lsreq_tx_timer;
	struct event		 ls_retrans_timer;
	struct event		 adj_timer;

	struct nbr_stats	 stats;

	struct lsa_head		 ls_retrans_list;
	struct lsa_head		 db_sum_list;
	struct lsa_head		 ls_req_list;

	struct in_addr		 addr;
	struct in_addr		 id;
	struct in_addr		 dr;		/* designated router */
	struct in_addr		 bdr;		/* backup designated router */

	struct iface		*iface;
	struct lsa_entry	*ls_req;
	struct lsa_entry	*dd_end;

	u_int32_t		 dd_seq_num;
	u_int32_t		 dd_pending;
	u_int32_t		 peerid;	/* unique ID in DB */
	u_int32_t		 ls_req_cnt;
	u_int32_t		 crypt_seq_num;

	int			 state;
	u_int8_t		 priority;
	u_int8_t		 options;
	u_int8_t		 last_rx_options;
	u_int8_t		 last_rx_bits;
	u_int8_t		 master;
};

/* auth.c */
int		 auth_validate(void *buf, u_int16_t len, struct iface *,
		     struct nbr *);
int		 auth_gen(struct buf *, struct iface *);
void		 md_list_init(struct iface *);
void		 md_list_add(struct iface *, u_int8_t, char *);
void		 md_list_clr(struct iface *);
int		 md_list_empty(struct iface *);
struct auth_md	*md_list_find(struct iface *, u_int8_t);

/* database.c */
int	 send_db_description(struct nbr *);
void	 recv_db_description(struct nbr *, char *, u_int16_t);
void	 db_sum_list_add(struct nbr *, struct lsa_hdr *);
int	 db_sum_list_del(struct nbr *, struct lsa_hdr *);
void	 db_sum_list_clr(struct nbr *);
void	 db_tx_timer(int, short, void *);
int	 start_db_tx_timer(struct nbr *);
int	 stop_db_tx_timer(struct nbr *);

/* hello.c */
int	 send_hello(struct iface *);
void	 recv_hello(struct iface *,  struct in_addr, u_int32_t,
	     char *, u_int16_t);

/* ospfe.c */
pid_t		 ospfe(struct ospfd_conf *, int[2], int[2], int[2]);
void		 ospfe_dispatch_main(int, short, void *);
void		 ospfe_dispatch_rde(int, short, void *);
int		 ospfe_imsg_compose_parent(int, pid_t, void *, u_int16_t);
int		 ospfe_imsg_compose_rde(int, u_int32_t, pid_t, void *,
		     u_int16_t);
u_int32_t	 ospfe_router_id(void);
void		 ospfe_iface_ctl(struct ctl_conn *, unsigned int);
void		 ospfe_nbr_ctl(struct ctl_conn *);
void		 orig_rtr_lsa(struct area *);
void		 orig_net_lsa(struct iface *);

/* interface.c */
int		 if_fsm(struct iface *, enum iface_event);

struct iface	*if_new(struct kif *);
void		 if_del(struct iface *);
void		 if_init(struct ospfd_conf *, struct iface *);

int		 if_act_start(struct iface *);
int		 if_act_elect(struct iface *);
int		 if_act_reset(struct iface *);

struct ctl_iface	*if_to_ctl(struct iface *);

const char	*if_state_name(int);
const char	*if_event_name(int);
const char	*if_action_name(int);
const char	*if_type_name(int);
const char	*if_auth_name(int);

int	 if_join_group(struct iface *, struct in_addr *);
int	 if_leave_group(struct iface *, struct in_addr *);
int	 if_set_mcast(struct iface *);
int	 if_set_mcast_ttl(int, u_int8_t);
int	 if_set_tos(int, int);
void	 if_set_recvbuf(int);
int	 if_set_mcast_loop(int);

/* lsack.c */
int	 delay_lsa_ack(struct iface *, struct lsa_hdr *);
int	 send_ls_ack(struct iface *, struct in_addr, void *data, int len);
void	 recv_ls_ack(struct nbr *, char *, u_int16_t);
int	 lsa_hdr_check(struct nbr *, struct lsa_hdr *);
void	 ls_ack_list_add(struct iface *, struct lsa_hdr *);
void	 ls_ack_list_free(struct iface *, struct lsa_entry *);
void	 ls_ack_list_clr(struct iface *);
int	 ls_ack_list_empty(struct iface *);
void	 ls_ack_tx_timer(int, short, void *);
int	 start_ls_ack_tx_timer(struct iface *);
int	 stop_ls_ack_tx_timer(struct iface *);

/* lsreq.c */
int	 send_ls_req(struct nbr *);
void	 recv_ls_req(struct nbr *, char *, u_int16_t);
void	 ls_req_list_add(struct nbr *, struct lsa_hdr *);
int	 ls_req_list_del(struct nbr *, struct lsa_hdr *);
struct lsa_entry	*ls_req_list_get(struct nbr *, struct lsa_hdr *);
void	 ls_req_list_free(struct nbr *, struct lsa_entry *);
void	 ls_req_list_clr(struct nbr *);
int	 ls_req_list_empty(struct nbr *);
void	 ls_req_tx_timer(int, short, void *);
int	 start_ls_req_tx_timer(struct nbr *);
int	 stop_ls_req_tx_timer(struct nbr *);

/* lsupdate.c */
int		 lsa_flood(struct iface *, struct nbr *, struct lsa_hdr *,
		     void *, u_int16_t);
int		 send_ls_update(struct iface *, struct in_addr, void *, int);
void		 recv_ls_update(struct nbr *, char *, u_int16_t);

void		 ls_retrans_list_add(struct nbr *, struct lsa_hdr *);
int		 ls_retrans_list_del(struct nbr *, struct lsa_hdr *);
struct lsa_entry	*ls_retrans_list_get(struct nbr *, struct lsa_hdr *);
void		 ls_retrans_list_free(struct nbr *, struct lsa_entry *);
void		 ls_retrans_list_clr(struct nbr *);
int		 ls_retrans_list_empty(struct nbr *);
void		 ls_retrans_timer(int, short, void *);

void		 lsa_cache_init(u_int32_t);
struct lsa_ref	*lsa_cache_add(void *, u_int16_t);
struct lsa_ref	*lsa_cache_get(struct lsa_hdr *);
void		 lsa_cache_put(struct lsa_ref *, struct nbr *);

/* neighbor.c */
void		 nbr_init(u_int32_t);
struct nbr	*nbr_new(u_int32_t, struct iface *, int);
void		 nbr_del(struct nbr *);

struct nbr	*nbr_find_id(struct iface *, u_int32_t);
struct nbr	*nbr_find_peerid(u_int32_t);

int	 nbr_fsm(struct nbr *, enum nbr_event);

void	 nbr_itimer(int, short, void *);
int	 nbr_start_itimer(struct nbr *);
int	 nbr_stop_itimer(struct nbr *);
int	 nbr_reset_itimer(struct nbr *);

void	 nbr_adj_timer(int, short, void *);
int	 nbr_start_adj_timer(struct nbr *);

int	 nbr_act_reset_itimer(struct nbr *);
int	 nbr_act_start_itimer(struct nbr *);
int	 nbr_act_eval(struct nbr *);
int	 nbr_act_snapshot(struct nbr *);
int	 nbr_act_exchange_done(struct nbr *);
int	 nbr_act_adj_ok(struct nbr *);
int	 nbr_act_restart_dd(struct nbr *);
int	 nbr_act_delete(struct nbr *);
int	 nbr_act_clear_lists(struct nbr *);
int	 nbr_act_hello_check(struct nbr *);

struct ctl_nbr	*nbr_to_ctl(struct nbr *);

const char	*nbr_state_name(int);
const char	*nbr_event_name(int);
const char	*nbr_action_name(int);

struct lsa_hdr	*lsa_hdr_new(void);

/* packet.c */
int	 gen_ospf_hdr(struct buf *, struct iface *, u_int8_t);
int	 send_packet(struct iface *, char *, int, struct sockaddr_in *);
void	 recv_packet(int, short, void *);

#endif	/* _OSPFE_H_ */
