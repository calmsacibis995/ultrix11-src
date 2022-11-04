
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)symlnk_.c	3.0	4/22/86
char id_symlnk[] = "(2.9BSD)  symlnk_.c  1.1";
 *
 * make a symbolic link to a file
 *
 * calling sequence:
 *	ierror = symlnk(name1, name2)
 * where:
 *	name1 is the pathname of an existing file
 *	name2 is a pathname that will become a symbolic link to name1
 *	ierror will be 0 if successful; a system error code otherwise.
 */

#include	<sys/param.h>
#include	"../libI77/fiodefs.h"

#ifndef	MAXPATHLEN
#define	MAXPATHLEN	128
#endif

ftnint symlnk_(name1, name2, n1len, n2len)
char *name1, *name2;
ftnlen n1len, n2len;
{
	char buf1[MAXPATHLEN];
	char buf2[MAXPATHLEN];

	if (n1len >= sizeof buf1 || n2len >= sizeof buf2)
		return((ftnint)(errno=F_ERARG));
	g_char(name1, n1len, buf1);
	g_char(name2, n2len, buf2);
	if (buf1[0] == '\0' || buf2[0] == '\0')
		return((ftnint)(errno=F_ERARG));
	if (symlink(buf1, buf2) != 0)
		return((ftnint)errno);
	return((ftnint) 0);
}
