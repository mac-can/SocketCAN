//
//  main.cpp
//  PCBUSB-Wrapper
//  Bart Simpson didn´t do it
//
#include "can_io.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <libgen.h>
#include <getopt.h>
#include <signal.h>
#include <errno.h>
#include <time.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include <linux/can.h>

#define __no_can_ioctl

#define PROMPT_NORMAL       0
#define PROMPT_OVERRUN      1
#define PROMPT_ISSUE198     2

#define OPTION_IO_POLLING   0
#define OPTION_IO_BLOCKING  1
#define OPTION_IO_CALLBACK  2

#define OPTION_NO           0
#define OPTION_YES          1

#ifndef BYTE
#define BYTE unsigned char
#endif
#ifndef WORD
#define WORD unsigned short
#endif
#ifndef DWORD
#define DWORD unsigned long
#endif
#ifndef QWORD
#define QWORD unsigned long long
#endif

static void sigterm(int signo);

static int running = 1;

int main(int argc, char *argv[])
{
    int rc = -1;
    int opt, i;

    int option_io = OPTION_IO_BLOCKING;
    int option_test = OPTION_NO;
    int option_info = OPTION_NO;
    //int option_stat = OPTION_NO;
    int option_stop = OPTION_NO;
    int option_echo = OPTION_YES;
    int option_check = OPTION_YES;
    //int option_trace = OPTION_NO;
    //int option_log = OPTION_NO;
    int option_transmit = 0;

    QWORD received = 0;
    QWORD expected = 0;
    QWORD frames = 0;
    QWORD errors = 0;

    char symbol[] = ">!?";
    int prompt = 0;

    can_msg_t message;
    char *device, *firmware, *software;

    struct _can_param can_param = {"can0", PF_CAN, SOCK_RAW, CAN_RAW};

    if((signal(SIGINT, sigterm) == SIG_ERR) ||
       (signal(SIGHUP, sigterm) == SIG_ERR) ||
       (signal(SIGTERM, sigterm) == SIG_ERR)) {
        perror("+++ error");
        return errno;
    }
    for(i = 1; i < argc; i++) {
        /* interface name */
        if(!strcmp(argv[i], "can0")) can_param.ifname = argv[i];
        if(!strcmp(argv[i], "can1")) can_param.ifname = argv[i];
        if(!strcmp(argv[i], "can2")) can_param.ifname = argv[i];
        if(!strcmp(argv[i], "can3")) can_param.ifname = argv[i];
        if(!strcmp(argv[i], "can4")) can_param.ifname = argv[i];
        if(!strcmp(argv[i], "can5")) can_param.ifname = argv[i];
        if(!strcmp(argv[i], "can6")) can_param.ifname = argv[i];
        if(!strcmp(argv[i], "can7")) can_param.ifname = argv[i];
        /* asynchronous IO */
        if(!strcmp(argv[i], "POLLING")) option_io = OPTION_IO_POLLING;
        if(!strcmp(argv[i], "BLOCKING")) option_io = OPTION_IO_BLOCKING;
        /* test all channels: not present / available / occupied */
        if(!strcmp(argv[i], "TEST")) option_test = OPTION_YES;
        /* query some informations: hw, sw, etc. */
        if(!strcmp(argv[i], "INFO")) option_info = OPTION_YES;
        /* query some statistical data */
        //if(!strcmp(argv[i], "STAT")) option_stat = OPTION_YES;
        /* stop on error */
        if(!strcmp(argv[i], "STOP")) option_stop = OPTION_YES;
        if(!strcmp(argv[i], "IGNORE")) option_check = OPTION_NO;
        if(!strcmp(argv[i], "CHECK")) option_check = OPTION_YES;
        /* echo ON/OFF */
        if(!strcmp(argv[i], "SILENT")) option_echo = OPTION_NO;
        /* logging and debugging */
        //if(!strcmp(argv[i], "TRACE")) option_trace = OPTION_YES;
        //if(!strcmp(argv[i], "LOG")) option_log = OPTION_YES;
        /* transmit messages */
        if(sscanf(argv[i], "%i", &opt) == 1 && opt > 0) option_transmit = opt;
    }
option_io = OPTION_IO_POLLING;  // FIXME: remove this once blocking read is realized
    /* offline informations */
    if(option_info) {
        fprintf(stdout, "can_test: "__DATE__" "__TIME__"\n");
        if((software = can_version()) != NULL)
            fprintf(stdout, "Software: %s\n", software);
    }
    /* channel tester */
    if(option_test) {
    }
    /* initialization */
    if((rc = can_init(CAN_NETDEV, &can_param)) < 0) {
        if(rc != CANERR_SOCKET)
            printf("+++ error(%i): can_init failed\n", rc);
            else
                perror("+++ error");
        goto end;
    }
    /* start communication */
    if((rc = can_start(CANBDR_SOCKET)) != CANERR_NOERROR) {
        printf("+++ error(%i): can_start failed\n", rc);
        goto end;
    }
    /* transmit messages */
    if(option_transmit > 0)
        printf("Transmit messages");
    for(i = 0; i < option_transmit; i++) {
        message.id = (DWORD)i % 0x800UL;
	    message.ext = 0;
	    message.rtr = 0;
        message.dlc = 8;
        message.data[0] = (BYTE)(((QWORD)i & 0x00000000000000FF) >> 0);
        message.data[1] = (BYTE)(((QWORD)i & 0x000000000000FF00) >> 8);
        message.data[2] = (BYTE)(((QWORD)i & 0x0000000000FF0000) >> 16);
        message.data[3] = (BYTE)(((QWORD)i & 0x00000000FF000000) >> 24);
        message.data[4] = (BYTE)(((QWORD)i & 0x000000FF00000000) >> 32);
        message.data[5] = (BYTE)(((QWORD)i & 0x0000FF0000000000) >> 40);
        message.data[6] = (BYTE)(((QWORD)i & 0x00FF000000000000) >> 48);
        message.data[7] = (BYTE)(((QWORD)i & 0xFF00000000000000) >> 56);

repeat:
        if((rc = can_transmit(&message)) != CANERR_NOERROR) {
            if((rc == CANERR_TX_BUSY) && running)
                 goto repeat;
            printf("+++ error(%i): can_transmit failed\n", rc);
            if(option_stop)
                goto end;
        }
        if(!(i % 2048)) {
            fprintf(stdout, ".");
            fflush(stdout);
        }
    }
    if(option_transmit > 0)
        printf("\n");
    /* reception loop */
    printf("Reception (%s)", (option_io == OPTION_IO_BLOCKING)? "blocking" : "polling");
    if(option_echo)
        printf(":\n");
    else
        fflush(stdout);
    while(running) {
        if((rc = can_receive(&message)) == CANERR_NOERROR) {
            if(option_echo) {
                printf("%c %llu\t", symbol[prompt], frames++);
                printf("%li.%04li\t", message.timestamp.sec, message.timestamp.usec / 100l);
                printf("%03lX [%u]\t", message.id, message.dlc);
                for(i = 0;i < message.dlc; i++)
                    printf("%02X ", message.data[i]);
                printf("\n");
            }
            else {
                if(!(frames++ % 2048)) {
                    fprintf(stdout, ".");
                    fflush(stdout);
                }
            }
            received = 0;
            if (message.dlc > 0) received |= (QWORD)message.data[0] << 0;
            if (message.dlc > 1) received |= (QWORD)message.data[1] << 8;
            if (message.dlc > 2) received |= (QWORD)message.data[2] << 16;
            if (message.dlc > 3) received |= (QWORD)message.data[3] << 24;
            if (message.dlc > 4) received |= (QWORD)message.data[4] << 32;
            if (message.dlc > 5) received |= (QWORD)message.data[5] << 40;
            if (message.dlc > 6) received |= (QWORD)message.data[6] << 48;
            if (message.dlc > 7) received |= (QWORD)message.data[7] << 56;
            if (received != expected) {
                if (option_check != OPTION_NO) {
                    printf("+++ error(X): received data is not equal to expected data (%llu : %llu)\n", received, expected);
                    if (expected > received) {
                        printf("              issue #198: old messages on pipe #3 (offset -%llu)\a\n", expected - received);
#if (STOP_FRAMES == 0)
                        if (option_stop)
                            goto end;
#endif
                        prompt = PROMPT_ISSUE198;
                    }
                    else {
                        prompt = prompt != PROMPT_ISSUE198 ? PROMPT_OVERRUN : PROMPT_ISSUE198;
                    }
                }
            }
#if (STOP_FRAMES != 0)
            if ((prompt == PROMPT_ISSUE198) && (option_stop) && (++stop_frames >= STOP_FRAMES))
                goto end;
#endif
            expected = received + 1;
        }
        else if (rc != CANERR_RX_EMPTY) {
            printf("+++ error(%i): can_read failed\n", rc);
            errors++;
        }
    }
    printf(":\n");
    /* online information */
    if(option_info) {
        if((device = can_hardware()) != NULL)
            fprintf(stdout, "Hardware: %s\n", device);
        if((firmware = can_software()) != NULL)
            fprintf(stdout, "Firmware: %s\n", firmware);
    }
end:
    printf("! BYE\n");
    if((rc = can_exit()) < 0) {
        printf("+++ error(%i): can_exit failed\n", rc);
    }
    return 0;
}

static void sigterm(int signo)
{
    /*printf("got signal %d\n", signo);*/
    running = 0;
}
