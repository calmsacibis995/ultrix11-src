
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#ifndef lint
static char sccsid[] = "@(#)log.c	3.0	4/22/86";
#endif

#include "tip.h"

static	FILE *flog = NULL;

/*
 * Log file maintenance routines
 */

logent(group, num, acu, message)
	char *group, *num, *acu, *message;
{
	char *user, *timestamp;
	struct passwd *pwd;
	long t;
#ifndef	LOCK_EX
	char buffer[BUFSIZ];
#endif	!LOCK_EX

	if (flog == NULL)
		return;
#ifdef	LOCK_EX
	if (flock(fileno(flog), LOCK_EX) < 0) {
		perror("tip: flock");
		return;
	}
#endif	LOCK_EX
	if ((user = getlogin()) == NOSTR)
		if ((pwd = getpwuid(getuid())) == NOPWD)
			user = "???";
		else
			user = pwd->pw_name;
	t = time(0);
	timestamp = ctime(&t);
	timestamp[24] = '\0';
#ifdef	LOCK_EX
	fprintf(flog, "%s (%s) <%s, %s, %s> %s\n",
#else
	sprintf(buffer, "%s (%s) <%s, %s, %s> %s\n",
#endif
		user, timestamp, group,
# ifdef PRISTINE
		"",
# else
		num,
# endif
		acu, message);
#ifdef	LOCK_EX
	fflush(flog);
	flock(fileno(flog), LOCK_UN);
#else
	/* make sure it is empty, then write it out and flush the buffer */
	/* this way, all the data should go out in one write and we know */
	/* that the single write will be contiguous. Not the cleanest, but */
	/* it should work for most cases. */
	fflush(flog);
	fwrite(buffer, sizeof(char), strlen(buffer), flog);
	fflush(flog);
#endif	LOCK_EX
}

loginit()
{

#ifdef ACULOG
	flog = fopen(value(LOG), "a");
	if (flog == NULL)
		fprintf(stderr, "can't open log file\r\n");
#endif
}
