
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)fork_.c	3.0	4/22/86
char id_fork[] = "(2.9BSD)  fork_.c  1.1";
 *
 * fork a copy of this process
 *
 * calling sequence:
 *	integer fork
 *	ierror = fork()
 * where:
 *	ierror will be	- child pid if parent and successful
 *			- 0 if child
 *			- -errno if unsuccessful
 */

#include	"../libI77/fiodefs.h"

extern int errno;

ftnint fork_()
{
	ftnint i;

	for (i = 0; i < MXUNIT; i++)
		flush_(&i);
	i = (ftnint)fork();
	if (i < 0)
		return((ftnint)(-errno));
	return(i);
}
