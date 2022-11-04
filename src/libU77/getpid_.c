
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)getpid_.c	3.0	4/22/86
char id_getpid[] = "(2.9BSD)  getpid_.c  1.1";
 *
 * get process id
 *
 * calling sequence:
 *	integer getpid, pid
 *	pid = getpid()
 * where:
 *	pid will be the current process id
 */

#include	"../libI77/fiodefs.h"

ftnint getpid_()
{
	return((ftnint)getpid());
}
