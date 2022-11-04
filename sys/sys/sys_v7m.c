
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)sys_v7m.c	3.0	4/21/86
 */

/*
 * This file contains system calls that are specific to
 * V7M-11.  They are accessed via the syslocal[] table.
 */

#include <sys/param.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/seg.h>
#include <sys/systm.h>
#include <sys/utsname.h>

unsigned	hnamelen = 0;

long		LocalAddr;

gethostid()
{
	u.u_r.r_off = LocalAddr;
}

sethostid()
{
	struct a {
		long	hostid;
	} *uap = (struct a *)u.u_ap;

	if (suser())
		LocalAddr = uap->hostid;
}

/* changed gethostname and sethostname for the hostname to be updated in
 * utsname.nodename (in sys/utsname.h). This structure is used by uname().
 * George Mathew: 7/30/85
 */

gethostname()
{
	struct a {
		char		*hostname;
		unsigned	len;
	};
	register struct a *uap;
	register unsigned len;

	uap  = (struct a *)u.u_ap;
	len = uap->len;
	if (len > hnamelen + 1)
		len = hnamelen + 1;
	if (copyout(utsname.nodename, uap->hostname, len))
		u.u_error = EFAULT;
}

sethostname()
{
	struct a {
		char		*hostname;
		unsigned	len;
	};
	register struct a *uap;
	register unsigned	len;

	uap = (struct a *)u.u_ap;
	if (!suser())
		return;
	if (uap->len > sizeof(utsname.nodename) - 1) {
		u.u_error = EINVAL;
		return;
	}
	hnamelen = uap->len;
	if (copyin(uap->hostname, utsname.nodename, uap->len))
		u.u_error = EFAULT;
	utsname.nodename[hnamelen] = 0;
}

/*
 * zaptty - zap the controlling tty. This is useful for restarting
 *	    demons that die for some reason and they shouldn't be
 *	    associated with any terminal.  If any terminal is opened
 *	    after the zaptty command, that terminal becomes the
 *	    controlling tty. Obviously, this is a superuser only command.
 */

zaptty()
{
	if (suser()) {
		u.u_procp->p_pgrp = 0;
		u.u_ttyp = 0;
		u.u_ttyd = 0;
	}
}

extern int	fpemulation;

fpsim()
{
	register struct a {
		int	nfpval;
	} *uap = (struct a *)u.u_ap;

	u.u_rval1 = fpemulation;
	if (uap->nfpval == 0 || uap->nfpval == 1) {
		if (u.u_uid)
			u.u_error = EPERM;
		else if (fpemulation != 2)
			fpemulation = uap->nfpval;
		else
			u.u_error = ENODEV;
	} else if (uap->nfpval != 2)
		u.u_error = EINVAL;
}

/*
 * event flag system call
 *
 * 1/1984	Bill Burns
 *
 * one syscall for all event flag functions
 * usage is as follows:
 *
 * 	evntflg(req, flag, value)
 * 	int	req;	request, read, write, release, set, clear
 * 	int	flag;	if request then flag == mode
 *			if read, write, release, set or clear, then flag == efid
 * 	long	value;	initail value for request
 *			new value for write
 *			bit position for set or clear
 *			not used for read or release.
 */

#include <sys/eflg.h>

/* extern int	NEFLG;  should come from sysgen (c.c) */
#define NEFLG	2

struct eflg {
	int	e_efid;		/* unique id */
	int	e_pid;		/* controlling pid */
	int	e_pgrp;		/* prog group */
	int	e_mode;		/* mode (own r/w, gr r/w, wo r/w) */
	long	e_flags;	/* actual flags */
} eflg[NEFLG];

int	efid;


