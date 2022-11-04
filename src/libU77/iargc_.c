
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)iargc_.c	3.0	4/22/86
char id_iargc[] = "(2.9BSD)  iargc_.c  1.1";
 *
 * return the number of args on the command line following the command name
 *
 * calling sequence:
 *	nargs = iargc()
 * where:
 *	nargs will be set to the number of args
 */

#include	"../libI77/fiodefs.h"

extern int xargc;

ftnint iargc_()
{
	return ((ftnint)(xargc - 1));
}
