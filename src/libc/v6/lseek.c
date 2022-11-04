
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)lseek.c	3.0	4/22/86
 */
lseek(fd, off, ptr)
int fd, ptr;
long off;
{
	unsigned a;

	a = off;

	if (a == off)
		return (syscall(19, fd, 0, a, ptr, 0));
	a = off/512;
	syscall(19, fd, 0, a, ptr+3, 0);
	return(syscall(19, fd, 0, (int) (off%512), ptr, 0));
}
