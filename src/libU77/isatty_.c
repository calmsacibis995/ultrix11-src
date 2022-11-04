
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)isatty_.c	3.0	4/22/86
char id_isatty[] = "(2.9BSD)  isatty_.c  1.1";
 *
 * determine if stream is associated with a tty (async port)
 *
 * calling sequence:
 *	logical	isatty, val
 *	val = isatty (lunit)
 * where:
 *	val will be .TRUE. if lunit is associated with a 'tty'
 */

#include	"../libI77/fiodefs.h"

extern unit units[];	/* logical units table from iolib */

ftnint isatty_(u)
ftnint *u;
{
	unit	*lu;

	if (*u < 0 || *u >= MXUNIT)
		return((ftnint) 0);
	lu = &units[*u];
	if (!lu->ufd)
		return((ftnint) 0);
	return((ftnint)(isatty(fileno(lu->ufd)) != 0));
}