evntflg()
{
	struct eflg *efp, *efgp;
	struct proc *pp;
	long oldflg;

	struct a {
		int	req;
		int	flag;
		long	value;
	} *uap;
	int	req, flag;
	long	value;

	uap  = (struct a *)u.u_ap;
	req = uap->req;
	flag = uap->flag;
	value = uap->value;

	/* weed out stupid calls */
	if(req == 0) {
		u.u_error = EINVAL;
		u.u_r.r_off = (long)-1;
		return;
	}
	switch(req) {
	case	EFREQ:
	retry:
		efid++;
		if(efid >= 30000) {
			efid = 1;
			goto retry;
		}
		for(efgp = &eflg[0]; efgp < &eflg[NEFLG]; efgp++) {
			if (efgp->e_efid==efid)
				goto retry;
		}
		efp = NULL;
		for(efgp = &eflg[0]; efgp < &eflg[NEFLG]; efgp++) {
			if((efgp->e_mode == NULL) && (efgp->e_efid == NULL)) {
				efp = efgp;
				break;
			}
		}
		if(!efp) {
			u.u_error = ENOSPC;
			u.u_r.r_off = (long)-1;
			return;
		}
		if((flag & ~0666) || (flag == 0)){
			u.u_error = EINVAL;
			u.u_r.r_off = (long)-1;
			return;
		}
		efp->e_efid = efid;
		efp->e_mode = flag;
		efp->e_flags = value;
		pp = u.u_procp;
		efp->e_pid =  pp->p_pid;
		efp->e_pgrp = pp->p_pgrp;
		u.u_r.r_off = efid;
		return;
	case	EFRD:
		if(!flag) {
			u.u_error = EINVAL;
			u.u_r.r_off = (long)-1;
			return;
		}
		efp = NULL;
		for(efgp = &eflg[0]; efgp < &eflg[NEFLG]; efgp++) {
			if (efgp->e_efid == flag)
				efp = efgp;
		}
		if(efp == NULL) {
			u.u_error = EINVAL;
			u.u_r.r_off = (long)-1;
			return;
		}
		pp = u.u_procp;
		if(efp->e_pid == pp->p_pid) {
			if(efp->e_mode & OWR) {	
				u.u_r.r_off = efp->e_flags;
				return;
			} else {
				u.u_error = EACCES;
				u.u_r.r_off = (long)-1;
				return;
			}
		} else if(efp->e_pgrp == pp->p_pgrp) {
			if(efp->e_mode & GRR) {
				u.u_r.r_off = efp->e_flags;
				return;
			} else {
				u.u_error = EACCES;
				u.u_r.r_off = (long)-1;
				return;
			}
		} else {
			if(efp->e_mode & WOR) {
				u.u_r.r_off = efp->e_flags;
				return;
			} else {
				u.u_error = EACCES;
				u.u_r.r_off = (long)-1;
				return;
			}
		}
	case	EFWRT:
		if(!flag) {
			u.u_error = EINVAL;
			u.u_r.r_off = (long)-1;
			return;
		}
		efp = NULL;
		for(efgp = &eflg[0]; efgp < &eflg[NEFLG]; efgp++) {
			if (efgp->e_efid == flag)
				efp = efgp;
		}
		if(efp == NULL) {
			u.u_error = EINVAL;
			u.u_r.r_off = (long)-1;
			return;
		}
		pp = u.u_procp;
		if(efp->e_pid == pp->p_pid) {
			if(efp->e_mode & OWW) {	
				oldflg = efp->e_flags;
				efp->e_flags = value;
				u.u_r.r_off = oldflg;
				return;
			} else {
				u.u_error = EACCES;
				u.u_r.r_off = (long)-1;
				return;
			}
		} else if(efp->e_pgrp == pp->p_pgrp) {
			if(efp->e_mode & GRW) {
				oldflg = efp->e_flags;
				efp->e_flags = value;
				u.u_r.r_off = oldflg;
				return;
			} else {
				u.u_error = EACCES;
				u.u_r.r_off = (long)-1;
				return;
			}
		} else {
			if(efp->e_mode & WOW) {
				oldflg = efp->e_flags;
				efp->e_flags = value;
				u.u_r.r_off = oldflg;
				return;
			} else {
				u.u_error = EACCES;
				u.u_r.r_off = (long)-1;
				return;
			}
		}
	case	EFSET:
		if(!flag) {
			u.u_error = EINVAL;
			u.u_r.r_off = (long)-1;
			return;
		}
		efp = NULL;
		for(efgp = &eflg[0]; efgp < &eflg[NEFLG]; efgp++) {
			if (efgp->e_efid == flag)
				efp = efgp;
		}
		if(efp == NULL) {
			u.u_error = EINVAL;
			u.u_r.r_off = (long)-1;
			return;
		}
		pp = u.u_procp;
		if(efp->e_pid == pp->p_pid) {
			if(efp->e_mode & OWW) {	
				oldflg = efp->e_flags;
				efp->e_flags |= (1 << value);
				u.u_r.r_off = oldflg;
				return;
			} else {
				u.u_error = EACCES;
				u.u_r.r_off = (long)-1;
				return;
			}
		} else if(efp->e_pgrp == pp->p_pgrp) {
			if(efp->e_mode & GRW) {
				oldflg = efp->e_flags;
				efp->e_flags |= (1 << value);
				u.u_r.r_off = oldflg;
				return;
			} else {
				u.u_error = EACCES;
				u.u_r.r_off = (long)-1;
				return;
			}
		} else {
			if(efp->e_mode & WOW) {
				oldflg = efp->e_flags;
				efp->e_flags |= (1 << value);
				u.u_r.r_off = oldflg;
				return;
			} else {
				u.u_error = EACCES;
				u.u_r.r_off = (long)-1;
				return;
			}
		}
	case	EFCLR:
		if(!flag) {
			u.u_error = EINVAL;
			u.u_r.r_off = (long)-1;
			return;
		}
		efp = NULL;
		for(efgp = &eflg[0]; efgp < &eflg[NEFLG]; efgp++) {
			if (efgp->e_efid == flag)
				efp = efgp;
		}
		if(efp == NULL) {
			u.u_error = EINVAL;
			u.u_r.r_off = (long)-1;
			return;
		}
		pp = u.u_procp;
		if(efp->e_pid == pp->p_pid) {
			if(efp->e_mode & OWW) {	
				oldflg = efp->e_flags;
				efp->e_flags &= ~(1 << value);
				u.u_r.r_off = oldflg;
				return;
			} else {
				u.u_error = EACCES;
				u.u_r.r_off = (long)-1;
				return;
			}
		} else if(efp->e_pgrp == pp->p_pgrp) {
			if(efp->e_mode & GRW) {
				oldflg = efp->e_flags;
				efp->e_flags &= ~(1 << value);
				u.u_r.r_off = oldflg;
				return;
			} else {
				u.u_error = EACCES;
				u.u_r.r_off = (long)-1;
				return;
			}
		} else {
			if(efp->e_mode & WOW) {
				oldflg = efp->e_flags;
				efp->e_flags &= ~(1 << value);
				u.u_r.r_off = oldflg;
				return;
			} else {
				u.u_error = EACCES;
				u.u_r.r_off = (long)-1;
				return;
			}
		}
	case	EFREL:
		efp = NULL;
		for(efgp = &eflg[0]; efgp < &eflg[NEFLG]; efgp++) {
			if (efgp->e_efid == flag)
				efp = efgp;
		}
		if(efp == NULL) {
			u.u_error = EINVAL;
			u.u_r.r_off = (long)-1;
			return;
		}
		pp = u.u_procp;
		if(efp->e_pid != pp->p_pid) {
			u.u_error = EACCES;
			u.u_r.r_off = (long)-1;
			return;
		}
		oldflg = efp->e_flags;
		efp->e_mode = 0;
		efp->e_efid = 0;
		u.u_r.r_off = oldflg;
		return;
	} /* end of switch */
}

