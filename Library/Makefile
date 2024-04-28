# #############################################################################
# Copyright (C) 2015-2024 Uwe Vogt, UV Software, Berlin (info@uv-software.de)
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
# Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#
# Makefile for 'libuvcansoc.so' (CAN API V2 for SocketCAN interfaces)
#
# For berliOS socketCAN over PEAK PCAN-USB Dongle. For information
# about socketCAN see "http://socketcan.berlios.de/".
#
# Written by Uwe Vogt, UV Software <http://www.uv-software.de/>
# #############################################################################

DRV	= ../Sources

VERSION = 0.1

LIBRARY = libuvcansoc

SONAME  = $(LIBRARY).so.3
TARGET  = $(SONAME).$(VERSION)
STATIC  = $(LIBRARY).a

INSTALL = /usr/local/lib

OBJECTS = $(OUTDIR)/can_api.o

CAN_API_DEPS = $(DRV)/can_api.h $(DRV)/can_defs.h

CFLAGS	= -fPIC -Wall -Wno-parentheses \
	-fno-strict-aliasing \
	-DOPTION_CANAPI_SO \
	-I$(DRV)

LDFLAGS += --shared -Wl,-soname,$(SONAME)

ARFLAGS = r

LIBRARIES = -lc -lpthread

CC = gcc
LD = gcc
AR = ar
RM = rm -f
CP = cp -f
LN = ln -s

OUTDIR = .objects


.PHONY: outdir


all: outdir $(STATIC) $(TARGET)

outdir:
	@mkdir -p $(OUTDIR)

clean:
	$(RM) $(TARGET) $(STATIC) $(OUTDIR)/*.o $(OUTDIR)/*.d

install:
	@echo "Copying library file..."
	$(RM) $(INSTALL)/$(TARGET) ; $(CP) $(TARGET) $(INSTALL)
	$(RM) $(INSTALL)/$(SONAME) ; $(LN) $(INSTALL)/$(TARGET) $(INSTALL)/$(SONAME)
	$(RM) $(INSTALL)/$(LIBRARY).so ; $(LN) $(INSTALL)/$(SONAME) $(INSTALL)/$(LIBRARY).so


$(OUTDIR)/can_api.o: $(DRV)/can_api.c $(CAN_API_DEPS)
	$(CC) $(CFLAGS) -o $@ -c $<


$(STATIC): $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $(OBJECTS)
	@echo "Target '"$@"' successfully build"

$(TARGET): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBRARIES)
	@echo "Target '"$@"' successfully build"


# #############################################################################