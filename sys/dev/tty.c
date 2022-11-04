
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)tty.c	3.5	1/14/88
 */
/*
 * TTY subroutines common to more than one line discipline
 *
 * Added TCSBRK command (System V)
 * Added VMIN/VTIME
 *	George Mathew:  7/10/85
 *
 * OHMS - 5/13/85 - added System V compatability.
 *	ttioctl moved to _ttioctl
 *	new ttioctl translates sysV to/from ULTRIX-11
 * 
 * The ttlocl() system call was added,
 * to allow for local terminal operation.
 *
 * Fred Canter 1/3/82
 */
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/dir.h>
#include <sys/user.h>
#define	OLDTIOC
#include <sys/tty.h>
#include <sys/proc.h>
#include <sys/inode.h>
#include <sys/file.h>
#include <sys/reg.h>
#include <sys/conf.h>

/*
 * Define for SVID termio fixes, i.e.,
 * make vmin and vtime work in raw and cbreak modes.
 * Must also define in ttynew.c.
 * 1/8/88 -- Fred Canter
 */
#define	TERMIO_FIX

#define LOCAL 01

#ifdef BVNTTY

int	ntty = BVNTTY;
struct	tty	tty_ts[BVNTTY];

#endif BVNTTY

extern	char	partab[];


/*
 * Input mapping table-- if an entry is non-zero, when the
 * corresponding character is typed preceded by "\" the escape
 * sequence is replaced by the table value.  Mostly used for
 * upper-case only terminals.
 */

char	maptab[] ={
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,'|',000,000,000,000,000,'`',
	'{','}',000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,'~',000,
	000,'A','B','C','D','E','F','G',
	'H','I','J','K','L','M','N','O',
	'P','Q','R','S','T','U','V','W',
	'X','Y','Z',000,000,000,000,000,
};

short	tthiwat[16] =
   { 100,100,100,100,100,100,100,200,200,400,400,400,650,650,650,650 };
short	ttlowat[16] =
   {  30, 30, 30, 30, 30, 30, 30, 50, 50,120,120,120,125,125,125,125 };

#define	OBUFSIZ	100

/*
 * shorthand
 */
#define	q1	tp->t_rawq
#define	q2	tp->t_canq
#define	q3	tp->t_outq
#define	q4	tp->t_un.t_ctlq


/*
 * Had to move three routines from ttynew.c to tty.c
 * if TERMIO_FIX defined, because root text segment
 * overflowed on bedrock (max config test case).
 */
#ifdef	TERMIO_FIX
/*
 * routine called on switching to NTTYDISC
 * where is the called from?? - 
 * there is no vector through the linesw?? - daver
 */
ntyopen(tp)
register struct tty *tp;
{
	if (tp->t_line != NTTYDISC)
		wflushtty(tp);
}

/*
 * clean tp on exiting NTTYDISC
 * called through the linesw table - daver
 */
ntyclose(tp)
struct tty *tp;
{
	wflushtty(tp);
}

/*
 * reinput pending characters after state switch
 * call at spl5().
 */
ntypend(tp)
register struct tty *tp;
{
	struct clist tq;
	register c;

	tp->t_local &= ~LPENDIN;
	tp->t_lstate |= LSTYPEN;
	tq = tp->t_rawq;
	tp->t_rawq.c_cc = 0;
	tp->t_rawq.c_cf = tp->t_rawq.c_cl = NULL;
	while ((c = getc(&tq)) >= 0)
		ntyinput(c, tp);
	tp->t_lstate &= ~LSTYPEN;
}
#endif	TERMIO_FIX

/*
 * routine called on first teletype open.
 * establishes a process group for distribution
 * of quits and interrupts from the tty.
 */
ttyopen(dev, tp)
dev_t dev;
register struct tty *tp;
{
	register struct proc *pp;

	pp = u.u_procp;
	tp->t_dev = dev;
	if(pp->p_pgrp == 0) {
		u.u_ttyp = tp;
		u.u_ttyd = dev;
		if (tp->t_pgrp==0)
			tp->t_pgrp = pp->p_pid;
		pp->p_pgrp = tp->t_pgrp;
	}
	if ((tp->t_state & ISOPEN) == 0)
		tp->t_line = DFLT_LDISC;
	tp->t_state &= ~WOPEN;
	tp->t_state |= ISOPEN;
	(*linesw[tp->t_line].l_open)(tp);
}

/*
 * clean tp on last close
 */
ttyclose(tp)
register struct tty *tp;
{
	tp->t_pgrp = 0;
	wflushtty(tp);
	tp->t_state = 0;
	tp->t_line = DFLT_LDISC;
}

/*
 * set default control characters.
 *
 * The definition FSTTY causes the DEC tty control
 * characters to be the default for the field service
 * version of Unix used with USEP.
 ****** FSTTY NOT CURRENTLY USED ******
 */
