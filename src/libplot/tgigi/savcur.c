
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)savcur.c	3.0	4/22/86
 */
#include <stdio.h>
extern ggi;
extern xnow,ynow;

savcur()	/* save current cursor position on gigi's stack */
{		/* could be tricky. when to recover cursor? */

	fprintf(ggi, "s(b)\n");
}
