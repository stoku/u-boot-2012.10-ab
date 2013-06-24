/*
 * Copyright (C) 2012 Nobuhiro Iwamatsu <nobuhiro.iwamatsu.yj@renesas.com>
 * Copyright (C) 2012 Renesas Solutions Corp.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <i2c.h>
#include <asm/io.h>

struct sh_i2c {
	u8 iccr1;
	u8 iccr2;
	u8 icmr;
	u8 icier;
	u8 icsr;
	u8 sar;
	u8 icdrt;
	u8 icdrr;
	u8 nf2cyc;
	u8 __pad0;
	u8 __pad1;
};

static struct sh_i2c *base;
static u8 iccr1_cks, nf2cyc;

/* ICCR1 */
#define SH_I2C_ICCR1_ICE	(1 << 7)
#define SH_I2C_ICCR1_RCVD	(1 << 6)
#define SH_I2C_ICCR1_MST	(1 << 5)
#define SH_I2C_ICCR1_TRS	(1 << 4)
#define SH_I2C_ICCR1_MTRS	\
	(SH_I2C_ICCR1_MST | SH_I2C_ICCR1_TRS)

/* ICCR1 */
#define SH_I2C_ICCR2_BBSY	(1 << 7)
#define SH_I2C_ICCR2_SCP	(1 << 6)
#define SH_I2C_ICCR2_SDAO	(1 << 5)
#define SH_I2C_ICCR2_SDAOP	(1 << 4)
#define SH_I2C_ICCR2_SCLO	(1 << 3)
#define SH_I2C_ICCR2_IICRST	(1 << 1)

#define SH_I2C_ICIER_TIE	(1 << 7)
#define SH_I2C_ICIER_TEIE	(1 << 6)
#define SH_I2C_ICIER_RIE	(1 << 5)
#define SH_I2C_ICIER_NAKIE	(1 << 4)
#define SH_I2C_ICIER_STIE	(1 << 3)
#define SH_I2C_ICIER_ACKE	(1 << 2)
#define SH_I2C_ICIER_ACKBR	(1 << 1)
#define SH_I2C_ICIER_ACKBT	(1 << 0)

#define SH_I2C_ICSR_TDRE	(1 << 7)
#define SH_I2C_ICSR_TEND	(1 << 6)
#define SH_I2C_ICSR_RDRF	(1 << 5)
#define SH_I2C_ICSR_NACKF	(1 << 4)
#define SH_I2C_ICSR_STOP	(1 << 3)
#define SH_I2C_ICSR_ALOVE	(1 << 2)
#define SH_I2C_ICSR_AAS		(1 << 1)
#define SH_I2C_ICSR_ADZ		(1 << 0)

#define SH_I2C_NF2CYC_PRS	(1 << 0)
#define SH_I2C_NF2CYC_NF2CYC	(1 << 0)

#define IRQ_WAIT 1000

static int check_icsr_bits(struct sh_i2c *base, u8 bits)
{
	int i;

	for (i = 0; i < IRQ_WAIT; i++) {
		if ((bits & readb(&base->icsr)) == bits)
			return 0;
		udelay(10);
	}

	return 1;
}

static int check_stop(struct sh_i2c *base)
{
	return check_icsr_bits(base, SH_I2C_ICSR_STOP);
}

static int check_tend(struct sh_i2c *base)
{
	return check_icsr_bits(base, SH_I2C_ICSR_TDRE | SH_I2C_ICSR_TEND);
}

static int check_tdre(struct sh_i2c *base)
{
	return check_icsr_bits(base, SH_I2C_ICSR_TDRE);
}

static int check_rdrf(struct sh_i2c *base)
{
	return check_icsr_bits(base, SH_I2C_ICSR_RDRF);
}

static int check_bbsy(struct sh_i2c *base)
{
	int i;

	for (i = 0 ; i < IRQ_WAIT ; i++) {
		if (!(SH_I2C_ICCR2_BBSY & readb(&base->iccr2)))
			return 0;
		udelay(10);
	}
	return 1;
}

static int check_ackbr(struct sh_i2c *base)
{
	int i;

	for (i = 0 ; i < IRQ_WAIT ; i++) {
		if (!(SH_I2C_ICIER_ACKBR & readb(&base->icier)))
			return 0;
		udelay(10);
	}

	return 1;
}

static void sh_i2c_reset(struct sh_i2c *base)
{
	setbits_8(&base->iccr2, SH_I2C_ICCR2_IICRST);

	udelay(100);

	clrbits_8(&base->iccr2, SH_I2C_ICCR2_IICRST);
}

