
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)ttynew.c	3.2	1/14/88
 */
/*
 * New version of tty driver, supported
 * as NTTYDISC.  Adapted from a tty.c written
 * by J.E.Kulp of IIASA, Austria (jekulp@mc)

 * Changes for MIN and TIME (systemV) 
 * Changes for O_NDELAY flag in open()
 * - George Mathew
 * Modified 3/1/86 (use u_fmode instead of t_lstate) -- Fred Canter
 */
#define	_spl0	spl0
#define	_spl5	spl5
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
#include <sys/buf.h>
#include <sys/seg.h>

#ifndef	SELECT
#define	ttwakeup(tp)	wakeup((caddr_t)&tp->t_rawq)
#endif	SELECT

/*
 * Define for SVID termio fixes, i.e.,
 * make vmin and vtime work in raw and cbreak modes.
 * Must also define in tty.c.
 * 1/8/88 -- Fred Canter
 */
#define	TERMIO_FIX

/*
 *	SCCS id	@(#)ttynew.c	1.9 (Berkeley)	10/27/82
 */


extern	char	partab[];

/*
 * Input mapping table-- if an entry is non-zero, when the
 * corresponding character is typed preceded by "\" the escape
 * sequence is replaced by the table value.  Mostly used for
 * upper-case only terminals.
 */

char	maptab[];		/* see tty.c */

/*
 * shorthand
 */
#define	q1	tp->t_rawq
#define	q2	tp->t_canq
#define	q3	tp->t_outq
#define	q4	tp->t_un.t_ctlq

#define	OBUFSIZ	100

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

/*
 * If TERMIO_FIX defined, following three routines
 * are moved to tty.c. This is because the root text
 * segment overflows on bedrock (max config test case).
 */
#ifndef	TERMIO_FIX
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
 * Place a character on raw TTY input queue, putting in delimiters
 * and waking up top half as needed.
 * Also echo if required.
 * The arguments are the character and the appropriate
 * tty structure.
 */
