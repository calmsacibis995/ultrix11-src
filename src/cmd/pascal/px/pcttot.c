
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#
/*
 * SCCSID: @(#)pcttot.c	3.0	4/22/86
 */

#include "0x.h"
#include "E.h"

/*
 * Constant set constructor (yechh!)
 */
pcttot(uprbp, lwrb, n, av)
{
	register *set;
	register l;
	int *ap, h;

	ap = &av;
	set = &ap[2 * n];
	while(--n >= 0) {
		if ((l = *ap++ - lwrb) < 0 || l > uprbp ||
		    (h = *ap++ - lwrb) < 0 || h > uprbp)
			error(ECTTOT);
		while (l <= h) {
			set[l >> 4] =| 1 << (l & 017);
			l++;
		}
	}
	return(set);
}
