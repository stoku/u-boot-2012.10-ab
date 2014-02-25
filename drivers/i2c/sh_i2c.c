/*
 * Copyright (C) 2011 Renesas Solutions Corp.
 * Copyright (C) 2011 Nobuhiro Iwamatsu <nobuhiro.iwamatsu.yj@renesas.com>
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
#include <asm/io.h>

/* Every register is 32bit aligned, but only 8bits in size */
#define ureg(name) u8 name; u8 __pad_##name##0; u16 __pad_##name##1;
struct sh_i2c {
	ureg(icdr);
	ureg(iccr);
	ureg(icsr);
	ureg(icic);
	ureg(iccl);
	ureg(icch);
};
#undef ureg

static struct sh_i2c *base;

/* ICCR */
#define SH_I2C_ICCR_ICE		(1 << 7)
#define SH_I2C_ICCR_RACK	(1 << 6)
#define SH_I2C_ICCR_RTS		(1 << 4)
#define SH_I2C_ICCR_BUSY	(1 << 2)
#define SH_I2C_ICCR_SCP		(1 << 0)

/* ICSR / ICIC */
#define SH_IC_BUSY	(1 << 4)
#define SH_IC_AL	(1 << 3)
#define SH_IC_TACK	(1 << 2)
#define SH_IC_WAIT	(1 << 1)
#define SH_IC_DTE	(1 << 0)

static u8 iccl, icch;

#define IRQ_WAIT 1000

static u8 wait_status(struct sh_i2c *base, u8 bit)
{
	int i;
	u8 sts;

	for (i = 0; i < IRQ_WAIT; i++) {
		sts = readb(&base->icsr);
		if (sts & (SH_IC_AL | SH_IC_TACK))
			return 0u;
		if (sts & bit)
			return sts;
		udelay(10);
	}
	return 0u;
}

static int wait_stop(struct sh_i2c *base)
{
	int i;

	for (i = 0 ; i < IRQ_WAIT ; i++) {
		if (!(SH_IC_BUSY & readb(&base->icsr)))
			return 0;
		udelay(10);
	}
	return -1;
}

static void clr_wait(struct sh_i2c *base, u8 sts)
{
	writeb(sts & ~SH_IC_WAIT, &base->icsr);
}

static u8 i2c_set_addr(struct sh_i2c *base, u8 id, u8 reg)
{
	u8 sts;

	writeb(readb(&base->iccr) & ~SH_I2C_ICCR_ICE, &base->iccr);
	writeb(readb(&base->iccr) | SH_I2C_ICCR_ICE, &base->iccr);

	writeb(iccl, &base->iccl);
	writeb(icch, &base->icch);
	writeb(SH_IC_AL|SH_IC_TACK|SH_IC_WAIT, &base->icic);

	writeb((SH_I2C_ICCR_ICE|SH_I2C_ICCR_RTS|SH_I2C_ICCR_BUSY), &base->iccr);

	if (!(sts = wait_status(base, SH_IC_DTE))) goto out;
	writeb(id << 1, &base->icdr);

	if (!(sts = wait_status(base, SH_IC_WAIT))) goto out;
	writeb(reg, &base->icdr);
out:
	return sts;
}

static void i2c_finish(struct sh_i2c *base)
{
	writeb(0, &base->icic);
	writeb(0, &base->icsr);
	writeb(readb(&base->iccr) & ~SH_I2C_ICCR_ICE, &base->iccr);
}

static int i2c_raw_write(struct sh_i2c *base, u8 id, u8 reg, u8 *val, int size)
{
	int i = 0, ret = -1;
	u8 sts = i2c_set_addr(base, id, reg);

	while (sts && i < size) {
		clr_wait(base, sts);
		sts = wait_status(base, SH_IC_WAIT);
		if (sts) writeb(val[i++], &base->icdr);
	}

	if (!sts) goto out;
	writeb((SH_I2C_ICCR_ICE|SH_I2C_ICCR_RTS), &base->iccr);
	clr_wait(base, sts);
	if (!(sts = wait_status(base, SH_IC_WAIT))) goto out;
	clr_wait(base, sts);
	if (wait_stop(base)) goto out;
	ret = 0;
out:
	i2c_finish(base);
	return ret;
}

