
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)time.c	3.0	4/22/86
 */
#include <stdio.h>
extern ggi;

time(t) 	/* cause gigi to delay reading next command */
{
	int i;
	for(i = 0; i < (t*60)/255; i++)
		fprintf(ggi, "s(t255)\n");
	fprintf(ggi,"s(t%d)\n", (t*60)%255);
}
