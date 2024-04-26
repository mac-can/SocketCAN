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
 * CAN Monitor (using berliOS socketCAN).
 *
 * Usage: can_moni <interface> [<options>]
 * Options:
 *  -b, --baudrate=<baudrate>    bit timing in kbps (default=125)
 *  -t, --time=(abs|rel|zero)    absolute or relative time
 *  -i  --id=(hex|dec|oct)       display mode of COB-IDs
 *  -d, --data=(hex|dec|oct)     display mode of data bytes
 *  -a, --ascii=(on|off)         display data bytes as ascii
 *  -x, --exclude=[~]<id-list>   exclude COB-IDs: <id>[-<id>]{,<id>[-<id>]}
 *  -s, --script=<filename>      execute a script file
 *  -h, --help                   display this help and exit
 *      --version                output version information and exit
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


/* ***	options  ***
 */

#define __no_can_ioctl


/* ***	defines  ***
 */

#define MAX_ID		2048

#define TIME_ZERO	0
#define TIME_ABS	1
#define TIME_REL	2

#define MODE_HEX	0
#define MODE_DEC	1
#define MODE_OCT	2

#define ASCII_OFF	0
#define ASCII_ON	1


/* ***	types  ***
 */


/* ***	prototypes  ***
 */

void sigterm(int signo);
void usage(FILE *stream, char *program);
void version(FILE *stream, char *program);

static void print_id(FILE *stream, unsigned long id, int mode);
static void print_data(FILE *stream, unsigned char data, int last, int mode);
static void print_space(FILE *stream, int last, int mode);
static void print_time(FILE *stream, struct timeval *timestamp, int mode);
static int get_exclusion(char *arg);


/* ***	variables  ***
 */

static int running = 1;
static int can_id[MAX_ID];
static struct timeval laststamp = {0, 0};


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
	int   opt, rc, i; 
	unsigned char ch;
	long  baudrate = 4; int bd = 0;
	int mode_time = TIME_ZERO; int mt = 0;
	int mode_id = MODE_HEX; int mi = 0;
	int mode_data = MODE_HEX; int md = 0;
	int mode_ascii = ASCII_ON; int ma = 0;
	int ex = 0;
