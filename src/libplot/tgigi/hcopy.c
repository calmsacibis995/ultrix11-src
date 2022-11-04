
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)hcopy.c	3.0	4/22/86
 */
#include <stdio.h>
extern ggi;

hcopy(y, y1)	/* hardcopy out from line y to line y1 */
{
	int ny,ny1;

	ny = ysc(y);
	ny1 = ysc(y1);
	fprintf(ggi, "s(h[,%d][,%d]\n", ny, ny1);
}
