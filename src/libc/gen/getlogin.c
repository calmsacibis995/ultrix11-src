
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)getlogin.c	3.0	4/22/86
 */
#include <stdio.h>	/* for NULL */
#include <utmp.h>

static	char	UTMP[]	= "/etc/utmp";
static	struct	utmp ubuf;

char *
getlogin()
{
	register me, uf;
	register char *cp;

/*
 * ttyslot for ULTRIX-11 returns 0 on error; from System V,
 * it returns -1 on any error.  Since a valid ttyslot number
 * cannot be 0, we just check here for <= 0, which satisfies
 * both cases (user is running System V environment or not.)
 * 	-jsd
 */
	if( (me = ttyslot()) <= 0 )
		return(NULL);
	if( (uf = open( UTMP, 0 )) < 0 )
		return(NULL);
	lseek( uf, (long)(me*sizeof(ubuf)), 0 );
	if (read(uf, (char *)&ubuf, sizeof(ubuf)) != sizeof(ubuf))
		return(NULL);
	close(uf);
	cp = ubuf.ut_name;
	if (*cp == '\0' || *cp == ' ')	/* make sure there's something there */
		return(NULL);
	for (cp[8] = ' '; *cp++!=' ';)
		;
	*--cp = '\0';
	return( ubuf.ut_name );
}
