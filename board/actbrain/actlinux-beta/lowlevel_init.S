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
	read32	SDRAM_DUMMY_A
	read32	SDRAM_A0
	read32	SDRAM_A1
	read32	SDRAM_A2
	read32	SDRAM_A3
	write32	DBCMDCNT_A,	DBCMDCNT_D0
	write32	DBCMDCNT_A,	DBCMDCNT_D1
	write32	DBPDCNT0_A,	DBPDCNT0_D1
	write32	DBRFPDN0_A,	DBRFPDN0_D

	/* MMU, PMB */
	write32	PASCR_A,	PASCR_29BIT_D
	write32 MMUCR_A,	MMUCR_D
	/***********************************************************
	 * ent	virt		phys		sz	c	wt
	 * 0	0x80000000	0x40000000	512MB	1	1
	 * 1	0xA0000000	0x00000000	64MB	0	0
	 * 2	0xA4000000	0x04000000	64MB	0	0
	 * 3	0xA8000000	0x08000000	128MB	0	0
	 * 4	0xB0000000	0x10000000	128MB	0	0
	 * 5	0xB8000000	0x18000000	64MB	0	0
	 * 6	0xBC000000	0x00000000	64MB	0	0
	 */
	write32 PMB_ADDR_A0,	PMB_ADDR_80_D
	write32	PMB_DATA_A0,	PMB_DATA_40_512M_C_D
	write32	PMB_ADDR_A1,	PMB_ADDR_A0_D
	write32	PMB_DATA_A1,	PMB_DATA_00_64M_U_D
	write32 PMB_ADDR_A2,	PMB_ADDR_A4_D
	write32	PMB_DATA_A2,	PMB_DATA_04_64M_U_D
	write32 PMB_ADDR_A3,	PMB_ADDR_A8_D
	write32	PMB_DATA_A3,	PMB_DATA_08_128M_U_D
	write32	PMB_ADDR_A4,	PMB_ADDR_B0_D
	write32	PMB_DATA_A4,	PMB_DATA_10_128M_U_D
	write32	PMB_ADDR_A5,	PMB_ADDR_B8_D
	write32	PMB_DATA_A5,	PMB_DATA_18_64M_U_D
	write32	PMB_ADDR_A6,	PMB_ADDR_BC_D
	write32	PMB_DATA_A6,	PMB_DATA_00_64M_U_D
	write32	PMB_ADDR_A7,	PMB_ADDR_UNUSED_D
	write32	PMB_ADDR_A8,	PMB_ADDR_UNUSED_D
	write32	PMB_ADDR_A9,	PMB_ADDR_UNUSED_D
	write32	PMB_ADDR_A10,	PMB_ADDR_UNUSED_D
	write32	PMB_ADDR_A11,	PMB_ADDR_UNUSED_D
	write32	PMB_ADDR_A12,	PMB_ADDR_UNUSED_D
	write32	PMB_ADDR_A13,	PMB_ADDR_UNUSED_D
	write32	PMB_ADDR_A14,	PMB_ADDR_UNUSED_D
	write32	PMB_ADDR_A15,	PMB_ADDR_UNUSED_D
	write32	PASCR_A,	PASCR_32BIT_D
	mov.l	PMB_DUMMY_A,	r0
	icbi	@r0

        /* CACHE */
        write32 CCR_A,          CCR_D

        /* Enable Interrupts */
        stc     sr,             r0
        mov.l   SR_MASK_D,      r1
        and     r1,             r0
        ldc     r0,             sr

        rts
        nop

        .align  2

/* WDT */
RWTCSR_A:	.long	RWTCSR
RWTCSR_D:	.long	0x0000A507

/* CPG */
PLLCR_A:	.long	PLLCR
PLLCR_D:	.long	0x00004000
FRQCRB_A:	.long	FRQCRB
FRQCRB_D:	.long	0x00000030
FRQCRA_A:	.long	FRQCRA
FRQCRA_D:	.long	0x8E003508
LSTATS_A:	.long	0xA4150060
LSTATS_D:	.long	0x00000001

/* BSC */
MMSELR_A:	.long	MMSELR
MMSELR_D:	.long	0xA5A50000
CMNCR_A:	.long	CMNCR
CMNCR_D:	.long	0x0000001A
CS0BCR_A:	.long	CS0BCR
CS0BCR_D:	.long	0x36DB0400
CS6ABCR_A:	.long	CS6ABCR
CS6ABCR_D:	.long	0x36DB0400
CS6BBCR_A:	.long	CS6BBCR
CS6BBCR_D:	.long	0x36DB0600    /* 32bit yamazaki*/
CS0WCR_A:	.long	CS0WCR
CS0WCR_D:	.long	0x00001BC2
CS6AWCR_A:	.long	CS6AWCR
CS6AWCR_D:	.long	0x00001BC2
CS6BWCR_A:	.long	CS6BWCR
CS6BWCR_D:	.long	0x00001D43