/*
 * routine called on exit of
 * a process to clean out eflg
 * structures.
 *
 */

eflgcln(p)
struct proc *p;
{
	struct eflg *efgp;

	for(efgp = &eflg[0]; efgp < &eflg[NEFLG]; efgp++) {
		if (efgp->e_pid == p->p_pid) {
			efgp->e_efid = NULL;
			efgp->e_mode = NULL;
		}
	}
}

getfp()
{
	register struct a {
		int	insptr;
		int	bufptr;
	} *uap;
	register unsigned inst;

	uap = (struct a *)u.u_ap;
	inst = fuiword((caddr_t)uap->insptr);
	suword((caddr_t)uap->bufptr, inst);
}

/*
 * fperr system call
 * return floating point error registers
 */

fperr()
{
	u.u_rval1 = u.u_fperr.f_fec;
	u.u_rval2 = u.u_fperr.f_fea;
}

/*
 * nostk -- Set up the process to have no stack segment.
 *	    The process is responsible for the management
 *	    of its own stack, and can thus use the full
 *	    64k byte address space.
 */

nostk()
{
	register size;

	size = u.u_procp->p_size - u.u_ssize;
	if (estabur(u.u_tsize, u.u_dsize, 0, u.u_sep, RO))
		return;
	u.u_ssize = 0;
	expand(size);
}

