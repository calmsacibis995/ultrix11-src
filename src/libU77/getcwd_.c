
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)getcwd_.c	3.0	4/22/86
char id_getcwd[] = "(2.9BSD)  getcwd_.c  1.5";
 * Get pathname of current working directory.
 *
 * calling sequence:
 *	character*128 path
 *	ierr = getcwd(path)
 * where:
 *	path will receive the pathname of the current working directory.
 *	ierr will be 0 if successful, a system error code otherwise.
 */

#include	"../libI77/fiodefs.h"
#include	<sys/param.h>
#ifndef	MAXPATHLEN
#define MAXPATHLEN	128
#endif

extern int errno;
char	*getwd();

ftnint
getcwd_(path, len)
char *path;
ftnlen len;
{
	char	*p;
	char	pathname[MAXPATHLEN];

	p = getwd(pathname);
	b_char(pathname, path, len);
	if (p)
		return((ftnint) 0);
	else
		return((ftnint)errno);
}
