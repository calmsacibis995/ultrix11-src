
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)gmtime_.c	3.0	4/22/86
char id_gmtime[] = "(2.9BSD)  gmtime_.c  1.1";
 *
 * return broken down time
 *
 * calling sequence:
 *	integer time, t[9]
 *	call gmtime(time, t)
 * where:
 *	time is a system time. (see time(3F))
 *	t will receive the broken down time assuming GMT.
 *	(see ctime(3))
 */

#include	<time.h>
#include	<sys/types.h>

struct tm *gmtime();

gmtime_(clock, t)
time_t *clock; struct tm *t;
{
	struct tm *g;

	g = gmtime(clock);
	*t = *g;
}
