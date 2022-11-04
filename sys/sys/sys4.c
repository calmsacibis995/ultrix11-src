
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)sys4.c	3.3	12/16/87
 */
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/reg.h>
#include <sys/inode.h>
#include <sys/proc.h>
#include <sys/timeb.h>
#include <sys/errlog.h>

int	hz;		/* line frequency, see c.c */

/*
 * Everything in this file is a routine implementing a system call.
 */

/*
 * return the current time (old-style entry)
 */
gtime()
{
	u.u_r.r_time = time;
}

/*
 * New time entry-- return TOD with milliseconds, timezone,
 * DST flag
 */
ftime()
{
	register struct a {
		struct	timeb	*tp;
	} *uap;
	struct timeb t;
	register unsigned ms;
	extern int timezone;
	extern int dstflag;

	uap = (struct a *)u.u_ap;
	spl7();
	t.time = time;
	ms = lbolt;
	spl0();
	if (ms > hz) {
		ms -= hz;
		t.time++;
	}
	t.millitm = (1000*ms)/hz;
	t.timezone = timezone;
	t.dstflag = dstflag;
	if (copyout((caddr_t)&t, (caddr_t)uap->tp, sizeof(t)) < 0)
		u.u_error = EFAULT;
}

/*
 * Set the time
 */
stime()
{
	register struct a {
		time_t	time;
	} *uap;
	time_t ntime;
	extern time_t boottime;

	uap = (struct a *)u.u_ap;
	if(suser()) {
/*
 * Enter the time change
 * in the error log.
 */
		ntime = uap->time;
		logerr(E_TC, &ntime, sizeof(ntime));
		boottime += uap->time - time;
		time = uap->time;
	}
}

setuid()
{
	register struct a {
		int	ruid;
		int	euid;
	} *uap = (struct a *)u.u_ap;

	uap->euid = uap->ruid;
	csetuid();
}

setreuid()
{
	register struct a {
		int ruid;
		int euid;
	} *uap = (struct a *)u.u_ap;

	if (uap->ruid == -1)
		uap->ruid = u.u_ruid;
	if (uap->euid == -1)
		uap->euid = u.u_uid;
	csetuid();
}

/*
 * Common code for setuid() and setreuid().  The will have
 * already set up uap->ruid & uap->euid as appropriate.
 */
static csetuid()
{
	register struct a {
		int ruid;
		int euid;
	} *uap = (struct a *)u.u_ap;
	register int ruid, euid;

	ruid = uap->ruid;
	euid = uap->euid;

	if (( (u.u_ruid != ruid && u.u_uid != ruid) ||
	      (u.u_ruid != euid && u.u_uid != euid)    ) && !suser())
		return;
	u.u_procp->p_uid = ruid;
	u.u_ruid = ruid;
	u.u_uid = euid;
}

getuid()
{

	u.u_rval1 = u.u_ruid;
	u.u_rval2 = u.u_uid;
}

setgid()
{
	register struct a {
		int	rgid;
		int	egid;
	} *uap = (struct a *)u.u_ap;

	uap->egid = uap->rgid;
	csetgid();
}

setregid()
{
	register struct a {
		int	rgid;
		int	egid;
	} *uap = (struct a *)u.u_ap;

	if (uap->rgid == -1)
		uap->rgid = u.u_rgid;
	if (uap->egid == -1)
		uap->egid = u.u_gid;
	csetgid();
}

/*
 * Common code for setgid() and setregid().  The will have
 * already set up uap->rgid & uap->egid as appropriate.
 */
static csetgid()
{
	register struct a {
		int	rgid;
		int	egid;
	} *uap = (struct a *)u.u_ap;
	register int rgid, egid;

	rgid = uap->rgid;
	egid = uap->egid;

	if (( (u.u_rgid != rgid && u.u_gid != rgid) ||
	      (u.u_rgid != egid && u.u_gid != egid)    ) && !suser())
		return;
	u.u_gid = egid;
	u.u_rgid = rgid;
}

getgid()
{

	u.u_rval1 = u.u_rgid;
	u.u_rval2 = u.u_gid;
}

getpid()
{
	u.u_rval1 = u.u_procp->p_pid;
	u.u_rval2 = u.u_procp->p_ppid;
}

sync()
{

	update();
}

