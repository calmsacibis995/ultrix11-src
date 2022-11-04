
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)ttyold.c	3.0	4/21/86
 */
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/tty.h>
#include <sys/proc.h>
#include <sys/inode.h>
#include <sys/file.h>
#include <sys/reg.h>
#include <sys/conf.h>

extern	char	partab[];
extern	char	maptab[];

/*
 * transfer raw input list to canonical list,
 * doing erase-kill processing and handling escapes.
 * It waits until a full line has been typed in cooked mode,
 * or until any character has been typed in raw mode.
 */
canon(tp)
register struct tty *tp;
{
	register char *bp;
	char *bp1;
	register int c;
	int mc;
	int s;

	s = spl5();

	while ((tp->t_flags&(RAW|CBREAK))==0 && tp->t_delct==0
	    || (tp->t_flags&(RAW|CBREAK))!=0 && tp->t_rawq.c_cc==0) {
			return(-1);
/*
		if ((tp->t_state&CARR_ON)==0) {
			return(-1);
		}
		sleep((caddr_t)&tp->t_rawq, TTIPRI);
*/
	}
	spl0();
loop:
	bp = &canonb[2];
	while ((c=getc(&tp->t_rawq)) >= 0) {
		if ((tp->t_flags&(RAW|CBREAK))==0) {
			if (c==0377) {
				tp->t_delct--;
				break;
			}
			if (bp[-1]!='\\') {
				if (c==tp->t_erase) {
					if (bp > &canonb[2])
						bp--;
					continue;
				}
				if (c==tp->t_kill)
					goto loop;
				if (c==tun.t_eofc)
					continue;
			} else {
				mc = maptab[c];
				if (c==tp->t_erase || c==tp->t_kill)
					mc = c;
				if (mc && (mc==c || (tp->t_flags&LCASE))) {
					if (bp[-2] != '\\')
						c = mc;
					bp--;
				}
			}
		}
		*bp++ = c;
		if (bp>=canonb+canbsiz)
			break;
	}
	bp1 = &canonb[2];
	b_to_q(bp1, bp-bp1, &tp->t_canq);

	if (tp->t_state&TBLOCK && tp->t_rawq.c_cc < TTYHOG/5) {
		if (putc(tun.t_startc, &tp->t_outq)==0) {
			tp->t_state &= ~TBLOCK;
			ttstart(tp);
		}
		tp->t_char = 0;
	}

	splx(s);
	return(0);
}

/*
 ************************************************
 *						*
 *	ttyrend() moved to /sys/dev/mpxpk.c	*
 *						*
 ************************************************
 */

/*
 * Place a character on raw TTY input queue, putting in delimiters
 * and waking up top half as needed.
 * Also echo if required.
 * The arguments are the character and the appropriate
 * tty structure.
 */
ttyinput(c, tp)
register c;
register struct tty *tp;
{
	register int t_flags;
	register tc;

	tk_nin += 1;
	c &= 0377;
	t_flags = tp->t_flags;
	if (t_flags&TANDEM)
		ttyblock(tp);
	if ((t_flags&RAW)==0) {
		c &= 0177;
		if (tp->t_state&TTSTOP) {
			if (c==tun.t_startc) {
				tp->t_state &= ~TTSTOP;
				ttstart(tp);
				return;
			}
			if (c==tun.t_stopc)
				return;
/*
 * The following two lines of code are commented out
 * in order to insure proper XON/XOFF handling.
 * This change causes tty output to be restarted only
 * by XON (control q) instead of by any character.
 *			tp->t_state &= ~TTSTOP;
 *			ttstart(tp);
 */
		} else {
			if (c==tun.t_stopc) {
				tp->t_state |= TTSTOP;
				(*cdevsw[major(tp->t_dev)].d_stop)(tp);
				return;
			}
			if (c==tun.t_startc)
				return;
		}
		if (c==tun.t_quitc || c==tun.t_intrc) {
			flushtty(tp, FREAD|FWRITE);
			tc = (c==tun.t_intrc) ? SIGINT:SIGQUIT;
			gsignal(tp->t_pgrp, tc);
			ttyecho(c, tp);
			ttstart(tp);
			tp->t_rocount = 0;
			tp->t_lstate &= ~LSERASE;
			return;
		}
		if (c=='\r' && t_flags&CRMOD)
			c = '\n';
	}
	if (tp->t_rawq.c_cc>TTYHOG) {
		flushtty(tp, FREAD|FWRITE);
		return;
	}
	if (t_flags&LCASE && c>='A' && c<='Z')
		c += 'a'-'A';
	putc(c, &tp->t_rawq);
	if (t_flags&(RAW|CBREAK)||(c=='\n'||c==tun.t_eofc||c==tun.t_brkc)) {
		if ((t_flags&(RAW|CBREAK))==0 && putc(0377, &tp->t_rawq)==0)
			tp->t_delct++;
			wakeup((caddr_t)&tp->t_rawq);
	}
	if (t_flags&ECHO) {
		if((t_flags&RAW) == 0 && (c==tp->t_erase || c == tp->t_kill))
			ttyrub(c, tp);
		else{
			if(tp->t_lstate & LSERASE){
				tp->t_lstate &= ~LSERASE;
				ttyecho('/', tp);
			}
			ttyecho(c, tp);
		}
		ttstart(tp);
	}
}


