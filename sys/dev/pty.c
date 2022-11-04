
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * Sccsid: @(#)pty.c	3.0	4/21/86
 * based on
 *	pty.c	4.21	82/03/23
 */

/*
 * Pseudo-teletype Driver
 * (Actually two drivers, requiring two entries in 'cdevsw')
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/tty.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/conf.h>
#include <sys/buf.h>
#include <sys/file.h>
#include <sys/proc.h>
#include <sys/seg.h>

#define BUFSIZ 100		/* Chunk size iomoved from user */

extern int npty;
extern int hz;

/*
 * pts == /dev/tty[pP]?
 * ptc == /dev/pty[pP]?
 */

#ifndef	SELECT
#define	ptcwakeup(tp) wakeup((caddr_t)&tp->t_outq.c_cf)
#endif

extern	struct tty *pt_tty[];
extern	struct pt_ioctl pt_ioctl[];

#define	PF_RCOLL	0x01
#define	PF_WCOLL	0x02
#define	PF_NBIO		0x04
#define	PF_PKT		0x08		/* packet mode */
#define	PF_STOPPED	0x10		/* user told stopped */
#define	PF_REMOTE	0x20		/* remote and flow controlled input */
#define	PF_NOSTOP	0x40
#define PF_WTIMER       0x80            /* waiting for timer to flush */

/*ARGSUSED*/
ptsopen(dev, flag)
	dev_t dev;
{
	register struct tty *tp;

	if (minor(dev) >= npty) {
		u.u_error = ENXIO;
		return;
	}
	tp = pt_tty[minor(dev)];
	if ((tp->t_state & ISOPEN) == 0) {
		ttychars(tp);		/* Set up default chars */
		tp->t_flags = 0;	/* No features (nor raw mode) */
		tp->t_line = DFLT_LDISC; /* Default line discipline */
		tp->t_ispeed = B9600;	/* Arbitrary	*/
		tp->t_ospeed = B9600;
	} else if (tp->t_state&XCLUDE && u.u_uid != 0) {
		u.u_error = EBUSY;
		return;
	}
	if (tp->t_oproc)			/* Ctrlr still around. */
		tp->t_state |= CARR_ON;
	if((flag&FNDELAY) == 0)
		while ((tp->t_state & CARR_ON) == 0) {
			tp->t_state |= WOPEN;
			sleep((caddr_t)&tp->t_rawq, TTIPRI);
		}
	ttyopen(dev,tp);
}

ptsclose(dev)
	dev_t dev;
{
	register struct tty *tp;

	tp = pt_tty[minor(dev)];
	(*linesw[tp->t_line].l_close)(tp);	/* DaveR */
	ttyclose(tp);
	ptcwakeup(tp);				/* DaveR */
}

ptsread(dev)
	dev_t dev;
{
	register struct tty *tp = pt_tty[minor(dev)];
	register struct pt_ioctl *pti = &pt_ioctl[minor(dev)];

again:
	if (pti->pt_flags & PF_REMOTE) {
		while (tp == u.u_ttyp && u.u_procp->p_pgrp != tp->t_pgrp) {
			if (u.u_signal[SIGTTIN] == SIG_IGN
			    || u.u_signal[SIGTTIN] == SIG_HOLD
	/*
			    || (u.u_procp->p_flag&SDETACH)
	*/
#ifdef	VFORK
			    || u.u_procp->p_flag&SVFORK
#endif
				)
				return;
			gsignal(u.u_procp->p_pgrp, SIGTTIN);
			sleep((caddr_t)&lbolt, TTIPRI);
		}
		if (tp->t_rawq.c_cc == 0) {
			if (tp->t_state & TS_NBIO) {
				u.u_error = EWOULDBLOCK;
				return;
			}
			sleep((caddr_t)&tp->t_rawq, TTIPRI);
			goto again;
		}
		while (tp->t_rawq.c_cc > 1 && passc(getc(&tp->t_rawq)) >= 0)
			;
		if (tp->t_rawq.c_cc == 1)
			(void) getc(&tp->t_rawq);
		if (tp->t_rawq.c_cc)
			return;
	} else
		if (tp->t_oproc)
			(*linesw[tp->t_line].l_read)(tp);
	wakeup((caddr_t)&tp->t_rawq.c_cf);
#ifdef	SELECT
	if (pti->pt_selw) {
		selwakeup(pti->pt_selw, pti->pt_flags & PF_WCOLL);
		pti->pt_selw = 0;
		pti->pt_flags &= ~PF_WCOLL;
	}
#endif
}

/*
 * Write to pseudo-tty.
 * Wakeups of controlling tty will happen
 * indirectly, when tty driver calls ptsstart.
 */
ptswrite(dev)
	dev_t dev;
{
	register struct tty *tp;

	tp = pt_tty[minor(dev)];
	if (tp->t_oproc)
		(*linesw[tp->t_line].l_write)(tp);
}

/*
 * Start output on pseudo-tty.
 * Wake up process selecting or sleeping for input from controlling tty.
 */
ptsstart(tp)
	struct tty *tp;
{
	register struct pt_ioctl *pti = &pt_ioctl[minor(tp->t_dev)];

	if (tp->t_state & TTSTOP)
		return;
	if (pti->pt_flags & PF_STOPPED) {
		pti->pt_flags &= ~PF_STOPPED;
		pti->pt_send = TIOCPKT_START;
	}
	if (tp->t_outq.c_cc < 200) {
		pti->pt_flags |= PF_WTIMER;
		return;
	}
	pti->pt_flags &= ~PF_WTIMER;
	ptcwakeup(tp);
}

#ifdef	SELECT
ptcwakeup(tp)
	struct tty *tp;
{
	struct pt_ioctl *pti = &pt_ioctl[minor(tp->t_dev)];
	int s = spl5();         /* any NZ spl will lockout ptctimer */

	if (pti->pt_selr) {
		selwakeup(pti->pt_selr, pti->pt_flags & PF_RCOLL);
		pti->pt_selr = 0;
		pti->pt_flags &= ~PF_RCOLL;
	}
	wakeup((caddr_t)&tp->t_outq.c_cf);
out:
	splx(s);
}
#endif

ptctimer()
{
	register struct tty *tp = pt_tty[0];
	register struct pt_ioctl *pti = &pt_ioctl[0];
	register i;

	timeout(&ptctimer, 0, hz/10);
	for (i=0; i<npty; i++, pti++, tp++) {
		if ((pti->pt_flags & PF_WTIMER) == 0)
			continue;
		pti->pt_flags &= ~PF_WTIMER;
		if (tp->t_oproc == 0)
			continue;
		ptcwakeup(tp);
	}
}

/*ARGSUSED*/
ptcopen(dev, flag)
	dev_t dev;
	int flag;
{
	register struct tty *tp;
	struct pt_ioctl *pti;
	static first;

	if (first == 0) {
		first++;
		ptctimer();
	}
	if (minor(dev) >= npty) {
		u.u_error = ENXIO;
		return;
	}
	tp = pt_tty[minor(dev)];
	if (tp->t_oproc) {
		u.u_error = EIO;
		return;
	}
	tp->t_iproc = NULL;
	tp->t_oproc = ptsstart;
	if (tp->t_state & WOPEN)
		wakeup((caddr_t)&tp->t_rawq);
	tp->t_state |= CARR_ON;
	pti = &pt_ioctl[minor(dev)];
	pti->pt_flags = 0;
	pti->pt_send = 0;
}

ptcclose(dev)
	dev_t dev;
{
	register struct tty *tp;

	tp = pt_tty[minor(dev)];
	if (tp->t_state & ISOPEN)
		gsignal(tp->t_pgrp, SIGHUP);
	tp->t_state &= ~CARR_ON;	/* virtual carrier gone */
	flushtty(tp, FREAD|FWRITE);
	tp->t_oproc = 0;		/* mark closed */
}

ptcread(dev)
	dev_t dev;
{
	register struct tty *tp;
	struct pt_ioctl *pti;

	tp = pt_tty[minor(dev)];
	if ((tp->t_state&(CARR_ON|ISOPEN)) == 0) 
		return;
	pti = &pt_ioctl[minor(dev)];
	if (pti->pt_flags & PF_PKT) {
		if (pti->pt_send) {
			(void) passc(pti->pt_send);
			pti->pt_send = 0;
			return;
		}
		(void) passc(0);
	}
	while (tp->t_outq.c_cc == 0 || (tp->t_state&TTSTOP)) {
		if (pti->pt_flags&PF_NBIO) {
			u.u_error = EWOULDBLOCK;
			return;
		}
		sleep((caddr_t)&tp->t_outq.c_cf, TTIPRI);
	}
	while (tp->t_outq.c_cc && passc(getc(&tp->t_outq)) >= 0)
		;
	if (tp->t_outq.c_cc <= TTLOWAT(tp)) {
		if (tp->t_state&ASLEEP) {
			tp->t_state &= ~ASLEEP;
			wakeup((caddr_t)&tp->t_outq);
		}
#ifdef	SELECT
		if (tp->t_wsel) {
			selwakeup(tp->t_wsel, tp->t_state & TS_WCOLL);
			tp->t_wsel = 0;
			tp->t_state &= ~TS_WCOLL;
		}
#endif	SELECT
	}
}

ptsstop(tp, flush)
	register struct tty *tp;
	int flush;
{
	struct pt_ioctl *pti = &pt_ioctl[minor(tp->t_dev)];

	/* note: FLUSHREAD and FLUSHWRITE already ok */
	if (flush == 0) {
		flush = TIOCPKT_STOP;
		pti->pt_flags |= PF_STOPPED;
	} else {
		pti->pt_flags &= ~PF_STOPPED;
	}
	pti->pt_send |= flush;
	ptcwakeup(tp);
}

#ifdef	SELECT
ptcselect(dev, rw)
	dev_t dev;
	int rw;
{
	register struct tty *tp = pt_tty[minor(dev)];
	struct pt_ioctl *pti = &pt_ioctl[minor(dev)];
	struct proc *p;
	int s;

	if ((tp->t_state&(CARR_ON|ISOPEN)) == 0)
		return (1);
	s = spl5();
	switch (rw) {

	case FREAD:
		if (tp->t_outq.c_cc && (tp->t_state&TTSTOP) == 0) {
			splx(s);
			return (1);
		}
		if ((p = pti->pt_selr) && p->p_wchan == (caddr_t)&selwait)
			pti->pt_flags |= PF_RCOLL;
		else
			pti->pt_selr = u.u_procp;
		break;

	case FWRITE:
		if ((pti->pt_flags & PF_REMOTE) == 0 || tp->t_rawq.c_cc == 0) {
			splx(s);
			return (1);
		}
		if ((p = pti->pt_selw) && p->p_wchan == (caddr_t)&selwait)
			pti->pt_flags |= PF_WCOLL;
		else
			pti->pt_selw = u.u_procp;
		break;
	}
	splx(s);
	return (0);
}
#endif	SELECT

ptcwrite(dev)
	dev_t dev;
{
	register struct tty *tp;
	register char *cp, *ce;
	register int cc;
	char locbuf[BUFSIZ];
	int cnt = 0;
	struct pt_ioctl *pti = &pt_ioctl[minor(dev)];

	tp = pt_tty[minor(dev)];
	if ((tp->t_state&(CARR_ON|ISOPEN)) == 0)
		return;
	do {
		cc = MIN(u.u_count, BUFSIZ);
		cp = locbuf;
#ifdef	notdef
		iomove(cp, (unsigned)cc, B_WRITE);
#else	notdef
		{
			long paddr;
			paddr = (((long)(*KDSA6))<<6L) +
					(long)(((unsigned)cp)&017777);
			pimove(paddr, cc, B_WRITE);
		}
#endif	notdef
		if (u.u_error)
			break;
		ce = cp + cc;
again:
		if (pti->pt_flags & PF_REMOTE) {
			if (tp->t_rawq.c_cc) {
				if (pti->pt_flags & PF_NBIO) {
					u.u_count += ce - cp;
					u.u_error = EWOULDBLOCK;
					return;
				}
				sleep((caddr_t)&tp->t_rawq.c_cf, TTOPRI);
				goto again;
			}
			(void) b_to_q(cp, cc, &tp->t_rawq);
			(void) putc(0, &tp->t_rawq);
			wakeup((caddr_t)&tp->t_rawq);
			return;
		}
		while (cp < ce) {
			while (tp->t_delct && tp->t_rawq.c_cc >= TTYHOG - 2) {
				wakeup((caddr_t)&tp->t_rawq);
				if (tp->t_state & TS_NBIO) {
					u.u_count += ce - cp;
					if (cnt == 0)
						u.u_error = EWOULDBLOCK;
					return;
				}
				/* Better than just flushing it! */
				/* Wait for something to be read */
				sleep((caddr_t)&tp->t_rawq.c_cf, TTOPRI);
				goto again;
			}
			(*linesw[tp->t_line].l_rint)(*cp++, tp);
			cnt++;
		}
	} while (u.u_count);
}

/*ARGSUSED*/
ptyioctl(dev, cmd, addr, flag)
	caddr_t addr;
	dev_t dev;
{
	register struct tty *tp;
	register struct pt_ioctl *pti;
	int info;

	if (minor(dev) >= npty) {
		u.u_error = ENXIO;
		return;
	}

	tp = pt_tty[minor(dev)];
	pti= &pt_ioctl[minor(dev)];
#ifdef	vax
	info = ttioctl(tp, cmd, addr, flag);
#else
	info = (*linesw[tp->t_line].l_ioctl)(tp, cmd, (caddr_t)addr, flag);
#endif
	if (cdevsw[major(dev)].d_open == ptcopen)
	switch (info)
	{
		case TIOCSETP:
			while (getc(&tp->t_outq) >= 0);
		case TIOCSETN:
			break;
		case TIOCPKT:
			if (copyin((caddr_t)addr, (caddr_t)&info, sizeof info)) {
				u.u_error = EFAULT;
				return;
			}
			if (info)
				pti->pt_flags |= PF_PKT;
			else
				pti->pt_flags &= ~PF_PKT;
			return;
		case TIOCREMOTE:
			if (copyin((caddr_t)addr, (caddr_t)&info, sizeof info)) {
				u.u_error = EFAULT;
				return;
			}
			if (info)
				pti->pt_flags |= PF_REMOTE;
			else
				pti->pt_flags &= ~PF_REMOTE;
			flushtty(tp, FREAD|FWRITE);
			return;
		default:
			u.u_error = ENOTTY;
		case 0:
			break;
	}
	if (cdevsw[major(dev)].d_open == ptcopen)
	{
		if (tp->t_state & TS_NBIO)
			pti->pt_flags |= PF_NBIO;
		else
			pti->pt_flags &= ~PF_NBIO;
	
		if (pti->pt_flags & PF_NBIO)
		{
			tp->t_state &= ~TS_NBIO;
			return;
		}
	}
	{
		int stop = (tp->t_un.t_chr.t_stopc == ('s'&037) &&
			    tp->t_un.t_chr.t_startc == ('q'&037));
		if (pti->pt_flags & PF_NOSTOP) {
			if (stop) {
				pti->pt_send &= ~TIOCPKT_NOSTOP;
				pti->pt_send |= TIOCPKT_DOSTOP;
				pti->pt_flags &= ~PF_NOSTOP;
				ptcwakeup(tp);
			}
		} else {
			if (stop == 0) {
				pti->pt_send &= ~TIOCPKT_DOSTOP;
				pti->pt_send |= TIOCPKT_NOSTOP;
				pti->pt_flags |= PF_NOSTOP;
				ptcwakeup(tp);
			}
		}
	}
}