/*
 * Sleep for less than 1 second.
 * Silently enforces that non-suser procesess can
 * only nap for 1 second.
 */
nap()
{
	register struct a {
		int	ticks;
	};
	register ticks;
	register s;
	extern int hz;
	int wakeup();

	ticks = ((struct a *)u.u_ap)->ticks;
	if (ticks < 0)
		return;
	if (ticks > hz && u.u_uid)
		ticks = hz;
	s = spl6();
	timeout (wakeup, (caddr_t)u.u_procp+1, ticks);
	sleep((caddr_t)u.u_procp+1, PZERO-1);
	splx(s);
}
/*
 * renice -- change the nice value of a process
 */
renice()
{
	register struct proc *p;
	register struct a {
		int pid;
		int nice;
	} *uap;

	uap = (struct a *) u.u_ap;

	/*
	 * Don't renice swapper, init, or elc!
	 */
	for (p = &proc[3]; p <= maxproc; p++)
		if (p->p_pid == uap->pid) {
			if (suser()) {
				u.u_rval1 = p->p_nice;
				if (uap->nice > 127)
					p->p_nice = 127;
				else if (uap->nice < -127)
					p->p_nice = -127;
				else
					p->p_nice = uap->nice;
			} else if (p->p_uid == u.u_uid) {
				u.u_rval1 = p->p_nice;
				if (uap->nice > p->p_nice) {
					if (uap->nice > 127)
						p->p_nice = 127;
					else
						p->p_nice = uap->nice;
				}
				u.u_error = 0;
			}
			return;
		}
	u.u_error = ESRCH;
}
#ifdef	NEWLIMITS
#include <sys/acct.h>
/*
 * Enforce login limits.  This is called by login.
 * Init calls this to set the limit.  Valid limits
 * are 8*17, 16*17, 32*17, and 100*17.  In octal that
 * is 0210, 0420, 01040, and 03244.  Bits 0174103 wil
 * never be set, so we check some of them to verify
 * a valid user limit.
 * The current number of users is kept in cnl, which
 * is 131*13 + current number of users.
 */
int cnl = 131*13;
lim()
{
	register struct a {
		int arg;
	} *uap = (struct a *) u.u_ap;
	static unsigned int x = 0173355;		/* garbage value */
	register unsigned int a1, a2;

	if (uap->arg && u.u_procp->p_pid == 1) {	/* Only init can set the limit */
		a2 = uap->arg;
		if (!(a2&024103))
			u.u_rval1 = (x = a2)/17;
		else {
			u.u_rval1 = 0;
			x = 0173355;	/* arbitray, just some bad bits */
		}
		return;
	}
	a1 = cnl;
	if (!((a2 = x)&0154103))	/* Has the limit been fiddled ? */
		a1 %= 131;		/* nope, get # users */
	a2 /= 17;
	if (u.u_acflag & ARESV)		/* are we already a login process? */
		return;			/* yep, we're already counted */
	if (a1 >= a2)
		u.u_error = E2BIG;
	else {
		u.u_acflag |= ARESV;	/* The reserve bit marks login procs */
		++cnl;
	}
}
#endif	NEWLIMITS
