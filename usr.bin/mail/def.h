/*	$OpenBSD: src/usr.bin/mail/def.h,v 1.13 2003/06/25 15:13:32 millert Exp $	*/
/*	$NetBSD: def.h,v 1.9 1996/12/28 07:11:00 tls Exp $	*/

/*
 * Copyright (c) 1980, 1993
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
 *
 *	@(#)def.h	8.4 (Berkeley) 4/20/95
 *	$OpenBSD: src/usr.bin/mail/def.h,v 1.13 2003/06/25 15:13:32 millert Exp $
 */

/*
 * Mail -- a mail program
 *
 * Author: Kurt Shoens (UCB) March 25, 1978
 */

#ifndef MAIL_DEF_H
#define MAIL_DEF_H

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <ctype.h>
#include <err.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <vis.h>
#include "pathnames.h"

#define	APPEND				/* New mail goes to end of mailbox */

#define	ESCAPE		'~'		/* Default escape for sending */
#define	NMLSIZE		1024		/* max names in a message list */
#define	PATHSIZE	MAXPATHLEN	/* Size of pathnames throughout */
#define	HSHSIZE		59		/* Hash size for aliases and vars */
#define	LINESIZE	BUFSIZ		/* max readable line width */
#define	STRINGSIZE	((unsigned) 128)/* Dynamic allocation units */
#define	MAXARGC		1024		/* Maximum list of raw strings */
#define	MAXEXP		25		/* Maximum expansion of aliases */

#define	equal(a, b)	(strcmp(a,b)==0)/* A nice function to string compare */

struct message {
	short	m_flag;			/* flags, see below */
	int	m_offset;		/* offset in block of message */
	int	m_block;		/* block number of this message */
	int	m_size;			/* Bytes in the message */
	int	m_lines;		/* Lines in the message */
};

/*
 * flag bits.
 */
#define	MUSED		(1<<0)		/* entry is used, but this bit isn't */
#define	MDELETED	(1<<1)		/* entry has been deleted */
#define	MSAVED		(1<<2)		/* entry has been saved */
#define	MTOUCH		(1<<3)		/* entry has been noticed */
#define	MPRESERVE	(1<<4)		/* keep entry in sys mailbox */
#define	MMARK		(1<<5)		/* message is marked! */
#define	MODIFY		(1<<6)		/* message has been modified */
#define	MNEW		(1<<7)		/* message has never been seen */
#define	MREAD		(1<<8)		/* message has been read sometime. */
#define	MSTATUS		(1<<9)		/* message status has changed */
#define	MBOX		(1<<10)		/* Send this to mbox, regardless */

/*
 * Given a file address, determine the block number it represents.
 */
#define blockof(off)			((int) ((off) / 4096))
#define offsetof(off)			((int) ((off) % 4096))
#define positionof(block, offset)	((off_t)(block) * 4096 + (offset))

/*
 * Format of the command description table.
 * The actual table is declared and initialized
 * in lex.c
 */
struct cmd {
	char	*c_name;		/* Name of command */
	union {
		int	(*c_func1)(void *);
		int	(*c_func2)(void *, void *);
	} cfunc;                        /* Implementor of the command */
#define c_func  cfunc.c_func1
#define c_func2 cfunc.c_func2
	short	c_argtype;		/* Type of arglist (see below) */
	short	c_msgflag;		/* Required flags of messages */
	short	c_msgmask;		/* Relevant flags of messages */
};

/* Yechh, can't initialize unions */
#define	c_minargs c_msgflag		/* Minimum argcount for RAWLIST */
#define	c_maxargs c_msgmask		/* Max argcount for RAWLIST */

/*
 * Argument types.
 */
#define	MSGLIST	0x0001		/* Message list type */
#define	STRLIST	0x0002		/* A pure string */
#define	RAWLIST	0x0004		/* Shell string list */
#define	NOLIST	0x0008		/* Just plain 0 */
#define	NDMLIST	0x0010		/* Message list, no defaults */

#define	P	0x0020		/* Autoprint dot after command */
#define	I	0x0040		/* Interactive command bit */
#define	M	0x0080		/* Legal from send mode bit */
#define	W	0x0100		/* Illegal when read only bit */
#define	F	0x0200		/* Is a conditional command */
#define	T	0x0400		/* Is a transparent command */
#define	R	0x0800		/* Cannot be called from collect */

