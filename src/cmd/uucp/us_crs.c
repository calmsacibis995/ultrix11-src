
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)us_crs.c	3.0	4/22/86";

 
/*
 * Whenever a command file (i.e. C.*****) file is spooled by uucp,
 * creates an entry in the beginning of "R_stat" file. 
 * Future expansion: An R_stat entry may be created by, e.g.
 * uux, rmail, or any command using uucp.
 * return:
 *	0	-> success
 *	FAIL	-> failure
 */


/***************
 * Mods:
 *	decvax!larry - store data in binary format 
 **************/

#include "uucp.h"
#ifdef UUSTAT
#include <sys/types.h>
#include "uust.h"


us_crs(cfile)
char *cfile;
{
	register FILE *fq;
	register short i;
	char *name, *s, buf[BUFSIZ];
	struct us_rsf u;
	long time();
 
	DEBUG(6, "Enter us_crs, cfile: %s\n", cfile);
	clear(&u,sizeof(u));
	if ((fq = fopen(R_stat, "a+")) == NULL) {
		DEBUG(3, "fopen of %s failed\n", s);
		return(FAIL);
	}

	/*
	 * manufacture a new entery
	 */
	name = cfile + strlen(cfile) - 4;
	strncpy(u.jobn, name, 4);
	u.jobn[4] = '\0';
	u.qtime = u.stime = time((long *) 0);
	u.ustat = USR_QUEUED;
	strncpy(u.user, User, NAME7);
	u.user[NAME7-1] = '\0';
	strncpy(u.rmt, Rmtname, 7);
	u.user[6] = '\0';
	fwrite(&u, sizeof(u), 1, fq);
	fflush(fq);
	fclose(fq);
	return(0);
}
clear(p, c)
register char *p;
register int c;
{
	register i;

	for(i=0;i<c;i++)
		*p++ = 0;
}
#endif
