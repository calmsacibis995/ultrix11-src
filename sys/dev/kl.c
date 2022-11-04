
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)kl.c	3.0	4/21/86
 */
/*
 *	Unix/v7m KL/DL-11 driver
 *
 *	Fred Canter 1/24/82
 *
 *	Modified to check exclusive bit and call tty routines through
 *	the linesw[] table.	-Dave Borman, 1/30/84
 *
 *	Added code to send break if arg to ioctl cmd TCSBRK is 0
 *	(system V). George Mathew: 7/10/85
 */
#include <sys/param.h>
#include <sys/conf.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/tty.h>
#include <sys/systm.h>
#include <sys/devmaj.h>

#define	DONE	0200
#define	IENABLE	0100
#define	MAINT	04
#define DTR	02
#define	RDRENB	01
#define	DLDELAY	4	/* Extra delay for DL's (double buff) */

/*
 * The base addresses are now in the io_csr[] array in c.c
 * and are accessed via the major device number as follows:
 *
 *	CO_RMAJ		CONSOLE
 *	KL_RMAJ		KL11 & DL11-A/B
 *	DL_RMAJ		DL11-C/D/E
 */

int	io_csr[];
int	nkl11;	/* (c.c) - number of KL11/DL11-A/B/C including console */
int	ndl11;	/* (c.c) - number of DL11-E */

#define	NL1	000400
#define	NL2	001000
#define	CR2	020000
#define	FF1	040000
#define	TAB1	002000

struct	tty *kl11[];
int	klstart();
int	ttrstrt();
char	partab[];

struct device {
	int	rcsr;
	int	rbuf;
	int	tcsr;
	int	tbuf;
};

klopen(dev, flag)
dev_t dev;
{
	register struct device *addr;
	register struct tty *tp;
	register d;

	d = minor(dev);
	if((d >= nkl11+ndl11) || ((tp = kl11[d]) == 0)) {
		u.u_error = ENXIO;
		return;
	}
	/*
	 * set up minor 0 to address KLADDR
	 * set up minor 1 thru nkl11-1 to address from KLBASE
	 * set up minor nkl11 on to address from DLBASE
	 */
	if(d == 0)
		addr = io_csr[CO_RMAJ];
	else if(d < nkl11) {
		addr = io_csr[KL_RMAJ];
		addr += (d - 1);
	} else {
		addr = io_csr[DL_RMAJ];
		addr += (d - nkl11);
	}
/*
 * klopen can now be called from klioctl
 * to set maint. loop back mode.
 */
	if(flag == TIOCSMLB) {
		addr->tcsr |= MAINT;
		return;
	}
	if(flag == TIOCCMLB) {
		addr->tcsr &= ~MAINT;
		return;
	}
	tp->t_addr = (caddr_t)addr;
	tp->t_oproc = klstart;
	if ((tp->t_state&ISOPEN) == 0) {
		tp->t_state = ISOPEN|CARR_ON;
		tp->t_flags = EVENP|ECHO|XTABS|CRMOD;
		ttychars(tp);
	} else if (tp->t_state&XCLUDE && u.u_uid!=0) {
		u.u_error = EBUSY;
		return;
	}
	addr->rcsr |= IENABLE|DTR|RDRENB;
	addr->tcsr |= IENABLE;
	ttyopen(dev, tp);
}

klclose(dev, flag)
dev_t dev;
int flag;
{
	register struct tty *tp;

	tp = kl11[minor(dev)];
	(*linesw[tp->t_line].l_close)(tp);
	ttyclose(tp);
}

klread(dev)
dev_t dev;
{
	register struct tty *tp;

	tp = kl11[minor(dev)];
	(*linesw[tp->t_line].l_read)(tp);
}

klwrite(dev)
dev_t dev;
{
	register struct tty *tp;

	tp = kl11[minor(dev)];
	(*linesw[tp->t_line].l_write)(tp);
}

klxint(dev)
dev_t dev;
{
	register struct tty *tp;

	tp = kl11[minor(dev)];
	ttstart(tp);
	if (tp->t_outq.c_cc<=TTLOWAT(tp)) {
		if (tp->t_state&ASLEEP)
			wakeup((caddr_t)&tp->t_outq);
#ifdef	SELECT
		if (tp->t_wsel) {
			selwakeup(tp->t_wsel, tp->t_state & TS_WCOLL);
			tp->t_wsel = 0;
			tp->t_state &= ~TS_WCOLL;
		}
#endif
	}
}

klrint(dev)
dev_t dev;
{
	register int c;
	register struct device *addr;
	register struct tty *tp;
	int	pri;

	tp = kl11[minor(dev)];
	addr = (struct device *)tp->t_addr;
	c = addr->rbuf;
	if(c < 0) {
		tp->t_errcnt++;		/* count errors on each line */
		tp->t_lastec = c;	/* save last error character */
	}
/*
 * Setting reader enable after each character causes
 * characters to be lost because it clears receiver done.
 * This is not a problem for characters being typed on the
 * terminal, but does cause errors when the (CMX) communications
 * device exerciser is running because CMX sends long messages
 * at full speed ! If the DL is to be used with an ASR 33 teletype
 * then the reader enable instruction should be replaced.
	addr->rcsr |= RDRENB;
 */
	(*linesw[tp->t_line].l_rint)(c, tp);
}

