
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)uh.c	3.0	4/21/86
 */
/*
 *
 *	ULTRIX-11 DHU11/DHV11 Driver
 *	
 *	Bill Burns	March 1984
 *
 *	Added code to send break if arg to ioctl cmd TCSBRK is 0
 *	(System V) George Mathew: 7/10/85
 *
 */
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/conf.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/tty.h>
#include <sys/devmaj.h>
#include <sys/file.h>		/* needed for FREAD & FWRITE */
#include <sys/uba.h>
#ifdef	UCB_CLIST
#include <sys/seg.h>
#endif

#define	q3	tp->t_outq
#define DHUTIME 	3
#define SSPEED	7		/* 300 baud */
extern	int 	nuh11;
extern	char	uhcc[];
extern	struct tty *uh11[];
extern	int	uhchars[];
extern	char	uh_shft;		/* dhu11 = 4, dhv11 = 3 */
extern	char	uh_mask;		/* dhu11 = 017, dhv11 = 07 */
char	uh_spds[] = {
	0, 0, 01, 02, 03, 04, 0, 05,		/* illegal speeds: B50 B200 */
	06, 07, 010, 012, 013, 015, 016, 0,	/* exta = 19200 */
	};					/* extb = illegal */
int	uhtimer();
int	uh_local[];
int	uhstart();
int	io_csr[];
int	ttrstrt();

/* register definitions */
/* csr lo byte definitions */
#define RDA	0200		/* receiver data available */
#define RINT	0100		/* receiver interrupt enable */
#define MR	040		/* master reset */
/* csr hi byte definitions */
#define TA	0200		/* transmitter action */
#define XINT	0100		/* transmit interrupt enable */
#define DIAG	040		/* diagnostic failure */
#define	DMAERR	020		/* DMA error */
#define XLINE	017		/* transmit line number */

/* rbuf bits */
#define VALID	0100000		/* data valid */
#define ERR	070000		/* error bits */
#define OVERR	040000		/* overrun error */
#define FERR	020000		/* framing error */
#define PERR	010000		/* parity error */

/* rit (receive interrupt timer) bits (dhu11 only) */

#define NODLY	01		/* no delay - interrupt immediatly */

/* lpr bits */
#define EPAR	0100		/* even parity - (opposite of old dh) */
#define TWOSB	0200		/* two stop bits */
#define PENABLE	040		/* parity enable */
#define BITS8	030		/* eight data bits */
#define BITS7	020		/* seven data bits */
#define BITS6	010		/* six data bits */

/* line status register */
#define DSR	0200		/* data set ready */
#define RING	040		/* ring indicator */
#define CD	020		/* carrier detect */
#define CTS	010		/* clear to send */
#define DHU11	01		/* on = DHU11, off = DHV11 */

/* line control register */
#define RTS	010000		/* request to send */
#define DTR	01000		/* data terminal ready */
#define LINK	0400		/* set for data set change in rbuf */
#define MAINT	0200		/* loopback mode */
#define XXOFF	040		/* transmit XOFF */
#define XFLOW	020		/* transmit flow control */
#define BREAK	010		/* transmit break */
#define RENA	04		/* reciever enable */
#define RFLOW	02		/* receiver flow control */
#define XABT	01		/* abort transmit */

/* transmit buffer address 2 */
#define TENA	0200		/* transmitter enable (hi byte) */
#define START	0200		/* start dma transfer (lo byte) */


/*
 * DHU11/DHV11 device registers
 */

struct device
{
	char	uhcsrl;		/* csr register lo byte */
	char	uhcsrh;		/* csr register hi byte */
	union {
		int	uhrbuf; /* receive buffer reg. */
		char	uhrit;	/* aka: recieve interrupt delay timer (dhu)*/
	} uh2;
	int	uhlpr;		/* line parameter reg. */
	union {
		int	uhdata;		/* xmit fifo data */
		struct 
		{
			char	uhsize;		/* fifo size (dhu) */
			char	uhstat; 	/* line status reg. */
		} uh6b;
	} uh6;
	int	uhctl;		/* line control reg. */
	int	uhxba;		/* transmit buffer address */
	char	uhxba2l;	/* transmit buffer address ext. lo byte */
	char	uhxba2h;	/* transmit buffer address ext. hi byte */
	int	uhxcnt;		/* transmit char count */
};

