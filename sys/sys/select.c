
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)select.c	3.0	4/21/86
 */
#include	<sys/param.h>
#include	<sys/dir.h>
#include	<sys/proc.h>
#include	<sys/user.h>
#include	<sys/systm.h>
#include	<sys/inode.h>
#include	<sys/file.h>
#include	<sys/stat.h>
#include	<sys/conf.h>
#include	<sys/seg.h>
#define		NBBY	8	/* number of bits to a byte */

int nselcoll;

select()
{
	register struct uap {
		int	nfd;
		fd_set *rp, *wp;
		int	*tp;
	}		*ap = (struct uap *)u.u_ap;
	fd_set		rd, wr;
	int		nfds = 0;
	long		selscan();
	long		readable = 0, writeable = 0;
	int		timo = 0;
	time_t		t = time;
	int		s, tsel, ncoll, rem;

	if (ap->nfd > NOFILE)
		ap->nfd = NOFILE;
	if (ap->nfd < 0) {
		u.u_error = EBADF;
		return;
	}
	if (ap->rp && copyin((caddr_t)ap->rp, (caddr_t)&rd, sizeof(fd_set)))
		return;
	if (ap->wp && copyin((caddr_t)ap->wp, (caddr_t)&wr, sizeof(fd_set)))
		return;
	if (ap->tp && copyin((caddr_t)ap->tp, (caddr_t)&timo, sizeof(timo)))
		return;
retry:
	ncoll = nselcoll;
	u.u_procp->p_flag |= SSEL;
	if (ap->rp)
		readable = selscan(ap->nfd, rd, &nfds, FREAD);
	if (ap->wp)
		writeable = selscan(ap->nfd, wr, &nfds, FWRITE);
	if (u.u_error)
		goto done;
	if (readable || writeable)
		goto done;
	if (ap->tp)
		if (timo == 0 || (rem = timo - (time - t)) <= 0)
			goto done;
	s = spl6();
	if ((u.u_procp->p_flag & SSEL) == 0 || nselcoll != ncoll) {
		u.u_procp->p_flag &= ~SSEL;
		splx(s);
		goto retry;
	}
	u.u_procp->p_flag &= ~SSEL;
	if (ap->tp) {
		tsel = tsleep((caddr_t)&selwait, PZERO + 1, rem);
		splx(s);
		switch (tsel) {
		case TS_OK:
			goto retry;
		case TS_SIG:
			u.u_error = EINTR;
			return;
		case TS_TIME:
			break;
		}
	} else {
		sleep((caddr_t)&selwait, PZERO + 1);
		splx(s);
		goto retry;
	}
done:
	rd.fds_bits[0] = readable;
	wr.fds_bits[0] = writeable;
	s = sizeof (fd_set);
	if (s * NBBY < ap->nfd)
		s = (ap->nfd + NBBY - 1) / NBBY;
	u.u_r.r_val1 = nfds;
	if (ap->rp)
		copyout((caddr_t)&rd, (caddr_t)ap->rp, sizeof(fd_set));
	if (ap->wp)
		copyout((caddr_t)&wr, (caddr_t)ap->wp, sizeof(fd_set));
}

static
long selscan(nfd, fds, nfdp, flag)
int	nfd;
fd_set	fds;
int	*nfdp, flag;
{
	struct file	*fp;
	struct inode	*ip;
	long		bits, res = 0;
	int		i, able;

	bits = fds.fds_bits[0];
	while (i = ffs(bits)) {
		if (i > nfd)
			break;
		bits &= ~(1L << (i - 1));
		fp = u.u_ofile[i-1];
		if (fp == NULL) {
			u.u_error = EBADF;
			return(0);
		}
#ifdef	UCB_NET
		if (fp->f_flag & FSOCKET)
			able = soo_select(fp->f_socket, flag);
		else
#endif	UCB_NET
		switch ((ip = fp->f_inode)->i_mode & IFMT) {
		case IFCHR:
		    {
			register dev_t dev = ip->i_rdev;
			able = (*cdevsw[major(dev)].d_select)(dev, flag);
			break;
		    }
		case IFIFO:
			able = pipeselect(ip, flag);
			break;
		case IFBLK:
		case IFREG:
		case IFDIR:
			able = 1;
			break;
		}
		if (able) {
			res |= (1L << (i - 1));
			(*nfdp)++;
		}
	}
	return (res);
}

static
ffs(mask)
long	mask;
{
	register int	i;
	register int	imask;

	if (mask == 0)
		return(0);

	imask = loint(mask);
	for (i = 1; i < 16; i++) {
		if (imask & 1)
			return (i);
		imask >>= 1;
	}
	imask = hiint(mask);
	for (; i<=32; i++) {
		if (imask & 1)
			return (i);
		imask >>= 1;
	}
	return (0);	/* can't get here anyway! */
}

selwakeup(p, coll)
register struct proc	*p;
int			coll;
{
	register int	s;
	extern int	selwait;

	if (coll) {
		nselcoll++;
		wakeup((caddr_t)&selwait);
	}
	s = spl6();
	if (p) {
		segm	map;
		/*
		 * We can get called with ka5 not pointing to the
		 * normal seg5.  Since we know we'll be looking at the
		 * proc table, we have to remap ourselves.
		 */
		saveseg5(map);
		normalseg5();
		if (p->p_wchan == (caddr_t)&selwait)
			setrun(p);
		else if (p->p_flag & SSEL)
			p->p_flag &= ~SSEL;
		restorseg5(map);
	}
	splx(s);
}

seltrue(dev, flag)
dev_t	dev;
int	flag;
{
	return(1);
}

static
tsleep(chan, pri, seconds)
caddr_t	chan;
int	pri, seconds;
{
	struct sqsav {
		label_t sq_t;
	}			lqsav;
	register struct proc	*pp;
	register		sec, n, rval;

	pp = u.u_procp;
	n = spl7();
	sec = 0;
	rval = 0;
	if (pp->p_clktim && pp->p_clktim < seconds)
		seconds = 0;
	if (seconds) {
		pp->p_flag |= STIMO;
		sec = pp->p_clktim - seconds;
		pp->p_clktim = seconds;
	}
	lqsav = *(struct sqsav *)&(u.u_qsav);
	if (save(u.u_qsav))
		rval = TS_SIG;
	else {
		sleep(chan, pri);
		if ((pp->p_flag & STIMO) == 0 && seconds)
			rval = TS_TIME;
		else
			rval = TS_OK;
	}
	pp->p_flag &= ~STIMO;
	*(struct sqsav *)&(u.u_qsav) = lqsav;
	if (sec > 0)
		pp->p_clktim += sec;
	else
		pp->p_clktim = 0;
	splx(n);
	return(rval);
}
