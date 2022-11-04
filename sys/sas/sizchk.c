static char Sccsid[] = "@(#)sizchk.c	3.0	4/21/86";

/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * ULTRIX-11 standalone program size check program (sizchk).
 *
 * text+data+bss must be <= argv[2] bytes
 * syssiz must be 0
 * magic must be 407
 *
 * Fred Canter 12/13/83
 */

#include <stdio.h>

main(argc, argv)
int	argc;
char	*argv[];
{
	FILE *fi;
	unsigned int magic, txtsiz, datsiz, bsssiz, symsiz, maxsiz;

	if(argc != 3) {
		printf("\nsizchk: bad arg count!\n");
		exit(1);
	}
	fi = fopen(argv[1], "r");
	if(fi == NULL) {
		printf("\nsizchk: Can't open %s!\n", argv[1]);
		exit(1);
	}
	magic = getw(fi);
	txtsiz = getw(fi);
	datsiz = getw(fi);
	bsssiz = getw(fi);
	symsiz = getw(fi);
	close(fi);
	if(magic != 0407) {
		printf("\n%s - (%o) magic number not 0407\n",argv[1],magic);
		exit(1);
	}
	maxsiz = atoi(argv[2]);
	if((maxsiz == 0) || (maxsiz > 57344)) {
		printf("\nBad size argument (%s)!\n", argv[2]);
		exit(1);
	}
	if((txtsiz+datsiz+bsssiz) > maxsiz) {
		printf("\n%s - total size exceeds %u bytes\n", argv[1], maxsiz);
		exit(1);
	}
	if(symsiz) {
		printf("\n%s - symbol table not stripped\n", argv[1]);
		exit(1);
	}
	exit(0);
}