ttychars(tp)
register struct tty *tp;
{
#ifdef FSTTY
	tun.t_intrc = 03;	/* control C */
#else
	tun.t_intrc = CINTR;
#endif FSTTY
	tun.t_quitc = CQUIT;
	tun.t_startc = CSTART;
	tun.t_stopc = CSTOP;
	tun.t_eofc = CEOT;
	tun.t_brkc = CBRK;
	tun.t_vmin = CMIN;
	tun.t_vtime = CTIME;
#ifdef FSTTY
	tp->t_erase = 0177;	/* delete */
#else
	tp->t_erase = CERASE;
#endif FSTTY
#ifdef FSTTY
	tp->t_kill = 034;	/* control U */
#else
	tp->t_kill = CKILL;
#endif FSTTY
	tlun.t_suspc = CTRL(z);
	tlun.t_dsuspc = CTRL(y);
	tlun.t_rprntc = CTRL(r);
	tlun.t_flushc = CTRL(o);
	tlun.t_werasc = CTRL(w);
	tlun.t_lnextc = CTRL(v);
	tp->t_local = 0;
	tp->t_lstate = 0;
}

/*
 * Wait for output to drain, then flush input waiting.
 */
wflushtty(tp)
register struct tty *tp;
{

	spl5();
	while (tp->t_outq.c_cc && tp->t_state&CARR_ON) {
		/*
		 * This used to blindly call t_oproc. What if
		 * it's already busy? Better check like we do
		 * in ttstart, or we could screw ourselves with
		 * multiple timeouts... -Dave Borman, 9/28/84
		 */
		if((tp->t_state&(TIMEOUT|TTSTOP|BUSY)) == 0)
			(*tp->t_oproc)(tp);
		tp->t_state |= ASLEEP;
		sleep((caddr_t)&tp->t_outq, TTOPRI);
	}
	flushtty(tp, FREAD|FWRITE);
	spl0();
}

/*
 * flush all TTY queues
 */
flushtty(tp, rw)
register struct tty *tp;
register rw;
{
	register s;

	s = spl6();
	if (rw & FREAD) {
		while (getc(&tp->t_canq) >= 0)
			;
		wakeup((caddr_t)&tp->t_rawq);
	}
	if (rw & FWRITE) {
		tp->t_state &= ~TTSTOP;
		(*cdevsw[major(tp->t_dev)].d_stop)(tp);
		while (getc(&tp->t_outq) >= 0)
			;
		wakeup((caddr_t)&tp->t_outq);
	}
	if (rw & FREAD) {
		while (getc(&tp->t_rawq) >= 0)
			;
		tp->t_delct = 0;
		tp->t_rocount = 0;
		tp->t_rocol = 0;
		tp->t_lstate = 0;
	}
	splx(s);
}

/*
 * Send stop character on input overflow.
 *
 * Fred Canter -- 1/4/88
 * Don't send stop char if input queue already blocked.
 * We were sending an XOFF for each char over TTYHOG/2.
 */
ttyblock(tp)
register struct tty *tp;
{
	register x;
	x = tp->t_rawq.c_cc + tp->t_canq.c_cc;
	if (tp->t_rawq.c_cc > TTYHOG) {
		flushtty(tp, FREAD|FWRITE);
		tp->t_state &= ~TBLOCK;
	}
	if ((tp->t_state&TBLOCK == 0) && (x >= TTYHOG/2)) {
		if (putc(tun.t_stopc, &tp->t_outq)==0) {
			tp->t_state |= TBLOCK;
			ttstart(tp);
		}
	}
}

/*
 * Restart typewriter output following a delay
 * timeout.
 * The name of the routine is passed to the timeout
 * subroutine and it is called during a clock interrupt.
 */
ttrstrt(tp)
register struct tty *tp;
{

	tp->t_state &= ~TIMEOUT;
	ttstart(tp);
}

/*
 * Start output on the typewriter. It is used from the top half
 * after some characters have been put on the output queue,
 * from the interrupt routine to transmit the next
 * character, and after a timeout has finished.
 */
ttstart(tp)
register struct tty *tp;
{
	register s;

	s = spl5();
	if((tp->t_state&(TIMEOUT|TTSTOP|BUSY)) == 0)
		(*tp->t_oproc)(tp);
	splx(s);
}

/*
 * Ioctl preprocessor, allows handling of System V style ioctl's for tty's
 *
 * If a System V call process data, else call directy on _ttioctl.
 *
 * OHMS 5/11/85
 */
/*ARGSUSED*/
ttioctl(tp, com, addr, flag)
register struct tty *tp;
caddr_t addr;
{
	struct sgttyb iocb;	/* place for sgttyb data */
	struct termio tiocb;	/* place for termio data */
	struct tchars tc;	/* place for tchars data */
	struct ltchars ltc;	/* place for ltchars data */
	int retval;

	/*
	 * This is especially so that isatty() will
	 * fail when carrier is gone.
	 */
	if ((com != TIOCLOCAL) && ((tp->t_state&CARR_ON) == 0)) {
		u.u_error = EBADF;
		return(0);
	}
	switch(com) {

	/*
	 * Set parameters - System V style
	 */
	case TCSETA:
	case TCSETAW:
	case TCSETAF:
		if(copyin(addr, (caddr_t)&tiocb, sizeof(tiocb))) {
			u.u_error = EFAULT;
			break;
		}
		if((retval = pack(tp,&tiocb,&iocb,&tc,&ltc,flag)) >= 0) 
			return(retval);
			
		else 
			return(0);
		

	/*
	 * Get parameters - System V style
	 */
	case TCGETA:
		if(unpack(tp,&tiocb,&iocb,&tc,&ltc,flag) >= 0)
			if(copyout((caddr_t)&tiocb, addr, sizeof(tiocb)))
				u.u_error = EFAULT;
		break;

	case TCSBRK:
		ttywait(tp);
		if (addr == 0){
			return(TIOTCSBRK);
		}
		break;

	case TCXONC:
		_ttioctl(tp, addr==0 ? TIOCSTOP : TIOCSTART, addr, flag, LOCAL);
		break;

	case TCFLSH:
		if(addr < 0 || addr > 2) {
			u.u_error = EINVAL;
			break;
		}
		_ttioctl(tp, TIOCFLUSH, addr, flag, LOCAL);
		break;

	default:	/* just passin' thru... */
		return(_ttioctl(tp, com, addr, flag, 0));
	}
	return(0);
}

