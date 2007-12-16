/*	$OpenBSD: src/sys/dev/usb/if_upgt.c,v 1.2 2007/12/16 18:19:57 mglocker Exp $ */

/*
 * Copyright (c) 2007 Marcus Glocker <mglocker@openbsd.org>
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

#include "bpfilter.h"

#include <sys/param.h>
#include <sys/sockio.h>
#include <sys/sysctl.h>
#include <sys/mbuf.h>
#include <sys/kernel.h>
#include <sys/socket.h>
#include <sys/systm.h>
#include <sys/timeout.h>
#include <sys/conf.h>
#include <sys/device.h>

#include <machine/bus.h>
#include <machine/endian.h>
#include <machine/intr.h>

#if NBPFILTER > 0
#include <net/bpf.h>
#endif
#include <net/if.h>
#include <net/if_arp.h>
#include <net/if_dl.h>
#include <net/if_media.h>
#include <net/if_types.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>

#include <net80211/ieee80211_var.h>
#include <net80211/ieee80211_amrr.h>
#include <net80211/ieee80211_radiotap.h>

#include <dev/usb/usb.h>
#include <dev/usb/usbdi.h>
#include <dev/usb/usbdi_util.h>
#include <dev/usb/usbdevs.h>

#include <dev/usb/if_upgtvar.h>

/*
 * Driver for the USB PrismGT devices.
 *
 * For now just USB 2.0 devices with the GW3887 chipset are supported.
 *
 * Parts of this driver has been influenced by reading the p54u driver
 * written by Jean-Baptiste Note <jean-baptiste.note@m4x.org> and
 * Sebastien Bourdeauducq <lekernel@prism54.org>.
 */

#ifdef UPGT_DEBUG
int upgt_debug = 2;
#define DPRINTF(l, x...) do { if ((l) <= upgt_debug) printf(x); } while (0)
#else
#define DPRINTF(l, x...)
#endif

/*
 * Prototypes.
 */
int		upgt_match(struct device *, void *, void *);
void		upgt_attach(struct device *, struct device *, void *);
void		upgt_attachhook(void *);
int		upgt_detach(struct device *, int);
int		upgt_activate(struct device *, enum devact);

int		upgt_device_init(struct upgt_softc *);
int		upgt_mem_init(struct upgt_softc *);
uint32_t	upgt_mem_alloc(struct upgt_softc *);
void		upgt_mem_free(struct upgt_softc *, uint32_t);
int		upgt_fw_alloc(struct upgt_softc *);
void		upgt_fw_free(struct upgt_softc *);
int		upgt_fw_verify(struct upgt_softc *);
int		upgt_fw_load(struct upgt_softc *);
int		upgt_fw_copy(char *, char *, int);
int		upgt_eeprom_read(struct upgt_softc *);
int		upgt_eeprom_parse(struct upgt_softc *);
void		upgt_eeprom_parse_hwrx(struct upgt_softc *, uint8_t *);
void		upgt_eeprom_parse_freq3(struct upgt_softc *, uint8_t *, int);
void		upgt_eeprom_parse_freq4(struct upgt_softc *, uint8_t *, int);
void		upgt_eeprom_parse_freq6(struct upgt_softc *, uint8_t *, int);

int		upgt_ioctl(struct ifnet *, u_long, caddr_t);
int		upgt_init(struct ifnet *);
void		upgt_stop(struct upgt_softc *);
int		upgt_media_change(struct ifnet *);
void		upgt_newassoc(struct ieee80211com *, struct ieee80211_node *,
		    int);
int		upgt_newstate(struct ieee80211com *, enum ieee80211_state, int);
void		upgt_newstate_task(void *);
void		upgt_next_scan(void *);
void		upgt_start(struct ifnet *);
void		upgt_watchdog(struct ifnet *);
void		upgt_tx_task(void *);
void		upgt_tx_done(struct upgt_softc *, uint8_t *);
void		upgt_rx_cb(usbd_xfer_handle, usbd_private_handle, usbd_status);
void		upgt_rx(struct upgt_softc *, uint8_t *, int);
int		upgt_set_macfilter(struct upgt_softc *, uint8_t state);
int		upgt_set_channel(struct upgt_softc *, unsigned);

int		upgt_alloc_tx(struct upgt_softc *);
int		upgt_alloc_rx(struct upgt_softc *);
int		upgt_alloc_cmd(struct upgt_softc *);
void		upgt_free_tx(struct upgt_softc *);
void		upgt_free_rx(struct upgt_softc *);
void		upgt_free_cmd(struct upgt_softc *);
int		upgt_bulk_xmit(struct upgt_softc *, struct upgt_data *,
		    usbd_pipe_handle, uint32_t *, int);

void		upgt_hexdump(void *, int);
uint32_t	upgt_crc32(const void *, size_t);

struct cfdriver upgt_cd = {
	NULL, "upgt", DV_IFNET
};

const struct cfattach upgt_ca = {
	sizeof(struct upgt_softc),
	upgt_match,
	upgt_attach,
	upgt_detach,
	upgt_activate,
};

static const struct usb_devno upgt_devs[] = {
	{ USB_VENDOR_SMC,	USB_PRODUCT_SMC_2862WG }
};

int
upgt_match(struct device *parent, void *match, void *aux)
{
	struct usb_attach_arg *uaa = aux;

	if (uaa->iface != NULL)
		return (UMATCH_NONE);

	return ((usb_lookup(upgt_devs, uaa->vendor, uaa->product) != NULL) ?
	    UMATCH_VENDOR_PRODUCT : UMATCH_NONE);
}

void
upgt_attach(struct device *parent, struct device *self, void *aux)
{
	struct upgt_softc *sc = (struct upgt_softc *)self;
	struct usb_attach_arg *uaa = aux;
	usb_interface_descriptor_t *id;
	usb_endpoint_descriptor_t *ed;
	usbd_status error;
	int i;

	/*
	 * Attach USB device.
	 */
	sc->sc_udev = uaa->device;

	/* set configuration number */
	if (usbd_set_config_no(sc->sc_udev, UPGT_CONFIG_NO, 0) != 0) {
		printf("%s: could not set configuration no!\n",
		    sc->sc_dev.dv_xname);
		return;
	}

	/* get the first interface handle */
	error = usbd_device2interface_handle(sc->sc_udev, UPGT_IFACE_INDEX,
	    &sc->sc_iface);
	if (error != 0) {
		printf("%s: could not get interface handle!\n",
		    sc->sc_dev.dv_xname);
		return;
	}

	/* find endpoints */
	id = usbd_get_interface_descriptor(sc->sc_iface);
	sc->sc_rx_no = sc->sc_tx_no = -1;
	for (i = 0; i < id->bNumEndpoints; i++) {
		ed = usbd_interface2endpoint_descriptor(sc->sc_iface, i);
		if (ed == NULL) {
			printf("%s: no endpoint descriptor for iface %d!\n",
			    sc->sc_dev.dv_xname, i);
			return;
		}

		if (UE_GET_DIR(ed->bEndpointAddress) == UE_DIR_OUT &&
		    UE_GET_XFERTYPE(ed->bmAttributes) == UE_BULK)
			sc->sc_tx_no = ed->bEndpointAddress;
		if (UE_GET_DIR(ed->bEndpointAddress) == UE_DIR_IN &&
		    UE_GET_XFERTYPE(ed->bmAttributes) == UE_BULK)
			sc->sc_rx_no = ed->bEndpointAddress;

		/*
		 * XXX
		 * Just get the version 2 bulk pipes for now.
		 * 0x01 TX pipe
		 * 0x81 RX pipe
		 * 0x02 TX MGMT pipe (not used with fw version >2.5.x)
		 * 0x82 TX MGMT pipe (not used with fw version >2.5.x)
		 */
		if (sc->sc_tx_no != -1 && sc->sc_rx_no != -1)
			break;
	}
	if (sc->sc_rx_no == -1 || sc->sc_tx_no == -1) {
		printf("%s: missing endpoint!\n", sc->sc_dev.dv_xname);
		return;
	}

	/* init tasks */
	usb_init_task(&sc->sc_task_newstate, upgt_newstate_task, sc);
	usb_init_task(&sc->sc_task_tx, upgt_tx_task, sc);
	timeout_set(&sc->scan_to, upgt_next_scan, sc);

	/*
	 * Open TX and RX USB bulk pipes.
	 */
	error = usbd_open_pipe(sc->sc_iface, sc->sc_tx_no, USBD_EXCLUSIVE_USE,
	    &sc->sc_tx_pipeh);
	if (error != 0) {
		printf("%s: could not open TX pipe: %s!\n",
		    sc->sc_dev.dv_xname, usbd_errstr(error));
		goto fail;
	}
	error = usbd_open_pipe(sc->sc_iface, sc->sc_rx_no, USBD_EXCLUSIVE_USE,
	    &sc->sc_rx_pipeh);
	if (error != 0) {
		printf("%s: could not open RX pipe: %s!\n",
		    sc->sc_dev.dv_xname, usbd_errstr(error));
		goto fail;
	}

	/*
	 * Allocate TX, RX, and CMD xfers.
	 */
	if (upgt_alloc_tx(sc) != 0)
		goto fail;
	if (upgt_alloc_rx(sc) != 0)
		goto fail;
	if (upgt_alloc_cmd(sc) != 0)
		goto fail;

	/*
	 * We need the firmware loaded to complete the attach.
	 */
	if (rootvp == NULL)
		mountroothook_establish(upgt_attachhook, sc);
	else
		upgt_attachhook(sc);

	return;
fail:
	printf("%s: %s failed!\n", sc->sc_dev.dv_xname, __func__);
}

