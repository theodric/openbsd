/*	$OpenBSD: src/usr.sbin/config/exec.c,v 1.2 2001/12/05 10:11:23 deraadt Exp $ */

/*
 * Copyright (c) 1999 Mats O Jansson.  All rights reserved.
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
 *	This product includes software developed by Mats O Jansson.
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

#include <err.h>
#include <sys/types.h>
#include <stdio.h>

#ifndef LINT
static char rcsid[] = "$OpenBSD: src/usr.sbin/config/exec.c,v 1.2 2001/12/05 10:11:23 deraadt Exp $";
#endif

#ifdef AOUT_SUPPORT
int	aout_check __P((char *));
void	aout_loadkernel __P((char *));
void	aout_savekernel __P((char *));
caddr_t	aout_adjust __P((caddr_t));
caddr_t	aout_readjust __P((caddr_t));
#endif

#ifdef ECOFF_SUPPORT
int	ecoff_check __P((char *));
void	ecoff_loadkernel __P((char *));
void	ecoff_savekernel __P((char *));
caddr_t	ecoff_adjust __P((caddr_t));
caddr_t	ecoff_readjust __P((caddr_t));
#endif

#ifdef ELF_SUPPORT
int	elf_check __P((char *));
void	elf_loadkernel __P((char *));
void	elf_savekernel __P((char *));
caddr_t	elf_adjust __P((caddr_t));
caddr_t	elf_readjust __P((caddr_t));
#endif

#define DO_AOUT	0
#define DO_ECOFF 1
#define	DO_ELF 2

int current_exec = -1;

caddr_t
adjust(x)
	caddr_t x;
{
	switch (current_exec) {
#ifdef AOUT_SUPPORT
	case DO_AOUT:
		return(aout_adjust(x));
		break;
#endif
#ifdef ECOFF_SUPPORT
	case DO_ECOFF:
		return(ecoff_adjust(x));
		break;
#endif
#ifdef ELF_SUPPORT
	case DO_ELF:
		return(elf_adjust(x));
		break;
#endif
	default:
		errx(1, "no supported exec type");
	}
}

caddr_t
readjust(x)
	caddr_t x;
{
	switch (current_exec) {
#ifdef AOUT_SUPPORT
	case DO_AOUT:
		return(aout_readjust(x));
		break;
#endif
#ifdef ECOFF_SUPPORT
	case DO_ECOFF:
		return(ecoff_readjust(x));
		break;
#endif
#ifdef ELF_SUPPORT
	case DO_ELF:
		return(elf_readjust(x));
		break;
#endif
	default:
		errx(1, "no supported exec type");
	}
}

void
loadkernel(file)
	char *file;
{
	current_exec = -1;

#ifdef AOUT_SUPPORT
	if (aout_check(file)) {
		current_exec = DO_AOUT;
	}
#endif

#ifdef ECOFF_SUPPORT
	if (ecoff_check(file)) {
		current_exec = DO_ECOFF;
	}
#endif

#ifdef ELF_SUPPORT
	if (elf_check(file)) {
		current_exec = DO_ELF;
	}
#endif

	switch (current_exec) {
#ifdef AOUT_SUPPORT
	case DO_AOUT:
		aout_loadkernel(file);
		break;
#endif
#ifdef ECOFF_SUPPORT
	case DO_ECOFF:
		ecoff_loadkernel(file);
		break;
#endif
#ifdef ELF_SUPPORT
	case DO_ELF:
		elf_loadkernel(file);
		break;
#endif
	default:
		errx(1, "no supported exec type");
	}
}

void
savekernel(outfile)
	char *outfile;
{
	switch (current_exec) {
#ifdef AOUT_SUPPORT
	case DO_AOUT:
		aout_savekernel(outfile);
		break;
#endif
#ifdef ECOFF_SUPPORT
	case DO_ECOFF:
		ecoff_savekernel(outfile);
		break;
#endif
#ifdef ELF_SUPPORT
	case DO_ELF:
		elf_savekernel(outfile);
		break;
#endif
	default:
		errx(1, "no supported exec type");
	}
}
