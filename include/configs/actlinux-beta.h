/*
 * Configuration settings for Act Brain actlinux-beta board
 *
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

#ifndef __ACTLINUX_BETA_H
#define __ACTLINUX_BETA_H

/******************************************************************************
 * CPU
 */

/* Architecture */
#define CONFIG_SH		1

/* Family */
#define	CONFIG_SH4		1
#define	CONFIG_SH4A		1

/* CPU */
#define CONFIG_CPU_SH7724	1

/* Peripheral clock */
#define CONFIG_SYS_CLK_FREQ	41666666 /* 41.66 MHz */

/* Divisor of timer clock */
#define CONFIG_SYS_TMU_CLK_DIV	4

/* Resolution of system ticks */
#define CONFIG_SYS_HZ		1000

/******************************************************************************
 * Board
 */

/* Board name */
#define CONFIG_ACTLINUX_BETA	1

/* Initialization routines */
#undef  CONFIG_BOARD_EARLY_INIT_F
#undef  CONFIG_BOARD_EARLY_INIT_R
#define CONFIG_BOARD_LATE_INIT
#undef  CONFIG_BOARD_POSTCLK_INIT

/******************************************************************************
 * Flash
 */

/*
 * This option also enables the building of the cfi_flash driver
 * in the drivers directory
 */
#define CONFIG_FLASH_CFI_DRIVER		1

/*
 * This option displays progress of flash write
 */
#define CONFIG_FLASH_SHOW_PROGRESS	45

/*
 * Define if the flash driver uses extra elements in the
 * common flash structure for storing flash geometry.
 */
#define CONFIG_SYS_FLASH_CFI

/*
 * If this option is defined, the common CFI flash doesn't
 * print it's warning upon not recognized FLASH banks. This
 * is useful, if some of the configured banks are only
 * optionally available.
 */
#undef  CONFIG_SYS_FLASH_QUIET_TEST

/* "flash_print_info" includes empty info */
#define CONFIG_SYS_FLASH_EMPTY_INFO

/* Physical start address of Flash memory */
#define CONFIG_SYS_FLASH_BASE		(0xA0000000)

/* Max number of Flash memory banks */
#define CONFIG_SYS_MAX_FLASH_BANKS	1

/* List of addresses of each Flash memory bank */
#define CONFIG_SYS_FLASH_BANKS_LIST	{ CONFIG_SYS_FLASH_BASE }

/* Max number of sectors on a Flash chip */
#define CONFIG_SYS_MAX_FLASH_SECT	512

/* Timeout for Flash erase operations (in ms) */
#define CONFIG_SYS_FLASH_ERASE_TOUT	3000

/* Timeout for Flash write operations (in ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	3000

/* Timeout for Flash set sector lock bits operations (in ms) */
#define CONFIG_SYS_FLASH_LOCK_TOUT	3000

/* Timeout for Flash clear lock bits operations (in ms) */
#define CONFIG_SYS_FLASH_UNLOCK_TOUT	3000

/*
 * If defined, hardware flash sectors protection is used
 * instead of U-Boot software protection.
 */
#undef  CONFIG_SYS_FLASH_PROTECTION

/*
 * Enable TFTP transfers directly to flash memory;
 * without this option such a download has to be
 * performed in two steps: (1) download to RAM, and (2)
 * copy from RAM to flash.
 *
 * The two-step approach is usually more reliable, since
 * you can check if the download worked before you erase
 * the flash, but in some situations (when system RAM is
 * too limited to allow for a temporary copy of the
 * downloaded image) this option may be very useful.
 */
#undef  CONFIG_SYS_DIRECT_FLASH_TFTP

/* MTD partition list */
#define MTDPARTS_DEFAULT		"mtdparts=physmap-flash.0:" \
					"256k(u-boot)ro," \
					"256k(u-boot-env)ro," \
					"512k(splash)," \
					"8m(kernel)," \
					"-(local)"

/******************************************************************************
 * SDRAM
 */