/*
 * System V tty ioctl syscall translation routines
 *
 * pack:	from termio to ULTRIX-11
 * unpack:	from ULTRIX-11 to termio
 *
 * Taken from BRL package as used on ULTRIX-32.
 * See their ioctl.c in libc.
 * OHMS 6/13/85
 */

static
pack(tp,tiop,sgtp,tc,ltc,flag)
struct tty *tp;
struct termio *tiop;
struct sgttyb *sgtp;
struct tchars *tc;
struct ltchars *ltc;
{
	register int sgflg;
	unsigned int lmw;	/* place for local mode word */

/* tchars */
	if((tiop->c_lflag & T_ICANON) == T_ICANON) {
		if((tc->t_eofc = tiop->c_cc[VEOF]) == 0)
			tc->t_eofc = (char)-1;
		if((tc->t_brkc = tiop->c_cc[VEOL]) == 0)
			tc->t_brkc = (char)-1;
	} else if(_ttioctl(tp, TIOCGETC, (char *)tc,flag,LOCAL) < 0) 
		return(-1);
	if(tiop->c_lflag & (T_ICANON | T_ISIG) == T_ICANON) {
		tc->t_intrc = tc->t_quitc = (char)-1;
	} else {
		if((tc->t_intrc = tiop->c_cc[VINTR]) == 0)
			tc->t_intrc = (char)-1;
		if((tc->t_quitc = tiop->c_cc[VQUIT]) == 0)
			tc->t_quitc = (char)-1;
	}
	if((tiop->c_iflag & T_IXON) != T_IXON) {
		tc->t_startc = tc->t_stopc = (char)-1;
	} else {
		tc->t_startc = CSTART;
		tc->t_stopc = CSTOP;
	}
	if((tc->t_vmin = tiop->c_cc[VMIN]) < 0)
		tc->t_vmin = CMIN;
	if((tc->t_vtime = tiop->c_cc[VTIME]) < 0)
		tc->t_vtime = CTIME;
	if(_ttioctl(tp, TIOCSETC, (char *)tc, flag, LOCAL) < 0) 
		return(-1);
/* ltchars */
	if(_ttioctl(tp, TIOCGLTC, (char *)ltc, flag, LOCAL) == 0) {
		if((ltc->t_suspc = tiop->c_cc[VSWTCH]) == 0)
			ltc->t_suspc = (char)-1;
		if(_ttioctl(tp, TIOCSLTC, (char *)ltc, flag, LOCAL) < 0) 
			return(-1);
	}
	if((tiop->c_cflag & T_HUPCL) == T_HUPCL)
		tp->t_state |= HUPCLS;
	else
		tp->t_state &= ~HUPCLS;
/* sgttyb */
	sgtp->sg_erase = tiop->c_cc[VERASE];
	sgtp->sg_kill = tiop->c_cc[VKILL];
	sgtp->sg_ispeed = sgtp->sg_ospeed = (tiop->c_cflag & T_CBAUD);
	sgflg = 0;		/* initialize */
	if((tiop->c_lflag & T_ICANON) != T_ICANON)
		sgflg |= CBREAK;
	if((tiop->c_lflag & T_XCASE) == T_XCASE)
		sgflg |= LCASE;
	if((tiop->c_lflag & T_ECHO) == T_ECHO)
		sgflg |= ECHO;
	if((tiop->c_cflag & T_PARODD) == T_PARODD)
		sgflg |= ODDP;
	else if((tiop->c_iflag & T_INPCK) == T_INPCK)
		sgflg |= EVENP;
	else
		sgflg |= ANYP;
	if((tiop->c_oflag & T_ONLCR) == T_ONLCR) {
		sgflg |= CRMOD;
		if((tiop->c_oflag & T_CRDLY) == T_CR1)
			sgflg |= NL1;
		else if((tiop->c_oflag & T_CRDLY) == T_CR2)
			sgflg |= CR1;
		else if((tiop->c_oflag & T_CRDLY) != 0)
			sgflg |= CR2;
	} else if((tiop->c_oflag & T_ONLRET) == T_ONLRET) {
		if((tiop->c_oflag & T_CR2) == T_CR2)
			sgflg |= NL2;
		else if((tiop->c_oflag & T_CR1) == T_CR1)
			sgflg |= NL1;
	} else if((tiop->c_oflag & T_NLDLY) == T_NLDLY)
			sgflg |= NL2;
	sgflg |= ((tiop->c_oflag & T_TABDLY) >> 1);
	if((tiop->c_oflag & (T_VTDLY | T_FFDLY)) != 0)
		sgflg |= VTDELAY;
	if((tiop->c_oflag & T_BSDLY) == T_BSDLY)
		sgflg |= BSDELAY;
	if((tiop->c_iflag & T_IXOFF) == T_IXOFF)
		sgflg |= TANDEM;
	sgtp->sg_flags = sgflg;		

/* local mode word */
	lmw = LCTLECH;
	if((tiop->c_lflag & T_ECHOE) == T_ECHOE) {
		lmw |= LCRTBS;
		if((tiop->c_cflag & T_CBAUD) >= B1200)
			lmw |= (LCRTERA | LCRTKIL);
	} else
		lmw |= LPRTERA;
	/*
	if((tiop->c_lflag & T_NOFLSH) == T_NOFLSH)
		lmw |=  XXX;
	*/
	if((tiop->c_cflag & T_CLOCAL) == T_CLOCAL)
		lmw |= (LMDMBUF | LNOHANG);
	if((tiop->c_iflag & (T_IXON | T_IXANY)) == T_IXON)
		lmw |= LDECCTQ;
	if(_ttioctl(tp, TIOCLSET, (char *)&lmw, flag, LOCAL) < 0) 
		return(-1);
	if(_ttioctl(tp, TIOCSETP, (char *)sgtp, flag, LOCAL) < 0) 
		return(-1);
	
