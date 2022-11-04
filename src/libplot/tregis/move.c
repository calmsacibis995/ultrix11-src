/*
 * SCCSID: @(#)move.c	3.0	4/22/86
 */

/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*

 Facility:	Part of the Plot package for the REGIS Terminals 

 Called by:	driver()

 Calls: 	xsc() and ysc()

 Abstract:	move(xi,yi) - move to new current point (xi,yi)

 Author:	Jerry Brenner	

 Modified by:

 Date:		By:		Reason:

 02-Apr-1985    Kevin J. Dunlap	 Add comments and convert from GIGI plot
				 to REGIS plot
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
