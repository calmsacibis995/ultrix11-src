/*
 * SCCSID: @(#)erase.c	3.0	4/22/86
 */

/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*

 Facility:	Part of the plot package for the LA100 Printer.

 Called by:	driver()

 Abstract:	Erase a page.  In this case being for a printer, just
		do a form feed.

 Author:	Kevin J. Dunlap

 Creation:	February 1984

*/

erase()
{
	printf("\033\\");          /*  Turn off graphics mode. */
	printf("\f");              /*  Print form feed.        */
	printf("\033\120\161");    /*  Turn on graphics mode.  */
}
