
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * static char sccsid[] = "@(#)readdir.c	3.0	4/22/86";
 */

#include <sys/types.h>
#include <ndir.h>

/*
 * Read old style directories and put them into the directory
 * buffer in the new format.
 */
#include "odir.h"

/*
 * get next entry in a directory.
 */
struct direct *
readdir(dirp)
	register DIR *dirp;
{
	register struct olddirect *odp;
	register struct direct *dp;
	char tbuf[(DIRBLKSIZ/OENTSIZ)*sizeof(struct olddirect)];
	long tsize;

	for (;;) {
		if (dirp->dd_loc == 0) {
			dirp->dd_size = read(dirp->dd_fd, tbuf, sizeof(tbuf));
			if (dirp->dd_size <= 0)
				return (NULL);
			tsize = dirp->dd_size;
			dirp->dd_size = 0;
			odp = tbuf;
			dp = dirp->dd_buf;
			for(;tsize > 0; odp++, tsize -= sizeof(struct olddirect)) {
				dirp->dd_size += OENTSIZ;
				dp->d_ino = odp->od_ino;
				dp->d_reclen = OENTSIZ;
				strncpy(dp->d_name, odp->od_name, ODIRSIZ);
				dp->d_name[ODIRSIZ] = '\0'; /* force null */
				dp->d_namlen = strlen(dp->d_name);
				dp = (struct direct *)((char *)dp + OENTSIZ);
			}
			if (dp == dirp->dd_buf)
				continue;
			dp = (struct direct *)((char *)dp - OENTSIZ);
		}
		if (dirp->dd_loc >= dirp->dd_size) {
			dirp->dd_loc = 0;
			continue;
		}
		dp = (struct direct *)(dirp->dd_buf + dirp->dd_loc);
		if (dp->d_reclen <= 0 ||
		    dp->d_reclen > DIRBLKSIZ + 1 - dirp->dd_loc)
			return (NULL);
		dirp->dd_loc += dp->d_reclen;
		if (dp->d_ino == 0)
			continue;
		return (dp);
	}
}
