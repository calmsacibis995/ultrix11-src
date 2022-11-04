/*
 * SCCSID: @(#)open.c	3.0	4/22/86
 */

/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*

 Facility:	Part of the plot package for REGIS terminals

 Called by:	driver()

 Calls: 	wrtng()

 Abstract:	open - open the terminal for REGIS graphics output

    The open function is responsible for initializing the variables
    for the REGIS Plot package and for sending the escape sequence
    to put the terminal in REGIS graphics mode.

 Author:	Kevin J. Dunlap 

 Creation:	March 1985

*/

#include <stdio.h>
FILE	*ggi;
int xnow;
int ynow;
int multi;

float boty;	/* the device coordinate of the bottom y */
float botx;	/* the device coordinate of the bottom x */
float oboty;	/* The x and Y corrdinates for the */
float obotx;	/* left corner of the plotting area */	
float scalex;	/* the scaling factor for x coordinates */
float scaley;	/* the real scaling factor for y coordinates */

openpl()
{
/* make sure open will restore the original scale */
/*	boty = 479.; */
/*	botx = 767.; */
	boty = 0.;
	botx = 0.;
	oboty = 0.;
	obotx = 0.;
	scalex = 1.;
/*	scaley = 1.601; */
	scaley = 1.;
	ggi = stdout;
 /* Turn on REGIS Graphics mode */
	fprintf(ggi,"\033Pp\n");
	wrtng("int,white noblink noshade nonegat pat,1 replace multi,1 ");
/* set Orgin to bottom left cornor */
	fprintf(ggi,"S(A[0,479][767,0])\n"); 
	return;
}