void
upgt_attachhook(void *arg)
{
	struct upgt_softc *sc = arg;
	struct ieee80211com *ic = &sc->sc_ic;
	struct ifnet *ifp = &ic->ic_if;
	usbd_status error;
	int i;

	/*
	 * Load firmware file into memory.
	 */
	if (upgt_fw_alloc(sc) != 0)
		goto fail;

	/*
	 * Initialize the device.
	 */
	if (upgt_device_init(sc) != 0)
		goto fail;

	/*
	 * Verify the firmware.
	 */
	if (upgt_fw_verify(sc) != 0)
		goto fail;

	/*
	 * Calculate device memory space.
	 */
	if (sc->sc_memaddr_frame_start == 0 || sc->sc_memaddr_frame_end == 0) {
		printf("%s: could not find memory space addresses on FW!\n",
		    sc->sc_dev.dv_xname);
		goto fail;
	}
	sc->sc_memaddr_frame_end -= UPGT_MEMSIZE_RX + 1;
	sc->sc_memaddr_rx_start = sc->sc_memaddr_frame_end + 1;

	DPRINTF(1, "%s: memory address frame start=0x%08x\n",
	    sc->sc_dev.dv_xname, sc->sc_memaddr_frame_start);
	DPRINTF(1, "%s: memory address frame end=0x%08x\n",
	    sc->sc_dev.dv_xname, sc->sc_memaddr_frame_end);
	DPRINTF(1, "%s: memory address rx start=0x%08x\n",
	    sc->sc_dev.dv_xname, sc->sc_memaddr_rx_start);

	upgt_mem_init(sc);

	/*
	 * Load the firmware.
	 */
	if (upgt_fw_load(sc) != 0)
		goto fail;

	/*
	 * Startup the RX pipe.
	 */
	struct upgt_data *data_rx = &sc->rx_data;

	usbd_setup_xfer(data_rx->xfer, sc->sc_rx_pipeh, data_rx, data_rx->buf,
	    MCLBYTES, USBD_SHORT_XFER_OK, USBD_NO_TIMEOUT, upgt_rx_cb);
	error = usbd_transfer(data_rx->xfer);
	if (error != 0 && error != USBD_IN_PROGRESS) {
		printf("%s: could not queue RX transfer!\n",
		    sc->sc_dev.dv_xname);
		goto fail;
	}

	/*
	 * Read the whole EEPROM content and parse it.
	 */
	if (upgt_eeprom_read(sc) != 0)
		goto fail;
	if (upgt_eeprom_parse(sc) != 0)
		goto fail;

	/*
	 * Setup the 802.11 device.
	 */
	ic->ic_phytype = IEEE80211_T_OFDM;
	ic->ic_opmode = IEEE80211_M_STA;
	ic->ic_state = IEEE80211_S_INIT;
	ic->ic_caps =
	    IEEE80211_C_MONITOR |
	    IEEE80211_C_SHPREAMBLE |
	    IEEE80211_C_SHSLOT |
	    IEEE80211_C_WEP;

	ic->ic_sup_rates[IEEE80211_MODE_11B] = ieee80211_std_rateset_11b;
	ic->ic_sup_rates[IEEE80211_MODE_11G] = ieee80211_std_rateset_11g;

	for (i = 1; i <= 14; i++) {
		ic->ic_channels[i].ic_freq =
		    ieee80211_ieee2mhz(i, IEEE80211_CHAN_2GHZ);
		ic->ic_channels[i].ic_flags =
		    IEEE80211_CHAN_CCK | IEEE80211_CHAN_OFDM |
		    IEEE80211_CHAN_DYN | IEEE80211_CHAN_2GHZ;
	}

	ifp->if_softc = sc;
	ifp->if_flags = IFF_BROADCAST | IFF_SIMPLEX | IFF_MULTICAST;
	ifp->if_init = upgt_init;
	ifp->if_ioctl = upgt_ioctl;
	ifp->if_start = upgt_start;
	ifp->if_watchdog = upgt_watchdog;
	IFQ_SET_READY(&ifp->if_snd);
	memcpy(ifp->if_xname, sc->sc_dev.dv_xname, IFNAMSIZ);

	if_attach(ifp);
	ieee80211_ifattach(ifp);
	ic->ic_newassoc = upgt_newassoc;

	sc->sc_newstate = ic->ic_newstate;
	ic->ic_newstate = upgt_newstate;
	ieee80211_media_init(ifp, upgt_media_change, ieee80211_media_status);

#if NBPFILTER > 0
	bpfattach(&sc->sc_drvbpf, ifp, DLT_IEEE802_11_RADIO,
	    sizeof(struct ieee80211_frame) + IEEE80211_RADIOTAP_HDRLEN);

	sc->sc_rxtap_len = sizeof(sc->sc_rxtapu);
	sc->sc_rxtap.wr_ihdr.it_len = htole16(sc->sc_rxtap_len);
	sc->sc_rxtap.wr_ihdr.it_present = htole32(UPGT_RX_RADIOTAP_PRESENT);

	sc->sc_txtap_len = sizeof(sc->sc_txtapu);
	sc->sc_txtap.wt_ihdr.it_len = htole16(sc->sc_txtap_len);
	sc->sc_txtap.wt_ihdr.it_present = htole32(UPGT_TX_RADIOTAP_PRESENT);
#endif

	usbd_add_drv_event(USB_EVENT_DRIVER_ATTACH, sc->sc_udev, &sc->sc_dev);

	printf("%s: address %s\n",
	    sc->sc_dev.dv_xname, ether_sprintf(ic->ic_myaddr));

	/* device attached */
	sc->sc_flags |= UPGT_DEVICE_ATTACHED;

	return;
fail:
	printf("%s: %s failed!\n", sc->sc_dev.dv_xname, __func__);
}

int
upgt_detach(struct device *self, int flags)
{
	struct upgt_softc *sc = (struct upgt_softc *)self;
	struct ifnet *ifp = &sc->sc_ic.ic_if;
	int s;

	if (!(sc->sc_flags & UPGT_DEVICE_ATTACHED))
		/* device was not properly attached */
		return (0);

	s = splusb();

	/* remove tasks and timeouts */
	usb_rem_task(sc->sc_udev, &sc->sc_task_newstate);
	usb_rem_task(sc->sc_udev, &sc->sc_task_tx);
	timeout_del(&sc->scan_to);

	/* abort and close TX / RX pipes */
	if (sc->sc_tx_pipeh != NULL) {
		usbd_abort_pipe(sc->sc_tx_pipeh);
		usbd_close_pipe(sc->sc_tx_pipeh);
	}
	if (sc->sc_rx_pipeh != NULL) {
		usbd_abort_pipe(sc->sc_rx_pipeh);
		usbd_close_pipe(sc->sc_rx_pipeh);
	}

	/* free xfers */
	upgt_free_tx(sc);
	upgt_free_rx(sc);
	upgt_free_cmd(sc);

	/* free firmware */
	upgt_fw_free(sc);

	/* detach interface */
	ieee80211_ifdetach(ifp);
	if_detach(ifp);

	splx(s);

	usbd_add_drv_event(USB_EVENT_DRIVER_DETACH, sc->sc_udev, &sc->sc_dev);

	return (0);
}

int
upgt_activate(struct device *self, enum devact act)
{
	switch (act) {
	case DVACT_ACTIVATE:
		return (EOPNOTSUPP);
	case (DVACT_DEACTIVATE):
		break;
	}

	return (0);
}

int
upgt_device_init(struct upgt_softc *sc)
{
	struct upgt_data *data_cmd = &sc->cmd_data;
	char init_cmd[] = { 0x7e, 0x7e, 0x7e, 0x7e };
	int len;

	len = sizeof(init_cmd);
	bcopy(init_cmd, data_cmd->buf, len);
	if (upgt_bulk_xmit(sc, data_cmd, sc->sc_tx_pipeh, &len, 0) != 0) {
		printf("%s: could not send device init string!\n",
		    sc->sc_dev.dv_xname);
		return (EIO);
	}
	usbd_delay_ms(sc->sc_udev, 100);

	DPRINTF(1, "%s: device initialized\n", sc->sc_dev.dv_xname);

	return (0);
}