nice()
{
	register n;
	register struct a {
		int	niceness;
	} *uap;

	uap = (struct a *)u.u_ap;
	n = uap->niceness;
	if(n < 0 && !suser())
		n = 0;
	n += u.u_procp->p_nice;
	if(n >= 2*NZERO)
		n = 2*NZERO -1;
	if(n < 0)
		n = 0;
	u.u_procp->p_nice = n;
}

/*
 * Unlink system call.
 * Hard to avoid races here, especially
 * in unlinking directories.
 */
unlink()
{
	register struct inode *ip, *pp;
	struct a {
		char	*fname;
	};

#ifdef	UCB_SYMLINKS
	pp = namei(uchar, DELETE, 0);
#else	UCB_SYMLINKS
	pp = namei(uchar, DELETE);
#endif	UCB_SYMLINKS
	if(pp == NULL)
		return;
	/*
	 * Check for unlink(".")
	 * to avoid hanging on the iget
	 */
	if (pp->i_number == u.u_dent.d_ino) {
		ip = pp;
		ip->i_count++;
	} else
		ip = iget(pp->i_dev, u.u_dent.d_ino);
	if(ip == NULL)
		goto out1;
	if((ip->i_mode&IFMT)==IFDIR && !suser())
		goto out;
	/*
	 * Don't unlink a mounted file.
	 */
	if (ip->i_dev != pp->i_dev) {
		u.u_error = EBUSY;
		goto out;
	}
	/*
	 * If the sticky bit is turned on the parent directory, then to
	 * remove the file you have meet one of the following conditions:
	 *	1) Be superuser
	 *	2) Own the file 
	 *	3) Own the parent directory
	 */
	if ((pp->i_mode & ISVTX) && u.u_uid &&
	    u.u_uid != pp->i_uid && ip->i_uid != u.u_uid) {
		u.u_error = EPERM;
		goto out;
	}
	if (ip->i_flag&ITEXT)
		xrele(ip);	/* try once to free text */
	if (ip->i_flag&ITEXT && ip->i_nlink==1) {
		u.u_error = ETXTBSY;
		goto out;
	}
	u.u_offset -= sizeof(struct direct);
	u.u_base = (caddr_t)&u.u_dent;
	u.u_count = sizeof(struct direct);
	u.u_dent.d_ino = 0;
	writei(pp);
	ip->i_nlink--;
	ip->i_flag |= ICHG;

out:
	iput(ip);
out1:
	iput(pp);
}
chdir()
{
	chdirec(&u.u_cdir);
}

chroot()
{
	if (suser())
		chdirec(&u.u_rdir);
}

static
chdirec(ipp)
register struct inode **ipp;
{
	register struct inode *ip;
	struct a {
		char	*fname;
	};

#ifdef	UCB_SYMLINKS
	ip = namei(uchar, LOOKUP, 1);
#else	UCB_SYMLINKS
	ip = namei(uchar, LOOKUP);
#endif	UCB_SYMLINKS
	if(ip == NULL)
		return;
	if((ip->i_mode&IFMT) != IFDIR) {
		u.u_error = ENOTDIR;
		goto bad;
	}
	if(access(ip, IEXEC))
		goto bad;
	prele(ip);
	if (*ipp) {
		plock(*ipp);
		iput(*ipp);
	}
	*ipp = ip;
	return;

bad:
	iput(ip);
}

chmod()
{
	register struct inode *ip;
	register struct a {
		char	*fname;
		int	fmode;
	} *uap;

	uap = (struct a *)u.u_ap;
#ifdef	UCB_SYMLINKS
	if ((ip = owner(1)) == NULL)
#else	UCB_SYMLINKS
	if ((ip = owner()) == NULL)
#endif	UCB_SYMLINKS
		return;
	ip->i_mode &= ~07777;
	if (u.u_uid) {
		uap->fmode &= ~ISVTX;
		/* clear set group ID bit if file group id != effective group id
		 * sysV compatible: George Mathew 6/14/85 */
		if (u.u_gid != ip->i_gid)
			uap->fmode &= ~ISGID;
	}
	ip->i_mode |= uap->fmode&07777;
	ip->i_flag |= ICHG;
	if (ip->i_flag&ITEXT && (ip->i_mode&ISVTX)==0)
		xrele(ip);
	iput(ip);
}

