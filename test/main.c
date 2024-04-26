/* ***	$Header$  ***
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
 * CAN Tester for berliOS socketCAN.
 *
 * Usage: can_test <interface> [<options>]
 * Options for receiver test (default):
 *	-r, --receive              Count received messages until ^C is pressed
 *	-b, --baudrate=<baudrate>  Bit timing in kbps (default=125)
 * Options for transmitter test:
 *	-t, --transmit=<time>      Send messages for the given time in seconds
 *	-c, --cycle=<cycle>        Cycle time in milliseconds (default=0)
 *	-d, --data=<length>        Send data of given length (default=0)
 *	-i, --id=<cob-id>          Use given identifier (default=100h)
 *	-b, --baudrate=<baudrate>  Bit timing in kbps (default=125)
 * Options:
 *  -h, --help                 display this help and exit
 *      --version              output version information and exit
 * 
 * Written by Uwe Vogt, UV Software <http://www.uv-software.de/>
 */

static const char* __version__ = "0.1.17";


/* ***	includes  ***
 */

#include "can_io.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
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


/* ***	defines  ***
 */

#define __no_can_ioctl
#ifndef BYTE
#define BYTE	unsigned char
#endif
#ifndef WORD
#define WORD	unsigned short
#endif
#define RxMODE	0
#define TxMODE	1


/* ***	types  ***
 */


/* ***	prototypes  ***
 */

void sigterm(int signo);
void usage(FILE *stream, char *program);
void version(FILE *stream, char *program);

int tx_test(long cob_id, short length, WORD delay, time_t duration);
int rx_test(void);

short can_start_timer(WORD timeout);
short can_is_timeout(void);


/* ***	variables  ***
 */

static int running = 1;

static char *prompt[4] = {"-\b", "/\b", "|\b", "\\\b"};


/* ***	main  ***
 */

