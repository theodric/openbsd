/*	$OpenBSD: src/lib/libc/arch/hppa64/gen/fpgetround.c,v 1.2 2014/04/18 15:09:52 guenther Exp $	*/

/*
 * Written by Miodrag Vallat.  Public domain
 */

#include <sys/types.h>
#include <ieeefp.h>

fp_rnd
fpgetround()
{
	u_int64_t fpsr;

	__asm__ volatile("fstd %%fr0,0(%1)" : "=m" (fpsr) : "r" (&fpsr));
	return ((fpsr >> 41) & 0x3);
}
