/*
 * SCCSID: @(#)maketape.c	3.0	4/21/86
 */
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#include <stdio.h>
#define MAXB 30
int mt;
int fd;
char	buf[MAXB*512];
char	name[50];
int	blksz;

main(argc, argv)
int	argc;
char	*argv[];
{
	int i, j, k, cnt;
	int *bp;
	FILE *mf;

	if (argc != 3) {
		fprintf(stderr, "Usage: maketape tapedrive makefile\n");
		exit(0);
	}
	if ((mt = creat(argv[1], 0666)) < 0) {
		perror(argv[1]);
		exit(1);
	}
	if ((mf = fopen(argv[2], "r")) == NULL) {
		perror(argv[2]);
		exit(2);
	}

	j = 0;
	k = 0;
	for (;;) {
		if ((i = fscanf(mf, "%s %d", name, &blksz))== EOF)
			exit(0);
		if (i != 2) {
			fprintf(stderr, "Help! Scanf didn't read 2 things (%d)\n", i);
			exit(1);
		}
		if (blksz <= 0 || blksz > MAXB) {
			fprintf(stderr, "Block size %d is invalid\n", blksz);
			continue;
		}
		if (strcmp(name, "*") == 0) {
			close(mt);
			mt = open(argv[1], 2);
			j = 0;
			k++;
			continue;
		}
		fd = open(name, 0);
		if (fd < 0) {
			perror(name);
			continue;
		}
		printf("%s: block %d, file %d\n", name, j, k);
/*
 * Zero the buffer before each read.
 * This makes sure any partial last record
 * is zero filled, so any junk past the EOF
 * will not corrupt the beginning of BSS space
 * in boot.
 */
		while(1) {
			bp = &buf;
			for(cnt=0; cnt<((512*blksz)/2); cnt++)
				*bp++ = 0;
			if(read(fd, buf, 512*blksz) <= 0)
				break;
			j++;
			write(mt, buf, 512*blksz);
		}
	}
}
