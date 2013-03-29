/*
 * Copyright (C) 2013 Sosuke Tokunaga <sosuke.tokunaga@courier-systems.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the Lisence, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be helpful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
 * GNU General Public License for more details.
 *
 */

#include <config.h>
#include <version.h>
#include <asm/processor.h>
#include <asm/macro.h>

.macro	dbwait
	mov.l	DBWAIT_A,	r0
	synco
	mov.l	@r0,		r1
	synco
.endm

	.global	lowlevel_init

	.text
	.align	2

lowlevel_init:

	/* WDT */
	write32	WDTCSR_A,	WDTCSR_D

	/* MMU */
	write32 MMUCR_A,	MMUCR_D

	/* CPG */
	write32 FRQCR2_A,       FRQCR2_D
        write32 FRQCR0_A,       FRQCR0_D

        bra     init_lbsc
        nop

        .align  4

WDTCSR_A:       .long   0xFFCC0004
MMUCR_A:        .long   0xFF000010
FRQCR0_A:       .long   0xFFC80000
FRQCR2_A:       .long   0xFFC80008

WDTCSR_D:       .long   0xA5000000
MMUCR_D:        .long   0x00000004
FRQCR0_D:       .long   0xCF000001
FRQCR2_D:       .long   0x00000000

        .align  2

init_lbsc:

        write32 CS0CTRL_A,      CS0CTRL_D
        write32 CS0CTRL2_A,     CS0CTRL2_D
        write32 CSPWCR0_A,      CSPWCR0_D
        write32 CSWCR0_A,       CSWCR0_D

        bra     init_dbsc3
        nop

        .align  4

CS0CTRL_A:      .long   0xFF800200
CS0CTRL2_A:     .long   0xFF800220
CSWCR0_A:       .long   0xFF800230
CSPWCR0_A:      .long   0xFF800280

CS0CTRL_D:      .long   0x00000020
CS0CTRL2_D:     .long   0x00004000 /* Flash 16MB */
#if defined(CONFIG_400MHZ_MODE)
CSWCR0_D:       .long   0x02120114
#else
CSWCR0_D:       .long   0x01120104
#endif /* defined(CONFIG_400MHZ_MODE) */
CSPWCR0_D:      .long   0x00000000

        .align  2

init_dbsc3:

	write32 DBPDCNT0_A,     DBPDCNT0_D
	write32 DBPDCNT1_A,     DBPDCNT1_D
	write32 DBPDCNT3_A,     DBPDCNT3_D0
	write32 DBCMD_A,        DBCMD_D_WAIT_200US
	dbwait
        write32 DBPDCNT3_A,     DBPDCNT3_D1
        write32 DBCMD_A,        DBCMD_D_WAIT_32CLK
        dbwait
        write32 DBPDCNT3_A,     DBPDCNT3_D2
        write32 DBCMD_A,        DBCMD_D_WAIT_100US
        dbwait
        write32 DBPDCNT3_A,     DBPDCNT3_D3
        write32 DBCMD_A,        DBCMD_D_WAIT_16CLK
        dbwait
        write32 DBPDCNT3_A,     DBPDCNT3_D4
        write32 DBCMD_A,        DBCMD_D_WAIT_200US
        dbwait
        write32 DBPDCNT3_A,     DBPDCNT3_D5
        write32 DBCMD_A,        DBCMD_D_WAIT_1CLK
        dbwait
        write32 DBPDCNT3_A,     DBPDCNT3_D6
        write32 DBCMD_A,        DBCMD_D_WAIT_10KCLK
        dbwait

        write32 DBKIND_A,       DBKIND_D
        write32 DBCONF_A,       DBCONF_D

        write32 DBTR0_A,        DBTR0_D
        write32 DBTR1_A,        DBTR1_D
        write32 DBTR2_A,        DBTR2_D
        write32 DBTR3_A,        DBTR3_D
        write32 DBTR4_A,        DBTR4_D
        write32 DBTR5_A,        DBTR5_D
        write32 DBTR6_A,        DBTR6_D
        write32 DBTR7_A,        DBTR7_D
        write32 DBTR8_A,        DBTR8_D
        write32 DBTR9_A,        DBTR9_D
        write32 DBTR10_A,       DBTR10_D
        write32 DBTR11_A,       DBTR11_D
        write32 DBTR12_A,       DBTR12_D
        write32 DBTR13_A,       DBTR13_D
        write32 DBTR14_A,       DBTR14_D
        write32 DBTR15_A,       DBTR15_D
        write32 DBTR16_A,       DBTR16_D
        write32 DBTR17_A,       DBTR17_D
        write32 DBTR18_A,       DBTR18_D
        write32 DBTR19_A,       DBTR19_D
        
        write32 DBBL_A,         DBBL_D
        write32 DBRNK0_A,       DBRNK0_D

        write32 DBCMD_A,        DBCMD_D_PDXT_400NS
        write32 DBCMD_A,        DBCMD_D_PREA
        write32 DBCMD_A,        DBCMD_D_MRS2
        write32 DBCMD_A,        DBCMD_D_MRS3
        write32 DBCMD_A,        DBCMD_D_MRS1_0
        write32 DBCMD_A,        DBCMD_D_MRS0_0
        write32 DBCMD_A,        DBCMD_D_PREA
        write32 DBCMD_A,        DBCMD_D_REF
        write32 DBCMD_A,        DBCMD_D_REF
        write32 DBCMD_A,        DBCMD_D_MRS0_1
        write32 DBCMD_A,        DBCMD_D_WAIT_200CLK
        write32 DBCMD_A,        DBCMD_D_MRS1_1
        write32 DBCMD_A,        DBCMD_D_MRS1_2

        write32 DBRFCNF0_A,     DBRFCNF0_D
        write32 DBRFCNF1_A,     DBRFCNF1_D
        write32 DBRFCNF2_A,     DBRFCNF2_D
        write32 DBRFEN_A,       DBRFEN_D
        write32 DBACEN_A,       DBACEN_D
        dbwait

        /* dummy read */
        mov.l   SDRAM_A,        r1
        synco
        mov.l   @r1,            r0
        synco

        bra     lowlevel_activate
        nop

        .align  4