/* Physical start address of SDRAM. */
#define CONFIG_SYS_SDRAM_BASE		(0x88000000)

/* Size of SDRAM */
#define CONFIG_SYS_SDRAM_SIZE		(256 * 1024 * 1024)

/*
 * Begin and End addresses of the area used by the simple memory test */
#define CONFIG_SYS_MEMTEST_START	(CONFIG_SYS_SDRAM_BASE)
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_SDRAM_BASE + CONFIG_SYS_SDRAM_SIZE)

/* Enable an alternate, more extensive memory test */
#undef  CONFIG_SYS_ALT_MEMTEST

/*
 * Scratch address used by the alternate memory test
 * You only need to set this if address zero isn't writeable
 */
#undef CONFIG_SYS_MEMTEST_SCRATCH

/******************************************************************************
 * Console
 */


#define CONFIG_SCIF				1
#define CONFIG_SCIF_CONSOLE			1
#define CONFIG_CONS_SCIF0			1

/* in bps */
#define CONFIG_BAUDRATE				115200

/* List of legal baudrate settings for this board. */
#define CONFIG_SYS_BAUDRATE_TABLE		{ 115200 }

/* Buffer size for input from the console */
#define CONFIG_SYS_CBSIZE			256

/* Buffer size for Console output */
#define CONFIG_SYS_PBSIZE			256

/* max. Numbfer of arguments accepted for monitor commands */
#define CONFIG_SYS_MAXARGS			16

/* This is what U-Boot prints on the console to prompt for user input. */
#define CONFIG_SYS_PROMPT			"=> "

/*
 * Defined when you want long help messages included;
 * undefine this when you're short of memory.
 */
#define CONFIG_SYS_LONGHELP

/* Suppress display of console information at boot */
#undef  CONFIG_SYS_CONSOLE_INFO_QUIET

/* Enable the call to overwrite_console(). */
#undef  CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE

/* Enable overwrite of previous console environment settings. */
#undef  CONFIG_SYS_CONSOLE_ENV_OVERWRITE

/* Enable temporary baudrate change while serial download */
#undef  CONFIG_SYS_LOADS_BAUD_CHANGE

/******************************************************************************
 * Network
 */

/* Asix AX88796B ethernet driver */
#define CONFIG_DRIVER_AX88796B		1
#define AX88796B_BASE			0xB9000000
#define AX88796B_REG_SHIFT		1

/* Enable random MAC address generation */
#define CONFIG_RANDOM_MACADDR

/******************************************************************************
 * I2C
 */

/* Select a hardware I2C controller */
#define CONFIG_HARD_I2C			1

/*
 * This option allows the use of multiple I2C buses, each of which
 * must have a controller. At any point in time, only one bus is
 * active. To switch to a different bus, use the 'i2c dev' command.
 * Note that bus numbering is zero-based.
 */
#define CONFIG_I2C_MULTI_BUS		1

#define CONFIG_SYS_MAX_I2C_BUS		2
#define CONFIG_SYS_I2C_MODULE		0
#define CONFIG_SYS_I2C_SPEED		100000 /* 100 kHz (standard mode) */
#define CONFIG_SYS_I2C_SLAVE		CONFIG_SYS_I2C_RTC_ADDR
#define CONFIG_SH_I2C			1
#define CONFIG_SH_I2C_CLOCK		CONFIG_SYS_CLK_FREQ
#define CONFIG_SH_I2C_DATA_HIGH		4
#define CONFIG_SH_I2C_DATA_LOW		5
#define CONFIG_SH_I2C_BASE0		0xA4470000
#define CONFIG_SH_I2C_BASE1		0xA4750000

/******************************************************************************
 * RTC
 */

#define CONFIG_RTC_RX8025		1
#define CONFIG_SYS_I2C_RTC_ADDR		0x32

/******************************************************************************
 * Video
 */

//#define CONFIG_CFB_CONSOLE
//#define CONFIG_VGA_AS_SINGLE_DEVICE
#define CONFIG_VIDEO
#define CONFIG_VIDEO_FB_SIZE		(1024 * 768 * 2)
#define CONFIG_VIDEO_RES_MODE		RES_MODE_1024x768
#define CONFIG_VIDEO_LOGO
#define CONFIG_VIDEO_BMP_GZIP