int main(int argc, char *argv[])
/*
 * function : main function of the application.
 *
 * parameter: argc - number of command line arguments.
 *	          argv - command line arguments as string vector.
 *
 * result   : 0    - no error occured.
 */
{
	int opt, rc;
	int mode = RxMODE, m = 0;
	time_t txtime = 0;
	long  baudrate = 4; int bd = 0;
	long  delay = 0; int t = 0;
	long  data = 0; int d = 0;
	long  cob_id = 0x100; int c = 0;
	char *device, *firmware, *software;
	
	struct  option long_options[] = {
		{"receive", no_argument, 0, 'r'},
		{"transmit", required_argument, 0, 't'},
		{"baudrate", required_argument, 0, 'b'},
		{"cycle", required_argument, 0, 'c'},
		{"data", required_argument, 0, 'd'},
		{"id", required_argument, 0, 'i'},
		{"help", no_argument, 0, 'h'},
		{"version", no_argument, 0, 'v'},
		{0, 0, 0, 0}
	};
	struct _can_param can_param = {"can0", PF_CAN, SOCK_RAW, CAN_RAW};

	signal(SIGINT, sigterm);	
	signal(SIGHUP, sigterm);	
	signal(SIGTERM, sigterm);	

	while((opt = getopt_long(argc, argv, "rt:b:c:d:i:hv", long_options, NULL)) != -1) {
		switch(opt) {
			case 'r':
				if(m++) {
					fprintf(stderr, "+++ error: conflict in option -- r\n");
					usage(stderr, basename(argv[0]));
					return 1;
				}
				mode = RxMODE;
				break;
			case 't':
				if(m++) {
					fprintf(stderr, "+++ error: conflict in option -- t\n");
					usage(stderr, basename(argv[0]));
					return 1;
				}
				if(sscanf(optarg, "%li", &txtime) != 1) {
					fprintf(stderr, "+++ error: illegal argument in option -- t\n");
					usage(stderr, basename(argv[0]));
					return 1;
				}
				mode = TxMODE;
				break;
			case 'b':
				if(bd++) {
					fprintf(stderr, "+++ error: conflict in option -- b\n");
					usage(stderr, basename(argv[0]));
					return 1;
				}
				if(sscanf(optarg, "%li", &baudrate) != 1) {
					fprintf(stderr, "+++ error: illegal argument in option -- b\n");
					usage(stderr, basename(argv[0]));
					return 1;
				}
				switch(baudrate) {
					case 1000: baudrate = 0; break;
					case 800:  baudrate = 1; break;
					case 500:  baudrate = 2; break;
					case 250:  baudrate = 3; break;
					case 125:  baudrate = 4; break;
					case 100:  baudrate = 5; break;
					case 50:   baudrate = 6; break;
					case 20:   baudrate = 7; break;
					case 10:   baudrate = 8; break;
				}
				if((baudrate < 0) || (8 < baudrate)) {
					fprintf(stderr, "+++ error: illegal value in option -- b\n");
					usage(stderr, basename(argv[0]));
					return 1;
				}
#ifdef __no_can_ioctl
 fprintf(stderr, "+++ sorry: unsupported option -- b\n");
 return 1;
#endif
				break;
			case 'c':
				if(t++) {
					fprintf(stderr, "+++ error: conflict in option -- c\n");
					usage(stderr, basename(argv[0]));
					return 1;
				}
				if(sscanf(optarg, "%li", &delay) != 1) {
					fprintf(stderr, "+++ error: illegal argument in option -- c\n");
					usage(stderr, basename(argv[0]));
					return 1;
				}
				break;
			case 'd':
				if(d++) {
					fprintf(stderr, "+++ error: conflict in option -- d\n");
					usage(stderr, basename(argv[0]));
					return 1;
				}
				if(sscanf(optarg, "%li", &data) != 1) {
					fprintf(stderr, "+++ error: illegal argument in option -- d\n");
					usage(stderr, basename(argv[0]));
					return 1;
				}
				if((data < 0) || (8 < data)) {
					fprintf(stderr, "+++ error: illegal value in option -- d\n");
					usage(stderr, basename(argv[0]));
					return 1;
				}
				break;
			case 'i':
				if(c++) {
					fprintf(stderr, "+++ error: conflict in option -- i\n");
					usage(stderr, basename(argv[0]));
					return 1;
				}
				if(sscanf(optarg, "%li", &cob_id) != 1) {
					fprintf(stderr, "+++ error: illegal argument in option -- i\n");
					usage(stderr, basename(argv[0]));
					return 1;
				}
				if((cob_id < 0x000) || (0x7FF < cob_id)) {
					fprintf(stderr, "+++ error: illegal value in option -- i\n");
					usage(stderr, basename(argv[0]));
					return 1;
				}
				break;
			case 'v':
				version(stdout, basename(argv[0]));
				return 0;
			case 'h':
			default:
				usage(stderr, basename(argv[0]));
				return 1;
		}
	}
	if(argc == optind) {
		fprintf(stderr, "+++ error: no interface given\n");
		usage(stderr, basename(argv[0]));
		return 1;
	}
	else
		can_param.ifname = argv[optind];
	
	fprintf(stdout, "%s (Tester for berliOS socketCAN), version %s of %s\n",
					 basename(argv[0]),__version__,__DATE__);
	fprintf(stdout, "Copyright (C) 2007 UV Software, Friedrichshafen\n\n");

    fprintf(stdout, "Hardware=%s...", can_param.ifname);
	fflush (stdout);
    if((rc = can_init(CAN_NETDEV, &can_param)) != CANERR_NOERROR) {
        fprintf(stdout, "FAILED!\n");
        if(rc != CANERR_SOCKET)
			fprintf(stderr, "+++ error: CAN Controller not initialized (can_init=%i)\n", rc);
		else
			perror("+++ error");
        return rc;
    }
    fprintf(stdout, "OK!\n");

#ifndef __no_can_ioctl
	fprintf(stdout, "Baudrate=%skBit/s...", baudrate == 0? "1000" :
											baudrate == 1? "800" :
											baudrate == 2? "500" :
											baudrate == 3? "250" :
											baudrate == 4? "125" :
											baudrate == 5? "100" :
											baudrate == 6? "50" :
											baudrate == 7? "20" :
											baudrate == 8? "10" : "???");
#else
 fprintf(stdout, "Baudrate=(taken from socket)...");
 baudrate = CANBDR_SOCKET;
#endif
	fflush (stdout);
	if((rc = can_start(baudrate)) != CANERR_NOERROR) {
		fprintf(stdout, "FAILED!\n");
        if(rc != CANERR_SOCKET)
			fprintf(stderr, "+++ error: CAN Controller not started (can_start=%i)\n", rc);
		else
			perror("+++ error");
		can_exit();
		return rc;
	}
	fprintf(stdout, "OK!\n");

	if(mode == TxMODE)					/* Transmitter-Test					*/
		tx_test(cob_id, (short)data, (WORD)delay, (time_t)txtime);
	else								/* Receiver-Test					*/
		rx_test();

	if(((device = can_hardware()) != NULL) &&
	   ((firmware = can_software()) != NULL) &&
	   ((software = can_version()) != NULL))
		fprintf(stdout, "Hardware: %s\n" \
						"Firmware: %s\n" \
						"Software: %s\n" \
						"Copyright (c) 2007 UV Software, Friedrichshafen\n",
				device, firmware, software);
	can_exit();

	return 0;
}

/* ***	functions  ***
 */

