
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)chmod_.c	3.0	4/22/86
char	id_chmod[]	= "(2.9BSD)  chmod_.c  1.2";
 *
 * chmod - change file mode bits
 *
 * synopsis:
 *	integer function chmod (fname, mode)
 *	character*(*) fname, mode
 */

#include	"../libI77/fiodefs.h"
#include	<sys/param.h>
#ifndef	MAXPATHLEN
#define MAXPATHLEN	128
#endif

ftnint chmod_(name, mode, namlen, modlen)
char	*name, *mode;
ftnlen	namlen, modlen;
{
	char	nambuf[MAXPATHLEN];
	char	modbuf[32];
	int	retcode;

	if (namlen >= sizeof nambuf || modlen >= sizeof modbuf)
		return((ftnint)(errno=F_ERARG));
	g_char(name, namlen, nambuf);
	g_char(mode, modlen, modbuf);
	if (nambuf[0] == '\0')
		return((ftnint)(errno=ENOENT));
	if (modbuf[0] == '\0')
		return((ftnint)(errno=F_ERARG));
	if (fork())
	{
		if (wait(&retcode) == -1)
			return((ftnint)errno);
		return((ftnint)retcode);
	}
	else
		execl("/bin/chmod", "chmod", modbuf, nambuf, (char *)0);
}
