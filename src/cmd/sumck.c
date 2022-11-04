
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * ULTRIX-11 sumcheck generator
 *
 * Generates a 16 bit checksum on the specified files
 * and stores it in the file at location `sumcheck'.
 * The file must have `unsigned sumcheck = 0;'
 * The file must be in the current directory.
 *
 * Fred Canter
 */

#include <stdio.h>
#include <a.out.h>

static char Sccsid[] = "@(#)sumck.c	3.0	4/22/86";

struct nlist nl[] =
{
	{ "_sumchec" },
	{ "" },
};

unsigned	sumck;
unsigned	scc;
unsigned	scloc;

main(argc, argv)
int	argc;
char	*argv[];
{
	FILE *fi;
	unsigned int magic, txtsiz, datsiz, bsssiz, symsiz;
	int fd, i;

	if(argc != 2) {
		printf("\nsumck: bad arg count !\n");
		exit(1);
	}
	fi = fopen(argv[1], "r");
	if(fi == NULL) {
		printf("\nsumck: Can't open %s !\n", argv[1]);
		exit(1);
	}
	magic = getw(fi);
	txtsiz = getw(fi);
	datsiz = getw(fi);
	bsssiz = getw(fi);
	symsiz = getw(fi);
	getw(fi);
	getw(fi);
	getw(fi);
	switch(magic) {
	case 0407:
	case 0410:
		break;
	default:
		printf("\n%s - (%o) bad magic number\n",argv[1],magic);
		exit(1);
	}
	sumck = 0;
	for(i=0; i<(txtsiz/2); i++)
		sumck += getw(fi);
	for(i=0; i<(datsiz/2); i++)
		sumck += getw(fi);
	close(fi);
	if(nlist(argv[1], nl) < 0) {
		printf("\nsumck: %s - no name list\n", argv[1]);
		exit(1);
	}
	if((nl[0].n_type == 0) || (nl[0].n_value == 0)) {
		printf("\nsumck: %s - sumcheck location missing\n", argv[1]);
		exit(1);
	}
	fd = open(argv[1], 2);
	if(fd < 0) {
		printf("\nsumck: Can't open %s for RW\n", argv[1]);
		exit(1);
	}
	if(magic == 0410)
		scloc = 020 + txtsiz + (nl[0].n_value - ((txtsiz+017777)&~017777));
	else
		scloc = 020 + nl[0].n_value;
	lseek(fd, (long)scloc, 0);
	read(fd, (char *)&scc, sizeof(scc));
	if(scc != 0L) {
		printf("\nsumck: %s - sumcheck location nonzero\n", argv[1]);
		exit(1);
	}
	lseek(fd, (long)scloc, 0);
	scc -= sumck;
	write(fd, (char *)&scc, sizeof(scc));
	close(fd);
	exit(0);
}