int tx_test(long cob_id, short length, WORD delay, time_t duration)
{
	time_t start = time(NULL);
	can_msg_t message;
	long frames = 0;
	long errors = 0;
	long calls = 0;
	int  rc;

	fprintf(stderr, "\nPress ^C to abort.\n");
	message.id  = cob_id;
	message.ext = 0;
	message.rtr = 0;
	message.dlc = length;
	fprintf(stdout, "\nTransmit message(s)...");
	fflush (stdout);
	while(time(NULL) < (start + duration)) {
		message.data[0] = (BYTE)(frames >> 0);
		message.data[1] = (BYTE)(frames >> 8);
		message.data[2] = (BYTE)(frames >> 16);
		message.data[3] = (BYTE)(frames >> 24);
		message.data[4] = message.data[5] = message.data[6] = message.data[7] = 0;
		can_start_timer(delay);
		if((rc = can_transmit(&message)) == CANERR_NOERROR)
			fprintf(stderr, "%s", prompt[(frames++ % 4)]);
		else
			errors++;
		calls++;
		while(!can_is_timeout()) {
			if(!running) {
				fprintf(stderr, "\b");
				fprintf(stdout, "STOP!\n\n");
				fprintf(stdout, "Message(s)=%li\n", frames);
				fprintf(stdout, "Error(s)=%li\n", errors);
				fprintf(stdout, "Call(s)=%li\n", calls);
				fprintf(stdout, "Time=%lisec\n\n", time(NULL) - start);
				return frames;
			}
		}
		if(!running) {
			fprintf(stderr, "\b");
			fprintf(stdout, "STOP!\n\n");
			fprintf(stdout, "Message(s)=%li\n", frames);
			fprintf(stdout, "Error(s)=%li\n", errors);
			fprintf(stdout, "Call(s)=%li\n", calls);
			fprintf(stdout, "Time=%lisec\n\n", time(NULL) - start);
			return frames;
		}
	}
	fprintf(stderr, "\b");
	fprintf(stdout, "OK!\n\n");
	fprintf(stdout, "Message(s)=%li\n", frames);
	fprintf(stdout, "Error(s)=%li\n", errors);
	fprintf(stdout, "Call(s)=%li\n", calls);
	fprintf(stdout, "Time=%lisec\n\n", time(NULL) - start);
	return frames;
}

int rx_test(void)
{
	time_t start = time(NULL);
	can_msg_t message;
	long frames = 0;
	long errors = 0;
	long calls = 0;
	int  rc;

	fprintf(stderr, "\nPress ^C to abort.\n");
	fprintf(stdout, "\nReceive message(s)...");
	fflush (stdout);
	for(;;) {
		if((rc = can_receive(&message)) == CANERR_NOERROR)
			fprintf(stderr, "%s", prompt[(frames++ % 4)]);
		else if(rc != CANERR_RX_EMPTY)
			errors++;
		else
			usleep(1);
		calls++;
		if(!running) {
			fprintf(stderr, "\b");
			fprintf(stdout, "OK!\n\n");
			fprintf(stdout, "Message(s)=%li\n", frames);
			fprintf(stdout, "Error(s)=%li\n", errors);
			fprintf(stdout, "Call(s)=%li\n", calls);
			fprintf(stdout, "Time=%lisec\n\n", time(NULL) - start);
			return frames;
		}
	}
}

static  __u64 llUntilStop = 0;

short can_start_timer(WORD timeout)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	
	llUntilStop = ((__u64)tv.tv_sec * (__u64)1000000) + (__u64)tv.tv_usec \
	            + ((__u64)timeout   * (__u64)1000);
	
	return 0;
}

short can_is_timeout(void)
{
	__u64 llNow;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	
	llNow = ((__u64)tv.tv_sec * (__u64)1000000) + (__u64)tv.tv_usec;

	if(llNow < llUntilStop)
		return 0;
	else
		return 1;
}

void sigterm(int signo)
{
	/*printf("got signal %d\n", signo);*/
	running = 0;
}

void version(FILE *stream, char *program)
{
	fprintf(stream, "%s (Tester for berliOS socketCAN), version %s of %s\n",
							   program,__version__,__DATE__);
	fprintf(stream, "Copyright (C) 2007 UV Software, Friedrichshafen\n");
	fprintf(stream, "This is free software. You may redistribute copies of it under the terms of\n");
	fprintf(stream, "the GNU General Public License <http://www.gnu.org/licenses/gpl.html>.\n");
	fprintf(stream, "There is NO WARRANTY, to the extent permitted by law.\n\n");

	fprintf(stream, "Written by Uwe Vogt, UV Software <http://www.uv-software.de/>\n");
}

void usage(FILE *stream, char *program)
{
	fprintf(stream, "Usage: %s <interface> [<options>]\n", program);
	fprintf(stream, "Options for receiver test (default):\n");
	fprintf(stream, " -r, --receive              Count received messages until ^C is pressed\n");
#ifndef __no_can_ioctl
	fprintf(stream, " -b, --baudrate=<baudrate>  Bit timing in kbps (default=125)\n");
#endif
	fprintf(stream, "Options for transmitter test:\n");
	fprintf(stream, " -t, --transmit=<time>      Send messages for the given time in seconds\n");
	fprintf(stream, " -c, --cycle=<cycle>        Cycle time in milliseconds (default=0)\n");
	fprintf(stream, " -d, --data=<length>        Send data of given length (default=0)\n");
	fprintf(stream, " -i, --id=<cob-id>          Use given identifier (default=100h)\n");
#ifndef __no_can_ioctl
	fprintf(stream, " -b, --baudrate=<baudrate>  Bit timing in kbps (default=125)\n");
#endif
	fprintf(stream, "Options:\n");
	fprintf(stream, " -h, --help                 display this help and exit\n");
	fprintf(stream, "     --version              show version information and exit\n");
}

/* ***	end of file  ***
 */
 
