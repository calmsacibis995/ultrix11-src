
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)mkspool.c	3.0	4/22/86";

/*********************************
 * mkspool.c
 *
 *	Program to make all spool subdirectories for 
 *	the specified systems.
 *
 *					
 *********************************/

#include "uucp.h"

main(argc,argv)
char **argv; int argc;
{
int orig_uid = getuid();
char tempname[NAMESIZE];

	setgid(getegid());
	setuid(geteuid());

	uucpname(tempname); /* init. subdir stuff */
	while(argc>1) {
		argc--; 
		mkspooldirs(*++argv);
		printf("made spool directories for: %s\n", *argv);
	}
}

cleanup(code)
int code; 
{
	exit(code);
}


