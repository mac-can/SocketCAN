/*  CAN API V3 for SocketCAN Interfaces
 *
 *  Copyright (C) 2007,2010 Uwe Vogt, UV Software, Friedrichshafen.
 *  Copyright (C) 2015-2024 Uwe Vogt, UV Software, Berlin (info@uv-software.de).
 *
 *  http://www.uv-software.de/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *  CAN Definitions and Options.
 *
 *  For berliOS socketCAN over PEAK PCAN-USB Dongle. For information
 *  about socketCAN see "http://socketcan.berlios.de/".
 * 
 *  Written by Uwe Vogt, UV Software <http://www.uv-software.de/>
 */
/** @addtogroup  can_api
 *  @{
 */
#ifndef CAN_DEFS_H_INCLUDED
#define CAN_DEFS_H_INCLUDED

/*  -----------  includes  -----------------------------------------------
 */

#include <stdint.h>                     /* C99 header for sized integer types */
#include <stdbool.h>                    /* C99 header for boolean type */


/*  -----------  options  ------------------------------------------------
 */


/*  -----------  defines  ------------------------------------------------
 */

#ifndef _CAN_DEFS                       /* SocketCAN interfaces          */

 #define CAN_NETDEV             (-1L)   /*   it´s a network device!      */

 struct _can_netdev_param               /*   installation parameters:    */
 {
    char* ifname;                       /*     interface name            */
    int   family;                       /*     protocol family           */
    int   type;                         /*     communication semantics   */
    int   protocol;                     /*     protocol to be used       */
 };
 #define CANBDR_SOCKET          (-1)    /*   SocketCAN bit-rate (external!) */
 #define CANERR_SOCKET          (-10000)/*   SocketCAN error ('errno' is set) */

 #define SOCKETCAN_BOARDS       (1)     /*   CAN interfaces are maintained by Linux kernel */

 #define SOCKETCAN_MAX_HANDLES  (128)   /*   maximum number of interface handles */

 #define SOCKETCAN_LIB_ID       (1000)  /*   SocketCAN library id         */
 #define SOCKETCAN_LIB_NAME                 "libuvcansoc.so.3"

 #define CAN_API_VENDOR                     "UV Software, Berlin"
 #define CAN_API_AUTHOR                     "Uwe Vogt, UV Software"
 #define CAN_API_WEBSITE                    "www.uv-software.com"
 #define CAN_API_CONTACT                    "info@uv-software.com"
 #define CAN_API_LICENSE                    "GPL-2.0-or-later"
 #define CAN_API_COPYRIGHT                  "Copyright (C) 2005-20%02u, UV Software, Berlin"
 #define CAN_API_HAZARD_NOTE                "Do not connect your CAN device to a real CAN network when using this program.\n" \
                                            "This can damage your application."
#endif
#endif /* CAN_DEFS_H_INCLUDED */
/** @}
 */
/*  ----------------------------------------------------------------------
 *  Uwe Vogt,  UV Software,  Chausseestrasse 33 A,  10115 Berlin,  Germany
 *  Tel.: +49-30-46799872,  Fax: +49-30-46799873,  Mobile: +49-170-3801903
 *  E-Mail: uwe.vogt@uv-software.de,  Homepage: http://www.uv-software.de/
 */
