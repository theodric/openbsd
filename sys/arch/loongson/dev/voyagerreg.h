/*	$OpenBSD: src/sys/arch/loongson/dev/voyagerreg.h,v 1.2 2010/02/26 14:53:11 miod Exp $	*/

/*
 * Copyright (c) 2010 Miodrag Vallat.
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
 * Silicon Motion SM501/SM502 registers
 */

#define	SM5XX_MMIO_BASE			0x000000

#define	VOYAGER_SYSTEM_CONTROL		0x000000
#define	VSC_DPMS_VSYNC_DISABLE		0x80000000
#define	VSC_DPMS_HSYNC_DISABLE		0x40000000
#define	VSC_COLOR_CONVERSION_BUSY	0x10000000
#define	VSC_FIFO_EMPTY			0x00100000
#define	VSC_2DENGINE_BUSY		0x00080000
#define	VSC_2DENGINE_ABORT		0x00003000	/* not a typo */
#define	VOYAGER_MISC_CONTROL		0x000004
#define	VOYAGER_GPIOL_CONTROL		0x000008
#define	VOYAGER_GPIOH_CONTROL		0x00000c
#define	VOYAGER_DRAM_CONTROL		0x000010
#define	VOYAGER_ARB_CONTROL		0x000014

#define	VOYAGER_COMMANDLIST_CONTROL	0x000018
#define	VOYAGER_COMMANDLIST_CONDITION	0x00001c
#define	VOYAGER_COMMANDLIST_RETURN	0x000020
#define	VOYAGER_COMMANDLIST_STATUS	0x000024

#define	VOYAGER_RAW_ISR			0x000028
#define	VOYAGER_RAWINTR_ZV1				6
#define	VOYAGER_RAWINTR_USB_PLUGIN			5
#define	VOYAGER_RAWINTR_ZV0				4
#define	VOYAGER_RAWINTR_CRT_VSYNC			3
#define	VOYAGER_RAWINTR_USB_SLAVE			2
#define	VOYAGER_RAWINTR_PANEL_VSYNC			1
#define	VOYAGER_RAWINTR_COMMAND_INTERPRETER		0
#define	VOYAGER_RAW_ICR			0x000028
#define	VOYAGER_ISR			0x00002c
#define	VOYAGER_INTR_USB_PLUGIN				31
#define	VOYAGER_INTR_GPIO54				30
#define	VOYAGER_INTR_GPIO53				29
#define	VOYAGER_INTR_GPIO52				28
#define	VOYAGER_INTR_GPIO51				27
#define	VOYAGER_INTR_GPIO50				26
#define	VOYAGER_INTR_GPIO49				25
#define	VOYAGER_INTR_GPIO48				24
#define	VOYAGER_INTR_I2C				23
#define	VOYAGER_INTR_PWM				22
#define	VOYAGER_INTR_DMA				20
#define	VOYAGER_INTR_PCI				19
#define	VOYAGER_INTR_I2S				18
#define	VOYAGER_INTR_AC97				17
#define	VOYAGER_INTR_USB_SLAVE				16
#define	VOYAGER_INTR_UART1				13
#define	VOYAGER_INTR_UART0				12
#define	VOYAGER_INTR_CRT_VSYNC				11
#define	VOYAGER_INTR_8051				10
#define	VOYAGER_INTR_SSP1				9
#define	VOYAGER_INTR_SSP0				8
#define	VOYAGER_INTR_USB_HOST				6
#define	VOYAGER_INTR_ZV1				4
#define	VOYAGER_INTR_2DENGINE				3
#define	VOYAGER_INTR_ZV0				2
#define	VOYAGER_INTR_PANEL_VSYNC			1
#define	VOYAGER_INTR_COMMAND_INTERPRETER		0
#define	VOYAGER_IMR			0x000030
#define	VOYAGER_DEBUG			0x000034

#define	VOYAGER_PM_CURRENT_GATE		0x000038
#define	VOYAGER_PM_CURRENT_CLOCK	0x00003c
#define	VOYAGER_PM_MODE0_GATE		0x000040
#define	VOYAGER_PM_MODE0_CLOCK		0x000044
#define	VOYAGER_PM_MODE1_GATE		0x000048
#define	VOYAGER_PM_MODE1_CLOCK		0x00004c
#define	VOYAGER_PM_SLEEP_GATE		0x000050
#define	VOYAGER_PM_CONTROL		0x000054

#define	VOYAGER_MASTER_PCI_BASE		0x000058
#define	VOYAGER_ENDIAN_CONTROL		0x00005c
#define	VOYAGER_DEVICE_ID		0x000060
#define	VOYAGER_PLL_COUNT		0x000064
#define	VOYAGER_MISC			0x000068
#define	VOYAGER_SDRAM_CLOCK		0x00006c
#define	VOYAGER_NON_CACHE_ADDRESS	0x000070
#define	VOYAGER_PLL_CONTROL		0x000074

/*
 * GPIO
 */

#define	VOYAGER_GPIO_DATA_LOW		0x010000
#define	VOYAGER_GPIO_DATA_HIGH		0x010004
#define	VOYAGER_GPIO_DIR_LOW		0x010008
#define	VOYAGER_GPIO_DIR_HIGH		0x01000c

/*
 * OHCI
 */

#define	VOYAGER_OHCI_BASE		0x040000
#define	VOYAGER_OHCI_SIZE		0x020000
