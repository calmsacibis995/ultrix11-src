
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)dz.c	3.0	4/21/86
 */
/*
 *	Unix/v7m DZ11/DZV11 driver
 *
 *	Modified for local/remote terminal operation
 *	and DZV11 support.
 *
 *	Fred Canter 11/12/82
 *
 *	Added check for exclusive bit, and made it call ttyopen through
 *	the linesw table. -Dave Borman, 1/30/84
 *
 *	Added code to send break if arg to ioctl cmd TCSBRK is 0
 *	(system V). George Mathew: 7/10/85
 *	
 *	Changes for O_NDELAY for open().  George Mathew: 7/24/85
 */

#include <sys/param.h>
#include <sys/conf.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/tty.h>
#include <sys/devmaj.h>
#include <sys/file.h>		/* needed for FREAD and FWRITE */

int	io_csr[];	/* CSR address now in config file (c.c) */
int	dz_cnt;		/* number of dz11 or dzv11 lines, see c.c */
			/* MUST BE A MULTIPLE OF THE NUMBER OF LINES PER UNIT */
			/* DZ11 has 8 lines per unit, DZV11 has 4 */
char	dz_shft;	/* DZ SHIFT - 3 for DZ11, 2 for DZV11 */
char	dz_mask;	/* DZ MASK  - 7 for DZ11, 3 for DZV11 */

extern struct tty *dz_tty[];
char	dz_stat;
char	dz_local[];	/* bit per line, 1 = local tty */
char	dz_brk[];	/* soft copy of brk reg (write only) */

char	dz_speeds[] = {
	0, 020, 021, 022, 023, 024, 0, 025,
	026, 027, 030, 032, 034, 036, 0, 0,
	};

#define	MAINT	010
#define	BITS7	020
#define	BITS8	030
#define	TWOSB	040
#define	PENABLE	0100
#define	OPAR	0200
#define	RCVENA	010000

#define	TXIE	040000		/* Transmitter interrupt enable */
#define	RXIE	000100		/* Receiver interrupt enable */
#define	MSE	000040		/* Master Scan enable */
#define	IE	(TXIE|RXIE|MSE)	/* Enable all interrupts */

#define	PERROR	010000		/* Parity error */
#define	FRERROR	020000		/* Framing error */
#define	ORERROR	040000		/* silo overrun error */
#define	ERR	(PERROR|FRERROR|ORERROR) /* Any error */

#define	SSPEED	7		/* standard speed: 300 baud */

#define	DZTIMEOUT	0	/* call timeout at end of dzscan */
#define	NODZTIMEOUT	1	/* don't call timeout at end of dzscan */

struct device {
	int	dzcsr, dzrbuf;
	char	dztcr, dzdtr;
	char	dztbuf, dzbrk;
};
#define	dzlpr	dzrbuf
#define	dzmsr	dzbrk

#define	ON	1
#define	OFF	0

dzopen(dev, flag)
{
	register struct tty *tp;
	register struct device *dzaddr;
	register int d;
	extern dzstart(), dzscan();

	d = minor(dev);
	if ((d >= dz_cnt) || ((tp = dz_tty[d]) == 0)) {
		u.u_error = ENXIO;
		return;
	}
	dzaddr = io_csr[DZ_RMAJ];
	dzaddr += d>>dz_shft;
/*
 * dzopen can now be called from dzioctl
 * to set maint. loop back mode.
 */
	if(flag == TIOCSMLB) {
		dzaddr->dzcsr |= MAINT;
		return;
	}
	if(flag == TIOCCMLB) {
		dzaddr->dzcsr &= ~MAINT;
		return;
	}
	tp->t_addr = (caddr_t)dzaddr;
	if ((tp->t_state&(ISOPEN|WOPEN)) == 0) {
		tp->t_oproc = dzstart;
		tp->t_iproc = NULL;
		ttychars(tp);
		tp->t_ispeed = SSPEED;
		tp->t_ospeed = SSPEED;
		tp->t_flags = ODDP|EVENP|ECHO;
		dzparam(d);
	} else if (tp->t_state&XCLUDE && u.u_uid!=0) {
		u.u_error = EBUSY;
		return;
	}
/* only set DTR on dialup lines */
	if((dz_local[(d>>dz_shft)&dz_mask] & (1 << (d&dz_mask))) == 0)
		dzmodem(d, ON);
	spl6();
	if((flag&FNDELAY) == 0)
		while ((tp->t_state&CARR_ON)==0) {
			tp->t_state |= WOPEN;
			sleep((caddr_t)&tp->t_rawq, TTIPRI);
		}
	ttyopen(dev, tp);
	spl0();
}

