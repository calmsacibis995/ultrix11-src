

/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * ULTRIX-11 TM02/3 - 800/1600 BPI tape driver
 *
 * SCCSID: @(#)ht.c	3.0	4/21/86
 *
 * Chung-Wu Lee, Aug-09-85
 *
 *	add ioctl routine (from 2.9 BSD).
 *
 * Fred Canter, Aug-07-85
 *
 *	changes for 1K block file system.
 *
 * Fred Canter 3/11/82
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/buf.h>
#include <sys/conf.h>
#include <sys/dir.h>
#include <sys/file.h>
#include <sys/user.h>
#include <sys/errlog.h>
#include <sys/devmaj.h>
#include <sys/mtio.h>

/*
 * SOFT ERROR NO LOG
 *
 * If SENOLOG is defined, a soft write error
 * that is recovered on the first retry will
 * NOT be logged. This is done to cut down on
 * the number of nuisance error log entries
 * for soft tape errors. If a tape problem is
 * suspected, SENOLOG should be removed so that
 * all tape errors will be logged.
 */
#define	SENOLOG	1

struct	device
{
	int	htcs1;	/* HT control & status 1 register */
	int	htwc;	/* HT word count register */
	caddr_t	htba;	/* HT bus address register */
	int	htfc;	/* HT frame count register */
	int	htcs2;	/* HT control & status 2 register */
	int	htds;	/* HT drive status register */
	int	hter;	/* HT error register */
	int	htas;	/* HT attention summary register */
	int	htck;	/* HT character check register */
	int	htdb;	/* HT data buffer register */
	int	htmr;	/* HT maintenance register */
	int	htdt;	/* HT drive type register */
	int	htsn;	/* HT serial number register */
	int	httc;	/* HT tape control register */
	int	htbae;	/* HT bus address extension register */
	int	htcs3;	/* HT control & status 3 register */
};

struct	buf	httab;
struct	buf	rhtbuf;
struct	buf	chtbuf[];

#define	INF	1000000

char	ht_openf[];
u_short	ht_flags[];
daddr_t	ht_blkno[];
daddr_t	ht_nxrec[];
u_short	ht_erreg[];
u_short	ht_dsreg[];
short	ht_resid[];

/*
 * Block device error log buffer, holds one
 * error log record until error retry sequence
 * has been completed.
 */

struct
{
	struct elrhdr ht_hdr;	/* record header */
	struct el_bdh ht_bdh;	/* block device header */
	int ht_reg[NHTREG];	/* device registers at error time */
} ht_ebuf;

int	io_csr[];	/* CSR address now in config file (c.c) */
char	io_bae[];	/* (c.c) - bus address BAE register */
int	nht;		/* number of drives, see c.c */

#define	GO	01
#define	WCOM	060
#define	RCOM	070
#define	NOP	0
#define	SENSE	0
#define REWOFFL	02
#define	WEOF	026
#define	SFORW	030
#define	SREV	032
#define	ERASE	024
#define	REW	06
#define	DCLR	010
#define P800	01300		/* 800 + pdp11 mode */
#define	P1600	02300		/* 1600 + pdp11 mode */
#define	IENABLE	0100
#define	RDY	0200
#define	BOT	02
#define	TM	04
#define	DRY	0200
#define EOT	02000
#define	PEF	0200
#define	INC	0100
#define CS	02000
#define COR	0100000
#define PES	040
#define WRL	04000
#define MOL	010000
#define ERR	040000
#define FCE	01000
#define	TRE	040000
#define HARD	064023	/* UNS|OPI|NEF|FMT|RMR|ILR|ILF */

#define	CLR	040	/* controller clear (in cs2) */

#define	NED	010000

#define	SIO	1
#define	SSFOR	2
#define	SSREV	3
#define SRETRY	4
#define SCOM	5
#define SOK	6
 
#define	CLR_DRV	0	/* clear drive only */
#define	CLR_DC	1	/* clear both drive and controller */

#define H_WRITTEN 01	/* last command is write command */
#define H_EOT 010	/* EOT encountered */
#define H_CSE 020	/* clear EOT */
#define H_DEOT 040	/* disable EOT */
#define	H_BUSY 0100	/* chtbuf is busy, switch for clx/cls */
#define	H_WAIT 0200	/* chtbuf is waited, switch for clx/cls */
#define	H_BUSYF 01000	/* good termination of busy */
#define	H_WAITF 02000	/* good termination of wait */
htopen(dev, flag)
{
	register struct device *htaddr;
	register unit, ds;

	htaddr = io_csr[HT_RMAJ];
	httab.b_flags |= B_TAPE;
	unit = minor(dev) & 077;
	if (unit >= nht) {
		u.u_error = ENXIO;
		return;
	}
	if (flag&FNDELAY) {
		ht_openf[unit] = 1;
		return;
	}
	if (ht_openf[unit] > 0) {
		u.u_error = ETO;
		return;
	}
	ht_blkno[unit] = 0;
	ht_nxrec[unit] = INF;
	ht_flags[unit] &= H_DEOT;
	ht_openf[unit] = 0;
	if(htaddr->htcs1 & TRE)
		htinit(CLR_DC);
	if ((ds = hcommand(dev, NOP, 1)) == -1) {
		u.u_error = ENXIO;
	}
	if (ds & EOT)
		ht_flags[unit] |= H_EOT;
	if ((ds & MOL) == 0) {
/*		printf("ht%d off line\n", unit);	*/
		u.u_error = ETOL;
	}
	if((flag & FWRITE) && (ds & WRL)) {
/*		printf("ht%d needs write ring\n", unit);	*/
		u.u_error = ETWL;
	}
	if (u.u_error==0)
		ht_openf[unit] = 1;
	else
		ht_openf[unit] = 0;
}

htclose(dev, flag)
{
	register int unit;

	unit = minor(dev) & 077;
	if (ht_openf[unit] <= 0)
		return;
	if(((flag & (FWRITE|FREAD)) == FWRITE) ||
	  ((flag&FWRITE) && (ht_flags[unit]&H_WRITTEN))) {
		if (hcommand(dev, WEOF, 1) == -1)
			goto badcls;
		if (hcommand(dev, WEOF, 1) == -1)
			goto badcls;
		if (hcommand(dev, SREV, 1) == -1)
			goto badcls;
	}
	if ((minor(dev)&0200) == 0) {
		if (hcommand(dev, REW, 0) == -1)
			goto badcls;
		ht_flags[unit] &= ~H_EOT;
	}
	ht_openf[unit] = 0;
	return;

badcls:
	u.u_error = ENXIO;
	return;
}

htioctl(dev, cmd, data, flag)
	dev_t dev;
	int cmd;
	caddr_t data;
	int flag;
{
	register struct buf *bp;
	struct buf *wbp;
	register callcount;
	int fcount, unit;
	struct mtop *mtop;
	struct mtget *mtget;
	/* we depend of the values and order of the MT codes here */
	static htops[] =
	{WEOF,SFORW,SREV,SFORW,SREV,REW,REWOFFL,SENSE,NOP,NOP,NOP,DCLR,DCLR,
	 NOP,NOP,NOP,NOP};

	unit = minor(dev) & 077;
	bp = &chtbuf[unit];

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

		case MTREW: case MTOFFL:
			callcount = 1;
			fcount = 1;
			break;

		case MTCSE:
			ht_flags[unit] |= H_CSE;
			return(0);

		case MTENAEOT:
			ht_flags[unit] &= ~H_DEOT;
			return(0);

		case MTDISEOT:
			ht_flags[unit] |= H_DEOT;
			return(0);

		case MTCACHE: case MTNOCACHE:
		case MTASYNC: case MTNOASYNC:
			return(0);

		case MTCLX:
		case MTCLS:
			if ((wbp = httab.b_actf) != NULL) {
				httab.b_actf = wbp->av_forw;
				if (wbp != bp) {
					wbp->b_error = EIO;
					wbp->b_flags |= B_ERROR;
					iodone(wbp);
				}
			}
			if (ht_flags[unit]&H_BUSY) {
				bp->b_flags &= ~B_BUSY;
				ht_flags[unit] &= ~H_BUSYF;
				iodone(bp);
			}
			if (ht_flags[unit]&H_WAIT) {
				bp->b_flags &= ~B_WANTED;
				ht_flags[unit] &= ~H_WAITF;
				wakeup((caddr_t)bp);
			}
			if (mtop->mt_op == MTCLX)
				htinit(CLR_DRV);
			else
				htinit(CLR_DC);
			httab.b_active = 0;
			ht_flags[unit] &= H_DEOT;
			ht_blkno[unit] = 0;
			ht_nxrec[unit] = INF;
			ht_openf[unit] = 0;
			htstart();
			return(0);

		default:
			return (ENXIO);
		}
		if (callcount <= 0 || fcount <= 0)
			return (EINVAL);
		while (--callcount >= 0) {
			if (hcommand(dev, htops[mtop->mt_op], fcount) == -1) {
				u.u_error= ENXIO;
				return(0);
			}
			if ((mtop->mt_op == MTFSR || mtop->mt_op == MTBSR) &&
			    bp->b_resid)
				return (EIO);
			if ((bp->b_flags&B_ERROR) || ht_dsreg[unit]&BOT)
				break;
		}
		return (geterror(bp));	

	case MTIOCGET:
		mtget = (struct mtget *)data;
		mtget->mt_dsreg = ht_dsreg[unit];
		mtget->mt_erreg = ht_erreg[unit];
		mtget->mt_resid = ht_resid[unit];
		mtget->mt_softstat = 0;
		if(ht_flags[unit]&H_EOT)
			mtget->mt_softstat |= MT_EOT;
		if(ht_flags[unit]&H_DEOT)
			mtget->mt_softstat |= MT_DISEOT;
		mtget->mt_type = MT_ISHT;
		break;

	default:
		return (ENXIO);
	}
	return (0);
}

