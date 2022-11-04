static char Sccsid[] = "@(#)nuset.c	3.0	4/21/86";

/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * ULTRIX-11 nuset program
 *
 * Modifies machdep.o to include the proper magic number
 * for the specified number of user limit (UMAX).
 * Works on a copy of machdep.o in the current directory.
 *
 * Fred Canter
 */

#include <stdio.h>
#include <a.out.h>

#define	NU8	023041
#define	NU16	035056
#define	NU32	022045
#define	NU100	021473

struct nlist nl[] =
{
	{ "_icode" },
	{ "" },
};

int	nuloc;		/* address of UMAX in icode[] */
int	cnuval;		/* current UMAX value from icode[] */
int	nuval;		/* new UMAX value from "machdep.o" */

main(argc, argv)
int	argc;
char	*argv[];
{
	FILE *fi;
	unsigned int magic;
	char	*p;
	int fd, i;

	if(argc != 2) {
		printf("\nnuset: bad arg count !\n");
		exit(1);
	}
	fi = fopen("machdep.o", "r");
	if(fi == NULL) {
		printf("\nnuset: Can't open %s !\n", "machdep.o");
		exit(1);
	}
	magic = getw(fi);
	if(magic != 0407) {
		printf("\n%s - (%o) bad magic number\n","machdep.o",magic);
		exit(1);
	}
	close(fi);
	if(nlist("machdep.o", nl) < 0) {
		printf("\nnuset: %s - no name list\n", "machdep.o");
		exit(1);
	}
	if((nl[0].n_type == 0) || (nl[0].n_value == 0)) {
		printf("\nnuset: %s - nuser location missing\n", "machdep.o");
		exit(1);
	}
	nuval = 0;
	for(p=argv[1]; *p; p++) {
		nuval <<= 3;
		nuval |= (*p & 7);
	}
	switch(nuval) {
	case NU8:
	case NU16:
	case NU32:
		break;
	default:
		printf("\nnuset: bad UMAX value (%o)\n", nuval);
		exit(1);
	}
	fd = open("machdep.o", 2);
	if(fd < 0) {
		printf("\nnuset: Can't open %s for RW\n", "machdep.o");
		exit(1);
	}
	nuloc = 020 + nl[0].n_value + 24;	/* icode[12] */
	lseek(fd, (long)nuloc, 0);
	read(fd, (char *)&cnuval, sizeof(cnuval));
	switch(cnuval) {
	case NU8:
	case NU16:
	case NU32:
	case NU100:
		break;
	default:
		printf("\nnuset: machdep.o - invalid current UMAX value (%o)\n",
			cnuval);
		exit(1);
	}
	lseek(fd, (long)nuloc, 0);
	write(fd, (char *)&nuval, sizeof(nuval));
	close(fd);
	exit(0);
}