DBACEN_A:       .long   0xFE800010
DBRFEN_A:       .long   0xFE800014
DBCMD_A:        .long   0xFE800018
DBWAIT_A:       .long   0xFE80001C
DBKIND_A:       .long   0xFE800020
DBCONF_A:       .long   0xFE800024
DBTR0_A:        .long   0xFE800040
DBTR1_A:        .long   0xFE800044
DBTR2_A:        .long   0xFE800048
DBTR3_A:        .long   0xFE800050
DBTR4_A:        .long   0xFE800054
DBTR5_A:        .long   0xFE800058
DBTR6_A:        .long   0xFE80005C
DBTR7_A:        .long   0xFE800060
DBTR8_A:        .long   0xFE800064
DBTR9_A:        .long   0xFE800068
DBTR10_A:       .long   0xFE80006C
DBTR11_A:       .long   0xFE800070
DBTR12_A:       .long   0xFE800074
DBTR13_A:       .long   0xFE800078
DBTR14_A:       .long   0xFE80007C
DBTR15_A:       .long   0xFE800080
DBTR16_A:       .long   0xFE800084
DBTR17_A:       .long   0xFE800088
DBTR18_A:       .long   0xFE80008C
DBTR19_A:       .long   0xFE800090
DBBL_A:         .long   0xFE8000B0
DBRFCNF0_A:     .long   0xFE8000E0
DBRFCNF1_A:     .long   0xFE8000E4
DBRFCNF2_A:     .long   0xFE8000E8
DBRNK0_A:       .long   0xFE800100
DBPDCNT0_A:     .long   0xFE800200
DBPDCNT1_A:     .long   0xFE800204
DBPDCNT3_A:     .long   0xFE80020C
SDRAM_A:        .long   0x0C000000

