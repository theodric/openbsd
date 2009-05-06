/*	$OpenBSD: src/sys/dev/pci/cmpcireg.h,v 1.4 2009/05/06 22:25:57 jakemsr Exp $	*/
/*	$NetBSD: cmpcireg.h,v 1.7 2005/12/11 12:22:48 christos Exp $	*/

/*
 * Copyright (c) 2000, 2001 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Takuya SHIOZAKI <tshiozak@NetBSD.org> .
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by ITOH Yasufumi.
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/* C-Media CMI8x38 Audio Chip Support */

#ifndef _DEV_PCI_CMPCIREG_H_
#define _DEV_PCI_CMPCIREG_H_

/*
 * PCI Configuration Registers
 */

#define CMPCI_PCI_IOBASEREG	(PCI_MAPREG_START)


/*
 * I/O Space
 */

#define CMPCI_REG_FUNC_0		0x00
#  define CMPCI_REG_CH0_DIR		0x00000001
#  define CMPCI_REG_CH1_DIR		0x00000002
#  define CMPCI_REG_CH0_PAUSE		0x00000004
#  define CMPCI_REG_CH1_PAUSE		0x00000008
#  define CMPCI_REG_CH0_ENABLE		0x00010000
#  define CMPCI_REG_CH1_ENABLE		0x00020000
#  define CMPCI_REG_CH0_RESET		0x00040000
#  define CMPCI_REG_CH1_RESET		0x00080000

#define CMPCI_REG_FUNC_1		0x04
#  define CMPCI_REG_JOY_ENABLE		0x00000002
#  define CMPCI_REG_UART_ENABLE		0x00000004
#  define CMPCI_REG_LEGACY_ENABLE	0x00000008
#  define CMPCI_REG_BREQ		0x00000010
#  define CMPCI_REG_MCBINTR_ENABLE	0x00000020
#  define CMPCI_REG_SPDIFOUT_DAC	0x00000040
#  define CMPCI_REG_SPDIF_LOOP		0x00000080
#  define CMPCI_REG_SPDIF0_ENABLE	0x00000100
#  define CMPCI_REG_SPDIF1_ENABLE	0x00000200
#  define CMPCI_REG_DAC_FS_SHIFT	10
#  define CMPCI_REG_DAC_FS_MASK		0x00000007
#  define CMPCI_REG_ADC_FS_SHIFT	13
#  define CMPCI_REG_ADC_FS_MASK		0x00000007

#define CMPCI_REG_CHANNEL_FORMAT	0x08
#  define CMPCI_REG_SPDIN_PHASE		0x80
#  define CMPCI_REG_CH0_FORMAT_SHIFT	0
#  define CMPCI_REG_CH0_FORMAT_MASK	0x00000003
#  define CMPCI_REG_CH1_FORMAT_SHIFT	2
#  define CMPCI_REG_CH1_FORMAT_MASK	0x00000003
#  define CMPCI_REG_FORMAT_MONO		0x00000000
#  define CMPCI_REG_FORMAT_STEREO	0x00000001
#  define CMPCI_REG_FORMAT_8BIT		0x00000000
#  define CMPCI_REG_FORMAT_16BIT	0x00000002
#  define CMPCI_REG_DBL_SPD_RATE	0x00000040
#  define CMPCI_REG_SPDIF_24		0x00020000 /* 24-bit spdif, model 37 */
#  define CMPCI_REG_AC3EN1		0x00100000 /* enable AC3, model 37 */
#  define CMCPI_REG_CHIP_037		0x01000000
#  define CMPCI_REG_CHIP_MASK1		0x1f000000
#  define CMPCI_REG_CHB3D		0x20000000
#  define CMPCI_REG_CHB3D5C		0x80000000

#define CMPCI_REG_INTR_CTRL		0x0c
#  define CMPCI_REG_CH0_INTR_ENABLE	0x00010000
#  define CMPCI_REG_CH1_INTR_ENABLE	0x00020000
#  define CMPCI_REG_TDMA_INTR_ENABLE	0x00040000
#  define CMPCI_REG_CHIP_MASK2		0xff000000
#  define CMPCI_REG_CHIP_039		0x04000000
#  define CMPCI_REG_CHIP_039_6CH	0x01000000
#  define CMPCI_REG_CHIP_055		0x08000000
#  define CMPCI_REG_CHIP_8768		0x20000000

