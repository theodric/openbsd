/*	$OpenBSD: src/usr.bin/make/lst.lib/Attic/lstAtFront.c,v 1.2 1996/06/26 05:36:43 deraadt Exp $	*/
/*	$NetBSD: lstAtFront.c,v 1.4 1995/06/14 15:20:48 christos Exp $	*/

/*
 * Copyright (c) 1988, 1989, 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Adam de Boor.
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
static char sccsid[] = "@(#)lstAtFront.c	5.3 (Berkeley) 6/1/90";
#else
static char rcsid[] = "$OpenBSD: src/usr.bin/make/lst.lib/Attic/lstAtFront.c,v 1.2 1996/06/26 05:36:43 deraadt Exp $";
#endif
#endif /* not lint */

/*-
 * LstAtFront.c --
 *	Add a node at the front of the list
 */

#include	"lstInt.h"

/*-
 *-----------------------------------------------------------------------
 * Lst_AtFront --
 *	Place a piece of data at the front of a list
 *
 * Results:
 *	SUCCESS or FAILURE
 *
 * Side Effects:
 *	A new ListNode is created and stuck at the front of the list.
 *	hence, firstPtr (and possible lastPtr) in the list are altered.
 *
 *-----------------------------------------------------------------------
 */
ReturnStatus
Lst_AtFront (l, d)
    Lst		l;
    ClientData	d;
{
    register LstNode	front;
    
    front = Lst_First (l);
    return (Lst_Insert (l, front, d));
}
