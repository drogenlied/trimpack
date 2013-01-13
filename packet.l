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


%{
#include "trimpack.h"

    unsigned char	pkt_type = 0;

    static int		yy_inp;
    
#define YY_DECL unsigned char packet(int inp, int *pkt_len, unsigned char *pkt_data, int max_pkt_len)

#define YY_NO_INPUT

#define YY_INPUT(buf, result, max_size) \
    {					\
	char	c;			\
	if (read(yy_inp, &c, 1) != 1) {	\
	    result = YY_NULL;		\
	} else {			\
	    result = 1;			\
	    buf[0] = c;			\
	}				\
    }

/* undefine conflicting definition from termios.h */
#ifdef ECHO
#undef ECHO
#endif

%}

%option 8bit
%option noyywrap



 /* The IN_PACKET start condition is used when we are accumulating data
    for a packet. We might start reading in the middle of a packet, which we
    must discard. */

%x IN_PACKET

 /* Character 0x10 (DLE) is used in packet start and end markers. When
    this character must be included in packet data, it will be escaped
    by repeating it twice. We will collapse any two 0x10 chars in a row to a
    single 0x10 character. */

DLE		\x10\x10

 /* Start and end of packet markers. Start of packet marker includes the
    packet type as its second byte. */

START		\x10[^\x10\x03]
END		\x10\x03



%%

%{
    yy_inp = inp;
%}

<<EOF>>			{
    return PKT_EOF;
  }

<INITIAL>{DLE}		{
    /* discard escaped DLE outside of packet */
 }

<INITIAL>{START}	{
    /* Start of new packet found. */
    *pkt_len  = 0;
    pkt_type = yytext[1];
    BEGIN(IN_PACKET);
 }

<INITIAL>[\x00-\xff]		{
    /* discard characters outside of packet. */
 }

<IN_PACKET>{END}	{
    /* Packet done. */
    BEGIN(INITIAL);
    return pkt_type;
 }

<IN_PACKET>{DLE}	{
    /* Collapse an escaped DLE char */
    if (*pkt_len >= max_pkt_len) {
	fprintf(stderr, "\nERROR: Giant packet encountered.\n\n");
	exit(1);
    }
    pkt_data[(*pkt_len)++] = 0x10;
 }

<IN_PACKET>[\x00-\xff]		{
    /* Add any other character to the packet payload. */
    if (*pkt_len >= max_pkt_len) {
	fprintf(stderr, "\nERROR: Giant packet encountered.\n\n");
	exit(1);
    }
    pkt_data[(*pkt_len)++] = yytext[0];
 }

%%

/* This function should not be called. It's here only to suppress a
   spurious warning abuot yyunput() being defined but not used when
   compiling with -Wall. */
void do_nothing(void) {
    yyunput(0, yytext);
}

/* For emacs: */
/* Local Variables: */
/* mode:c */
/* End: */

