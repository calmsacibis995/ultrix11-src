
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)movscr.c	3.0	4/22/86
 */
#include <stdio.h>
extern ggi;

movscr(x, y)		/* move screen origin location to x and y */
{
	fprintf(ggi, "s[%d,%d]\n", x, y);
}