int
upgt_mem_init(struct upgt_softc *sc)
{
	int i;

	for (i = 0; i < UPGT_MEMORY_MAX_PAGES; i++) {
		sc->sc_memory.page[i].used = 0;

		if (i == 0) {
			/*
			 * The first memory page is always reserved for
			 * command data.
			 */
			sc->sc_memory.page[i].addr =
			    sc->sc_memaddr_frame_start + MCLBYTES;
		} else {
			sc->sc_memory.page[i].addr =
			    sc->sc_memory.page[i - 1].addr + MCLBYTES;
		}

		if (sc->sc_memory.page[i].addr + MCLBYTES >=
		    sc->sc_memaddr_frame_end)
			break;

		DPRINTF(2, "%s: memory address page %d=0x%08x\n",
		    sc->sc_dev.dv_xname, i, sc->sc_memory.page[i].addr);
	}

	sc->sc_memory.pages = i;

	DPRINTF(2, "%s: memory pages=%d\n",
	    sc->sc_dev.dv_xname, sc->sc_memory.pages);

	return (0);
}

uint32_t
upgt_mem_alloc(struct upgt_softc *sc)
{
	int i;

	for (i = 0; i < sc->sc_memory.pages; i++) {
		if (sc->sc_memory.page[i].used == 0) {
			sc->sc_memory.page[i].used = 1;
			return (sc->sc_memory.page[i].addr);
		}
	}

	return (0);
}

void
upgt_mem_free(struct upgt_softc *sc, uint32_t addr)
{
	int i;

	for (i = 0; i < sc->sc_memory.pages; i++) {
		if (sc->sc_memory.page[i].addr == addr) {
			sc->sc_memory.page[i].used = 0;
			return;
		}
	}

	printf("%s: could not free memory address 0x%08x!\n",
	    sc->sc_dev.dv_xname, addr);
}


int
upgt_fw_alloc(struct upgt_softc *sc)
{
	const char *name = "upgt-gw3887";
	int error;

	if (sc->sc_fw == NULL) {
		error = loadfirmware(name, &sc->sc_fw, &sc->sc_fw_size);
		if (error != 0) {
			printf("%s: error %d, could not read firmware %s!\n",
			    sc->sc_dev.dv_xname, error, name);
			return (EIO);
		}
	}

	DPRINTF(1, "%s: firmware %s allocated\n", sc->sc_dev.dv_xname, name);

	return (0);
}

void
upgt_fw_free(struct upgt_softc *sc)
{
	if (sc->sc_fw != NULL) {
		free(sc->sc_fw, M_DEVBUF);
		sc->sc_fw = NULL;
		DPRINTF(1, "%s: firmware freed\n", sc->sc_dev.dv_xname);
	}
}

int
upgt_fw_verify(struct upgt_softc *sc)
{
	struct upgt_fw_bra_option *bra_option;
	uint32_t bra_option_type, bra_option_len;
	uint32_t *uc;
	int offset, bra_end = 0;

	/*
	 * Seek to beginning of Boot Record Area (BRA).
	 */
	for (offset = 0; offset < sc->sc_fw_size; offset += sizeof(uc)) {
		uc = (uint32_t *)(sc->sc_fw + offset);
		if (*uc == 0)
			break;
	}
	for (; offset < sc->sc_fw_size; offset += sizeof(uc)) {
		uc = (uint32_t *)(sc->sc_fw + offset);
		if (*uc != 0)
			break;
	}
	if (offset == sc->sc_fw_size) { 
		printf("%s: firmware Boot Record Area not found!\n",
		    sc->sc_dev.dv_xname);
		return (EIO);
	}
	DPRINTF(1, "%s: firmware Boot Record Area found at offset %d\n",
	    sc->sc_dev.dv_xname, offset);

	/*
	 * Parse Boot Record Area (BRA) options.
	 */
	while (offset < sc->sc_fw_size && bra_end == 0) {
		/* get current BRA option */
		bra_option = (struct upgt_fw_bra_option *)(sc->sc_fw + offset);
		bra_option_type = letoh32(bra_option->type);
		bra_option_len = letoh32(bra_option->len) * sizeof(uc);

		switch (bra_option_type) {
		case UPGT_BRA_TYPE_FW:
			DPRINTF(1, "%s: UPGT_BRA_TYPE_FW len=%d\n",
			    sc->sc_dev.dv_xname, bra_option_len);

			if (bra_option_len != UPGT_BRA_FWTYPE_SIZE) {
				printf("%s: wrong UPGT_BRA_TYPE_FW len!\n",
				    sc->sc_dev.dv_xname);
				return (EIO);
			}
			if (memcmp(UPGT_BRA_FWTYPE_LM86, bra_option->data,
			    bra_option_len) == 0) {
				sc->sc_fw_type = UPGT_FWTYPE_LM86;
				break;
			}
			if (memcmp(UPGT_BRA_FWTYPE_LM87, bra_option->data,
			    bra_option_len) == 0) {
				sc->sc_fw_type = UPGT_FWTYPE_LM87;
				break;
			}
			if (memcmp(UPGT_BRA_FWTYPE_FMAC, bra_option->data,
			    bra_option_len) == 0) {
				sc->sc_fw_type = UPGT_FWTYPE_FMAC;
				break;
			}
			printf("%s: unsupported firmware type!\n",
			    sc->sc_dev.dv_xname);
			return (EIO);
		case UPGT_BRA_TYPE_VERSION:
			DPRINTF(1, "%s: UPGT_BRA_TYPE_VERSION len=%d\n",
			    sc->sc_dev.dv_xname, bra_option_len);
			break;
		case UPGT_BRA_TYPE_DEPIF:
			DPRINTF(1, "%s: UPGT_BRA_TYPE_DEPIF len=%d\n",
			    sc->sc_dev.dv_xname, bra_option_len);
			break;
		case UPGT_BRA_TYPE_EXPIF:
			DPRINTF(1, "%s: UPGT_BRA_TYPE_EXPIF len=%d\n",
			    sc->sc_dev.dv_xname, bra_option_len);
			break;
		case UPGT_BRA_TYPE_DESCR:
			DPRINTF(1, "%s: UPGT_BRA_TYPE_DESCR len=%d\n",
			    sc->sc_dev.dv_xname, bra_option_len);

			struct upgt_fw_bra_descr *descr =
				(struct upgt_fw_bra_descr *)bra_option->data;

			sc->sc_memaddr_frame_start =
			    letoh32(descr->memaddr_space_start);
			sc->sc_memaddr_frame_end =
			    letoh32(descr->memaddr_space_end);

			DPRINTF(2, "%s: memory address space start=0x%08x\n",
			    sc->sc_dev.dv_xname, sc->sc_memaddr_frame_start);
			DPRINTF(2, "%s: memory address space end=0x%08x\n",
			    sc->sc_dev.dv_xname, sc->sc_memaddr_frame_end);
			break;
		case UPGT_BRA_TYPE_END:
			DPRINTF(1, "%s: UPGT_BRA_TYPE_END len=%d\n",
			    sc->sc_dev.dv_xname, bra_option_len);
			bra_end = 1;
			break;
		default:
			DPRINTF(1, "%s: unknown BRA option len=%d\n",
			    sc->sc_dev.dv_xname, bra_option_len);
			return (EIO);
		}

		/* jump to next BRA option */
		offset += sizeof(struct upgt_fw_bra_option) + bra_option_len;
	}

	DPRINTF(1, "%s: firmware verified\n", sc->sc_dev.dv_xname);

	return (0);
}

