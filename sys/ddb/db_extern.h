/*	$OpenBSD: src/sys/ddb/db_extern.h,v 1.10 2001/02/10 10:42:35 niklas Exp $	*/
/*	$NetBSD: db_extern.h,v 1.1 1996/02/05 01:57:00 christos Exp $	*/

/*
 * Copyright (c) 1995 Christos Zoulas.  All rights reserved.
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
 *	This product includes software developed by Christos Zoulas.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef _DDB_DB_EXTERN_H_
#define _DDB_DB_EXTERN_H_

/* db_sym.c */
void ddb_init __P((void));

/* db_examine.c */
void db_examine_cmd __P((db_expr_t, int, db_expr_t, char *));
void db_examine __P((db_addr_t, char *, int));
void db_print_cmd __P((db_expr_t, int, db_expr_t, char *));
void db_print_loc_and_inst __P((db_addr_t));
void db_strcpy __P((char *, char *));
void db_search_cmd __P((db_expr_t, boolean_t, db_expr_t, char *));
void db_search __P((db_addr_t, int, db_expr_t, db_expr_t, db_expr_t));

/* db_expr.c */
boolean_t db_term __P((db_expr_t *));
boolean_t db_unary __P((db_expr_t *));
boolean_t db_mult_expr __P((db_expr_t *));
boolean_t db_add_expr __P((db_expr_t *));
boolean_t db_shift_expr __P((db_expr_t *));
int db_expression __P((db_expr_t *));

/* db_hangman.c */
void db_hangman __P((db_expr_t, int, db_expr_t, char *));

/* db_input.c */
void db_putstring __P((char *, int));
void db_putnchars __P((int, int));
void db_delete __P((int, int));
void db_delete_line __P((void));
int db_inputchar __P((int));
int db_readline __P((char *, int));
void db_check_interrupt __P((void));

/* db_print.c */
void db_show_regs __P((db_expr_t, boolean_t, db_expr_t, char *));

/* db_trap.c */
void db_trap(int, int);

/* db_write_cmd.c */
void db_write_cmd __P((db_expr_t, boolean_t, db_expr_t, char *));

#endif /* _DDB_DB_EXTERN_H_ */
