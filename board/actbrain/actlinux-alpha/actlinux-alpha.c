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
#include <asm/cache.h>
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
#define DEFR2			DU_REG(0x0034)

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
#define DU_PLANE(i, offset)	DU_REG((((i) + 1) * 0x100) + (offset))
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
#define PSPXR(i)		DU_PLANE(i, 0x30)
#define PSPYR(i)		DU_PLANE(i, 0x34)
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
#define ESCR_DCLKSEL		(0x01 << 20)
#define ESCR_FRQSEL(i)		((i) - 1)
#define OTAR			DU_REG(0x10004)

#define SIZE(x, y)		(((x) << 16) | (y))

#define VERSION_NUMBER(major, minor)	(((major) << 16) | ((minor) << 8))
#define VERSION_MASK(v)			((v) & 0xFFFFFF00)

#define SWAB16(d)		((((d) & (u16)0x00FFu) << 8) | \
				 (((d) & (u16)0xFF00u) >> 8))
#define SWAB32(d)		((((d) & (u32)0x000000FFul) << 24) | \
				 (((d) & (u32)0x0000FF00ul) <<  8) | \
				 (((d) & (u32)0x00FF0000ul) >>  8) | \
				 (((d) & (u32)0xFF000000ul) >> 24))

DECLARE_GLOBAL_DATA_PTR;

static void mac_set(void)
{
	u8 mac[6];

	if (!eth_getenv_enetaddr("ethaddr", mac)) {
		eth_random_enetaddr(mac);
		eth_setenv_enetaddr("ethaddr", mac);
	}
	writel(((u32)mac[0] << 24) | ((u32)mac[1] << 16) |
		((u32)mac[2] << 8) | ((u32)mac[3] << 0), MAHR0);
	writel(((u32)mac[4] << 8) | ((u32)mac[5] << 0), MALR0);
}

static u32 read32(const u32 *addr)
{
	return *addr;
}

static u32 read32swab(const u32 *addr)
{
	return SWAB32(*addr);
}

static u16 read16(const u16 *addr)
{
	return *addr;
}

static u16 read16swab(const u16 *addr)
{
	return SWAB16(*addr);
}