int
upgt_fw_load(struct upgt_softc *sc)
{
	struct upgt_data *data_cmd = &sc->cmd_data;
	struct upgt_data *data_rx = &sc->rx_data;
	char start_fwload_cmd[] = { 0x3c, 0x0d };
	int offset, bsize, n, i, len;
	uint32_t crc32;

	/* send firmware start load command */
	len = sizeof(start_fwload_cmd);
	bcopy(start_fwload_cmd, data_cmd->buf, len);
	if (upgt_bulk_xmit(sc, data_cmd, sc->sc_tx_pipeh, &len, 0) != 0) {
		printf("%s: could not send start_firmware_load command!\n",
		    sc->sc_dev.dv_xname);
		return (EIO);
	}

	/* send X2 header */
	len = sizeof(struct upgt_fw_x2_header);
	struct upgt_fw_x2_header *x2 = data_cmd->buf;
	bcopy(UPGT_X2_SIGNATURE, x2->signature, UPGT_X2_SIGNATURE_SIZE);
	x2->startaddr = htole32(UPGT_MEMADDR_FIRMWARE_START);
	x2->len = htole32(sc->sc_fw_size);
	x2->crc = htole32(upgt_crc32(data_cmd->buf + UPGT_X2_SIGNATURE_SIZE,
	    sizeof(struct upgt_fw_x2_header) - UPGT_X2_SIGNATURE_SIZE -
	    sizeof(uint32_t)));
	if (upgt_bulk_xmit(sc, data_cmd, sc->sc_tx_pipeh, &len, 0) != 0) {
		printf("%s: could not send firmware X2 header!\n",
		    sc->sc_dev.dv_xname);
		return (EIO);
	}

	/* download firmware */
	for (offset = 0; offset < sc->sc_fw_size; offset += bsize) {
		if (sc->sc_fw_size - offset > UPGT_FW_BLOCK_SIZE)
			bsize = UPGT_FW_BLOCK_SIZE;
		else
			bsize = sc->sc_fw_size - offset;

		n = upgt_fw_copy(sc->sc_fw + offset, data_cmd->buf, bsize);

		DPRINTF(1, "%s: FW offset=%d, read=%d, sent=%d\n",
		    sc->sc_dev.dv_xname, offset, n, bsize);

		if (upgt_bulk_xmit(sc, data_cmd, sc->sc_tx_pipeh, &bsize, 0)
		    != 0) {
			printf("%s: error while downloading firmware block!\n",
			    sc->sc_dev.dv_xname);
			return (EIO);
		}

		bsize = n;
	}
	DPRINTF(1, "%s: firmware downloaded\n", sc->sc_dev.dv_xname);

	/* load firmware */
	crc32 = htole32(upgt_crc32(sc->sc_fw, sc->sc_fw_size));
	*((uint32_t *)(data_cmd->buf)    ) = crc32;
	*((uint8_t  *)(data_cmd->buf) + 4) = 'g';
	*((uint8_t  *)(data_cmd->buf) + 5) = '\r';
	len = 6;
	if (upgt_bulk_xmit(sc, data_cmd, sc->sc_tx_pipeh, &len, 0) != 0) {
		printf("%s: could not send load_firmware command!\n",
		    sc->sc_dev.dv_xname);
		return (EIO);
	}

	for (i = 0; i < UPGT_FIRMWARE_TIMEOUT; i++) {
		len = UPGT_FW_BLOCK_SIZE;
		bzero(data_rx->buf, MCLBYTES);
		if (upgt_bulk_xmit(sc, data_rx, sc->sc_rx_pipeh, &len,
		    USBD_SHORT_XFER_OK) != 0) {
			printf("%s: could not read firmware response!\n",
			    sc->sc_dev.dv_xname);
			return (EIO);
		}

		if (memcmp(data_rx->buf, "OK", 2) == 0)
			break;	/* firmware load was successful */
	}
	if (i == UPGT_FIRMWARE_TIMEOUT) {
		printf("%s: firmware load failed!\n", sc->sc_dev.dv_xname);
		return (EIO);
	}
	DPRINTF(1, "%s: firmware loaded\n", sc->sc_dev.dv_xname);

	return (0);
}

/*
 * While copying the version 2 firmware, we need to replace two characters:
 *
 * 0x7e -> 0x7d 0x5e
 * 0x7d -> 0x7d 0x5d
 */
int
upgt_fw_copy(char *src, char *dst, int size)
{
	int i, j;

	for (i = 0, j = 0; i < size && j < size; i++) {
		switch (src[i]) {
		case 0x7e:
			dst[j] = 0x7d;
			j++;
			dst[j] = 0x5e;
			j++;
			break;
		case 0x7d:
			dst[j] = 0x7d;
			j++;
			dst[j] = 0x5d;
			j++;
			break;
		default:
			dst[j] = src[i];
			j++;
			break;
		}
	}

	return (i);
}

int
upgt_eeprom_read(struct upgt_softc *sc)
{
	struct upgt_data *data_cmd = &sc->cmd_data;
	struct upgt_lmac_eeprom	*eeprom;
	int offset, block, len;

	block = UPGT_EEPROM_BLOCK_SIZE;
	offset = UPGT_EEPROM_SIZE;
	while (offset > 0) {
		offset = offset - block;

		DPRINTF(1, "%s: request EEPROM block (offset=%d, len=%d)\n",
		    sc->sc_dev.dv_xname, offset, block);

		/*
		 * Transmit the first URB containing the Prism memory address.
		 */
		*((uint32_t *)data_cmd->buf) =
		    htole32(sc->sc_memaddr_frame_start +
		    UPGT_MEMSIZE_FRAME_HEAD);

		len = sizeof(uint32_t);
		if (upgt_bulk_xmit(sc, data_cmd, sc->sc_tx_pipeh, &len, 0)
		    != 0) {
			printf("%s: could not transmit EEPROM memaddr URB!\n",
			    sc->sc_dev.dv_xname);
			return (EIO);
		}

		/*
		 * Transmit the second URB containing the CMD data itself.
		 */
		eeprom = (struct upgt_lmac_eeprom *)data_cmd->buf;
		eeprom->header1.flags = 0;
		eeprom->header1.type = UPGT_H1_TYPE_CTRL;
		eeprom->header1.len = htole16((
		    sizeof(struct upgt_lmac_eeprom) -
		    sizeof(struct upgt_lmac_header)) + block);

		eeprom->header2.reqid = htole32(sc->sc_memaddr_frame_start);
		eeprom->header2.type = htole16(UPGT_H2_TYPE_EEPROM);
		eeprom->header2.flags = 0;

		eeprom->offset = htole16(offset);
		eeprom->len = htole16(block);

		len = sizeof(struct upgt_lmac_eeprom) + block;
		if (upgt_bulk_xmit(sc, data_cmd, sc->sc_tx_pipeh, &len, 0)
		    != 0) {
			printf("%s: could not transmit EEPROM data URB!\n",
			    sc->sc_dev.dv_xname);
			return (EIO);
		}
		if (tsleep(sc, 0, "eeprom_request", UPGT_USB_TIMEOUT)) {
			printf("%s: timeout while waiting for EEPROM data!\n",
			    sc->sc_dev.dv_xname);
			return (EIO);
		}

		if (offset < block)
			block = offset;
	}

	return (0);
}

int
upgt_eeprom_parse(struct upgt_softc *sc)
{
	struct ieee80211com *ic = &sc->sc_ic;
	struct upgt_eeprom_header *eeprom_header;
	struct upgt_eeprom_option *eeprom_option;
	uint16_t option_len;
	uint16_t option_type;
	uint16_t preamble_len;
	int option_end = 0;

	/* calculate eeprom options start offset */
	eeprom_header = (struct upgt_eeprom_header *)sc->sc_eeprom;
	preamble_len = letoh16(eeprom_header->preamble_len);
	eeprom_option = (struct upgt_eeprom_option *)(sc->sc_eeprom +
	    (sizeof(struct upgt_eeprom_header) + preamble_len));

	while (!option_end) {
		/* the eeprom option length is stored in words */
		option_len =
		    (letoh16(eeprom_option->len) - 1) * sizeof(uint16_t);
		option_type =
		    letoh16(eeprom_option->type);

		switch (option_type) {
		case UPGT_EEPROM_TYPE_NAME:
			DPRINTF(1, "%s: EEPROM name len=%d\n",
			    sc->sc_dev.dv_xname, option_len);
			break;
		case UPGT_EEPROM_TYPE_SERIAL:
			DPRINTF(1, "%s: EEPROM serial len=%d\n",
			    sc->sc_dev.dv_xname, option_len);
			break;
		case UPGT_EEPROM_TYPE_MAC:
			DPRINTF(1, "%s: EEPROM mac len=%d\n",
			    sc->sc_dev.dv_xname, option_len);

			IEEE80211_ADDR_COPY(ic->ic_myaddr, eeprom_option->data);
			break;
		case UPGT_EEPROM_TYPE_HWRX:
			DPRINTF(1, "%s: EEPROM hwrx len=%d\n",
			    sc->sc_dev.dv_xname, option_len);

			upgt_eeprom_parse_hwrx(sc, eeprom_option->data);
			break;
		case UPGT_EEPROM_TYPE_CHIP:
			DPRINTF(1, "%s: EEPROM chip len=%d\n",
			    sc->sc_dev.dv_xname, option_len);
			break;
		case UPGT_EEPROM_TYPE_FREQ3:
			DPRINTF(1, "%s: EEPROM freq3 len=%d\n",
			    sc->sc_dev.dv_xname, option_len);

			upgt_eeprom_parse_freq3(sc, eeprom_option->data,
			    option_len);
			break;
		case UPGT_EEPROM_TYPE_FREQ4:
			DPRINTF(1, "%s: EEPROM freq4 len=%d\n",
			    sc->sc_dev.dv_xname, option_len);

			upgt_eeprom_parse_freq4(sc, eeprom_option->data,
			    option_len);
			break;
		case UPGT_EEPROM_TYPE_FREQ5:
			DPRINTF(1, "%s: EEPROM freq5 len=%d\n",
			    sc->sc_dev.dv_xname, option_len);
			break;
		case UPGT_EEPROM_TYPE_FREQ6:
			DPRINTF(1, "%s: EEPROM freq6 len=%d\n",
			    sc->sc_dev.dv_xname, option_len);

			upgt_eeprom_parse_freq6(sc, eeprom_option->data,
			    option_len);
			break;
		case UPGT_EEPROM_TYPE_END:
			DPRINTF(1, "%s: EEPROM end len=%d\n",
			    sc->sc_dev.dv_xname, option_len);
			option_end = 1;
			break;
		case UPGT_EEPROM_TYPE_OFF:
			DPRINTF(1, "%s: EEPROM off without end option!\n",
			    sc->sc_dev.dv_xname);
			return (EIO);
		default:
			DPRINTF(1, "%s: EEPROM unknown type 0x%04x len=%d\n",
			    sc->sc_dev.dv_xname, option_type, option_len);
			break;
		}

		/* jump to next EEPROM option */
		eeprom_option = (struct upgt_eeprom_option *)
		    (eeprom_option->data + option_len);
	}

	return (0);
}