/* splash image may be 4, 8, 24 bpp */
#define CONFIG_SYS_VIDEO_LOGO_MAX_SIZE	(140 + (1024 * 768 * 3))

#define CONFIG_SH_MOBILE_LCD		1
#define CONFIG_SH_MOBILE_LCD_ICKSEL	0u
#define CONFIG_SH_MOBILE_LCD_CLKIN	(CONFIG_SYS_CLK_FREQ * 2)
#define CONFIG_SH_MOBILE_LCD_PLANE	0

#define CONFIG_SPLASH_SCREEN
#define CONFIG_SPLASH_SCREEN_ALIGN
#define CONFIG_SPLASH_ADDR		0x00080000

/******************************************************************************
 * U-boot
 */

/*
 * Physical start address of boot monitor code (set by
 * make config files to be same as the text base address
 * (CONFIG_SYS_TEXT_BASE) used when linking) - same as
 * CONFIG_SYS_FLASH_BASE when booting from flash.
 */
#define CONFIG_SYS_MONITOR_BASE	(CONFIG_SYS_FLASH_BASE)

/*
 * Size of memory reserved for monitor code, used to
 * determine _at_compile_time_ (!) if the environment is
 * embedded within the U-Boot image, or in a separate
 * flash sector
 */
#define CONFIG_SYS_MONITOR_LEN	(256 * 1024)

/* Size of DRAM reserved for malloc() use. */
#define CONFIG_SYS_MALLOC_LEN	((256 * 1024) + CONFIG_VIDEO_FB_SIZE + CONFIG_SYS_VIDEO_LOGO_MAX_SIZE)
//#define CONFIG_SYS_MALLOC_LEN	(256 * 1024)

/*
 * Physical address to load boot monitor code
 * (CONFIG_SYS_SDRAM_BASE + CONFIG_SYS_SDRAM_SIZE - CONFIG_SYS_MONITOR_LEN)
 */
#define CONFIG_SYS_TEXT_BASE	0x97FC0000

/* Default load address for commands like "tftp" or "loads". */
#define CONFIG_SYS_LOAD_ADDR	(CONFIG_SYS_SDRAM_BASE + (16 * 1024 * 1024))

/******************************************************************************
 * Environment
 */

/* Define this if the environment is in flash memory. */
#define CONFIG_ENV_IS_IN_FLASH

/* Size of the sector containing the environment. */
#define CONFIG_ENV_SECT_SIZE	(128 * 1024)

/*
 * If you use this in combination with CONFIG_ENV_IS_IN_FLASH
 * and CONFIG_ENV_SECT_SIZE, you can specify to use only a part
 * of this flash sector for the environment. This saves
 * memory for the RAM copy of the environment.
 *
 * It may also save flash memory if you decide to use this
 * when your environment is "embedded" within U-Boot code,
 * since then the remainder of the flash sector could be used
 * for U-Boot code. It should be pointed out that this is
 * STRONGLY DISCOURAGED from a robustness point of view:
 * updating the environment in flash makes it always
 * necessary to erase the WHOLE sector. If something goes
 * wrong before the contents has been restored from a copy in
 * RAM, your target system will be dead.
 */
#define CONFIG_ENV_SIZE		(CONFIG_ENV_SECT_SIZE)

/*
 * Offset of environment data (variable data) to the
 * beginning of flash memory; for instance, with bottom boot
 * type flash chips the second sector can be used: the offset
 * for this sector is given here.
 *
 * CONFIG_ENV_OFFSET is used relative to CONFIG_SYS_FLASH_BASE
 */
#undef  CONFIG_ENV_OFFSET

/*
 * This is just another way to specify the start address of
 * the flash sector containing the environment (instead of
 * CONFIG_ENV_OFFSET
 */
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN)

