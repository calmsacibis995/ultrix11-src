
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)lp.c	3.0	4/21/86
 */
/*
 *  Line printer driver
 *
 *  Would require some rewrite to support
 *  more than one printer.
 */

#include <sys/param.h>
#include <sys/devmaj.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/tty.h>


#define	LPPRI	(PZERO+8)
#define	LPLOWAT	40
#define	LPHIWAT	100
#define	LPMAX	1

struct device {
	int	lpcsr, lpbuf;
};

int	io_csr[];	/* CSR address, see c.c */
struct	device	*lp_addr[LPMAX];

struct lp {
	struct	clist l_outq;
	char	flag, ind;
	int	ccc, mcc, mlc;
	int	line, col;
} lp_dt[LPMAX];

/*
 * CAUTION - flags also defined in /usr/src/cmd/lpset.c
 */
#define	OPEN	004
#define	FFCLOSE	010
#define	CAP	020
#define	NOCR	040
#define	ASLP	0100
#define	RAWLP	0200

#define	FORM	014

lpopen(dev, flag)
{
	register unit;
	register struct lp *lp;

	unit = dev&03;
	lp = &lp_dt[unit];
	lp_addr[unit] = (struct device *)io_csr[LP_RMAJ];
	if ((unit >= LPMAX) ||
	 (lp->flag & OPEN) || (lp_addr[unit]->lpcsr < 0)) {
		u.u_error = ENXIO;
		return;
	}
	lp->flag |= OPEN; /* if multi-unit, enter unit # also */
	if (dev & 04)			/*RAWLP*/
		lp->flag |= RAWLP;	/*RAWLP*/
	if(lp->col == 0) {
		lp->ind = 0;
		lp->col = 132;
		lp->line = 66;
	}
	lp_addr[unit]->lpcsr |= IENABLE;
/*	lpoutput(unit, FORM);	*/
}

lpclose(dev)
{
	register unit;

	unit = dev&03;
	if(lp_dt[unit].flag & FFCLOSE)
		lpoutput(unit, FORM);
	lp_dt[unit].flag &= ~OPEN;
	lp_dt[unit].flag &= ~RAWLP;	/*RAWLP*/
}

lpwrite(dev)
{
	register unit;
	register c;
	register struct lp *lp;
	register s;

	unit = dev&03;
	lp = &lp_dt[unit];
	while (u.u_count) {
		s = spl4();
		while(lp->l_outq.c_cc > LPHIWAT) {
			lpintr(unit);
			lp->flag |= ASLP;
			sleep(lp, LPPRI);
		}
		splx(s);
		c = fubyte(u.u_base++);
		if (c < 0) {
			u.u_error = EFAULT;
			break;
		}
		u.u_count--;
		lpoutput(unit, c);
	}
	s = spl4();
	lpintr(unit);
	splx(s);
}

static
lpoutput(dev, c)
register dev, c;
{
	register struct lp *lp;
	register s;

	lp = &lp_dt[dev];
	if (lp->flag&RAWLP) {		/*RAWLP*/
		putc(c, &lp->l_outq);	/*RAWLP*/
		return;			/*RAWLP*/
	}				/*RAWLP*/
	if(lp->flag&CAP) {
		if(c>='a' && c<='z')
			c += 'A'-'a'; else
		switch(c) {
		case '{':
			c = '(';
			goto esc;
		case '}':
			c = ')';
			goto esc;
		case '`':
			c = '\'';
			goto esc;
		case '|':
			c = '!';
			goto esc;
		case '~':
			c = '^';
		esc:
			lpoutput(dev, c);
			lp->ccc--;
			c = '-';
		}
	}
	switch(c) {
	case '\t':
		lp->ccc = ((lp->ccc+8-lp->ind) & ~7) + lp->ind;
		return;
	case '\n':
		lp->mlc++;
		if(lp->mlc >= lp->line )
			c = FORM;
	case FORM:
		lp->mcc = 0;
		if (lp->mlc) {
			putc(c, &lp->l_outq);
			if(c == FORM)
				lp->mlc = 0;
		}
	case '\r':
		lp->ccc = lp->ind;
		s = spl4();
		lpintr(dev);
		splx(s);
		return;
	case 010:
		if(lp->ccc > lp->ind)
			lp->ccc--;
		return;
	case ' ':
		lp->ccc++;
		return;
	default:
		if(lp->ccc < lp->mcc) {
			if (lp->flag&NOCR) {
				lp->ccc++;
				return;
			}
			putc('\r', &lp->l_outq);
			lp->mcc = 0;
		}
		if(lp->ccc < lp->col) {
			while(lp->ccc > lp->mcc) {
				putc(' ', &lp->l_outq);
				lp->mcc++;
			}
			putc(c, &lp->l_outq);
			lp->mcc++;
		}
		lp->ccc++;
	}
}

lpintr(dev)
register dev;
{
	register struct lp *lp;
	register c;

	lp = &lp_dt[dev];
	while (lp_addr[dev]->lpcsr&DONE && (c = getc(&lp->l_outq)) >= 0)
		lp_addr[dev]->lpbuf = c;
	if (lp->l_outq.c_cc <= LPLOWAT && lp->flag&ASLP) {
		lp->flag &= ~ASLP;
		wakeup(lp);
	}
}

/*
 * LP I/O control routine.
 *
 * Used by lpset(1) to set line printer parameters
 */

lpioctl(dev, cmd, addr, flag)
{
	struct	lpmode {
		char	lpm_flag;
		char	lpm_ind;
		int	lpm_line;
		int	lpm_col;
	} lp_mode;

	register struct lp *lp;
	register struct lpmode *lpm;

	if((dev&3) >= LPMAX) {
		u.u_error = ENODEV;
		return;
	}
	lp = &lp_dt[dev&3];
	lpm = &lp_mode;
	if(cmd == TIOCSETP) {
		copyin(addr, (caddr_t)lpm, sizeof(struct lpmode));
		/* would need unit # for multi-unit support */
/* We need to preserve the RAWLP bit, and probably should
   preserve the ASLP bit.
		lp->flag = (lpm->lpm_flag&070) | OPEN;
 */
		lp->flag &= ~070;			/*RAWLP*/
		lp->flag |= (lpm->lpm_flag&070) | OPEN;	/*RAWLP*/
		lp->ind = lpm->lpm_ind;
		lp->line = lpm->lpm_line;
		lp->col = lpm->lpm_col;
		return;
	}
	if(cmd == TIOCGETP) {
		lpm->lpm_flag = lp->flag;
		lpm->lpm_ind = lp->ind;
		lpm->lpm_line = lp->line;
		lpm->lpm_col = lp->col;
		copyout((caddr_t)lpm, addr, sizeof(struct lpmode));
		return;
	}
	u.u_error = ENOTTY;
}
