
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)acct.c	3.0	4/21/86
 */
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/acct.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/inode.h>
#include <sys/proc.h>
#include <sys/seg.h>
#include <sys/text.h>
#include <sys/lock.h>

#ifdef	ACCT

/*
 * Perform process accounting functions.
 */

sysacct()
{
	register struct inode *ip;
	register struct a {
		char	*fname;
	} *uap;

	uap = (struct a *)u.u_ap;
	if (suser()) {
		if (uap->fname==NULL) {
			if (acctp) {
				plock(acctp);
				iput(acctp);
				acctp = NULL;
			}
			return;
		}
		if (acctp) {
			u.u_error = EBUSY;
			return;
		}
#ifdef	UCB_SYMLINKS
		ip = namei(uchar, LOOKUP, 1);
#else	UCB_SYMLINKS
		ip = namei(uchar, LOOKUP);
#endif	UCB_SYMLINKS
		if(ip == NULL)
			return;
		if((ip->i_mode & IFMT) != IFREG) {
			u.u_error = EACCES;
			iput(ip);
			return;
		}
		acctp = ip;
		prele(ip);
	}
}
#endif	ACCT

/*
 * On exit, write a record on the accounting file.
 */
acct()
{
#ifdef	ACCT
	register i;
	register struct inode *ip;
	off_t siz;

	if ((ip=acctp)==NULL)
		return;
	plock(ip);
	for (i=0; i<sizeof(acctbuf.ac_comm); i++)
		acctbuf.ac_comm[i] = u.u_comm[i];
	acctbuf.ac_utime = compress(u.u_utime);
	acctbuf.ac_stime = compress(u.u_stime);
	acctbuf.ac_etime = compress(time - u.u_start);
	acctbuf.ac_btime = u.u_start;
	acctbuf.ac_uid = u.u_ruid;
	acctbuf.ac_gid = u.u_rgid;
	acctbuf.ac_mem = 0;
	acctbuf.ac_io = 0;
	acctbuf.ac_tty = u.u_ttyd;
	acctbuf.ac_flag = u.u_acflag;
	siz = ip->i_size;
	u.u_offset = siz;
	u.u_base = (caddr_t)&acctbuf;
	u.u_count = sizeof(acctbuf);
	u.u_segflg = 1;
	u.u_error = 0;
	u.u_limit = (daddr_t)5000;
	writei(ip);
	if(u.u_error)
		ip->i_size = siz;
	prele(ip);
#endif	ACCT
}

#ifdef	ACCT
/*
 * Produce a pseudo-floating point representation
 * with 3 bits base-8 exponent, 13 bits fraction.
 */
static
compress(t)
register time_t t;
{
	register exp = 0, round = 0;

	while (t >= 8192) {
		exp++;
		round = t&04;
		t >>= 3;
	}
	if (round) {
		t++;
		if (t >= 8192) {
			t >>= 3;
			exp++;
		}
	}
	return((exp<<13) + t);
}
#endif	ACCT

/*
 * lock user into core as much
 * as possible. swapping may still
 * occur if core grows.
 */

char	runlock;	/* used to tell sched when to call shuffle */

syslock()
{
	register struct proc *p;
	register struct a {
		int	flag;
	} *uap;
	register struct text *xp;

	uap = (struct a *)u.u_ap;
	if (suser()) {
		p = u.u_procp;
		xp = p->p_textp;
		switch(uap->flag) {
		case UNLOCK:		/* unlock a process */
			if (p->p_flag&(SULKMSK)) {
				punlock();
				return;
			}
			goto bad;

		case TXTLOCK:		/* lock text segment of a shared text */
			if ((p->p_flag&(SULOCKT)) || (u.u_procp->p_textp==NULL))
				goto bad;
			p->p_flag |= (SULOCKT);
			if (xp->x_lcount++ == 0) {
				runlock++;
				xp->x_ccount++;
			}
			break;

		case DATLOCK:		/* lock data segment of a shared text */
			if ((p->p_flag&(SULOCKD)) || (u.u_procp->p_textp==NULL))
				goto bad;
			p->p_flag |= (SULOCKD);
			runlock++;
			break;

	/* The v7 lock call will satisfy this case */
		case PROCLOCK:		/* lock text and data of a  process */
			if (p->p_flag&(SULKMSK))
				goto bad;
			p->p_flag |= (SULKMSK);
			if (u.u_procp->p_textp != NULL)
				if (xp->x_lcount++ == 0) {
					xp->x_ccount++;
				}
			runlock++;
			break;

		default:
			goto bad;
		}
		if (runlock==0)
			return;
		if (runout) {		/* wakeup sched to shuffle */
			runout = 0;
			wakeup((caddr_t)&runout);
		}
		sleep((caddr_t)&runlock,PZERO-1); /* shuffle will wake us up */
		return;

bad:
		u.u_error = EINVAL;
	}
}

punlock()
{
	struct text *xp;

	u.u_procp->p_flag &= ~(SULKMSK);
	if (((xp=u.u_procp->p_textp) != NULL) && ( --(xp->x_lcount) == 0))
		xccdec(xp);
	runlock++;
}