DBACEN_D:       .long   0x000000001
DBRFEN_D:       .long   0x000000001
DBCMD_D_WAIT_1CLK:      .long   1
DBCMD_D_WAIT_16CLK:     .long   16
DBCMD_D_WAIT_32CLK:     .long   32
DBCMD_D_WAIT_200CLK:    .long   200
DBCMD_D_WAIT_10KCLK:    .long   10000
#if defined(CONFIG_400MHZ_MODE)
DBCMD_D_WAIT_100US:     .long   20000
DBCMD_D_WAIT_200US:     .long   40000
DBCMD_D_PDXT_400NS:     .long   0x11000050
#else
DBCMD_D_WAIT_100US:     .long   26650
DBCMD_D_WAIT_200US:     .long   53300
DBCMD_D_PDXT_400NS:     .long   0x1100006B
#endif /* defined(CONFIG_400MHZ_MODE) */
DBCMD_D_PREA:   .long   0x0B000000
DBCMD_D_REF:    .long   0x0C000000
#if defined(CONFIG_400MHZ_MODE)
DBCMD_D_MRS0_0: .long   0x28000533
DBCMD_D_MRS0_1: .long   0x28000433
#else
DBCMD_D_MRS0_0: .long   0x28000744
DBCMD_D_MRS0_1: .long   0x28000644
#endif /* defined(CONFIG_400MHZ_MODE) */
DBCMD_D_MRS1_0: .long   0x29000004
DBCMD_D_MRS1_1: .long   0x29000384
DBCMD_D_MRS1_2: .long   0x29000004
DBCMD_D_MRS2:   .long   0x2A000000
DBCMD_D_MRS3:   .long   0x2B000000
DBKIND_D:       .long   0x00000005 /* DDR2-SDRAM */
DBCONF_D:       .long   0x0D020A01 /* 4banks, row:13bits, col:10bits */
#if defined(CONFIG_400MHZ_MODE)
DBTR0_D:        .long   0x00000003
DBTR1_D:        .long   0x00000002
DBTR2_D:        .long   0x00000000
DBTR3_D:        .long   0x00000003
DBTR4_D:        .long   0x00050003
DBTR5_D:        .long   0x0000000B
DBTR6_D:        .long   0x00000008
DBTR7_D:        .long   0x00000002
DBTR8_D:        .long   0x0000000A
DBTR9_D:        .long   0x00000002
DBTR10_D:       .long   0x00000003
DBTR11_D:       .long   0x00000008
DBTR12_D:       .long   0x00000008
DBTR13_D:       .long   0x0000001A
DBTR14_D:       .long   0x00070002
DBTR15_D:       .long   0x00000003
DBTR16_D:       .long   0x12091001
DBTR17_D:       .long   0x00030000
DBTR18_D:       .long   0x02000207
DBTR19_D:       .long   0x00000040
#else
DBTR0_D:        .long   0x00000004
DBTR1_D:        .long   0x00000003
DBTR2_D:        .long   0x00000000
DBTR3_D:        .long   0x00000004
DBTR4_D:        .long   0x00050004
DBTR5_D:        .long   0x0000000F
DBTR6_D:        .long   0x0000000B
DBTR7_D:        .long   0x00000002
DBTR8_D:        .long   0x0000000D
DBTR9_D:        .long   0x00000002
DBTR10_D:       .long   0x00000004
DBTR11_D:       .long   0x00000008
DBTR12_D:       .long   0x00000009
DBTR13_D:       .long   0x00000022
DBTR14_D:       .long   0x00070002
DBTR15_D:       .long   0x00000003
DBTR16_D:       .long   0x120A1001
DBTR17_D:       .long   0x00040000
DBTR18_D:       .long   0x02010200
DBTR19_D:       .long   0x00000040
#endif /* defined(CONFIG_400MHZ_MODE) */
DBBL_D:         .long   0x00000000
DBRFCNF0_D:     .long   0x000001FF
#if defined(CONFIG_400MHZ_MODE)
DBRFCNF1_D:     .long   0x00000618
#else
DBRFCNF1_D:     .long   0x00000805
#endif /* defined(CONFIG_400MHZ_MODE) */
DBRFCNF2_D:     .long   0x00000000
DBRNK0_D:       .long   0x00000001
DBPDCNT0_D:     .long   0x00010200
DBPDCNT1_D:     .long   0x00000014
DBPDCNT3_D0:    .long   0x800F0000
DBPDCNT3_D1:    .long   0x800F1000
DBPDCNT3_D2:    .long   0x820F1000
DBPDCNT3_D3:    .long   0x860F1000
DBPDCNT3_D4:    .long   0x870F1000
DBPDCNT3_D5:    .long   0x870F3000
DBPDCNT3_D6:    .long   0x870F7000

        .align  2

lowlevel_activate:

        /* CACHE */
        write32 CCR_A,          CCR_D

        /* Enable Interrupts */
        stc     sr,             r0
        mov.l   SR_MASK_D,      r1
        and     r1,             r0
        ldc     r0,             sr

        rts
        nop

        .align  4

CCR_A:          .long   CCR
CCR_D:          .long   CCR_CACHE_INIT
SR_MASK_D:      .long   0xEFFFFF0F