static void sh_i2c_set_speed(struct sh_i2c *base, s32 speed)
{
	/* from SH7734 user's manual table 17.3 transfer rate */
	const s32 denom_tbl[] = {
		44, 52, 64, 72,
		84, 92, 100, 108,
		176, 208, 256, 288,
		336, 368, 400, 432,
		352, 416, 512, 576,
		672, 736, 800, 864,
		704, 832, 1024, 1152,
		1344, 1472, 1600, 1728
	};
	u8 clk, i;
	s32 speed_tmp, speed_cur;

	speed_cur = 0;
	for (i = 0; i < ARRAY_SIZE(denom_tbl); i++) {
		speed_tmp = CONFIG_SH_I2C_CLOCK / denom_tbl[i];

		if (speed > speed_tmp)
			continue;

		if (!speed_cur || (speed_cur > speed_tmp)) {
			speed_cur = speed_tmp;
			clk = i;
		}
	}
	if (!speed_cur) {
		puts("I2C:   failed to determine speed\n");
	} else {
		/* ICE enable and set clock */
		writeb(SH_I2C_ICCR1_ICE | (clk & 0x0f), &base->iccr1);
		writeb(SH_I2C_NF2CYC_NF2CYC | (clk & 0x10), &base->nf2cyc);
		printf("I2C:   %d bit/s\n", speed_cur);
	}
}

static int sh_i2c_stop(struct sh_i2c *base)
{
	clrbits_8(&base->icsr, SH_I2C_ICSR_STOP);
	clrbits_8(&base->iccr2, SH_I2C_ICCR2_BBSY | SH_I2C_ICCR2_SCP);
	udelay(10);
	return check_stop(base);
}

static int i2c_set_addr(struct sh_i2c *base, u8 id, u8 reg)
{
	const char *msg = NULL;

	if (check_bbsy(base)) {
		msg = "bus busy";
		goto fail;
	}

	setbits_8(&base->iccr1, SH_I2C_ICCR1_MTRS);
	clrsetbits_8(&base->iccr2, SH_I2C_ICCR2_SCP, SH_I2C_ICCR2_BBSY);

	if (check_tdre(base)) {
		msg = "TDRE check failed";
		goto fail;
	}

	writeb((id << 1), &base->icdrt);

	if (check_tend(base)) {
		msg = "TEND check failed";
		goto fail;
	}

	if (check_ackbr(base)) {
		goto fail;
	}

	writeb(reg, &base->icdrt);

	return 0;

fail:
	if (msg)
		printf("I2C: can't set slave address %02x-%02x (%s)\n",
                       id, reg, msg);

	return 1;
}

static int
i2c_raw_write(struct sh_i2c *base, u8 id, u8 reg, u8 *val, int size)
{
	int i, ret = -1;
	const char *msg = NULL;

	if (i2c_set_addr(base, id, reg))
		goto out;

	for (i = 0; i < size; i++) {
		if (check_tdre(base)) {
			msg = "TDRE check failed";
			goto out;
		}
		writeb(val[i], &base->icdrt);
	}

	if (check_tend(base)) {
		msg = "TEND check failed";
		goto out;
	}

	ret = 0;

out:
	clrbits_8(&base->icsr, SH_I2C_ICSR_TEND);
	sh_i2c_stop(base);
	clrbits_8(&base->iccr1, SH_I2C_ICCR1_MTRS);
	clrbits_8(&base->icsr, SH_I2C_ICSR_TDRE);

	if (msg)
		printf("I2C: can't write to %02x-%02x (%s)\n",
			id, reg + i, msg);

	return ret;
}

