# #############################################################################
# Copyright (C) 2007 by UV Software, Friedrichshafen, Germany.
#
# http://www.uv-software.de/
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
# Makefile for can_moni (CAN Monitor using socketCAN)
#
# For berliOS socketCAN over PEAK PCAN-USB Dongle. For information
# about socketCAN see "http://socketcan.berlios.de/".
#
# Written by Uwe Vogt, UV Software <http://www.uv-software.de/>
# #############################################################################

CFLAGS	= -O2 -Wall -Wno-parentheses \
	  -fno-strict-aliasing \
	  -DPF_CAN=29 \
	  -DAF_CAN=PF_CAN

PROGRAM	= can_moni

OBJECTS = main.o can_io.o

MAIN_DEPS = can_io.h can_defs.h

CAN_IO_DEPS = can_io.h can_defs.h


all: $(PROGRAM)

clean:
	rm -f $(PROGRAM) *.o

install:
	cp -f $(PROGRAM) /usr/local/bin

distclean:
	rm -f $(PROGRAM) *.o *~


main.o: main.c $(MAIN_DEPS)

can_io.o: can_io.c $(CAN_IO_DEPS)


can_moni: $(OBJECTS)
	gcc -o $(PROGRAM) $(OBJECTS)


# #############################################################################