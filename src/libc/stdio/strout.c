
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)strout.c	3.0	4/22/86
 */
/*	(System 5)  strout.c	1.2	*/
/*LINTLIBRARY*/

#include <stdio.h>

void
_strout(string, count, adjust, file)
register char *string;
register int adjust, count;
register FILE *file;
{
	while(adjust > 0) {
		(void) putc(' ', file);
		--adjust;
	}
	while(--count >= 0)
		(void) putc(*string++, file);
	while(adjust < 0) {
		(void) putc(' ', file);
		++adjust;
	}
}