/*
 * open a DHU11/DHV11 line
 */

uhopen(dev, flag)
{
	register struct tty *tp;
	register d;
	register struct device *addr;
	static timer_on;
	int s, c, i;

	d = minor(dev);
/*
 * If no tty struct...
 */
	if ((d >= nuh11) || ((tp = uh11[d]) == 0)) {
		u.u_error = ENXIO;
		return;
	}
	addr = io_csr[UH_RMAJ];
/*
 * adjust address
 */
	addr += (d >> uh_shft);
/*
 * uhopen can be called from uhioctl
 * to set maint. loop back mode.
 */
	if(flag == TIOCSMLB) {
		spl5();
		addr->uhcsrl = (RINT|(d & uh_mask));
/* below used for hardware flow control */
		/* addr->uhctl &= ~(XFLOW); */
		addr->uhctl |= (MAINT);
		spl0();
		return;
	}
	if(flag == TIOCCMLB) {
		spl5();
		addr->uhcsrl = (RINT|(d & uh_mask));
		addr->uhctl &= ~(MAINT);
		spl0();
		return;
	}

/*
 * set up tty struct
 */
	tp->t_addr = (caddr_t)addr;
	tp->t_oproc = uhstart;
	tp->t_iproc = NULL;
	tp->t_state |= WOPEN;
	/* timer routine not used
	if(addr->uh6.uhstat & 01) {
		s = spl6();
		if(!timer_on) {
			timer_on++;
			timeout(uhtimer, (caddr_t)0, DHUTIME);
		} 
		splx(s);
	}
	*/
	addr->uhcsrl = RINT;
	if((tp->t_state & ISOPEN) == 0) {
		ttychars(tp);			/* set control chars */
		tp->t_ispeed = SSPEED;
		tp->t_ospeed = SSPEED;
		tp->t_flags = ODDP|EVENP|ECHO;
		uhparam(d);
	}
	if(tp->t_state&XCLUDE && u.u_uid != 0) {
		u.u_error = EBUSY;
		return;
	}
/*
 * set no interrupt delay for DHU11
 */
	if(addr->uh6.uh6b.uhstat & 01) {
		spl5();
		addr->uhcsrl = RINT;	/* must select line zero */
		addr->uh2.uhrit = NODLY;
		spl0();
	}
/*
 * only set DTR on dialup lines 
 */
	if(uh_local[(d>>uh_shft) & uh_mask] & (1 << (d & uh_mask))) {
		spl5();
		addr->uhcsrl = (RINT|(d & uh_mask));
		addr->uhctl &= ~(RTS|DTR|LINK);
		addr->uhctl |= RENA;
		tp->t_state |= CARR_ON;
		spl0();
	} else {
		spl5();
		addr->uhcsrl = (RINT|(d & uh_mask));
		addr->uhctl |= (RTS|DTR|RENA|LINK);
		if (addr->uh6.uh6b.uhstat & CD)
			tp->t_state |= CARR_ON;
		if((flag&FNDELAY) == 0)
			while ((tp->t_state & CARR_ON) == 0)
				sleep((caddr_t)&tp->t_rawq, TTIPRI);
		spl0();
	}
	ttyopen(dev, tp);
}

/*
 * close a DHU11/DHV11 line
 */

uhclose(dev, flag)
dev_t dev;
int flag;
{
	register struct tty *tp;
	register struct device *addr;
	register d;

	d = minor(dev);
	tp = uh11[d];
	(*linesw[tp->t_line].l_close)(tp);
	if((tp->t_state&HUPCLS) || (tp->t_state&CARR_ON) == 0) {
		addr = tp->t_addr;
		spl5();
		addr->uhcsrl = (RINT|(d & uh_mask));
		addr->uhctl = NULL;
		spl0();
	}
	ttyclose(tp);
}

/*
 * Read from a DHU11/DHV11 line
 */

uhread(dev)
{
	register struct tty *tp;

	tp = uh11[minor(dev)];
	(*linesw[tp->t_line].l_read)(tp);
}

/*
 * Write to a DHU11/DHV11 line. 
 */

