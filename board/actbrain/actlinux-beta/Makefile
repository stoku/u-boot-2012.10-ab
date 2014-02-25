#
# Copyright (C) 2013 Sosuke Tokunaga <sosuke.tokunaga@courier-systems.com>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the Lisence, or (at your option) any later version.
#
# This program is distributed in the hope that it will be helpful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
# GNU General Public License for more details.
#

include $(TOPDIR)/config.mk

LIB	:= $(obj)lib$(BOARD).o

COBJS	:= actlinux-beta.o
SOBJS	:= lowlevel_init.o

SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS) $(SOBJS))

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

include $(SRCTREE)/rules.mk

sinclude $(obj).depend

