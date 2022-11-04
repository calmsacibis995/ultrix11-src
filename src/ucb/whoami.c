
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)whoami.c	3.0	4/22/86";
#include <stdio.h>
#include <pwd.h>
struct passwd *getpwuid();

main()
{
	struct passwd *pp;
	int uid

	pp=getpwuid(uid = getuid());

	if (pp == NULL)
		printf("%d\n", uid);
	else
		printf("%s\n", pp->pw_name);
	exit(0);
}