uhwrite(dev)
{
	register struct tty *tp;

	tp = uh11[minor(dev)];
	(*linesw[tp->t_line].l_write)(tp);
}

/*
 * DHU11/DHV11 reciever interrupt
 */
	
uhrint(dev)
{
	register struct tty *tp;
	register int c;
	register struct device *addr;
	int ln, d;

	d = minor(dev);
	addr = io_csr[UH_RMAJ];
	addr =+ minor(dev);

	while((c = addr->uh2.uhrbuf) < 0) {	/* process characters */
/*
 * construct a minor device number from the dev code
 */
		ln = (d<<uh_shft) + ((c>>8)&uh_mask);

		uhchars[minor(dev)]++;
		if((ln >= nuh11) || ((tp = uh11[ln]) == 0))
				continue;
		if((c&ERR) == ERR) {  /* &&((c&0377)!= 0))/* data set change */
			if(c&01) {			  /* or diag code */
				continue;
			} else {		/* modem status change */
				if ((ln < nuh11) && tp) {
					wakeup((caddr_t)&tp->t_rawq);
					if ((( c & CD) == 0) &&
					    (uh_local[d] & (1 << (ln&uh_mask)))==0) {
						if ((tp->t_state & WOPEN)==0) {
							gsignal(tp->t_pgrp, SIGHUP);
							addr->uhcsrl = (RINT|(ln&uh_mask));
							addr->uhctl = NULL;
							flushtty(tp, FREAD|FWRITE);
						}
						tp->t_state &= ~CARR_ON;
					} else {
						tp->t_state |= CARR_ON;
					}
				}
			}
		continue;
		}
		if((tp->t_state&ISOPEN)==0) {
			wakeup((caddr_t)tp);
			continue;
		}
		if(c & PERR) {
			if((tp->t_flags&(EVENP|ODDP)) == EVENP
			   || (tp->t_flags&(EVENP|ODDP)) == ODDP ){
				tp->t_errcnt++;
				tp->t_lastec = c;
				continue;
			} else
				c &= ~(PERR);
		}
		if(c & FERR)			/* break */
			if(tp->t_flags&RAW)
				c = 0;		/* null (for getty) */
			else
				c = tun.t_intrc;	/* DEL (intr) */
		if(c & ERR) {
			tp->t_errcnt++;		/* count errors on each line */
			tp->t_lastec = c;	/* save last error character */
		}
		(*linesw[tp->t_line].l_rint)(c, tp);
	}
}

/*
 * stty/gtty for DHU11/DHV11
 */

uhioctl(dev, cmd, addr, flag)
caddr_t addr;
{
	register struct tty *tp;
	register int i;
	int s;
	extern wakeup();
	extern int hz;

	tp = uh11[minor(dev)];
	cmd = (*linesw[tp->t_line].l_ioctl)(tp, cmd, addr, flag);
	if (cmd == 0)
		return;
	if (cmd == TIOCSETP || cmd == TIOCSETN)
		uhparam(dev);
	else if(cmd == TIOCLOCAL) {
		if (u.u_uid)
			u.u_error = EPERM;
		else if(minor(dev) < nuh11) {
			i = (dev >> uh_shft) & uh_mask;
			uh_local[i] &= ~(1 << (dev & uh_mask));
			uh_local[i] |= (flag << (dev & uh_mask));
		}
	} else if (cmd == TIOCSBRK) {
		spl5();
		((struct device *)tp->t_addr)->uhcsrl = (RINT|(dev & uh_mask));
		((struct device *)tp->t_addr)->uhctl |= BREAK;
		spl0();
	} else if (cmd == TIOCCBRK) {
		spl5();
		((struct device *)tp->t_addr)->uhcsrl = (RINT|(dev & uh_mask));
		((struct device *)tp->t_addr)->uhctl &= ~(BREAK);
		spl0();
	} else if (cmd == TIOTCSBRK) {    /* if arg is 0 for TCSBRK cmd 
					   * George Mathew: 7/11/85 */
		((struct device *)tp->t_addr)->uhcsrl = (RINT|(dev & uh_mask));
		((struct device *)tp->t_addr)->uhctl |= BREAK;
		/*tp->t_state |= TIMEOUT; is this required??? */
		s = spl5();
		timeout(wakeup,(caddr_t)tp,hz/4);
		sleep((caddr_t)tp,PZERO-1);
		splx(s);
		((struct device *)tp->t_addr)->uhcsrl = (RINT|(dev & uh_mask));
		((struct device *)tp->t_addr)->uhctl &= ~(BREAK);
	} else if (cmd == TIOCSDTR) {
		spl5();
		((struct device *)tp->t_addr)->uhcsrl = (RINT|(dev & uh_mask));
		((struct device *)tp->t_addr)->uhctl |= (DTR|RTS);
		spl0();
	} else if (cmd == TIOCCDTR) {
		spl5();
		((struct device *)tp->t_addr)->uhcsrl = (RINT|(dev & uh_mask));
		((struct device *)tp->t_addr)->uhctl &= ~(DTR|RTS);
		spl0();
	} else if (cmd == TIOCSMLB || cmd == TIOCCMLB) {
		if (u.u_uid)
			u.u_error = EPERM;
		else
			uhopen(dev, cmd);
	} else {
		u.u_error = ENOTTY;
	}
}

