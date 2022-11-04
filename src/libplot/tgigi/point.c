
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)point.c	3.0	4/22/86
 */
#include <stdio.h>
extern ggi;

point(xi,yi)
{
	move(xi, yi);
	fprintf(ggi, "v[]\n");
}
