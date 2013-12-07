/*	$OpenBSD: src/sys/arch/amd64/pci/agp_machdep.c,v 1.8 2013/12/07 10:57:06 kettenis Exp $	*/

/*
 * Copyright (c) 2008 - 2009 Owain G. Ainsworth <oga@openbsd.org>
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
/*
 * Copyright (c) 2002 Michael Shalayeff
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR OR HIS RELATIVES BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF MIND, USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/device.h>
#include <sys/malloc.h>

#include <dev/pci/pcivar.h>
#include <dev/pci/pcireg.h>
#include <dev/pci/pcidevs.h>
#include <dev/pci/agpvar.h>

#include <machine/cpufunc.h>
#include <machine/bus.h>

#include <uvm/uvm.h>

#include "intagp.h"

/* bus_dma functions */

#if NINTAGP > 0
void	intagp_dma_sync(bus_dma_tag_t, bus_dmamap_t, bus_addr_t,
	    bus_size_t, int);
#endif

void
agp_flush_cache(void)
{
	wbinvd();
}

void
agp_flush_cache_range(vaddr_t va, vsize_t sz)
{
	pmap_flush_cache(va, sz);
}

struct agp_map {
	bus_space_tag_t		bst;
	bus_space_handle_t	bsh;
	bus_size_t		size;
};

int
agp_init_map(bus_space_tag_t tag, bus_addr_t address, bus_size_t size,
    int flags, struct agp_map **mapp)
{
	struct agp_map	*map;
	int		 err;

	map = malloc(sizeof(*map), M_AGP, M_WAITOK | M_CANFAIL);
	if (map == NULL)
		return (ENOMEM);

	map->bst = tag;
	map->size = size;
	
	if ((err = bus_space_map(tag, address, size, flags, &map->bsh)) != 0) {
		free(map, M_AGP);
		return (err);
	}
	*mapp = map;
	return (0);
}

void
agp_destroy_map(struct agp_map *map)
{
	bus_space_unmap(map->bst, map->bsh, map->size);
	free(map, M_AGP);
}


int
agp_map_subregion(struct agp_map *map, bus_size_t offset, bus_size_t size,
    bus_space_handle_t *bshp)
{
	if (offset > map->size || size > map->size || offset + size > map->size)
		return (EINVAL);
	return (bus_space_subregion(map->bst, map->bsh, offset, size, bshp));
	
}

void
agp_unmap_subregion(struct agp_map *map, bus_space_handle_t bsh,
    bus_size_t size)
{
	/* subregion doesn't need unmapping, do nothing */
}

/*
 * ick ick ick. However, the rest of this driver is supposedly MI (though
 * they only exist on x86), so this can't be in dev/pci.
 */

#if NINTAGP > 0

/*
 * bus_dmamap_sync routine for intagp.
 *
 * This is tailored to the usage that drm with the GEM memory manager
 * will be using, since intagp is for intel IGD, and thus shouldn't be
 * used for anything other than gpu-based work. Essentially for the intel GEM
 * driver we use bus_dma as an abstraction to convert our memory into a gtt
 * address and deal with any cache incoherencies that we create.
 *
 * We use the cflush instruction to deal with clearing the caches, since our
 * cache is physically indexed, we can even map then clear the page and it'll
 * work. on i386 we need to check for the presence of cflush() in cpuid,
 * however, all cpus that have a new enough intel GMCH should be suitable.
 */
void	
intagp_dma_sync(bus_dma_tag_t tag, bus_dmamap_t dmam,
    bus_addr_t offset, bus_size_t size, int ops)
{
	bus_dma_segment_t	*segp;
	struct sg_page_map	*spm;
	void			*addr;
	paddr_t	 		 pa;
	bus_addr_t		 poff, endoff, soff;

#ifdef DIAGNOSTIC
	if ((ops & (BUS_DMASYNC_PREREAD | BUS_DMASYNC_PREWRITE)) != 0 &&
	    (ops & (BUS_DMASYNC_POSTREAD | BUS_DMASYNC_POSTWRITE)) != 0)
		panic("agp_dmamap_sync: mix PRE and POST");
	if (offset >= dmam->dm_mapsize)
		panic("_intagp_dma_sync: bad offset %lu (size = %lu)",
		    offset, dmam->dm_mapsize);
	if (size == 0 || (offset + size) > dmam->dm_mapsize)
		panic("intagp_dma_sync: bad length");
#endif /* DIAGNOSTIC */
	
	/* Coherent mappings need no sync. */
	if (dmam->_dm_flags & BUS_DMA_COHERENT)
		return;

	/*
	 * We need to clflush the object cache in all cases but postwrite.
	 *
	 * - Due to gpu incoherency, postread we need to flush speculative
	 * reads (which are not written back on intel cpus).
	 *
	 * - preread we need to flush data which will very soon be stale from
	 * the caches
	 *
	 * - prewrite we need to make sure our data hits the memory before the 
	 * gpu hoovers it up.
	 *
	 * The chipset also may need flushing, but that fits badly into
	 * bus_dma and it done in the driver.
	 */
	soff = trunc_page(offset);
	endoff = round_page(offset + size);
	if (ops & BUS_DMASYNC_POSTREAD || ops & BUS_DMASYNC_PREREAD ||
	    ops & BUS_DMASYNC_PREWRITE) {
		if (curcpu()->ci_cflushsz == 0) {
			/* save some wbinvd()s. we're MD anyway so it's ok */
			wbinvd();
			return;
		}

		mfence();
		spm = dmam->_dm_cookie;
		switch (spm->spm_buftype) {
		case BUS_BUFTYPE_LINEAR:
			addr = spm->spm_origbuf + soff;
			while (soff < endoff) {
				pmap_flush_cache((vaddr_t)addr, PAGE_SIZE);
				soff += PAGE_SIZE;
				addr += PAGE_SIZE;
			} break;
		case BUS_BUFTYPE_RAW:
			segp = (bus_dma_segment_t *)spm->spm_origbuf;
			poff = 0;

			while (poff < soff) {
				if (poff + segp->ds_len > soff)
					break;
				poff += segp->ds_len;
				segp++;
			}
			/* first time round may not start at seg beginning */
			pa = segp->ds_addr + (soff - poff);
			while (poff < endoff) {
				for (; pa < segp->ds_addr + segp->ds_len &&
				    poff < endoff; pa += PAGE_SIZE) {
					pmap_flush_page(pa);
					poff += PAGE_SIZE;
				}
				segp++;
				if (poff < endoff)
					pa = segp->ds_addr;
			}
			break;
		/* You do not want to load mbufs or uios onto a graphics card */
		case BUS_BUFTYPE_MBUF:
			/* FALLTHROUGH */
		case BUS_BUFTYPE_UIO:
			/* FALLTHROUGH */
		default:
			panic("intagp_dmamap_sync: bad buftype %d",
			    spm->spm_buftype);
			
		}
		mfence();
	}
}
#endif /* NINTAGP > 0 */
