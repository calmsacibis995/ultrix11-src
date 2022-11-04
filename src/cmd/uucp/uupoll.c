
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)uupoll.c	3.0	4/22/86";

/*
 * Poll named system(s).
 *
 * The poll occurs even if recent attempts have failed,
 * but not if L.sys prohibits the call (e.g. wrong time of day).
 *
 * AUTHOR
 *	Tom Truscott (rti!trt)
 */

#include "uucp.h"

main(argc, argv)
int argc;
char **argv;
{
	chdir(Spool);
	strcpy(Progname, "uupoll");
	uucpname(Myname);
	if (argc < 2) {
		fprintf(stderr, "usage: uupoll system ...\n");
		exit(1);
	}

	for (--argc, ++argv; argc > 0; --argc, ++argv) {
		if (strcmp(argv[0], Myname) == SAME) {
			fprintf(stderr, "This *is* %s!\n", Myname);
			continue;
		}
		if (versys(argv[0])) {
			fprintf(stderr, "%s: unknown system.\n", argv[0]);
			continue;
		}
		/* Remove any STST file that might stop the poll */
		rmstat(argv[0]);
		/* Attempt the call */
		xuucico(argv[0]);
	}
	exit(0);
}

cleanup(code)
int code;
{
	exit(code);
}
