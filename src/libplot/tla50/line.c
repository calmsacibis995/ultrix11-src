/*
 * SCCSID:  @(#)line.c	3.0	4/22/86
 */

/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*

 Facility:	Part of the Plot package for the LA50 printer

 Called by:	driver()

 Calls: 	move(), and cont()

 Abstract:	line(x0,y0,x1,y1) draw a line from (x0,y0) to (x1,y1)

	This is just a couple of calls, move the current point to (x0,y0)
	and then draw the line to (x1,y1).

 Modified by:

 Date:		By:		Reason:

 ??-Mar-1984	David Roberts.	Add the comments and the debugging output.

*/

#include <stdio.h>
#define DEBUG 0
line(x0,y0,x1,y1)
{
#if DEBUG
	fprintf(stderr,"line: draw from %d,%d to %d,%d \n",x0,y0,x1,y1);
#endif
	move(x0,y0);	/* move to new current position and */
	cont(x1,y1);	/* draw from there to x1,y1 */
}
#undef DEBUG