void
upgt_eeprom_parse_hwrx(struct upgt_softc *sc, uint8_t *data)
{
	struct upgt_eeprom_option_hwrx *option_hwrx;

	option_hwrx = (struct upgt_eeprom_option_hwrx *)data;

	sc->sc_eeprom_hwrx = option_hwrx->rxfilter - UPGT_EEPROM_RX_CONST;

	DPRINTF(2, "%s: hwrx option value=0x%04x\n",
	    sc->sc_dev.dv_xname, sc->sc_eeprom_hwrx);
}

void
upgt_eeprom_parse_freq3(struct upgt_softc *sc, uint8_t *data, int len)
{
	struct upgt_eeprom_freq3_header *freq3_header;
	struct upgt_lmac_freq3 *freq3;
	int i, elements, flags;
	unsigned channel;

	freq3_header = (struct upgt_eeprom_freq3_header *)data;
	freq3 = (struct upgt_lmac_freq3 *)(freq3_header + 1);

	flags = freq3_header->flags;
	elements = freq3_header->elements;

	DPRINTF(2, "%s: flags=0x%02x\n", sc->sc_dev.dv_xname, flags);
	DPRINTF(2, "%s: elements=%d\n", sc->sc_dev.dv_xname, elements);

	for (i = 0; i < elements; i++) {
		channel = ieee80211_mhz2ieee(letoh16(freq3[i].freq), 0);

		sc->sc_eeprom_freq3[channel] = freq3[i];

		DPRINTF(2, "%s: frequence=%d, channel=%d\n",
		    sc->sc_dev.dv_xname,
		    letoh16(sc->sc_eeprom_freq3[channel].freq), channel);
	}
}

void
upgt_eeprom_parse_freq4(struct upgt_softc *sc, uint8_t *data, int len)
{
	struct upgt_eeprom_freq4_header *freq4_header;
	struct upgt_eeprom_freq4_1 *freq4_1;
	struct upgt_eeprom_freq4_2 *freq4_2;
	int i, j, elements, settings, flags;
	unsigned channel;

	freq4_header = (struct upgt_eeprom_freq4_header *)data;
	freq4_1 = (struct upgt_eeprom_freq4_1 *)(freq4_header + 1);

	flags = freq4_header->flags;
	elements = freq4_header->elements;
	settings = freq4_header->settings;

	/* we need this value later */
	sc->sc_eeprom_freq6_settings = freq4_header->settings;

	DPRINTF(2, "%s: flags=0x%02x\n", sc->sc_dev.dv_xname, flags);
	DPRINTF(2, "%s: elements=%d\n", sc->sc_dev.dv_xname, elements);
	DPRINTF(2, "%s: settings=%d\n", sc->sc_dev.dv_xname, settings);

	for (i = 0; i < elements; i++) {
		channel = ieee80211_mhz2ieee(letoh16(freq4_1[i].freq), 0);

		freq4_2 = (struct upgt_eeprom_freq4_2 *)freq4_1[i].data;

		for (j = 0; j < settings; j++) {
			sc->sc_eeprom_freq4[channel][j].cmd = freq4_2[j];
			sc->sc_eeprom_freq4[channel][j].pad = 0;
		}

		DPRINTF(2, "%s: frequence=%d, channel=%d\n",
		    sc->sc_dev.dv_xname,
		    letoh16(freq4_1[i].freq), channel);
	}
}

void
upgt_eeprom_parse_freq6(struct upgt_softc *sc, uint8_t *data, int len)
{
	struct upgt_lmac_freq6 *freq6;
	int i, elements;
	unsigned channel;

	freq6 = (struct upgt_lmac_freq6 *)data;

	elements = len / sizeof(struct upgt_lmac_freq6);

	DPRINTF(2, "%s: elements=%d\n", sc->sc_dev.dv_xname, elements);

	for (i = 0; i < elements; i++) {
		channel = ieee80211_mhz2ieee(letoh16(freq6[i].freq), 0);

		sc->sc_eeprom_freq6[channel] = freq6[i];

		DPRINTF(2, "%s: frequence=%d, channel=%d\n",
		    sc->sc_dev.dv_xname,
		    letoh16(sc->sc_eeprom_freq6[channel].freq), channel);
	}
}

int
upgt_ioctl(struct ifnet *ifp, u_long cmd, caddr_t data)
{
	struct upgt_softc *sc = ifp->if_softc;
	struct ieee80211com *ic = &sc->sc_ic;
	struct ifaddr *ifa;
	struct ifreq *ifr;
	int s, error = 0;

	s = splnet();

	switch (cmd) {
	case SIOCSIFADDR:
		ifa = (struct ifaddr *)data;
		ifp->if_flags |= IFF_UP;
#ifdef INET
		if (ifa->ifa_addr->sa_family == AF_INET)
			arp_ifinit(&ic->ic_ac, ifa);
#endif
		/* FALLTHROUGH */
	case SIOCSIFFLAGS:
		if (ifp->if_flags & IFF_UP) {
			if ((ifp->if_flags & IFF_RUNNING) == 0)
				upgt_init(ifp);
		} else {
			if (ifp->if_flags & IFF_RUNNING)
				upgt_stop(sc);
		}
		break;
	case SIOCADDMULTI:
	case SIOCDELMULTI:
		ifr = (struct ifreq *)data;
		error = (cmd == SIOCADDMULTI) ?
			ether_addmulti(ifr, &ic->ic_ac) :
			ether_delmulti(ifr, &ic->ic_ac);
		if (error == ENETRESET)
			error = 0;
		break;
	default:
		error = ieee80211_ioctl(ifp, cmd, data);
		break;
	}

	if (error == ENETRESET) {
		if (ifp->if_flags & (IFF_UP | IFF_RUNNING))
			upgt_init(ifp);
		error = 0;
	}

	splx(s);

	return (error);
}

int
upgt_init(struct ifnet *ifp)
{
	struct upgt_softc *sc = ifp->if_softc;
	struct ieee80211com *ic = &sc->sc_ic;

	DPRINTF(1, "%s: %s\n", sc->sc_dev.dv_xname, __func__);

	IEEE80211_ADDR_COPY(ic->ic_myaddr, LLADDR(ifp->if_sadl));

	/* select default channel */
	ic->ic_bss->ni_chan = ic->ic_ibss_chan;
	sc->sc_curchan = ieee80211_chan2ieee(ic, ic->ic_bss->ni_chan);

	/* set default channel */
	upgt_set_channel(sc, sc->sc_curchan);

	upgt_set_macfilter(sc, IEEE80211_S_SCAN);

	ifp->if_flags |= IFF_RUNNING;
	ifp->if_flags &= ~IFF_OACTIVE;

	if (ic->ic_opmode == IEEE80211_M_MONITOR)
		ieee80211_new_state(ic, IEEE80211_S_RUN, -1);
	else
		ieee80211_new_state(ic, IEEE80211_S_SCAN, -1);

	return (0);
}

void
upgt_stop(struct upgt_softc *sc)
{
	struct ieee80211com *ic = &sc->sc_ic;
	struct ifnet *ifp = &ic->ic_if;

	/* device down */
	ifp->if_timer = 0;
	ifp->if_flags &= ~(IFF_RUNNING | IFF_OACTIVE);

	/* change device back to initial state */
	ieee80211_new_state(ic, IEEE80211_S_INIT, -1);
}

int
upgt_media_change(struct ifnet *ifp)
{
	int error;

	if ((error = ieee80211_media_change(ifp) != ENETRESET))
		return (error);

	if (ifp->if_flags & (IFF_UP | IFF_RUNNING))
		upgt_init(ifp);

	return (0);
}

void
upgt_newassoc(struct ieee80211com *ic, struct ieee80211_node *ni, int isnew)
{
	ni->ni_txrate = 0;
}

int
upgt_newstate(struct ieee80211com *ic, enum ieee80211_state nstate, int arg)
{
	struct upgt_softc *sc = ic->ic_if.if_softc;

	usb_rem_task(sc->sc_udev, &sc->sc_task_newstate);
	timeout_del(&sc->scan_to);

	/* do it in a process context */
	sc->sc_state = nstate;
	sc->sc_arg = arg;
	usb_add_task(sc->sc_udev, &sc->sc_task_newstate);

	return (0);
}

