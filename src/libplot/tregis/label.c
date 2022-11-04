/*
 * SCCSID: @(#)label.c	3.0	4/22/86 
 */

/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

 /*

 Called by:	driver()

 Facility:	Part of the plot package for the REGIS terminals.

 Abstract:	label(s) - writes the string at the current point.

 Author:	Jerry Brenner

*/

#include <stdio.h>
extern ggi;
label(s)
char *s;
{
	fprintf(ggi,"W(R)\n");
	fprintf(ggi,"T\'%s\'\n", s);
}
