/*
 * SCCSID: @(#)cont.c	3.0	4/22/86
 */

/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*

 Facility:	Part of the plot package for the REGIS terminals.

 Called by:	driver(),line(),point()

 Abstract:	cont(x0,y0) - draws a line from the current point to (x0,y0)

 Author:	Kevin J. Dunlap

 Creation:	March 1985.

*/

#include <stdio.h>

cont(x0,y0){
extern ggi, xnow, ynow;
	xnow = xsc(x0);
	ynow = ysc(y0);
	fprintf(ggi,"V[%d,%d]\n",xnow,ynow);
}

