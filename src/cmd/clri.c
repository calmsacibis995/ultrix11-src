
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * clri filsys inumber ...
 */

static char Sccsid[] = "@(#)clri.c 3.0 4/21/86";
#ifdef	UCB_NKB
#include <sys/param.h>
#else
#include <sys/types.h>
#endif	UCB_NKB
#include <sys/ino.h>
#define ISIZE	(sizeof(struct dinode))
#ifndef	UCB_NKB
#define	BSIZE	512
#endif	UCB_NKB
#define	NI	(BSIZE/ISIZE)
struct	ino
{
	char	junk[ISIZE];
};
struct	ino	buf[NI];
int	status;

main(argc, argv)
char *argv[];
{
	register i, f;
	unsigned n;
	int j, k;
	long off;

	if(argc < 3) {
		printf("usage: clri filsys inumber ...\n");
		exit(4);
	}
	f = open(argv[1], 2);
	if(f < 0) {
		printf("cannot open %s\n", argv[1]);
		exit(4);
	}
	for(i=2; i<argc; i++) {
		if(!isnumber(argv[i])) {
			printf("%s: is not a number\n", argv[i]);
			status = 1;
			continue;
		}
		n = atoi(argv[i]);
		if(n == 0) {
			printf("%s: is zero\n", argv[i]);
			status = 1;
			continue;
		}
#ifdef	UCB_NKB
		off = itod(n) * BSIZE;
#else
		off = ((n-1)/NI + 2) * (long)512;
#endif	UCB_NKB
		lseek(f, off, 0);
		if(read(f, (char *)buf, BSIZE) != BSIZE) {
			printf("%s: read error\n", argv[i]);
			status = 1;
		}
	}
	if(status)
		exit(status);
	for(i=2; i<argc; i++) {
		n = atoi(argv[i]);
		printf("clearing %u\n", n);
#ifdef	UCB_NKB
		off = itod(n) * BSIZE;
#else
		off = ((n-1)/NI + 2) * (long)512;
#endif	UCB_NKB
		lseek(f, off, 0);
		read(f, (char *)buf, BSIZE);
#ifdef	UCB_NKB
		j = itoo(n);
#else
		j = (n-1)%NI;
#endif	UCB_NKB
		for(k=0; k<ISIZE; k++)
			buf[j].junk[k] = 0;
		lseek(f, off, 0);
		write(f, (char *)buf, BSIZE);
	}
	exit(status);
}

isnumber(s)
char *s;
{
	register c;

	while(c = *s++)
		if(c < '0' || c > '9')
			return(0);
	return(1);
}
