
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)sys_berk.c	3.0	4/21/86
 *
 * This module contains the system calls that are from Berkeley code
 */

#include <sys/param.h>
#include <sys/proc.h>
#include <sys/inode.h>
#include <sys/dir.h>
#include <sys/systm.h>
#include <sys/user.h>

/*
 * Setpgrp on specified process and its descendants.
 * Pid of zero implies current process.
 * Pgrp of zero is getpgrp system call returning
 * current process group.
 */
setpgrp()
{
	register struct proc *top;
	register struct a {
		int pid;
		int pgrp;
	} *uap;

	uap = (struct a *)u.u_ap;
	if (uap->pid == 0)
		top = u.u_procp;
	else {
		for(top = &proc[0]; top <= maxproc; top++)
			if (uap->pid == top->p_pid) goto found1;
		top = 0;
	found1:
		if (top == 0) {
			u.u_error = ESRCH;
			return;
		}
	}
	if (uap->pgrp <= 0) {
		u.u_rval1 = top->p_pgrp;
		return;
	}
	if (top->p_uid != u.u_uid && u.u_uid && !inferior(top))
		u.u_error = EPERM;
	else
		top->p_pgrp = uap->pgrp;
}

spgrp(top, npgrp)
register struct proc *top;
{
	register struct proc *pp, *p;
	int f = 0;

	for (p=top; npgrp == -1 || u.u_uid == p->p_uid ||
	    !u.u_uid || inferior(p); p = pp) {
		if (npgrp == -1) {
#define bit(a)	(1L<<(a-1))
			p->p_sig &= ~(bit(SIGTSTP)|bit(SIGTTIN)|bit(SIGTTOU));
			p->p_flag |= SDETACH;
		} else
			p->p_pgrp = npgrp;
		f++;
		/*
		 * Search for children.
		 */
		for (pp = &proc[0]; pp <= maxproc; pp++)
			if (pp->p_pptr == p)
				goto cont;
		/*
		 * Search for siblings.
		 */
		for (; p != top; p = p->p_pptr)
			for (pp = p + 1; pp <= maxproc; pp++)
				if (pp->p_pptr == p->p_pptr)
					goto cont;
		break;
	cont:
		;
	}
	return (f);
}

/*
 * Is p an inferior of the current process?
 */
inferior(p)
register struct proc *p;
{
	for (; p != u.u_procp; p = p->p_pptr)
		if (p <= &proc[1])
			return (0);
	return(1);
}