ntyinput(c, tp)
register c;
register struct tty *tp;
{
	register int t_flags;
	int i;

	if ((tp->t_flags&(RAW|ECHO)) == RAW) {
		/*
		 * Raw and no echo.  There is a lot of normal
		 * overhead that can be zapped in this case,
		 * so we just do what's needed and return.
		 * We don't call ttyblock(), because there
		 * is fat there that we can also trim out.
		 * Fred Canter -- 1/4/88
		 * Don't send stop char if input queue already blocked.
		 * We were sending an XOFF for each char over TTYHOG/2.
		 */
		tk_nin++;
		if (tp->t_flags&TANDEM) {
			if (tp->t_rawq.c_cc >= TTYHOG/2) {
				if (tp->t_state&TBLOCK == 0) {
				    if (putc(tun.t_stopc, &tp->t_outq)==0) {
					tp->t_state |= TBLOCK;
					ttstart(tp);
				    }
				}
				if (tp->t_rawq.c_cc > TTYHOG) {
					flushtty(tp, FREAD|FWRITE);
					tp->t_state &= ~TBLOCK;
					return;
				}
			}
		} else if (tp->t_rawq.c_cc > TTYHOG) {
			flushtty(tp, FREAD|FWRITE);
			return;
		}
		if (putc(c, &tp->t_rawq) >= 0) {
			if (tp->t_local&LINTRUP)
				gsignal(tp->t_pgrp, SIGTINT);
			tp->t_lstate &= ~LRTO;
			if ((tp->t_rawq.c_cc < tun.t_vmin) &&
			    (tun.t_vtime)) {
				if(!(tp->t_lstate&LTACT))
					tttimeo(tp);
			}
			else if (tp->t_rawq.c_cc >= tun.t_vmin)
				ttwakeup(tp);
			else if ((tun.t_vmin == 0) && (tun.t_vtime))
				if(!(tp->t_lstate&LTACT))
					tttimeo(tp);
			else if ((tun.t_vmin == 0) && (tun.t_vtime == 0))
					ttwakeup(tp);
		}
		return;
	}

	/*
	 * If input is pending - on rawq, take it first
	 */
	if (tp->t_local&LPENDIN)
		ntypend(tp);

	tk_nin++;
	c &= 0377;
	t_flags = tp->t_flags;

	/*
	 * In tandem mode, check high water mark
	 */
	if (t_flags&TANDEM)
		ttyblock(tp);

	if ((t_flags&RAW)==0) {
		if ((tp->t_lstate&LSTYPEN) == 0)
			c &= 0177;

	/* check for literal nexting before any other processing */
		if (tp->t_lstate&LSLNCH) {
			/*
			 * accept this char literally
			 * and leave the state
			 */
			c |= 0200;
			tp->t_lstate &= ~LSLNCH;
		}
		if (tp->t_line == NTTYDISC && c==tlun.t_lnextc) {
			/*
			 * New line discipline only
			 * setup to accept the next char literally
			 * echo ^ backspace.
			 */
			if (tp->t_flags&ECHO)
				ntyout("^\b", tp);
			tp->t_lstate |= LSLNCH;

	/* check for output control functions */
		} else if (c==tun.t_stopc) {
			/*
			 * stop output if we aren't already
			 */
			if ((tp->t_state&TTSTOP)==0) {
				tp->t_state |= TTSTOP;
				(*cdevsw[major(tp->t_dev)].d_stop)(tp);
				return;
			}
			if (c!=tun.t_startc)
				return;
			/*
			 * Note if c = tun.t_startc
			 * we are falling through here.
			 * this is the case when 
			 * startc == stopc
			 * - DaveR
			 */
		} else if (c==tun.t_startc) {
			/*
			 * restart output
			 */
			tp->t_state &= ~TTSTOP;
			tp->t_local &= ~LFLUSHO;
			ttstart(tp);
			return;

	/* check for input interrupts (and flushed output) */
		} else if (tp->t_line == NTTYDISC && c==tlun.t_flushc) {
			if (tp->t_local & LFLUSHO)
				tp->t_local &= ~LFLUSHO;
			else {
				flushtty(tp, FWRITE);
				ntyecho(c, tp);
				if (tp->t_rawq.c_cc+tp->t_canq.c_cc)
					ntyretype(tp);
				tp->t_local |= LFLUSHO;
			}
			ttstart(tp);
			return;
#ifdef	iflushcstuff
		} else if (c==tlun.t_iflushc) {
			if ((t_flags&CBREAK) == 0) {
				/* easy */
				flushtty(tp, FREAD);
				ntyecho(c, tp);
			} else {
				while ((c=unputc(&tp->t_canq)) >= 0) {
					if (isdelim(c))
						if (--tp->t_delct < 1) {
							putc(c, &tp->t_canq);
							tp->t_delct++;
							break;
						}
				}
				if ((tp->t_state&TBLOCK) &&
				    tp->t_canq.c_cc < TTLOWAT(tp)) {
					if (putc(tun.t_startc, &tp->t_outq)==0){
						tp->t_state &= ~TBLOCK;
						ttstart(tp);
					}
				}
			}
			return;
#endif
		} else if ( (tp->t_line == NTTYDISC && c==tlun.t_suspc) ||
				c==tun.t_quitc || c==tun.t_intrc ) {
				if ((tp->t_local & LNOFLSH) == 0)
					flushtty(tp, c==tlun.t_suspc ? FREAD : FREAD|FWRITE);
			ntyecho(c, tp);
			c = c==tun.t_intrc ? SIGINT :
				((c==tun.t_quitc) ? SIGQUIT : SIGTSTP);
			gsignal(tp->t_pgrp, c);

	/* check for buffer editing functions - cooked mode */
		} else if ((t_flags&CBREAK) == 0) {
			if ((tp->t_lstate&LSQUOT) &&
			    (c==tp->t_erase||c==tp->t_kill)) {
				/*
			 	* last char was a '\' - this is being
			 	* used to escape the kill or erase char
			 	* - this is old tty compatibility code
			 	* but is supported by the new tty  discipline
			 	*/
				register int t;
				if ((t = unputc(&tp->t_rawq)) > 0)
					ntyrub(t, tp);
				c |= 0200;
			}
			if (c==tp->t_erase) {
				/*
				 * erase last char - if its still on the
				 * rawq
				 */
				if (tp->t_rawq.c_cc) {
					register int t;
					if ((t = unputc(&tp->t_rawq)) > 0)
						ntyrub(t, tp);
				}
			} else if (c==tp->t_kill) {
				/*
				 * kill the current line
				 */
				if (tp->t_local&LCRTKIL &&
				    tp->t_rawq.c_cc == tp->t_rocount) {
					/*
					 * erase the whole line with
					 * \b\b\b...\b
					 */
					while (tp->t_rawq.c_cc) {
						register int t;
						if ((t=unputc(&tp->t_rawq)) > 0)
							ntyrub(t, tp);
						else
							break;
					}
				} else {
					/*
					 * echo the kill char and a
					 * newline
					 */
					ntyecho(c, tp);
					ntyecho('\n', tp);
					while (getc(&tp->t_rawq) > 0)
						;
					tp->t_rocount = 0;
				}
				tp->t_lstate = 0;
			} else if (tp->t_line == NTTYDISC && c==tlun.t_werasc) {
				/*
				 * word erase - newtty line discipline
				 * only
				 */
				if (tp->t_rawq.c_cc == 0)
					goto out;
				/*
				 * remove trailing spaces,tabs or '/'
				 * at the end of the last word
				 */
				do {
					c = unputc(&tp->t_rawq);
					if (c != ' ' && c != '\t' && c != '/')
						goto erasenb;
					ntyrub(c, tp);
				} while (tp->t_rawq.c_cc);
				goto out;
			    erasenb:
				/*
				 * now delete the word itself
				 */
				do {
					if (c < 0)	/* unputc() failed */
						goto out;
					ntyrub(c, tp);
					if (tp->t_rawq.c_cc == 0)
						goto out;
					c = unputc(&tp->t_rawq);
				} while (c != ' ' && c != '\t' && c != '/');
				/*
				 * put the last char back
				 */
				(void) putc(c, &tp->t_rawq);

			} else if (tp->t_line == NTTYDISC && c==tlun.t_rprntc) {
				/*
				 * retype the current line of input
				 * - new line discipline only
				 */
				ntyretype(tp);

			/*
			 * check for cooked mode input buffer overflow 
			 */
			} else if (tp->t_rawq.c_cc + tp->t_canq.c_cc > TTYHOG) {
				/* we should start a timeout that flushes the
				   buffer if it stays full - same in CBREAK */
				if (tp->t_line == NTTYDISC && 
					tp->t_outq.c_cc < TTHIWAT(tp))
					(void) ntyoutput(CTRL(g), tp);

			/*
			 * put data char in q for user and wakeup if
			 *  a break char 
			 */
			} else if (putc(c, &tp->t_rawq) >= 0) {
				if (!ntbreakc(c, tp)) {
					if (tp->t_rocount++ == 0)
						tp->t_rocol = tp->t_col;
				} else {
					/*
					 * Its a break char
					 * increment delimeter count?? daver
					 */
					tp->t_rocount = 0;
					catq(&tp->t_rawq, &tp->t_canq);
					ttwakeup(tp);
					if (tp->t_local&LINTRUP)
						gsignal(tp->t_pgrp, SIGTINT);
				}
				tp->t_lstate &= ~LSQUOT;
				if (c == '\\')
					tp->t_lstate |= LSQUOT;
				if (tp->t_lstate&LSERASE) {
					/*
					 * within a \.../ erase, this
					 * is now ending - output
					 * the closing /
					 */
					tp->t_lstate &= ~LSERASE;
					(void) ntyoutput('/', tp);
				}
				i = tp->t_col;
				ntyecho(c, tp);
				if (c==tun.t_eofc && tp->t_flags&ECHO) {
					i = MIN(2, tp->t_col - i);
					while (i > 0) {
						(void) ntyoutput('\b', tp);
						i--;
					}
				}
			}
	/* CBREAK mode */
		} else if (tp->t_rawq.c_cc > TTYHOG) {
			if (tp->t_line == NTTYDISC && 
				tp->t_outq.c_cc < TTHIWAT(tp))
				(void) ntyoutput(CTRL(g), tp);
		} else if (putc(c, &tp->t_rawq) >= 0) {
			if (tp->t_local&LINTRUP)
				gsignal(tp->t_pgrp, SIGTINT);
#ifdef	TERMIO_FIX
			tp->t_lstate &= ~LRTO;
			if ((tp->t_rawq.c_cc < tun.t_vmin) &&
			    (tun.t_vtime)) {
				if(!(tp->t_lstate&LTACT))
					tttimeo(tp);
			}
			else if (tp->t_rawq.c_cc >= tun.t_vmin)
				ttwakeup(tp);
			else if ((tun.t_vmin == 0) && (tun.t_vtime))
				if(!(tp->t_lstate&LTACT))
					tttimeo(tp);
			else if ((tun.t_vmin == 0) && (tun.t_vtime == 0))
					ttwakeup(tp);
#else	TERMIO_FIX
			ttwakeup(tp);
#endif	TERMIO_FIX
			ntyecho(c, tp);
		}
	/* RAW mode */
	} else if (tp->t_rawq.c_cc > TTYHOG) 
		flushtty(tp, FREAD|FWRITE);
	else {
		if (putc(c, &tp->t_rawq) >= 0) {
			if (tp->t_local&LINTRUP)
				gsignal(tp->t_pgrp, SIGTINT);
			tp->t_lstate &= ~LRTO;
			if ((tp->t_rawq.c_cc < tun.t_vmin) &&
			    (tun.t_vtime)) {
				if(!(tp->t_lstate&LTACT))
					tttimeo(tp);
			}
			else if (tp->t_rawq.c_cc >= tun.t_vmin)
				ttwakeup(tp);
			else if ((tun.t_vmin == 0) && (tun.t_vtime))
				if(!(tp->t_lstate&LTACT))
					tttimeo(tp);
			else if ((tun.t_vmin == 0) && (tun.t_vtime == 0))
					ttwakeup(tp);
		}
		ntyecho(c, tp);
	}
out:
	if (tp->t_local & LDECCTQ && tp->t_state & TTSTOP &&
		tun.t_startc != tun.t_stopc )
		return;
	tp->t_state &= ~TTSTOP;
	tp->t_local &= ~LFLUSHO;
	ttstart(tp);
}

