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

#include <common.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <net.h>

#define WRITE_PFC(d, a)	\
	do { 	\
		writel(~(d), PMMR); \
		writel(d, a); \
	} while (0)

#define MSTPCR1			(0xFFC80034)
#define MSTPSR1			(0xFFC80044)
#define MSTPSR1_GETHER		(1 << 14)
#define MSTPSR1_DU		(1 << 3)

#define MAHR0			(0xFEE005C0)
#define MALR0			(0xFEE005C8)

#define MODEMR			(0xFFCC0020)
#define MODEMR_ENDIAN_LITTLE	(0x01 << 8)

/* DU base address */
#define DU_BASE			(0xFFF80000)
#define DU_REG(offset)		(DU_BASE + (offset))

/* DU display control */
#define DSYSR			DU_REG(0x0000)
#define DSMR			DU_REG(0x0004)
#define DSSR			DU_REG(0x0008)
#define DSRCR			DU_REG(0x000C)
#define DIER			DU_REG(0x0010)
#define CPCR			DU_REG(0x0014)
#define DPPR			DU_REG(0x0018)

/* DU display timing */
#define HDSR			DU_REG(0x0040)
#define HDER			DU_REG(0x0044)
#define VDSR			DU_REG(0x0048)
#define VDER			DU_REG(0x004C)
#define HCR			DU_REG(0x0050)
#define HSWR			DU_REG(0x0054)
#define VCR			DU_REG(0x0058)
#define VSPR			DU_REG(0x005C)
#define EQWR			DU_REG(0x0060)
#define SPWR			DU_REG(0x0064)
#define CLAMPSR			DU_REG(0x0070)
#define CLAMPWR			DU_REG(0x0074)
#define DESR			DU_REG(0x0078)
#define DEWR			DU_REG(0x007C)

/* DU display attributes */
#define DOOR			DU_REG(0x0090)
#define CDER			DU_REG(0x0094)
#define BPOR			DU_REG(0x0098)
#define RINTOFSR		DU_REG(0x009C)
#define COLOR_MASK		(0x00FCFCFC)

/* DU display planes */
#define DU_PLANE(i, offset)	DU_REG(((i) * 0x100) + (offset))
#define PMR(i)			DU_PLANE(i, 0x00)
#define PMWR(i)			DU_PLANE(i, 0x04)
#define PALPHAR(i)		DU_PLANE(i, 0x08)
#define PDSXR(i)		DU_PLANE(i, 0x10)
#define PDSYR(i)		DU_PLANE(i, 0x14)
#define PDPXR(i)		DU_PLANE(i, 0x18)
#define PDPYR(i)		DU_PLANE(i, 0x1C)
#define PDSA0R(i)		DU_PLANE(i, 0x20)
#define PDSA1R(i)		DU_PLANE(i, 0x24)
#define PDSA2R(i)		DU_PLANE(i, 0x28)
#define PDSPXR(i)		DU_PLANE(i, 0x30)
#define PDSPYR(i)		DU_PLANE(i, 0x34)
#define PWASPR(i)		DU_PLANE(i, 0x38)
#define PWAMWR(i)		DU_PLANE(i, 0x3C)
#define PBTR(i)			DU_PLANE(i, 0x40)
#define PTC1R(i)		DU_PLANE(i, 0x44)
#define PTC2R(i)		DU_PLANE(i, 0x48)
#define PMLR(i)			DU_PLANE(i, 0x50)
#define PSWAPR(i)		DU_PLANE(i, 0x80)
#define PDDCR(i)		DU_PLANE(i, 0x84)
#define PDDCR2(i)		DU_PLANE(i, 0x88)

/* DU external synchronization control */
#define ESCR			DU_REG(0x10000)
#define OTAR			DU_REG(0x10004)

DECLARE_GLOBAL_DATA_PTR;

void du_start(void)
{
	u32 endian;

	/* DSEC = little endian, DRES = 1, DEN = 0 */
	endian = readl(MODEMR) & MODEMR_ENDIAN_LITTLE;
	endian = (endian) ? 0x00000000u : 0x00100000u;
	writel(0x00000200 | endian, DSYSR);

	/* CSPM = 1 */
	writel(0x01000000u, DSMR);

	/* DCLKSEL = 1, FRQSEL = 2 (DOTCLK = 66.667MHz) */
	writel(0x00100002u, ESCR);

	/* 1280x768@60Hz (DOTCLOCK = 65MHz) */
	writel(277u, HDSR);
	writel(1301u, HDER);
	writel(27u, VDSR);
	writel(795u, VDER);
	writel(1343u, HCR);
	writel(135u, HSWR);
	writel(805u, VCR);
	writel(799u, VSPR);

	/* irrelevant */
	writel(0u, EQWR);
	writel(0u, SPWR);
	writel(0u, CLAMPSR);
	writel(0u, CLAMPWR);
	writel(0u, DESR);
	writel(0u, DEWR);
	writel(0u, CDER);
	writel(0u, RINTOFSR);
	writel(0u, OTAR);

	/* background color */
	writel(0u, DOOR);
#if defined(CONFIG_DISPLAY_BG_COLOR)
	writel(CONFIG_DISPLAY_BG_COLOR & COLOR_MASK, BPOR);
#else /* defined(CONFIG_DISPLAY_BG_COLOR) */
	writel(0u, BPOR);
#endif /* defined(CONFIG_DISPLAY_BG_COLOR) */

	/* plane priority */
	writel(0x76543210u, DPPR);

	/* display start */
	writel(0x00000100 | endian, DSYSR);
}

