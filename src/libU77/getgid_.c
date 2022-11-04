
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)getgid_.c	3.0	4/22/86
char id_getgid[] = "(2.9BSD)  getgid_.c  1.1";
 *
 * get group id
 *
 * calling sequence:
 *	integer getgid, gid
 *	gid = getgid()
 * where:
 *	gid will be the real group id
 */

#include	"../libI77/fiodefs.h"

ftnint getgid_()
{
	return((ftnint)getgid());
}
