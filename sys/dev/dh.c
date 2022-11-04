
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)dh.c	3.0	4/21/86
 */
/*
 *	ULTRIX-11 DH11 driver
 *
 *	Modified for local/remote terminal operation.
 *
 *	Fred Canter 1/21/82
 *
 *	This driver calls on the DHDM driver.
 *	If the DH has no DM11-BB, then the latter will
 *	be fake. To insure loading of the correct DM code,
 *	lib2 should have dhdm.o, dh.o and dhfdm.o in that order.
 *
 *	Added code to send break if arg to ioctl cmd TCSBRK is 0
 *	(System V). George Mathew: 7/10/85
 *	Changes for O_NDELAY flag in open(). Removed 3/1/86 -- Fred
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/conf.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/tty.h>
#include <sys/devmaj.h>
#include <sys/uba.h>
#include <sys/file.h>
#ifdef	UCB_CLIST
#include <sys/seg.h>
#endif

#define	q3	tp->t_outq
int	io_csr[];	/* CSR address now in config file (c.c) */
extern int ndh11;	/* number of DH lines */
int	dh_local[];	/* bit per line, 1 = local tty */

extern struct	tty *dh11[];
extern char	dhcc[];
extern int dhchars[];
int	dhstart();
int	ttrstrt();

/*
 * Hardware control bits
 */
#define	BITS6	01
#define	BITS7	02
#define	BITS8	03
#define	TWOSB	04
#define	PENABLE	020
/* DEC manuals incorrectly say this bit causes generation of even parity. */
#define	OPAR	040
#define	MAINT	01000	/* sets DH maint. loop back mode */
#define	HDUPLX	040000

#define	IENAB	030100
#define	ERR	070000
#define	PERROR	010000
#define	FRERROR	020000
#define	OVERRUN	040000
#define	XINT	0100000
#define	SSPEED	7	/* standard speed: 300 baud */
#define	NSILO	16
#define	DHTIME	3
extern int dhtimer();

/*
 * DM control bits
 */
#define	TURNON	03	/* CD lead + line enable */
#define	TURNOFF	01	/* line enable */
#define	RQS	04	/* request to send */
#define	DM_DTR	02	/* DM data terminal ready */
#define	DM_RTS	04	/* DM request to send */

/*
 * Software copy of last dhbar
 */
extern int dhsar[];

struct device
{
	union {
		int	dhcsr;
		char	dhcsrl;
	} un;
	int	dhnxch;
	int	dhlpr;
	char	*dhcar;
	int	dhbcr;
	int	dhbar;
	int	dhbreak;
	int	dhsilo;
};

/*
 * Open a DH11 line.
 */
dhopen(dev, flag)
{
	register struct tty *tp;
	register d;
	register struct device *addr;
	static	timer_on;
	int s;

	d = minor(dev);
	if ((d >= ndh11) || ((tp = dh11[d]) == 0)) {
		u.u_error = ENXIO;
		return;
	}
	addr = io_csr[DH_RMAJ];
	addr += d>>4;
/*
 * dhopen can be called from dhioctl
 * to set maint. loop back mode.
 */
	if(flag == TIOCSMLB) {
		addr->un.dhcsr |= MAINT;
		return;
	}
	if(flag == TIOCCMLB) {
		addr->un.dhcsr &= ~MAINT;
		return;
	}
	tp->t_addr = (caddr_t)addr;
	tp->t_oproc = dhstart;
	tp->t_iproc = NULL;
	tp->t_state |= WOPEN;
	s = spl6();
	if (!timer_on) {
		timer_on++;
		timeout(dhtimer, (caddr_t)0, DHTIME);
	}
	splx(s);
	addr->un.dhcsr |= IENAB;
	if ((tp->t_state&ISOPEN) == 0) {
		ttychars(tp);
		tp->t_ispeed = SSPEED;
		tp->t_ospeed = SSPEED;
		tp->t_flags = ODDP|EVENP|ECHO;
		dhparam(d);
	}
	if (tp->t_state&XCLUDE && u.u_uid!=0) {
		u.u_error = EBUSY;
		return;
	}
	dmopen(d, flag);
	ttyopen(dev, tp);
}

/*
 * Close a DH11 line.
 */
dhclose(dev, flag)
dev_t dev;
int  flag;
{
	register struct tty *tp;
	register d;

	d = minor(dev);
	tp = dh11[d];
	(*linesw[tp->t_line].l_close)(tp);
	if ((tp->t_state&HUPCLS) || (tp->t_state&CARR_ON) == 0)
		dmctl(d, TURNOFF);
	ttyclose(tp);
}

/*
 * Read from a DH11 line.
 */
dhread(dev)
{
register struct tty *tp;

	tp = dh11[minor(dev)];
	(*linesw[tp->t_line].l_read)(tp);
}

/*
 * write on a DH11 line
 */
dhwrite(dev)
{
register struct tty *tp;

	tp = dh11[minor(dev)];
	(*linesw[tp->t_line].l_write)(tp);
}

/*
 * DH11 receiver interrupt.
 */
