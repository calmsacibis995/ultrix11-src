
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)hostnm_.c	3.0	4/22/86
 * hostnm - return this machines hostname
 *	(2.9BSD)  hostnm_.c  1.1
 *
 * synopsis:
 *	integer function hostnm (name)
 *	character(*) name
 *
 * where:
 *	name	will receive the host name
 *	The returned value will be 0 if successful, an error number otherwise.
 */

#include	"../libI77/fiodefs.h"

extern int	errno;

ftnint
hostnm_ (name, len)
char	*name;
ftnlen	len;
{
	char	buf[64];
	register char	*bp;
	int	blen	= sizeof buf;

	if (gethostname (buf, blen) == 0)
	{
		b_char (buf, name, len);
		return ((ftnint) 0);
	}
	else
		return((ftnint)errno);
}
