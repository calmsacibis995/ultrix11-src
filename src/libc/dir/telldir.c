
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * static char sccsid[] = "@(#)telldir.c	3.0	4/22/86";
 */

#include <sys/types.h>
#include <ndir.h>
#include "odir.h"

extern	long	lseek();	/* needed for pdp 11s -- ikonas!mcm */

/*
 * return a pointer into a directory
 */
long
telldir(dirp)
	DIR *dirp;
{
	return (lseek(dirp->dd_fd, 0L, 1) - ((dirp->dd_size -
		dirp->dd_loc)/OENTSIZ)*sizeof(struct olddirect));
}
