
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)sys1.c	3.0	5/5/86
 */
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/map.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/buf.h>
#include <sys/reg.h>
#include <sys/inode.h>
#include <sys/seg.h>
#include <sys/acct.h>
#include <sys/tty.h>

int	ncargs;		/* value set in c.c */

/*
 * exec system call, with and without environments.
 */
struct execa {
	char	*fname;
	char	**argp;
	char	**envp;
};

exec()
{
	((struct execa *)u.u_ap)->envp = NULL;
	exece();
}

exece()
{
	register nc;
	register char *cp;
	register int c;
	register struct buf *bp;
	register struct execa *uap;
	int na, ne, bno, ucp, ap;
	struct inode *ip;

#ifdef	UCB_SYMLINKS
	if ((ip = namei(uchar, LOOKUP, 1)) == NULL)
#else	UCB_SYMLINKS
	if ((ip = namei(uchar, LOOKUP)) == NULL)
#endif	UCB_SYMLINKS
		return;
	bno = 0;
	bp = 0;
	if(access(ip, IEXEC))
		goto bad1;
	if((ip->i_mode & IFMT) != IFREG ||
	   (ip->i_mode & (IEXEC|(IEXEC>>3)|(IEXEC>>6))) == 0) {
		u.u_error = EACCES;
		goto bad1;
	}
	/*
	 * Collect arguments on "file" in swap space.
	 */
	na = 0;
	ne = 0;
	nc = 0;
	uap = (struct execa *)u.u_ap;
#ifndef	UCB_NKB
	if ((bno = malloc(swapmap,(ncargs+BSIZE-1)/BSIZE)) == 0)
		panic("Out of swap");
#else
	if ((bno = malloc(swapmap, ctod((int) btoc(ncargs + BSIZE)))) == 0)
		panic("Out of swap");
#endif	UCB_NKB
	if (uap->argp) for (;;) {
		ap = NULL;
		if (uap->argp) {
			ap = fuword((caddr_t)uap->argp);
			uap->argp++;
		}
		if (ap==NULL && uap->envp) {
			uap->argp = NULL;
			if ((ap = fuword((caddr_t)uap->envp)) == NULL)
				break;
			uap->envp++;
			ne++;
		}
		if (ap==NULL)
			break;
		na++;
		if(ap == -1)
			u.u_error = EFAULT;
		for (;;) {
			if (nc >= ncargs-1) {
				u.u_error = E2BIG;
				goto bad2;
			}
			if ((nc&BMASK) == 0) {
				if (bp)	 {
					mapout(bp);
					bdwrite(bp);
				}
#ifndef	UCB_NKB
				bp = getblk(swapdev, swplo+bno+(nc>>BSHIFT));
#else
				bp = getblk(swapdev,
				    dbtofsb(clrnd(swplo + bno)) + (nc >> BSHIFT));
#endif	UCB_NKB
				cp = mapin(bp);
			}
			/*
			 * -(nc|~BMASK) == BSIZE - (nc&BMASK)
			 * the first is not as obvious, but a bit faster.
			 */
			if ((c = copysin(ap, cp, -(nc|~BMASK))) < 0) {
				u.u_error = EFAULT;
				goto bad2;
			}
			ap += c;
			nc += c;
			cp += c;
			if (*(cp-1) == 0)	/* did we hit the null? */
				break;
		}
	}
	if (bp) {
		mapout(bp);
		bdwrite(bp);
	}
	bp = 0;
	nc = (nc + NBPW-1) & ~(NBPW-1);
	/*
	 * In addition to the number of characters in the argument
	 * list, we also add in space to hold the argv[] and envp[]
	 * arrays.  4/29/84 -Dave Borman
	 */
	if (getxfile(ip, nc + na*NBPW + 4*NBPW) || u.u_error)
		goto bad2;

	/*
	 * copy back arglist
	 */

	ucp = -nc - NBPW;
	ap = ucp - na*NBPW - 3*NBPW;
	u.u_ar0[R6] = ap;
	suword((caddr_t)ap, na-ne);
	nc = 0;
	for (;;) {
		ap += NBPW;
		if (na==ne) {
			suword((caddr_t)ap, 0);
			ap += NBPW;
		}
		if (--na < 0)
			break;
		suword((caddr_t)ap, ucp);
		for (;;) {
			if ((nc&BMASK) == 0) {
				if (bp) {
					mapout(bp);
					bp->b_flags |= B_AGE;
					bunhash(bp);
					bp->b_dev = NODEV;
					brelse(bp);
				}
#ifndef	UCB_NKB
				bp = bread(swapdev, swplo+bno+(nc>>BSHIFT));
#else
				bp = bread(swapdev,
				    dbtofsb(clrnd(swplo + bno)) + (nc >> BSHIFT));
#endif	UCB_NKB
				bp->b_flags &= ~B_DELWRI;
				cp = mapin(bp);
			}
			c = copysout(cp, ucp, -(nc|~BMASK));
			nc += c;
			cp += c;
			ucp += c;
			if (*(cp-1) == 0)	/* did we hit the null? */
				break;
		}
	}
	suword((caddr_t)ap, 0);
	suword((caddr_t)ucp, 0);
	if(bp) {
		mapout(bp);
		bp->b_flags |= B_AGE;
		bunhash(bp);
		bp->b_dev = NODEV;
		brelse(bp);
	}
	setregs();
bad3:
	if(bno)
#ifndef	UCB_NKB
		mfree(swapmap, (ncargs+BSIZE-1)/BSIZE, bno);
#else
		mfree(swapmap, ctod((int) btoc(ncargs + BSIZE)), bno);
#endif	UCB_NKB
bad1:
	iput(ip);
	return;
bad2:
	if (bp) {
		mapout(bp);
		bp->b_flags |= B_AGE;
		bunhash(bp);
		bp->b_dev = NODEV;
		brelse(bp);
	}
	/*
	 * Get the buffers that are still around back,
	 * and mark remove all memory of them (to keep
	 * a later sync from writing them over other data
	 * after we've relesed the swap space). 7/31/84 -Dave Borman
	 */
#ifndef	UCB_NKB
	for (nc = 0; nc < (ncargs+BSIZE-1)/BSIZE; nc++)
		if (incore(swapdev, swplo+bno+nc)) {
			bp = getblk(swapdev, swplo+bno+nc);
#else	UCB_NKB
	na = (ncargs-2)>>BSHIFT;
	for (nc = 0; nc <= na; nc++)
		if (incore(swapdev, dbtofsb(clrnd(swplo+bno)) + nc)) {
			bp = getblk(swapdev, dbtofsb(clrnd(swplo+bno)) + nc);
#endif	UCB_NKB
			bp->b_flags &= ~B_DELWRI;
			bp->b_flags |= B_AGE;
			bunhash(bp);
			bp->b_dev = NODEV;
			brelse(bp);
		}
	goto bad3;
}

/*
 * Read in and set up memory for executed file.
 * Zero return is normal;
 * non-zero means only the text is being replaced
 */
static
getxfile(ip, nargc)
register struct inode *ip;
{
	register unsigned ds;
	register sep;
	register unsigned ts, ss;
	register i, overlay;
	register ovflag, ovmax;
	struct u_ovd sovdata;
	unsigned ovhead[8];
	long lsize;
	int totsize;  

	/*
	 * read in first few bytes
	 * of file for segment
	 * sizes:
	 * ux_mag = 407/410/411/405
	 *  407 is plain executable
	 *  410 is RO text
	 *  411 is separated ID
	 *  405 is overlaid text
	 *  430 is non-sep auto-overlay
	 *  431 is sep i/d auto-overlay
	 */

	if (u.u_procp->p_flag & (SULOCKT|SULOCKD))
		punlock();
	u.u_base = (caddr_t)&u.u_exdata;
	u.u_count = sizeof(u.u_exdata);
	u.u_offset = 0;
	u.u_segflg = 1;
	readi(ip);
	u.u_segflg = 0;
	if(u.u_error)
		goto bad;
	if (u.u_count!=0) {
		u.u_error = ENOEXEC;
		goto bad;
	}
	sep = 0;
	overlay = 0;
	ovflag = 0;
	if(u.u_exdata.ux_mag == 0407) {
		lsize = (long)u.u_exdata.ux_dsize + u.u_exdata.ux_tsize;
		u.u_exdata.ux_dsize = lsize;
		if (lsize != u.u_exdata.ux_dsize) {	/* check overflow */
			u.u_error = ENOMEM;
			goto bad;
		}
		u.u_exdata.ux_tsize = 0;
	} else if (u.u_exdata.ux_mag == 0411)
		sep++;
	else if (u.u_exdata.ux_mag == 0405)
		overlay++;
	else if (u.u_exdata.ux_mag == 0430)
		ovflag++;
	else if (u.u_exdata.ux_mag == 0431){
		sep++;
		ovflag++;
	}
	else if (u.u_exdata.ux_mag != 0410) {
		u.u_error = ENOEXEC;
		goto bad;
	}
/*
	if(u.u_exdata.ux_tsize!=0 && (ip->i_flag&ITEXT)==0 && ip->i_count!=1) {
*/
	if((u.u_exdata.ux_tsize!=0 && (ip->i_flag&ITEXT)==0 && ip->i_count!=1)
	    && (u.u_procp->p_flag & STRC) == 0) {
		u.u_error = ETXTBSY;
		goto bad;
	}

	/*
	 * find text and data sizes
	 * try them out for possible
	 * overflow of max sizes
	 */
	ts = btoc(u.u_exdata.ux_tsize);
	lsize = (long)u.u_exdata.ux_dsize + u.u_exdata.ux_bsize;
	if (lsize != (unsigned)lsize) {
		u.u_error = ENOMEM;
		goto bad;
	}
	ds = btoc(lsize);

	ss = SSIZE + btoc(nargc);
	/*
	 * if auto overlay get second header
	 */
	sovdata = u.u_ovdata;
	u.u_ovdata.uo_ovbase = 0;
	u.u_ovdata.uo_curov = 0;

	if(ovflag){
		u.u_base = (caddr_t) ovhead;
		u.u_count = sizeof(ovhead);
		u.u_offset = sizeof(u.u_exdata);
		u.u_segflg = 1;
		readi(ip);
		u.u_segflg = 0;
		if(u.u_count != 0)
			u.u_error = ENOEXEC;
		if(u.u_error){
			u.u_ovdata = sovdata;
			goto bad;
		}
		/*
		 * set beginning of overlay address space
		 */
		u.u_ovdata.uo_ovbase = ts;

		/*
		 * 0 entry is max size of a given overlay
		 */
		ovmax = btoc(ovhead[0]);

		/*
		 * set max number of segment registers to be used
		 */
		u.u_ovdata.uo_nseg = ctos(ovmax);

		/*
		 * set base of data space
		 */
		u.u_ovdata.uo_dbase = ts + ovmax;

		/*
		 * Setup a table of offsets to each of the
		 * overlay segements.  The i'th overlay runs
		 * from ov_offst[i-1] to ov_offst[i].
		 */
		if (u.u_exdata.ux_mag == 0430)

			/* the number 700 was found to be the optimum value
			 * (safe??) to avoid any currently supported utility
			 * from hanging. G.Mathew */

			totsize = ds+USIZE+ss+ts + 700;  
		for(i = 0; i < 8; i++){
			register t;
			if(i != 0)
				/*
				 * check if any overlay is larger that ovmax
				 */
				if((t = btoc(ovhead[i])) > ovmax){
					u.u_error = ENOEXEC;
					u.u_ovdata = sovdata;
					goto bad;
				}
				else {
					u.u_ovdata.uo_ov_offst[i] = t + u.u_ovdata.uo_ov_offst[i-1];
					if (u.u_exdata.ux_mag == 0430)
						totsize += btoc(ovhead[i]); 
				}
			else
				u.u_ovdata.uo_ov_offst[i] = ts;
		}
		if (u.u_exdata.ux_mag == 0430) {
			if ( (totsize) >= usermem) {
				u.u_error = ENOMEM;
				goto bad;
			}  
		}  
	}
	if (overlay) {
		if (u.u_sep==0 && ctos(ts) != ctos(u.u_tsize) || nargc) {
			u.u_error = ENOMEM;
			u.u_ovdata = sovdata;
			goto bad;
		}
		ss = u.u_ssize;
		sep = u.u_sep;
		xfree();
		xalloc(ip);
		u.u_ar0[PC] = u.u_exdata.ux_entloc & ~01;
	} else {
		if(estabur(ts, ds, ss, sep, RO))
		{
			u.u_ovdata = sovdata;
			goto bad;
		}

		/*
		 * allocate and clear core
		 * at this point, committed
		 * to the new image
		 */
	
		u.u_prof.pr_scale = 0;
		xfree();
		u.u_dsize = ds;
		i = USIZE+ds+ss;

		expand(i);
#ifdef	OLDCOPY
		while(--i >= USIZE)
			clearseg(u.u_procp->p_addr+i);
#else	OLDCOPY
		ds = USIZE + ((u.u_exdata.ux_dsize>>6)&01777);
		clear(u.u_procp->p_addr + ds, i - ds);
#endif	OLDCOPY
		xalloc(ip);
	
		/*
		 * read in data segment
		 */
	
		estabur((unsigned)0, u.u_dsize, (unsigned)0, 0, RO);
		u.u_base = 0;
		u.u_offset = sizeof(u.u_exdata);
		if(ovflag){
			u.u_offset += sizeof(ovhead);
			u.u_offset += (((long)u.u_ovdata.uo_ov_offst[7])<<6);
		}
		else
			u.u_offset += u.u_exdata.ux_tsize;
		u.u_count = u.u_exdata.ux_dsize;
		readi(ip);
		/*
		 * set SUID/SGID protections, if no tracing
		 */
		if ((u.u_procp->p_flag&STRC)==0) {
			if(ip->i_mode&ISUID)
				if(u.u_uid != 0) {
					u.u_uid = ip->i_uid;
					u.u_procp->p_uid = ip->i_uid;
				}
			if(ip->i_mode&ISGID)
				u.u_gid = ip->i_gid;
		} else
			psignal(u.u_procp, SIGTRAP);
	}
	u.u_tsize = ts;
	u.u_ssize = ss;
	u.u_sep = sep;
	estabur(ts, u.u_dsize, ss, sep, RO);
bad:
	return(overlay);
}

/*
 * Clear registers on exec
 */
static
setregs()
{
	register int (**rp)();
	long sigmask;
	register char *cp;
	register i;

	u.u_procp->p_flag &= ~SNUSIG;
	for(rp = &u.u_signal[1], sigmask = 1L; rp < &u.u_signal[NSIG];
	    sigmask <<=1, rp++) {
		switch (*rp) {

		case SIG_HOLD:
			u.u_procp->p_flag |= SNUSIG;
			continue;
		case SIG_IGN:
		case SIG_DFL:
			continue;

		default:
			/*
			 * Normal or deferring catch; revert to default.
			 */
			spl6();
			*rp = SIG_DFL;
			if ((int)SIG_DFL & 1)
				u.u_procp->p_siga0 |= sigmask;
			else
				u.u_procp->p_siga0 &= ~sigmask;
			if ((int)SIG_DFL & 2)
				u.u_procp->p_siga1 |= sigmask;
			else
				u.u_procp->p_siga1 &= ~sigmask;
			spl0();
			continue;
		}
	}
	for(cp = &regloc[0]; cp < &regloc[6];)
		u.u_ar0[*cp++] = 0;
	u.u_ar0[PC] = u.u_exdata.ux_entloc & ~01;
	for(rp = (int *)&u.u_fps; rp < (int *)&u.u_fps.u_fpregs[6];)
		*rp++ = 0;
	for(i=0; i<NOFILE; i++) {
		if (u.u_pofile[i]&EXCLOSE) {
#ifndef	UCB_NET
			closef(u.u_ofile[i]);
#else	UCB_NET
			closef(u.u_ofile[i], 1);
#endif	UCB_NET
			u.u_ofile[i] = NULL;
			u.u_pofile[i] &= ~EXCLOSE;
		}
	}
	/*
	 * Remember file name for accounting.
	 */
	u.u_acflag &= ~AFORK;
	bcopy((caddr_t)u.u_dbuf, (caddr_t)u.u_comm, DIRSIZ);
}

/*
 * exit system call:
 * pass back caller's arg
 */
rexit()
{
	register struct a {
		int	rval;
	} *uap;

	uap = (struct a *)u.u_ap;
	exit((uap->rval & 0377) << 8);
}

/*
 * Release resources.
 * Save u. area for parent to look at.
 * Enter zombie state.
 * Wake up parent and init processes,
 * and dispose of children.
 */
exit(rv)
{
	register int i;
	register struct proc *p, *q;
#ifdef	NEWLIMITS
	extern int cnl;
#endif	NEWLIMITS

	p = u.u_procp;
	p->p_flag &= ~(STRC|SULKMSK);
	p->p_clktim = 0;
	spl6();
	if (((int)SIG_IGN) & 1)
		p->p_siga0 = ~0L;
	else
		p->p_siga0 = 0L;
	if (((int)SIG_IGN) & 2)
		p->p_siga1 = ~0L;
	else
		p->p_siga1 = 0L;
	spl0();
	for(i=0; i<NSIG; i++)
		u.u_signal[i] = 1;

	/* send SIGHUP to each process that has process group ID equal to that of
	 * the calling process if the process ID, tty group ID and process group
	 * ID are equal: George Mathew 7/2/85 */

/* REMOVED THE ABOVE MENTIONED CHANGES DUE TO PROBLEMS WHILE USING PIPES! */

	/* if ((p->p_pid == p->p_pgrp)    
	 && (u.u_ttyp != NULL)
	 && (u.u_ttyp->t_pgrp == p->p_pgrp)) {
		u.u_ttyp->t_pgrp = 0;
		gsignal(p->p_pgrp, SIGHUP); 
	} */

	for(i=0; i<NOFILE; i++)
		if(u.u_ofile[i] != NULL)
#ifndef	UCB_NET
			closef(u.u_ofile[i]);
#else	UCB_NET
			closef(u.u_ofile[i], 1);
#endif	UCB_NET
	semexit();
	if (u.u_procp->p_flag & (SULOCKT|SULOCKD))
		punlock();
	plock(u.u_cdir);
	iput(u.u_cdir);
	if (u.u_rdir) {
		plock(u.u_rdir);
		iput(u.u_rdir);
	}
	xfree();
#ifdef	NEWLIMITS
	if (u.u_acflag & ARESV)
		if (cnl%131)	/* don't go below zero! */
			--cnl;
#endif	NEWLIMITS
	acct();
	mfree(coremap, p->p_size, p->p_addr);
	eflgcln(p);		/* clean up eventflags */
	p->p_stat = SZOMB;
	if (p->p_pid == 1)
		panic("init died");
	((struct xproc *)p)->xp_xstat = rv;
	((struct xproc *)p)->xp_utime = u.u_cutime + u.u_utime;
	((struct xproc *)p)->xp_stime = u.u_cstime + u.u_stime;
	for (q = &proc[0]; q <= maxproc; q++)
		if (q->p_pptr == p) {
			q->p_pptr = &proc[1];
			q->p_ppid = 1;
			wakeup((caddr_t)&proc[1]);
			/*
			 * Traced processes are killed
			 * since their existence means someone is screwing up.
			 * Stopped processes are sent a hangup and a continue;
			 * This is designed to be ``save'' for setuid
			 * processes since they must be willing to tolerate
			 * hangups anyways.
			 */
			if (q->p_flag&STRC) {
				q->p_flag &= ~STRC;
				psignal(q, SIGKILL);
			} else if (q->p_stat == SSTOP) {
				psignal(q, SIGHUP);
				psignal(q, SIGCONT);
			}
			/*
			 * Protect this process from future
			 * tty signals, clear TSTP/TTIN/TTOU if pending,
			 * and set SDETACH bit on procs.
			 */
			spgrp(q, -1);
		}
	wakeup((caddr_t)p->p_pptr);
	psignal(p->p_pptr, SIGCHLD);
	swtch();
}

/*
 * Wait system call.
 * Search for a terminated (zombie) child,
 * finally lay it to rest, and collect its status.
 * Look also for stopped (traced) children,
 * and pass back status from them.
 */
wait()
{
	register f;
	register struct proc *p;
	register options;

	options = (u.u_ar0[RPS] & PS_ALLCC) == PS_ALLCC ? u.u_ar0[R0] : 0;
	f = 0;

loop:
	/*
	 * No one will ever wait for swapper (slot 0).  No one will
	 * ever wait for init (slot 1).  No one (i.e., init) should
	 * ever wait for elc.  If init dies we'll panic elsewere,
	 * if elc dies we need to keep the slot and pid around forever
	 * because other parts of the kernel always skip it.
	 *	-Dave Borman 10/19/85
	 */
	for(p = &proc[3]; p <= maxproc; p++)
	if (p->p_pptr == u.u_procp)
	{
		f++;
		if(p->p_stat == SZOMB) {
			u.u_rval1 = p->p_pid;
			u.u_rval2 = ((struct xproc *)p)->xp_xstat;
			((struct xproc *)p)->xp_xstat = 0;
			p->p_pptr = 0;
			p->p_siga0 = 0L;
			p->p_siga1 = 0L;
			p->p_cursig = 0;
			u.u_cutime += ((struct xproc *)p)->xp_utime;
			u.u_cstime += ((struct xproc *)p)->xp_stime;
			p->p_pid = 0;
			p->p_ppid = 0;
			p->p_pgrp = 0;
			p->p_sig = 0;
			p->p_flag = 0;
			p->p_wchan = 0;
			p->p_stat = NULL;
			if (p == maxproc)
				while (maxproc->p_stat == NULL)
					maxproc--;
			return;
		}
		if(p->p_stat == SSTOP && (p->p_flag&SWTED) == 0 &&
		  (p->p_flag & STRC || options & 2)) {
			p->p_flag |= SWTED;
			u.u_rval1 = p->p_pid;
			u.u_rval2 = (p->p_cursig << 8) | 0177;
			return;
		}
	}
	if(f) {
		if (options & 1) {
			u.u_rval1 = 0;
			return;
		} else {
			if ((u.u_procp->p_flag & SNUSIG) && save(u.u_qsav)) {
				u.u_eosys = RESTARTSYS;
				return;
			}
			sleep((caddr_t)u.u_procp, PWAIT);
			goto loop;
		}
	}
	u.u_error = ECHILD;
}

/*
 * fork system call.
 */
fork()
{
	register struct proc *p1, *p2;
	register a;
	extern int maxuprc;

	/*
	 * Make sure there's enough swap space for max
	 * core image, thus reducing chances of running out
	 */
	if ((a = malloc(swapmap, ctod(MAXMEM))) == 0) {
		u.u_error = ENOMEM;
		goto out;
	}
	mfree(swapmap, ctod(MAXMEM), a);
	a = 0;
	p2 = NULL;
	for(p1 = &proc[0]; p1 < &proc[nproc]; p1++) {
		if (p1->p_stat==NULL && p2==NULL)
			p2 = p1;
		else {
			if (p1->p_uid==u.u_uid && p1->p_stat!=NULL)
				a++;
		}
	}
	/*
	 * Disallow if
	 *  No processes at all;
	 *  not su and too many procs owned; or
	 *  not su and would take last slot.
	 */
	if (p2==NULL || (u.u_uid!=0 && (p2==&proc[nproc-1] || a>maxuprc))) {
		u.u_error = EAGAIN;
		goto out;
	}
	p1 = u.u_procp;
	if(newproc()) {
		u.u_rval1 = p1->p_pid;
		u.u_start = time;
		u.u_cstime = 0;
		u.u_stime = 0;
		u.u_cutime = 0;
		u.u_utime = 0;
		u.u_acflag = AFORK;
		return;
	}
	u.u_rval1 = p2->p_pid;

out:
	u.u_ar0[R7] += NBPW;
}

/*
 * break system call.
 *  -- bad planning: "break" is a dirty word in C.
 */
sbreak()
{
	struct a {
		char	*nsiz;
	};
	register a, n, d;
#ifdef	OLDCOPY
	int i;
#endif	OLDCOPY

	/*
	 * set n to new data size
	 * set d to new-old
	 * set n to new total size
	 */

	n = btoc((int)((struct a *)u.u_ap)->nsiz);
	if(!u.u_sep)
		if(u.u_ovdata.uo_ovbase)
			n -= ctos((unsigned)u.u_ovdata.uo_dbase) * stoc(1);
		else
			n -= ctos(u.u_tsize) * stoc(1);
	if(n < 0)
		n = 0;
	d = n - u.u_dsize;
	n += USIZE+u.u_ssize;
	if(estabur(u.u_tsize, u.u_dsize+d, u.u_ssize, u.u_sep, RO))
		return;
	u.u_dsize += d;
	if(d > 0)
		goto bigger;
	a = u.u_procp->p_addr + n - u.u_ssize;
#ifdef	OLDCOPY
	i = n;
	n = u.u_ssize;
	while(n--) {
		copyseg(a-d, a);
		a++;
	}
	expand(i);
#else	OLDCOPY
	copy(a-d, a, u.u_ssize);	/* d is negative */
	expand(n);
#endif	OLDCOPY
	return;

bigger:
	expand(n);
#ifdef	OLDCOPY
	a = u.u_procp->p_addr + n;
	n = u.u_ssize;
	while(n--) {
		a--;
		copyseg(a-d, a);
	}
	while(d--)
		clearseg(--a);
#else	OLDCOPY
	a = u.u_procp->p_addr + n - u.u_ssize - d;
	n = u.u_ssize;
	while (n >= d) {
		n -= d;
		copy(a+n, a+n+d, d);
	}
	copy(a, a+d, n);
	clear(a, d);
#endif	OLDCOPY
}
