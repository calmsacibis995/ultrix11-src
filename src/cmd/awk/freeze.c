
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/
static char Sccsid[] = "@(#)freeze.c 3.0 4/21/86";
#include "stdio.h"
freeze(s) char *s;
{	int fd;
	unsigned int *len;
	len = (unsigned int *)sbrk(0);
	if((fd = creat(s, 0666)) < 0) {
		fprintf(stderr, "cannot create %s\n", s);
		return(1);
	}
	write(fd, &len, sizeof(len));
	write(fd, (char *)0, len);
	close(fd);
	return(0);
}

thaw(s) char *s;
{	int fd;
	unsigned int *len;
	if(*s == 0) {
		fprintf(stderr, "empty restore file\n");
		return(1);
	}
	if((fd = open(s, 0)) < 0) {
		fprintf(stderr, "cannot open %s\n", s);
		return(1);
	}
	read(fd, &len, sizeof(len));
	/*(void)*/ brk(len);
	read(fd, (char *)0, len);
	close(fd);
	return(0);
}
