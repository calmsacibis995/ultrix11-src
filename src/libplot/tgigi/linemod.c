
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)linemod.c	3.0	4/22/86
 */
#include <stdio.h>
extern ggi;
linemod(s)
char *s;
{
	char c;
	switch(s[0]){
	case 'l':	
		c = '2';		/* longdashed */
		break;
	case 'd':	
		if(s[3] != 'd')c='4';	/* dotted */
		else c='3';		/* dotdashed */
		break;
	case 's':
		if(s[5] != '\0')c='5';	/* shortdashed */
		else c='1';		/* solid */
	}
	fprintf(ggi, "W(P%c)\n", c);
}
