
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)gtty.c	3.0	4/22/86
 */
gtty(fd, buf)
int fd;
int *buf;
{
	if (syscall(32, fd, 0, buf, 0, 0) < 0)
		return(-1);
	return(0);
}
