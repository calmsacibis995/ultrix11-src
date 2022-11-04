
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)loc_.c	3.0	4/22/86
char id_loc[] = "(2.9BSD)  loc_.c  1.1";
 *
 * Return the address of the argument.
 *
 * calling sequence:
 *	iloc = loc (arg)
 * where:
 *	iloc will receive the address of arg
 */

#include	"../libI77/fiodefs.h"

ftnint loc_(arg)
ftnint *arg;
{
	return((ftnint)arg);
}