void
upgt_newstate_task(void *arg)
{
	struct upgt_softc *sc = arg;
	struct ieee80211com *ic = &sc->sc_ic;
	struct ieee80211_node *ni;
	enum ieee80211_state ostate;
	unsigned channel;

	ostate = ic->ic_state;

	switch (sc->sc_state) {
	case IEEE80211_S_INIT:
		DPRINTF(1, "%s: newstate is IEEE80211_S_INIT\n",
		    sc->sc_dev.dv_xname);
		break;
	case IEEE80211_S_SCAN:
		DPRINTF(1, "%s: newstate is IEEE80211_S_SCAN\n",
		    sc->sc_dev.dv_xname);

		channel = ieee80211_chan2ieee(ic, ic->ic_bss->ni_chan);
		upgt_set_channel(sc, channel);

		timeout_add(&sc->scan_to, hz / 5);
		break;
	case IEEE80211_S_AUTH:
		DPRINTF(1, "%s: newstate is IEEE80211_S_AUTH\n",
		    sc->sc_dev.dv_xname);

		channel = ieee80211_chan2ieee(ic, ic->ic_bss->ni_chan);
		upgt_set_channel(sc, channel);
		break;
	case IEEE80211_S_ASSOC:
		DPRINTF(1, "%s: newstate is IEEE80211_S_ASSOC\n",
		    sc->sc_dev.dv_xname);
		break;
	case IEEE80211_S_RUN:
		DPRINTF(1, "%s: newstate is IEEE80211_S_RUN\n",
		    sc->sc_dev.dv_xname);

		ni = ic->ic_bss;

		upgt_set_macfilter(sc, IEEE80211_S_RUN);
		break;
	}

	sc->sc_newstate(ic, sc->sc_state, sc->sc_arg);
}

void
upgt_next_scan(void *arg)
{
	struct upgt_softc *sc = arg;
	struct ieee80211com *ic = &sc->sc_ic;
	struct ifnet *ifp = &ic->ic_if;

	DPRINTF(2, "%s: %s\n", sc->sc_dev.dv_xname, __func__);

	if (ic->ic_state == IEEE80211_S_SCAN)
		ieee80211_next_scan(ifp);
}

void
upgt_start(struct ifnet *ifp)
{
	struct upgt_softc *sc = ifp->if_softc;
	struct ieee80211com *ic = &sc->sc_ic;
	struct ieee80211_node *ni;
	struct mbuf *m;
	int i;

	/* don't transmit packets if interface is busy or down */
	if ((ifp->if_flags & (IFF_RUNNING | IFF_OACTIVE)) != IFF_RUNNING)
		return;

	DPRINTF(2, "%s: %s\n", sc->sc_dev.dv_xname, __func__);

	usb_rem_task(sc->sc_udev, &sc->sc_task_tx);

	for (i = 0; i < UPGT_TX_COUNT; i++) {
		struct upgt_data *data_tx = &sc->tx_data[i];

		IF_POLL(&ic->ic_mgtq, m);
		if (m != NULL) {
			/* management frame */
			IF_DEQUEUE(&ic->ic_mgtq, m);

			ni = (struct ieee80211_node *)m->m_pkthdr.rcvif;
			m->m_pkthdr.rcvif = NULL;
#if NBPFILTER > 0
			if (ic->ic_rawbpf != NULL)
				bpf_mtap(ic->ic_rawbpf, m, BPF_DIRECTION_OUT);
#endif
			if ((data_tx->addr = upgt_mem_alloc(sc)) == 0) {
				printf("%s: no free memory!\n",
				    sc->sc_dev.dv_xname);
				return;
			}
			data_tx->ni = ni;
			data_tx->m = m;
			sc->tx_queued++;
		} else {
			/* data frame */
			if (ic->ic_state != IEEE80211_S_RUN)
				break;

			IFQ_POLL(&ifp->if_snd, m);
			if (m == NULL)
				break;

			IFQ_DEQUEUE(&ifp->if_snd, m);
#if NBPFILTER > 0
			if (ifp->if_bpf != NULL)
				bpf_mtap(ifp->if_bpf, m, BPF_DIRECTION_OUT);
#endif
			m = ieee80211_encap(ifp, m, &ni);
			if (m == NULL)
				continue;
#if NBPFILTER > 0
			if (ic->ic_rawbpf != NULL)
				bpf_mtap(ic->ic_rawbpf, m, BPF_DIRECTION_OUT);
#endif
			if ((data_tx->addr = upgt_mem_alloc(sc)) == 0) {
				printf("%s: no free memory!\n",
				    sc->sc_dev.dv_xname);
				return;
			}
			data_tx->ni = ni;
			data_tx->m = m;
			sc->tx_queued++;
		}
	}

	if (sc->tx_queued > 0) {
		DPRINTF(2, "%s: tx_queued=%d\n",
		    sc->sc_dev.dv_xname, sc->tx_queued);
		/* process the TX queue in process context */
		ifp->if_timer = 5;
		ifp->if_flags |= IFF_OACTIVE;
		usb_add_task(sc->sc_udev, &sc->sc_task_tx);
	}
}

void
upgt_watchdog(struct ifnet *ifp)
{
	struct upgt_softc *sc;

	sc = ifp->if_softc;

	DPRINTF(1, "%s: %s\n", sc->sc_dev.dv_xname, __func__);

	/* TODO: what shall we do on TX timeout? */

	ieee80211_watchdog(ifp);
}

void
upgt_tx_task(void *arg)
{
	struct upgt_softc *sc = arg;
	struct ieee80211com *ic = &sc->sc_ic;
	struct ifnet *ifp = &ic->ic_if;
	struct ieee80211_frame *wh;
	struct upgt_lmac_tx_desc *txdesc;
	struct mbuf *m;
	uint32_t addr;
	int len, i;

	for (i = 0; i < UPGT_TX_COUNT; i++) {
		struct upgt_data *data_tx = &sc->tx_data[i];

		if (data_tx->m == NULL)
			continue;

		m = data_tx->m;
		addr = data_tx->addr + UPGT_MEMSIZE_FRAME_HEAD;

		/*
		 * Software WEP.
		 */
		wh = mtod(m, struct ieee80211_frame *);

		if (wh->i_fc[1] & IEEE80211_FC1_WEP) {
			m = ieee80211_wep_crypt(ifp, m, 1);
			if (m == NULL)
				return;
		}

		/*
		 * Transmit the first URB containing the Prism memory address.
		 */
		*((uint32_t *)data_tx->buf) = htole32(addr);

		len = sizeof(uint32_t);
		if (upgt_bulk_xmit(sc, data_tx, sc->sc_tx_pipeh, &len, 0)
		    != 0) {
			printf("%s: could not transmit TX memaddr URB!\n",
			    sc->sc_dev.dv_xname);
			return;
		}

		DPRINTF(2, "%s: TX memaddr sent\n", sc->sc_dev.dv_xname);

		/*
		 * Transmit the second URB containing the TX data itself.
		 */
		bzero(data_tx->buf, MCLBYTES);

		txdesc = (struct upgt_lmac_tx_desc *)data_tx->buf;

		/* XXX differ between data and mgmt frames? */
		txdesc->header1.flags = UPGT_H1_FLAGS_TX_DATA;
		txdesc->header1.type = UPGT_H1_TYPE_DATA_TX;
		txdesc->header1.len = htole16(m->m_pkthdr.len);

		txdesc->header2.reqid = htole32(data_tx->addr);
		txdesc->header2.type = htole16(UPGT_H2_TYPE_TX_NOACK);
		txdesc->header2.flags = 0;

		txdesc->type = htole32(UPGT_TX_DESC_TYPE_DATA);
		txdesc->unknown1 = 0;
		txdesc->unknown2 = 0;
		/* round TX descriptor header up to 4 byte boundary */
		txdesc->frame_pad = UPGT_TX_DESC_FRAME_PAD;

#if NBPFILTER > 0
		if (sc->sc_drvbpf != NULL) {
			struct mbuf mb;
			struct upgt_tx_radiotap_header *tap = &sc->sc_txtap;

			tap->wt_flags = 0;
			tap->wt_rate = 0;
			tap->wt_chan_freq =
			    htole16(ic->ic_bss->ni_chan->ic_freq);
			tap->wt_chan_flags =
			    htole16(ic->ic_bss->ni_chan->ic_flags);
			tap->wt_antenna = 0;

			mb.m_data = (caddr_t)tap;
			mb.m_len = sc->sc_txtap_len;
			mb.m_next = m;
			mb.m_nextpkt = NULL;
			mb.m_type = 0;
			mb.m_flags = 0;
			bpf_mtap(sc->sc_drvbpf, &mb, BPF_DIRECTION_OUT);
		}
#endif

		/* copy frame below our TX descriptor header */
		m_copydata(m, 0, m->m_pkthdr.len,
		    data_tx->buf + sizeof(struct upgt_lmac_tx_desc));

		len = sizeof(struct upgt_lmac_tx_desc) + m->m_pkthdr.len;
		if (upgt_bulk_xmit(sc, data_tx, sc->sc_tx_pipeh, &len, 0)
		    != 0) {
			printf("%s: could not transmit TX data URB!\n",
			    sc->sc_dev.dv_xname);
			return;
		}

		/* we do not need the mbuf anymore */
		m_freem(m);

		DPRINTF(2, "%s: TX sent: len=%d\n", sc->sc_dev.dv_xname, len);
	}
}

