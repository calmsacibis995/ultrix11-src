
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * Hacked up version of Perror. This avoids all the data that
 * perror normally has, thus saving us a couple of K of data space.
 */
 
static char *sccsid = "@(#)perror.c	3.0	(ULTRIX-11)	4/22/86";

#include <stdio.h>
int	errno;
char * perror(s)
char *s;
{
	register char *c;
	register n;
	static char buf[20];

	sprintf(buf, "Error code %d", errno);
	if (s == NULL)
		return(buf);
	if (*s)
		fprintf(stderr, "%s: ", s);
	fprintf(stderr, "%s\n", buf);
}
