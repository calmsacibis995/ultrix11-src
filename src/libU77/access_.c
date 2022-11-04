
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)access_.c	3.0	4/22/86
char id_access[] = "(2.9BSD)  access_.c  1.3";
 *
 * determine accessability of a file
 *
 * calling format:
 *	integer access
 *	ierror = access(filename, mode)
 * where:
 *	ierror will be 0 for successful access; an error number otherwise.
 *	filename is a character string
 *	mode is a character string which may include any combination of
 *	'r', 'w', 'x', ' '. (' ' => test for existence)
 */

#include "../libI77/fiodefs.h"
#include <sys/param.h>
#ifndef	MAXPATHLEN
#define MAXPATHLEN	128
#endif

ftnint access_(name, mode, namlen, modlen)
char *name, *mode;
ftnlen namlen, modlen;
{
	char buf[MAXPATHLEN];
	int m = 0;

	if (namlen >= sizeof buf)
		return((ftnint)(errno=F_ERARG));
	g_char(name, namlen, buf);
	if (buf[0] == '\0')
		return((ftnint)(errno=ENOENT));
	if (access(buf, 0) < 0)
		return((ftnint)errno);
	while (modlen--) switch(*mode++)
	{
		case 'x':
			m |= 1;
			break;

		case 'w':
			m |= 2;
			break;

		case 'r':
			m |= 4;
			break;
	}
	if (m > 0 && access(buf, m) < 0)
		return((ftnint)errno);
	return((ftnint) 0);
}
