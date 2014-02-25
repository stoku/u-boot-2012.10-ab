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

.macro	wait_while, addr, data
	mov.l	\data,	r2
	mov.l	\addr,	r1
1:
	nop
	mov.l	@r1,	r0
	synco
	tst	r0,	r2
	bf	1b
	nop
.endm

.macro	dbwait, count
	mov.l	\count,		r3
	mov.l	DBSTATE_A,	r1
1:
	nop
	mov.l	@r1,	r0
	synco
	tst	r3,	r3
	bf/s	1b
	dt	r3
.endm

.macro read32, addr
	mov.l	\addr,	r1
	mov.l	@r1,	r0
	synco
.endm

	.global	lowlevel_init

	.text
	.align	2

lowlevel_init:

	/* WDT */
	write16	RWTCSR_A,	RWTCSR_D

	/* MMU */
	write32 MMUCR_A,	MMUCR_D

	/* CPG */
	write32 PLLCR_A,	PLLCR_D
	write32 FRQCRB_A,       FRQCRB_D
        write32 FRQCRA_A,       FRQCRA_D
	wait_while	LSTATS_A,	LSTATS_D

	/* BSC */
	write32	MMSELR_A,	MMSELR_D
	write32	CMNCR_A,	CMNCR_D
	write32	CS0BCR_A,	CS0BCR_D
	write32 CS6ABCR_A,	CS6ABCR_D
	write32	CS6BBCR_A,	CS6BBCR_D
	write32	CS0WCR_A,	CS0WCR_D
	write32	CS6AWCR_A,	CS6AWCR_D
	write32	CS6BWCR_A,	CS6BWCR_D

	/* DBSC */
	dbwait	DBWAIT_200US
	write32	DBPDCNT0_A,	DBPDCNT0_D0
	write32	DBCONF_A,	DBCONF_D
	write32	DBTR0_A,	DBTR0_D
	write32	DBTR1_A,	DBTR1_D
	write32	DBTR2_A,	DBTR2_D
	write32	DBTR3_A,	DBTR3_D
	write32 DBKIND_A,	DBKIND_D
	write32	DBCKECNT_A,	DBCKECNT_D
	dbwait	DBWAIT_400NS
	write32	DBCMDCNT_A,	DBCMDCNT_D0
	write32	DBMRCNT_A,	DBMRCNT_D0
	write32	DBMRCNT_A,	DBMRCNT_D1
	write32	DBMRCNT_A,	DBMRCNT_D2
	write32	DBMRCNT_A,	DBMRCNT_D3
	write32	DBCMDCNT_A,	DBCMDCNT_D0
	write32	DBCMDCNT_A,	DBCMDCNT_D1
	write32	DBCMDCNT_A,	DBCMDCNT_D1
	write32	DBMRCNT_A,	DBMRCNT_D4
	dbwait	DBWAIT_200CLK
	write32	DBEN_A,		DBEN_D
	write32	DBRFPDN1_A,	DBRFPDN1_D
	write32	DBRFPDN2_A,	DBRFPDN2_D
	write32	DBCMDCNT_A,	DBCMDCNT_D0
	read32	DUMMY_A
	read32	SDRAM_A0
	read32	SDRAM_A1
	read32	SDRAM_A2
	read32	SDRAM_A3
	write32	DBCMDCNT_A,	DBCMDCNT_D0
	write32	DBCMDCNT_A,	DBCMDCNT_D1
	write32	DBPDCNT0_A,	DBPDCNT0_D1
	write32	DBRFPDN0_A,	DBRFPDN0_D

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

RWTCSR_A:	.long	RWTCSR
RWTCSR_D:	.long	0x0000A507
MMUCR_A:	.long	MMUCR
MMUCR_D:	.long	0x00000004
PLLCR_A:	.long	PLLCR
PLLCR_D:	.long	0x00004000
FRQCRB_A:	.long	FRQCRB
FRQCRB_D:	.long	0x00000030
FRQCRA_A:	.long	FRQCRA
FRQCRA_D:	.long	0x8E003508
LSTATS_A:	.long	0xA4150060
LSTATS_D:	.long	0x00000001
MMSELR_A:	.long	MMSELR
MMSELR_D:	.long	0xA5A50000
CMNCR_A:	.long	CMNCR
CMNCR_D:	.long	0x0000001A
CS0BCR_A:	.long	CS0BCR
CS0BCR_D:	.long	0x36DB0400
CS6ABCR_A:	.long	CS6ABCR
CS6ABCR_D:	.long	0x36DB0400
CS6BBCR_A:	.long	CS6BBCR
CS6BBCR_D:	.long	0x36DB0400
CS0WCR_A:	.long	CS0WCR
CS0WCR_D:	.long	0x00001BC2
CS6AWCR_A:	.long	CS6AWCR
CS6AWCR_D:	.long	0x00001BC2
CS6BWCR_A:	.long	CS6BWCR
CS6BWCR_D:	.long	0x00001D43
DBSTATE_A:	.long	DBSTATE
DBWAIT_200US:	.long	4167
DBWAIT_400NS:	.long	9
DBWAIT_200CLK:	.long	40
DBPDCNT0_A:	.long	DBPDCNT0
DBPDCNT0_D0:	.long	0x00000181	/* ODT=75ohm */
DBPDCNT0_D1:	.long	0x00000080
DBCONF_A:	.long	DBCONF
DBCONF_D:	.long	0x015B0002
DBTR0_A:	.long	DBTR0
DBTR0_D:	.long	0x03061502
DBTR1_A:	.long	DBTR1
DBTR1_D:	.long	0x02020102
DBTR2_A:	.long	DBTR2
DBTR2_D:	.long	0x01090305
DBTR3_A:	.long	DBTR3
DBTR3_D:	.long	0x00000002
DBKIND_A:	.long	DBKIND
DBKIND_D:	.long	0x00000005
DBCKECNT_A:	.long	DBCKECNT
DBCKECNT_D:	.long	0x00000001
DBCMDCNT_A:	.long	DBCMDCNT
DBCMDCNT_D0:	.long	0x00000002	/* PALL	*/
DBCMDCNT_D1:	.long	0x00000004	/* REF	*/
DBMRCNT_A:	.long	DBMRCNT
DBMRCNT_D0:	.long	0x00020000	/* EMR2	*/
DBMRCNT_D1:	.long	0x00030000	/* EMR3	*/
DBMRCNT_D2:	.long	0x00010040	/* EMR (ODT=150ohm)	*/
DBMRCNT_D3:	.long	0x00000532	/* MR (DLL Reset=Yes)	*/
DBMRCNT_D4:	.long	0x00000432	/* MR (DLL Reset=No)	*/
DBEN_A:		.long	DBEN
DBEN_D:		.long	0x00000001
DBRFPDN0_A:	.long	DBRFPDN0
DBRFPDN0_D:	.long	0x00010000
DBRFPDN1_A:	.long	DBRFPDN1
DBRFPDN1_D:	.long	0x00000613
DBRFPDN2_A:	.long	DBRFPDN2
DBRFPDN2_D:	.long	0x238C003A
DUMMY_A:	.long	0x0C400000
SDRAM_A0:	.long	0xA8000000
SDRAM_A1:	.long	0xA8000004
SDRAM_A2:	.long	0xA8000008
SDRAM_A3:	.long	0xA800000C
CCR_A:          .long   CCR
CCR_D:          .long   0x0000090B /* Write Through for ethernet */
SR_MASK_D:      .long   0xEFFFFF0F