/*
 * put character on TTY output queue, adding delays,
 * expanding tabs, and handling the CR/NL bit.
 * It is called both from the top half for output, and from
 * interrupt level for echoing.
 * The arguments are the character and the tty structure.
 */
ttyoutput(c, tp)
register c;
register struct tty *tp;
{
	register char *colp;
	register ctype;

	tk_nout += 1;
	/*
	 * Ignore EOT in normal mode to avoid hanging up
	 * certain terminals.
	 * In raw mode dump the char unchanged.
	 */

	if ((tp->t_flags&RAW)==0) {
		c &= 0177;
		if (c==CEOT)
			return;
	} else {
		putc(c, &tp->t_outq);
		return;
	}

	/*
	 * Turn tabs to spaces as required
	 */
	if (c=='\t' && (tp->t_flags&TBDELAY)==XTABS) {
		c = 8;
		do
			ttyoutput(' ', tp);
		while (--c >= 0 && tp->t_col&07);
		return;
	}
	/*
	 * for upper-case-only terminals,
	 * generate escapes.
	 */
	if (tp->t_flags&LCASE) {
		colp = "({)}!|^~'`";
		while(*colp++)
			if(c == *colp++) {
				ttyoutput('\\', tp);
				c = colp[-2];
				break;
			}
		if ('a'<=c && c<='z')
			c += 'A' - 'a';
	}
	/*
	 * turn <nl> to <cr><lf> if desired.
	 */
	if (c=='\n' && tp->t_flags&CRMOD)
		ttyoutput('\r', tp);
	putc(c, &tp->t_outq);
	/*
	 * Calculate delays.
	 * The numbers here represent clock ticks
	 * and are not necessarily optimal for all terminals.
	 * The delays are indicated by characters above 0200.
	 * In raw mode there are no delays and the
	 * transmission path is 8 bits wide.
	 */
	colp = &tp->t_col;
	ctype = partab[c];
	c = 0;
	switch (ctype&077) {

	/* ordinary */
	case 0:
		(*colp)++;

	/* non-printing */
	case 1:
		break;

	/* backspace */
	case 2:
		if (*colp)
			(*colp)--;
		break;

	/* newline */
	case 3:
		ctype = (tp->t_flags >> 8) & 03;
		if(ctype == 1) { /* tty 37 */
			if (*colp)
				c = MAX(((unsigned)*colp>>4) + 3, (unsigned)6);
		} else
		if(ctype == 2) { /* vt05 */
			c = 6;
		}
		*colp = 0;
		break;

	/* tab */
	case 4:
		ctype = (tp->t_flags >> 10) & 03;
		if(ctype == 1) { /* tty 37 */
			c = 1 - (*colp | ~07);
			if(c < 5)
				c = 0;
		}
		*colp |= 07;
		(*colp)++;
		break;

	/* vertical motion */
	case 5:
		if(tp->t_flags & VTDELAY) /* tty 37 */
			c = 0177;
		break;

	/* carriage return */
	case 6:
		ctype = (tp->t_flags >> 12) & 03;
		if(ctype == 1) { /* tn 300 */
			c = 5;
		} else if(ctype == 2) { /* ti 700 */
			c = 10;
		}
		*colp = 0;
	}
	if(c)
		putc(c|0200, &tp->t_outq);
}

/*
 * Called from device's read routine after it has
 * calculated the tty-structure given as argument.
 */
ttread(tp)
register struct tty *tp;
{
	register s;

	if ((tp->t_state&CARR_ON)==0)
		return(-1);
	s = spl5();
	if (tp->t_canq.c_cc == 0)
		while ((canon(tp)<0) && (tp->t_state&CARR_ON)) {
			sleep((caddr_t)&tp->t_rawq, TTIPRI);
		}
	splx(s);
	while (tp->t_canq.c_cc && passc(getc(&tp->t_canq))>=0)
		;
	return(tp->t_rawq.c_cc + tp->t_canq.c_cc);
}

/*
 * Called from the device's write routine after it has
 * calculated the tty-structure given as argument.
 */
caddr_t
ttwrite(tp)
register struct tty *tp;
{
	register c;

	if ((tp->t_state&CARR_ON)==0)
		return(NULL);
	while (u.u_count) {
		spl5();
		while (tp->t_outq.c_cc > TTHIWAT(tp)) {
			ttstart(tp);
			tp->t_state |= ASLEEP;
			sleep((caddr_t)&tp->t_outq, TTOPRI);
		}
		spl0();
		if ((c = cpass()) < 0)
			break;
		ttyoutput(c, tp);
	}
	ttstart(tp);
	return(NULL);
}

/*
 * ttlocl system call
 * Allows for local operation of a terminal.
 * Only the super-user can execute this call.
 * This call is intended for use by the "init"
 * process only, no other process should ever call it.
 * The actual control is done via an ioctl call
 * to the dh or dz driver (and permissions are checked there).
 */