dhrint(dev)
{
	register struct tty *tp;
	register int c;
	register struct device *addr;
	int	ln;

	addr = io_csr[DH_RMAJ];
	addr += minor(dev);
	while ((c = addr->dhnxch) < 0) {	/* char. present */
		ln = (minor(dev)<<4) + ((c>>8)&017);
		dhchars[minor(dev)]++;
		if ((ln >= ndh11) || ((tp = dh11[ln]) == 0))
			continue;
		if((tp->t_state&ISOPEN)==0) {
			wakeup((caddr_t)tp);
			continue;
		}
		if (c&PERROR){
			if ((tp->t_flags&(EVENP|ODDP))==EVENP
			 || (tp->t_flags&(EVENP|ODDP))==ODDP ){
				tp->t_errcnt++;
				tp->t_lastec = c;
				continue;
			} else
				c &= ~(PERROR);
		}
		if (c&FRERROR)		/* break */
			if (tp->t_flags&RAW)
				c = 0;	/* null (for getty) */
			else
				c = tun.t_intrc;	/* DEL (intr) */
		if(c & ERR) {
			tp->t_errcnt++;		/* count errors on each line */
			tp->t_lastec = c;	/* save last error character */
		}
		(*linesw[tp->t_line].l_rint)(c,tp);
	}
}

/*
 * stty/gtty for DH11
 */
dhioctl(dev, cmd, addr, flag)
caddr_t addr;
{
	register struct tty *tp;
	register int i;
	int s;
	extern wakeup();
	extern int hz;

	tp = dh11[minor(dev)];
	cmd = (*linesw[tp->t_line].l_ioctl)(tp, cmd, addr, flag);
	if (cmd == 0)
		return;
	if (cmd == TIOCSETP || cmd == TIOCSETN) {
		dhparam(dev);
	} else if (cmd == TIOCLOCAL) {
		if (u.u_uid)
			u.u_error = EPERM;
		else if(minor(dev) < ndh11) {
			i = (dev >> 4) & 7;
			dh_local[i] &= ~(1 << (dev & 017));
			dh_local[i] |= (flag << (dev & 017));
		}
	} else if (cmd == TIOCSBRK) {
		((struct device *)tp->t_addr)->dhbreak |= (1<<(dev&017));
	} else if (cmd == TIOCCBRK) {
		((struct device *)tp->t_addr)->dhbreak &= ~(1<<(dev&017));
	} else if (cmd == TIOCSDTR) {
		dmctl(dev, (DM_DTR|DM_RTS));
	} else if (cmd == TIOCCDTR) {
		dmctl(dev, 0);
	} else if (cmd == TIOTCSBRK) {    /* if arg is 0 for TCSBRK cmd 
					   * George Mathew: 7/11/85 */
		((struct device *)tp->t_addr)->dhbreak |= (1<<(dev&017));
		/*tp->t_state |= TIMEOUT; is this required??? */
		s = spl5();
		timeout(wakeup,(caddr_t)tp,hz/4);
		sleep((caddr_t)tp,PZERO-1);
		splx(s);
		((struct device *)tp->t_addr)->dhbreak &= ~(1<<(dev&017));
	} else if (cmd == TIOCSMLB || cmd == TIOCCMLB) {
		if (u.u_uid)
			u.u_error = EPERM;
		else
			dhopen(dev, cmd);
	} else {
		u.u_error = ENOTTY;
	}
}

/*
 * Set parameters from open or stty into the DH hardware
 * registers.
 */
dhparam(dev)
{
	register struct tty *tp;
	register struct device *addr;
	register d;

	d = minor(dev);
	tp = dh11[d];
	addr = (struct device *)tp->t_addr;
	spl5();
	addr->un.dhcsrl = (d&017) | IENAB;
	/*
	 * Hang up line?
	 */
	if ((tp->t_ispeed)==0) {
		tp->t_state |= HUPCLS;
		dmctl(d, TURNOFF);
		spl0();		/* OHMS added 3/26/84 */
		return;
	}
	d = ((tp->t_ospeed)<<10) | ((tp->t_ispeed)<<6);
	if ((tp->t_ispeed) == 4)		/* 134.5 baud */
		d |= BITS6|PENABLE|HDUPLX;
	else if (tp->t_flags&RAW)
		d |= BITS8;
	else
		d |= BITS7|PENABLE;
	if ((tp->t_flags&EVENP) == 0)
		d |= OPAR;
	if ((tp->t_ospeed) == 3)	/* 110 baud */
		d |= TWOSB;
	addr->dhlpr = d;
	spl0();
}

/*
 * DH11 transmitter interrupt.
 * Restart each line which used to be active but has
 * terminated transmission since the last interrupt.
 */
