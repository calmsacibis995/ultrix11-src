
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)zaptty.c	3.0	4/22/86";

#include <stdio.h>

main(argc, argv)
int argc;
char **argv;
{
	char	buf[1024];
	if (--argc <= 0) {
		fprintf(stderr,"Usage: zaptty command\n");
		exit(1);
	}
	if (zaptty() == -1) {
		fprintf(stderr, "zaptty: not super-user\n");
		exit(1);
	}
	switch(fork()) {
	case -1:
		perror("fork");
		exit(1);
	case 0:
		break;
	default:
		exit(0);
	}
	++argv;
	/*
	 * save *argv, since it could get modified if we are
	 * executing a shell script and the shell can't be
	 * executed.
	 */
	strncpy(buf, *argv, 1024);
	execvp(*argv, argv);
	perror(buf);
	exit(1);
}