	/* set line discipline */
	if (tiop->c_line != tp->t_line) {
		if(_ttioctl(tp,TIOCSETD,tiop->c_line,flag,LOCAL) < 0)
			return(-1);
	}
	/* must return TIOCSETP for device drivers to change params */
	return(TIOCSETP);
}

static
unpack(tp,tiop,sgtp,tc,ltc,flag)
struct tty *tp;
register struct termio *tiop;
register struct sgttyb *sgtp;
struct tchars *tc;
struct ltchars *ltc;
{
	unsigned int lmw;	/* place for local mode word */

/* local mode word */
	if(_ttioctl(tp, TIOCLGET, (char *)&lmw, flag, LOCAL) < 0) 
		return(-1);
/* tchars */
	if(_ttioctl(tp, TIOCGETC, (char *)tc, flag, LOCAL) < 0 ) 
		return(-1);		/* errno already set */
	if(tc->t_intrc == (char)-1 && tc->t_quitc == (char)-1) {			/* assume defaults */
		tiop->c_cc[VINTR] = CINTR;
		tiop->c_cc[VQUIT] = CQUIT;
		tiop->c_lflag = 0;	/* no ISIG */
	} else	{
		if((tiop->c_cc[VINTR] = tc->t_intrc) == (char)-1)
			tiop->c_cc[VINTR] = CNUL;
		if((tiop->c_cc[VQUIT] = tc->t_quitc) == (char)-1)
			tiop->c_cc[VQUIT] = CNUL;
		tiop->c_lflag = T_ISIG;
	}
	if(tc->t_startc == (char)-1 && tc->t_stopc == (char)-1)
		tiop->c_iflag = 0;	/* no T_IXON */
	else
		tiop->c_iflag = (lmw & LDECCTQ) != 0 ? T_IXON
						       : T_IXON | T_IXANY;
	if((tiop->c_cc[VEOF] = tc->t_eofc) == (char)-1)
		tiop->c_cc[VEOF] = CNUL;
	if((tiop->c_cc[VEOL] = tc->t_brkc) == (char)-1)
		tiop->c_cc[VEOL] = CNUL;
	tiop->c_cc[VEOL2] = CNUL;
	if ((tiop->c_cc[VMIN] = tc->t_vmin) < 0) {
		tiop->c_cc[VMIN] = CMIN;
	}
	if ((tiop->c_cc[VTIME] = tc->t_vtime) < 0) {
		tiop->c_cc[VTIME] = CTIME;
	}

/* sgtty */
	if(_ttioctl(tp, TIOCGETP, (char *)sgtp, flag, LOCAL) < 0 ) 
		return(-1);		/* errno already set */
	tiop->c_cc[VERASE] = sgtp->sg_erase;
	tiop->c_cc[VKILL] = sgtp->sg_kill;
/* ltchars */
	if(_ttioctl(tp, TIOCGLTC, (char *)ltc, flag, LOCAL) < 0
					/* old tty handler */
	  || (tiop->c_cc[VSWTCH] = ltc->t_suspc) == (char)-1)
		tiop->c_cc[VSWTCH] = CNUL;
	tiop->c_oflag = (sgtp->sg_flags & TBDELAY) << 1;
	if((tiop->c_cflag = sgtp->sg_ispeed & T_CBAUD | T_CREAD)
	      == (B110 | T_CREAD))
		tiop->c_cflag |= T_CSTOPB;
	if((lmw & (LMDMBUF | LNOHANG)) != 0)
		tiop->c_cflag |= T_CLOCAL;
	if((sgtp->sg_flags & LCASE) != 0) {
		tiop->c_iflag |= T_IUCLC;
		tiop->c_oflag |= T_OLCUC;
		tiop->c_lflag |= T_XCASE;
	}
	if((sgtp->sg_flags & ECHO) != 0)
		tiop->c_lflag |= T_ECHO;
	/*
	if((sgtp->sg_flags & X_NOFLSH) != 0)
		tiop->c_lflag |= T_NOFLSH;
	else
		tiop->c_lflag |= T_ECHOK;
	*/
	if((sgtp->sg_flags & CRMOD) != 0) {
		tiop->c_iflag |= T_ICRNL;
		tiop->c_oflag |= T_ONLCR;
		if((sgtp->sg_flags & NL2) != 0)	/* NL2 or NL3 */
			tiop->c_oflag |= T_NL1;
		else if((sgtp->sg_flags & NL1) != 0) /* NL1 */
			tiop->c_oflag |= T_CR1;
		else if((sgtp->sg_flags & CR2) != 0) /* CR2 or CR3 */
			tiop->c_oflag |= T_ONOCR | T_CR3;	/* approx. */
		else if((sgtp->sg_flags & CR1) != 0) /* CR1 */
			tiop->c_oflag |= T_ONOCR | T_CR2;	/* approx. */
	} else {
		tiop->c_oflag |= T_ONLRET;
		if((sgtp->sg_flags & NL1) != 0)
			tiop->c_oflag |= T_CR1;
		if((sgtp->sg_flags & NL2) != 0)
			tiop->c_oflag |= T_CR2;
	}
	if((sgtp->sg_flags & VTDELAY) != 0)
		tiop->c_oflag |= T_FF1 | T_VT1;
	if((sgtp->sg_flags & BSDELAY) != 0)
		tiop->c_oflag |= T_BS1;
	if((sgtp->sg_flags & RAW) != 0) {
		tiop->c_cflag |= T_CS8;
		tiop->c_iflag &= ~(T_ICRNL | T_IUCLC);
		tiop->c_lflag &= ~T_ISIG;
	} else {
		tiop->c_cflag |= T_CS7 | T_PARENB;
		tiop->c_iflag |= T_BRKINT | T_IGNPAR | T_INPCK | T_ISTRIP;
		tiop->c_oflag |= T_OPOST;
		if((sgtp->sg_flags & CBREAK) == 0)
			tiop->c_lflag |= T_ICANON;
	}
	if((sgtp->sg_flags & ODDP) != 0)
		if((sgtp->sg_flags & EVENP) != 0)
			tiop->c_iflag &= ~T_INPCK;
		else
			tiop->c_cflag |= T_PARODD;
	if((lmw & LCRTBS) != 0)
		tiop->c_lflag |= T_ECHOE;
	if((sgtp->sg_flags & TANDEM) != 0)
		tiop->c_iflag |= T_IXOFF;

