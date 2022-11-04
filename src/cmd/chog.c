
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/


/*
 * Chog: CHange Owner and Group
 * chog user file ...
 */

static char Sccsid[] = "@(#)chog.c	3.0	4/21/86";

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <pwd.h>


main(argc, argv)
int	argc;
char	*argv[];
{
	int	uid, gid;
	int	status = 0;
	struct	passwd	*pwd, *getpwnam(), *getpwuid();

	if(argc < 3) {
		printf("usage: chog user file ...\n");
		exit(1);
	}
	--argc; ++argv;
	if (isnumber(*argv)) {
		uid = atoi(*argv);
		pwd = getpwuid(uid);
	} else
		pwd = getpwnam(*argv);
	if (pwd == NULL) {
		printf("unknown user id: %s\n", *argv);
		exit(1);
	}
	uid = pwd->pw_uid;
	gid = pwd->pw_gid;

	while (++argv,--argc) {
		if(chown(*argv, uid, gid) < 0) {
			perror(*argv);
			status = 1;
		}
	}
	exit(status);
}

isnumber(s)
register char *s;
{
	while (isdigit(*s))
		s++;
	return((*s == '\0') ? 1 : 0);
}
