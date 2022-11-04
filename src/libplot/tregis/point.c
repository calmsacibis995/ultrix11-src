/*
 * SCCSID: @(#)point.c	3.0	4/22/86 
 */

/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*

 Facility:	Part of the Plot package for the REGIS terminals.

 Called by:	driver()

 Calls: 	move() 

 Abstract:	point(xi,yi)  - plot the point given by (xi,yi)

 Modified by:

 Date:		By:		Reason:

02-Apr-1985	Kevin J. Dunlap	Comments and add REGIS code

*/

#include <stdio.h>
extern ggi;

point(xi,yi)
{
	move(xi, yi);
	fprintf(ggi, "v[]\n");
}
