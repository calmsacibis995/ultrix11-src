
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)getlog_.c	3.0	4/22/86
char id_getlog[] = "(2.9BSD)  getlog_.c  1.1";
 *
 * get login name of user
 *
 * calling sequence:
 *	character*8 getlog, name
 *	name = getlog()
 * or
 *	call getlog (name)
 * where:
 *	name will receive the login name of the user, or all blanks if
 *	this is a detached process.
 */

#include	"../libI77/fiodefs.h"

char *getlogin();

getlog_(name, len)
char *name; ftnlen len;
{
	char *l = getlogin();

	b_char(l?l:" ", name, len);
}
