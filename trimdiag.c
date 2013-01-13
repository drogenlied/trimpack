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
 *
 * $Id: $
 **************************************************************************/


#include "trimpack.h"

/* flags to keep track of which packets have been received */
#define GOT_46	0x01
#define GOT_4B  0x02
#define GOT_ALL	(GOT_46 | GOT_4B)


#define TIMEOUT 5

/**************************************************************************
 * Timeout handler
 **************************************************************************/

void alarm_handler (int sig) {
    fprintf(stderr, "Timeout!\n");
    exit(1);
}



/**************************************************************************
 * Main entry point of trimdiag.
 **************************************************************************/

int main (int argc, char **argv)
{

    int			inp;		/* Input port */
    unsigned char	pkt_data[MAX_PKT_LEN+1];
    int			pkt_len;
    int			pkt_type;
    struct sigaction	new_action;

    int			pkt_flags = 0;


    /******************************************
     * Parse arguments
     ******************************************/
    
    if (argc != 2) {
	fprintf(stderr, "\nUsage: %s <port>\n\n", argv[0]);
	exit(1);
    }


    /******************************************
     * Open input port.
     ******************************************/
    
    inp = open_trimpack(argv[1]);


    /******************************************
     * Set up watchdog handler
     ******************************************/

    new_action.sa_handler = alarm_handler;
    new_action.sa_flags = 0;
    new_action.sa_mask = 0;
    sigaction(SIGALRM, &new_action, NULL);
    

    /******************************************
     * Get software versions
     ******************************************/

    alarm(TIMEOUT);
    send_packet(inp, 0x1F, 0, NULL);
    while ((pkt_type = packet(inp, &pkt_len, pkt_data, MAX_PKT_LEN)) != 0x45);
    alarm(0);
    print_packet(pkt_type, pkt_len, pkt_data);


    /******************************************
     * Get health
     ******************************************/

    pkt_flags = 0;
    alarm(TIMEOUT);
    send_packet(inp, 0x26, 0, NULL);
    while (pkt_flags != GOT_ALL) {
	pkt_type = packet(inp, &pkt_len, pkt_data, MAX_PKT_LEN);

	if ((pkt_type == 0x46) && !(pkt_flags & GOT_46)) {
	    pkt_flags |= GOT_46;
	    print_packet(pkt_type, pkt_len, pkt_data);
	} else if ((pkt_type == 0x4B) && !(pkt_flags & GOT_4B)) {
	    pkt_flags |= GOT_4B;
	    print_packet(pkt_type, pkt_len, pkt_data);
	}
    }
    alarm(0);


    /******************************************
     * Get operating parameters
     ******************************************/

    pkt_data[0] = 0;
    pack_single(-1.0, &pkt_data[1]);
    pack_single(-1.0, &pkt_data[5]);
    pack_single(-1.0, &pkt_data[9]);
    pack_single(-1.0, &pkt_data[12]);

    alarm(TIMEOUT);
    send_packet(inp, 0x2C, 17, pkt_data);
    while ((pkt_type = packet(inp, &pkt_len, pkt_data, MAX_PKT_LEN)) != 0x4C);
    alarm(0);
    print_packet(pkt_type, pkt_len, pkt_data);


    /******************************************
     * Get I/O options
     ******************************************/

    alarm(TIMEOUT);
    send_packet(inp, 0x35, 0, NULL);
    while ((pkt_type = packet(inp, &pkt_len, pkt_data, MAX_PKT_LEN)) != 0x55);
    alarm(0);
    print_packet(pkt_type, pkt_len, pkt_data);

    
    /******************************************
     * Get oscillator offset
     ******************************************/

    alarm(TIMEOUT);
    send_packet(inp, 0x2D, 0, NULL);
    while ((pkt_type = packet(inp, &pkt_len, pkt_data, MAX_PKT_LEN)) != 0x4D);
    alarm(0);
    print_packet(pkt_type, pkt_len, pkt_data);


    /******************************************
     * Get A-D readings
     ******************************************/

    alarm(TIMEOUT);
    send_packet(inp, 0x33, 0, NULL);
    while ((pkt_type = packet(inp, &pkt_len, pkt_data, MAX_PKT_LEN)) != 0x53);
    alarm(0);
    print_packet(pkt_type, pkt_len, pkt_data);


    close(inp);
    
    return 0;
}
