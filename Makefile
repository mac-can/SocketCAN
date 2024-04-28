#
#	SocketCAN - CAN API V3 for SocketCAN Interfaces on Linux
#
#	Copyright (c) 2007,2019-2024  Uwe Vogt, UV Software, Berlin (info@uv-software.com)
#
#	This program is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; either version 2 of the License, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program; if not, write to the Free Software
#	Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#
all:
	@./build_no.sh
	@echo "\033[1mBuilding SocketCAN...\033[0m"
	$(MAKE) -C Trial $@
	$(MAKE) -C Library $@
	$(MAKE) -C Utilities/can_test $@
	$(MAKE) -C Utilities/can_moni $@

clean:
	$(MAKE) -C Trial $@
	$(MAKE) -C Library $@
	$(MAKE) -C Utilities/can_test $@
	$(MAKE) -C Utilities/can_moni $@

install:
#	$(MAKE) -C Trial $@
	$(MAKE) -C Library $@
#	$(MAKE) -C Utilities/can_test $@
#	$(MAKE) -C Utilities/can_moni $@

build_no:
	@./build_no.sh
	@cat Sources/can_vers.h
