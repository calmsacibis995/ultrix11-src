/*
 * SCCSID: @(#)open.c	3.0 4/22/86
 */

/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*

 Facility:	Part of the plot package for the LA50 printer

 Called by:	driver()

 Abstract:	open - open the LA50 printer for graphics output

    The open function is responsible for initializing the variables
    for the LA50 Plot package and for sending escape sequences
    to put the printer in graphics mode.

 Author:	Kevin J. Dunlap

 Creation:	March 1984

 Modified by:

 Date:		   By:		Reason:

October 29, 1984   KJD 	        Port from PRO/V7M to ULTRIX-11

*/

#include	"la50.h"


int xnow,ynow;			/* the x and y coordinates of the */

int currx, curry;
int dminx=9999, dminy=9999, dmaxx=0, dmaxy=0;
int x0=0;
int y0=MAXY-1; 


char bits[] = { 
	1, 2, 4, 8, 16, 32 } 
;/* Bits we turn on when setting sixels */


float boty;		/* the device coordinate of the bottom y */
float botx;		/* the device coordinate of the bottom x */
float obotx;		/* the x and y coordinates of the bottom */
float oboty;		/* left corner of the plotting area */
float scalex;		/* the scaling factor for x coordinates */
float scaley;		/* the scaling factor for y coordinates */

openpl()
{
	int tmpx, tmpy;

	boty =  0.;
	botx =  0.;
	obotx = 0.;
	oboty = 0.;
	scalex = 1.;
	scaley = 1.;
	printf("\33[5i");		/* turn on printer */
	printf("\33P2q");		/* turn on graphics mode on la50 */
	for(tmpx=0; tmpx < MAXX ; tmpx++) { /* Vaccum mamory, Zero the bits */
		for(tmpy=0; tmpy < MAXY/6; tmpy++) 
			bitmap[tmpx][tmpy] =0;
	}
}

