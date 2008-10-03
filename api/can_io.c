/* ***	$Header: /home/uwe/sources/socketCAN/RCS/can_io.c,v 1.1 2007/08/20 19:50:23 uwe Sav uwe $  ***
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

static char _id[] = "$Id: can_io.c,v 1.1 2007/08/20 19:50:23 uwe Sav uwe $";


/* ***	includes  ***
 */

#include "can_io.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include <linux/can.h>
#include <linux/can/raw.h>


/* ***	defines  ***
 */

#ifndef OK
#define OK		0
#endif
#ifndef TRUE
#define TRUE	(0==0)
#endif
#ifndef FALSE
#define FALSE	(0!=0)
#endif

/* ***	types  ***
 */


/* ***	prototypes  ***
 */


/* ***	variables  ***
 */

int can_baudrate = -1;					/* index to the bit-timing table	*/

static int   fd = -1;					/* file descriptor (it´s a socket)	*/
static char  ifname[IFNAMSIZ] = "";		/* interface name					*/
static int   family = PF_CAN;			/* protocol family					*/
static int   type = SOCK_RAW;			/* communication semantics			*/
static int   protocol = CAN_RAW;		/* protocol to be used with the socket	*/
static char  hardware[256];				/* hardware version of the CAN interface board	*/
static char  software[256];				/* software version of the PCAN-Light interface	*/
static int   init = FALSE;				/* initialization flag of interface	*/

static const int bit_timing[9] = {		/* bit-timing table:				*/
	1000,								/*   1000 Kbps						*/
	/* n/a */ 0,						/*    800 Kbps						*/
	500,								/*    500 Kbps						*/
	250,								/*    250 Kbps						*/
	125,								/*    125 Kbps						*/
	100,								/*    100 Kbps						*/
	50,									/*     50 Kbps						*/
	20,									/*     20 Kbps						*/
	10									/*     10 Kbps						*/
};
static unsigned char can_state = 0x80;	/* 8-bit status register			*/


/* ***	functions  ***
 */