/*
 * These settings describe a second storage area used to hold
 * a redundant copy of the environment data, so that there is
 * a valid backup copy in case there is a power failure during
 * a "saveenv" operation.
 */
#define CONFIG_ENV_ADDR_REDUND	(CONFIG_ENV_ADDR + CONFIG_ENV_SIZE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SIZE)

/*
 * If defined, the write protection for vendor parameters
 * (i.e. "serial#" and "ethaddr") is completely disabled.
 * Anybody can change or delete these parameters
 */
#define CONFIG_ENV_OVERWRITE	1

/*
 * If this variable is defined, an environmant variable
 * named "ver" is created by U-Boot showing the U-Boot
 * version as printed by the "version" command.
 * Any change to this variable will be reverted at the
 * next reset
 */
#define CONFIG_VERSION_VARIABLE

/******************************************************************************
 * Boot
 */

/*
 * Delay before automatically booting the default image; (in seconds)
 * set to -1 to disable autoboot.
 * set to -2 to autoboot with no delay and not check for abort
 * (even when CONFIG_ZERO_BOOTDELAY_CHECK is defined).
 */
#define CONFIG_BOOTDELAY	3

/*
 * Only needed when CONFIG_BOOTDELAY is enabled;
 * define a command string that is automaticaly executed
 * when no character is read on the console interface
 * within "Boot Delay" after reset.
 */
#define CONFIG_BOOTCOMMAND	"bootm a0100000"

/*
 * This can be used to pass arguments to the bootm
 * command. The value of CONFIG_BOOTARGS goes into the
 * environment value "bootargs".
 */
#define CONFIG_BOOTARGS		"console=ttySC0,115200 " \
				"root=/dev/ram " \
				"mem=192M " \
				MTDPARTS_DEFAULT

/*
 * Buffer size for Boot Arguments which are passed to
 * the application (usually a Linux kernel) when it is booted
 */
#define CONFIG_SYS_BARGSIZE	512

/*
 * Maximum size of memory mapped by the startup code of
 * the Linux kernel; all data that must be processed by
 * the Linux kernel (bd_info, boot arguments, FDT blob if
 * used) must be put below this limit, unless "bootm_low"
 * environment variable is defined and non-zero. In such case
 * all data for the Linux kernel must be between "bootm_low"
 * and "bootm_low" + CONFIG_SYS_BOOTMAPSZ. The environment
 * variable "bootm_mapsize" will override the value of
 * CONFIG_SYS_BOOTMAPSZ. If CONFIG_SYS_BOOTMAPSZ is undefined,
 * then the value in "bootm_size" will be used instead.
 */
#define CONFIG_SYS_BOOTMAPSZ	(8 * 1024 * 1024)

/*
 * Defining this option allows to add some board-
 * specific code (calling a user-provided function
 * "show_boot_progress(int)") that enables you to
 * show the systems's boot progress on some display
 * (for example, some LED's) on your board.
 */
#undef  CONFIG_SHOW_BOOT_PROGRESS

/******************************************************************************
 * Commands
 */

/* flinfo, erase, protect */
#define CONFIG_CMD_FLASH
/* md, mm, nm, mw, cp, cmp, crc, base, loop, loopw, mtest */
#define CONFIG_CMD_MEMORY
/* bootp, tftpboot, rarpboot */
#define CONFIG_CMD_NET
/* send ICMP ECHO_REQUEST to network host */
#define CONFIG_CMD_PING
/* MII utility commands */
#define CONFIG_CMD_MII
/* nfs */
#define CONFIG_CMD_NFS
/* I2C serial bus support */
#define CONFIG_CMD_I2C
/* date */
#define CONFIG_CMD_DATE
/* print SDRAM configuration information (requires CONFIG_CMD_I2C) */
#define CONFIG_CMD_SDRAM
/* printenv, setenv */
#define CONFIG_CMD_ENV
/* saveenv */
#define CONFIG_CMD_SAVEENV
/* run */
#define CONFIG_CMD_RUN

#endif /* __ACTLINUX_ALPHA_H */