/*
 * put character on TTY output queue, adding delays,
 * expanding tabs, and handling the CR/NL bit.
 * It is called both from the top half for output, and from
 * interrupt level for echoing.
 * The arguments are the character and the tty structure.
 * Returns < 0 if putc succeeds, otherwise returns char to resend
 * Must be recursive.
 */
static
ntyoutput(c, tp)
register c;
register struct tty *tp;
{
	register char *colp;
	register ctype;

	if (tp->t_flags&RAW || tp->t_local&LLITOUT) {
		if (tp->t_local&LFLUSHO)
			return (-1);
		if (putc(c, &tp->t_outq))
			return(c);
		tk_nout++;
		return (-1);
	}
	/*
	 * Ignore EOT in normal mode to avoid hanging up
	 * certain terminals.
	 */
	c &= 0177;
	if (c==CEOT && (tp->t_flags&CBREAK)==0)
		return (-1);
	/*
	 * Turn tabs to spaces as required
	 */
	if (c=='\t' && (tp->t_flags&TBDELAY)==XTABS) {
		register int s;

		c = 8 - (tp->t_col&7);
		if ((tp->t_local&LFLUSHO) == 0) {
			s = spl5();		/* don't interrupt tabs */
			c -= b_to_q("        ", c, &tp->t_outq);
			tk_nout += c;
			splx(s);
		}
		tp->t_col += c;
		return (c ? -1 : '\t');
	}
	tk_nout++;
	/*
	 * for upper-case-only terminals,
	 * generate escapes.
	 */
	if (tp->t_flags&LCASE) {
		colp = "({)}!|^~'`";
		while(*colp++)
			if(c == *colp++) {
				if (ntyoutput('\\', tp) >= 0)
					return (c);
				c = colp[-2];
				break;
			}
		/* added for vax compatibility - daver */
		if ('A' <= c && c <= 'Z') {
			if (ntyoutput('\\',tp) >= 0)
				return(c);
		} else if ('a'<=c && c<='z')
			c += 'A' - 'a';
	}
	/*
	 * turn <nl> to <cr><lf> if desired.
	 */
	if (c=='\n' && tp->t_flags&CRMOD)
		if (ntyoutput('\r', tp) >= 0)
			return (c);
	if (c=='~' && tp->t_local&LTILDE)
		c = '`';
	if ((tp->t_local&LFLUSHO) == 0 && putc(c, &tp->t_outq))
		return (c);
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

	case ORDINARY:
		(*colp)++;

	case CONTROL:
		break;

	case BACKSPACE:
		if (*colp)
			(*colp)--;
		break;

	case NEWLINE:
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

	case TAB:
		ctype = (tp->t_flags >> 10) & 03;
		if(ctype == 1) { /* tty 37 */
			c = 1 - (*colp | ~07);
			if(c < 5)
				c = 0;
		}
		*colp |= 07;
		(*colp)++;
		break;

	case VTAB:
		if(tp->t_flags & VTDELAY) /* tty 37 */
			c = 0177;
		break;

	case RETURN:
		ctype = (tp->t_flags >> 12) & 03;
		if(ctype == 1) { /* tn 300 */
			c = 5;
		} else if(ctype == 2) { /* ti 700 */
			c = 10;
		} else if(ctype == 3) { /* concept 100 */
			int i;
			if ((i = *colp) >= 0)
				for (; i<9; i++)
					(void) putc(0177, &tp->t_outq);
		}
		*colp = 0;
	}
	if(c && (tp->t_local&LFLUSHO) == 0)
		(void) putc(c|0200, &tp->t_outq);
	return (-1);
}

