
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)color.c	3.0	4/22/86
 */
#include <stdio.h>
extern ggi;

scolor(s)			 /* set screen background color */
char *s;
{
	char c;
	switch(s[0])
	{
		case 'd':		/* 'd'ark or 0 */
		case '\0':
		default :
			c = '0';	/* dark */
			break;
		case 'b':		/* 'b'lue or 1 */
		case '\01':
			c = '1';	/* blue */
			break;
		case 'r':		/* 'r'ed or 2 */
		case '\02':
			c = '2';	/* red */
			break;
		case 'm':
		case '\03':
			c = '3';	/* magenta */
			break;
		case 'g':
		case '\04':
			c = '4';	/* green */
			break;
		case 'c':
		case '\05':
			c = '5';	/* cyan */
			break;
		case 'y':
		case '\06':
			c = '6';	/* yellow */
			break;
		case 'w':
		case '\07':
			c = '7';	/* white */
			break;
	}
	fprintf(ggi, "s(i%c)\n", c);
}
