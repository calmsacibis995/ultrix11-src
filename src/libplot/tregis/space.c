/*
 * SCCSID: @(#)space.c	3.0	4/22/86
 */

/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*

 Facility:	Part of the Plot package for the LA50 printer.

 Called by:	driver()

 Abstract:	space(x0,y0,x1,y1) set a new plotting area.

 Modified by:

 Date:		By:		Reason:

 Feb-1984	Kevin J. Dunlap change deltx,delty, botx and boty constants.

 Mar-1984	David Roberts	Add debug output.

*/

extern float boty;
extern float botx;
extern float oboty;
extern float obotx;
extern float scalex;
extern float scaley;
space(x0,y0,x1,y1){
float delta 479.;
/* float deltx 767.;
float delty 479.; */
#if DEBUG
	fprintf(stderr,"Space called with %d %d %d %d\n",x0,y0,x1,y1);
	fprintf(stderr,deltx is %f, delty is %f \n",deltx,delty);
#endif
	botx = 0.;
	boty = 0.;
	obotx = x0;
	oboty = y0;
	scalex = delta/(x1-x0);
	scaley = delta/(y1-y0);
/*	scalex = deltx/(x1-x0);
	scaley = delty/(y1-y0); */
#if DEBUG
	fprintf(stderr,"Space: scalex set to %f, scaley set to %f\n",
			scalex,scaley);
#endif
}