/*
 * Called from device's read routine after it has
 * calculated the tty-structure given as argument.
 */
ntread(tp)
register struct tty *tp;
{
	register struct clist *qp;
	register c, first;

	if ((tp->t_state&CARR_ON) == 0)
		return(0);
loop:
	(void) _spl5();
	/*
	 * Take any pending input first
	 */
	if (tp->t_local&LPENDIN)
		ntypend(tp);
	(void) _spl0();

	/*
	 * hang process if its in the background
	 */
	while (tp == u.u_ttyp && u.u_procp->p_pgrp != tp->t_pgrp) {
		if (u.u_signal[SIGTTIN] == SIG_IGN ||
		    u.u_signal[SIGTTIN] == SIG_HOLD ||
		    (u.u_procp->p_flag&SDETACH))
			return (0);
		gsignal(u.u_procp->p_pgrp, SIGTTIN);
		sleep((caddr_t)&lbolt, TTIPRI);
	}
	/*
	 * In raw mode take characters directly from the raw
	 * queue without processing. Lock out interrupts whilst
	 * dealing with the rawq
	 */
	if (tp->t_flags&RAW) {
		(void) _spl5();
		if (tp->t_rawq.c_cc <= 0) {
			if (((tp->t_state&CARR_ON)==0) || (u.u_fmode&FNDELAY)
#ifdef	SELECT
			     || (tp->t_state&TS_NBIO)
#endif
			    )
			{
				(void) _spl0();
				return (0);
			}
			if (tp->t_local&LINTRUP &&
			    u.u_signal[SIGTINT] != SIG_DFL) {
				u.u_error = EWOULDBLOCK;
				(void) _spl0();
				return (0);
			}
#ifdef	TERMIO_FIX
			/*
			 * So SysV vmin/vtime work in raw mode.
			 */
			if(tun.t_vmin == 0) {
				if(tun.t_vtime == 0)
					goto cont1;
				tp->t_lstate &= ~LRTO;
				if(!(tp->t_lstate&LTACT))
					tttimeo(tp);
			}
#endif	TERMIO_FIX
			sleep((caddr_t)&tp->t_rawq, TTIPRI);
#ifdef	TERMIO_FIX
			/*
			 * Wakeup was due to t_vtime tiemout.
			 */
			if(tp->t_delct) {
				tp->t_delct = 0;
				goto cont1;
			}
#endif	TERMIO_FIX
			(void) _spl0();
			goto loop;
		}
		(void) _spl0();
cont1:
		while (tp->t_rawq.c_cc && passc(getc(&tp->t_rawq))>=0)
			;
	} else {
		/*
		 * In cbreak mode use the rawq, otherwise take characters
		 * from the canonicalised queue.
		 */
		qp = tp->t_flags & CBREAK ? &tp->t_rawq : &tp->t_canq;
		(void) _spl5();
		if (qp->c_cc <= 0) {
			if (((tp->t_state&CARR_ON)==0) || (u.u_fmode&FNDELAY)
#ifdef	SELECT
			    || (tp->t_state&TS_NBIO)
#endif
			   )
			{
				(void) _spl0();
				return (0);
			}
			if (tp->t_local&LINTRUP &&
			    u.u_signal[SIGTINT] != SIG_DFL) {
				u.u_error = EWOULDBLOCK;
				(void) _spl0();
				return (0);
			}
#ifdef	TERMIO_FIX
			/*
			 * So SysV vmin/vtime work in cbreak mode
			 * which is equivalent to ICANON off.
			 */
			if(tp->t_flags&CBREAK && (tun.t_vmin == 0)) {
				if(tun.t_vtime == 0)
					goto cont;
				tp->t_lstate &= ~LRTO;
				if(!(tp->t_lstate&LTACT))
					tttimeo(tp);
			}
#endif	TERMIO_FIX
			sleep((caddr_t)&tp->t_rawq, TTIPRI);
#ifdef	TERMIO_FIX
			/*
			 * Wakeup was due to t_vtime tiemout.
			 */
			if(tp->t_delct) {
				tp->t_delct = 0;
				goto cont;
			}
#endif	TERMIO_FIX
			(void) _spl0();
			goto loop;
		}
cont:
		(void) _spl0();
		/*
		 * Input present, perform input mapping and
		 * processing - we're not in raw mode
		 */
		first = 1;
		while ((c = getc(qp)) >= 0) {
			if (tp->t_flags&CRMOD && c == '\r')
				c = '\n';
			/*
			 * Hack lower case simulation on upper case
			 * only terminals.
			 */
			if (tp->t_flags&LCASE && c <= 0177)
				if (tp->t_lstate&LSBKSL) {
					if (maptab[c])
						c = maptab[c];
					tp->t_lstate &= ~LSBKSL;
				} else if (c >= 'A' && c <= 'Z')
					c += 'a' - 'A';
				else if (c == '\\') {
					tp->t_lstate |= LSBKSL;
					continue;
				}
			/*
			 * Check for delayed suspend character.
			 * - newline discipline only
			 */
			if (tp->t_line == NTTYDISC && c == tlun.t_dsuspc) {
				gsignal(tp->t_pgrp, SIGTSTP);
				if (first) {
					sleep((caddr_t)&lbolt, TTIPRI);
					goto loop;
				}
				break;
			}
			/*
			 * Interpret EOF only in cooked mode
			 */
			if (c == tun.t_eofc && (tp->t_flags&CBREAK)==0)
				break;
			if (passc(c & 0177) < 0)
				break;

			/*
			 * In cooked mode check for a "break character"
			 * marking the end of a "line of input".
			 */
			if ((tp->t_flags&CBREAK)==0 && ntbreakc(c, tp))
				break;
			first = 0;
		}
		tp->t_lstate &= ~LSBKSL;
	}

	/*
	 *check tandem
	 *look to unblock output now that (presumably)
	 *the input queue has gone down.
	 */
	if (tp->t_state&TBLOCK && tp->t_rawq.c_cc < TTYHOG/5) {
		if (putc(tun.t_startc, &tp->t_outq)==0) {
			tp->t_state &= ~TBLOCK;
			ttstart(tp);
		}
	}

	return (tp->t_rawq.c_cc + tp->t_canq.c_cc);
}