	/* get line discipline */
	tiop->c_line = tp->t_line;

	if((tp->t_state & HUPCLS) == HUPCLS) 
		tiop->c_cflag |= T_HUPCL;
	else
		tiop->c_cflag &= ~T_HUPCL;
	return(0);
}

/*
 * This function was previously called ttioctl.
 * ttioctl now front ends this code to provide for System V style ioctls.
 * OHMS 5/13/85
 *
 * Common code for tty ioctls.
 * Return values:
 *	cmd			cmd is not done (driver must do some/all)
 *	0			cmd is done
 */
/*ARGSUSED*/
static
_ttioctl(tp, com, addr, flag, localflg)
register struct tty *tp;
caddr_t addr;
{
	register dev_t dev;
	unsigned t;
	struct sgttyb iocb;	/* space for copyin of sgttyb */
	struct sgttyb *iocbp;	/* pointer to iocb */
	struct clist tq;
	extern int nldisp;
	register c;
	int temp;
	extern nodev();
	int savflg;
	struct tchars *tcp;
	struct ltchars *ltcp;
#ifdef	TERMIO_FIX
	struct	tchars	tc;
#endif	TERMIO_FIX

	/* NOW DONE IN IOCTL PREPROCESSOR
	 * This is especially so that isatty() will
	 * fail when carrier is gone.
	if ((com != TIOCLOCAL) && ((tp->t_state&CARR_ON) == 0)) {
		u.u_error = EBADF;
		return(0);
	}
	 */

	dev = tp->t_dev;
	if(localflg == LOCAL)
		iocbp = addr;		/* iocb in ttioctl (sysV compat) */
	else
		iocbp = &iocb;		/* use _ttioctl iocb */
	/*
	 * If the ioctl involves modification,
	 * hang if in the background.
	 */
	switch(com) {
	case TIOCSETD:
	case TIOCSETP:
	case TIOCSETN:
	case TIOCFLUSH:
	case TIOCSETC:
#ifdef	TERMIO_FIX
	case TIOCSETV:
#endif	TERMIO_FIX
	case TIOCSLTC:
	case TIOCSPGRP:
	case TIOCLBIS:
	case TIOCLBIC:
	case TIOCLSET:
	case TIOCSTI:
		while (tp->t_line == NTTYDISC &&
		    u.u_procp->p_pgrp != tp->t_pgrp && tp == u.u_ttyp &&
#	ifdef	VIRUS_VFORK
		    (u.u_procp->p_flag&SVFORK) == 0 &&
#	endif
		    u.u_signal[SIGTTOU] != SIG_IGN &&
		    u.u_signal[SIGTTOU] != SIG_HOLD &&
		    (u.u_procp->p_flag&SDETACH)==0) {
			gsignal(u.u_procp->p_pgrp, SIGTTOU);
			sleep((caddr_t)&lbolt, TTOPRI);
		}
		break;
	}

	/*
	 * Process the ioctl.
	 */

	switch(com) {

	/*
	 * Get discipline number
	 */
	case TIOCGETD:
		t = tp->t_line;
		if (copyout((caddr_t)&t, addr, sizeof(t)))
			u.u_error = EFAULT;
		break;

	/*
	 * Set line discipline
	 */
	case TIOCSETD:
		if (localflg != LOCAL) {   /* if not SYSV */
			if (copyin(addr, (caddr_t)&t, sizeof(t))) {
				u.u_error = EFAULT;
				break;
			}
		}
		else t = addr;
		if ((t >= nldisp) || (linesw[t].l_open == nodev)) {
			u.u_error = ENXIO;
			break;
		}
		if (t != tp->t_line) {
			spl5();
			(*linesw[tp->t_line].l_close)(tp);
			(*linesw[t].l_open)(tp);
			if (u.u_error==0)
				tp->t_line = t;
			spl0();
		}
		break;

	/*
	 * prevent more opens on channel
	 */
	case TIOCEXCL:
		tp->t_state |= XCLUDE;
		break;
	case TIOCNXCL:
		tp->t_state &= ~XCLUDE;
		break;

	/*
	 * Set new parameters
	 */
	case TIOCSETP:
	case TIOCSETN:
		if(localflg != LOCAL) {
			if (copyin(addr, (caddr_t)&iocb, sizeof(iocb))) {
				u.u_error = EFAULT;
				break;
			}
		}
		spl5();
		if (tp->t_line == OTTYDISC) {
			if (com == TIOCSETP)
				wflushtty(tp);
		}
		if (tp->t_line == NTTYDISC) {
			if (tp->t_flags&RAW || iocbp->sg_flags&RAW ||
			    com == TIOCSETP)
				wflushtty(tp);
			else if ((tp->t_flags&CBREAK) != (iocbp->sg_flags&CBREAK)) {
				if (iocbp->sg_flags & CBREAK) {
					catq(&tp->t_rawq, &tp->t_canq);
					tq = tp->t_rawq;
					tp->t_rawq = tp->t_canq;
					tp->t_canq = tq;
				} else {
					tp->t_local |= LPENDIN;
					wakeup((caddr_t)&tp->t_rawq);
				}
			}
		}
		if ((tp->t_state&SPEEDS)==0) {
			tp->t_ispeed = iocbp->sg_ispeed;
			tp->t_ospeed = iocbp->sg_ospeed;
		}
		tp->t_erase = iocbp->sg_erase;
		tp->t_kill = iocbp->sg_kill;
		tp->t_flags = iocbp->sg_flags;
#ifdef	TERMIO_FIX
		/*
		 * RAW needs to return if any input available.
		 */
		if (tp->t_flags & RAW) {
			tun.t_vmin = 1;
			tun.t_vtime = 0;
		}
#endif	TERMIO_FIX
		spl0();
		return(com);

	/*
	 * send current parameters to user
	 */
	case TIOCGETP:
		iocbp->sg_ispeed = tp->t_ispeed;
		iocbp->sg_ospeed = tp->t_ospeed;
		iocbp->sg_erase = tp->t_erase;
		iocbp->sg_kill = tp->t_kill;
		iocbp->sg_flags = tp->t_flags;
		if(localflg != LOCAL) {
			if (copyout((caddr_t)&iocb, addr, sizeof(iocb)))
				u.u_error = EFAULT;
		}
		break;

	/*
	 * get internal state bits. Used to check XCLUDE and HUPCLS
	 */
	case TIOCGETS:
		t = tp->t_state;
		if (copyout((caddr_t)&t, addr, sizeof(t)))
			u.u_error = EFAULT;
		break;

	/*
	 * Hang up line on last close
	 */

	case TIOCHPCL:
		tp->t_state |= HUPCLS;
		break;

	case TIOCCHPCL:
		tp->t_state &= ~HUPCLS;
		break;

	case TIOCFLUSH:
		flushtty(tp, FREAD|FWRITE);
		break;

	case TIOCSTOP:
		spl5();
		if((tp->t_state & TTSTOP) == 0) {
			tp->t_state |= TTSTOP;
			(*cdevsw[major(tp->t_dev)].d_stop)(tp);
		}
		spl0();
		break;

	case TIOCSTART:
		spl5();
		if((tp->t_state & TTSTOP) || (tp->t_local & LFLUSHO)) {
			tp->t_state &= ~TTSTOP;
			tp->t_local &= ~LFLUSHO;
			ttstart(tp);
		}
		spl0();
		break;

#ifdef	SELECT
	case FIONBIO: {
		int nbio;
		if (copyin(addr, (caddr_t)&nbio, sizeof(nbio))) {
			u.u_error = EFAULT;
			return(1);
		}
		if (nbio)
			tp->t_state |= TS_NBIO;
		else
			tp->t_state &= ~TS_NBIO;
		break;
	}
#endif	SELECT

	/*
	 * ioctl entries to line discipline
	 */
/*
	case DIOCSETP:
	case DIOCGETP:
		(*linesw[tp->t_line].l_ioctl)(com, tp, addr);
		break;
*/

#ifdef	TERMIO_FIX
	/*
	 * set/fetch System V vmin and vtime to/from tchars
	 * struct without changing rest of tchars.
	 * allows use of vmin/vtime by ULTRIX users in raw mode.
	 */
	case TIOCSETV:
		if(copyin(addr, (caddr_t)&tc, sizeof(struct tchars)))
		{
			u.u_error = EFAULT;
			break;
		}
		tun.t_vmin = tc.t_vmin;
		tun.t_vtime = tc.t_vtime;
		break;

	case TIOCGETV:
		if(copyin(addr, (caddr_t)&tc, sizeof(struct tchars)))
		{
			u.u_error = EFAULT;
			break;
		}
		tc.t_vmin = tun.t_vmin;
		tc.t_vtime = tun.t_vtime;
		if(copyout((caddr_t)&tc, addr, sizeof(struct tchars)))
		{
			u.u_error = EFAULT;
			break;
		}
		break;
#endif	TERMIO_FIX

	/*
	 * set and fetch special characters
	 */
	case TIOCSETC:
		if(localflg != LOCAL) {
#ifdef	TERMIO_FIX
			/* Don't set vmin and vtime, must use TIOCSETV */
			if (copyin(addr,(caddr_t)&tun,sizeof(struct tchars)-2))
#else	TERMIO_FIX
			if (copyin(addr, (caddr_t)&tun, sizeof(struct tchars)))
#endif	TERMIO_FIX
				u.u_error = EFAULT;
		} else {
			tcp = addr;
			tun.t_intrc = tcp->t_intrc;
			tun.t_quitc = tcp->t_quitc;
			tun.t_startc = tcp->t_startc;
			tun.t_stopc = tcp->t_stopc;
			tun.t_eofc = tcp->t_eofc;
			tun.t_brkc = tcp->t_brkc;
			tun.t_vmin = tcp->t_vmin;
			tun.t_vtime = tcp->t_vtime;
		}
		break;

	case TIOCGETC:
		if(localflg != LOCAL) {
#ifdef	TERMIO_FIX
			/* Don't get vmin and vtime, must use TIOCGETV */
			if (copyout((caddr_t)&tun,addr,sizeof(struct tchars)-2))
#else	TERMIO_FIX
			if (copyout((caddr_t)&tun, addr, sizeof(struct tchars)))
#endif	TERMIO_FIX
				u.u_error = EFAULT;
		} else {
			tcp = addr;
			tcp->t_intrc = tun.t_intrc;
			tcp->t_quitc = tun.t_quitc;
			tcp->t_startc = tun.t_startc;
			tcp->t_stopc = tun.t_stopc;
			tcp->t_eofc = tun.t_eofc;
			tcp->t_brkc = tun.t_brkc;
			tcp->t_vmin = tun.t_vmin;
			tcp->t_vtime = tun.t_vtime;
		}
		break;
/*
 * The TIOCETQ ioctl function was added for multi terminal service.
 * It returns the byte count of the raw or the canonical queue,
 * which ever is greater, in the flags field.
 */
	case TIOCETQ:
		savflg = iocbp->sg_flags;
		iocbp->sg_flags = (tp->t_rawq.c_cc > tp->t_canq.c_cc)?
			tp->t_rawq.c_cc : tp->t_canq.c_cc;
		if(localflg != LOCAL) {
			if(copyout((caddr_t)&iocb, addr, sizeof(iocb)))
				u.u_error = EFAULT;
		}
		iocbp->sg_flags = savflg;
		break;

/*
 * The next two ioctl functions deal with the terminal type flag byte.
 * This byte is used for erase/kill processing.
 * TIOCSETT sets the flag byte.
 * TIOCGETT returns the flag byte.
 * They are kept here only for compatability and should probably be
 * thrown away sometime in the future.
 */
#define	OCRTBS	01
#define	OCRTERA	02
#define	OPRTERA 04
#define OCRTKIL 010
#define OCTLECH 020
#define OCRTSLO 040
#define	OBITS (LCRTBS|LCRTERA|LPRTERA|LCRTKIL|LCTLECH)
	case TIOCSETT:
		if (copyin(addr, (caddr_t)&t, sizeof(t)))
			u.u_error = EFAULT;
		tp->t_local &= ~OBITS;
		if (t&OCRTBS)	tp->t_local |= LCRTBS;
		if (t&OCRTERA)	tp->t_local |= LCRTERA;
		if (t&OPRTERA)	tp->t_local |= LPRTERA;
		if (t&OCRTKIL)	tp->t_local |= LCRTKIL;
		if (t&OCTLECH)	tp->t_local |= LCTLECH;
		break;

	case TIOCGETT:
		t = 0;
		if (tp->t_local&LCRTBS) t |= OCRTBS;
		if (tp->t_local&LCRTERA) t |= OCRTERA;
		if (tp->t_local&LPRTERA) t |= OPRTERA;
		if (tp->t_local&LCRTKIL) t |= OCRTKIL;
		if (tp->t_local&LCTLECH) t |= OCTLECH;
		if (tp->t_ospeed < B1200) t |= OCRTSLO;
		if (copyout((caddr_t)&t, addr, sizeof(t)))
			u.u_error = EFAULT;
		break;
	/*
	 * Set/get local special characters.
	 */
	case TIOCSLTC:
		if(localflg != LOCAL) {
			if(copyin(addr, (caddr_t)&tlun, sizeof(struct ltchars)))
				u.u_error = EFAULT;
		} else {
			ltcp = addr;
			tlun.t_suspc = ltcp->t_suspc;
			tlun.t_dsuspc = ltcp->t_dsuspc;
			tlun.t_rprntc = ltcp->t_rprntc;
			tlun.t_flushc = ltcp->t_flushc;
			tlun.t_werasc = ltcp->t_werasc;
			tlun.t_lnextc = ltcp->t_lnextc;
			tlun.t_iflushc = ltcp->t_iflushc;
			tlun.t_tstatc = ltcp->t_tstatc;
		}
		break;

	case TIOCGLTC:
		if(localflg != LOCAL) {
			if(copyout((caddr_t)&tlun,addr,sizeof(struct ltchars)))
				u.u_error = EFAULT;
		} else {
			ltcp = addr;
			ltcp->t_suspc = tlun.t_suspc;
			ltcp->t_dsuspc = tlun.t_dsuspc;
			ltcp->t_rprntc = tlun.t_rprntc;
			ltcp->t_flushc = tlun.t_flushc;
			ltcp->t_werasc = tlun.t_werasc;
			ltcp->t_lnextc = tlun.t_lnextc;
			ltcp->t_iflushc = tlun.t_iflushc;
			ltcp->t_tstatc = tlun.t_tstatc;
		}
		break;

	/*
	 * Return number of characters immediately available.
	 */

	case FIONREAD: {
		off_t nread;

		switch (tp->t_line) {
		case OTTYDISC:
		case NTTYDISC:
#ifdef	SELECT
			nread = ttnread(tp);
#else	SELECT
			nread = tp->t_canq.c_cc;
			if (tp->t_flags & (RAW|CBREAK))
				nread += tp->t_rawq.c_cc;
#endif	SELECT
			break;
		}
		if (copyout((caddr_t)&nread, addr, sizeof (off_t)))
			u.u_error = EFAULT;
		break;
	    }
	/*
	 * Should allow SPGRP and GPGRP only if tty open for reading.
	 */
	case TIOCSPGRP:
		if (copyin(addr, (caddr_t)&tp->t_pgrp, sizeof (tp->t_pgrp)))
			u.u_error = EFAULT;
		break;

	case TIOCGPGRP:
		if (copyout((caddr_t)&tp->t_pgrp, addr, sizeof(tp->t_pgrp)))
			u.u_error = EFAULT;
		break;

	/*
	 * Modify local mode word.
	 */
	case TIOCLBIS:
		if (copyin(addr, (caddr_t)&temp, sizeof (tp->t_local)))
			u.u_error = EFAULT;
		else
			tp->t_local |= temp;
		break;
	
	case TIOCLBIC:
		if (copyin(addr, (caddr_t)&temp, sizeof (tp->t_local)))
			u.u_error = EFAULT;
		else
			tp->t_local &= ~temp;
		break;

	case TIOCLSET:
		if(localflg != LOCAL) {
			if (copyin(addr, (caddr_t)&temp, sizeof (tp->t_local)))
				u.u_error = EFAULT;
			else
				tp->t_local = temp;
		} else
			tp->t_local = *((unsigned int *)addr);
		break;

	case TIOCLGET:
		if(localflg != LOCAL) {
		    if(copyout((caddr_t)&tp->t_local,addr,sizeof(tp->t_local)))
			    u.u_error = EFAULT;
		} else
			*((unsigned int *)addr) = tp->t_local;
		break;

	/*
	 * Return number of characters in
	 * the output.
	 */
	case TIOCOUTQ:
		if (copyout((caddr_t)&tp->t_outq.c_cc, addr, sizeof(tp->t_outq.c_cc)))
			u.u_error = EFAULT;
		break;

	case TIOCSTI:
		c = fubyte(addr);
		if (u.u_uid && u.u_ttyp != tp || c < 0)
			u.u_error = EFAULT;
		else
			(*linesw[tp->t_line].l_rint)(c, tp);
		break;

	default:
		return(com);
	}
	return(0);
}

