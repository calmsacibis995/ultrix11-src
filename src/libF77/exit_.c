
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 *	SCCSID: @(#)exit_.c	3.0	4/22/86
 *	(Berkeley 2.9)  exit_.c  1.1
 */

exit_(n)
long *n;
{
	f_exit();
	_cleanup();
	exit((int)*n);
}