dhxint(dev)
{
	register struct tty *tp;
	register struct device *addr;
	register d;
	int ttybit, bar, *sbar;
	char	*p;	/* address offset (if UB map used) */

#ifndef	UCB_CLIST
	if(ubmaps)
		p = &cfree;
	else
		p = 0;
#endif	UCB_CLIST
	d = minor(dev);
	addr = io_csr[DH_RMAJ];
	addr += d;
	addr->un.dhcsr &= ~XINT;
	sbar = &dhsar[d];
	bar = *sbar & ~addr->dhbar;
	d <<= 4; ttybit = 1;

	for(; bar; d++, ttybit <<= 1) {
	    if(bar&ttybit) {
		*sbar &= ~ttybit;
		bar &= ~ttybit;
		tp = dh11[d];
		addr->un.dhcsrl = (d&017)|IENAB;
		if (tp->t_state&FLUSH)
		    tp->t_state &= ~FLUSH;
		else {
#ifndef	UCB_CLIST
		    ndflush(&q3, (addr->dhcar+p)-q3.c_cf);
#else	UCB_CLIST
		    if(ubmaps)
			p = (char *)addr->dhcar + (char *)cfree -
			    CLIST_UBADDR - q3.c_cf;
		    else
			p = (char *)(((((long)(addr->dhsilo&0300))<<10L) +
			    (((long)addr->dhcar)&0xffffL)) - clstaddr -
			    (long)(q3.c_cf - SEG5));
		    ndflush(&q3, p);
#endif	UCB_CLIST
		}
		tp->t_state &= ~BUSY;
		if (tp->t_line)
		    (*linesw[tp->t_line].l_start)(tp);
		else
		    dhstart(tp);
	    }
	}
}

/*
 * Start (restart) transmission on the given DH11 line.
 */
dhstart(tp)
register struct tty *tp;
{
	register struct device *addr;
	register nch;
	int s, d;

	/*
	 * If it's currently active, or delaying,
	 * no need to do anything.
	 */
	s = spl5();
/*
 * Find the minor device number from the TTY pointer,
 * the old way will not work because the TTY structure
 * assignments may not be in order.
 */
	for(d=0; d<ndh11; d++)
		if(dh11[d] == tp)
			break;
	addr = (struct device *)tp->t_addr;
	if (tp->t_state&(TIMEOUT|BUSY|TTSTOP))
		goto out;


	/*
	 * If the writer was sleeping on output overflow,
	 * wake him when low tide is reached.
	 */
	if (tp->t_outq.c_cc<=TTLOWAT(tp)) {
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
#endif
	}

	if (tp->t_outq.c_cc == 0)
		goto out;



	/*
	 * Find number of characters to transfer.
	 */
	if (tp->t_flags & RAW) {
		nch = ndqb(&tp->t_outq, 0);
	} else {
		nch = ndqb(&tp->t_outq, 0200);
		if (nch == 0) {
			nch = getc(&tp->t_outq);
			timeout(ttrstrt, (caddr_t)tp, (nch&0177)+6);
			tp->t_state |= TIMEOUT;
			goto out;
		}
	}
	/*
	 * If any characters were set up, start transmission;
	 */
	if (nch) {
#ifndef	UCB_CLIST
		addr->un.dhcsrl = (d&017)|IENAB;
		if(ubmaps)	/* must offset address if UB map used */
			addr->dhcar = tp->t_outq.c_cf - (char *)cfree;
		else
			addr->dhcar = tp->t_outq.c_cf;
#else	UCB_CLIST
		if(ubmaps) {
		    addr->un.dhcsrl = (d&017)|IENAB;
		    addr->dhcar = CLIST_UBADDR + tp->t_outq.c_cf -
			(char *)cfree;
		} else {
		    long paddr;
		    paddr = clstaddr + (long)(tp->t_outq.c_cf - SEG5);
		    addr->un.dhcsrl = (d&017) | ((hiint(paddr)&3) << 4) | IENAB;
		    addr->dhcar = loint(paddr);
		}
#endif	UCB_CLIST
		addr->dhbcr = -nch;
		dhcc[d] = nch;
		nch = 1<<(d&017);
		addr->dhbar |= nch;
		dhsar[d>>4] |= nch;
		tp->t_state |= BUSY;
	}
    out:
	splx(s);
}


/*
 * Stop output on a line.
 */
dhstop(tp, flag)
register struct tty *tp;
{
	register struct device *addr;
	register d, s;

	addr = (struct device *)tp->t_addr;
	s = spl6();
	if (tp->t_state & BUSY) {
		d = minor(tp->t_dev);
		addr->un.dhcsrl = (d&017) | IENAB;
		if ((tp->t_state&TTSTOP)==0) {
			tp->t_state |= FLUSH;
		}
		addr->dhbcr = -1;
	}
	splx(s);
}

dhtimer(dev)
{
register d,cc;
register struct device *addr;

	addr = io_csr[DH_RMAJ]; d = 0;
	do {
		cc = dhchars[d];
		dhchars[d] = 0;
		if (cc > 50)
			cc = 32; else
			if (cc > 16)
				cc = 16; else
				cc = 0;
		addr->dhsilo = cc;
		addr += 1;
		dhrint(d++);
	} while (d < (ndh11+15)/16);
	timeout(dhtimer, (caddr_t)0, DHTIME);
}

