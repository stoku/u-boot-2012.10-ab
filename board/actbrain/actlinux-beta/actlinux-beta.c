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
#include <i2c.h>
#include <rtc.h>
#include <net.h>
#include <netdev.h>

#define VERSION_NUMBER(major, minor)	(((major) << 16) | ((minor) << 8))
#define VERSION_MASK(v)			((v) & 0xFFFFFF00)

#define SWAB16(d)		((((d) & (u16)0x00FFu) << 8) | \
				 (((d) & (u16)0xFF00u) >> 8))
#define SWAB32(d)		((((d) & (u32)0x000000FFul) << 24) | \
				 (((d) & (u32)0x0000FF00ul) <<  8) | \
				 (((d) & (u32)0x00FF0000ul) >>  8) | \
				 (((d) & (u32)0xFF000000ul) >> 24))

DECLARE_GLOBAL_DATA_PTR;

static void init_seed(void)
{
	unsigned long seed = 0;

	if (!i2c_probe(CONFIG_SYS_I2C_RTC_ADDR)) {
		struct rtc_time rt;
		if(rtc_get(&rt)) {
			/* need reset */
			rtc_reset();
			rtc_get(&rt);
		}
		seed = mktime(rt.tm_year, rt.tm_mon, rt.tm_mday,
				rt.tm_hour, rt.tm_min, rt.tm_sec);
	}

	/* set random seed to milliseconds from epoch */
	seed = (seed * 1000) + (get_timer(0) % 1000);
	srand(seed);
}

static void init_mac(void)
{
	u8 mac[6];

	if (!eth_getenv_enetaddr("ethaddr", mac)) {
		unsigned int rval;

		rval = rand();
		mac[0] = (rval >>  0) & 0xff;
		mac[1] = (rval >>  8) & 0xff;
		mac[2] = (rval >> 16) & 0xff;
		rval = rand();
		mac[3] = (rval >>  0) & 0xff;
		mac[4] = (rval >>  8) & 0xff;
		mac[5] = (rval >> 16) & 0xff;
		/* make sure it's local and unicast */
		mac[0] = (mac[0] | 0x02) & ~0x01;

		eth_setenv_enetaddr("ethaddr", mac);
	}
}

int board_init(void)
{
	/* PFC */
	writew(0x0100u, PSELC);
	writew(0x0000u, PCCR);
	writew(0x0000u, PDCR);
	writew(0xA000u, PECR);
	writew(0x0000u, PFCR);
	writew(0xA80Au, PLCR);

	return 0;
}

int checkboard(void)
{
	puts("BOARD: Act Brain Actlinux-Beta\n");
	return 0;
}

int dram_init(void)
{
	printf("DRAM:  %luMB\n", gd->bd->bi_memsize / (1024u * 1024u));
	return 0;
}

int board_late_init(void)
{
	init_seed();
	return 0;
}

int board_eth_init(bd_t *bis)
{
	init_mac();
	return ax88796b_initialize(bis);
}

