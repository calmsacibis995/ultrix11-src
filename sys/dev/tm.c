
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * ULTRIX-11 TM11 - 800 BPI tape driver
 *
 * SCCSID: @(#)tm.c	3.0	4/21/86
 *
 * Chung-Wu Lee, Aug-09-85
 *
 *	add ioctl routine (from 2.9 BSD).
 *
 * Fred Canter, Aug-07-85
 *
 *	Changes for 1K block file system.
 *
 * Fred Canter 3/11/82
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/buf.h>
#include <sys/dir.h>
#include <sys/conf.h>
#include <sys/file.h>
#include <sys/user.h>
#include <sys/errlog.h>
#include <sys/devmaj.h>
#include <sys/mtio.h>

struct device {
	int	tmer;	/* TM status register */
	int	tmcs;	/* TM command register */
	int	tmbc;	/* TM byte record count register */
	char	*tmba;	/* TM current memory address register */
	int	tmdb;	/* TM data buffer register */
	int	tmrd;	/* TM read data lines */
};
/*
 * Fred Canter 8/30/81
 *
 * `tmtab' is now in data space instead of bss,
 * so that tapes which look like the TM11
 * will work. For some reason, if tmtab is in bss space
 * it gets corrupted and the tape hangs, thinks its open
 * but really is not ?
 * What makes this even stranger is that it works fine
 * on a real DEC TM11 tape !
 */
struct	buf	tmtab = 0;
struct	buf	ctmbuf[];
struct	buf	rtmbuf;

char	tm_openf[];
u_short	tm_flags[];
daddr_t	tm_blkno[];
daddr_t	tm_nxrec[];
u_short	tm_erreg[];
u_short	tm_dsreg[];
short	tm_resid[];

/*
 * Block device error log buffer, holds one
 * error log record until error retry sequence
 * has been completed.
 */

struct
{
	struct elrhdr tm_hdr;	/* record header */
	struct el_bdh tm_bdh;	/* block device header */
	int tm_reg[NTMREG];	/* device registers at error time */
} tm_ebuf;

int	io_csr[];	/* CSR address now in config file (c.c) */
int	ntm;		/* number of drives, see c.c */

int	wakeup();
int	hz;

#define	OFFL	0
#define	GO	01
#define	RCOM	02
#define	RWS	02
#define	WCOM	04
#define	WEOF	06
#define	SELR	0100
#define	NOP	0100
#define	SFORW	010
#define	SREV	012
#define	WIRG	014
#define	FUNC	016
#define	REW	016
#define	DCLR	010000
#define	DENS	060000		/* 9-channel */
#define	IENABLE	0100
#define	CRDY	0200
#define GAPSD	010000
#define	TUR	1
#define	HARD	0102200	/* ILC, EOT, NXM */
#define RLE	01000
#define	EOF	0040000
#define	EOT	0002000
#define	BOT	0000040
#define	WL	04

#define	SSEEK	1
#define	SIO	2
#define	SCOM	3

#define	INF	1000000

#define T_WRITTEN 01	/* last command is write command */
#define	T_EOT	  010	/* EOT encountered */
#define	T_CSE	  020	/* clear EOT */
#define	T_DEOT	  040	/* disable EOT */
#define	T_BUSY	  0100	/* ctnbuf is busy, switch for clx/cls */
#define	T_WAIT	  0200	/* ctmbuf is waited, switch for clx/cls */
#define	T_BUSYF	  01000	/* good termination of busy */
#define	T_WAITF	  02000	/* good termination of wait */

tmopen(dev, flag)
{
	register unit, ds;

	unit = minor(dev) & 07;
	if (unit >= ntm) {
		u.u_error = ENXIO;
		return;
	}
	if (flag&FNDELAY) {
		tm_openf[unit] = 1;
		return;
	}
	if (tm_openf[unit] > 0) {
		u.u_error = ETO;
		return;
	}
	tm_blkno[unit] = 0;
	tm_nxrec[unit] = 65535;
	tm_flags[unit] &= T_DEOT;

	tmtab.b_flags |= B_TAPE;
	if ((ds = tcommand(dev, NOP, 1)) == -1) {
		u.u_error = ENXIO;
		return;
	}
	if (ds & EOT)
		tm_flags[unit] |= T_EOT;
	if ((ds&TUR)==0) {
/*		printf("mt%d off line\n",unit);	*/
		u.u_error = ETOL;
	}
	if ((flag & FWRITE) && (ds & WL)) {
/*		printf("mt%d needs write ring\n",unit);	*/
		u.u_error = ETWL;
	}
	if (u.u_error==0)
		tm_openf[unit] = 1;
	else
		tm_openf[unit] = 0;
}

tmclose(dev, flag)
dev_t dev;
int flag;
{
	register int unit;

	unit = minor(dev) & 07;
	if (tm_openf[unit] <= 0)
		return;
	if(((flag & (FWRITE|FREAD)) == FWRITE) ||
	   ((flag&FWRITE) && (tm_flags[unit]&T_WRITTEN))) {
		if (tcommand(dev, WEOF, 1) == -1)
			goto badcls;
		if (tcommand(dev, WEOF, 1) == -1)
			goto badcls;
		if (tcommand(dev, SREV, 1) == -1)
			goto badcls;
	}
	if ((minor(dev)&0200) == 0) {
		if (tcommand(dev, REW, 0) == -1)
			goto badcls;
		tm_flags[unit] &= ~T_EOT;
	}
	tm_openf[unit] = 0;
	return;

badcls:
	u.u_error = ENXIO;
	return;
}

tmioctl(dev, cmd, data, flag)
	caddr_t data;
	dev_t dev;
{
	int unit;
	register struct buf *bp;
	struct buf *wbp;
	register callcount;
	int fcount;
	struct mtop *mtop;
	struct mtget *mtget;
	struct device *tmaddr;
	/* we depend of the values and order of the MT codes here */
	static tmops[] =
	   {WEOF,SFORW,SREV,SFORW,SREV,REW,OFFL,NOP,NOP,NOP,NOP,DCLR,DCLR,
	    NOP,NOP,NOP,NOP};

	tmaddr = io_csr[TM_RMAJ];
	unit = minor(dev) & 07;
	bp = &ctmbuf[unit];
	switch (cmd) {

	case MTIOCTOP:	/* tape operation */
		mtop = (struct mtop *)data;
		switch (mtop->mt_op) {

		case MTWEOF:
			callcount = mtop->mt_count;
			fcount = 1;
			break;

		case MTFSF: case MTBSF:
			callcount = mtop->mt_count;
			fcount = INF;
			break;

		case MTFSR: case MTBSR:
			callcount = 1;
			fcount = mtop->mt_count;
			break;

		case MTREW: case MTOFFL: case MTNOP:
			callcount = 1;
			fcount = 1;
			break;

		case MTCACHE: case MTNOCACHE:
		case MTASYNC: case MTNOASYNC:
			return(0);

		case MTCSE:
			tm_flags[unit] |= T_CSE;
			return(0);

		case MTENAEOT:
			tm_flags[unit] &= ~T_DEOT;
			return(0);

		case MTDISEOT:
			tm_flags[unit] |= T_DEOT;
			return(0);

		case MTCLX:
			return(0);

		case MTCLS:
			if ((wbp = tmtab.b_actf) != NULL) {
				tmtab.b_actf = wbp->av_forw;
				if (wbp != bp) {
					wbp->b_error = EIO;
					wbp->b_flags |= B_ERROR;
					iodone(wbp);
				}
			}
			if (tm_flags[unit]&T_BUSY) {
				bp->b_flags &= ~B_BUSY;
				tm_flags[unit] &= ~T_BUSYF;
				iodone(bp);
			}
			if (tm_flags[unit]&T_WAIT) {
				bp->b_flags &= ~B_WANTED;
				tm_flags[unit] &= ~T_WAITF;
				wakeup((caddr_t)bp);
			}
			tmaddr->tmcs = DCLR;
			tm_blkno[unit] = 0;
			tm_nxrec[unit] = 65535;
			tm_flags[unit] &= T_DEOT;
			tmtab.b_active == NULL;
			tmstart();
			return(0);

		default:
			return (ENXIO);
		}
		if (callcount <= 0 || fcount <= 0)
			return (EINVAL);
		while (--callcount >= 0) {
			if (tcommand(dev, tmops[mtop->mt_op], fcount) == -1) {
				u.u_error = ENXIO;
				return;
			}
			if ((mtop->mt_op == MTFSR || mtop->mt_op == MTBSR) &&
			    bp->b_resid)
				return (EIO);
			if ((bp->b_flags&B_ERROR) || tm_erreg[unit]&BOT)
				break;
		}
		return (geterror(bp));

	case MTIOCGET:
		mtget = (struct mtget *)data;
		mtget->mt_dsreg = tm_dsreg[unit];
		mtget->mt_erreg = tm_erreg[unit];
		mtget->mt_resid = tm_resid[unit];
		mtget->mt_softstat = 0;
		if (tm_flags[unit]&T_EOT)
			mtget->mt_softstat |= MT_EOT;
		if (tm_flags[unit]&T_DEOT)
			mtget->mt_softstat |= MT_DISEOT;
		mtget->mt_type = MT_ISTM;
		break;

	default:
		return (ENXIO);
	}
	return (0);
}

static
tcommand(dev, com, count)
{
	register struct buf *bp;
	int	unit;

	unit = minor(dev) & 07;
	bp = &ctmbuf[unit];
	spl5();
	while (bp->b_flags&B_BUSY) {
		bp->b_flags |= B_WANTED;
		tm_flags[unit] |= (T_WAIT|T_WAITF);
		sleep((caddr_t)bp, PRIBIO);
		if ((tm_flags[unit]&T_WAITF) == 0)
			return(-1);
	}
	bp->b_flags = B_BUSY|B_READ;
	spl0();
	bp->b_dev = dev;
	bp->b_resid = com;
	bp->b_bcount = count;
	bp->b_blkno = 0;
	tm_flags[unit] |= (T_BUSY|T_BUSYF);
	tmstrategy(bp);
	iowait(bp);
	if ((tm_flags[unit]&T_BUSYF) == 0)
		return(-1);
	if (bp->b_flags&B_WANTED) {
		tm_flags[unit] &= ~T_WAIT;
		wakeup((caddr_t)bp);
	}
	bp->b_flags &= B_ERROR;
	return(bp->b_resid);
}

tmstrategy(bp)
register struct buf *bp;
{
	register daddr_t *p;
	register struct device *tmaddr;
	int	unit;

	tmaddr = io_csr[TM_RMAJ];
	mapalloc(bp);
	unit = minor(bp->b_dev)&07;
	if (bp != &ctmbuf[unit]) {
		p = &tm_nxrec[unit];
#ifdef	UCB_NKB
		if (*p <= dbtofsb(bp->b_blkno)) {
#else
		if (*p <= bp->b_blkno) {
#endif	UCB_NKB
#ifdef	UCB_NKB
			if (*p < dbtofsb(bp->b_blkno)) {
#else
			if (*p < bp->b_blkno) {
#endif	UCB_NKB
				bp->b_flags |= B_ERROR;
				bp->b_error = ENXIO;
				iodone(bp);
				return;
			}
			if (bp->b_flags&B_READ) {
				if((bp->b_flags&B_PHYS) == 0)
					clrbuf(bp);
				bp->b_resid = bp->b_bcount;
				iodone(bp);
				return;
			}
		}
		tm_flags[unit] &= ~T_WRITTEN;
		if ((bp->b_flags&B_READ) == 0) {
			tm_flags[unit] |= T_WRITTEN;
#ifdef	UCB_NKB
			*p = dbtofsb(bp->b_blkno) + 1;
#else
			*p = bp->b_blkno+1;
#endif	UCB_NKB
		}
	}
	bp->av_forw = 0;
	spl5();
	if (tmtab.b_actf == NULL)
		tmtab.b_actf = bp;
	else
		tmtab.b_actl->av_forw = bp;
	tmtab.b_actl = bp;
	if (tmtab.b_active == NULL)
		tmstart();
	spl0();
}

static
tmstart()
{
	register struct buf *bp;
	register struct device *tmaddr;
	register daddr_t *blkno;
	int com;
	int unit;

	tmaddr = io_csr[TM_RMAJ];
    loop:
	if ((bp = tmtab.b_actf) == 0)
		return;
	unit = minor(bp->b_dev)&07;
	blkno = &tm_blkno[unit];
	if (tm_openf[unit] < 0 || (tmaddr->tmcs & CRDY) == NULL) {
		bp->b_flags |= B_ERROR;
		bp->b_error = ETOL;
		goto next;
	}
	if (bp == &ctmbuf[unit]) {
		if (bp->b_resid == NOP) {
			tmaddr->tmcs = (unit<<8);
			bp->b_resid = tmaddr->tmer;
			goto next;
		}
		tmtab.b_active = SCOM;
		tmaddr->tmbc = -bp->b_bcount;
		tmaddr->tmcs = DENS|bp->b_resid|GO| (unit<<8) | IENABLE;
		return;
	}
	com = (unit<<8) | ((bp->b_xmem & 03) << 4) | IENABLE|DENS;
#ifdef	UCB_NKB
	if (*blkno != dbtofsb(bp->b_blkno)) {
#else
	if (*blkno != bp->b_blkno) {
#endif	UCB_NKB
		tmtab.b_active = SSEEK;
#ifdef	UCB_NKB
		if (*blkno < dbtofsb(bp->b_blkno)) {
#else
		if (*blkno < bp->b_blkno) {
#endif	UCB_NKB
			com |= SFORW|GO;
#ifdef	UCB_NKB
			tmaddr->tmbc = *blkno - dbtofsb(bp->b_blkno);
#else
			tmaddr->tmbc = *blkno - bp->b_blkno;
#endif	UCB_NKB
		} else {
			if (bp->b_blkno == 0)
				com |= REW|GO;
			else {
				com |= SREV|GO;
#ifdef	UCB_NKB
				tmaddr->tmbc = dbtofsb(bp->b_blkno) - *blkno;
#else
				tmaddr->tmbc = bp->b_blkno - *blkno;
#endif	UCB_NKB
			}
		}
		tmaddr->tmcs = com;
		return;
	}
	if ((tm_flags[unit]&T_DEOT) == 0) {
		if ((tmaddr->tmer&EOT) && (tm_flags[unit]&T_CSE) == 0) {
			bp->b_resid = bp->b_bcount;
			bp->b_error = ENOSPC;
			bp->b_flags |= B_ERROR;
			goto next;
		}
	}
	tmtab.b_active = SIO;
	tmaddr->tmbc = -bp->b_bcount;
	tmaddr->tmba = bp->b_un.b_addr;
	tmaddr->tmcs = com | ((bp->b_flags&B_READ)? RCOM|GO:
	    ((tmtab.b_errcnt)? WIRG|GO: WCOM|GO));
	el_bdact |= (1 << TM_BMAJ);	/* device is active */
	return;

next:
	tmtab.b_active == NULL;
	tmtab.b_actf = bp->av_forw;
	if (bp == &ctmbuf[unit])
		tm_flags[unit] &= ~T_BUSY;
	iodone(bp);
	goto loop;
}

tmintr()
{
	register struct buf *bp;
	register struct device *tmaddr;
	register int unit;
	int	state;

	tmaddr = io_csr[TM_RMAJ];
	if(((tmaddr->tmcs & FUNC) == REW) && (tmaddr->tmer & RWS))
		return;
	if ((bp = tmtab.b_actf) == NULL) {
		logsi(tmaddr);
		return;
		}
	unit = minor(bp->b_dev)&07;
	tm_erreg[unit] = tmaddr->tmer;
	tm_resid[unit] = -tmaddr->tmbc;
	state = tmtab.b_active;
	tmtab.b_active = 0;
	if (tmaddr->tmer&EOT)
		tm_flags[unit] |= T_EOT;
	else
		tm_flags[unit] &= ~(T_EOT|T_CSE);
	if (tmaddr->tmcs < 0) {		/* error bit */
		if(tmtab.b_errcnt == 0)
			fmtbde(bp, &tm_ebuf, tmaddr, NTMREG, TMDBOFF);
		while(tmaddr->tmrd & GAPSD) ; /* wait for gap shutdown */
		if (tmaddr->tmer&EOT)
			goto out;
		if (tmaddr->tmer&EOF) {
#ifdef	UCB_NKB
			tm_nxrec[unit] = dbtofsb(bp->b_blkno);
#else
			tm_nxrec[unit] = bp->b_blkno;
#endif	UCB_NKB
			state = SCOM;
			tmaddr->tmbc = -bp->b_bcount;
			goto out;
		}
		if ((tmaddr->tmer&HARD) == 0 && tmaddr->tmer&RLE) {
			state = SIO;
			goto out;
		}
		if ((tmaddr->tmer&(HARD|EOF)) == NULL && state==SIO) {
			if (++tmtab.b_errcnt < 10) {
				tm_blkno[unit]++;
				el_bdact &= ~(1 << TM_BMAJ);
				tmtab.b_active = 0;
				tmstart();
				return;
			}
		} else
			if (tm_openf[unit]>0 && bp!=&rtmbuf &&
				(tmaddr->tmer&EOF)==0 )
				tm_openf[unit] = -1;
		bp->b_flags |= B_ERROR;
		bp->b_error = ETPL;
		state = SIO;
	}
out:
	switch ( state ) {
	case SIO:
		tm_blkno[unit] += (bp->b_bcount>>BSHIFT);
	case SCOM:
		if(tmtab.b_errcnt || (bp->b_flags & B_ERROR))
		{
			tm_ebuf.tm_bdh.bd_errcnt = tmtab.b_errcnt;
			if(!logerr(E_BD, &tm_ebuf, sizeof(tm_ebuf)))
			    deverror(bp, tm_ebuf.tm_reg[0], tm_ebuf.tm_reg[1]);
		}
		tmtab.b_errcnt = 0;
		tmtab.b_actf = bp->av_forw;
		bp->b_resid = -tmaddr->tmbc;
		el_bdact &= ~(1 << TM_BMAJ);
		if (bp == &ctmbuf[unit])
			tm_flags[unit] &= ~T_BUSY;
		iodone(bp);
		break;
	case SSEEK:
#ifdef	UCB_NKB
		tm_blkno[unit] = dbtofsb(bp->b_blkno);
#else
		tm_blkno[unit] = bp->b_blkno;
#endif	UCB_NKB
		el_bdact &= ~(1 << TM_BMAJ);
		break;
	default:
		return;
	}
	tmstart();
}

tmread(dev)
{
	tmphys(dev);
	physio(tmstrategy, &rtmbuf, dev, B_READ);
}

tmwrite(dev)
{
	tmphys(dev);
	physio(tmstrategy, &rtmbuf, dev, B_WRITE);
}

static
tmphys(dev)
{
	register unit;
	daddr_t a;

	unit = minor(dev) & 07;
	if(unit < ntm) {
#ifdef	UCB_NKB
		a = dbtofsb(u.u_offset >> 9);
#else
		a = u.u_offset >> 9;
#endif	UCB_NKB
		tm_blkno[unit] = a;
		tm_nxrec[unit] = a+1;
	}
}
