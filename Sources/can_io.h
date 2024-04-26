/* ***	$Header: /home/uwe/sources/socketCAN/RCS/can_io.h,v 1.1 2007/08/20 19:52:13 uwe Sav uwe $  ***
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
 * CAN I/O Interface.
 *
 * For berliOS socketCAN over PEAK PCAN-USB Dongle. For information
 * about socketCAN see "http://socketcan.berlios.de/".
 * 
 * Written by Uwe Vogt, UV Software <http://www.uv-software.de/>
 */

#ifndef __CAN_IO_H
#define __CAN_IO_H


/* ***	includes  ***
 */

#include "can_defs.h"


/* ***	defines  ***
 */

#ifndef _CAN_DEFS
 #define CANBDR_1000				 0	/* baud rate: 1000 kBit/s			*/
 #define CANBDR_800					 1	/* baud rate:  800 kBit/s			*/
 #define CANBDR_500					 2	/* baud rate:  500 kBit/s			*/
 #define CANBDR_250					 3	/* baud rate:  250 kBit/s			*/
 #define CANBDR_125					 4	/* baud rate:  125 kBit/s			*/
 #define CANBDR_100					 5	/* baud rate:  100 kBit/s			*/
 #define CANBDR_50					 6	/* baud rate:   50 kBit/s			*/
 #define CANBDR_20					 7	/* baud rate:   20 kBit/s			*/
 #define CANBDR_10					 8	/* baud rate:   10 kBit/s			*/

 #define CANERR_NOERROR				 0	/* no error!						*/
 #define CANERR_BOFF				-1	/* CAN - busoff status				*/
 #define CANERR_EWRN				-2 	/* CAN - error warning status		*/
 #define CANERR_BERR				-3	/* CAN - bus error					*/
 #define CANERR_OFFLINE				-9	/* CAN - not started				*/
 #define CANERR_ONLINE				-8	/* CAN - already started			*/
 #define CANERR_MSG_LST				-10	/* CAN - message lost				*/
 #define CANERR_LEC_STUFF			-11	/* LEC - stuff error				*/
 #define CANERR_LEC_FORM			-12	/* LEC - form error					*/
 #define CANERR_LEC_ACK				-13	/* LEC - acknowledge error			*/
 #define CANERR_LEC_BIT1			-14	/* LEC - recessive bit error		*/
 #define CANERR_LEC_BIT0			-15	/* LEC - dominant bit error			*/
 #define CANERR_LEC_CRC				-16	/* LEC - checksum error				*/
 #define CANERR_TX_BUSY				-20	/* USR - transmitter busy			*/
 #define CANERR_RX_EMPTY			-30	/* USR - receiver empty				*/
 #define CANERR_TIMEOUT				-50	/* USR - time-out					*/
 #define CANERR_BAUDRATE			-91	/* USR - illegal baudrate			*/
 #define CANERR_ILLPARA				-93	/* USR - illegal parameter			*/
 #define CANERR_NULLPTR				-94	/* USR - null-pointer assignement	*/
 #define CANERR_NOTINIT				-95	/* USR - not initialized			*/
 #define CANERR_YETINIT				-96	/* USR - already initialized		*/
 #define CANERR_NOTSUPP				-98	/* USR - not supported				*/
 #define CANERR_FATAL				-99	/* USR - other errors				*/

 #define CANSTAT_RESET				0x80/* CAN status: controller stopped	*/
 #define CANSTAT_BOFF				0x40/* CAN status: busoff status		*/
 #define CANSTAT_EWRN				0x20/* CAN status: error warning level	*/
 #define CANSTAT_BERR				0x10/* CAN status: bus error (LEC)		*/
 #define CANSTAT_TX_BUSY			0x08/* CAN status: transmitter busy		*/
 #define CANSTAT_RX_EMPTY			0x04/* CAN status: receiver empty		*/
 #define CANSTAT_MSG_LST			0x02/* CAN status: message lost			*/
 #define CANSTAT_QUE_OVR			0x01/* CAN status: event-queue overrun	*/
#endif

/* ***	types  ***
 */

typedef struct _can_msg_t {				/* CAN message:						*/
	unsigned long id;					/*   CAN identifier					*/
	int rtr;							/*   flag: RTR frame				*/
	int ext;							/*   flag: extended format			*/
	unsigned char dlc;					/*   data length code				*/
	unsigned char data[8];				/*   data bytes						*/
	struct _timestamp {					/*   time stamp:					*/
		long sec;						/*     seconds (relative)			*/
		long usec;						/*     microseconds					*/
	} timestamp;
}	can_msg_t;


/* ***	variables  ***
 */

extern int can_baudrate;				/* index to the bit-timing table	*/


/* ***	prototypes  ***
 */

int can_init(int board, void *param);
/*
 * function : 
 *
 * parameter: board - type of the CAN Controller interface.
 *	          param - pointer to board-specific parameters.
 *
 * result   : 0 if successful, or a negative value on error.
 */

int can_exit(void);
/*
 * function : 
 *
 * parameter: (none)
 *
 * result   : 0 if successful, or a negative value on error.
 */

int can_start(int baudrate);
/*
 * function : 
 *
 * parameter: baudrate - index (0,..,8) to the bit-timing table, or (-1).
 *
 * result   : 0 if successful, or a negative value on error.
 */

int can_reset(void);
/*
 * function : 
 *
 * parameter: (none)
 *
 * result   : 0 if successful, or a negative value on error.
 */

int can_transmit(can_msg_t *msg);
/*
 * function : 
 *
 * parameter: msg - pointer to the message to send.
 *
 * result   : 0 if successful, or a negative value on error.
 */

int can_receive(can_msg_t *msg);
/*
 * function : 
 *
 * parameter: msg - pointer to a message buffer.
 *
 * result   : 0 if successful, or a negative value on error.
 */

int can_status(unsigned char *status);
/*
 * function : 
 *
 * parameter: status - 8-bit status register.
 *
 * result   : 0 if successful, or a negative value on error.
 */

int can_busload(unsigned char *load, unsigned char *status);
/*
 * function : 
 *
 * parameter: load   - bus-load in [percent].
 *	          status - 8-bit status register.
 *
 * result   : 0 if successful, or a negative value on error.
 */

char *can_hardware(void);
/*
 * function : retrieves the hardware version of the CAN Controller
 *	          as a zero-terminated string.
 *
 * parameter: (none)
 *
 * result   : pointer to a zero-terminated string, or NULL on error.
 */

char *can_software(void);
/*
 * function : retrieves the firmware version of the CAN Controller
 *	          as a zero-terminated string.
 *
 * parameter: (none)
 *
 * result   : pointer to a zero-terminated string, or NULL on error.
 */

char *can_version(void);
/*
 * function : retrieves version information of the CAN Controller API
 *	          as a zero-terminated string.
 *
 * parameter: (none)
 *
 * result   : pointer to a zero-terminated string, or NULL on error.
 */

#endif

/* ***	end of file  ***
 */
 
