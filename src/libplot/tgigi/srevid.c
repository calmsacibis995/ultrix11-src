
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)srevid.c	3.0	4/22/86
 */
#include <stdio.h>
extern ggi;

srevid()	/* set reverse video */
{
	fprintf(ggi, "s(n1)\n");
}
