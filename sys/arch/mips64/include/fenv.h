/*	$OpenBSD: src/sys/arch/mips64/include/fenv.h,v 1.1 2011/04/26 21:14:07 martynas Exp $	*/

/*
 * Copyright (c) 2011 Martynas Venckus <martynas@openbsd.org>
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

#ifndef	_MIPS64_FENV_H_
#define	_MIPS64_FENV_H_

/*
 * Each symbol representing a floating point exception expands to an integer
 * constant expression with values, such that bitwise-inclusive ORs of _all
 * combinations_ of the constants result in distinct values.
 *
 * We use such values that allow direct bitwise operations on FPU registers.
 */
#define	FE_INEXACT	0x04
#define	FE_UNDERFLOW	0x08
#define	FE_OVERFLOW	0x10
#define	FE_DIVBYZERO	0x20
#define	FE_INVALID	0x40

/*
 * The following symbol is simply the bitwise-inclusive OR of all floating-point
 * exception constants defined above.
 */
#define	FE_ALL_EXCEPT		\
	(FE_DIVBYZERO | FE_INEXACT | FE_INVALID | FE_OVERFLOW | FE_UNDERFLOW)

/*
 * Each symbol representing the rounding direction, expands to an integer
 * constant expression whose value is distinct non-negative value.
 *
 * We use such values that allow direct bitwise operations on FPU registers.
 */
#define	FE_TONEAREST		0x0
#define	FE_TOWARDZERO		0x1
#define	FE_UPWARD		0x2
#define	FE_DOWNWARD		0x3

/*
 * FPSCR encodes rounding modes by bits 0-1.
 * FPSCR flags and exception mask shifts by 5.
 */
#define	_ROUND_MASK		0x3
#define	_EMASK_SHIFT		5

/*
 * fenv_t represents the entire floating-point environment
 */
typedef unsigned int fenv_t;

extern fenv_t			__fe_dfl_env;
#define	FE_DFL_ENV		((const fenv_t *) &__fe_dfl_env)

/*
 * fexcept_t represents the floating-point status flags collectively, including
 * any status the implementation associates with the flags.
 *
 * A floating-point status flag is a system variable whose value is set (but
 * never cleared) when a floating-point exception is raised, which occurs as a
 * side effect of exceptional floating-point arithmetic to provide auxiliary
 * information.
 *
 * A floating-point control mode is a system variable whose value may be set by
 * the user to affect the subsequent behavior of floating-point arithmetic.
 */
typedef unsigned int fexcept_t;

#endif	/* ! _MIPS64_FENV_H_ */