ttlocl()
{
	register struct a {
		int	tt_dev;		/* maj/min device from /dev/tty?? */
		int	tt_mode;	/* 0 = remote, 1 = local */
		} *uap;

	uap = (struct a *)u.u_ap;
    (*cdevsw[major(uap->tt_dev)].d_ioctl)(uap->tt_dev,TIOCLOCAL,0,uap->tt_mode);
}

ttyecho(c, tp)
register c;
register struct tty *tp;
{
	int cnt, tcnt, tcol;

	if((tp->t_flags&ECHO) == 0)
		return;
	switch(partab[c&0177]&0177){
		case 0:		/* ORDINARY */
			tp->t_rocount++;
			break;
		case 4:		/* TAB */
			tcnt = 8;
			tcol = tp->t_col;
			do{
				tp->t_rocount++;
				tcol++;
			}while (--tcnt >= 0 && tcol&07);
			break;
		case 1:		/* CONTROL */
			if(tp->t_local & LCTLECH){
				tp->t_rocount++;
				ttyoutput('^', tp);
				c &= 0177;
				if(c == 0177)
					c = '?';
				else if(tp->t_flags&LCASE)
					c += 'a' - 1;
				else
					c += 'A' - 1;
				tp->t_rocount++;
			}
			else if(c==tun.t_eofc||c==tun.t_brkc)
				return;
			break;
		case 2:		/* BACKSPACE */
			if(tp->t_rocount > 0)
				tp->t_rocount--;
			break;
		case 3:		/* newline */
		case 6:		/* carriage return */
			tp->t_rocount = 0;
			break;
		default:
			break;
	}
	ttyoutput(c, tp);
}

ttyrub(c, tp)
register c;
register struct tty *tp;
{
	char *cp;
	int cnt, tcol;
	int erchr;

	erchr = unputc(&tp->t_rawq);
	if(c==tp->t_erase){
		if((erchr = unputc(&tp->t_rawq)) < 0){
			return;
		}
		else {
			cnt = 0;
			switch(partab[erchr&0177]&0177){
				case 0:		/* ORDINARY */
					cnt++;
					break;
				case 1:		/* CONTROL */
					if(tp->t_local & LCTLECH)
						cnt += 2;
					break;
				case 4:		/* TAB */
					tcol = tp->t_col - tp->t_rocount;
					cp = (tp->t_rawq.c_cf)-1;
					while((cp=nextc(&tp->t_rawq,cp)) > 0){
						ttycsiz(*cp,tp, &cnt, &tcol);
					}
					cnt = tp->t_rocount - cnt;
					break;
				default:
					cnt++;
					break;
			}
			if (tp->t_local & LCRTBS) {
				for(;cnt > 0; cnt--){
					ttyecho('\b', tp);
					if (tp->t_local & LCRTERA) {
						ttyecho(' ', tp);
						ttyecho('\b', tp);
					}
				}
			} else if (tp->t_local & LPRTERA) {
				if((tp->t_lstate&LSERASE) == 0){
					tp->t_lstate |= LSERASE;
					ttyecho('\\', tp);
				}
				ttyecho(erchr, tp);
			} else {
				ttyecho(c, tp);
			}
		}
	}
	else if(c==tp->t_kill){
		while((unputc(&tp->t_rawq)) > 0);
		if(tp->t_ospeed < B1200){
			ttyecho(c, tp);
			ttyecho('\n', tp);
		}
		else if(tp->t_local & LCRTKIL){
			while(tp->t_rocount > 0){
				ttyecho('\b', tp);
				ttyecho(' ', tp);
				ttyecho('\b', tp);
			}
		}
		else switch(tp->t_local&(LCRTBS|LCRTERA|LPRTERA)){
			case LCRTBS:
			case LCRTERA:
				while(tp->t_rocount > 0)
					ttyecho('\b', tp);
				break;
			case LPRTERA:
				if(tp->t_lstate&LSERASE){
					ttyecho('/', tp);
					tp->t_lstate &= ~LSERASE;
				}
				ttyecho(c, tp);
				ttyecho('\n', tp);
				break;
			default:
				ttyecho(c, tp);
				ttyecho('\n', tp);
				break;
		}
	}
}

ttycsiz(c, tp, cnt, col)
char c;
struct tty *tp;
char *cnt;
char *col;
{
	int tcnt;

	switch(partab[c&0177]&0177){
		case 0:		/* Ordinary */
			(*cnt)++;
			(*col)++;
			break;
		case 1:		/* Control */
			if(tp->t_local&LCTLECH){
				(*cnt) += 2;
				(*col) += 2;
			}
			break;
		case 2:		/* Back space */
			if(*cnt > 0 && *col > 0){
				(*cnt)--;
				(*col)--;
			}
			break;
		case 4:		/* Tab */
			tcnt = 8;
			do{
				(*cnt)++;
				(*col)++;
			}while(--tcnt >= 0 && ((*col) &07));
			break;
		case 3:		/* Line Feed */
		case 6:		/* Caret */
			(*cnt) = 0;
			(*col) = 0;
			break;
		default:
			break;
	}
}
