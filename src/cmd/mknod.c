
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)mknod.c 3.0 4/21/86";

/* added fifo files. Use p option to create fifo file
 *	George Mathew  6/24/85 */

#include	<sys/types.h>
#include	<sys/stat.h>
main(argc, argv)
int argc;
char **argv;
{
	int m, a, b;

	if (argc ==3 && !strcmp(argv[2], "p")) {  /* FIFO */
		a = mknod(argv[1], S_IFIFO | 0666, 0);
		chown(argv[1],getuid(),getgid());
		if(a)
			perror("mknod");
		exit(a == 0? 0:2);
	}
	if(argc != 5) {
		printf("arg count\n");
		goto usage;
	}
	if(*argv[2] == 'b')
		m = 060666; else
	if(*argv[2] == 'c')
		m = 020666; else
		goto usage;
	a = number(argv[3]);
	if(a < 0)
		goto usage;
	b = number(argv[4]);
	if(b < 0)
		goto usage;
	if(mknod(argv[1], m, (a<<8)|b) < 0)
		perror("mknod");
	exit(0);

usage:
	printf("usage: mknod name b/c major minor\n");
	printf("usage for pipe/fifo: mknod name p\n");
}

number(s)
char *s;
{
	int n, c;

	n = 0;
	while(c = *s++) {
		if(c<'0' || c>'9')
			return(-1);
		n = n*10 + c-'0';
	}
	return(n);
}
