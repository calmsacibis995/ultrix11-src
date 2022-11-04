
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)move.c	3.0	4/22/86
 */
#include <stdio.h>
extern ggi;
extern xnow,ynow;

move(xi,yi)
{
	xnow = xsc(xi);
	ynow = ysc(yi);
	fprintf(ggi, "p[%d,%d]\n", xnow, ynow);
}
