
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)wait_.c	3.0	4/22/86
char id_wait[] = "(2.9BSD)  wait_.c  1.1";
 *
 * wait for a child to die
 *
 * calling sequence:
 *	integer wait, status, chilid
 *	chilid = wait(status)
 * where:
 *	chilid will be	- >0 if child process id
 *			- <0 if (negative of) system error code
 *	status will contain the exit status of the child
 *		(see wait(2))
 */

#include	"../libI77/fiodefs.h"

extern int errno;

ftnint wait_(status)
ftnint *status;
{
	int stat;
	int chid = wait(&stat);
	if (chid < 0)
		return((ftnint)(-errno));
	*status = (ftnint)stat;
	return((ftnint)chid);
}