//	char *script_file = NULL;
	can_msg_t can_msg; long frames = 0;
	char *device, *firmware, *software;
	
	struct option long_options[] = {
		{"baudrate", required_argument, 0, 'b'},
		{"time", required_argument, 0, 't'},
		{"id", required_argument, 0, 'i'},
		{"data", required_argument, 0, 'd'},
		{"ascii", required_argument, 0, 'a'},
		{"exclude", required_argument, 0, 'x'},
		{"script", required_argument, 0, 's'},
		{"help", no_argument, 0, 'h'},
		{"version", no_argument, 0, 'v'},
		{0, 0, 0, 0}
	};
	struct _can_param can_param = {"can0", PF_CAN, SOCK_RAW, CAN_RAW};
	
	signal(SIGINT, sigterm);	
	signal(SIGHUP, sigterm);	
	signal(SIGTERM, sigterm);	

	for(i = 0; i < MAX_ID; i++)
		can_id[i] = 1;
	
	while((opt = getopt_long(argc, argv, "b:t:i:d:a:x:s:h", long_options, NULL)) != -1) {
		switch(opt) {
			case 'b':
				if(bd++) {
					fprintf(stderr, "+++ error: conflict in option -- b\n");
					usage(stderr, basename(argv[0]));
					return 1;
				}
				if(optarg && optarg[0] == '-') {
					fprintf(stderr, "+++ error: missing argument in option -- b\n");
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
			case 't':
				if(mt++) {
					fprintf(stderr, "+++ error: conflict in option -- t\n");
					usage(stderr, basename(argv[0]));
					return 1;
				}
				if(optarg && optarg[0] == '-') {
					fprintf(stderr, "+++ error: missing argument in option -- t\n");
					usage(stderr, basename(argv[0]));
					return 1;
				}
				if(!strcmp(optarg, "absolute") || !strcmp(optarg, "abs") || !strcmp(optarg, "a"))
					mode_time = TIME_ABS;
				else if(!strcmp(optarg, "relative") || !strcmp(optarg, "rel") || !strcmp(optarg, "r"))
					mode_time = TIME_REL;
				else if(!strcmp(optarg, "zero") || !strcmp(optarg, "z") || !strcmp(optarg, "0"))
					mode_time = TIME_ZERO;
				else {
					fprintf(stderr, "+++ error: illegal argument in option -- t\n");
					usage(stderr, basename(argv[0]));
					return 1;
				}
				break;
			case 'i':
				if(mi++) {
					fprintf(stderr, "+++ error: conflict in option -- i\n");
					usage(stderr, basename(argv[0]));
					return 1;
				}
				if(optarg && optarg[0] == '-') {
					fprintf(stderr, "+++ error: missing argument in option -- i\n");
					usage(stderr, basename(argv[0]));
					return 1;
				}
				if(!strcmp(optarg, "hexadecimal") || !strcmp(optarg, "hex") || !strcmp(optarg, "h") || !strcmp(optarg, "16"))
					mode_id = MODE_HEX;
				else if(!strcmp(optarg, "decimal") || !strcmp(optarg, "dec") || !strcmp(optarg, "d") || !strcmp(optarg, "10"))
					mode_id = MODE_DEC;
				else if(!strcmp(optarg, "octal") || !strcmp(optarg, "oct") || !strcmp(optarg, "o") || !strcmp(optarg, "8"))
					mode_id = MODE_OCT;
				else {
					fprintf(stderr, "+++ error: illegal argument in option -- i\n");
					usage(stderr, basename(argv[0]));
					return 1;
				}
				break;
			case 'd':
				if(md++) {
					fprintf(stderr, "+++ error: conflict in option -- d\n");
					usage(stderr, basename(argv[0]));
					return 1;
				}
				if(optarg && optarg[0] == '-') {
					fprintf(stderr, "+++ error: missing argument in option -- d\n");
					usage(stderr, basename(argv[0]));
					return 1;
				}
				if(!strcmp(optarg, "hexadecimal") || !strcmp(optarg, "hex") || !strcmp(optarg, "h") || !strcmp(optarg, "16"))
					mode_data = MODE_HEX;
				else if(!strcmp(optarg, "decimal") || !strcmp(optarg, "dec") || !strcmp(optarg, "d") || !strcmp(optarg, "10"))
					mode_data = MODE_DEC;
				else if(!strcmp(optarg, "octal") || !strcmp(optarg, "oct") || !strcmp(optarg, "o") || !strcmp(optarg, "8"))
					mode_data = MODE_OCT;
				else {
					fprintf(stderr, "+++ error: illegal argument in option -- d\n");
					usage(stderr, basename(argv[0]));
					return 1;
				}
				break;
			case 'a':
				if(ma++) {
					fprintf(stderr, "+++ error: conflict in option -- a\n");
					usage(stderr, basename(argv[0]));
					return 1;
				}
				if(optarg && optarg[0] == '-') {
					fprintf(stderr, "+++ error: missing argument in option -- a\n");
					usage(stderr, basename(argv[0]));
					return 1;
				}
				if(!strcmp(optarg, "off") || !strcmp(optarg, "no") || !strcmp(optarg, "n") || !strcmp(optarg, "0"))
					mode_ascii = ASCII_OFF;
				else if(!strcmp(optarg, "on") || !strcmp(optarg, "yes") || !strcmp(optarg, "y") || !strcmp(optarg, "1"))
					mode_ascii = ASCII_ON;
				else {
					fprintf(stderr, "+++ error: illegal argument in option -- a\n");
					usage(stderr, basename(argv[0]));
					return 1;
				}
				break;
			case 'x':
				if(ex++) {
					fprintf(stderr, "+++ error: conflict in option -- x\n");
					usage(stderr, basename(argv[0]));
					return 1;
				}
				if(optarg && optarg[0] == '-') {
					fprintf(stderr, "+++ error: missing argument in option -- x\n");
					usage(stderr, basename(argv[0]));
					return 1;
				}
				if(!get_exclusion(optarg)) {
					fprintf(stderr, "+++ error: illegal argument in option -- x\n");
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
	if(argc >= optind + 2) {
		fprintf(stderr, "+++ error: too many arguments\n");
		usage(stderr, basename(argv[0]));
		return 1;
	}
	else
		can_param.ifname = argv[optind];
	
	fprintf(stdout, "%s (CAN Monitor using socketCAN), version %s of %s\n",
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

	fprintf(stderr, "\nPress ^C to abort.\n\n");

	while(running) {
		if((rc = can_receive(&can_msg)) == CANERR_NOERROR) {
			if(((can_msg.id < MAX_ID) && can_id[can_msg.id]) || (can_msg.id >= MAX_ID)) {
				fprintf(stdout, "%li\t", frames++);
				print_time(stdout, (struct timeval*)&can_msg.timestamp, mode_time);
				print_id(stdout, can_msg.id, mode_id);
				if(!can_msg.rtr) {
					fprintf(stdout, "rcv\t");
					for(ch = 0; ch < can_msg.dlc; ch++)
						print_data(stdout, can_msg.data[ch], (ch==7), mode_data);
					for(ch = ch; ch < 8; ch++)
						print_space(stdout, (ch==7), mode_data);
					if(mode_ascii) {
						fprintf(stdout, "\t");
						for(ch = 0; ch < can_msg.dlc; ch++)
							if((32 <= can_msg.data[ch]) && (can_msg.data[ch] <= 127))
								fprintf(stdout, "%c", can_msg.data[ch]);
							else
								fprintf(stdout, ".");
					}
				}
				else {
					fprintf(stdout, "rtr\tdlc=%u", can_msg.dlc);
				}
				fprintf(stdout, "\n");
			}
		}
		else if(rc == CANERR_RX_EMPTY)
			usleep(1);
	}
	fprintf(stdout, "\n");
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

void sigterm(int signo)
{
	/*printf("got signal %d\n", signo);*/
	running = 0;
}

void version(FILE *stream, char *program)
{
	fprintf(stream, "%s (CAN Monitor using socketCAN), version %s of %s\n",
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
	fprintf(stream, "Options:\n");
#ifndef __no_can_ioctl
	fprintf(stream, " -b, --baudrate=<baudrate>                    bit timing in kbps (default=125)\n");
#endif
	fprintf(stream, " -t, --time=(abs|rel|zero)                    absolute or relative time\n");
	fprintf(stream, " -i  --id=(hex|dec|oct)                       display mode of COB-IDs\n");
	fprintf(stream, " -d, --data=(hex|dec|oct)                     display mode of data bytes\n");
	fprintf(stream, " -a, --ascii=(on|off)                         display data bytes as ascii\n");
	fprintf(stream, " -x, --exclude=[~]<id>[-<id>]{,<id>[-<id>]}   exclude COB-IDs (~ means inclusion)\n");
//	fprintf(stream, " -s, --script=<filename>                      execute a script file\n");
	fprintf(stream, " -h, --help                                   display this help and exit\n");
	fprintf(stream, "     --version                                show version information and exit\n");
}

/* ***	local functions  ***
 */

static void print_id(FILE *stream, unsigned long id, int mode)
{
	switch(mode) {
		case MODE_DEC:
			fprintf(stream, "%lu\t", id);
			break;
		case MODE_OCT:
			fprintf(stream, "\\%04lo\t", id);
			break;
		case MODE_HEX:
			fprintf(stream, "%03lX\t", id);
			break;
	}	
}

static void print_data(FILE *stream, unsigned char data, int last, int mode)
{
	switch(mode) {
		case MODE_DEC:
			if(!last)
				fprintf(stream, "%-3u ", data);
			else
				fprintf(stream, "%-3u", data);
			break;
		case MODE_OCT:
			if(!last)
				fprintf(stream, "\\%03o ", data);
			else
				fprintf(stream, "\\%03o", data);
			break;
		case MODE_HEX:
			if(!last)
				fprintf(stream, "%02X ", data);
			else
				fprintf(stream, "%02X", data);
			break;
	}	
}

static void print_space(FILE *stream, int last, int mode)
{
	switch(mode) {
		case MODE_DEC:
			if(!last)
				fprintf(stream, "    ");
			else
				fprintf(stream, "   ");
			break;
		case MODE_OCT:
			if(!last)
				fprintf(stream, "     ");
			else
				fprintf(stream, "    ");
			break;
		case MODE_HEX:
			if(!last)
				fprintf(stream, "   ");
			else
				fprintf(stream, "  ");
			break;
	}	
}

static void print_time(FILE *stream, struct timeval *timestamp, int mode)
{
	struct timeval difftime;
	struct tm tm; char timestring[25];
	
	switch(mode) {
		case TIME_REL:
		case TIME_ZERO:
			if (laststamp.tv_sec == 0)	/* first init */
			    laststamp = *timestamp;
			difftime.tv_sec  = timestamp->tv_sec  - laststamp.tv_sec;
			difftime.tv_usec = timestamp->tv_usec - laststamp.tv_usec;
			if (difftime.tv_usec < 0)
			    difftime.tv_sec--, difftime.tv_usec += 1000000;
			if (difftime.tv_sec < 0)
			    difftime.tv_sec = difftime.tv_usec = 0;

			tm = *gmtime(&difftime.tv_sec);
			strftime(timestring, 24, "%H:%M:%S", &tm);
			fprintf(stream, "%s.%04li\t", timestring, difftime.tv_usec / 100L);
				
			if (mode == TIME_REL)
			    laststamp = *timestamp;	/* update for delta calculation */
			break;
		case TIME_ABS:
			tm = *localtime(&timestamp->tv_sec);
			strftime(timestring, 24, "%H:%M:%S", &tm);
			fprintf(stream, "%s.%04li\t", timestring, timestamp->tv_usec / 100L);
			break;
	}
}

static int get_exclusion(char *arg)
{
	char *val, *end;
	int i, inv = 0;
	long id, last = -1;
	
	if(!arg)
		return 0;
	else
		val = arg;
	if(*val == '~') {
		inv = 1;
		val++;
	}
	while(1) {
		errno = 0;
		id = strtol(val, &end, 0);
	
		if(errno == ERANGE && (id == LONG_MAX || id == LONG_MIN))
			return 0;
		if(errno != 0 && id == 0)
			return 0;
		if(val == end)
			return 0;
		
		if(id < MAX_ID)
			can_id[id] = 0;
		
		if(*end == '\0') {
			if(last != -1) {
				while(last != id) {
					if(last < id)
						last++;
					else
						last--;
					can_id[last] = 0;
				}
				last = -1;
			}
			break;
		}
		if(*end == ',') {
			if(last != -1) {
				while(last != id) {
					if(last < id)
						last++;
					else
						last--;
					can_id[last] = 0;
				}
				last = -1;
			}
		}
		else if(*end == '-')
			last = id;
		else
			return 0;
		
		val = ++end;
	}
	if(inv) {
		for(i = 0; i < MAX_ID; i++)
			can_id[i] = !can_id[i];
	}
	return 1;
}
/* ***	end of file  ***
 */
 