chown()
{
	register struct inode *ip;
	register struct a {
		char	*fname;
		int	uid;
		int	gid;
	} *uap;

	uap = (struct a *)u.u_ap;

	/* now non super users can invoke this call.
	 * sysV compatible. George Mathew 6/14/85 */
#ifdef	UCB_SYMLINKS
	if ((ip = owner(0)) == NULL)
#else	UCB_SYMLINKS
	if ((ip = owner()) == NULL)
#endif	UCB_SYMLINKS
		return;
	ip->i_uid = uap->uid;
	ip->i_gid = uap->gid;

	/* if not super user clear set-user-id and set-group-id bits
	 * SysV compatible: George Mathew 6/14/85 */
	if(u.u_uid != 0)
		ip->i_mode &= ~(ISUID|ISGID);
	ip->i_flag |= ICHG;
	iput(ip);
}

ssig()
{
	register int (*f)();
	register struct a {
		int	signo;
		int	(*fun)();
	} *uap;
	register struct proc *p = u.u_procp;
	register a;
	long sigmask;

	uap = (struct a *)u.u_ap;
	a = uap->signo & SIGNUMMASK;
	f = uap->fun;
	if (a<=0 || a>=NSIG || a==SIGKILL || a==SIGSTOP ||
	    a==SIGCONT && (f == SIG_IGN || f == SIG_HOLD)) {
		u.u_error = EINVAL;
		return;
	}
	if ((uap->signo &~ SIGNUMMASK) || (f != SIG_DFL && f != SIG_IGN &&
	    SIGISDEFER(f)))
		u.u_procp->p_flag |= SNUSIG;
	/*
	 * Don't clobber registers if we are to simulate
	 * a ret+rti.
	 */
	if ((uap->signo&SIGDORTI) == 0)
		u.u_rval1 = (int)u.u_signal[a];
	/*
	 * Change setting atomically.
	 */
	spl6();
	sigmask = 1L << (a-1);
	if (f == SIG_IGN)
		p->p_sig &= ~sigmask;		/* never to be seen again */
	u.u_signal[a] = f;
	if (f != SIG_DFL && f != SIG_IGN && f != SIG_HOLD)
		f = SIG_CATCH;
	if ((int)f & 1)
		p->p_siga0 |= sigmask;
	else
		p->p_siga0 &= ~sigmask;
	if ((int)f & 2)
		p->p_siga1 |= sigmask;
	else
		p->p_siga1 &= ~sigmask;
	spl0();
	/*
	 * Now handle options.
	 */
	if (uap->signo & SIGDOPAUSE) {
		/*
		 * Simulate a PDP11 style wait instruction which
		 * atomically lowers priority, enables interrupts
		 * and hangs.
		 */
		pause();
		/*NOTREACHED*/
	}
	if (uap->signo & SIGDORTI)
		u.u_eosys = SIMULATERTI;
}

	/* changes made to make kill() sysV compatible. GMM */
kill()
{
	register struct proc *p;
	register a, sig;
	register struct a {
		int	pid;
		int	signo;
	} *uap;
	int f, priv;

	uap = (struct a *)u.u_ap;
	f = 0;
	a = uap->pid;
	priv = 0;
	sig = uap->signo;
	if (sig < 0)
		/*
		 * A negative signal means send to process group.
		 */
		uap->signo = -uap->signo;
	if (uap->signo > NSIG) {
		u.u_error = EINVAL;
		return;
	}
	if (a > 0) { 
		if (sig > 0) {
			for(p = &proc[0]; p <= maxproc; p++)
				if (p->p_pid == a) goto found;
			p = 0;
	found:
			if (p == 0 || u.u_uid && u.u_uid != p->p_uid) {
				u.u_error = ESRCH;
				return;
			}
			psignal(p, uap->signo);
			return;
		}
	} else if (a==-1 ) {
		priv++;
		a = 0;
		sig = -1;		/* like sending to pgrp */
	} else if (a==0) {
		/*
		 * Zero process id means send to my process group.
		 */
		sig = -1;
		a = u.u_procp->p_pgrp;
		if (a == 0) {
			u.u_error = EINVAL;
			return;
		}
	} else if ( a < -1) {   /* send to all processes whose process group ID
				 * is equal to the absolute value of pid */
		sig = -1;
		a = -a;
	}
	for(p = &proc[0]; p <= maxproc; p++) {
		if (p->p_stat == NULL)
			continue;
		/*
		 * sig = 0, just checking for valid pid
		 */
		if (uap->signo == 0 && p->p_pid == uap->pid) {
			f++;
			break;
		}
		if ( sig > 0  ){
			if (p->p_pid != a)
				continue;
		/*
		 * Always skip pid 0 (swapper), 1 (init), and 2 (elc).
		 */
		} else if (p->p_pgrp!=a && priv==0 || p<=&proc[2] ||
		    (priv && p==u.u_procp))
			continue;
		if (u.u_uid != 0 && u.u_uid != p->p_uid &&
		    (uap->signo != SIGCONT || priv || !inferior(p)))
			continue;
		f++;
		if(uap->signo)
			psignal(p, uap->signo);
	}
	if(f == 0)
		u.u_error = ESRCH;
}

