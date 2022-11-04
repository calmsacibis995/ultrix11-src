
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)kill_.c	3.0	4/22/86
char id_kill[] = "(2.9BSD)  kill_.c  1.1";
 *
 * send a signal to a process
 *
 * calling sequence:
 *	ierror = kill(pid, signum)
 * where:
 *	pid must be the process id of one of the user's processes
 *	signum must be a valid signal number (see signal(2))
 *	ierror will be 0 if successful; an error code otherwise.
 */

#include "../libI77/fiodefs.h"

ftnint kill_(pid, signum)
ftnint *pid, *signum;
{
	if (*pid < 0 || *pid > (ftnint) 32767 || *signum < 1 || *signum > 16)
		return((ftnint)(errno=F_ERARG));
	if (kill((int)*pid, (int)*signum) != 0)
		return((ftnint)errno);
	return((ftnint) 0);
}
