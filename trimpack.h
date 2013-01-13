/**************************************************************************
 * Copyright (C) 2009 Mark J. Blair, NF6X
 *
 * This file is part of trimpack.
 *
 *  trimpack is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  trimpack is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with trimpack.  If not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/


#ifndef _TRIMPACK_H_
#define _TRIMPACK_H_

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <termios.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>


#define PKT_EOF 0x03

/**************************************************************************
 * Lexical analyzer created by  lex/flex. Reads from input stream,
 * returning complete packets. Returns packet type, or PKT_EOF for EOF.
 **************************************************************************/
extern unsigned char packet(int inp, int *pkt_len, unsigned char *pkt_data,
			    int max_pkt_len);

#define MAX_PKT_LEN 1024


/**************************************************************************
 * Utility functions for interpreting and creating packets
 **************************************************************************/
extern void dump_pkt(int pkt_len, unsigned char *pkt_data);
extern int parse_integer(unsigned char *d);
extern float parse_single(unsigned char *d);
extern double parse_double(unsigned char *d);
extern void pack_single(float f, unsigned char *buf);
extern void print_packet (unsigned char pkt_type,
			  int pkt_len, unsigned char *pkt_data);


/**************************************************************************
 * Open a serial port, configure baud rate for TRIMPACK, return
 * file descriptor. exit(1) on fail.
 **************************************************************************/
extern int open_trimpack(char *path);


/**************************************************************************
 * Send a packet to a Trimpack
 **************************************************************************/

extern void send_packet(int fd, unsigned char pkt_type, int pkt_len,
			unsigned char *pkt_data);


#endif // _TRIMPACK_H_