static
uhparam(dev)
{
	register struct tty *tp;
	register struct device *addr;
	register d;

	d = minor(dev);
	tp = uh11[d];
	addr = ((struct device *)tp->t_addr);
	spl5();
	addr->uhcsrl = (RINT|(d&uh_mask));
	if((tp->t_ispeed)==0) {
		tp->t_state |= HUPCLS;
		((struct device *)tp->t_addr)->uhctl &= ~(LINK|DTR|RTS);
		spl0();
		return;
	}
/*
 * set up line parameter reg 
 */
	d = (uh_spds[tp->t_ospeed] << 12) | (uh_spds[tp->t_ispeed] << 8);
	if((tp->t_ispeed) == 4)		/* 134.5 baud */
		d |= (BITS6|PENABLE);	/* HDUPLX in stock dh */
	else if(tp->t_flags&RAW)
		d |= BITS8;
	else
		d |= (BITS7|PENABLE);
	if(tp->t_flags&EVENP)
		d |= EPAR;
	if((tp->t_ospeed) == 3)		/* 110 baud */
		d |= TWOSB;
	(struct device *)addr->uhlpr = d;
/* below was used for hardware flow control
	 if((tun.t_startc == CSTART) && (tun.t_stopc == CSTOP))
		addr->uhctl |= (XFLOW);
	else
		addr->uhctl &= ~(XFLOW);
*/
	spl0();
}

/*
 * DHU11/DHV11 transmitter interrupt
 */

uhxint(dev)
{
	register struct tty *tp;
	register struct device *addr;
	register d;
	char csrsv;
	char	*p;	/* address offset (if UB map used) */

#ifndef	UCB_CLIST
	if(ubmaps)
		p = &cfree;
	else
		p = 0;
#endif	UCB_CLIST
	d = minor(dev);
	addr = io_csr[UH_RMAJ];
	addr += d;
	while((csrsv = addr->uhcsrh) < 0) {
		/*
		if (csrsv & DMAERR)
			printf("dhuv: dma error\n");
		if (csrsv & DIAG)
			printf("dhuv: diag failure\n");
		*/
		d = minor(dev);
		d <<= uh_shft;
		d |= (csrsv & uh_mask);
		tp = uh11[d];
		addr->uhcsrl = ((d&uh_mask)|RINT);
		if(tp->t_state&FLUSH)
			tp->t_state &= ~FLUSH;
		else {
#ifndef	UCB_CLIST
			ndflush(&q3, ((char *)addr->uhxba+p)-q3.c_cf);
#else	UCB_CLIST
			if(ubmaps)
			    p = (char *)addr->uhxba + (char *)cfree -
				CLIST_UBADDR - q3.c_cf;
			else
			    p = (char *)(((((long)(addr->uhxba2l&077))<<16L) +
				(((long)addr->uhxba)&0xffffL)) - clstaddr -
				(long)(q3.c_cf - SEG5));
			ndflush(&q3, p);
#endif	UCB_CLIST
		}
		tp->t_state &= ~BUSY;
		if(tp->t_line)
			(*linesw[tp->t_line].l_start)(tp);
		else
			uhstart(tp);
	}
}

/*
 * DHU11/DHV11 start (restart) routine
 */

