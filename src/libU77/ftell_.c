
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)ftell_.c	3.0	4/22/86
char id_ftell[] = "(2.9BSD)  ftell_.c  1.2";
 *
 * return current file position
 *
 * calling sequence:
 *	integer curpos, ftell
 *	curpos = ftell(lunit)
 * where:
 *	lunit is an open logical unit
 *	curpos will be the current offset in bytes from the start of the
 *		file associated with that logical unit
 *		or a (negative) system error code.
 */

#include	"../libI77/fiodefs.h"
#include	<sys/types.h>

extern unit units[];

off_t ftell_(lu)
ftnint *lu;
{
	if (*lu < 0 || *lu >= MXUNIT)
		return(-(off_t)(errno=F_ERUNIT));
	if (!units[*lu].ufd)
		return(-(off_t)(errno=F_ERNOPEN));
	return((off_t) ftell(units[*lu].ufd));
}
