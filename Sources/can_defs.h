/* ***	$Header: /home/uwe/sources/socketCAN/RCS/can_defs.h,v 1.1 2007/08/20 19:52:42 uwe Sav uwe $  ***
 *
 * Copyright (C) 2007 UV Software, Friedrichshafen.
 *
 * http://www.uv-software.de/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * CAN Definitions and Options.
 *
 * For berliOS socketCAN over PEAK PCAN-USB Dongle. For information
 * about socketCAN see "http://socketcan.berlios.de/".
 * 
 * Written by Uwe Vogt, UV Software <http://www.uv-software.de/>
 */

#ifndef __CAN_DEFS_H
#define __CAN_DEFS_H


/* ***	options  ***
 */


/* ***	defines  ***
 */

#ifndef _CAN_DEFS						/* socketCAN interfaces				*/

 #define CAN_NETDEV				(-1L)	/*   it´s a network device			*/
 
 struct _can_param						/*   installation parameters:		*/
 {
 	char* ifname;						/*     interface name				*/
 	int   family;						/*     protocol family				*/
 	int   type;							/*     communication semantics		*/
 	int   protocol;						/*     protocol to be used 			*/
 };
 #define CANBDR_SOCKET			(-1)	/*   socketCAN baud rate (external!) */
 #define CANERR_SOCKET			(-10000)/*   socketCAN error ('errno' is set)*/
#endif

#endif

/* ***	end of file  ***
 */
 
