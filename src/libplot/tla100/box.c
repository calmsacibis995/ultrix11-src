/*
 * SCCSID: @(#)box.c	3.0	4/22/86
 */

/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*

 Facility:	Part of the plot package for the LA100 Printer.

 Called by:	driver()

 Calls: 	move(), cont()

 Abstract:	Draw a box

 Author:	Kevin J. Dunlap

 Creation:	March 1984

*/

box(x0,y0,x1,y1)
{
	move(x0,y0);
	cont(x0,y1);
	cont(x1,y1);
	cont(x1,y0);
	cont(x0,y0);
	move(x1,y1);
}

