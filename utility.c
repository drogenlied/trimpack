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

#include "trimpack.h"

/**************************************************************************
 * Hex dump of packet contents.
 **************************************************************************/

void dump_pkt(int pkt_len, unsigned char *pkt_data) {
    int n;
    printf(" ");
    for (n=0; n<pkt_len; n++) {
	printf(" %2.2X", pkt_data[n]);
	if (((n % 8) == 7) && (n<(pkt_len-1))) {
	    printf("\n ");
	}
    }
    printf("\n\n");
}


/**************************************************************************
 * Unpacks integer in packet.
 **************************************************************************/

int parse_integer(unsigned char *d) {
    return ((int)d[0] << 8) | d[1];
}


/**************************************************************************
 * Unpacks single-precision floating point number in packet.
 * Assumes that this computer uses IEEE-954 floating point number
 * format, and simply repacks the bytes to deal with endianness issues.
 **************************************************************************/


float parse_single(unsigned char *d) {
    uint32_t	i;
    float	f;

    i = ((uint32_t)d[0] << 24)
	| ((uint32_t)d[1] << 16)
	| ((uint32_t)d[2] << 8)
	| (uint32_t)d[3];

    f = *(float *)&i;
    return f;
}


/**************************************************************************
 * Unpacks double-precision floating point number in packet.
 * Assumes that this computer uses IEEE-954 floating point number
 * format, and simply repacks the bytes to deal with endianness issues.
 **************************************************************************/

double parse_double(unsigned char *d) {
    uint64_t	i;
    double	f;

    i = ((uint64_t)d[0] << 56)
	| ((uint64_t)d[1] << 48)
	| ((uint64_t)d[2] << 40)
	| ((uint64_t)d[3] << 32)
	| ((uint64_t)d[4] << 24)
	| ((uint64_t)d[5] << 16)
	| ((uint64_t)d[6] << 8)
	| (uint64_t)d[7];

    f = *(double *)&i;
    return f;
}



/**************************************************************************
 * Packs single-precision floating point number into packet.
 * Assumes that this computer uses IEEE-954 floating point number
 * format, and simply repacks the bytes to deal with endianness issues.
 * Assumes that buffer is large enough (4 bytes).
 **************************************************************************/

void pack_single(float f, unsigned char *buf) {
    uint32_t	i;

    i = *(uint32_t *)&f;
    
    buf[3] = i & 0xff;
    i >>= 8;
    
    buf[2] = i & 0xff;
    i >>= 8;

    buf[1] = i & 0xff;
    i >>= 8;

    buf[0] = i & 0xff;
}


/**************************************************************************
 * Open a serial port, configure baud rate for TRIMPACK, return
 * file descriptor. exit(1) on fail.
 **************************************************************************/

int open_trimpack(char *path) {
    int			fd;
    struct termios	fd_termios;

    fd = open(path, O_RDWR);

    if (fd == 0) {
	perror("Could not open port");
	exit(1);
    }

    if (tcgetattr(fd, &fd_termios)) {
	perror("Could not get serial port settings");
	exit(1);
    }

    cfsetspeed(&fd_termios, B9600);
    fd_termios.c_iflag = IGNBRK;
    fd_termios.c_oflag = 0;
    fd_termios.c_cflag = CS8 | PARENB | PARODD | CREAD | CLOCAL;
    fd_termios.c_lflag = 0;

    if (tcsetattr(fd, TCSAFLUSH, &fd_termios)) {
	perror("Could not set serial port settings");
	exit(1);
    }

    return fd;
}


/**************************************************************************
 * Send a packet to a Trimpack
 **************************************************************************/

void send_packet(int fd, unsigned char pkt_type, int pkt_len,
		 unsigned char *pkt_data) {

    unsigned char	buf[2];
    int			n;
    
    buf[0] = 0x10;
    buf[1] = pkt_type;
    write(fd, &buf, 2);

    if (pkt_data) {
	for (n = 0; n < pkt_len; n++) {
	    buf[0] = pkt_data[n];
	    write(fd, &buf, 1);
	    if (buf[0] == 0x10) {
		// escape DLE by sending twice
		write(fd, &buf, 1);
	    }
	}
    }

    buf[0] = 0x10;
    buf[1] = 0x03;
    write(fd, &buf, 2);
}



/**************************************************************************
 * Print human-readable description of packet.
 * Major brute force!
 **************************************************************************/