#define CMPCI_REG_INTR_STATUS		0x10
#  define CMPCI_REG_CH0_INTR		0x00000001
#  define CMPCI_REG_CH1_INTR		0x00000002
#  define CMPCI_REG_CH0_BUSY		0x00000004
#  define CMPCI_REG_CH1_BUSY		0x00000008
#  define CMPCI_REG_LEGACY_STEREO	0x00000010
#  define CMPCI_REG_LEGACY_HDMA		0x00000020
#  define CMPCI_REG_DMASTAT		0x00000040
#  define CMPCI_REG_XDO46		0x00000080
#  define CMPCI_REG_HTDMA_INTR		0x00004000
#  define CMPCI_REG_LTDMA_INTR		0x00008000
#  define CMPCI_REG_UART_INTR		0x00010000
#  define CMPCI_REG_MCB_INTR		0x04000000
#  define CMPCI_REG_VCO			0x08000000
#  define CMPCI_REG_ANY_INTR		0x80000000

#define CMPCI_REG_LEGACY_CTRL		0x14
#  define CMPCI_REG_LEGACY_SPDIF_ENABLE	0x00200000
#  define CMPCI_REG_SPDIF_COPYRIGHT	0x00400000
#  define CMPCI_REG_XSPDIF_ENABLE	0x00800000
#  define CMPCI_REG_FMSEL_SHIFT		24
#  define CMPCI_REG_FMSEL_MASK		0x00000003
#  define CMPCI_REG_VSBSEL_SHIFT	26
#  define CMPCI_REG_VSBSEL_MASK		0x00000003
#  define CMPCI_REG_VMPUSEL_SHIFT	29
#  define CMPCI_REG_VMPUSEL_MASK	0x00000003
#  define CMPCI_REG_CHB3D6C		0x00008000
#  define CMPCI_REG_NXCHG		0x80000000

#define CMPCI_REG_MISC			0x18
#  define CMPCI_REG_ENCENTER		0x00000080
#  define CMPCI_REG_2ND_SPDIFIN		0x00000100
#  define CMPCI_REG_SPDIFOUT_48K	0x00008000
#  define CMPCI_REG_AC3EN2		0x00040000 /* enable AC3, model 39 */
#  define CMPCI_REG_FM_ENABLE		0x00080000
#  define CMPCI_REG_SPDFLOOPI		0x00100000
#  define CMPCI_REG_SPD32SEL		0x00200000
#  define CMPCI_REG_XCHGDAC		0x00400000
#  define CMPCI_REG_ENDBDAC		0x00800000
#  define CMPCI_REG_SPDIF48K		0x01000000
#  define CMPCI_REG_5V			0x02000000
#  define CMPCI_REG_N4SPK3D		0x04000000


#define CMPCI_REG_SBDATA		0x22
#define CMPCI_REG_SBADDR		0x23
#  define CMPCI_SB16_MIXER_RESET	0x00
#  define CMPCI_SB16_MIXER_MASTER_L	0x30
#  define CMPCI_SB16_MIXER_MASTER_R	0x31
#  define CMPCI_SB16_MIXER_VOICE_L	0x32
#  define CMPCI_SB16_MIXER_VOICE_R	0x33
#  define CMPCI_SB16_MIXER_FM_L		0x34
#  define CMPCI_SB16_MIXER_FM_R		0x35
#  define CMPCI_SB16_MIXER_CDDA_L	0x36
#  define CMPCI_SB16_MIXER_CDDA_R	0x37
#  define CMPCI_SB16_MIXER_LINE_L	0x38
#  define CMPCI_SB16_MIXER_LINE_R	0x39
#  define CMPCI_SB16_MIXER_MIC		0x3A
#    define CMPCI_SB16_MIXER_VALBITS		5
#  define CMPCI_SB16_MIXER_SPEAKER	0x3B
#    define CMPCI_SB16_MIXER_SPEAKER_VALBITS	2
#  define CMPCI_SB16_MIXER_OUTMIX	0x3C
#    define CMPCI_SB16_SW_MIC		0x01
#    define CMPCI_SB16_SW_CD_R		0x02
#    define CMPCI_SB16_SW_CD_L		0x04
#    define CMPCI_SB16_SW_CD		(CMPCI_SB16_SW_CD_L|CMPCI_SB16_SW_CD_R)
#    define CMPCI_SB16_SW_LINE_R	0x08
#    define CMPCI_SB16_SW_LINE_L	0x10
#    define CMPCI_SB16_SW_LINE	(CMPCI_SB16_SW_LINE_L|CMPCI_SB16_SW_LINE_R)
#    define CMPCI_SB16_SW_FM_R		0x20
#    define CMPCI_SB16_SW_FM_L		0x40
#    define CMPCI_SB16_SW_FM		(CMPCI_SB16_SW_FM_L|CMPCI_SB16_SW_FM_R)
#  define CMPCI_SB16_MIXER_ADCMIX_L	0x3D
#  define CMPCI_SB16_MIXER_ADCMIX_R	0x3E
#    define CMPCI_SB16_MIXER_FM_SRC_R	0x20
#    define CMPCI_SB16_MIXER_LINE_SRC_R	0x08
#    define CMPCI_SB16_MIXER_CD_SRC_R	0x02
#    define CMPCI_SB16_MIXER_MIC_SRC	0x01
#    define CMPCI_SB16_MIXER_SRC_R_TO_L(v)	((v) << 1)

