/*
 * SCCSID: @(#)space.c	3.0	4/22/86
 */

/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*

 Facility:	Part of the Plot package for the LA100 printer.

 Called by:	driver()


 Modified by:

 Date:		By:		Reason:

 Feb-1984	Kevin J. Dunlap change deltx,delty, botx and boty constants.

 Mar-1984	David Roberts	Add debug output.

*/

#include "la100.h"

space(x1,y1,x2,y2){
	float deltx = MAXX;
	float delty = MAXY;
#if DEBUG
	fprintf(stderr,"Space called with %d %d %d %d\n",x1,y1,x2,y2);
	fprintf(stderr,deltx is %f, delty is %f \n",deltx,delty);
#endif
	botx = 0.;
	boty = 0.;
	obotx = x1;
	oboty = y1;
	scalex = deltx/(x2-x1);
	scaley = delty/(y2-y1);
#if DEBUG
	fprintf(stderr,"Space: scalex set to %f, scaley set to %f\n",
			scalex,scaley);
#endif
}
