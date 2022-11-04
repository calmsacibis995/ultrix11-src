
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)putc_.c	3.0	4/22/86
char id_putc[] = "(2.9BSD)  putc_.c  1.3";
 *
 * write a character to the standard output
 *
 * calling sequence:
 *	integer putc
 *	ierror =  putc (char)
 * where:
 *	char will be sent to the standard output, usually the terminal
 *	ierror will be 0 if successful; a system error code otherwise.
 */

#include	"../libI77/fiodefs.h"

extern unit units[];	/* logical units table from iolib */

ftnint putc_(c, clen)
char *c; ftnlen clen;
{
	int	i;
	unit	*lu;

	lu = &units[STDOUT];
	if (!lu->ufd)
		return((ftnint)(errno=F_ERNOPEN));
	if (!lu->uwrt && ! nowwriting(lu))
		return((ftnint)errno);
	putc (*c, lu->ufd);
	if (ferror(lu->ufd))
	{
		i = errno;
		clearerr(lu->ufd);
		return((ftnint)i);
	}
	return((ftnint) 0);
}