/*
 * Called from the device's write routine after it has
 * calculated the tty-structure given as argument.
 */
caddr_t
ntwrite(tp)
register struct tty *tp;
{
	register char *cp;
	register int cc, ce;
	register i;
	char obuf[OBUFSIZ];
	register c;
	int hiwat = TTHIWAT(tp);
#ifdef	SELECT
	int cnt = u.u_count;
#endif

	if ((tp->t_state&CARR_ON)==0)
		return (NULL);
loop:
	/*
	 * Hang process if its in the background.
	 */

	while (u.u_procp->p_pgrp != tp->t_pgrp && tp == u.u_ttyp &&
	    (tp->t_local&LTOSTOP) && 
	    u.u_signal[SIGTTOU] != SIG_IGN &&
	    u.u_signal[SIGTTOU] != SIG_HOLD &&
	    (u.u_procp->p_flag&SDETACH)==0) {
		gsignal(u.u_procp->p_pgrp, SIGTTOU);
		sleep((caddr_t)&lbolt, TTIPRI);
	}
	while (u.u_count) {
		cc = MIN(u.u_count, OBUFSIZ);
		cp = obuf;
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
		if (tp->t_outq.c_cc > hiwat)
			goto ovhiwat;
		if (tp->t_local&LFLUSHO)
			continue;
		/*
		 * If we're mapping lower case or kludging tildes,
		 * then we've got to look at each character, so
		 * just feed the stuff to ttyouput...
		 */
		if (tp->t_flags&LCASE || tp->t_local&LTILDE) {
			while (cc--) {
				c = *cp++;
				tp->t_rocount = 0;
				while((c = ntyoutput(c, tp)) >= 0) {
					/* out of clists, wait a bit */
					ttstart(tp);
					sleep((caddr_t)&lbolt, TTOPRI);
					tp->t_rocount = 0;
				}
				if (tp->t_outq.c_cc > hiwat)
					goto ovhiwat;
			}
			continue;
		}
		while (cc) {
			if (tp->t_flags&RAW || tp->t_local&LLITOUT)
				ce = cc;
			else {
				ce=0;
				/*
				 * find the first character with the high
				 * bit set or needs to be treated specially
				 * ( (partab[character]&77) != 0 ) and hand
				 * it off to ntyoutput.
				 */
				while ( !((*(cp+ce))&0200)
				     && ((partab[*(cp+ce)]&077)==0)
				     && (ce<cc))
					ce++;
				if (ce==0) {
					tp->t_rocount = 0;
					if (ntyoutput(*cp, tp) >= 0) {
						ttstart(tp);
						sleep((caddr_t)&lbolt, TTOPRI);
						continue;
					}
					cp++;
					cc--;
					if (tp->t_outq.c_cc > hiwat)
						goto ovhiwat;
				}
			}
			tp->t_rocount = 0;
			i=b_to_q(cp,ce,&tp->t_outq);
			ce-=i;
			tk_nout+=ce;
			tp->t_col+=ce;
			cp+=ce;
			cc-=ce;
			if (i) {
				ttstart(tp);
				sleep((caddr_t)&lbolt, TTOPRI);
			}
			if (ce || tp->t_outq.c_cc > hiwat)
				goto ovhiwat;
		}
	}
	ttstart(tp);
	return(NULL);

ovhiwat:
	(void) _spl5();
	u.u_base -= cc;
	u.u_offset -= cc;
	u.u_count += cc;
	if (tp->t_outq.c_cc <= hiwat) {
		(void) _spl0();
		goto loop;
	}
	ttstart(tp);
#ifdef	SELECT
	if (tp->t_state & TS_NBIO) {
		if (u.u_count == cnt)
			u.u_error = EWOULDBLOCK;
		return(NULL);
	}
#endif
	tp->t_state |= ASLEEP;
	sleep((caddr_t)&tp->t_outq, TTOPRI);
	(void) _spl0();
	goto loop;
}