int checkboard(void)
{
	puts("BOARD: Act Brain Actlinux-Alpha\n");
	return 0;
}

int board_init(void)
{
	/* PFC */

	/* GETHER_A, INTC_B */
	WRITE_PFC(0x00000001u, MODESEL0);

	/* SCIF4_D */
	WRITE_PFC(0x00003000u, MODESEL2);

	/* ET0_ETXD0, ET0_GTX_CLK_A, ET0_ETXD1_A, ET0_ETXD2_A, ET0_ETXD3_A, */
	/* ET0_ETXD4, ET0_ETXD5_A, ET0_ETXD6_A, ET0_ETXD7                   */
	WRITE_PFC(0x25AE4920u, IPSR3);

	/* ET0_ERXD7, ET0_RX_DV, ET0_RX_ER, ET0_CRS, ET0_COL, */
	/* ET0_MDC, ET0_MDIO_A, ET0_LINK_A, ET0_PHY_INT_A     */
	/* CTXS1#_B, RTS1#_B, SCK2_A                          */
	WRITE_PFC(0x573E4924u, IPSR4);

	/* ET0_ERXD0, ET0_ERXD1, ET0_ERXD2_A, ET0_ERXD3_A */
	WRITE_PFC(0x3E400000u, IPSR8);

	/* RX4_D, TX4_D, IRQ1_B, IRQ0_B */
	WRITE_PFC(0x06C00000u, IPSR10);

	/* ET0_ERXD4, ET0_ERXD5, ET0_ERXD6, ET0_TX_EN, ET0_TX_ER, */
	/* ET0_TX_CLK_A, ET0_RX_CLK_A                             */
	WRITE_PFC(0x0E500E40u, IPSR11);

	/* A0-25, D0-4, PRESETOUT# */
	WRITE_PFC(0xFFFFFFFFu, GPSR0);

	/* EX_WAIT0, ET0_ETXD0-7(_A), ET0_EX_EN, ET0_TX_ER, D5-15, CLKOUT, */
	/* BS#, CS0#, CS1#, ET0_GTX_CLK, RD#, WE0#, WE1#, ET0_TX_CKL_A     */
	WRITE_PFC(0xFFFF7FFFu, GPSR1);

	/* ET0_ERXD0-7(_A), ET0_RX_CLK_A, ET0_RX_DV, ET0_RX_ER, */
	/* ET0_CRS, ET0_COL, ET0_MDC, ET0_MDIO_A, ET0_PHY_INT_A */
	WRITE_PFC(0x4005FEFFu, GPSR2);

	/* DU0_DR0-7, DU0_DG0-7, DU0_DB0-7, DU0_DOTCLOCKOUT, */
	/* DU0_HSYNC, DU0_VSYNC, DU0_DISP */
	WRITE_PFC(0x2EFFFFFFu, GPSR3);

	/* SCL0, SDA0, PENC0/1, RX4_D, TX4_D */
	WRITE_PFC(0xCF000000u, GPSR4);

	/* IRQ0-3_B */
	WRITE_PFC(0x0000040Du, GPSR5);

	{
		u32 sts, ctl;

		ctl = sts = readl(MSTPSR1);
#if defined(CONFIG_SH_DU)
		ctl &= ~MSTPSR1_DU;
#endif /* defined(CONFIG_SH_DU) */

#if defined(CONFIG_SH_ETHER)
		ctl &= ~MSTPSR1_GETHER;
#endif /* defined(CONFIG_SH_ETHER) */
		if (ctl != sts)
			writel(ctl, MSTPCR1);
	}

	return 0;
}

int board_late_init(void)
{
	u8 mac[6];

	if (!eth_getenv_enetaddr("ethaddr", mac)) {
		eth_random_enetaddr(mac);
		eth_setenv_enetaddr("ethaddr", mac);
	}
	writel(((u32)mac[0] << 24) | ((u32)mac[1] << 16) |
		((u32)mac[2] << 8) | ((u32)mac[3] << 0), MAHR0);
	writel(((u32)mac[4] << 8) | ((u32)mac[5] << 0), MALR0);

#ifdef CONFIG_SH_DU
	du_start();
#endif /* CONFIG_SH_DU */

	return 0;
}

int dram_init(void)
{
	printf("DRAM:	%luMB\n", gd->bd->bi_memsize / (1024u * 1024u));
	return 0;
}

