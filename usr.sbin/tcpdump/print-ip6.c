/*	$OpenBSD: src/usr.sbin/tcpdump/print-ip6.c,v 1.11 2009/10/27 23:59:55 deraadt Exp $	*/

/*
 * Copyright (c) 1988, 1989, 1990, 1991, 1992, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution, and (3) all advertising materials mentioning
 * features or use of this software display the following acknowledgement:
 * ``This product includes software developed by the University of California,
 * Lawrence Berkeley Laboratory and its contributors.'' Neither the name of
 * the University nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior
 * written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifdef INET6

#include <sys/param.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>
#include <netinet/tcp.h>

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "interface.h"
#include "addrtoname.h"

#include <netinet/ip6.h>

/*
 * print an IP6 datagram.
 */
void
ip6_print(register const u_char *bp, register int length)
{
	register const struct ip6_hdr *ip6;
	register int hlen;
	register int len;
	register const u_char *cp;
	int nh;
	u_int flow;
	
	ip6 = (const struct ip6_hdr *)bp;

	/*
	 * The IP header is not word aligned, so copy into abuf.
	 * This will never happen with BPF.  It does happen with
	 * raw packet dumps from -r.
	 */
	if ((intptr_t)ip6 & (sizeof(long)-1)) {
		static u_char *abuf = NULL;
		static int didwarn = 0;
		int clen = snapend - bp;
		if (clen > snaplen)
			clen = snaplen;

		if (abuf == NULL) {
			abuf = (u_char *)malloc(snaplen);
			if (abuf == NULL)
				error("ip6_print: malloc");
		}
		memmove((char *)abuf, (char *)ip6, min(length, clen));
		snapend = abuf + clen;
		packetp = abuf;
		ip6 = (struct ip6_hdr *)abuf;
		/* We really want libpcap to give us aligned packets */
		if (!didwarn) {
			warning("compensating for unaligned libpcap packets");
			++didwarn;
		}
	}

	if ((u_char *)(ip6 + 1) > snapend) {
		printf("[|ip6]");
		return;
	}
	if (length < sizeof (struct ip6_hdr)) {
		(void)printf("truncated-ip6 %d", length);
		return;
	}
	if ((ip6->ip6_vfc & IPV6_VERSION_MASK) != IPV6_VERSION) {
		(void)printf("bad-ip6-version %u", ip6->ip6_vfc >> 4);
		return;
	}
	hlen = sizeof(struct ip6_hdr);

	len = ntohs(ip6->ip6_plen);
	if (length < len + hlen)
		(void)printf("truncated-ip6 - %d bytes missing!",
			len + hlen - length);

	cp = (const u_char *)ip6;
	nh = ip6->ip6_nxt;
	while (cp + hlen < snapend) {
		cp += hlen;

		if (cp == (u_char *)(ip6 + 1) &&
		    nh != IPPROTO_TCP && nh != IPPROTO_UDP &&
		    nh != IPPROTO_ESP && nh != IPPROTO_AH) {
			(void)printf("%s > %s: ", ip6addr_string(&ip6->ip6_src),
				     ip6addr_string(&ip6->ip6_dst));
		}

		switch (nh) {
		case IPPROTO_HOPOPTS:
			hlen = hbhopt_print(cp);
			nh = *cp;
			break;
		case IPPROTO_DSTOPTS:
			hlen = dstopt_print(cp);
			nh = *cp;
			break;
		case IPPROTO_FRAGMENT:
			hlen = frag6_print(cp, (const u_char *)ip6);
			if (snapend <= cp + hlen)
				goto end;
			nh = *cp;
			break;
		case IPPROTO_ROUTING:
			hlen = rt6_print(cp, (const u_char *)ip6);
			nh = *cp;
			break;
		case IPPROTO_TCP:
			tcp_print(cp, len + sizeof(struct ip6_hdr) - (cp - bp),
				(const u_char *)ip6);
			goto end;
		case IPPROTO_UDP:
			udp_print(cp, len + sizeof(struct ip6_hdr) - (cp - bp),
				(const u_char *)ip6);
			goto end;
		case IPPROTO_ESP:
			esp_print(cp, len + sizeof(struct ip6_hdr) - (cp - bp),
				(const u_char *)ip6);
			goto end;
		case IPPROTO_AH:
			ah_print(cp, len + sizeof(struct ip6_hdr) - (cp - bp),
				(const u_char *)ip6);
			goto end;
		case IPPROTO_ICMPV6:
			icmp6_print(cp, (const u_char *)ip6);
			goto end;
		case IPPROTO_PIM:
			(void)printf("PIM");
			pim_print(cp, len);
			goto end;
#ifndef IPPROTO_OSPF
#define IPPROTO_OSPF 89
#endif
		case IPPROTO_OSPF:
			ospf6_print(cp, len);
			goto end;
		case IPPROTO_IPV6:
			ip6_print(cp, len);
			goto end;
#ifndef IPPROTO_IPV4
#define IPPROTO_IPV4	4
#endif
		case IPPROTO_IPV4:
			ip_print(cp, len);
			goto end;
		case IPPROTO_NONE:
			(void)printf("no next header");
			goto end;

#ifndef IPPROTO_CARP  
#define IPPROTO_CARP 112
#endif
		case IPPROTO_CARP:
			if (packettype == PT_VRRP)
				vrrp_print(cp, len, ip6->ip6_hlim);
			else
				carp_print(cp, len, ip6->ip6_hlim);
			break;

		default:
			(void)printf("ip-proto-%d %d", ip6->ip6_nxt, len);
			goto end;
		}
		if (hlen == 0)
			break;
	}

 end:
	
	flow = ntohl(ip6->ip6_flow);
#if 0
	/* rfc1883 */
	if (flow & 0x0f000000)
		(void)printf(" [pri 0x%x]", (flow & 0x0f000000) >> 24);
	if (flow & 0x00ffffff)
		(void)printf(" [flowlabel 0x%x]", flow & 0x00ffffff);
#else
	/* RFC 2460 */
	if (flow & 0x0ff00000)
		(void)printf(" [class 0x%x]", (flow & 0x0ff00000) >> 20);
	if (flow & 0x000fffff)
		(void)printf(" [flowlabel 0x%x]", flow & 0x000fffff);
#endif

	if (ip6->ip6_hlim <= 1)
		(void)printf(" [hlim %d]", (int)ip6->ip6_hlim);

	if (vflag) {
		printf(" (");
		(void)printf("len %d", len);
		if (ip6->ip6_hlim > 1)
			(void)printf(", hlim %d", (int)ip6->ip6_hlim);
		printf(")");
	}
}

#endif /* INET6 */
