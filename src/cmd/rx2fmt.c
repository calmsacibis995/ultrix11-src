
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * ULTIRX-11 RX02 diskette format command (rx2fmt)
 *
 * Fred Canter 5/6/83
 *
 * Usage: /etc/rx2fmt [-s] unit#
 *
 */

static char Sccsid[] = "@(#)rx2fmt.c 3.0 4/22/86";
#include <sgtty.h>

#define FORMAT 010
#define SINGLE 0
#define DOUBLE 2

struct sgttyb sgttyb = FORMAT;

int	density = DOUBLE;
int	unit;

char	dn[] = "/dev/rhx?";

main(argc, argv)
int	argc;
char	*argv[];
{
	register int i, j;

	if(argc < 2 || argc > 3) {
	usage:
		printf("\nUsage: /etc/rx2fmt [-s] unit#\n");
		exit(1);
	}
	i = 1;
	if(argc == 3) {
		if((argv[1][0] != '-') || (argv[1][1] != 's'))
			goto usage;
		density = SINGLE;
		i = 2;
	}
	if((argv[i][0] > '1') || (argv[i][0] < '0'))
		goto usage;
	unit = argv[i][0] - '0';
	unit |= density;
	dn[8] = unit + '0';
	i = open(&dn, 2);
	if(i < 0) {
		printf("\nrx2fmt: Can't open %s\n", dn);
		exit(1);
	}
	j = ioctl(i, TIOCSETP, &sgttyb);
	if(j != 0) {
		printf("\nFormat of %s FAILED", dn);
		perror(" ");
		exit(1);
	}
	close(i);
}
