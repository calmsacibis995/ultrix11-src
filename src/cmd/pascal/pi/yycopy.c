
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*	SCCSID: @(#)yycopy.c	3.0	4/22/86	*/
/* Copyright (c) 1979 Regents of the University of California */
#include	"0.h"
#include	"yy.h"

OYcopy ()
    {
	register int	*r0 = & OY;
	register int	*r1 = & Y;
	register int	r2 = ( sizeof ( struct yytok ) ) / ( sizeof ( int ) );

	do
	    {
		* r0 ++ = * r1 ++ ;
	    }
	    while ( -- r2 > 0 );
    }