dzclose(dev)
{
	register struct tty *tp;
	register int unit;

	dev = minor(dev);
	unit = (dev >> dz_shft) & dz_mask;
	tp = dz_tty[dev];
	(*linesw[tp->t_line].l_close)(tp);
	dz_brk[unit] &= ~(1 << (dev & dz_mask));
	((struct device *)tp->t_addr)->dzbrk = dz_brk[unit];
	if ((tp->t_state&HUPCLS) || (tp->t_state&CARR_ON) == 0) {
		dzmodem(dev, OFF);
	}
	ttyclose(tp);
}

dzread(dev)
{
	register struct tty *tp;

	tp = dz_tty[minor(dev)];
	(*linesw[tp->t_line].l_read)(tp);
}

dzwrite(dev)
{
	register struct tty *tp;

	tp = dz_tty[minor(dev)];
	(*linesw[tp->t_line].l_write)(tp);
}

dzioctl(dev, cmd, addr, flag)
{
	register struct tty *tp;
	register int unit;
	int s;
	extern int hz;
	extern wakeup();

	unit = (dev >> dz_shft) & dz_mask;
	tp = dz_tty[minor(dev)];
	cmd = (*linesw[tp->t_line].l_ioctl)(tp, cmd, (caddr_t)addr, flag);
	if (cmd==0) {
		return;
	} else if (cmd == TIOCLOCAL) {
		if (u.u_uid)
			u.u_error = EPERM;
		else if(minor(dev) < dz_cnt) {
			dz_local[unit] &= ~(1 << (dev & dz_mask));
			dz_local[unit] |= (flag << (dev & dz_mask));
			/*
			 * Call the scan routine directly to
			 * make the change take effect now.
			 * This keeps init from waiting 2 seconds on
			 * each line it enables for logins.
			 */
			dzscan(NODZTIMEOUT);
		}
	} else if (cmd == TIOCSETP || cmd == TIOCSETN) {
		dzparam(minor(dev));
	} else if (cmd == TIOCSDTR) {
		dzmodem(minor(dev), ON);
	} else if (cmd == TIOCCDTR) {
		dzmodem(minor(dev), OFF);
	} else if (cmd == TIOCSBRK) {
		dz_brk[unit] |= (1 << (dev & dz_mask));
		((struct device *)tp->t_addr)->dzbrk = dz_brk[unit];
	} else if (cmd == TIOCCBRK) {
		dz_brk[unit] &= ~(1 << (dev & dz_mask));
		((struct device *)tp->t_addr)->dzbrk = dz_brk[unit];
	} else if (cmd == TIOCSMLB || cmd == TIOCCMLB) {
		if (u.u_uid)
			u.u_error = EPERM;
		else
			dzopen(dev, cmd);
	} else if (cmd == TIOTCSBRK) {    /* if arg is 0 for TCSBRK cmd 
					   * George Mathew: 7/10/85 */
		dz_brk[unit] |= (1 << (dev & dz_mask));
		((struct device *)tp->t_addr)->dzbrk = dz_brk[unit];
		/*tp->t_state |= TIMEOUT; is this required??? */
		s = spl5();
		timeout(wakeup,(caddr_t)tp,hz/4);
		sleep((caddr_t)tp,PZERO-1);
		splx(s);
		dz_brk[unit] &= ~(1 << (dev & dz_mask));
		((struct device *)tp->t_addr)->dzbrk = dz_brk[unit];
	} else {
		u.u_error = ENOTTY;
	}
}

static
dzparam(dev)
{
	register struct tty *tp;
	register struct device *dzaddr;
	register lpr;

	tp = dz_tty[dev];
	dzaddr = io_csr[DZ_RMAJ];
	dzaddr += dev>>dz_shft;
/*
 * The following three lines of code were inserted
 * to allow the comm. mux. exerciser (cmx) to run
 * the DZ11 in maintenance loop back mode.
 */
	if(dzaddr->dzcsr&MAINT)
		dzaddr->dzcsr |= IE;
	else
		dzaddr->dzcsr = IE;
	if (dz_stat==0) {
		dzscan(DZTIMEOUT);
		dz_stat++;
	}
	if (tp->t_ispeed==0) {	/* Hang up line */
		dzmodem(dev, OFF);
		return;
	}
	lpr = (dz_speeds[tp->t_ispeed]<<8)|(dev&dz_mask);
	if (tp->t_flags&RAW)
		lpr |= BITS8;
	else
		lpr |= BITS7|PENABLE;
	if ((tp->t_flags&EVENP)==0)
		lpr |= OPAR;
	if (tp->t_ispeed == 3)	/* 110 baud */
		lpr |= TWOSB;
	dzaddr->dzlpr = lpr;
}

