/*
 * SCCSID: @(#)close.c	3.0	4/22/86
 */

/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*

 Facility:	Part of the plot package for the LA100 Printer

 Called by:	driver()

 Calls: 	drawmap

 Abstract:	Call drawmap to print the bitmap to the printer and
		then turn off the printers graphic mode.

 Author:	Kevin J. Dunlap

 Creation:	March 1984

*/

closepl()
{
	drawmap();
	printf("\033\\");		/* turn off graphic mode on la100 */
	printf("\33[4i");
}

