/*	$OpenBSD: src/usr.bin/window/Attic/wwgets.c,v 1.3 1996/06/26 05:43:43 deraadt Exp $	*/
/*	$NetBSD: wwgets.c,v 1.6 1996/02/08 20:45:08 mycroft Exp $	*/

/*
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Edward Wang at The University of California, Berkeley.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
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
static char sccsid[] = "@(#)wwgets.c	8.1 (Berkeley) 6/6/93";
#else
static char rcsid[] = "$OpenBSD: src/usr.bin/window/Attic/wwgets.c,v 1.3 1996/06/26 05:43:43 deraadt Exp $";
#endif
#endif /* not lint */

#include "ww.h"
#include "char.h"
#include <string.h>

wwgets(buf, n, w)
char *buf;
int n;
register struct ww *w;
{
	register char *p = buf;
	register int c;
	int uc = ISSET(w->ww_wflags, WWW_UNCTRL);
	static void rub();

	CLR(w->ww_wflags, WWW_UNCTRL);
	for (;;) {
		wwcurtowin(w);
		while ((c = wwgetc()) < 0)
			wwiomux();
#ifdef OLD_TTY
		if (c == wwoldtty.ww_sgttyb.sg_erase)
#else
		if (c == wwoldtty.ww_termios.c_cc[VERASE])
#endif
		{
			if (p > buf)
				rub(*--p, w);
		} else
#ifdef OLD_TTY
		if (c == wwoldtty.ww_sgttyb.sg_kill)
#else
		if (c == wwoldtty.ww_termios.c_cc[VKILL])
#endif
		{
			while (p > buf)
				rub(*--p, w);
		} else
#ifdef OLD_TTY
		if (c == wwoldtty.ww_ltchars.t_werasc)
#else
		if (c == wwoldtty.ww_termios.c_cc[VWERASE])
#endif
		{
			while (--p >= buf && (*p == ' ' || *p == '\t'))
				rub(*p, w);
			while (p >= buf && *p != ' ' && *p != '\t')
				rub(*p--, w);
			p++;
		} else if (c == '\r' || c == '\n') {
			break;
		} else {
			if (p >= buf + n - 1)
				wwputc(ctrl('g'), w);
			else
				wwputs(unctrl(*p++ = c), w);
		}
	}
	*p = 0;
	SET(w->ww_wflags, uc);
}

static void
rub(c, w)
struct ww *w;
{
	register i;

	for (i = strlen(unctrl(c)); --i >= 0;)
		(void) wwwrite(w, "\b \b", 3);
}
