/*	$OpenBSD: src/sys/dev/isa/boca.c,v 1.8 1996/04/21 22:22:52 deraadt Exp $ */
/*	$NetBSD: boca.c,v 1.13 1996/04/15 18:55:28 cgd Exp $	*/

/*
 * Copyright (c) 1996 Christopher G. Demetriou.  All rights reserved.
 * Copyright (c) 1995 Charles Hannum.  All rights reserved.
 *
 * This code is derived from public-domain software written by
 * Roland McGrath, and information provided by David Muir Sharnoff.
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
 *	This product includes software developed by Charles Hannum.
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

#include <sys/param.h>
#include <sys/device.h>
#include <sys/termios.h>

#ifdef i386							/* XXX */
#include <machine/cpu.h>					/* XXX */
#else								/* XXX */
#include <machine/intr.h>
#endif								/* XXX */
#include <machine/bus.h>

#include <dev/isa/isavar.h>
#include <dev/isa/comreg.h>
#include <dev/isa/comvar.h>

#define	NSLAVES	8

struct boca_softc {
	struct device sc_dev;
	void *sc_ih;

	bus_chipset_tag_t sc_bc;
	int sc_iobase;

	int sc_alive;			/* mask of slave units attached */
	void *sc_slaves[NSLAVES];	/* com device unit numbers */
	bus_io_handle_t sc_slaveioh[NSLAVES];
};

int bocaprobe();
void bocaattach();
int bocaintr __P((void *));

struct cfattach boca_ca = {
	sizeof(struct boca_softc), bocaprobe, bocaattach,
};

struct cfdriver boca_cd = {
	NULL, "boca", DV_TTY
};

int
bocaprobe(parent, self, aux)
	struct device *parent, *self;
	void *aux;
{
	struct isa_attach_args *ia = aux;
	int iobase = ia->ia_iobase;
	bus_chipset_tag_t bc = ia->ia_bc;
	bus_io_handle_t ioh;
	int i, rv = 1;

	/*
	 * Do the normal com probe for the first UART and assume
	 * its presence, and the ability to map the other UARTS,
	 * means there is a multiport board there.
	 * XXX Needs more robustness.
	 */

	/* if the first port is in use as console, then it. */
	if (iobase == comconsaddr && !comconsattached)
		goto checkmappings;

	if (bus_io_map(bc, iobase, COM_NPORTS, &ioh)) {
		rv = 0;
		goto out;
	}
	rv = comprobe1(bc, ioh, iobase);
	bus_io_unmap(bc, ioh, COM_NPORTS);
	if (rv == 0)
		goto out;

checkmappings:
	for (i = 1; i < NSLAVES; i++) {
		iobase += COM_NPORTS;

		if (iobase == comconsaddr && !comconsattached)
			continue;

		if (bus_io_map(bc, iobase, COM_NPORTS, &ioh)) {
			rv = 0;
			goto out;
		}
		bus_io_unmap(bc, ioh, COM_NPORTS);
	}

out:
	if (rv)
		ia->ia_iosize = NSLAVES * COM_NPORTS;
	return (rv);
}

int
bocaprint(aux, pnp)
	void *aux;
	char *pnp;
{
	struct commulti_attach_args *ca = aux;

	if (pnp)
		printf("com at %s", pnp);
	printf(" slave %d", ca->ca_slave);
	return (UNCONF);
}

void
bocaattach(parent, self, aux)
	struct device *parent, *self;
	void *aux;
{
	struct boca_softc *sc = (void *)self;
	struct isa_attach_args *ia = aux;
	struct commulti_attach_args ca;
	int i, subunit;

	sc->sc_bc = ia->ia_bc;
	sc->sc_iobase = ia->ia_iobase;

	for (i = 0; i < NSLAVES; i++)
		if (bus_io_map(bc, sc->sc_iobase + i * COM_NPORTS, COM_NPORTS,
		    &sc->sc_slaveioh[i]))
			panic("bocaattach: couldn't map slave %d", i);

	printf("\n");

	for (i = 0; i < NSLAVES; i++) {
		struct cfdata *match;

		ca.ca_slave = i;
		ca.ca_bc = sc->sc_bc;
		ca.ca_ioh = sc->sc_slaveioh[i];
		ca.ca_iobase = sc->sc_iobase + i * COM_NPORTS;
		ca.ca_noien = 0;

		sc->sc_slaves[i] = config_found(self, &ca, bocaprint);
		if (sc->sc_slaves[i] != NULL)
			sc->sc_alive |= 1 << i;
	}

	sc->sc_ih = isa_intr_establish(ia->ia_ic, ia->ia_irq, IST_EDGE,
	    IPL_TTY, bocaintr, sc, sc->sc_dev.dv_xname);
}

int
bocaintr(arg)
	void *arg;
{
	struct boca_softc *sc = arg;
	bus_chipset_tag_t bc = sc->sc_bc;
	int alive = sc->sc_alive;
	int bits;

	bits = bus_io_read_1(bc, sc->sc_slaveioh[0], 7) & alive;
	if (bits == 0)
		return (0);

	for (;;) {
#define	TRY(n) \
		if (bits & (1 << (n))) \
			comintr(sc->sc_slaves[n]);
		TRY(0);
		TRY(1);
		TRY(2);
		TRY(3);
		TRY(4);
		TRY(5);
		TRY(6);
		TRY(7);
#undef TRY
		bits = bus_io_read_1(bc, sc->sc_slaveioh[0], 7) & alive;
		if (bits == 0)
			return (1);
 	}
}
