/*	$OpenBSD: src/games/boggle/boggle/bog.h,v 1.3 2003/06/03 03:01:39 millert Exp $	*/
/*	$NetBSD: bog.h,v 1.2 1995/03/21 12:14:32 cgd Exp $	*/

/*-
 * Copyright (c) 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Barry Brachman.
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
 *
 *	@(#)bog.h	8.1 (Berkeley) 6/11/93
 */

#define LOADDICT		1	/* Load the dictionary for speed */

/*
 * Locations for the dictionary (generated by mkdict),
 * index (generated by mkindex), and helpfile
 */
#define DICT			"/usr/share/games/boggle/dictionary"
#define DICTINDEX		"/usr/share/games/boggle/dictindex"
#define HELPFILE		"/usr/share/games/boggle/helpfile"

/*
 * The theoretical maximum for MAXWORDLEN is ('a' - 1) == 96
 */
#define MAXWORDLEN		40	/* Maximum word length */
#define MAXPWORDS		200	/* Maximum number of player's words */
#define MAXMWORDS		200	/* Maximum number of machine's words */
#define MAXPSPACE		2000	/* Space for player's words */
#define MAXMSPACE		4000	/* Space for machines's words */

#define MAXCOLS			20

/*
 * The following determine the screen layout
 */
#define PROMPT_COL		20
#define PROMPT_LINE		2

#define BOARD_COL		0
#define BOARD_LINE		0

#define SCORE_COL		20
#define SCORE_LINE		0

#define LIST_COL		0
#define LIST_LINE		10

#define TIMER_COL		20
#define TIMER_LINE		2

/*
 * Internal dictionary index
 * Initialized from the file created by mkindex
 */
struct dictindex {
	long start;
	long length;
};