/*
 * Rubout one character from the rawq of tp
 * as cleanly as possible.
 */
static
ntyrub(c, tp)
register c;
register struct tty *tp;
{
	register char *cp;
	register int savecol;
	int s;
	char *nextc();

	if ((tp->t_flags&ECHO)==0)
		return;
	tp->t_local &= ~LFLUSHO;
	c &= 0377;
	if (tp->t_local&LCRTBS) {
		if (tp->t_rocount == 0) {
			/*
			 * Screwed by ttwrite; retype
			 */
			ntyretype(tp);
			return;
		}
		if (c==('\t'|0200) || c==('\n'|0200))
			ntyrubo(tp, 2);
		else switch(partab[c&=0177] & 0177) {

		case ORDINARY:
			/* vax has this - daver */
			if (tp->t_flags&LCASE && c >= 'A' && c <= 'Z')
				ntyrubo(tp,2);
			else
				ntyrubo(tp, 1);
			break;

		case VTAB:
		case BACKSPACE:
		case CONTROL:
		case RETURN:
		case NEWLINE:
			if (tp->t_local & LCTLECH)
				ntyrubo(tp, 2);
			break;

		case TAB:
			if (tp->t_rocount < tp->t_rawq.c_cc) {
				ntyretype(tp);
				return;
			}
			s = spl5();
			savecol = tp->t_col;
			tp->t_lstate |= LSCNTTB;
			tp->t_local |= LFLUSHO;
			tp->t_col = tp->t_rocol;
			for (cp = tp->t_rawq.c_cf; cp; cp = nextc(&tp->t_rawq, cp))
				ntyecho(lookc(cp), tp);
			tp->t_local &= ~LFLUSHO;
			tp->t_lstate &= ~LSCNTTB;
			splx(s);
			/*
			 * savecol will now be length of the tab
			 */
			savecol -= tp->t_col;
			tp->t_col += savecol;
			if (savecol > 8)
				savecol = 8;		/* overflow screw */
			while (--savecol >= 0)
				(void) ntyoutput('\b', tp);
			break;

		default:
			panic("ttyrub");
		}
	} else if (tp->t_local&LPRTERA) {
		if ((tp->t_lstate&LSERASE) == 0) {
			(void) ntyoutput('\\', tp);
			tp->t_lstate |= LSERASE;
		}
		ntyecho(c, tp);
	} else
		ntyecho(tp->t_erase, tp);
	tp->t_rocount--;
}

