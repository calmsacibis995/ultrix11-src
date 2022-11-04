
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)shuffle.c	3.0	4/21/86
 */

/*
 * Memory shuffle. Called when a process requests or releases
 * a memory lock, or when a locked process grows.
 * This code is a sysgen option. A dummy shuffle routine will
 * be included in c.c if this code is not desired.
 * The dummy shuffle routine must include:
 *	wakeup((caddr_t)&runlock);
 * Ohms 5/7/85
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/acct.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/inode.h>
#include <sys/proc.h>
#include <sys/text.h>
#include <sys/seg.h>
#include <sys/lock.h>

/*
 *
 * Shuffle memory. This is done when sched() finds runlock
 * non-zero. Runlock gets incremented whenever a process
 * sets/clears a lock, or when a locked process grows.
 * This code is separated so it can be a sysgen option.
 * 
 * Ohms 3/20/84
 */

shuffle()
{
	register struct proc *pp;
	register struct text *xp;

	runlock = 0;
	for(pp = &proc[1]; pp < &proc[nproc]; pp++) {
		if (pp->p_stat == SIDL)
			continue;
		if ((pp->p_flag&SLOCK) || (pp->p_stat==NULL) || (pp->p_stat==SZOMB)
		  || ((xp=pp->p_textp) && xp->x_flag&XLOCK))
			continue;
		if (pp->p_flag&SLOAD) {
			/* swap out complete process */
			pp->p_flag &= ~SLOAD;
			xswap(pp,1,0);
		}
		if (xp != NULL && xp->x_lcount && xp->x_ccount==1)
			/*swap text out */
			xccdec(xp);
	}
	for(pp = &proc[1]; pp < &proc[nproc]; pp++) {
		if (pp->p_stat == SIDL)
			continue;
		if ((pp->p_stat==NULL) || (pp->p_stat==SZOMB))
			continue;
		if ((xp=pp->p_textp) != NULL && xp->x_lcount
			&& xp->x_ccount == 0)
			/*swap text in only */
			xswapin(xp);
		if ((pp->p_flag&SSYS) && (pp->p_flag&SLOAD) == 0)
			/*swap data only in */
			swapin(pp);
	}
	wakeup((caddr_t)&runlock);
}