#ifdef	SELECT
static
ttnread(tp)
struct tty	*tp;
{
	int nread = 0;

	if (tp->t_local & LPENDIN)
		ntypend(tp);
	nread = tp->t_canq.c_cc;
	if (tp->t_flags & (RAW|CBREAK))
		nread += tp->t_rawq.c_cc;
	return (nread);
}

ttselect(dev, rw)
dev_t	dev;
int	rw;
{
	register struct tty	*tp = cdevsw[major(dev)].d_ttys[minor(dev)];
	int			nread;
	int			s = spl5();


	if (rw == FREAD) {
		if (ttnread(tp) > 0)
			goto win;
		if (tp->t_rsel && tp->t_rsel->p_wchan == (caddr_t)&selwait)
			tp->t_state |= TS_RCOLL;
		else
			tp->t_rsel = u.u_procp;
	} else if (rw == FWRITE) {
		if (tp->t_outq.c_cc <= TTLOWAT(tp))
			goto win;
		if (tp->t_wsel && tp->t_wsel->p_wchan == (caddr_t)&selwait)
			tp->t_state |= TS_WCOLL;
		else
			tp->t_wsel = u.u_procp;
	}
	splx(s);
	return(0);
win:
	splx(s);
	return(1);
}
#endif	SELECT

static
ttywait(tp)
struct tty *tp;
{
	extern int hz;
	spl5();
	while (tp->t_outq.c_cc || (tp->t_state & (BUSY | TIMEOUT))) {
		tp->t_state |= ASLEEP;
		/* printf("going to sleep in ttywait\n"); */
		sleep((caddr_t)&tp->t_outq,TTOPRI);
	}
	spl0();
	delay(hz/15);
}


#define	PDELAY	(PZERO-1)
delay(ticks)
{
	extern wakeup();

	if (ticks<=0)
		return;
	timeout(wakeup, (caddr_t)u.u_procp+1, ticks);
	sleep((caddr_t)u.u_procp+1, PDELAY);
}
