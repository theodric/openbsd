/*	$OpenBSD: src/lib/libc/gen/vwarnx.c,v 1.1 1996/04/21 23:39:35 deraadt Exp $ */
/*	$NetBSD: vwarnx.c,v 1.1 1996/04/15 23:45:40 jtc Exp $	*/

/*-
 * Copyright (c) 1993
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

#if defined(LIBC_SCCS) && !defined(lint)
#if 0
static char sccsid[] = "@(#)err.c	8.1 (Berkeley) 6/4/93";
#else
static char rcsid[] = "$NetBSD: vwarnx.c,v 1.1 1996/04/15 23:45:40 jtc Exp $";
#endif
#endif /* LIBC_SCCS and not lint */

#include <err.h>
#include <stdio.h>

#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

extern char *__progname;		/* Program name, from crt0. */

void
_vwarnx(fmt, ap)
	const char *fmt;
	va_list ap;
{
	(void)fprintf(stderr, "%s: ", __progname);
	if (fmt != NULL)
		(void)vfprintf(stderr, fmt, ap);
	(void)fprintf(stderr, "\n");
}
