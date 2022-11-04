
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)mem.c	3.0	4/21/86
 */
#
/*
 */

/* Made changes to implement maus
 * George Mathew  6/26/85 */

/* Removed mmoff routine from this file and put in sys/maus.c
 * easier to make maus sysgenable  George Mathew 6/28/85 */

/*
 *	Memory special file
 *	minor device 0 is physical memory
 *	minor device 1 is kernel memory
 *	minor device 2 is EOF/RATHOLE
 *	minor device >= 8 are MAUS
 */

#include <sys/param.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/conf.h>
#include <sys/seg.h>
#include <sys/maus.h>

#define	NBPC	64	/* number of bytes/click */

mmread(dev)
register dev;
{
	register unsigned n;
	long offset;

	if ((dev = minor(dev)) == 2)
		return;
	while (u.u_error == 0 && u.u_count != 0) {
		if (dev > 2) {
			if ( (offset = mmoff(dev)) < 0)
				break;
			n = MIN(u.u_count, NBPC);
		} else {
			offset = u.u_offset;
			n = MIN(u.u_count, BSIZE);
		}
		if (dev == 1) {
			if (copyout((short)offset, u.u_base, n))
				u.u_error = ENXIO;
		} else {
			if (copyio(offset, u.u_base, n, U_RUD))
				u.u_error = ENXIO;
		}
		u.u_offset += n;
		u.u_base += n;
		u.u_count -= n;
	}
}

mmwrite(dev)
register dev;
{
	register unsigned n;
	long offset;

	if ((dev = minor(dev)) == 2) {
		u.u_count = 0;
		return;
	}
	while (u.u_error == 0 && u.u_count != 0) {
		if (minor(dev) > 2) {    /* if not for maus */
			if ( (offset = mmoff(minor(dev))) <0)
				break;
			n = MIN(u.u_count, NBPC);
		} else {
			offset = u.u_offset;
			n = MIN(u.u_count, BSIZE);
		}
		if (dev == 1) {
			if (copyin(u.u_base, (short)offset, n))
				u.u_error = ENXIO;
		} else {
			if (copyio(offset, u.u_base, n, U_WUD))
				u.u_error = ENXIO;
		}
		u.u_offset +=n;
		u.u_base += n;
		u.u_count -= n;
	}
}
