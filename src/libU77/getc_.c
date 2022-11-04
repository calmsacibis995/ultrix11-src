
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)getc_.c	3.0	4/22/86
char id_getc[] = "(2.9BSD)  getc_.c  1.3";
 *
 * get a character from the standard input
 *
 * calling sequence:
 *	integer getc
 *	ierror = getc (char)
 * where:
 *	char will be read from the standard input, usually the terminal
 *	ierror will be 0 if successful; a system error code otherwise.
 */

#include	"../libI77/fiodefs.h"

extern unit units[];	/* logical units table from iolib */

ftnint getc_(c, clen)
char *c; ftnlen clen;
{
	int	i;
	unit	*lu;

	lu = &units[STDIN];
	if (!lu->ufd)
		return((ftnint)(errno=F_ERNOPEN));
	if (lu->uwrt && ! nowreading(lu))
		return((ftnint)errno);
	if ((i = getc (lu->ufd)) < 0)
	{
		if (feof(lu->ufd))
			return((ftnint) -1);
		i = errno;
		clearerr(lu->ufd);
		return((ftnint)i);
	}
	*c = i & 0177;
	return((ftnint) 0);
}
