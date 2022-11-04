/*
 * SCCSID: @(#)la50.h	3.0	4/22/86 
 */

/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*

 Facility:	Part of the plot package

 Called by:	driver()

 Calls: 	nothing

 Abstract:	Struture definitions for LA50 Plot Library

 Author:	Kevin J. Dunlap

 Creation:	March 1984

 Modified by:

 Date:		     By:	Reason:

 October 19, 1984    KJD       Port from Pro/V7m to ULTRIX-32 

*/

#include <stdio.h>
#include <signal.h>

#define MAXX 492
#define MAXY 492

extern int currx, curry, countp ;
extern int dminx, dminy, dmaxx, dmaxy;
extern int x0;
extern int y0; 
extern float botx;    /* The device coordinate of the bottom x */
extern float boty;    /* The device coordinate of the bottom y */
extern float obotx;   /* The x and y coordinate of the bottom */
extern float oboty;   /* left corner of the ploting area */
extern float scalex;  /* the scaleing factor for x */
extern float scaley;  /* the scaleing factor of y */


extern char	bits[];	/* Bits we turn on when setting sixels */

char	bitmap[492][82];   /* The bitmap */