/*
 * Oft-used mask values
 */
#define	MMNORM		(MDELETED|MSAVED)/* Look at both save and delete bits */
#define	MMNDEL		MDELETED	/* Look only at deleted bit */

/*
 * Structure used to return a break down of a head
 * line (hats off to Bill Joy!)
 */
struct headline {
	char	*l_from;	/* The name of the sender */
	char	*l_tty;		/* His tty string (if any) */
	char	*l_date;	/* The entire date string */
};

#define	GTO	1		/* Grab To: line */
#define	GSUBJECT 2		/* Likewise, Subject: line */
#define	GCC	4		/* And the Cc: line */
#define	GBCC	8		/* And also the Bcc: line */
#define	GMASK	(GTO|GSUBJECT|GCC|GBCC)
				/* Mask of places from whence */

#define	GNL	16		/* Print blank line after */
#define	GDEL	32		/* Entity removed from list */
#define	GCOMMA	64		/* detract puts in commas */

/*
 * Structure used to pass about the current
 * state of the user-typed message header.
 */
struct header {
	struct name *h_to;		/* Dynamic "To:" string */
	char *h_subject;		/* Subject string */
	struct name *h_cc;		/* Carbon copies string */
	struct name *h_bcc;		/* Blind carbon copies */
	struct name *h_smopts;		/* Sendmail options */
};

/*
 * Structure of namelist nodes used in processing
 * the recipients of mail and aliases and all that
 * kind of stuff.
 */
struct name {
	struct	name *n_flink;		/* Forward link in list. */
	struct	name *n_blink;		/* Backward list link */
	short	n_type;			/* From which list it came */
	char	*n_name;		/* This fella's name */
};

/*
 * Structure of a variable node.  All variables are
 * kept on a singly-linked list of these, rooted by
 * "variables"
 */

struct var {
	struct	var *v_link;		/* Forward link to next variable */
	char	*v_name;		/* The variable's name */
	char	*v_value;		/* And it's current value */
};

struct group {
	struct	group *ge_link;		/* Next person in this group */
	char	*ge_name;		/* This person's user name */
};

struct grouphead {
	struct	grouphead *g_link;	/* Next grouphead in list */
	char	*g_name;		/* Name of this group */
	struct	group *g_list;		/* Users in group. */
};

/*
 * Structure of the hash table of ignored header fields
 */
struct ignoretab {
	int i_count;			/* Number of entries */
	struct ignore {
		struct ignore *i_link;	/* Next ignored field in bucket */
		char *i_field;		/* This ignored field */
	} *i_head[HSHSIZE];
};

/*
 * Token values returned by the scanner used for argument lists.
 * Also, sizes of scanner-related things.
 */
#define	TEOL	0			/* End of the command line */
#define	TNUMBER	1			/* A message number */
#define	TDASH	2			/* A simple dash */
#define	TSTRING	3			/* A string (possibly containing -) */
#define	TDOT	4			/* A "." */
#define	TUP	5			/* An "^" */
#define	TDOLLAR	6			/* A "$" */
#define	TSTAR	7			/* A "*" */
#define	TOPEN	8			/* An '(' */
#define	TCLOSE	9			/* A ')' */
#define TPLUS	10			/* A '+' */
#define TERROR	11			/* A lexical error */

#define	REGDEP	2			/* Maximum regret depth. */
#define	STRINGLEN	1024		/* Maximum length of string token */

/*
 * Constants for conditional commands.
 * These describe whether we should be executing stuff or not.
 */
#define	CANY		0		/* Execute in send or receive mode */
#define	CRCV		1		/* Execute in receive mode only */
#define	CSEND		2		/* Execute in send mode only */

/*
 * Truncate a file to the last character written. This is
 * useful just before closing an old file that was opened
 * for read/write.
 */
#define trunc(stream) do {						\
	(void)fflush(stream); 						\
	(void)ftruncate(fileno(stream), (off_t)ftell(stream));		\
} while(0)

#endif /* MAIL_DEF_H */
