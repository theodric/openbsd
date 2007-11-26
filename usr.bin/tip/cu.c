/*	$OpenBSD: src/usr.bin/tip/cu.c,v 1.23 2007/11/26 09:28:34 martynas Exp $	*/
/*	$NetBSD: cu.c,v 1.5 1997/02/11 09:24:05 mrg Exp $	*/

/*
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
#if 0
static char sccsid[] = "@(#)cu.c	8.1 (Berkeley) 6/6/93";
#endif
static const char rcsid[] = "$OpenBSD: src/usr.bin/tip/cu.c,v 1.23 2007/11/26 09:28:34 martynas Exp $";
#endif /* not lint */

#include <err.h>
#include <paths.h>

#include "tip.h"

static void	cuusage(void);

/*
 * Botch the interface to look like cu's
 */
void
cumain(int argc, char *argv[])
{
	int ch, i, parity;
	const char *errstr;
	static char sbuf[12];

	if (argc < 2)
		cuusage();
	CU = DV = NULL;
	BR = DEFBR;
	parity = 0;	/* none */

	/*
	 * Convert obsolecent -### speed to modern -s### syntax which
	 * getopt() can handle.
	 */
	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			switch (argv[i][1]) {
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				ch = snprintf(sbuf, sizeof(sbuf), "-s%s",
				    &argv[i][1]);
				if (ch <= 0 || ch >= sizeof(sbuf)) {
					errx(3, "invalid speed: %s",
					    &argv[i][1]);
				}
				argv[i] = sbuf;
				break;
			case '-':
				/* if we get "--" stop processing args */
				if (argv[i][2] == '\0')
					goto getopt;
				break;
			}
		}
	}

getopt:
	while ((ch = getopt(argc, argv, "a:l:s:htoe")) != -1) {
		switch (ch) {
		case 'a':
			if (optarg[0] == '\0')
				errx(3, "invalid acu: \"\"");
			CU = optarg;
			break;
		case 'l':
			if (DV != NULL) {
				fprintf(stderr,
				    "%s: cannot specify multiple -l options\n",
				    __progname);
				exit(3);
			}
			if (strchr(optarg, '/'))
				DV = optarg;
			else
				if (asprintf(&DV, "%s%s", _PATH_DEV, optarg) == -1)
					err(3, "asprintf");
			break;
		case 's':
			BR = (int)strtonum(optarg, 0, INT_MAX, &errstr);
			if (errstr)
				errx(3, "speed is %s: %s", errstr, optarg);
			break;
		case 'h':
			setboolean(value(LECHO), TRUE);
			HD = TRUE;
			break;
		case 't':
			HW = 1, DU = -1;
			break;
		case 'o':
			if (parity != 0)
				parity = 0;	/* -e -o */
			else
				parity = 1;	/* odd */
			break;
		case 'e':
			if (parity != 0)
				parity = 0;	/* -o -e */
			else
				parity = -1;	/* even */
			break;
		default:
			cuusage();
			break;
		}
	}
	argc -= optind;
	argv += optind;

	switch (argc) {
	case 1:
		PN = argv[0];
		break;
	case 0:
		break;
	default:
		cuusage();
		break;
	}

	signal(SIGINT, cleanup);
	signal(SIGQUIT, cleanup);
	signal(SIGHUP, cleanup);
	signal(SIGTERM, cleanup);
	signal(SIGCHLD, SIG_DFL);

	/*
	 * The "cu" host name is used to define the
	 * attributes of the generic dialer.
	 */
	(void)snprintf(sbuf, sizeof(sbuf), "cu%ld", BR);
	if ((i = hunt(sbuf)) == 0) {
		printf("all ports busy\n");
		exit(3);
	}
	if (i == -1) {
		printf("link down\n");
		(void)uu_unlock(uucplock);
		exit(3);
	}
	setbuf(stdout, NULL);
	loginit();
	user_uid();
	vinit();
	switch (parity) {
	case -1:
		setparity("even");
		break;
	case 1:
		setparity("odd");
		break;
	default:
		setparity("none");
		break;
	}
	setboolean(value(VERBOSE), FALSE);
	if (HW && ttysetup(BR)) {
		fprintf(stderr, "%s: unsupported speed %ld\n",
		    __progname, BR);
		daemon_uid();
		(void)uu_unlock(uucplock);
		exit(3);
	}
	if (con()) {
		printf("Connect failed\n");
		daemon_uid();
		(void)uu_unlock(uucplock);
		exit(1);
	}
	if (!HW && ttysetup(BR)) {
		fprintf(stderr, "%s: unsupported speed %ld\n",
		    __progname, BR);
		daemon_uid();
		(void)uu_unlock(uucplock);
		exit(3);
	}
}

static void
cuusage(void)
{
	fprintf(stderr, "usage: cu [-ehot] [-a acu] [-l line] "
	    "[-s speed | -speed] [phone-number]\n");
	exit(8);
}
