
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)alarm_.c	3.0	4/22/86
char id_alarm[] = "(2.9BSD)  alarm_.c  1.1";
 *
 * set an alarm time, arrange for user specified action, and return.
 *
 * calling sequence:
 *	integer	flag
 *	external alfunc
 *	lastiv = alarm (intval, alfunc)
 * where:
 *	intval	= the alarm interval in seconds; 0 turns off the alarm.
 *	alfunc	= the function to be called after the alarm interval,
 *
 *	The returned value will be the time remaining on the last alarm.
 */

#include	"../libI77/fiodefs.h"
#include	<signal.h>

ftnint alarm_(sec, proc)
ftnint	*sec;
int	(* proc)();
{
	register ftnint	lt;

	lt = (ftnint) alarm(1000);	/* time to maneuver */

	if (*sec)
		signal(SIGALRM, proc);

	alarm((unsigned) *sec);
	return(lt);
}
