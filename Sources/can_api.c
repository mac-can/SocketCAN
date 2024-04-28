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
 *  CAN I/O Interface.
 *
 *  For berliOS socketCAN over PEAK PCAN-USB Dongle. For information
 *  about socketCAN see "http://socketcan.berlios.de/".
 * 
 *  Written by Uwe Vogt, UV Software <http://www.uv-software.de/>
 */
/** @addtogroup  can_api
 *  @{
 */
#include "can_vers.h"
#define VERSION_MAJOR     0
#define VERSION_MINOR     1
#define VERSION_PATCH     0
#define VERSION_BUILD     BUILD_NO
#define VERSION_STRING    TOSTRING(VERSION_MAJOR)"." TOSTRING(VERSION_MINOR)"."TOSTRING(VERSION_PATCH)"-"TOSTRING(BUILD_NO)
#if defined(__linux__)
#define PLATFORM  "Linux"
#else
#error Platform not supported
#endif
#ifdef _DEBUG
    static char _id[] = "CAN API V3 for SocketCAN Interfaces, Version "VERSION_STRING" ("PLATFORM") _DEBUG";
#else
    static char _id[] = "CAN API V3 for SocketCAN Interfaces, Version "VERSION_STRING" ("PLATFORM")";
#endif

/*  -----------  includes  -----------------------------------------------
 */
#include "can_defs.h"
#include "can_api.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include <linux/can.h>
#include <linux/can/raw.h>
#include <linux/sockios.h>


/*  -----------  options  ------------------------------------------------
 */
#if (OPTION_CAN_2_0_ONLY != 0)
#ifdef _MSC_VER
#pragma message ( "Compilation with legacy CAN 2.0 frame format!" )
#else
#warning Compilation with legacy CAN 2.0 frame format!
#endif
#endif
#if (OPTION_CANAPI_SO != 0)
__attribute__((constructor))
static void _initializer() {
    // default initializer
}
__attribute__((destructor))
static void _finalizer() {
    // default finalizer
}
#define EXPORT  __attribute__((visibility("default")))
#else
#define EXPORT
#endif

/*  -----------  defines  ------------------------------------------------
 */
#ifndef CAN_MAX_HANDLES
#define CAN_MAX_HANDLES         (128)   // maximum number of open handles
#else
#define CAN_MAX_HANDLES         SOCKETCAN_MAX_HANDLES
#endif
#define INVALID_HANDLE          (-1)
#define IS_HANDLE_VALID(hnd)    ((0 <= (hnd)) && ((hnd) < CAN_MAX_HANDLES))
#ifndef DLC2LEN
#define DLC2LEN(x)              dlc_table[(x) & 0xF]
#endif
#ifndef LEN2DLC
#define LEN2DLC(x)              len2dlc(x)
#endif

/*  -----------  types  --------------------------------------------------
 */
typedef struct {                        // frame counters:
    uint64_t tx;                        //   number of transmitted CAN frames
    uint64_t rx;                        //   number of received CAN frames
    uint64_t err;                       //   number of receiced error frames
} can_counter_t;

typedef struct {                        // SocketCAN interface:
    int   fd;                           // file descriptor (it´s a socket)
    char  ifname[IFNAMSIZ+1];           // interface name
    int   family;                       // protocol family
    int   type;                         // communication semantics
    int   protocol;                     // protocol to be used with the socket
    can_mode_t mode;                    //   operation mode of the CAN channel
    can_status_t status;                //   8-bit status register
    can_counter_t counters;             //   statistical counters
} can_interface_t;


/*  -----------  prototypes  ---------------------------------------------
 */

#ifndef CAN_20_ONLY
 static uint8_t len2dlc(uint8_t len);
#endif
static int lib_parameter(uint16_t param, void *value, size_t nbytes);
static int drv_parameter(int handle, uint16_t param, void *value, size_t nbytes);


/*  -----------  variables  ----------------------------------------------
 */

#ifdef _CANAPI_EXPORTS
#define ATTRIB  __declspec(dllexport)
#else
#define ATTRIB
#endif
ATTRIB can_board_t can_boards[SOCKETCAN_BOARDS+1] = // list of CAN Interface boards:
{
    {CAN_NETDEV,                          "can?"},
    {EOF, NULL}
};
#ifndef CAN_20_ONLY
 static const uint8_t dlc_table[16] = { // DLC to length
    0,1,2,3,4,5,6,7,8,12,16,20,24,32,48,64
 };
