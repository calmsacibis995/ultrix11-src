
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)ierrno_.c	3.0	4/22/86
char id_ierrno[] = "(2.9BSD)  ierrno_.c  1.1";
 *
 * return the current value of the system error register
 *
 * calling sequence:
 *	ier = ierrno()
 * where:
 *	ier will receive the current value of errno
 */

#include	"../libI77/fiodefs.h"

extern int errno;

ftnint ierrno_()
{
	return((ftnint)errno);
}
