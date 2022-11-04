
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)unlink_.c	3.0	4/22/86
char id_unlink[] = "(2.9BSD)  unlink_.c  1.2";
 *
 * unlink (remove) a file
 *
 * calling sequence:
 *	integer unlink
 *	ierror = unlink(filename)
 * where:
 *	ierror will be a returned status (0 == OK)
 *	filename is the file to be unlinked
 */

#include	"../libI77/fiodefs.h"
#include	<sys/param.h>
#ifndef	MAXPATHLEN
#define MAXPATHLEN	128
#endif

ftnint
unlink_(fname, namlen)
char *fname;
ftnlen namlen;
{
	char buf[MAXPATHLEN];

	if (namlen >= sizeof buf)
		return((ftnint)(errno=F_ERARG));
	g_char(fname, namlen, buf);
	if (unlink(buf) != 0)
		return((ftnint)errno);
	return((ftnint) 0);
}
