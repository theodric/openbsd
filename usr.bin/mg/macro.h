/*	$OpenBSD: src/usr.bin/mg/macro.h,v 1.3 2001/01/29 01:58:08 niklas Exp $	*/

/* definitions for keyboard macros */

#ifndef EXTERN
#define EXTERN extern
#define INIT(i)
#endif

#define MAXMACRO 256		/* maximum functs in a macro */

EXTERN int inmacro INIT(FALSE);
EXTERN int macrodef INIT(FALSE);
EXTERN int macrocount INIT(0);

EXTERN union {
	PF	m_funct;
	int	m_count;	/* for count-prefix	 */
} macro[MAXMACRO];

EXTERN LINE    *maclhead INIT(NULL);
EXTERN LINE    *maclcur;

#undef	EXTERN
#undef	INIT
