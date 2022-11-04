
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*	SCCSID: @(#)lockf.c	3.0	4/22/86	*/
/*	(System 5)	1.3	*/
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

lockf(fildes, function, size)
long size;
int fildes, function;
{
	struct flock l;
	int rv;

	l.l_whence = 1;
	if (size < 0) {
		l.l_start = size;
		l.l_len = -size;
	} else {
		l.l_start = 0L;
		l.l_len = size;
	}
	switch (function) {
	case F_ULOCK:
		l.l_type = F_UNLCK;
		rv = fcntl(fildes, F_SETLK, &l);
		break;
	case F_LOCK:
		l.l_type = F_WRLCK;
		rv = fcntl(fildes, F_SETLKW, &l);
		break;
	case F_TLOCK:
		l.l_type = F_WRLCK;
		rv = fcntl(fildes, F_SETLK, &l);
		break;
	case F_TEST:
		l.l_type = F_WRLCK;
		rv = fcntl(fildes, F_GETLK, &l);
		if (rv != -1) {
			if (l.l_type == F_UNLCK)
				return (0);
			else {
				errno = EAGAIN;  /* changed to comply with SV interface definiton: George Mathew*/
				return (-1);
			}
		}
	default:
		errno = EINVAL;
		return (-1);
	}
	if (rv < 0) {
		switch(errno) {
		case EMFILE:
		case ENOSPC:
			/* A deadlock error is given if we run out of resources,
			 * in compliance with /usr/group standards.
			 */
			errno = EDEADLK;
			break;
		default:
			break;
		}
		return(-1);
	}
	return (0);
}



