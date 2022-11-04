
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)assert.c	3.0	4/22/86";

#include "uucp.h"
#include <time.h>
#include <sys/types.h>
#include <errno.h>

/*******
 *	assert - print out assetion error
 *	       - prints error message to screen if debugging turned on
 *	       - places error message in ERRLOG if debugging turned off
 *
 *	return code - none
 */

assert(s1, s2, i1)
char *s1, *s2;
{
	FILE *errlog;
	struct tm *tp;
	extern struct tm *localtime();
	extern time_t time();
	time_t clock;
	int pid;

	if (Debug)
		errlog = stderr;
	else {
		int savemask;
		savemask = umask(LOGMASK);
		errlog = fopen(ERRLOG, "a");
		umask(savemask);
	}
	if (errlog == NULL)
		return;

	pid = getpid();
	fprintf(errlog, "ASSERT ERROR (%.9s)  ", Progname);
	fprintf(errlog, "pid: %d  ", pid);
	time(&clock);
	tp = localtime(&clock);
	fprintf(errlog, "(%d/%d-%d:%02d) ", tp->tm_mon + 1,
		tp->tm_mday, tp->tm_hour, tp->tm_min);
	fprintf(errlog, "%s %s (%d)\n", s1, s2, i1);
	if (!Debug)
		fclose(errlog);
	return;
}
