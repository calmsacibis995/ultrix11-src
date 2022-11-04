
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)slimit.c	3.0	4/22/86
 */
#include <stdio.h>
extern ggi;

slimit(x, y, x1, y1)	/* set screen coordinates */
{
		/* x is x_top, y is y_top, x1 is x_bottom, y1 is y_bottom */
		/* default to max gigi limits for now */

	fprintf(ggi, "s(a[0,0][767,479]\n");
}