#endif
static can_interface_t can[CAN_MAX_HANDLES];  // interface handles
static int init = 0;                    // initialization flag


/*  -----------  functions  ----------------------------------------------
 */
#if (1)
EXPORT
int can_test(int32_t board, uint8_t mode, const void *param, int *result)
{
    int rc = CANERR_FATAL;              /* return value */

    // TODO: ...

    return rc;
}

EXPORT
int can_init(int32_t board, uint8_t mode, const void *param)
{
    int fd;
    struct ifreq ifr;
    struct sockaddr_can addr;
    can_err_mask_t  err_mask = CAN_ERR_MASK;
    int i;

    if(!init) {                         // when not init before:
        for(i = 0; i < CAN_MAX_HANDLES; i++) {
            can[i].fd = -1;
            can[i].ifname[0] = '\0';
            can[i].family = PF_CAN;
            can[i].type = SOCK_RAW;
            can[i].protocol = CAN_RAW;
            can[i].mode.byte = CANMODE_DEFAULT;
            can[i].status.byte = CANSTAT_RESET;
            can[i].counters.tx = 0ull;
            can[i].counters.rx = 0ull;
            can[i].counters.err = 0ull;
        }
        init = 1;                       // set initialization flag
    }
    for(i = 0; i < CAN_MAX_HANDLES; i++) {
        if(can[i].fd == -1)             // get an unused handle, if any
            break;
    }
    if(!IS_HANDLE_VALID(i))             // no free handle found
        return CANERR_HANDLE;

    switch(board)                       // supported CAN boards:
    {
    case CAN_NETDEV:                    //   SocketCAN interface
        if(param == NULL)               //     null-pointer assignement?
            return CANERR_NULLPTR;      //       error!

        strncpy(can[i].ifname, ((struct _can_netdev_param*)param)->ifname, IFNAMSIZ);
        can[i].ifname[IFNAMSIZ] = '\0';
        can[i].family = ((struct _can_netdev_param*)param)->family;
        can[i].type = ((struct _can_netdev_param*)param)->type;
        can[i].protocol = ((struct _can_netdev_param*)param)->protocol;

        if((fd = socket(can[i].family, can[i].type, can[i].protocol)) < 0)
        {
            return CANERR_SOCKET;       //   errno is set!
        }
        memset(&ifr.ifr_name, 0, sizeof(ifr.ifr_name));
        strcpy(ifr.ifr_name, can[i].ifname);
        ioctl(fd, SIOCGIFINDEX, &ifr);

        addr.can_family = can[i].family;
        addr.can_ifindex = ifr.ifr_ifindex;

        if(err_mask)
        {
            setsockopt(fd, SOL_CAN_RAW, CAN_RAW_ERR_FILTER, &err_mask, sizeof(err_mask));
        }
#ifndef CAN_20_ONLY
        if((mode & CANMODE_FDOE))
        {
            ioctl(fd, SIOCGIFMTU, &ifr);
            if(ifr.ifr_mtu != CANFD_MTU)
                return CANERR_NOTINIT;
            int canfd_mode = 1;
            setsockopt(fd, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &canfd_mode, sizeof(canfd_mode));
        }
#endif
        if(bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        {
            return CANERR_SOCKET;       //   errno is set!
        }
        /* TODO: reset CAN controller */
        break;
    default:                            //   unknown CAN board
        return CANERR_ILLPARA;
    }
    can[i].fd = fd;                     // file descriptor of the CAN channel
    can[i].mode.byte = mode;            // store selected operation mode
    can[i].status.byte = CANSTAT_RESET; // CAN controller not started yet!

    return i;                           // return the handle
}

EXPORT
int can_exit(int handle)
{
    int i;

    if(!init)                           // must be initialized
        return CANERR_NOTINIT;
    if(handle != CANEXIT_ALL) {
        if(!IS_HANDLE_VALID(handle))    // must be a valid handle
            return CANERR_HANDLE;
        if(can[handle].fd < 0)          // must be an opened handle
            return CANERR_HANDLE;

        /*if(!can[handle].status.b.can_stopped) // release the CAN interface!*/
        {
            /* TODO: reset CAN controller */
        }
        close(can[handle].fd);          // resistance is futile!

        can[handle].status.byte |= CANSTAT_RESET;// CAN controller in INIT state
        can[handle].fd = -1;            // handle can be used again
    }
    else {
        for(i = 0; i < CAN_MAX_HANDLES; i++) {
            if(can[i].fd >= 0)          // must be an opened handle
            {
                /*if(!can[i].status.b.can_stopped) // release the CAN interface!*/
                {
                    /* TODO: reset CAN controller */
                }
                close(can[i].fd);       // resistance is futile!

                can[i].status.byte |= CANSTAT_RESET;// CAN controller in INIT state
                can[i].fd = -1;         // handle can be used again
            }
        }
    }
    return CANERR_NOERROR;
}

EXPORT
int can_start(int handle, const can_bitrate_t *bitrate)
{
    if(!init)                           // must be initialized
        return CANERR_NOTINIT;
    if(!IS_HANDLE_VALID(handle))        // must be a valid handle
        return CANERR_HANDLE;
    if(can[handle].fd < 0)              // must be an opened handle
        return CANERR_HANDLE;
    if(bitrate == NULL)                 // check for null-pointer
        return CANERR_NULLPTR;
    if(!can[handle].status.b.can_stopped)// must be stopped!
        return CANERR_ONLINE;

    // bit-rate is configured externally!
    if(bitrate->index != CANBDR_SOCKET)
        return CANERR_BAUDRATE;
    /* TODO: set baud rate */
    /* TODO: start CAN controller */
    can[handle].status.byte = 0x00;     // clear old status bits and counters
    can[handle].counters.tx = 0ull;
    can[handle].counters.rx = 0ull;
    can[handle].counters.err = 0ull;
    can[handle].status.b.can_stopped = 0;// CAN controller started!

    return CANERR_NOERROR;
}

EXPORT
int can_reset(int handle)
{
    if(!init)                           // must be initialized
        return CANERR_NOTINIT;
    if(!IS_HANDLE_VALID(handle))        // must be a valid handle
        return CANERR_HANDLE;
    if(can[handle].fd < 0)              // must be an opened handle
        return CANERR_HANDLE;

    if(can[handle].status.b.can_stopped) { // when running then go bus off
        /* TODO: reset CAN controller */
    }
    can[handle].status.b.can_stopped = 1; // CAN controller stopped!

    return CANERR_NOERROR;
}

EXPORT
int can_write(int handle, const can_msg_t *msg)
{
    if(!init)                           // must be initialized
        return CANERR_NOTINIT;
    if(!IS_HANDLE_VALID(handle))        // must be a valid handle
        return CANERR_HANDLE;
    if(can[handle].fd < 0)              // must be an opened handle
        return CANERR_HANDLE;
    if(msg == NULL)                     // check for null-pointer
        return CANERR_NULLPTR;
    if(can[handle].status.b.can_stopped)// must be running
        return CANERR_OFFLINE;

#ifndef CAN_20_ONLY
    if(!can[handle].mode.b.fdoe)
#endif
    {
        struct can_frame frame;
        memset(&frame, 0, sizeof(struct can_frame));

        if(msg->dlc > CAN_MAX_DLC)      //   dlc 0 .. 8!
            return CANERR_ILLPARA;
        frame.can_id  = msg->id;
        frame.can_id |= msg->ext? CAN_EFF_FLAG : 0;
        frame.can_id |= msg->rtr? CAN_RTR_FLAG : 0;
        frame.can_dlc = msg->dlc;
        memcpy(frame.data, msg->data, msg->dlc);

        if(write(can[handle].fd, &frame, sizeof(struct can_frame)) != CAN_MTU) {
            /*@ToDo: return codes are not quite clear!
             */
            can[handle].status.b.transmitter_busy = 1;
            return CANERR_TX_BUSY;      //     transmitter busy!
        }
    }
#ifndef CAN_20_ONLY
    else
    {
        struct canfd_frame fdframe;
        memset(&fdframe, 0, sizeof(struct canfd_frame));

        if(msg->dlc > CANFD_MAX_DLC)    //   dlc 0 .. 15!
            return CANERR_ILLPARA;
        fdframe.can_id  = msg->id;
        fdframe.can_id |= msg->ext? CAN_EFF_FLAG : 0;
        fdframe.can_id |= msg->rtr? CAN_RTR_FLAG : 0;
        fdframe.len = DLC2LEN(msg->dlc);
        fdframe.flags |= msg->brs? CANFD_BRS : 0;
        fdframe.flags |= msg->esi? CANFD_ESI : 0;
        memcpy(fdframe.data, msg->data, DLC2LEN(msg->dlc));

        if(write(can[handle].fd, &fdframe, sizeof(struct canfd_frame)) != CANFD_MTU) {
            /*@ToDo: return codes are not quite clear!
             */
            can[handle].status.b.transmitter_busy = 1;
            return CANERR_TX_BUSY;      //     transmitter busy!
        }
    }
#endif
    can[handle].status.b.transmitter_busy = 0;  // message transmitted!
    can[handle].counters.tx++;

    return CANERR_NOERROR;
}

EXPORT
int can_read(int handle, can_msg_t *msg, uint16_t timeout)
{
    fd_set rdfs;
    struct timeval timeo;

    FD_ZERO(&rdfs);
    FD_SET(can[handle].fd, &rdfs);

    timeo.tv_sec  = (long)(timeout / 1000u);
    timeo.tv_usec = (long)(timeout % 1000u) * 1000l;

    if(!init)                           // must be initialized
        return CANERR_NOTINIT;
    if(!IS_HANDLE_VALID(handle))        // must be a valid handle
        return CANERR_HANDLE;
    if(can[handle].fd < 0)              // must be an opened handle
        return CANERR_HANDLE;
    if(msg == NULL)                     // check for null-pointer
        return CANERR_NULLPTR;
    if(can[handle].status.b.can_stopped)// must be running
        return CANERR_OFFLINE;

    if(select(can[handle].fd+1, &rdfs, NULL, NULL, \
              (timeout < 0xFFFFu) ? &timeo : NULL) < 0) {
        can[handle].status.b.receiver_empty = 1;
        return CANERR_RX_EMPTY;         //   receiver empty!
    }
    if(!FD_ISSET(can[handle].fd, &rdfs)) {
        can[handle].status.b.receiver_empty = 1;
        return CANERR_RX_EMPTY;         //   receiver empty!
    }
#ifndef CAN_20_ONLY
    if(!can[handle].mode.b.fdoe)
#endif
    {
        struct can_frame frame;
        memset(&frame, 0, sizeof(struct can_frame));

        if(read(can[handle].fd, &frame, sizeof(struct can_frame)) != CAN_MTU) {
            /*@ToDo: return codes are not quite clear!
             */
            can[handle].status.b.message_lost = 1;
            return CANERR_MSG_LST;          //   message lost!
        }
        if((frame.can_id & CAN_ERR_FLAG) != CAN_ERR_FLAG) {
            msg->id  = (frame.can_id & CAN_ERR_MASK);
            msg->ext = (frame.can_id & CAN_EFF_FLAG)? 1 : 0;
            msg->rtr = (frame.can_id & CAN_RTR_FLAG)? 1 : 0;
#ifndef CAN_20_ONLY
            msg->fdf = 0;
            msg->brs = 0;
            msg->esi = 0;
#endif
            msg->dlc = frame.can_dlc;
            memcpy(msg->data, frame.data, frame.can_dlc);
            if(ioctl(can[handle].fd, SIOCGSTAMP, &msg->timestamp) > 0) {
                msg->timestamp.sec = 0;
                msg->timestamp.usec = 0;
            }
            can[handle].status.b.receiver_empty = 0;
            can[handle].status.b.message_lost = 0;
            can[handle].counters.rx++;
            return CANERR_NOERROR;        //   message read!
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
            can[handle].status.b.receiver_empty = 1;
            return CANERR_RX_EMPTY;       //   receiver empty!
        }
    }
#ifndef CAN_20_ONLY
    else
    {
        struct canfd_frame fdframe;
        memset(&fdframe, 0, sizeof(struct canfd_frame));

        if(read(can[handle].fd, &fdframe, sizeof(struct canfd_frame)) != CANFD_MTU) {
            /*@ToDo: return codes are not quite clear!
             */
            can[handle].status.b.message_lost = 1;
            return CANERR_MSG_LST;          //   message lost!
        }
        if((fdframe.can_id & CAN_ERR_FLAG) != CAN_ERR_FLAG) {
            msg->id  = (fdframe.can_id & CAN_ERR_MASK);
            msg->ext = (fdframe.can_id & CAN_EFF_FLAG) ? 1 : 0;
            msg->rtr = (fdframe.can_id & CAN_RTR_FLAG) ? 1 : 0;
            msg->fdf = 1;
            msg->brs = (fdframe.flags & CANFD_BRS) ? 1 : 0;
            msg->esi = (fdframe.flags & CANFD_ESI) ? 1 : 0;
            msg->dlc = LEN2DLC(fdframe.len);
            memcpy(msg->data, fdframe.data, fdframe.len);
            if(ioctl(can[handle].fd, SIOCGSTAMP, &msg->timestamp) > 0) {
                msg->timestamp.sec = 0;
                msg->timestamp.usec = 0;
            }
            can[handle].status.b.receiver_empty = 0;
            can[handle].status.b.message_lost = 0;
            can[handle].counters.rx++;
            return CANERR_NOERROR;        //   message read!
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
            can[handle].status.b.receiver_empty = 1;
            return CANERR_RX_EMPTY;      //   receiver empty!
        }
    }
#endif
}

EXPORT
int can_status(int handle, uint8_t *status)
{
    if(!init)                           // must be initialized
        return CANERR_NOTINIT;
    if(!IS_HANDLE_VALID(handle))        // must be a valid handle
        return CANERR_HANDLE;
    if(can[handle].fd < 0)              // must be an opened handle
        return CANERR_HANDLE;

    /* TODO: get CAN controller status */
    if(status)                          // status-register
      *status = can[handle].status.byte;

    return CANERR_NOERROR;
}

EXPORT
int can_busload(int handle, uint8_t *load, uint8_t *status)
{
    float busload = 0.0;                // bus-load (in [percent])

    if(!init)                           // must be initialized
        return CANERR_NOTINIT;
    if(!IS_HANDLE_VALID(handle))        // must be a valid handle
        return CANERR_HANDLE;
    if(can[handle].fd < 0)              // must be an opened handle
        return CANERR_HANDLE;

    if(!can[handle].status.b.can_stopped) { // when running get bus load
        (void)busload; //  TODO: measure bus load
    }
    if(load)                            // bus-load (in [percent])
        *load = (uint8_t)busload;
     return can_status(handle, status); // status-register
}

EXPORT
int can_bitrate(int handle, can_bitrate_t *bitrate, can_speed_t *speed)
{
    int rc;                             // return value

    if(!init)                           // must be initialized
        return CANERR_NOTINIT;
    if(!IS_HANDLE_VALID(handle))        // must be a valid handle
        return CANERR_HANDLE;
    if(can[handle].fd < 0)              // must be an opened handle
        return CANERR_HANDLE;

    /* TODO: get bit-rate settings */
    if(!can[handle].status.b.can_stopped)
        rc = CANERR_NOERROR;
    else
        rc = CANERR_OFFLINE;
    return rc;
}

EXPORT
int can_property(int handle, uint16_t param, void *value, uint32_t nbytes)
{
    if(!init || !IS_HANDLE_VALID(handle)) {
        return lib_parameter(param, value, (size_t)nbytes);
    }
    if(!init)                           // must be initialized
         return CANERR_NOTINIT;
     if(!IS_HANDLE_VALID(handle))       // must be a valid handle
         return CANERR_HANDLE;
     if(can[handle].fd < 0)             // must be an opened handle
         return CANERR_HANDLE;

     return drv_parameter(handle, param, value, (size_t)nbytes);
}

EXPORT
char *can_hardware(int handle)
{
    static char hardware[256] = "";     // hardware version

    if(!init)                           // must be initialized
        return NULL;
    if(!IS_HANDLE_VALID(handle))        // must be a valid handle
        return NULL;
    if(can[handle].fd < 0)              // must be an opened handle
        return NULL;

   sprintf(hardware, "interface=\"%s\", family=%d, type=%d, protocol=%d",
                      can[handle].ifname,
                      can[handle].family,
                      can[handle].type,
                      can[handle].protocol);

    return (char*)hardware;             // hardware version
}

EXPORT
char *can_software(int handle)
{
    static char software[256] = "";     // software version

    if(!init)                           // must be initialized
        return NULL;
    (void)handle;                       // handle not needed here

    sprintf(software, "linux-can (Linux Kernel CAN a.k.a. SocketCAN)");

    return (char*)software;             // software version
}
#endif

/*  -----------  local functions  ----------------------------------------
 */

#ifndef CAN_20_ONLY
 static uint8_t len2dlc(uint8_t len)
 {
     uint8_t dlc = len;

     if(len > 8) dlc = 0x9;
     if(len > 12) dlc = 0xA;
     if(len > 16) dlc = 0xB;
     if(len > 20) dlc = 0xC;
     if(len > 24) dlc = 0xD;
     if(len > 32) dlc = 0xE;
     if(len > 48) dlc = 0xF;

     return dlc;
 }
#endif

/*  - - - - - -  CAN API V3 properties  - - - - - - - - - - - - - - - - -
 */
static int lib_parameter(uint16_t param, void *value, size_t nbytes)
{
    int rc = CANERR_ILLPARA;            // suppose an invalid parameter

    if(value == NULL)                   // check for null-pointer
        return CANERR_NULLPTR;

    /* CAN library properties */
    switch(param) {
    case CANPROP_GET_SPEC:              // version of the wrapper specification (uint16_t)
        if(nbytes == sizeof(uint16_t)) {
            *(uint16_t*)value = (uint16_t)CAN_API_SPEC;
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_VERSION:           // version number of the library (uint16_t)
        if(nbytes == sizeof(uint16_t)) {
            *(uint16_t*)value = ((uint16_t)VERSION_MAJOR << 8)
                                    | ((uint16_t)VERSION_MINOR & 0xFu);
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_PATCH_NO:          // patch number of the library (uint8_t)
        if(nbytes == sizeof(uint8_t)) {
            *(uint8_t*)value = (uint8_t)VERSION_PATCH;
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_BUILD_NO:          // build number of the library (uint32_t)
        if(nbytes == sizeof(uint32_t)) {
            *(uint32_t*)value = (uint32_t)VERSION_BUILD;
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_LIBRARY_ID:        // library id of the library (int32_t)
        if(nbytes == sizeof(int32_t)) {
            *(int32_t*)value = (int32_t)SOCKETCAN_LIB_ID;
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_LIBRARY_VENDOR:    // vendor name of the library (char[256])
        if((nbytes > strlen(CAN_API_VENDOR)) && (nbytes <= CANPROP_BUFFER_SIZE)) {
            strcpy((char*)value, CAN_API_VENDOR);
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_LIBRARY_DLLNAME:   // file name of the library (char[256])
        if ((nbytes > strlen(SOCKETCAN_LIB_NAME)) && (nbytes <= CANPROP_BUFFER_SIZE)) {
            strcpy((char*)value, SOCKETCAN_LIB_NAME);
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_BOARD_VENDOR:      // vendor name of the CAN interface (char[256])
        if((nbytes > strlen("n/a")) && (nbytes <= CANPROP_BUFFER_SIZE)) {
            strcpy((char*)value, "n/a");
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_BOARD_DLLNAME:     // file name of the CAN interface (char[256])
        if((nbytes > strlen("n/a")) && (nbytes <= CANPROP_BUFFER_SIZE)) {
            strcpy((char*)value, "n/a");
            rc = CANERR_NOERROR;
        }
        break;
    default:
        rc = CANERR_NOTSUPP;
        break;
    }
    return rc;
}

static int drv_parameter(int handle, uint16_t param, void *value, size_t nbytes)
{
    int rc = CANERR_ILLPARA;            // suppose an invalid parameter
    can_bitrate_t bitrate = {};
    can_speed_t speed = {};
    //can_mode_t mode;
    uint8_t status;
    uint8_t load;

    assert(IS_HANDLE_VALID(handle));    // just to make sure

    if(value == NULL)                   // check for null-pointer
        return CANERR_NULLPTR;

    /* CAN interface properties */
    switch(param) {
    case CANPROP_GET_BOARD_TYPE:        // board type of the CAN interface (int32_t)
        if(nbytes == sizeof(int32_t)) {
            *(int32_t*)value = (int32_t)CAN_NETDEV;
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_BOARD_NAME:        // board name of the CAN interface (char[256])
        if((nbytes > strlen(can[handle].ifname)) && (nbytes <= CANPROP_BUFFER_SIZE)) {
            strcpy((char*)value, can[handle].ifname);
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_BOARD_PARAM:       // board parameter of the CAN interface (char[256])
        if(nbytes == sizeof(struct _can_netdev_param)) {
            strcpy((char*)((struct _can_netdev_param*)value)->ifname, can[handle].ifname);
            ((struct _can_netdev_param*)value)->family = (int)can[handle].family;
            ((struct _can_netdev_param*)value)->type = (int)can[handle].type;
            ((struct _can_netdev_param*)value)->protocol = (int)can[handle].protocol;
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_OP_CAPABILITY:     // supported operation modes of the CAN controller (uint8_t)
        /* TODO: capability */
        break;
    case CANPROP_GET_OP_MODE:           // active operation mode of the CAN controller (uint8_t)
        if (nbytes == sizeof(uint8_t)) {
            *(uint8_t*)value = (uint8_t)can[handle].mode.byte;
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_BITRATE:           // active bit-rate of the CAN controller (can_bitrate_t)
        if(((rc = can_bitrate(handle, &bitrate, NULL)) == CANERR_NOERROR) || (rc == CANERR_OFFLINE)) {
            if(nbytes == sizeof(can_bitrate_t)) {
                memcpy(value, &bitrate, sizeof(can_bitrate_t));
                rc = CANERR_NOERROR;
            }
        }
        break;
    case CANPROP_GET_SPEED:             // active bus speed of the CAN controller (can_speed_t)
        if(((rc = can_bitrate(handle, NULL, &speed)) == CANERR_NOERROR) || (rc == CANERR_OFFLINE)) {
            if(nbytes == sizeof(can_speed_t)) {
                memcpy(value, &speed, sizeof(can_speed_t));
                rc = CANERR_NOERROR;
            }
        }
        break;
    case CANPROP_GET_STATUS:            // current status register of the CAN controller (uint8_t)
        if((rc = can_status(handle, &status)) == CANERR_NOERROR) {
            if(nbytes == sizeof(uint8_t)) {
                *(uint8_t*)value = (uint8_t)status;
                rc = CANERR_NOERROR;
            }
        }
        break;
    case CANPROP_GET_BUSLOAD:           // current bus load of the CAN controller (uint8_t)
        if((rc = can_busload(handle, &load, NULL)) == CANERR_NOERROR) {
            if(nbytes == sizeof(uint8_t)) {
                *(uint8_t*)value = (uint8_t)load;
                rc = CANERR_NOERROR;
            }
        }
        break;
    case CANPROP_GET_TX_COUNTER:        // total number of sent messages (uint64_t)
        if(nbytes == sizeof(uint64_t)) {
            *(uint64_t*)value = (uint64_t)can[handle].counters.tx;
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_RX_COUNTER:        // total number of reveiced messages (uint64_t)
        if(nbytes == sizeof(uint64_t)) {
            *(uint64_t*)value = (uint64_t)can[handle].counters.rx;
            rc = CANERR_NOERROR;
        }
        break;
    case CANPROP_GET_ERR_COUNTER:       // total number of reveiced error frames (uint64_t)
        if(nbytes == sizeof(uint64_t)) {
            *(uint64_t*)value = (uint64_t)can[handle].counters.err;
            rc = CANERR_NOERROR;
        }
        break;
    default:
        rc = lib_parameter(param, value, nbytes);
        break;
    }
    return rc;
}

/*  -----------  revision control  ---------------------------------------
 */
EXPORT
char *can_version(void)
{
    return (char*)_id;
}
/** @}
 */
/*  ----------------------------------------------------------------------
 *  Uwe Vogt,  UV Software,  Chausseestrasse 33 A,  10115 Berlin,  Germany
 *  Tel.: +49-30-46799872,  Fax: +49-30-46799873,  Mobile: +49-170-3801903
 *  E-Mail: uwe.vogt@uv-software.de,  Homepage: http://www.uv-software.de/
 */