void
upgt_tx_done(struct upgt_softc *sc, uint8_t *data)
{
	struct ieee80211com *ic = &sc->sc_ic;
	struct ifnet *ifp = &ic->ic_if;
	struct upgt_lmac_tx_done_desc *desc;
	int i, s;

	s = splnet();

	desc = (struct upgt_lmac_tx_done_desc *)data;

	for (i = 0; i < UPGT_TX_COUNT; i++) {
		struct upgt_data *data_tx = &sc->tx_data[i];

		if (data_tx->addr == letoh32(desc->header2.reqid)) {
			ieee80211_release_node(ic, data_tx->ni);
			data_tx->ni = NULL;
			data_tx->m = NULL;
			upgt_mem_free(sc, data_tx->addr);
			data_tx->addr = 0;

			sc->tx_queued--;
			ifp->if_opackets++;

			DPRINTF(2, "%s: TX done: ", sc->sc_dev.dv_xname);
			DPRINTF(2, "memaddr=0x%08x, status=0x%04x, rssi=%d, ",
			    letoh32(desc->header2.reqid),
			    letoh16(desc->status),
			    letoh16(desc->rssi));
			DPRINTF(2, "seq=%d\n", letoh16(desc->seq));

			break;
		}
	}

	if (sc->tx_queued == 0) {
		/* TX queued was processed, continue */
		ifp->if_timer = 0;
		ifp->if_flags &= ~IFF_OACTIVE;
		upgt_start(ifp);
	}

	splx(s);
}

void
upgt_rx_cb(usbd_xfer_handle xfer, usbd_private_handle priv, usbd_status status)
{
	struct upgt_data *data_rx = priv;
	struct upgt_softc *sc = data_rx->sc;
	int len;
	struct upgt_lmac_header *header;
	struct upgt_lmac_eeprom *eeprom;
	uint8_t h1_type;
	uint16_t h2_type;

	DPRINTF(3, "%s: %s\n", sc->sc_dev.dv_xname, __func__);

	if (status != USBD_NORMAL_COMPLETION) {
		if (status == USBD_NOT_STARTED || status == USBD_CANCELLED)
			return;
		if (status == USBD_STALLED)
			usbd_clear_endpoint_stall_async(sc->sc_rx_pipeh);
		goto skip;
	}
	usbd_get_xfer_status(xfer, NULL, NULL, &len, NULL);

	/*
	 * Check what type of frame came in.
	 */
	header = (struct upgt_lmac_header *)data_rx->buf;

	h1_type = header->header1.type;
	h2_type = letoh16(header->header2.type);

	if (h1_type == UPGT_H1_TYPE_CTRL &&
	    h2_type == UPGT_H2_TYPE_EEPROM) {
		eeprom = (struct upgt_lmac_eeprom *)data_rx->buf;
		uint16_t eeprom_offset = letoh16(eeprom->offset);
		uint16_t eeprom_len = letoh16(eeprom->len);

		DPRINTF(2, "%s: received EEPROM block (offset=%d, len=%d)\n",
			sc->sc_dev.dv_xname, eeprom_offset, eeprom_len);

		bcopy(data_rx->buf + sizeof(struct upgt_lmac_eeprom),
			sc->sc_eeprom + eeprom_offset, eeprom_len);

		/* EEPROM data has arrived in time, wakeup tsleep() */
		wakeup(sc);
	} else
	if (h1_type == UPGT_H1_TYPE_CTRL &&
	    h2_type == UPGT_H2_TYPE_TX_DONE) {
		DPRINTF(2, "%s: received 802.11 TX done\n",
		    sc->sc_dev.dv_xname);

		upgt_tx_done(sc, data_rx->buf);
	} else
	if (h1_type == UPGT_H1_TYPE_DATA_RX ||
	    h1_type == UPGT_H1_TYPE_DATA_RX_MGMT) {
		DPRINTF(3, "%s: received 802.11 RX data\n",
		    sc->sc_dev.dv_xname);

		upgt_rx(sc, data_rx->buf, letoh16(header->header1.len));
	} else {
		/* ignore unknown frame types */
		DPRINTF(1, "%s: received unknown frame type 0x%02x\n",
		    sc->sc_dev.dv_xname, header->header1.type);
	}

skip:	/* setup new transfer */
	usbd_setup_xfer(xfer, sc->sc_rx_pipeh, data_rx, data_rx->buf, MCLBYTES,
	    USBD_SHORT_XFER_OK, USBD_NO_TIMEOUT, upgt_rx_cb);
	(void)usbd_transfer(xfer);
}

void
upgt_rx(struct upgt_softc *sc, uint8_t *data, int pkglen)
{
	struct ieee80211com *ic = &sc->sc_ic;
	struct ifnet *ifp = &ic->ic_if;
	struct upgt_lmac_rx_desc *rxdesc;
	struct ieee80211_frame *wh;
	struct ieee80211_node *ni;
	struct mbuf *m;
	int s;

	/* access RX packet descriptor */
	rxdesc = (struct upgt_lmac_rx_desc *)data;

	/* create mbuf which is suitable for strict alignment archs */
	m = m_devget(rxdesc->data - ETHER_ALIGN, pkglen + ETHER_ALIGN, 0, ifp,
	    NULL);
	if (m == NULL) {
		printf("%s: could not create RX mbuf!\n", sc->sc_dev.dv_xname);
		return;
	}
	m_adj(m, ETHER_ALIGN);

	/* trim FCS */
	m_adj(m, -IEEE80211_CRC_LEN);

	s = splnet();

#if NBPFILTER > 0
	if (sc->sc_drvbpf != NULL) {
		struct mbuf mb;
		struct upgt_rx_radiotap_header *tap = &sc->sc_rxtap;

		tap->wr_flags = 0;
		tap->wr_rate = 0;
		tap->wr_chan_freq = htole16(ic->ic_bss->ni_chan->ic_freq);
		tap->wr_chan_flags = htole16(ic->ic_bss->ni_chan->ic_flags);
		tap->wr_antenna = 0;
		tap->wr_antsignal = rxdesc->rssi;

		mb.m_data = (caddr_t)tap;
		mb.m_len = sc->sc_rxtap_len;
		mb.m_next = m;
		mb.m_nextpkt = NULL;
		mb.m_type = 0;
		mb.m_flags = 0;
		bpf_mtap(sc->sc_drvbpf, &mb, BPF_DIRECTION_IN);
	}
#endif

	wh = mtod(m, struct ieee80211_frame *);
	ni = ieee80211_find_rxnode(ic, wh);

	/* push the frame up to the 802.11 stack */
	ieee80211_input(ifp, m, ni, rxdesc->rssi, 0);

	/* node is no longer needed */
	ieee80211_release_node(ic, ni);

	/* increment success incoming packet counter */
	ifp->if_ipackets++;

	splx(s);

	DPRINTF(3, "%s: RX done\n", sc->sc_dev.dv_xname);
}