static int
i2c_raw_read(struct sh_i2c *base, u8 id, u8 reg, u8 *val, int size)
{
	int i;
	const char *msg = NULL;
	u8 data;

	i = 0;

	if (i2c_set_addr(base, id, reg))
		goto fail;

	if (check_tend(base)) {
		msg = "TEND check failed";
		goto fail;
	}

	clrsetbits_8(&base->iccr2, SH_I2C_ICCR2_SCP, SH_I2C_ICCR2_BBSY);
	writeb((id << 1) | 1, &base->icdrt);

	if (check_tend(base)) {
		msg = "TEND check failed";
		goto fail;
	}

	clrbits_8(&base->icsr, SH_I2C_ICSR_TEND);
	clrsetbits_8(&base->iccr1, SH_I2C_ICCR1_TRS, SH_I2C_ICCR1_MST);
	clrbits_8(&base->icsr, SH_I2C_ICSR_TDRE);

	i = 0;
	if (size > 1) {
		clrbits_8(&base->icier, SH_I2C_ICIER_ACKBT);
		data = readb(&base->icdrr); /* dummy read */
		while (i < size - 2) {
			if (check_rdrf(base)) {
				msg = "RDRF check failed";
				goto fail;
			}
			val[i++] = readb(&base->icdrr);
		}
		if (check_rdrf(base)) {
			msg = "RDRF check failed";
			goto fail;
		}
	}
	setbits_8(&base->icier, SH_I2C_ICIER_ACKBT);
	setbits_8(&base->iccr1, SH_I2C_ICCR1_RCVD);
	data = readb(&base->icdrr);
	if (check_rdrf(base)) {
		msg = "RDRF check failed";
		goto fail;
	}
	if (size > 1)
		val[i++] = data;
	sh_i2c_stop(base);
	val[i] = readb(&base->icdrr);
	clrbits_8(&base->iccr1, SH_I2C_ICCR1_RCVD);
	clrbits_8(&base->iccr1, SH_I2C_ICCR1_MTRS);

	return 0;

fail:
	clrbits_8(&base->icsr, SH_I2C_ICSR_TEND);
	sh_i2c_stop(base);
	clrbits_8(&base->iccr1, SH_I2C_ICCR1_RCVD);
	clrbits_8(&base->iccr1, SH_I2C_ICCR1_MTRS);
	clrbits_8(&base->icsr, SH_I2C_ICSR_TDRE);

	if (msg)
		printf("I2C: can't read from %02x-%02x (%s)\n",
			id, reg + i, msg);

	return -1;
}

#ifdef CONFIG_I2C_MULTI_BUS
static unsigned int current_bus;

/**
 * i2c_set_bus_num - change active I2C bus
 *	@bus: bus index, zero based
 *	@returns: 0 on success, non-0 on failure
 */
int i2c_set_bus_num(unsigned int bus)
{
	switch (bus) {
	case 0:
		base = (void *)CONFIG_SH_I2C_BASE0;
		break;
	case 1:
		base = (void *)CONFIG_SH_I2C_BASE1;
		break;
	default:
		printf("Bad bus: %d\n", bus);
		return -1;
	}

	current_bus = bus;

	return 0;
}

/**
 * i2c_get_bus_num - returns index of active I2C bus
 */
unsigned int i2c_get_bus_num(void)
{
	return current_bus;
}
#endif

void i2c_init(int speed, int slaveaddr)
{
#ifdef CONFIG_I2C_MULTI_BUS
	current_bus = 0;
#endif
	base = (struct sh_i2c *)CONFIG_SH_I2C_BASE0;

	/* Reset */
	sh_i2c_reset(base);

	/* ICE enable and set clock */
	sh_i2c_set_speed(base, speed);
}

/*
 * i2c_read: - Read multiple bytes from an i2c device
 *
 * The higher level routines take into account that this function is only
 * called with len < page length of the device (see configuration file)
 *
 * @chip:   address of the chip which is to be read
 * @addr:   i2c data address within the chip
 * @alen:   length of the i2c data address (1..2 bytes)
 * @buffer: where to write the data
 * @len:    how much byte do we want to read
 * @return: 0 in case of success
 */
int i2c_read(u8 chip, u32 addr, int alen, u8 *buffer, int len)
{
	return i2c_raw_read(base, chip, addr, buffer, len);
}

/*
 * i2c_write: -  Write multiple bytes to an i2c device
 *
 * The higher level routines take into account that this function is only
 * called with len < page length of the device (see configuration file)
 *
 * @chip:   address of the chip which is to be written
 * @addr:   i2c data address within the chip
 * @alen:   length of the i2c data address (1..2 bytes)
 * @buffer: where to find the data to be written
 * @len:    how much byte do we want to read
 * @return: 0 in case of success
 */
int i2c_write(u8 chip, u32 addr, int alen, u8 *buffer, int len)
{
	return i2c_raw_write(base, chip, addr, buffer, len);
}

/*
 * i2c_probe: - Test if a chip answers for a given i2c address
 *
 * @chip:   address of the chip which is searched for
 * @return: 0 if a chip was found, -1 otherwhise
 */
int i2c_probe(u8 chip)
{
	return i2c_raw_write(base, chip, 0, NULL, 0);
}