uhstart(tp)
register struct tty *tp;
{
	register struct device *addr;
	register nch;
	int s, d;

/* fifo code - not used
	int free;
	char *cp = tp->t_outq.c_cf;
	union {
		int x;
		char y[2];
	} wdbuf;
*/

/*
 * Find the minor device number from the TTY pointer,
 * the old way will not work because the TTY structure
 * assignments may not be in order.
 */
	s = spl5();
	for(d=0; d<nuh11; d++)
		if(uh11[d] == tp)
			break;
	addr = (struct device *)tp->t_addr;
	/*
	 * If it's currently active, or delaying,
	 * no need to do anything.
	 */
	if (tp->t_state&(TIMEOUT|BUSY|TTSTOP))
		goto out;

	/*
	 * If the writer was sleeping on output overflow,
	 * wake him when low tide is reached.
	 */
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
	if (nch) {		/* dma transfer */
		addr->uhcsrl = ((d & uh_mask)|RINT);
		addr->uhcsrh = XINT;
		if (addr->uhctl & XABT)
			addr->uhctl &= ~(XABT);
		uhcc[d] = nch;		/* What is this used for ? */
#ifndef	UCB_CLIST
		if(ubmaps)	/* must offset address if UB map used */
			addr->uhxba = (char *)tp->t_outq.c_cf - (char *)&cfree;
		else
			addr->uhxba = tp->t_outq.c_cf;
		addr->uhxcnt = nch;
		addr->uhxba2h = TENA;
		addr->uhxba2l = START;
#else	UCB_CLIST
		if(ubmaps) {	/* must offset address if UB map used */
			addr->uhxba = CLIST_UBADDR + tp->t_outq.c_cf - (char *)cfree;
			addr->uhxcnt = nch;
			addr->uhxba2h = TENA;
			addr->uhxba2l = START;
		} else {
			long paddr;
			paddr = clstaddr + (long)(tp->t_outq.c_cf - SEG5);
			addr->uhxba = loint(paddr);
			addr->uhxcnt = nch;
			addr->uhxba2h = TENA;
			addr->uhxba2l = START|(hiint(paddr)&077);
		}
#endif	UCB_CLIST
		tp->t_state |= BUSY;
	}
/* fifo code - not used 
	if (nch) {
		addr->uhcsrl = ((d & uh_mask)|RINT);
		addr->uhcsrh = XINT;
		free = addr->uh6.uh6b.uhsize;
		if (nch > free)
			nch = free;
		else
			free = nch;
		while (free > 1) {
			wdbuf.y[0] = *cp++;
			wdbuf.y[1] = *cp++;
			addr->uh6.uhdata = wdbuf.x;
			free -= 2;
		}
		if (free == 1)
			addr->uh6.uhdata = *cp++;
		ndflush(&tp->t_outq, nch);
		tp->t_state |= BUSY;
	}
 end of not used fifo code */
    out:
	splx(s);
}

/*
 * Stop output on a line
 */

uhstop(tp, flag)
register struct tty *tp;
{
	register struct device *addr;
	register d, s;

	addr = (struct device *)tp->t_addr;
	s = spl6();
	if(tp->t_state & BUSY) {
		d = minor(tp->t_dev);
		addr->uhcsrl = ((d & uh_mask) | RINT);
		if((tp->t_state & TTSTOP)==0) {
			tp->t_state |= FLUSH;
		}
/* below was for hardware flow control 
		if(!(addr->uhctl & XFLOW))
			addr->uhctl |= XABT;
*/
		addr->uhctl |= XABT;
	}
	splx(s);
}

/*
 * timer routine
 *
 * This routine is only used for the dhu11
 *
 */

uhtimer(dev)
{
	register d, cc;
	register struct device *addr;
	int x;

	addr = io_csr[UH_RMAJ];
	d = 0;
	do {
		cc = uhchars[d];
		uhchars[d] = 0;
		if(cc > 50)
			x = 10;
		else if(cc >16)
				x = 5;
			else
				x = 1;
		addr->uh2.uhrit = x;
		addr += 1;
		uhrint(d++);
	} while (d < (nuh11+15)/16);
	timeout(uhtimer, (caddr_t)0, DHUTIME);
}