#  define CMPCI_SB16_MIXER_L_TO_R(addr) ((addr)+1)

#  define CMPCI_ADJUST_MIC_GAIN(sc, x)	cmpci_adjust((x), 0xf8)
#  define CMPCI_ADJUST_GAIN(sc, x)	cmpci_adjust((x), 0xf8)
#  define CMPCI_ADJUST_2_GAIN(sc, x)	cmpci_adjust((x), 0xc0)

#define CMPCI_REG_MIXER24		0x24
#  define CMPCI_REG_SPDIN_MONITOR	0x01
#  define CMPCI_REG_SURROUND		0x02
#  define CMPCI_REG_INDIVIDUAL		0x20
#  define CMPCI_REG_REVERSE_FR		0x10
#  define CMPCI_REG_FMMUTE		0x80
#  define CMPCI_REG_WSMUTE		0x40
#  define CMPCI_REG_WAVEINL		0x08
#  define CMPCI_REG_WAVEINR		0x04

#define CMPCI_REG_MIXER25		0x25
#  define CMPCI_REG_RAUXREN		0x80
#  define CMPCI_REG_RAUXLEN		0x40
#  define CMPCI_REG_VAUXRM		0x20	/* 0: mute, 1: unmute */
#  define CMPCI_REG_VAUXLM		0x10
#  define CMPCI_REG_VADMIC		0x0E
#  define CMPCI_REG_MICGAINZ		0x01	/* 1: disable preamp */

#  define CMPCI_ADJUST_ADMIC_GAIN(sc, x)	(cmpci_adjust((x), 0xe0) >> 5)
#  define CMPCI_REG_ADMIC_VALBITS	3
#  define CMPCI_REG_ADMIC_MASK		0x07
#  define CMPCI_REG_ADMIC_SHIFT		0x01

/* Note that the doc tells a lie */
#define CMPCI_REG_MIXER_AUX		0x26
#  define CMPCI_ADJUST_AUX_GAIN(sc, l, r)	\
	(cmpci_adjust((l), 0xc0) >> 4 | cmpci_adjust((r), 0xc0))
#  define CMPCI_REG_AUX_VALBITS	4

#define CMPCI_REG_MPU_BASE		0x40
#define CMPCI_REG_MPU_SIZE		0x10
#define CMPCI_REG_FM_BASE		0x50
#define CMPCI_REG_FM_SIZE		0x10

#define CMPCI_REG_DMA0_BASE		0x80
#define CMPCI_REG_DMA0_BYTES		0x84
#define CMPCI_REG_DMA0_SAMPLES		0x86
#define CMPCI_REG_DMA1_BASE		0x88
#define CMPCI_REG_DMA1_BYTES		0x8C
#define CMPCI_REG_DMA1_SAMPLES		0x8E

#define CMPCI_REG_8768_MISC		0x92
#define   CMPCI_REG_CHB3D8C		0x20

/* sample rate */
#define CMPCI_REG_RATE_5512		0
#define CMPCI_REG_RATE_11025		1
#define CMPCI_REG_RATE_22050		2
#define CMPCI_REG_RATE_44100		3
#define CMPCI_REG_RATE_8000		4
#define CMPCI_REG_RATE_16000		5
#define CMPCI_REG_RATE_32000		6
#define CMPCI_REG_RATE_48000		7
#define CMPCI_REG_NUMRATE		8

#endif /* _DEV_PCI_CMPCIREG_H_ */

/* end of file */
