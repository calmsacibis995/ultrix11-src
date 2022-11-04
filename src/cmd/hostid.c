
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)hostid.c	3.0	4/21/86";
/*
 * Based on:
 * 	"@(#)hostid.c	4.2 (Berkeley) 8/11/83";
 */

main(argc, argv)
int argc;
char **argv;
{
	long gethostid();

	if (argc > 1) {
		long hostid;
		sscanf(argv[1], "%lx", &hostid);
		if (sethostid(hostid) < 0) {
			perror("hostid");
			exit(1);
		}
	} else
		printf("%lx\n", gethostid());
	exit(0);
}
