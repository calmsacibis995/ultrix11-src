
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)stty.c	3.0	4/22/86
 */
/*
 * Writearound to old stty and gtty system calls
 */

#include <sgtty.h>

stty(fd, ap)
struct sgtty *ap;
{
	return(ioctl(fd, TIOCSETP, ap));
}

gtty(fd, ap)
struct sgtty *ap;
{
	return(ioctl(fd, TIOCGETP, ap));
}