/*
 * Crt back over cnt chars perhaps
 * erasing them.
 */
static
ntyrubo(tp, cnt)
register struct tty *tp;
register cnt;
{

	while (--cnt >= 0)
		ntyout(tp->t_local&LCRTERA ? "\b \b" : "\b", tp);
}

/*
 * Reprint the rawq line.
 * We assume c_cc has already been checked.
 */
static
ntyretype(tp)
register struct tty *tp;
{
	register char *cp;
	char *nextc();
	register s;

	if (tlun.t_rprntc != 0377)
		ntyecho(tlun.t_rprntc, tp);
	(void) ntyoutput('\n', tp);
	s = spl5();
	for (cp = tp->t_canq.c_cf; cp; cp = nextc(&tp->t_canq, cp))
		ntyecho(lookc(cp), tp);
	for (cp = tp->t_rawq.c_cf; cp; cp = nextc(&tp->t_rawq, cp))
		ntyecho(lookc(cp), tp);
	tp->t_lstate &= ~LSERASE;
	splx(s);
	tp->t_rocount = tp->t_rawq.c_cc;
	tp->t_rocol = 0;
}

/*
 * Echo a typed character to the terminal
 */
static
ntyecho(c, tp)
register c;
register struct tty *tp;
{

	if ((tp->t_lstate & LSCNTTB) == 0)
		tp->t_local &= ~LFLUSHO;
	if ((tp->t_flags&ECHO) == 0)
		return;
	c &= 0377;
	if (tp->t_flags&RAW) {
		(void) ntyoutput(c, tp);
		return;
	}
	if (c == '\r' && tp->t_flags&CRMOD)
		c = '\n';
	if (tp->t_local&LCTLECH) {
		if ((c&0177) <= 037 && c!='\t' && c!='\n' || (c&0177)==0177) {
			(void) ntyoutput('^', tp);
			c &= 0177;
			if (c == 0177)
				c = '?';
			else if (tp->t_flags&LCASE)
				c += 'a' - 1;
			else
				c += 'A' - 1;
		}
	}
	if ((tp->t_flags&LCASE) && (c >= 'A' && c <= 'Z'))
		c += 'a' - 'A';
	(void) ntyoutput(c & 0177, tp);
}

