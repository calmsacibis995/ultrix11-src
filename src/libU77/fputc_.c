
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)fputc_.c	3.0	4/22/86
char id_fputc[] = (2.9BSD)  fputc_.c  1.4";
 *
 * write a character to a logical unit bypassing formatted I/O
 *
 * calling sequence:
 *	integer fputc
 *	ierror = fputc (unit, char)
 * where:
 *	char will be sent to the logical unit
 *	ierror will be 0 if successful; a system error code otherwise.
 */

#include	"../libI77/fiodefs.h"

extern unit units[];	/* logical units table from iolib */

ftnint fputc_(u, c, clen)
ftnint *u; char *c; ftnlen clen;
{
	int	i;
	unit	*lu;

	if (*u < 0 || *u >= MXUNIT)
		return((ftnint)(errno=F_ERUNIT));
	lu = &units[*u];
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
