/*
 * SCCSID: @(#)la100.h	3.0	4/22/86
 */

/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*

 Facility:	Part of the plot package for the LA100 printer

 Called by:	driver()

 Abstract:	Struture definitions for LA100 Plot Library

 Author:	Kevin J. Dunlap

 Creation:	March 1984

*/

#include <stdio.h>
#include <signal.h>

#define MAXX 492	/* Maximum number of X points */
#define MAXY 492	/* Maximum number of Y points */

extern int currx, curry, countp ;
extern int dminx, dminy, dmaxx, dmaxy;
extern int x0;
extern int y0; 
extern int pt_cnt;
extern int x_cnt;
extern int y_cnt;
extern float botx; 
extern float boty;
extern float obotx;
extern float oboty;
extern float scalex;
extern float scaley;


extern char	bits[];	/* Bits we turn on when setting sixels */

char	bitmap[492][82];

