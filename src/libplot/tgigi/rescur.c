
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)rescur.c	3.0	4/22/86
 */
#include <stdio.h>
extern ggi;
extern xnow,ynow;

rescur()	/* restore cursor position from gigi's stack */
{
	fprintf(ggi, "s(e)\n");
}