dzrint(dev)
{
	register struct device *dzaddr;
	register struct tty *tp;
	register c;
	int	ln;

	dzaddr = io_csr[DZ_RMAJ];
	dzaddr += minor(dev);
	while ((c = dzaddr->dzrbuf) < 0) {	/* char. present */
		ln = ((c>>8)&dz_mask)|(dev<<dz_shft);
		if ((ln >= dz_cnt) || ((tp = dz_tty[ln]) == 0))
			continue;
		if((tp->t_state&ISOPEN)==0) {
			wakeup((caddr_t)&tp->t_rawq);
			continue;
		}
		if (c & ERR) {
			if (c&FRERROR) {
				/* framing error, probably a break */
				if (tp->t_flags&RAW)
					c = 0;		/* null (for getty) */
				else
					c = tun.t_intrc;;	/* DEL (intr) */
			} else {
				if (c&PERROR){
					if ((tp->t_flags&(EVENP|ODDP))==EVENP
					 || (tp->t_flags&(EVENP|ODDP))==ODDP ){
						/* parity error */
						tp->t_errcnt++;
						tp->t_lastec = c;
						continue;
					} else
						c &= ~PERROR;
				}
				if(c & ORERROR) {
					/* count errors on each line */
					tp->t_errcnt++;
					/* save last error character */
					tp->t_lastec = c;
				}
			}
		}
		(*linesw[tp->t_line].l_rint)(c, tp);
	}
}

dzxint(dev)
{
	register struct tty *tp;
	register struct device *dzaddr;
	register int csrsav;

	dzaddr = io_csr[DZ_RMAJ];
	dzaddr += minor(dev);
	while((csrsav = dzaddr->dzcsr) < 0) {	/* TX rdy */
		tp = dz_tty[((dev<<dz_shft)|(csrsav>>8)&dz_mask)];
/* below is fix for dzv11 multiple character problem  OHMS */
		if(tp->t_state & BUSY) {
			dzaddr->dztbuf = tp->t_char;
			tp->t_state &= ~BUSY;
		} else
			dzaddr->dztcr &= ~(1<<((csrsav>>8)&dz_mask));
		dzstart(tp);
	}
}

dzstart(tp)
register struct tty *tp;
{
	register struct device *dzaddr;
	register c;
	int s, unit;
	extern ttrstrt();

/*
 * Find the minor device number from the TTY pointer,
 * the old method will not work because the TTY
 * structure assignments may not be in order.
 */
	for(unit=0; unit<dz_cnt; unit++)
		if(dz_tty[unit] == tp)
			break;
	dzaddr = io_csr[DZ_RMAJ];
	dzaddr += unit>>dz_shft;
	unit = 1<<(unit&dz_mask);
	s = spl5();
	if (tp->t_state&(TIMEOUT|BUSY)) {
		splx(s);
		return;
	}
	if (tp->t_state&TTSTOP) {
		dzaddr->dztcr &= ~unit;
		splx(s);
		return;
	}
	if ((c=getc(&tp->t_outq)) >= 0) {
		if (c>=0200 && (tp->t_flags&RAW)==0) {
			dzaddr->dztcr &= ~unit;
			tp->t_state |= TIMEOUT;
			timeout(ttrstrt, (caddr_t)tp, (c&0177)+6);
		} else {
			tp->t_char = c;
			tp->t_state |= BUSY;
			dzaddr->dztcr |= unit;
		}
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
	}
	splx(s);
}

static
dzmodem(dev, flag)
{
	register struct device *dzaddr;
	register bit;

	dzaddr = io_csr[DZ_RMAJ];
	dzaddr += minor(dev)>>dz_shft;
	bit = 1<<(minor(dev)&dz_mask);
	if (flag==OFF)
		dzaddr->dzdtr &= ~bit;
	else	dzaddr->dzdtr |= bit;
}

dzscan(flag)
int flag;
{
	register i;
	register struct device *dzaddr;
	register struct tty *tp;
	char	bit;

	for (i=0; i<dz_cnt; i++) {
		dzaddr = io_csr[DZ_RMAJ];
		dzaddr += i>>dz_shft;
		if((tp = dz_tty[i]) == 0)
			continue;
		bit = 1 << (i&dz_mask);
		if (dzaddr->dzmsr&bit || (dz_local[i>>dz_shft] & bit)) {
			if ((tp->t_state&CARR_ON)==0) {
				wakeup((caddr_t)&tp->t_rawq);
				tp->t_state |= CARR_ON;
			}
		} else {
			if (tp->t_state&CARR_ON) {
				if (tp->t_state&ISOPEN) {
					gsignal(tp->t_pgrp, SIGHUP);
					dzaddr->dzdtr &= ~bit;
					flushtty(tp, FREAD|FWRITE);
				}
				tp->t_state &= ~CARR_ON;
			}
		}
	}
	if (flag == DZTIMEOUT)
		timeout(dzscan, (caddr_t)DZTIMEOUT, 120);
}
