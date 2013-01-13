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

/**************************************************************************
 * Main entry point of trimdump.
 **************************************************************************/

int main (int argc, char **argv)
{

    int			inp;		/* Input port */
    int			pkt_len;
    int			pkt_type;
    unsigned char	pkt_data[MAX_PKT_LEN+1];



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
     * Read packets
     ******************************************/

    while ((pkt_type = packet(inp, &pkt_len, pkt_data, MAX_PKT_LEN))
	   != PKT_EOF) {

	print_packet(pkt_type, pkt_len, pkt_data);

    }
    
    close(inp);
    
    return 0;
}

