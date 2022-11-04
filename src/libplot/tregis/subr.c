/*
 * SCCSID: @(#)subr.c	3.0	4/22/86 
 */

/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*

  Facility:	 Part of the plot package for the REGIS terminals.

  Called by:	 driver()

  Abstract:  This contains all the subroutines for scaleing X & Y

  Author:	 Kevin J. Dunlap

  Creation:	April 1985 

*/

extern float obotx;
extern float oboty;
extern float boty;
extern float botx;
extern float scalex;
extern float scaley;

/* scale X */
xsc(xi){
	int xa;
	xa = (xi-obotx)*scalex+botx;
	return(xa);
}

/* scale Y */
ysc(yi){
	int ya;
/*  Note that the next line was changed from ya = (yi - oboty)*scaley +boty;
    This is because on the REGIS terminals, the TOP left cornor of the display
    is (0,0).  The BOTTOM of the display is (0,479), therfore we have to
    subtract.
*/
	ya = (yi - oboty)*scaley +boty;
	return(ya);
}