static int i2c_raw_read(struct sh_i2c *base, u8 id, u8 reg, u8 *val, int size)
{
	int i = 0, ret = -1;
	u8 sts = i2c_set_addr(base, id, reg);

	if (!sts) goto out;
	writeb((SH_I2C_ICCR_ICE|SH_I2C_ICCR_RTS|SH_I2C_ICCR_BUSY), &base->iccr);
	clr_wait(base, sts);
	if (!(sts = wait_status(base, SH_IC_WAIT))) goto out;
	clr_wait(base, sts);
	if (!(sts = wait_status(base, SH_IC_DTE))) goto out;
	writeb(id << 1 | 0x01, &base->icdr);

	if (!(sts = wait_status(base, SH_IC_WAIT))) goto out;
	writeb((SH_I2C_ICCR_ICE|SH_I2C_ICCR_SCP), &base->iccr);
	clr_wait(base, sts);

	while (i < size - 1) {
		sts = wait_status(base, SH_IC_WAIT | SH_IC_DTE);
		if (sts & SH_IC_DTE)
			val[i++] = readb(&base->icdr);
		else if (sts & SH_IC_WAIT)
			clr_wait(base, sts);
		else
			goto out;
	}

	if (!(sts = wait_status(base, SH_IC_WAIT))) goto out;
	writeb((SH_I2C_ICCR_ICE|SH_I2C_ICCR_RACK), &base->iccr);
	clr_wait(base, sts);

	while (i < size) {
		sts = wait_status(base, SH_IC_WAIT | SH_IC_DTE);
		if (sts & SH_IC_DTE)
			val[i++] = readb(&base->icdr);
		else if (sts & SH_IC_WAIT)
			clr_wait(base, sts);
		else
			goto out;
	}
	if (wait_stop(base)) goto out;
	ret = 0;
out:
	i2c_finish(base);
	return ret;
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
	if ((bus < 0) || (bus >= CONFIG_SYS_MAX_I2C_BUS)) {
		printf("Bad bus: %d\n", bus);
		return -1;
	}

	switch (bus) {
	case 0:
		base = (void *)CONFIG_SH_I2C_BASE0;
		break;
	case 1:
		base = (void *)CONFIG_SH_I2C_BASE1;
		break;
	default:
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

#define SH_I2C_ICCL_CALC(clk, date, t_low, t_high) \
		((clk / rate) * (t_low / t_low + t_high))
#define SH_I2C_ICCH_CALC(clk, date, t_low, t_high) \
		((clk / rate) * (t_high / t_low + t_high))

void i2c_init(int speed, int slaveaddr)
{
	int num, denom, tmp;

#ifdef CONFIG_I2C_MULTI_BUS
	current_bus = 0;
#endif
	base = (struct sh_i2c *)CONFIG_SH_I2C_BASE0;

	/*
	 * Calculate the value for iccl. From the data sheet:
	 * iccl = (p-clock / transfer-rate) * (L / (L + H))
	 * where L and H are the SCL low and high ratio.
	 */
	num = CONFIG_SH_I2C_CLOCK * CONFIG_SH_I2C_DATA_LOW;
	denom = speed * (CONFIG_SH_I2C_DATA_HIGH + CONFIG_SH_I2C_DATA_LOW);
	tmp = num * 10 / denom;
	if (tmp % 10 >= 5)
		iccl = (u8)((num/denom) + 1);
	else
		iccl = (u8)(num/denom);

	/* Calculate the value for icch. From the data sheet:
	   icch = (p clock / transfer rate) * (H / (L + H)) */
	num = CONFIG_SH_I2C_CLOCK * CONFIG_SH_I2C_DATA_HIGH;
	tmp = num * 10 / denom;
	if (tmp % 10 >= 5)
		icch = (u8)((num/denom) + 1);
	else
		icch = (u8)(num/denom);
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
