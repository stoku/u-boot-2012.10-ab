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
#include <video_fb.h>
#include "videomodes.h"

#ifdef CONFIG_CPU_SH7724
#define SH_LCD_BASE		0xFE940000
#else
# error "Unsupported CPU"
#endif

#define LCD_REG(offset)	\
	(SH_LCD_BASE + (offset))
#define LCD_PLANE_REG(plane, offset) \
	(SH_LCD_BASE + (0x1000 * (plane)) + (offset))

#define MLDDCKPAT1R		LCD_REG(0x400)
#define MLDDCKPAT2R		LCD_REG(0x404)
#define LDDCKR			LCD_REG(0x410)
#define MLDMT1R(plane)		LCD_PLANE_REG(plane, 0x418)
#define MLDMT2R(plane)		LCD_PLANE_REG(plane, 0x41C)
#define MLDMT3R(plane)		LCD_PLANE_REG(plane, 0x420)
#define MLDDFR(plane)		LCD_PLANE_REG(plane, 0x424)
#define MLDSM1R(plane)		LCD_PLANE_REG(plane, 0x428)
#define MLDSM2R			LCD_REG(0x42C)
#define MLDSA1R(plane)		LCD_PLANE_REG(plane, 0x430)
#define MLDSA2R(plane)		LCD_PLANE_REG(plane, 0x434)
#define MLDMLSR(plane)		LCD_PLANE_REG(plane, 0x438)
#define MLDHCNR(plane)		LCD_PLANE_REG(plane, 0x448)
#define MLDHSYNR(plane)		LCD_PLANE_REG(plane, 0x44C)
#define MLDVLNR(plane)		LCD_PLANE_REG(plane, 0x450)
#define MLDVSYNR(plane)		LCD_PLANE_REG(plane, 0x454)
#define MLDHPDR(plane)		LCD_PLANE_REG(plane, 0x458)
#define MLDVPDR(plane)		LCD_PLANE_REG(plane, 0x45C)
#define MLDPMR			LCD_REG(0x460)
#define LDINTR			LCD_REG(0x468)
#define LDSR			LCD_REG(0x46C)
#define LDCNT1R			LCD_REG(0x470)
#define LDCNT2R			LCD_REG(0x474)
#define LDRCNTR			LCD_REG(0x478)
#define LDDDSR			LCD_REG(0x47C)
#define LDRCR			LCD_REG(0x484)

static int set_dot_clk(int dclk)
{
	__u32 value;

	value = dclk * (CONFIG_SH_MOBILE_LCD_CLKIN / 10000);
	value = (value + 50000000) / 100000000;

	if (value <= 1u) {
		value = 0x40u | 60u;
	} else {
		__u32 denom, remain, d, r;
		__u64 pat;

		denom = 0u;
		remain = value;
		for (d = 42u; d <= 60u; d += 6) {
			r = denom % value;
			if (r < remain) {
				denom = d;
				remain = r;
				if (r == 0u) break;
			}
		}
		pat = (1ull << (value >> 1)) - 1ull;
		while (value < denom) {
			pat |= pat << value;
			value <<= 1;
		}
		pat &= (1ull << denom) - 1ull;
		writel((__u32)pat, MLDDCKPAT2R);
		pat >>= 32;
		writel((__u32)pat, MLDDCKPAT1R);
		value = denom;
	}
	value |= CONFIG_SH_MOBILE_LCD_ICKSEL << 16;
	writel(value, LDDCKR);

	return 0;
}

static int set_res_mode(int plane, const struct ctfb_res_modes *mode)
{
	__u32 value;

	if (set_dot_clk(mode->pixclock))
		return -1;

	value = (__u32)(mode->xres / 8) << 16 |
		(__u32)((mode->hsync_len + mode->left_margin +
			mode->xres + mode->right_margin) / 8);
	writel(value, MLDHCNR(plane));
	value = (__u32)(mode->hsync_len / 8) << 16 |
		(__u32)((mode->xres + mode->right_margin) / 8);
	writel(value, MLDHSYNR(plane));
	value = (__u32)mode->yres << 16 |
		(__u32)(mode->vsync_len + mode->upper_margin +
			mode->yres + mode->lower_margin);
	writel(value, MLDVLNR(plane));
	value = (__u32)mode->vsync_len << 16 |
		(__u32)(mode->yres + mode->lower_margin);
	writel(value, MLDVSYNR(plane));

	return 0;
}

void *video_hw_init(void)
{
	static GraphicDevice gd;
	const struct ctfb_res_modes *mode = &res_mode_init[CONFIG_VIDEO_RES_MODE];
	int plane = CONFIG_SH_MOBILE_LCD_PLANE;
	unsigned int fb = (unsigned int)malloc(CONFIG_VIDEO_FB_SIZE);

	writel(readl(MSTPCR2) & ~0x00000001, MSTPCR2);

	if (set_res_mode(plane, mode)) return NULL;

	writel(0x1800000A, MLDMT1R(plane)); // configure for TFP410
	writel(0x00000003, MLDDFR(plane)); // RGB565
	writel(fb & ~0xE0000000, MLDSA1R(plane));
	writel(mode->xres * 2, MLDMLSR(plane));
	writel(0x00000006, LDDDSR);
	writel(plane << 1, LDRCNTR);
	writel(0x03, LDCNT2R);

	memset(&gd, 0, sizeof(gd));
	gd.frameAdrs = fb;
	gd.winSizeX = gd.plnSizeX = mode->xres;
	gd.winSizeY = gd.plnSizeY = mode->yres;
	gd.gdfIndex = GDF_16BIT_565RGB;
	gd.gdfBytesPP = 2;
	gd.memSize = gd.winSizeX * gd.winSizeY * gd.gdfBytesPP;
	
	if (getenv("splashpos") == NULL)
		setenv("splashpos", "m,m");
	if (getenv("splashimage") == NULL)
		setenv_addr("splashimage", (const void *)CONFIG_SPLASH_ADDR);

	return &gd;
}

