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
#include <addr_map.h>

#define VERSION_NUMBER(major, minor)	(((major) << 16) | ((minor) << 8))
#define VERSION_MASK(v)			((v) & 0xFFFFFF00)

#define SWAB16(d)		((((d) & (u16)0x00FFu) << 8) | \
				 (((d) & (u16)0xFF00u) >> 8))
#define SWAB32(d)		((((d) & (u32)0x000000FFul) << 24) | \
				 (((d) & (u32)0x0000FF00ul) <<  8) | \
				 (((d) & (u32)0x00FF0000ul) >>  8) | \
				 (((d) & (u32)0xFF000000ul) >> 24))

#define PMB_VALID_MASK		(1ul << PMB_V)
#define PMB_IS_VALID(d)		(((d) & PMB_VALID_MASK) != 0ul)
#define PMB_SIZE_VALUE(b1, b0)	(((b1) << PMB_SZ1) | ((b0) << PMB_SZ0))
#define PMB_SIZE_MASK		PMB_SIZE_VALUE(1, 1)
#define PMB_SIZE(d)		((d) & PMB_SIZE_MASK)
#define PMB_VA_MASK		(~((1ul << PMB_VPN) - 1ul))
#define PMB_VA(d)		((d) & PMB_VA_MASK)
#define PMB_PA_MASK		(~((1ul << PMB_PPN) - 1ul))
#define PMB_PA(d)		((d) & PMB_PA_MASK)

/*
 * Jump to uncached area.
 * When access to PMB arrays, we need to do it from an uncached area.
 */
#define jump_to_uncached()			\
do {						\
	unsigned long __dummy;			\
						\
	__asm__ __volatile__(			\
		"mova	1f, %0\n\t"		\
		"add	%1, %0\n\t"		\
		"jmp	@%0\n\t"		\
		" nop\n\t"			\
		".balign 4\n"			\
		"1:"				\
		: "=&z" (__dummy)		\
		: "r" (cached_to_uncached));	\
} while (0)

/*
 * Back to cached area.
 */
#define back_to_cached()			\
do {						\
	unsigned long __dummy;			\
	__asm__ __volatile__(			\
		"icbi	@%1\n\t"		\
		"mov.l	1f, %0\n\t"		\
		"jmp	@%0\n\t"		\
		" nop\n\t"			\
		".balign 4\n"			\
		"1:	.long 2f\n"		\
		"2:"				\
		: "=&r" (__dummy)		\
		: "r" (text_base));		\
} while (0)

DECLARE_GLOBAL_DATA_PTR;

static unsigned long text_base = CONFIG_SYS_TEXT_BASE;
static unsigned long cached_to_uncached = 0x20000000;

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

static void update_addrmap(void)
{
	unsigned long addr;
	unsigned long data;
	unsigned long size;
	int i;

	/*     mk_pmb_data_val( ppn, ub, v, sz1, sz0, c, wt) */
	data = mk_pmb_data_val(0x40,  0, 1,   0,   1, 0,  0);

	/* replace mapping entry 1
	 * before:	0xA0000000 -> 0x00000000 (64MB, uncached)
	 * after:	0xA0000000 -> 0x40000000 (64MB, uncached)
	 */
	jump_to_uncached();
	writel(data, PMB_DATA_BASE(1));
	back_to_cached();

	for (i = 0; i < CONFIG_SYS_NUM_ADDR_MAP; i++) {

		addr = readl(PMB_ADDR_BASE(i));
		data = readl(PMB_DATA_BASE(i));
		if (PMB_IS_VALID(addr)) {
			switch (PMB_SIZE(data)) {
			case PMB_SIZE_VALUE(0, 0):
				size = 16 * 1024 * 1024;
				break;
			case PMB_SIZE_VALUE(0, 1):
				size = 64 * 1024 * 1024;
				break;
			case PMB_SIZE_VALUE(1, 0):
				size = 128 * 1024 * 1024;
				break;
			default:
				size = 512 * 1024 * 1024;
				break;
			}

			addrmap_set_entry(PMB_VA(addr),
					PMB_PA(data),
					size, i);
		}
	}
}

int board_init(void)
{
	/* PFC */
	writew(0x0040u, PSELA);
	writew(0x0100u, PSELC);
	writew(0x0000u, PCCR);
	writew(0x0000u, PDCR);
	writew(0xA000u, PECR);
	writew(0x0000u, PFCR);
	writew(0xA80Au, PLCR);
	writew(0xAAA2u, PMCR);

	update_addrmap();

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