int can_init(int board, void *param)
{
	struct ifreq ifr;
	struct sockaddr_can addr;
	can_err_mask_t  err_mask = CAN_ERR_MASK;
	
	if(init)							/* must not be initialized!			*/
		return CANERR_YETINIT;
	switch(board)						/* supported CAN boards				*/
	{
	case CAN_NETDEV:					/*   socketCAN interface			*/
		if(param == NULL)				/*     null-pointer assignement?	*/
			return CANERR_NULLPTR;		/*       error!						*/
		
		strncpy(ifname, ((struct _can_param*)param)->ifname, IFNAMSIZ);
		family = ((struct _can_param*)param)->family;
		type = ((struct _can_param*)param)->type;
		protocol = ((struct _can_param*)param)->protocol;
		
		if((fd = socket(family, type, protocol)) < 0)
		{
			return CANERR_SOCKET;
		}
		strcpy(ifr.ifr_name, ifname);
		ioctl(fd, SIOCGIFINDEX, &ifr);

		addr.can_family = family;
		addr.can_ifindex = ifr.ifr_ifindex;

		if(bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		{
			return CANERR_SOCKET;
    	}
    	/*@ToDo: reset CAN controller?
    	 *       (not supported on berliOS)
    	 */
		setsockopt(fd, SOL_CAN_RAW, CAN_RAW_ERR_FILTER, &err_mask, sizeof(err_mask));
		break;
	default:							/*   unknown CAN board				*/
		return CANERR_ILLPARA;
	}
	can_state = 0x80;					/* CAN controller not started yet!	*/
	init = TRUE;						/* set initialization flag			*/
	return OK;
}

int can_exit(void)
{
	if(init)							/* must be initialized!				*/
	{
		close(fd);						/*   close the socket				*/
	}
	can_state |= 0x80;					/* CAN controller in INIT state		*/
	strcpy(ifname, "");					/* interface name					*/
	family = PF_CAN;					/* protocol family					*/
	type = SOCK_RAW;					/* communication semantics			*/
	protocol = CAN_RAW;					/* protocol to be used with the socket */
	init = FALSE;						/* clear initialization flag		*/
	return OK;
}

int can_start(int baudrate)
{
	if(!init)							/* must be initialized!				*/
		return CANERR_NOTINIT;
	if((can_state & 0x80) != 0x80)		/* must be stopped!					*/
		return CANERR_ONLINE;
	/*if((baudrate < CANBDR_1000) || (CANBDR_10 < baudrate) || (CANBDR_800 == baudrate))*/
	if(baudrate != CANBDR_SOCKET)
		return CANERR_BAUDRATE;
	/*@ToDo: set baud rate!
	 *       (not supported on berliOS)
	 */
	/*@ToDo: start CAN controller?
	 *       (not supported on berliOS)
	 */
	can_baudrate = baudrate;			/* index to the bit-timing table	*/
	can_state = 0x00;				 	/* CAN controller started!			*/
	return OK;
}

int can_reset(void)
{
	if(!init)							/* must be initialized!				*/
		return CANERR_NOTINIT;
	if((can_state & 0x80) != 0x80) {	/* CAN started, then reset			*/
		if(can_baudrate == CANBDR_800)	/*   800 kBit/s not supported!		*/
			return CANERR_BAUDRATE;		/*     ==> error!					*/
    	/*@ToDo: reset CAN controller?
    	 *       (not supported on berliOS)
    	 */
	}
	can_state |= 0x80;	 				/* CAN controller stopped!			*/
	return OK;
}

int can_transmit(can_msg_t *msg)
{
	struct can_frame frame;

	if(!init)							/* must be initialized!				*/
		return CANERR_NOTINIT;
	if((can_state & 0x80) == 0x80)		/* must be running!					*/
		return CANERR_OFFLINE;
	if(msg == NULL)						/* null-pointer assignment!			*/
		return CANERR_NULLPTR;
	if(msg->dlc > 8)					/* data length 0 .. 8				*/
		return CANERR_ILLPARA;
		
	frame.can_id  = msg->id;
	frame.can_id |= msg->ext? CAN_EFF_FLAG : 0;
	frame.can_id |= msg->rtr? CAN_RTR_FLAG : 0;
	frame.can_dlc = msg->dlc;
	memcpy(frame.data, msg->data, msg->dlc);

	if(write(fd, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame)) {
		/*@ToDo: return codes are not quite clear!
		 */
		can_state |= CANSTAT_TX_BUSY;
		return CANERR_TX_BUSY;
	}
	if(ioctl(fd, SIOCGSTAMP, &msg->timestamp) > 0) {
		msg->timestamp.sec = 0;
		msg->timestamp.usec = 0;
	}
	can_state &= ~CANSTAT_TX_BUSY;
	return CANERR_NOERROR;
}

int can_receive(can_msg_t *msg)
{
	fd_set rdfs;
	struct timeval timeo;
	struct can_frame frame;
	
    FD_ZERO(&rdfs);
    FD_SET(fd, &rdfs);
    
    timeo.tv_sec  = 0;
    timeo.tv_usec = 0;

	if(!init)							/* must be initialized!				*/
		return CANERR_NOTINIT;
	if((can_state & 0x80) == 0x80)		/* must be running!					*/
		return CANERR_OFFLINE;
	if(msg == NULL)						/* null-pointer assignment!			*/
		return CANERR_NULLPTR;

    if(select(fd+1, &rdfs, NULL, NULL, &timeo) < 0) {
		can_state |= CANSTAT_RX_EMPTY;		
		return CANERR_RX_EMPTY;			/* no data present					*/
    }
	if(!FD_ISSET(fd, &rdfs)) {			/* no message read?					*/
		can_state |= CANSTAT_RX_EMPTY;		
		return CANERR_RX_EMPTY;			/*   receiver empty!				*/
	}
	if(read(fd, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame)) {
		/*@ToDo: return codes are not quite clear!
		 */
		can_state |= CANSTAT_MSG_LST;		
		return CANERR_MSG_LST;			/*   message lost!					*/
	}
	if((frame.can_id & CAN_ERR_FLAG) != CAN_ERR_FLAG) {
		msg->id  = (frame.can_id & CAN_ERR_MASK);
		msg->ext = (frame.can_id & CAN_EFF_FLAG)? 1 : 0;
		msg->rtr = (frame.can_id & CAN_RTR_FLAG)? 1 : 0;
		msg->dlc = (frame.can_dlc);
		memcpy(msg->data, frame.data, frame.can_dlc);
		if(ioctl(fd, SIOCGSTAMP, &msg->timestamp) > 0) {
			msg->timestamp.sec = 0;
			msg->timestamp.usec = 0;
		}
		can_state &= ~CANSTAT_RX_EMPTY;		
		can_state &= ~CANSTAT_MSG_LST;		
		return CANERR_NOERROR;
	}
	else {
		/*@ToDo: handle error frames
		 */
		/* *** **
		can_state.b.bus_off = (can_msg.DATA[3] & CAN_ERR_BUSOFF) != CAN_ERR_OK;
		can_state.b.bus_error = (can_msg.DATA[3] & CAN_ERR_BUSHEAVY) != CAN_ERR_OK;
		can_state.b.warning_level = (can_msg.DATA[3] & CAN_ERR_BUSLIGHT) != CAN_ERR_OK;
		can_state.b.message_lost |= (can_msg.DATA[3] & CAN_ERR_OVERRUN) != CAN_ERR_OK;
		** *** */
		can_state |= CANSTAT_RX_EMPTY;		
		return CANERR_RX_EMPTY;			/*   receiver empty!				*/
	}
}

int can_status(unsigned char *status)
{
	if(!init)							/* must be initialized!				*/
		return CANERR_NOTINIT;
/*@ToDo: status
 */
return CANERR_NOTSUPP;
	if(status)							/* status-register					*/
	  *status = can_state;
	can_state &= 0xF0;					/* clear low nibble					*/
	return OK;
}

int can_busload(unsigned char *load, unsigned char *status)
{
	if(!init)							/* must be initialized!				*/
		return CANERR_NOTINIT;
/*@ToDo: status
 */
return CANERR_NOTSUPP;
	if(load)							/* bus-load							*/
	  *load = 0;
	if(status)							/* status-register					*/
	  *status = can_state;
	can_state &= 0xF0;					/* clear low nibble					*/
	return OK;
}

char *can_hardware(void)
{
	sprintf(hardware, "interface=\"%s\", family=%d, type=%d, protocol=%d", ifname, family, type, protocol);
	return (char*)hardware;
}

char *can_software(void)
{
	sprintf(software, "berliOS socketCAN <http://socketcan.berlios.de/>");
	return (char*)software;
}

/* ***	local functions  ***
 */


/* ***	revision control  ***
 */

char *can_version()
{
	return (char*)_id;
}

/* ***	end of file  ***
 */
