/*
 * SCCSID: @(#)point.c	3.0 4/22/86
 */

/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*

 Facility:	Part of the Plot package for the LA50 printer.

 Called by:	driver()

 Calls: 	move() and cont()

 Abstract:	point(xi,yi)  - plot the point given by (xi,yi)

*/

point(xi,yi)
{
	move(xi,yi);
	cont(xi,yi);
}