/*
 * Is c a break char for tp?
 */
static
ntbreakc(c, tp)
register c;
register struct tty *tp;
{
	return (c == '\n' || c == tun.t_eofc || c == tun.t_brkc ||
		c == '\r' && (tp->t_flags&CRMOD));
}

/*
 * send string cp to tp
 */
static
ntyout(cp, tp)
register char *cp;
register struct tty *tp;
{
	register char c;

	while (c = *cp++)
		(void) ntyoutput(c, tp);
}

#ifdef	SELECT
ttwakeup(tp)
struct tty *tp;
{
	if (tp->t_rsel) {
		selwakeup(tp->t_rsel, tp->t_state&TS_RCOLL);
		tp->t_state &= ~TS_RCOLL;
		tp->t_rsel = 0;
	}
	wakeup((caddr_t)&tp->t_rawq);
}
#endif	SELECT

tttimeo(tp)
struct tty *tp;
{
	int s;
	extern int hz;
	tp->t_lstate &= ~LTACT;
#ifdef	TERMIO_FIX
	if ((tp->t_flags & (RAW|CBREAK)) && (tun.t_vtime)) {
#else	TERMIO_FIX
	if ((tp->t_flags & RAW) && (tun.t_vtime)) {
#endif	TERMIO_FIX
		if( (tp->t_rawq.c_cc == 0) && (tun.t_vmin))
			return;
		if(!(tp->t_lstate & LRTO)) {
			tp->t_lstate |= LRTO|LTACT;
			timeout(tttimeo,(caddr_t)tp,tun.t_vtime*(hz/10));
		} else {
#ifdef	TERMIO_FIX
			tp->t_delct = 1;
#endif	TERMIO_FIX
			ttwakeup(tp);
		}
	}
}