int
upgt_set_macfilter(struct upgt_softc *sc, uint8_t state)
{
	struct ieee80211com *ic = &sc->sc_ic;
	struct ieee80211_node *ni = ic->ic_bss;
	struct upgt_data *data_cmd = &sc->cmd_data;
	struct upgt_lmac_filter *filter;
	int len;
	uint8_t broadcast[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

	/*
	 * Transmit the first URB containing the Prism memory address.
	 */
	*((uint32_t *)data_cmd->buf) =
	    htole32(sc->sc_memaddr_frame_start + UPGT_MEMSIZE_FRAME_HEAD);

	len = sizeof(uint32_t);
	if (upgt_bulk_xmit(sc, data_cmd, sc->sc_tx_pipeh, &len, 0) != 0) {
		printf("%s: could not transmit macfilter CMD memaddr URB!\n",
		    sc->sc_dev.dv_xname);
		return (EIO);
	}

	/*
	 * Transmit the second URB containing the CMD data itself.
	 */
	bzero(data_cmd->buf, MCLBYTES);

	filter = (struct upgt_lmac_filter *)data_cmd->buf;

	filter->header1.flags = UPGT_H1_FLAGS_TX_NO_CALLBACK;
	filter->header1.type = UPGT_H1_TYPE_CTRL;
	filter->header1.len = htole16(
	    sizeof(struct upgt_lmac_filter) -
	    sizeof(struct upgt_lmac_header));

	filter->header2.reqid = htole32(sc->sc_memaddr_frame_start);
	filter->header2.type = htole16(UPGT_H2_TYPE_MACFILTER);
	filter->header2.flags = 0;

	switch (state) {
	case IEEE80211_S_SCAN:
		DPRINTF(1, "%s: set MAC filter to SCAN (bssid %s)\n",
		    sc->sc_dev.dv_xname, ether_sprintf(broadcast));

		filter->type = htole16(UPGT_FILTER_TYPE_NONE);
		IEEE80211_ADDR_COPY(filter->dst, ic->ic_myaddr);
		IEEE80211_ADDR_COPY(filter->src, broadcast);
		filter->rxaddr = htole32(sc->sc_memaddr_rx_start);
		filter->rxhw = htole16(sc->sc_eeprom_hwrx);
		filter->unknown5 = htole16(UPGT_FILTER_UNKNOWN5_V2);
		break;
	case IEEE80211_S_RUN:
		DPRINTF(1, "%s: set MAC filter to RUN (bssid %s)\n",
		    sc->sc_dev.dv_xname, ether_sprintf(ni->ni_bssid));

		filter->type = htole16(UPGT_FILTER_TYPE_STA);
		IEEE80211_ADDR_COPY(filter->dst, ic->ic_myaddr);
		IEEE80211_ADDR_COPY(filter->src, ni->ni_bssid);
		filter->unknown2 = htole32(UPGT_FILTER_UNKNOWN2_STA);
		filter->rxaddr = htole32(sc->sc_memaddr_rx_start);
		filter->unknown5 = htole16(UPGT_FILTER_UNKNOWN5_V2);
		filter->rxhw = htole16(sc->sc_eeprom_hwrx);
		filter->unknown6 = htole16(UPGT_FILTER_UNKNOWN6_STA);
		break;
	default:
		printf("%s: MAC filter does not know that state!\n",
		    sc->sc_dev.dv_xname);
		break;
	}

	len = sizeof(struct upgt_lmac_filter);
	if (upgt_bulk_xmit(sc, data_cmd, sc->sc_tx_pipeh, &len, 0) != 0) {
		printf("%s: could not transmit macfilter CMD data URB!\n",
		    sc->sc_dev.dv_xname);
		return (EIO);
	}

	return (0);
}

int
upgt_set_channel(struct upgt_softc *sc, unsigned channel)
{
	struct upgt_data *data_cmd = &sc->cmd_data;
	struct upgt_lmac_channel *chan;
	int len;

	DPRINTF(1, "%s: %s: %d\n", sc->sc_dev.dv_xname, __func__, channel);

	/*
	 * Transmit the first URB containing the Prism memory address.
	 */
	*((uint32_t *)data_cmd->buf) =
	    htole32(sc->sc_memaddr_frame_start + UPGT_MEMSIZE_FRAME_HEAD);

	len = sizeof(uint32_t);
	if (upgt_bulk_xmit(sc, data_cmd, sc->sc_tx_pipeh, &len, 0) != 0) {
		printf("%s: could not transmit channel CMD memaddr URB!\n",
		    sc->sc_dev.dv_xname);
		return (EIO);
	}

	/*
	 * Transmit the second URB containing the CMD data itself.
	 */
	bzero(data_cmd->buf, MCLBYTES);

	chan = (struct upgt_lmac_channel *)data_cmd->buf;

	chan->header1.flags = UPGT_H1_FLAGS_TX_NO_CALLBACK;
	chan->header1.type = UPGT_H1_TYPE_CTRL;
	chan->header1.len = htole16(
	    sizeof(struct upgt_lmac_channel) -
	    sizeof(struct upgt_lmac_header));

	chan->header2.reqid = htole32(sc->sc_memaddr_frame_start);
	chan->header2.type = htole16(UPGT_H2_TYPE_CHANNEL);
	chan->header2.flags = 0;

	chan->unknown1 = htole16(UPGT_CHANNEL_UNKNOWN1);
	chan->unknown2 = htole16(UPGT_CHANNEL_UNKNOWN2);
	chan->freq6 = sc->sc_eeprom_freq6[channel];
	chan->settings = sc->sc_eeprom_freq6_settings;
	chan->unknown3 = UPGT_CHANNEL_UNKNOWN3;

	bcopy(&sc->sc_eeprom_freq3[channel].data, chan->freq3_1,
	    sizeof(chan->freq3_1));

	bcopy(&sc->sc_eeprom_freq4[channel], chan->freq4,
	    sizeof(sc->sc_eeprom_freq4[channel]));

	bcopy(&sc->sc_eeprom_freq3[channel].data, chan->freq3_2,
	    sizeof(chan->freq3_2));

	len = sizeof(struct upgt_lmac_channel);
	if (upgt_bulk_xmit(sc, data_cmd, sc->sc_tx_pipeh, &len, 0) != 0) {
		printf("%s: could not transmit channel CMD data URB!\n",
		    sc->sc_dev.dv_xname);
		return (EIO);
	}

	return (0);
}

int
upgt_alloc_tx(struct upgt_softc *sc)
{
	int i;

	sc->tx_queued = 0;

	for (i = 0; i < UPGT_TX_COUNT; i++) {
		struct upgt_data *data_tx = &sc->tx_data[i];

		data_tx->sc = sc;

		data_tx->xfer = usbd_alloc_xfer(sc->sc_udev);
		if (data_tx->xfer == NULL) {
			printf("%s: could not allocate TX xfer!\n",
			    sc->sc_dev.dv_xname);
			return (ENOMEM);
		}

		data_tx->buf = usbd_alloc_buffer(data_tx->xfer, MCLBYTES);
		if (data_tx->buf == NULL) {
			printf("%s: could not allocate TX buffer!\n",
			    sc->sc_dev.dv_xname);
			return (ENOMEM);
		}

		bzero(data_tx->buf, MCLBYTES);
	}

	return (0);
}

int
upgt_alloc_rx(struct upgt_softc *sc)
{
	struct upgt_data *data_rx = &sc->rx_data;

	data_rx->sc = sc;

	data_rx->xfer = usbd_alloc_xfer(sc->sc_udev);
	if (data_rx->xfer == NULL) {
		printf("%s: could not allocate RX xfer!\n",
		    sc->sc_dev.dv_xname);
		return (ENOMEM);
	}

	data_rx->buf = usbd_alloc_buffer(data_rx->xfer, MCLBYTES);
	if (data_rx->buf == NULL) {
		printf("%s: could not allocate RX buffer!\n",
		    sc->sc_dev.dv_xname);
		return (ENOMEM);
	}

	bzero(data_rx->buf, MCLBYTES);

	return (0);
}

int
upgt_alloc_cmd(struct upgt_softc *sc)
{
	struct upgt_data *data_cmd = &sc->cmd_data;

	data_cmd->sc = sc;

	data_cmd->xfer = usbd_alloc_xfer(sc->sc_udev);
	if (data_cmd->xfer == NULL) {
		printf("%s: could not allocate RX xfer!\n",
		    sc->sc_dev.dv_xname);
		return (ENOMEM);
	}

	data_cmd->buf = usbd_alloc_buffer(data_cmd->xfer, MCLBYTES);
	if (data_cmd->buf == NULL) {
		printf("%s: could not allocate RX buffer!\n",
		    sc->sc_dev.dv_xname);
		return (ENOMEM);
	}

	bzero(data_cmd->buf, MCLBYTES);

	return (0);
}

void
upgt_free_tx(struct upgt_softc *sc)
{
	int i;

	for (i = 0; i < UPGT_TX_COUNT; i++) {
		struct upgt_data *data_tx = &sc->tx_data[i];

		if (data_tx->xfer != NULL) {
			usbd_free_xfer(data_tx->xfer);
			data_tx->xfer = NULL;
		}

		data_tx->ni = NULL;
	}
}

void
upgt_free_rx(struct upgt_softc *sc)
{
	struct upgt_data *data_rx = &sc->rx_data;

	if (data_rx->xfer != NULL) {
		usbd_free_xfer(data_rx->xfer);
		data_rx->xfer = NULL;
	}

	data_rx->ni = NULL;
}

void
upgt_free_cmd(struct upgt_softc *sc)
{
	struct upgt_data *data_cmd = &sc->cmd_data;

	if (data_cmd->xfer != NULL) {
		usbd_free_xfer(data_cmd->xfer);
		data_cmd->xfer = NULL;
	}
}

/*
 * For this device we use usbd_bulk_transfer() for all TX transfers.
 */
int
upgt_bulk_xmit(struct upgt_softc *sc, struct upgt_data *data,
    usbd_pipe_handle pipeh, uint32_t *size, int flags)
{
        usbd_status status;

	status = usbd_bulk_transfer(data->xfer, pipeh,
	    USBD_NO_COPY | flags, UPGT_USB_TIMEOUT, data->buf, size,
	    "upgt_bulk_xmit");
	if (status != USBD_NORMAL_COMPLETION) {
		printf("%s: %s: error %s!\n",
		    sc->sc_dev.dv_xname, __func__, usbd_errstr(status));
		return (EIO);
	}

	return (0);
}

void
upgt_hexdump(void *buf, int len)
{
	int i;

	for (i = 0; i < len; i++) {
		if (i % 16 == 0)
			printf("%s%5i:", i ? "\n" : "", i);
		if (i % 4 == 0)
			printf(" ");
		printf("%02x", (int)*((u_char *)buf + i));
	}
	printf("\n");
}

/*
 * COPYRIGHT (C) 1986 Gary S. Brown.  You may use this program, or
 * code or tables extracted from it, as desired without restriction.
 */
static uint32_t upgt_crc32_tab[] = {
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
	0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
	0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
	0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
	0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
	0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
	0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
	0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
	0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
	0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
	0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
	0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
	0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
	0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
	0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
	0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
	0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
	0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
	0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
	0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
	0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
	0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

uint32_t
upgt_crc32(const void *buf, size_t size)
{
	const uint8_t *p;
	uint32_t crc;

	p = buf;
	crc = ~0U;

	while (size--)
		crc = upgt_crc32_tab[(crc ^ *p++) & 0xff] ^ (crc >> 8);

	return (crc ^ ~0U);
}