/* DBSC */
DBSTATE_A:	.long	DBSTATE
DBWAIT_200US:	.long	4167
DBWAIT_400NS:	.long	9
DBWAIT_200CLK:	.long	40
DBPDCNT0_A:	.long	DBPDCNT0
DBPDCNT0_D0:	.long	0x00000181	/* ODT=75ohm */
DBPDCNT0_D1:	.long	0x00000080
DBCONF_A:	.long	DBCONF
DBCONF_D:	.long	0x01630002	/* <- 0x015B0002 yamazaki */
DBTR0_A:	.long	DBTR0
DBTR0_D:	.long	0x03071F02	/* <- 0x03061502 yamazaki */
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
SDRAM_DUMMY_A:	.long	0x0C400000
SDRAM_A0:	.long	0xA8000000
SDRAM_A1:	.long	0xA8000004
SDRAM_A2:	.long	0xA8000008
SDRAM_A3:	.long	0xA800000C

/* MMU */
MMUCR_A:	.long	MMUCR
MMUCR_D:	.long	0x00000004
PASCR_A:	.long	PASCR
PASCR_29BIT_D:	.long	0x00000000
PASCR_32BIT_D:	.long	0x80000080

/* PMB */
PMB_ADDR_A0:		.long	PMB_ADDR_BASE(0)
PMB_ADDR_A1:		.long	PMB_ADDR_BASE(1)
PMB_ADDR_A2:		.long	PMB_ADDR_BASE(2)
PMB_ADDR_A3:		.long	PMB_ADDR_BASE(3)
PMB_ADDR_A4:		.long	PMB_ADDR_BASE(4)
PMB_ADDR_A5:		.long	PMB_ADDR_BASE(5)
PMB_ADDR_A6:		.long	PMB_ADDR_BASE(6)
PMB_ADDR_A7:		.long	PMB_ADDR_BASE(7)
PMB_ADDR_A8:		.long	PMB_ADDR_BASE(8)
PMB_ADDR_A9:		.long	PMB_ADDR_BASE(9)
PMB_ADDR_A10:		.long	PMB_ADDR_BASE(10)
PMB_ADDR_A11:		.long	PMB_ADDR_BASE(11)
PMB_ADDR_A12:		.long	PMB_ADDR_BASE(12)
PMB_ADDR_A13:		.long	PMB_ADDR_BASE(13)
PMB_ADDR_A14:		.long	PMB_ADDR_BASE(14)
PMB_ADDR_A15:		.long	PMB_ADDR_BASE(15)
PMB_DATA_A0:		.long	PMB_DATA_BASE(0)
PMB_DATA_A1:		.long	PMB_DATA_BASE(1)
PMB_DATA_A2:		.long	PMB_DATA_BASE(2)
PMB_DATA_A3:		.long	PMB_DATA_BASE(3)
PMB_DATA_A4:		.long	PMB_DATA_BASE(4)
PMB_DATA_A5:		.long	PMB_DATA_BASE(5)
PMB_DATA_A6:		.long	PMB_DATA_BASE(6)
PMB_DUMMY_A:		.long	0xA0000000
/*				mk_pmb_addr_val(vpn) */
PMB_ADDR_80_D:		.long	mk_pmb_addr_val(0x80)
PMB_ADDR_A0_D:		.long	mk_pmb_addr_val(0xA0)
PMB_ADDR_A4_D:		.long	mk_pmb_addr_val(0xA4)
PMB_ADDR_A8_D:		.long	mk_pmb_addr_val(0xA8)
PMB_ADDR_B0_D:		.long	mk_pmb_addr_val(0xB0)
PMB_ADDR_B8_D:		.long	mk_pmb_addr_val(0xB8)
PMB_ADDR_BC_D:		.long	mk_pmb_addr_val(0xBC)
PMB_ADDR_UNUSED_D:	.long	0x00000000
/*				mk_pmb_data_val(ppn, ub, v, s1, s0, c, wt) */
PMB_DATA_40_512M_C_D:	.long	mk_pmb_data_val(0x40, 0, 1,  1,  1, 1,  1)
PMB_DATA_00_64M_U_D:	.long	mk_pmb_data_val(0x00, 0, 1,  0,  1, 0,  0)
PMB_DATA_04_64M_U_D:	.long	mk_pmb_data_val(0x04, 0, 1,  0,  1, 0,  0)
PMB_DATA_08_128M_U_D:	.long	mk_pmb_data_val(0x08, 0, 1,  1,  0, 0,  0)
PMB_DATA_10_128M_U_D:	.long	mk_pmb_data_val(0x10, 0, 1,  1,  0, 0,  0)
PMB_DATA_18_64M_U_D:	.long	mk_pmb_data_val(0x18, 0, 1,  0,  1, 0,  0)

/* CACHE */
CCR_A:          .long   CCR
CCR_D:          .long   0x0000090B /* Write Through for ethernet */

/* SR */
SR_MASK_D:      .long   0xEFFFFF0F