klioctl(dev, cmd, addr, flag)
caddr_t addr;
dev_t dev;
{
	register struct tty *tp;
	int s;
	extern wakeup();
	extern int hz;

	/*
	 * We are not guaranteed a valid device, since TIOCLOCAL
	 * can be called through the ttlocl() sytem call. Thus,
	 * we have to check for invalid device numbers.
	 */
	if((minor(dev) >= nkl11+ndl11) || ((tp = kl11[minor(dev)]) == 0)) {
		u.u_error = ENXIO;
		return;
	}
	cmd = (*linesw[tp->t_line].l_ioctl)(tp, cmd, addr, flag);
	if (cmd == 0 || cmd == TIOCSETP || cmd == TIOCSETN)
		return;
	if (cmd == TIOCSDTR) {
		((struct device *)tp->t_addr)->rcsr |= DTR;
	} else if (cmd == TIOCCDTR) {
		((struct device *)tp->t_addr)->rcsr &= ~DTR;
	} else if (cmd == TIOCSBRK) {
		((struct device *)tp->t_addr)->tcsr |= 1;
	} else if (cmd == TIOCCBRK) {
		((struct device *)tp->t_addr)->tcsr &= ~1;
	} else if (cmd == TIOCLOCAL) {
		/*
		 * it doesn't do anything, but let the luser
		 * know that he can't do this anyway...
		 */
		if (u.u_uid)
			u.u_error = EPERM;
	} else if (cmd == TIOCSMLB || cmd == TIOCCMLB) {
		if (u.u_uid)
			u.u_error = EPERM;
		else
			klopen(dev, cmd);
	} else if (cmd == TIOTCSBRK) {    /* if arg is 0 for TCSBRK cmd 
					   * George Mathew: 7/10/85 */
		((struct device *)tp->t_addr)->tcsr |= 1;
		/*tp->t_state |= TIMEOUT; is this required??? */
		s = spl5();
		timeout(wakeup,(caddr_t)tp,hz/4);
		sleep((caddr_t)tp,PZERO-1);
		splx(s);
		((struct device *)tp->t_addr)->tcsr &= ~1;
	} else {
		u.u_error = ENOTTY;
	}
}

klstart(tp)
register struct tty *tp;
{
	register c;
	register struct device *addr;

	addr = (struct device *)tp->t_addr;
	if((addr->tcsr&DONE) == 0)
		return;
	if ((c=getc(&tp->t_outq)) >= 0) {
		if (tp->t_flags&RAW)
			addr->tbuf = c;
		else if (c<=0177)
			addr->tbuf = c | (partab[c]&0200);
		else {
			timeout(ttrstrt, (caddr_t)tp, (c&0177) + DLDELAY);
			tp->t_state |= TIMEOUT;
		}
	}
}

/*
 * The mbuf_off variable allows messages from internal
 * printf() to be turned off/on by toggling <CTRL/O>.
 * The driver used to not prinf if the last char in the
 * rbuf was break or null, this caused problems on the
 * Micro/PDP-11s. The first boot after powering on the
 * terminal would loose the mem messages -- Fred 8/16/84
 */
int	mbuf_off;		/* 0 for ON, 1 for OFF */
int	msgbufs;		/* size of MSGBUF, see c.c */
char	*msgbufp = msgbuf;	/* Next saved printf character */
/*
 * Print a character on console.
 * Attempts to save and restore device
 * status.
 *
 * The last MSGBUFS characters
 * are saved in msgbuf for inspection later.
 */
putchar(c)
register c;
{
	register struct device *kladdr;
	register timo;
	int s;

	kladdr = io_csr[CO_RMAJ];
	if (c != '\0' && c != '\r' && c != 0177) {
		*msgbufp++ = c;
		if(msgbufp >= &msgbuf[msgbufs])
			msgbufp = msgbuf;
	}
	/*
	 * If last char was <CTRL/K> set mbuf_off flag,
	 * if last char was <CTRL/A> clear mbuf_off flag,
	 * don't print if mbuf_off is non zero.
	 */
	if ((kladdr->rbuf&0177) == CTRL(k)) {
		mbuf_off = 1;
	}
	if ((kladdr->rbuf&0177) == CTRL(a)) {
		mbuf_off = 0;
	}
	if(mbuf_off)
		return;
	timo = 30000;
	/*
	 * Try waiting for the console tty to come ready,
	 * otherwise give up after a reasonable time.
	 */
	while((kladdr->tcsr&0200) == 0)
		if(--timo == 0)
			break;
	if(c == 0)
		return;
	s = kladdr->tcsr;
	kladdr->tcsr = 0;
	kladdr->tbuf = c;
	if(c == '\n') {
		putchar('\r');
		putchar(0177);
		putchar(0177);
	}
	putchar(0);
	kladdr->tcsr = s;
}