void print_packet (unsigned char pkt_type,
		   int pkt_len, unsigned char *pkt_data) {

    int 	n;

    switch (pkt_type) {

	/***********************************/
      case 0x40:
	printf("Almanac Data Page\n");
	if (pkt_len != 39) {
	    printf("  BAD FORMAT\n");
	    dump_pkt(pkt_len, pkt_data);
	} else {
	    printf("  satellite:     %d\n", pkt_data[0]);
	    printf("  T_zc:          %f s\n", parse_single(&pkt_data[1]));
	    printf("  week number:   %d\n", parse_integer(&pkt_data[5]));
	    printf("  eccentricity:  %f\n", parse_single(&pkt_data[7]));
	    printf("  T_oa:          %f s\n", parse_single(&pkt_data[11]));
	    printf("  i_o:           %f rad\n", parse_single(&pkt_data[15]));
	    printf("  OMEGA_dot:     %f rad/s\n",
		   parse_single(&pkt_data[19]));
	    printf("  square root A: %f m^1/2\n",
		   parse_single(&pkt_data[23]));
	    printf("  OMEGA o:       %f rad\n", parse_single(&pkt_data[27]));
	    printf("  omega:         %f rad\n", parse_single(&pkt_data[31]));
	    printf("  M o:           %f rad\n", parse_single(&pkt_data[35]));
	    printf("\n");
	}
	break;

	/***********************************/
      case 0x41:
	printf("GPS Time\n");
	if (pkt_len != 10) {
	    printf("  BAD FORMAT\n");
	    dump_pkt(pkt_len, pkt_data);
	} else {
	    printf("  GPS time of week: %f\n", parse_single(&pkt_data[0]));
	    printf("  GPS week number:  %d\n", parse_integer(&pkt_data[4]));
	    printf("  GPS/UTC offset:   %f\n", parse_single(&pkt_data[6]));
	    printf("\n");
	}
	break;

	/***********************************/
      case 0x42:
	printf("Single-precision Position Fix, XYZ ECEF\n");
	if (pkt_len != 16) {
	    printf("  BAD FORMAT\n");
	    dump_pkt(pkt_len, pkt_data);
	} else {
	    printf("  X:           %f m\n", parse_single(&pkt_data[0]));
	    printf("  Y:           %f m\n", parse_single(&pkt_data[4]));
	    printf("  Z:           %f m\n", parse_single(&pkt_data[8]));
	    printf("  time-of-fix: %f s\n", parse_single(&pkt_data[12]));
	    printf("\n");
	}
	break;

	/***********************************/
      case 0x43:
	printf("Velocity Fix, XYZ ECEF\n");
	if (pkt_len != 20) {
	    printf("  BAD FORMAT\n");
	    dump_pkt(pkt_len, pkt_data);
	} else {
	    printf("  X velocity:  %f m/s\n", parse_single(&pkt_data[0]));
	    printf("  Y velocity:  %f m/s\n", parse_single(&pkt_data[4]));
	    printf("  Z velocity:  %f m/s\n", parse_single(&pkt_data[8]));
	    printf("  bias rate:   %f m/s\n", parse_single(&pkt_data[12]));
	    printf("  time-of-fix: %f s\n", parse_single(&pkt_data[16]));
	    printf("\n");
	}
	break;

	/***********************************/
      case 0x44:
	printf("Satellite Selection\n");
	if (pkt_len != 21) {
	    printf("  BAD FORMAT\n");
	    dump_pkt(pkt_len, pkt_data);
	} else {
	    printf("  Mode:              ");
	    switch (pkt_data[0]) {
	      case 0x01:
		printf("Auto, 1-satellite, 0-D\n");
		break;
		    
	      case 0x03:
		printf("Auto, 3-satellite, 2-D\n");
		break;
		    
	      case 0x04:
		printf("Auto, 4-satellite, 3-D\n");
		break;
		    
	      case 0x11:
		printf("Manual, 1-satellite, 0-D\n");
		break;
		    
	      case 0x13:
		printf("Manual, 3-satellite, 2-D\n");
		break;
		    
	      case 0x14:
		printf("Manual, 4-satellite, 3-D\n");
		break;

	      default:
		printf("UNKNOWN\n");
		break;
	    }

	    printf("  Satellite numbers:");
	    for (n=0; n<4; n++) {
		printf(" %d", (int)pkt_data[n+1]);
	    }
	    printf("\n");

	    printf("  PDOP:              %f\n",
		   parse_single(&pkt_data[5]));
	    printf("  HDOP:              %f\n",
		   parse_single(&pkt_data[9]));
	    printf("  VDOP:              %f\n",
		   parse_single(&pkt_data[13]));
	    printf("  TDOP:              %f\n",
		   parse_single(&pkt_data[17]));

	    printf("\n");

	}
	break;

	/***********************************/
      case 0x45:
	printf("Software Version Information\n");
	if (pkt_len != 10) {
	    printf("  BAD FORMAT\n");
	    dump_pkt(pkt_len, pkt_data);
	} else {
	    printf("  Navigation Processor: %d.%d %d/%d/%d\n",
		   pkt_data[0], pkt_data[1],
		   pkt_data[2], pkt_data[3], pkt_data[4] + 1900);
	    printf("  Signal Processor:     %d.%d %d/%d/%d\n",
		   pkt_data[5], pkt_data[6],
		   pkt_data[7], pkt_data[8], pkt_data[9] + 1900);
	    printf("\n");
	}
	break;

	/***********************************/
      case 0x46:
	printf("Health of TRIMPACK\n");
	if (pkt_len != 2) {
	    printf("  BAD FORMAT\n");
	    dump_pkt(pkt_len, pkt_data);
	} else {
	    printf("  Status: ");
	    switch(pkt_data[0]) {
	      case 0x00:
		printf("Doing position fixes\n");
		break;

	      case 0x01:
		printf("Don't have GPS time yet\n");
		break;

	      case 0x03:
		printf("PDOP is too high\n");
		break;

	      case 0x08:
		printf("No usable satellites\n");
		break;

	      case 0x09:
		printf("Only 1 usable satellite\n");
		break;

	      case 0x0A:
		printf("Only 2 usable satellites\n");
		break;

	      case 0x0B:
		printf("Only 3 usable satellites\n");
		break;

	      case 0x0C:
		printf("The chosen satellite is unusable\n");
		break;

	      default:
		printf("UNKNOWN\n");
		break;
	    }

	    if (pkt_data[1]) {
		int code = pkt_data[1];
		printf("  ERROR CODES: %2.2X\n", code);

		if (code & 1)
		    printf("    Battery back-up failed\n");
		code >>= 1;
		    
		if (code & 1)
		    printf("    Signal processor error\n");
		code >>= 1;
		    
		if (code & 1)
		    printf("    Alignment error, channel or chip 1\n");
		code >>= 1;
		    
		if (code & 1)
		    printf("    Alignment error, channel or chip 2\n");
		code >>= 1;
		    
		if (code & 1)
		    printf("    Antenna feedline fault\n");
		code >>= 1;
		    
		if (code & 1)
		    printf("    Excessive ref freq error\n");
		code >>= 1;
		    
		if (code & 1)
		    printf("    UNKNOWN 6\n");
		code >>= 1;
		    
		if (code & 1)
		    printf("    UNKNOWN 7\n");

		printf("\n");
	    }
	    printf("\n");
	}
	break;


	/***********************************/
      case 0x47:
	printf("Signal Levels for all Satellites\n");
	if (pkt_len != ((pkt_data[0] * 5) + 1)) {
	    printf("  BAD FORMAT\n");
	    dump_pkt(pkt_len, pkt_data);
	} else {
	    for (n = 0; n < pkt_data[0]; n++) {
		printf("  %2d: %f\n", pkt_data[(n*5)+1],
		       parse_single(&pkt_data[(n*5)+2]));
	    }

	    printf("\n");
	}
	break;

	/***********************************/
      case 0x48:
	printf("GPS System Message\n");
	if (pkt_len != 22) {
	    printf("  BAD FORMAT\n");
	    dump_pkt(pkt_len, pkt_data);
	} else {
	    printf("  ");
	    for (n = 0; n < 22; n++) {
		printf("%c", isprint(pkt_data[n]) ? pkt_data[n] : '_');
	    }

	    printf("\n");
	}
	break;

	/***********************************/
      case 0x49:
	printf("Almanac Health Page\n");
	if (pkt_len != 32) {
	    printf("  BAD FORMAT\n");
	    dump_pkt(pkt_len, pkt_data);
	} else {
	    for (n = 0; n < 32; n++) {
		printf("  %2d: %3d\n", n+1, pkt_data[n]);
	    }

	    printf("\n");
	}
	break;

	/***********************************/
      case 0x4A:
	printf("Single-precision LLA Position Fix\n");
	if (pkt_len != 20) {
	    printf("  BAD FORMAT\n");
	    dump_pkt(pkt_len, pkt_data);
	} else {
	    float	r, d;

	    r = parse_single(&pkt_data[0]);
	    d = r * (360.0/(2.0 * M_PI));
	    printf("  Latitude:    %f rad (%f deg)\n", r, d);

	    r = parse_single(&pkt_data[4]);
	    d = r * (360.0/(2.0 * M_PI));
	    printf("  Longitude:   %f rad (%f deg)\n", r, d);

	    r = parse_single(&pkt_data[8]);
	    printf("  Altitude:    %f m\n", r);

	    r = parse_single(&pkt_data[12]);
	    printf("  Clock Bias:  %f m\n", r);

	    r = parse_single(&pkt_data[16]);
	    printf("  Time-of-fix: %f s\n", r);

	    printf("\n");
	}
	break;

	/***********************************/
      case 0x4B:
	printf("Machine/Code ID and Additional Status\n");
	if (pkt_len != 3) {
	    printf("  BAD FORMAT\n");
	    dump_pkt(pkt_len, pkt_data);
	} else {
	    printf("  Machine ID: ");
	    switch (pkt_data[0]) {
	      case 6:
		printf("2-channel TRIMPACK\n");
		break;

	      case 10:
		printf("3-channel TRIMPACK\n");
		break;

	      default:
		printf("UNKNOWN %d\n", pkt_data[0]);
		break;
	    }
		
	    if (pkt_data[1]) {
		int code = pkt_data[1];
		printf("  Status 1: %2.2X\n", code);

		if (code & 1)
		    printf("    Synthesizer Fault\n");
		code >>= 1;
		    
		if (code & 1)
		    printf("    Battery Powered Time Clock Fault\n");
		code >>= 1;
		    
		if (code & 1)
		    printf("    A-to-D Converter Fault\n");
		code >>= 1;
		    
		if (code & 1)
		    printf("    The almanac stored in the TRIMPACK is not complete and current\n");
		code >>= 1;
		    
		if (code & 1)
		    printf("    UNKNOWN 4\n");
		code >>= 1;
		    
		if (code & 1)
		    printf("    UNKNOWN 5\n");
		code >>= 1;
		    
		if (code & 1)
		    printf("    UNKNOWN 6\n");
		code >>= 1;
		    
		if (code & 1)
		    printf("    UNKNOWN 7\n");
		code >>= 1;
		    
	    }

	    if (pkt_data[2]) {
		int code = pkt_data[2];
		printf("  Status 2: %2.2X\n", code);
	    }

	    printf("\n");

	}
	break;

	/***********************************/
      case 0x4C:
	printf("Report Operating Parameters\n");
	if (pkt_len != 17) {
	    printf("  BAD FORMAT\n");
	    dump_pkt(pkt_len, pkt_data);
	} else {
	    float	r, d;

	    printf("  Dynamics code:        ");

	    switch (pkt_data[0]) {
	      case 1:
		printf("land (< 120 knots)\n");
		break;
		    
	      case 2:
		printf("sea (< 50 knots)\n");
		break;
		    
	      case 3:
		printf("air (< 600 knots)\n");
		break;
		    
	      case 4:
		printf("static\n");
		break;
		    
	      default:
		printf("UNKNOWN\n");
		break;
	    }

	    r = parse_single(&pkt_data[1]);
	    d = r * (360.0/(2.0 * M_PI));
	    printf("  Elevation angle mask: %f rad (%f deg)\n", r, d);

	    printf("  Signal level mask:    %f\n",
		   parse_single(&pkt_data[5]));
	    printf("  PDOP mask:            %f\n",
		   parse_single(&pkt_data[9]));
	    printf("  PDOP switch:          %f\n",
		   parse_single(&pkt_data[13]));

	    printf("\n");
	}
	break;

	/***********************************/
      case 0x4D:
	printf("Oscillator Offset\n");
	if (pkt_len != 4) {
	    printf("  BAD FORMAT\n");
	    dump_pkt(pkt_len, pkt_data);
	} else {
	    printf("  %f Hz\n", parse_single(&pkt_data[0]));
	    printf("\n");
	}
	break;

	/***********************************/
      case 0x4E:
	printf("Response to Set GPS Time\n");
	if (pkt_len != 1) {
	    printf("  BAD FORMAT\n");
	    dump_pkt(pkt_len, pkt_data);
	} else {
	    printf("  %c\n", pkt_data[0]);
	    printf("\n");
	}
	break;

	    
	/***********************************/
      case 0x53:
	printf("A-to-D Readings\n");
	if (pkt_len != 32) {
	    printf("  BAD FORMAT\n");
	    dump_pkt(pkt_len, pkt_data);
	} else {
	    float	v, d;

	    v = parse_single(&pkt_data[0]);
	    d = (v/0.01) - 273.15;
	    printf("  Temperature:                 %f v (%f deg C)\n",
		   v, d);

	    v = parse_single(&pkt_data[4]);
	    printf("  Synthesizer Control Voltage: %f v\n", v);

	    v = parse_single(&pkt_data[8]);
	    printf("  IF Signal Level:             %f v\n", v);

	    v = parse_single(&pkt_data[12]);
	    printf("  External Antenna Voltage:    %f v\n", v);

	    v = parse_single(&pkt_data[16]) * 3.56;
	    printf("  Internal Battery Voltage:    %f v\n", v);

	    v = parse_single(&pkt_data[20]);
	    d = (v/0.01) - 273.15;
	    printf("  Display Temperature:         %f v (%f deg C)\n",
		   v, d);

	    v = parse_single(&pkt_data[24]);
	    printf("  Internal Antenna Voltage:    %f v\n", v);

	    v = parse_single(&pkt_data[28]);
	    printf("  +5V:                         %f v\n", v);

	    printf("\n");
	}
	break;
	    

	/***********************************/
      case 0x54:
	printf("One-Satellite Bias and Bias Rate\n");
	if (pkt_len != 12) {
	    printf("  BAD FORMAT\n");
	    dump_pkt(pkt_len, pkt_data);
	} else {
	    printf("  Bias:        %f m\n", parse_single(&pkt_data[0]));
	    printf("  Bias rate:   %f m/s\n", parse_single(&pkt_data[4]));
	    printf("  Time of fix: %f s\n", parse_single(&pkt_data[8]));

	    printf("\n");
	}
	break;


	/***********************************/
      case 0x55:
	printf("I/O Options\n");
	if (pkt_len != 4) {
	    printf("  BAD FORMAT\n");
	    dump_pkt(pkt_len, pkt_data);
	} else {
	    char	c;

	    printf("  position:\n");
	    c = pkt_data[0];

	    printf("    XYZ ECEF Output:              %s\n",
		   (c&1) ? "on" : "off");
	    c >>= 1;

	    printf("    LLA Output:                   %s\n",
		   (c&1) ? "on" : "off");
	    c >>= 1;

	    printf("    LLA ALT Output:               %s\n",
		   (c&1) ? "MSL geoid" : "HAE WGS-84");
	    c >>= 1;

	    printf("    ALT input:                    %s\n",
		   (c&1) ? "MSL geoid" : "HAE WGS-84");
	    c >>= 1;

	    printf("    precision-of-position output: %s\n",
		   (c&1) ? "double-precision" : "single-precision");
	    c >>= 1;

	    if (c) {
		printf("    UNKNOWN OPTIONS:              %2.2X\n",
		       pkt_data[0]);
	    }

	    printf("  velocity:\n");
	    c = pkt_data[1];

	    printf("    XYZ ECEF Output:              %s\n",
		   (c&1) ? "on" : "off");
	    c >>= 1;

	    printf("    ENU Output:                   %s\n",
		   (c&1) ? "on" : "off");
	    c >>= 1;

	    if (c) {
		printf("    UNKNOWN OPTIONS:              %2.2X\n",
		       pkt_data[1]);
	    }

		
	    printf("  timing:\n");
	    c = pkt_data[2];

	    printf("    time type:                    %s\n",
		   (c&1) ? "UTC" : "GPS time");
	    c >>= 1;

	    printf("    fix computation time:         %s\n",
		   (c&1) ? "next integer sec" : "ASAP");
	    c >>= 1;

	    printf("    output time:                  %s\n",
		   (c&1) ? "only on request" : "when computed");
	    c >>= 1;

	    if (c) {
		printf("    UNKNOWN OPTIONS:              %2.2X\n",
		       pkt_data[2]);
	    }

	    printf("  Auxiliary:\n");
	    c = pkt_data[3];

	    printf("    raw measurements:             %s\n",
		   (c&1) ? "on" : "off");
	    c >>= 1;

	    if (c) {
		printf("    UNKNOWN OPTIONS:              %2.2X\n",
		       pkt_data[3]);
	    }


	    printf("\n");
	}
	break;


	/***********************************/
      case 0x56:
	printf("Velocity Fix, East-North-Up (ENU)\n");
	if (pkt_len != 20) {
	    printf("  BAD FORMAT\n");
	    dump_pkt(pkt_len, pkt_data);
	} else {
	    printf("  East velocity:   %f m/s\n",
		   parse_single(&pkt_data[0]));
	    printf("  North velocity:  %f m/s\n",
		   parse_single(&pkt_data[4]));
	    printf("  Up velocity:     %f m/s\n",
		   parse_single(&pkt_data[8]));
	    printf("  Clock Bias Rate: %f m/s\n",
		   parse_single(&pkt_data[12]));
	    printf("  Time-of-Fix:     %f s\n",
		   parse_single(&pkt_data[16]));
	    printf("\n");
	}
	break;

	/***********************************/
      case 0x57:
	printf("Information About Last Computed Fix\n");
	if (pkt_len != 8) {
	    printf("  BAD FORMAT\n");
	    dump_pkt(pkt_len, pkt_data);
	} else {
	    printf("  Source of information: ");
	    switch(pkt_data[0]) {
	      case 0x00:
		printf("none\n");
		break;
		    
	      case 0x01:
		printf("regular fix\n");
		break;
		    
	      case 0x02:
	      case 0x04:
		printf("initialization diagnostic\n");
		break;
		    
	      case 0x05:
		printf("entered by packet 23 or 2B\n");
		break;
		    
	      case 0x06:
		printf("entered by packet 31 or 32\n");
		break;
		    
	      case 0x08:
		printf("default position after RAM battery fail\n");
		break;
		    
	      default:
		printf("UNKNOWN\n");
		break;
		    
	    }

	    printf("  Manuf. diagnostic:     %2.2X\n", pkt_data[1]);
	    printf("  Time of last fix:      %f s, GPS time\n",
		   parse_single(&pkt_data[2]));
	    printf("  Week of last fix:      %d weeks, GPS time\n",
		   parse_integer(&pkt_data[6]));
		
	    printf("\n");
	}
	break;

	/***********************************/
      case 0x58:
	printf("Satellite System Data/Acknowledge from TRIMPACK\n");
	if (pkt_len != pkt_data[3] + 4) {
	    printf("  BAD FORMAT\n");
	    dump_pkt(pkt_len, pkt_data);
	} else {
	    printf("  Operation:    ");
	    switch (pkt_data[0]) {
	      case 0:
		printf("Acknowledge, can't use\n");
		break;

	      case 1:
		printf("Acknowledge\n");
		break;

	      case 2:
		printf("Data Out\n");
		break;

	      case 3:
		printf("No Data on SV\n");
		break;

	      default:
		printf("UNKNOWN\n");
		break;
	    }

	    printf("  Type of data: ");
	    switch (pkt_data[1]) {
	      case 1:
		printf("not used\n");
		break;

	      case 2:
		printf("Almanac\n");
		break;

	      case 3:
		printf("Health page, T_oa, WN_oa\n");
		break;

	      case 4:
		printf("Ionosphere\n");
		break;

	      case 5:
		printf("UTC\n");
		break;

	      case 6:
		printf("Ephemeris\n");
		break;

	      default:
		printf("UNKNOWN\n");

	    }

	    printf("  Sat PRN #:    %d\n", pkt_data[2]);

	    dump_pkt(pkt_data[3], &pkt_data[4]);
	}
	break;

	/***********************************/
      case 0x59:
	printf("Status of Satellite Disable or Ignore Health\n");
	if (pkt_len != 33) {
	    printf("  BAD FORMAT\n");
	    dump_pkt(pkt_len, pkt_data);
	} else {
	    if (pkt_data[0] == 3) {
		for (n=1; n<=32; n++) {
		    printf("  PRN %2d: ", n);
		    if (pkt_data[n] == 0) {
			printf("enable satellite selection\n");
		    } else if (pkt_data[n] == 1) {
			printf("disable satellite selection\n");
		    } else {
			printf("UNKNOWN\n");
		    }
		}
	    } else if (pkt_data[0] == 6) {
		for (n=1; n<=32; n++) {
		    printf("  PRN %2d: ", n);
		    if (pkt_data[n] == 0) {
			printf("heed satellite health\n");
		    } else if (pkt_data[n] == 1) {
			printf("ignore satellite health\n");
		    } else {
			printf("UNKNOWN\n");
		    }
		}
	    } else {
		printf("  UNKNOWN OPERATION\n");
		dump_pkt(pkt_len, pkt_data);
	    }
	    printf("\n");
	}
	break;
	    
	/***********************************/
      case 0x5A:
	printf("Raw Measurement Data\n");
	if (pkt_len != 25) {
	    printf("  BAD FORMAT\n");
	    dump_pkt(pkt_len, pkt_data);
	} else {
	    printf("  PRN:                 %d\n", pkt_data[0]);
	    printf("  Sample Length:       %f ms\n",
		   parse_single(&pkt_data[1]));
	    printf("  Signal Level:        %f\n",
		   parse_single(&pkt_data[5]));
	    printf("  Code phase:          %f chips\n",
		   parse_single(&pkt_data[9]) / 16.0);
	    printf("  Doppler:             %f Hz\n",
		   parse_single(&pkt_data[13]));
	    printf("  Time of Measurement: %lf s\n",
		   parse_double(&pkt_data[17]));
		
	    printf("\n");
	}
	break;
	    
	/***********************************/
      case 0x5B:
	printf("Satellite Ephemeris Status\n");
	if (pkt_len != 16) {
	    printf("  BAD FORMAT\n");
	    dump_pkt(pkt_len, pkt_data);
	} else {
	    printf("  PRN:                     %d\n", pkt_data[0]);
	    printf("  GPS time of collection:  %f s\n",
		   parse_single(&pkt_data[1]));
	    printf("  Ephemeris health:        %2.2X\n", pkt_data[5]);
	    printf("  IODE:                    %2.2X\n", pkt_data[6]);
	    printf("  t_oe:                    %f s\n",
		   parse_single(&pkt_data[1]));
	    printf("  curve fit interval flag: %2.2X\n", pkt_data[11]);
	    printf("  URA:                     %f s\n",
		   parse_single(&pkt_data[12]));
		
	    printf("\n");
	}
	break;
	    
	/***********************************/
      case 0x5C:
	printf("Satellite Tracking Status\n");
	if (pkt_len != 24) {
	    printf("  BAD FORMAT\n");
	    dump_pkt(pkt_len, pkt_data);
	} else {
	    float	r, d;

	    printf("  PRN:                          %d\n", pkt_data[0]);
	    printf("  channel and slot code:        ");
	    printf("slot %d channel ", (pkt_data[1] & 0x07)+1);
	    if ((pkt_data[1] >>3) == 0x10) {
		// 2-channel trimpack only
		printf("2\n");
	    } else {
		printf("%d\n", (pkt_data[1] >> 3)+1);
	    }

	    printf("  acquisition flag:             ");
	    switch (pkt_data[2]) {
	      case 0:
		printf("never acquired\n");
		break;

	      case 1:
		printf("acquired\n");
		break;

	      case 2:
		printf("re-opened search\n");
		break;

	      default:
		printf("UNKNOWN\n");
		break;
	    }

	    printf("  ephemeris flag:               %s\n",
		   pkt_data[3] ? "good ephemeris" : "flag not set");

	    printf("  signal level:                 %f s\n",
		   parse_single(&pkt_data[4]));

	    printf("  GPS time of last measurement: %f s\n",
		   parse_single(&pkt_data[8]));

	    r = parse_single(&pkt_data[12]);
	    d = r * (360.0/(2.0 * M_PI));
	    printf("  elevation:                    %f rad (%f deg)\n",
		   r, d);

	    r = parse_single(&pkt_data[16]);
	    d = r * (360.0/(2.0 * M_PI));
	    printf("  azimuth:                      %f rad (%f deg)\n",
		   r, d);

	    printf("  old measurement flag:         %s\n",
		   pkt_data[20] ? "measurement too old" : "flag not set");

	    printf("  integer msec flag:            ");
	    switch (pkt_data[21]) {
	      case 0:
		printf("don't have good knowledge\n");
		break;

	      case 1:
		printf("msec from sub-frame data collection\n");
		break;

	      case 2:
		printf("verified by a bit crossinf time\n");
		break;

	      case 3:
		printf("verified by a successful position fix\n");
		break;

	      case 4:
		printf("suspected msec error\n");
		break;

	      default:
		printf("UNKNOWN\n");
		break;
	    }

	    printf("  bad data flag:                ");
	    switch (pkt_data[22]) {
	      case 0:
		printf("flag not set\n");
		break;

	      case 1:
		printf("bad parity\n");
		break;

	      case 2:
		printf("bad ephemeris health\n");
		break;

	      default:
		printf("UNKNOWN\n");
		break;

	    }

	    printf("  data collect flag:            %s\n",
		   pkt_data[22] ? "trying to collect data"
		   : "flag not set");

	    printf("\n");
	}
	break;
	    
	/***********************************/
      case 0x5F:
	printf("Failure Report\n");
	pkt_data[pkt_len] = 0x00;
	printf("  %s\n", pkt_data);
	break;

	/***********************************/
      case 0x83:
	printf("Double-precision Position Fix And Bias Information\n");
	if (pkt_len != 36) {
	    printf("  BAD FORMAT\n");
	    dump_pkt(pkt_len, pkt_data);
	} else {
	    printf("  X:           %lf m\n", parse_double(&pkt_data[0]));
	    printf("  Y:           %lf m\n", parse_double(&pkt_data[8]));
	    printf("  Z:           %lf m\n", parse_double(&pkt_data[16]));
	    printf("  clock bias:  %lf m\n", parse_double(&pkt_data[24]));
	    printf("  time-of-fix: %f s\n", parse_single(&pkt_data[32]));
	    printf("\n");
	}
	break;

	/***********************************/
      case 0x84:
	printf("Double-precision LLA Position Fix And Bias Information\n");
	if (pkt_len != 36) {
	    printf("  BAD FORMAT\n");
	    dump_pkt(pkt_len, pkt_data);
	} else {
	    double	r, d;

	    r = parse_double(&pkt_data[0]);
	    d = r * (360.0/(2.0 * M_PI));
	    printf("  latitude:    %lf rad (%lf deg)\n", r, d);

	    r = parse_double(&pkt_data[8]);
	    d = r * (360.0/(2.0 * M_PI));
	    printf("  longitude:   %lf rad (%lf deg)\n", r, d);

	    printf("  altitude:    %lf m\n", parse_double(&pkt_data[16]));
	    printf("  clock bias:  %lf m\n", parse_double(&pkt_data[24]));
	    printf("  time-of-fix: %f s\n", parse_single(&pkt_data[32]));
	    printf("\n");
	}
	break;

	/***********************************/
      case 0x90:
	printf("Waypoint Information\n");
	if ((pkt_len > 44) || (pkt_len < 43)) {
	    // Document specifies length 43, but newer 6-channel Trimpack
	    // appears to have an extra byte on the end
	    printf("  BAD FORMAT\n");
	    dump_pkt(pkt_len, pkt_data);
	} else {
	    printf("  Waypoint number:  %d\n", parse_integer(&pkt_data[0]));

	    printf("  User label field: ");
	    for (n=0; n<12; n++) {
		if (isprint(pkt_data[n+2])) {
		    printf("%c", pkt_data[n+2]);
		} else {
		    printf("_");
		}
	    }
	    printf("\n");

	    printf("  X:                %lf\n",
		   parse_double(&pkt_data[14]));
	    printf("  Y:                %lf\n",
		   parse_double(&pkt_data[22]));
	    printf("  Z:                %lf\n",
		   parse_double(&pkt_data[30]));

	    printf("  Altitude:         %f\n",
		   parse_single(&pkt_data[38]));

	    printf("  Time stamp code:  %2.2X", pkt_data[42]);
	    if (pkt_len == 44) {
		printf("%2.2X", pkt_data[43]);
	    }
		
	    printf("\n\n");
	}
	break;

	/***********************************/
      case 0x92:
	printf("Acknowledge 9X Packet Series\n\n");
	break;
	    
	/***********************************/
      case 0x93:
	printf("Negative Acknowledge\n\n");
	break;
	    
	/***********************************/
      default:
	printf("Unknown Packet Type %2.2X\n", pkt_type);
	dump_pkt(pkt_len, pkt_data);
	break;

    }
}