times()
{
	register struct a {
		time_t	(*times)[4];
	} *uap;

	uap = (struct a *)u.u_ap;
	if (copyout((caddr_t)&u.u_utime, (caddr_t)uap->times, sizeof(*uap->times)) < 0)
		u.u_error = EFAULT;
}

profil()
{
	register struct a {
		short	*bufbase;
		unsigned bufsize;
		unsigned pcoffset;
		unsigned pcscale;
	} *uap;

	uap = (struct a *)u.u_ap;
	u.u_prof.pr_base = uap->bufbase;
	u.u_prof.pr_size = uap->bufsize;
	u.u_prof.pr_off = uap->pcoffset;
	u.u_prof.pr_scale = uap->pcscale;
}

/*
 * alarm clock signal
 */
alarm()
{
	register struct proc *p;
	register c;
	register struct a {
		int	deltat;
	} *uap;

	uap = (struct a *)u.u_ap;
	p = u.u_procp;
	c = p->p_clktim;
	p->p_clktim = uap->deltat;
	u.u_rval1 = c;
}

/*
 * indefinite wait.
 * no one should wakeup(&u)
 */
pause()
{

	for(;;)
		sleep((caddr_t)&u, PSLEP);
}

/*
 * mode mask for creation of files
 */
umask()
{
	register struct a {
		int	mask;
	} *uap;
	register t;

	uap = (struct a *)u.u_ap;
	t = u.u_cmask;
	u.u_cmask = uap->mask & 0777;
	u.u_rval1 = t;
}

/*
 * Set IUPD and IACC times on file.
 * Can't set ICHG.
 */
utime()
{
	register struct a {
		char	*fname;
		time_t	*tptr;
	} *uap;
	register struct inode *ip;
	time_t tv[2];

	uap = (struct a *)u.u_ap;
#ifdef	UCB_SYMLINKS
	if ((ip = owner(1)) == NULL)
#else	UCB_SYMLINKS
	if ((ip = owner()) == NULL)
#endif	UCB_SYMLINKS
		return;
	if (copyin((caddr_t)uap->tptr, (caddr_t)tv, sizeof(tv))) {
		u.u_error = EFAULT;
	} else {
		ip->i_flag |= IACC|IUPD|ICHG;
		iupdat(ip, &tv[0], &tv[1], 0);
	}
	iput(ip);
}

ulimit()
{
	register struct a {
		int	cmd;
		long	arg;
	} *uap;
	int	j, k;

	register brk, stk;

	uap = (struct a *)u.u_ap;
	switch(uap->cmd) {
	case 2:			/* set file size limit */
		if (uap->arg > u.u_limit && !suser())
			return;
		u.u_limit = uap->arg;
	case 1:			/* get file size limit */
		u.u_r.r_off = u.u_limit;
		break;

	case 3:			/* get maximum break value */
		brk = 1024 - u.u_dsize
			- ((u.u_ssize + btoct(8*1024) - 1)/btoct(8*1024))
			 * btoct(8*1024);
/*** below is the SYSTEM V brk value calculation
		brk = 1024 - ((u.u_ssize + btoct(8*1024) - 1)/btoct(8*1024))
			 * btoct(8*1024);
***/
		u.u_r.r_off = ctob((long)brk);
		if (u.u_mbitm) {
			for(j=k=7; j>=0; j--)
				if ((u.u_mbitm) & (1 << j))
					k = j;
			u.u_r.r_off = (unsigned int)(k << 13);
		}
		break;
	}
}
