
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)fgetc.c	3.0	4/22/86
 */
#include <stdio.h>

fgetc(fp)
FILE *fp;
{
	return(getc(fp));
}