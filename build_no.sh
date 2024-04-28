#!/bin/sh
echo "/*  -- Do not commit this file --" > ./Sources/can_vers.h
echo " *" >> ./Sources/can_vers.h
echo " *  SocketCAN - CAN API V3 for SocketCAN Interfaces on Linux" >> ./Sources/can_vers.h
echo " *" >> ./Sources/can_vers.h
echo " *  Copyright (c) 2007,2019-2024  Uwe Vogt, UV Software, Berlin (info@uv-software.com)" >> ./Sources/can_vers.h
echo " *" >> ./Sources/can_vers.h
echo " *  This program is free software; you can redistribute it and/or modify" >> ./Sources/can_vers.h
echo " *  it under the terms of the GNU General Public License as published by" >> ./Sources/can_vers.h
echo " *  the Free Software Foundation; either version 2 of the License, or" >> ./Sources/can_vers.h
echo " *  (at your option) any later version." >> ./Sources/can_vers.h
echo " *" >> ./Sources/can_vers.h
echo " *  This program is distributed in the hope that it will be useful," >> ./Sources/can_vers.h
echo " *  but WITHOUT ANY WARRANTY; without even the implied warranty of" >> ./Sources/can_vers.h
echo " *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the" >> ./Sources/can_vers.h
echo " *  GNU General Public License for more details." >> ./Sources/can_vers.h
echo " *" >> ./Sources/can_vers.h
echo " *  You should have received a copy of the GNU General Public License" >> ./Sources/can_vers.h
echo " *  along with this program; if not, write to the Free Software" >> ./Sources/can_vers.h
echo " *  Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA." >> ./Sources/can_vers.h
echo " */" >> ./Sources/can_vers.h
echo "#ifndef BUILD_NO_H_INCLUDED" >> ./Sources/can_vers.h
echo "#define BUILD_NO_H_INCLUDED" >> ./Sources/can_vers.h
git log -1 --pretty=format:%h > /dev/null 2> /dev/null
if [ $? -eq 0 ]
then
    echo "#define BUILD_NO 0x"$(git log -1 --pretty=format:%h) >> ./Sources/can_vers.h
else
    echo "#define BUILD_NO 0xDEADC0DE" >> ./Sources/can_vers.h
fi
echo "#define STRINGIFY(X) #X" >> ./Sources/can_vers.h
echo "#define TOSTRING(X) STRINGIFY(X)" >> ./Sources/can_vers.h
echo "#endif" >> ./Sources/can_vers.h
