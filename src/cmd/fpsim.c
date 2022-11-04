
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)fpsim.c	3.0	4/21/86";
#include <stdio.h>
#include <errno.h>
extern int errno;
#define	TURNOFF	0
#define	TURNON	1
#define	GETSTAT 2

main(argc, argv)
int	argc;
char	**argv;
{
	int ostat, nstat, ostat2;
	switch (argc) {
	case 2:
		if (strcmp(argv[1], "on") == 0)
			ostat = fpsim(TURNON);
		else if (strcmp(argv[1], "off") == 0)
			ostat = fpsim(TURNOFF);
		else
			usage();
		if (ostat == -1) {
		    switch(errno) {
		    case EPERM:
			fprintf(stderr, "must be super-user to change status");
			break;
		    case ENODEV:
			fprintf(stderr, "fpsim not configured\n");
			break;
		    default:
			fprintf(stderr, "unknown error in changing status\n");
			break;
		    }
		    break;
		}
		/*FALL THROUGH*/
	case 1:
	report:
		switch(fpsim(GETSTAT)) {
		case 0:
			printf("disabled\n");
			break;
		case 1:
			printf("enabled\n");
			break;
		case 2:
			printf("not configured in\n");
			break;
		default:
			printf("can't get status\n");
			break;
		}
		break;
	default:
		usage();
	}
	exit(0);
}
usage()
{
	fprintf(stderr, "Usage: /etc/fpsim [on] [off]\n");
	exit(1);
}
