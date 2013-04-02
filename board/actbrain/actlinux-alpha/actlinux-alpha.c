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
#include <i2c.h>
#include <net.h>

#define WRITE_PFC(d, a)	\
	do { 	\
		writel(~(d), PMMR); \
		writel(d, a); \
	} while (0)

#define MSTPCR1		(0xFFC80034)
#define MSTPSR1		(0xFFC80044)
#define MSTPSR1_GETHER	(1 << 14)
#define MSTPSR1_DU	(1 << 3)

DECLARE_GLOBAL_DATA_PTR;

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

	/* ET0_ERXD7, ET0_RX_DV, ET0_RX_ER, ET0_CRS, ET0_COL, ET0_MDC */
	/* ET0_MDIO_A, ET0_LINK_A, ET0_MAGIC_A, ET0_PHY_INT_A         */
	/* CTXS1#_B, RTS1#_B, SCK2_A                                  */
	WRITE_PFC(0x57FE4924u, IPSR4);

	/* ET0_ERXD0, ET0_ERXD1, ET0_ERXD2_A, ET0_ERXD3_A */
	WRITE_PFC(0x3ED00000u, IPSR8);

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

	/* SCL0, SDA0, PENC0/1, USB_OVC0/1, RX4_D, TX4_D */
	WRITE_PFC(0xFF000000u, GPSR4);

	/* IRQ0-3_B */
	WRITE_PFC(0x0000040Du, GPSR5);


	{
		u32 sts, ctl;

		ctl = sts = readl(MSTPSR1);
		ctl &= ~MSTPSR1_DU;
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

	/* Read Mac Address and set*/
	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
	i2c_set_bus_num(CONFIG_SYS_I2C_MODULE);

	/* Read MAC address */
	i2c_read(0x50, 0x0, 0, mac, 6);

	if (is_valid_ether_addr(mac))
		eth_setenv_enetaddr("ethaddr", mac);

	return 0;
}

int dram_init(void)
{
	printf("DRAM:	%luMB\n", gd->bd->bi_memsize / (1024u * 1024u));
	return 0;
}

