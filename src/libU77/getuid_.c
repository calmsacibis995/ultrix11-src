
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)getuid_.c	3.0	4/22/86
char id_getuid[] = "(2.9BSD)  getuid_.c  1.1";
 *
 * get user id
 *
 * calling sequence:
 *	integer getuid, uid
 *	uid = getuid()
 * where:
 *	uid will be the real user id
 */

#include	"../libI77/fiodefs.h"

ftnint getuid_()
{
	return((ftnint)getuid());
}