static
hcommand(dev, com, count)
{
	register struct buf *bp;
	int unit;

	unit = minor(dev)&077;
	bp = &chtbuf[unit];
	spl5();
	while(bp->b_flags&B_BUSY) {
		bp->b_flags |= B_WANTED;
		ht_flags[unit] |= (H_WAIT|H_WAITF);
		sleep((caddr_t)bp, PRIBIO);
		if ((ht_flags[unit]&H_WAITF) == 0)
			return(-1);
	}
	spl0();
	bp->b_dev = dev;
	bp->b_resid = com;
	bp->b_bcount = count;
	bp->b_blkno = 0;
	bp->b_flags = B_BUSY|B_READ;
	ht_flags[unit] |= (H_BUSY|H_BUSYF);
	htstrategy(bp);
	iowait(bp);
	if ((ht_flags[unit]&H_BUSYF) == 0)
		return(-1);
	if(bp->b_flags&B_WANTED) {
		ht_flags[unit] &= ~H_WAIT;
		wakeup((caddr_t)bp);
	}
	bp->b_flags &= B_ERROR;
	return(bp->b_resid);
}

htstrategy(bp)
register struct buf *bp;
{
	register struct device *htaddr;
	register daddr_t *p;

	htaddr = io_csr[HT_RMAJ];
	if(!io_bae[HT_BMAJ])
		mapalloc(bp);
	if(bp != &chtbuf[minor(bp->b_dev)&077]) {
		p = &ht_nxrec[minor(bp->b_dev)&077];
#ifdef	UCB_NKB
		if(dbtofsb(bp->b_blkno) > *p) {
#else
		if(bp->b_blkno > *p) {
#endif	UCB_NKB
			bp->b_flags |= B_ERROR;
			bp->b_error = ENXIO;
			iodone(bp);
			return;
		}
#ifdef	UCB_NKB
		if(dbtofsb(bp->b_blkno) == *p && bp->b_flags&B_READ) {
#else
		if(bp->b_blkno == *p && bp->b_flags&B_READ) {
#endif	UCB_NKB
			if((bp->b_flags&B_PHYS) == 0)
				clrbuf(bp);
			bp->b_resid = bp->b_bcount;
			iodone(bp);
			return;
		}
		ht_flags[minor(bp->b_dev)&077] &= ~H_WRITTEN;
		if ((bp->b_flags&B_READ)==0) {
#ifdef	UCB_NKB
			*p = dbtofsb(bp->b_blkno) + 1;
#else
			*p = bp->b_blkno + 1;
#endif	UCB_NKB
			ht_flags[minor(bp->b_dev)&077] |= H_WRITTEN;
		}
	}
	bp->av_forw = NULL;
	spl5();
	if (httab.b_actf == NULL)
		httab.b_actf = bp;
	else
		httab.b_actl->av_forw = bp;
	httab.b_actl = bp;
	if (httab.b_active==0)
		htstart();
	spl0();
}

static
htstart()
{
	register struct buf *bp;
	register struct device *htaddr;
	register unit;
	int den;
	daddr_t blkno;

	htaddr = io_csr[HT_RMAJ];
    loop:
	if ((bp = httab.b_actf) == NULL)
		return;
	unit = minor(bp->b_dev) & 0177;
	htaddr->htcs2 = ((unit>>3)&07);
	den = P1600 | (unit&07);
	if(unit > 077)
		den = P800 | (unit&07);
	if((htaddr->httc&03777) != den)
		htaddr->httc = den;
	if (htaddr->htcs2 & NED || (htaddr->htds&MOL)==0) {
		ht_openf[unit&077] = -1;
		bp->b_error = ETOL;
/*		goto abort;	*/
	}
	unit &= 077;
	blkno = ht_blkno[unit];
	if (bp == &chtbuf[unit]) {
		if (bp->b_resid==NOP) {
			bp->b_resid = htaddr->htds;
			goto next;
		}
		httab.b_active = SCOM;
		htaddr->htfc = -bp->b_bcount;
		htaddr->htcs1 = bp->b_resid|IENABLE|GO;
		return;
	}
	if(ht_openf[unit] < 0) {
		bp->b_error = ETPL;
		goto abort;
	}
#ifdef	UCB_NKB
	if(dbtofsb(bp->b_blkno) > ht_nxrec[unit])
#else
	if(bp->b_blkno > ht_nxrec[unit])
#endif	UCB_NKB
		goto abort;
#ifdef	UCB_NKB
	if (blkno == dbtofsb(bp->b_blkno)) {
#else
	if (blkno == bp->b_blkno) {
#endif	UCB_NKB
		httab.b_active = SIO;
		el_bdact |= (1 << HT_BMAJ);	/* device is active */
		htaddr->htba = bp->b_un.b_addr;
		if(io_bae[HT_BMAJ])
			htaddr->htbae = bp->b_xmem;
		htaddr->htfc = -bp->b_bcount;
		htaddr->htwc = -(bp->b_bcount>>1);
		den = ((bp->b_xmem&3) << 8) | IENABLE | GO;
		if((ht_flags[unit]&H_DEOT) == 0) {
			if((htaddr->htds&EOT) && (ht_flags[unit]&H_CSE) == 0) {
				bp->b_resid = bp->b_bcount;
				bp->b_error = ENOSPC;
				bp->b_flags |= B_ERROR;
				goto abort;
			}
		}
		if(bp->b_flags & B_READ)
			den |= RCOM;
		else
			den |= WCOM;
		htaddr->htcs1 = den;
	} else {
#ifdef	UCB_NKB
		if (blkno < dbtofsb(bp->b_blkno)) {
#else
		if (blkno < bp->b_blkno) {
#endif	UCB_NKB
			httab.b_active = SSFOR;
#ifdef	UCB_NKB
			htaddr->htfc = blkno - dbtofsb(bp->b_blkno);
#else
			htaddr->htfc = blkno - bp->b_blkno;
#endif	UCB_NKB
			htaddr->htcs1 = SFORW|IENABLE|GO;
		} else {
			httab.b_active = SSREV;
#ifdef	UCB_NKB
			htaddr->htfc = dbtofsb(bp->b_blkno) - blkno;
#else
			htaddr->htfc = bp->b_blkno - blkno;
#endif	UCB_NKB
			htaddr->htcs1 = SREV|IENABLE|GO;
		}
	}
	return;

    abort:
	bp->b_flags |= B_ERROR;

    next:
	httab.b_active = 0;
	httab.b_actf = bp->av_forw;
	if (bp == &chtbuf[unit])
		ht_flags[unit] &= ~H_BUSY;
	iodone(bp);
	goto loop;
}

htintr()
{
	register struct buf *bp;
	register struct device *htaddr;
	register int unit;
	int	state;
	int	err;
	int	nreg;

	htaddr = io_csr[HT_RMAJ];
	if ((bp = httab.b_actf)==NULL) {
		logsi(htaddr);
		return;
		}
	unit = minor(bp->b_dev) & 077;
	ht_dsreg[unit] = htaddr->htds;
	ht_erreg[unit] = htaddr->hter;
	ht_resid[unit] = htaddr->htfc;
	state = httab.b_active;
	httab.b_active = 0;
	if(htaddr->htds&EOT)
		ht_flags[unit] |= H_EOT;
	else
		ht_flags[unit] &= ~(H_EOT|H_CSE);
	if (htaddr->htcs1&TRE) {
		if(httab.b_errcnt == 0) {
			if(io_bae[HT_BMAJ])
				nreg = NHTREG;
			else
				nreg = (NHTREG - 2);
			fmtbde(bp, &ht_ebuf, htaddr, nreg, HTDBOFF);
			}
		err = htaddr->hter;
		if (htaddr->htcs2&077400 || (err&HARD))
			state = 0;
		if (bp == &rhtbuf)
			err &= ~FCE;
		if ((bp->b_flags&B_READ) && (htaddr->htds&PES))
			err &= ~(CS|COR);
		if((htaddr->htds&MOL) == 0) {
			if(ht_openf[unit])
				ht_openf[unit] = -1;
		}
		else if(htaddr->htds&TM) {
			htaddr->htwc = -(bp->b_bcount>>1);
#ifdef	UCB_NKB
			ht_nxrec[unit] = dbtofsb(bp->b_blkno);
#else
			ht_nxrec[unit] = bp->b_blkno;
#endif	UCB_NKB
			state = SOK;
		}
		else if(state && err == 0)
			state = SOK;
		htinit(CLR_DC);
		if (state==SIO && ++httab.b_errcnt < 10) {
			httab.b_active = SRETRY;
			ht_blkno[unit]++;
			htaddr->htfc = -1;
			htaddr->htcs1 = SREV|IENABLE|GO;
			return;
		}
		if (state!=SOK) {
			bp->b_error = ETPL;
			bp->b_flags |= B_ERROR;
			ht_openf[unit] = -1;
			state = SIO;
		}
	} else if (htaddr->htcs1 < 0) {	/* SC */
		if(htaddr->htds & ERR)
			htinit(CLR_DC);
	}
	switch(state) {
	case SIO:
	case SOK:
		ht_blkno[unit]++;

	case SCOM:
		if(httab.b_errcnt || (bp->b_flags & B_ERROR))
		{
			ht_ebuf.ht_bdh.bd_errcnt = httab.b_errcnt;
#ifdef SENOLOG
			if((ht_ebuf.ht_reg[5] & PES)
			  && (httab.b_errcnt == 1)
			  && ((ht_ebuf.ht_reg[6] & ~(CS|COR|PEF|INC)) == 0))
				goto nolog;
#endif
			if(!logerr(E_BD, &ht_ebuf, sizeof(ht_ebuf)))
			    deverror(bp, ht_ebuf.ht_reg[6], ht_ebuf.ht_reg[4]);
		}
	nolog:
		httab.b_errcnt = 0;
		el_bdact &= ~(1 << HT_BMAJ);	/* device no longer active */
		httab.b_actf = bp->av_forw;
		if (bp == &chtbuf[unit])
			ht_flags[unit] &= ~H_BUSY;
		iodone(bp);
		bp->b_resid = (-htaddr->htwc)<<1;
		break;

	case SRETRY:
		if((bp->b_flags&B_READ)==0) {
			httab.b_active = SSFOR;
			htaddr->htcs1 = ERASE|IENABLE|GO;
			return;
		}

	case SSFOR:
	case SSREV:
		if(htaddr->htds & TM) {
			if(state == SSREV) {
#ifdef	UCB_NKB
				ht_nxrec[unit] = dbtofsb(bp->b_blkno) - htaddr->htfc;
#else
				ht_nxrec[unit] = bp->b_blkno - htaddr->htfc;
#endif	UCB_NKB
				ht_blkno[unit] = ht_nxrec[unit];
			} else {
#ifdef	UCB_NKB
				ht_nxrec[unit] = dbtofsb(bp->b_blkno) + htaddr->htfc - 1;
#else
				ht_nxrec[unit] = bp->b_blkno + htaddr->htfc - 1;
#endif	UCB_NKB
				ht_blkno[unit] = ht_nxrec[unit]+1;
			}
		} else
#ifdef	UCB_NKB
			ht_blkno[unit] = dbtofsb(bp->b_blkno);
#else
			ht_blkno[unit] = bp->b_blkno;
#endif	UCB_NKB
		break;

	default:
		return;
	}
	htstart();
}

static
htinit(flag)
{
	register struct device *htaddr;
	register ocs2;
	register omttc;
	
	htaddr = io_csr[HT_RMAJ];
	omttc = htaddr->httc & 03777;	/* preserve old slave select, dens, format */
	ocs2 = htaddr->htcs2 & 07;	/* preserve old unit */

	if (flag == CLR_DC)
		htaddr->htcs2 = CLR;
	htaddr->htcs2 = ocs2;
	htaddr->httc = omttc;
	htaddr->htcs1 = DCLR|GO;
}

htread(dev)
{
	htphys(dev);
	physio(htstrategy, &rhtbuf, dev, B_READ);
}

htwrite(dev)
{
	htphys(dev);
	physio(htstrategy, &rhtbuf, dev, B_WRITE);
}

static
htphys(dev)
{
	register unit;
	daddr_t a;

	unit = minor(dev) & 077;
	if(unit < nht) {
#ifdef	UCB_NKB
		a = dbtofsb(u.u_offset >> 9);
#else
		a = u.u_offset >> 9;
#endif	UCB_NKB
		ht_blkno[unit] = a;
		ht_nxrec[unit] = a+1;
	}
}