static void du_start(void)
{
	u32 be, dw, dh;

	/* DSEC = endian, DRES = 1, DEN = 0 */
	be = (readl(MODEMR) & MODEMR_ENDIAN_LITTLE) ? 0u : 0x100000u;
	writel(0x00000200 | be, DSYSR);

	/* CSPM = 1 */
	writel(0x01000000u, DSMR);

	/* DEFE2G = 1 */
	writel(0x77750001, DEFR2);

	dw = 0;
	dh = 0;

	/* setup display timing registers */
	{
		u32 dotclk, hds, hde, vds, vde, hc, hsw, vc, vsp;

#if defined(CONFIG_DISPLAY_WIDTH)
		dw = CONFIG_DISPLAY_WIDTH;
#endif

#if defined(CONFIG_DISPLAY_HEIGHT)
		dh = CONFIG_DISPLAY_HEIGHT;
#endif

		switch(SIZE(dw, dh)) {
		case SIZE(640, 480): /* 640x480@60Hz */
			dotclk	= 25175000; /* 25.175MHz */
			hds	= 125;
			hde	= 765;
			vds	= 31;
			vde	= 511;
			hc	= 799;
			hsw	= 95;
			vc	= 524;
			vsp	= 522;
			break;
		case SIZE(800, 600): /* 800x600@60Hz */
			dotclk	= 40000000; /* 40MHz */
			hds	= 197;
			hde	= 997;
			vds	= 21;
			vde	= 621;
			hc	= 1055;
			hsw	= 127;
			vc	= 627;
			vsp	= 623;
			break;
		case SIZE(1280, 768): /* 1280x768@60Hz */
			dotclk	= 68250000; /* 68.25MHz */
			hds	= 93;
			hde	= 1373;
			vds	= 10;
			vde	= 778;
			hc	= 1439;
			hsw	= 31;
			vc	= 789;
			vsp	= 782;
			break;
		default:
			puts("Display: invalid display size, "
				"use 1024x768 instead\n");
			/* fall through */
		case SIZE(1024, 768):
			dotclk	= 65000000; /* 65MHz */
			hds	= 277;
			hde	= 1301;
			vds	= 27;
			vde	= 795;
			hc	= 1343;
			hsw	= 135;
			vc	= 805;
			vsp	= 799;
			dw	= 1024;
			dh	= 768;
			break;
		}

		dotclk = (200000000 + (dotclk / 2)) / dotclk; /* freq div */
		writel(ESCR_DCLKSEL | ESCR_FRQSEL(dotclk), ESCR);

		writel(hds, HDSR);
		writel(hde, HDER);
		writel(vds, VDSR);
		writel(vde, VDER);
		writel(hc, HCR);
		writel(hsw, HSWR);
		writel(vc, VCR);
		writel(vsp, VSPR);

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
	}

	/* background color */
	writel(0u, DOOR);
#if defined(CONFIG_DISPLAY_BG_COLOR)
	writel(CONFIG_DISPLAY_BG_COLOR & COLOR_MASK, BPOR);
#else /* defined(CONFIG_DISPLAY_BG_COLOR) */
	writel(0u, BPOR);
#endif /* defined(CONFIG_DISPLAY_BG_COLOR) */

#if defined(CONFIG_DISPLAY_IMAGE_DATA_ADDR) && \
	defined(CONFIG_DISPLAY_IMAGE_LOAD_ADDR)
	/* setup plane registers */
	{
		u32 mode, iw, ih, is, x, y, plane;
		const u32 *src32;
		const u16 *src16;
		u16 *dst16;
		u32 (*read32be)(const u32 *);
		u16 (*read16be)(const u16 *);

		if (be) {
			read32be = read32;
			read16be = read16;
		} else {
			read32be = read32swab;
			read16be = read16swab;
		}

		src32 = (const u32 *)CONFIG_DISPLAY_IMAGE_DATA_ADDR;
		if (strncmp((const char *)src32, "spim", 4)) {
			puts("Splash Image: invalid signature\n");
			goto skip_image;
		}
		if (VERSION_MASK(read32be(src32 + 1)) != VERSION_NUMBER(1, 0)) {
			puts("Splash Image: invalid version\n");
			goto skip_image;
		}

		switch (read32be(src32 + 2)) {
		case 565:
			mode = 0x00000001; /* PnDDF = RGB565 */
			break;
		case 1555:
			mode = 0x00000002; /* PnDDF = ARGB1555 */
			break;
		default:
			puts("Splash Image: invalid pixel format\n");
			goto skip_image;
		}
		mode |= 0x00005000; /* PnSPIM = 5 (no colorkey, blend) */

		iw    = read32be(src32 + 3);
		ih    = read32be(src32 + 4);
		is    = (iw + 15u) & ~15u; /* stride */
		src16 = (const u16 *)(src32 + 5);
		dst16 = (u16 *)CONFIG_DISPLAY_IMAGE_LOAD_ADDR;

		/* P1 is write-through mode and no need to flush cache */
		for (y = 0; y < ih; y++)
			for (x = 0; x < iw; x++)
				dst16[x + (y * is)] =
					read16be(&src16[x + (y * iw)]);

		plane = 7; /* use plane 7 (zero based index) */

		writel(mode, PMR(plane));
		writel(is, PMWR(plane));
		writel(MIN(iw, dw), PDSXR(plane));
		writel(MIN(ih, dh), PDSYR(plane));
		writel((iw < dw) ? (dw - iw) / 2 : 0, PDPXR(plane));
		writel((ih < dh) ? (dh - ih) / 2 : 0, PDPYR(plane));
		writel((iw > dw) ? (iw - dw) / 2 : 0, PSPXR(plane));
		writel((ih > dw) ? (ih - dh) / 2 : 0, PSPYR(plane));
		writel(0, PWASPR(plane));
		writel(ih, PWAMWR(plane));
		writel((u32)dst16 & 0x1FFFFFFF, PDSA0R(plane));
		writel(0x000000FF, PALPHAR(plane));

		/* plane priority and visibility */
		writel((0x8 | plane) << (4 * plane), DPPR);
	}
skip_image:
#endif
	/* display start */
	writel(0x00000100 | be, DSYSR);
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
	mac_set();
#ifdef CONFIG_SH_DU
	du_start();
#endif /* CONFIG_SH_DU */

	return 0;
}

int dram_init(void)
{
	printf("DRAM:  %luMB\n", gd->bd->bi_memsize / (1024u * 1024u));
	return 0;
}

