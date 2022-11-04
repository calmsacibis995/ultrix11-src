
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*	SCCSID: @(#)matherr.c	3.0	4/22/86	*/
/*	(System 5)  matherr.c	1.2	*/
/*LINTLIBRARY*/

#include <math.h>

/*ARGSUSED*/
int
matherr(x)
struct exception *x;
{
	return (0);
}
