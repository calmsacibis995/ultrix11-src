
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * chgrp gid file ...
 */

static char Sccsid[] = "@(#)chgrp.c 3.0 4/21/86";
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <grp.h>

struct	group	*gr,*getgrnam();
struct	stat	stbuf;
int	gid;
int	status;

main(argc, argv)
char *argv[];
{
	register c;

	if(argc < 3) {
		printf("usage: chgrp gid file ...\n");
		exit(4);
	}
	if(isnumber(argv[1])) {
		gid = atoi(argv[1]);
	} else {
		if((gr=getgrnam(argv[1])) == NULL) {
			printf("unknown group: %s\n",argv[1]);
			exit(4);
		}
		gid = gr->gr_gid;
	}
	for(c=2; c<argc; c++) {
		stat(argv[c], &stbuf);
		if(chown(argv[c], stbuf.st_uid, gid) < 0) {
			perror(argv[c]);	

			/* SYSTEM V compatibility */
			/* printf("%s: permission denied, must be super-user\n",
				argv[c]); */
			status = 1;
		}
	}
	exit(status);
}

isnumber(s)
char *s;
{
	register c;

	while(c = *s++)
		if(!isdigit(c))
			return(0);
	return(1);
}
