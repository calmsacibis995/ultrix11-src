
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)rp.c	3.0	4/21/86
 */
/*
 * Unix/v7m RP02/3 disk driver
 *
 * Fred Canter 6/16/83
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/buf.h>
#include <sys/dir.h>
#include <sys/conf.h>
#include <sys/user.h>
#include <sys/errlog.h>
#include <sys/devmaj.h>

struct device {
	int	rpds;		/* RP drive status register */
	int	rper;		/* RP error register */
	union {
		int	w;
		char	c;
	} rpcs;			/* RP control & status register */
	int	rpwc;		/* RP word count register */
	char	*rpba;		/* RP bus address register */
	int	rpca;		/* RP cylinder address register */
	int	rpda;		/* RP disk address register */
	int	rpm1;		/* RP maintenance 1 register */
	int	rpm2;		/* RP maintenance 2 register */
	int	rpm3;		/* RP maintenance 3 register */
	int	rpsuca;		/* RP selected unit cylinder address register */
	int	rpsilo;		/* RP silo memory register */
};

int	io_csr[];	/* CSR address now in config file (c.c) */
int	nrp;		/* number of drives, see c.c */

/*
 * Sizes table now in /usr/sys/conf/dksizes.c
 */
extern	struct {
	daddr_t	nblocks;
	int	cyloff;
} rp_sizes[];

/*
 * Block device error log buffer, holds one error log record
 * until the error retry sequence has been completed.
 */
struct
{
	struct	elrhdr	rp_hdr;	/* record header */
	struct	el_bdh	rp_bdh;	/* block device header */
	int	rp_reg[NRPREG];	/* device registers at error time */
} rp_ebuf;

/*
 * Drive type,
 * -1 = nonexistent drive
 *  0 = RP02
 *  1 = RP03
 * size MUST be 8, independent of nrp.
 */
#define	RP02	0
#define	RP03	1

char	rp_dt[8] = {-1,-1,-1,-1,-1,-1,-1,-1};

char	rp_opn;
struct	buf	rptab;
struct	buf	rrpbuf;

#define	GO	01
#define	RESET	0
#define	HSEEK	014

#define	IENABLE	0100
#define	READY	0200
#define	RCOM	4
#define	WCOM	2

#define	SUFU	01000
#define	SUSU	02000
#define	SUSI	04000
#define	HNF	010000
#define	SUOL	040000

/*
 * Use av_back to save track+sector,
 * b_resid for cylinder.
 */

#define	trksec	av_back
#define	cylin	b_resid

/*
 * Monitoring device number
 * and iostat structure.
 */
struct	ios	rp_ios[];
#define	DK_N	4
#define	DK_T	8	/* RP03 transfer rate indicator */

rpstrategy(bp)
register struct buf *bp;
{
	register struct buf *dp;
	register int unit;
	int pri;
	long sz;

	mapalloc(bp);
	unit = minor(bp->b_dev);
	sz = bp->b_bcount;
	sz = (sz+511)>>9;
	if (unit >= (nrp<<3) ||
	   bp->b_blkno+sz > rp_sizes[unit&07].nblocks) {
		bp->b_flags |= B_ERROR;
		bp->b_error = ENXIO;
		iodone(bp);
		return;
	}
	bp->av_forw = NULL;
	unit >>= 3;
	if(rp_opn == 0) {
		rp_opn++;
		pri = spl6();
		dk_iop[DK_N] = &rp_ios[0];
		dk_nd[DK_N] = nrp;
		splx(pri);
		}
	pri = spl5();
	dp = &rptab;
	if (dp->b_actf == NULL)
		dp->b_actf = bp;
	else
		dp->b_actl->av_forw = bp;
	dp->b_actl = bp;
	if (dp->b_active == NULL)
		rpstart();
	splx(pri);
}

rpstart()
{
	register struct buf *bp;
	register struct device *rpaddr;
	register int unit;
	int com,cn,tn,sn,dn;
	daddr_t bn;



	rpaddr = io_csr[RP_RMAJ];
	if ((bp = rptab.b_actf) == NULL)
		return;
	rptab.b_active++;
	unit = minor(bp->b_dev);
	dn = unit>>3;
	bn = bp->b_blkno;
	cn = bn/(20*10) + rp_sizes[unit&07].cyloff;
	sn = bn%(20*10);
	tn = sn/10;
	sn = sn%10;
	rpaddr->rpcs.w = (dn<<8);
/*
 * Check for drive available
 * and set rp_dt.
 */
	if((rpaddr->rpds & SUOL) == 0) {
	rp_bad:
		rptab.b_active = NULL;
		rptab.b_actf = bp->av_forw;
		bp->b_flags |= B_ERROR;
		bp->b_error = ENXIO;
		iodone(bp);
		return;
		}
	if((rpaddr->rpds & RP03) == 0)
		rp_dt[dn] = RP02;
	else
		rp_dt[dn] = RP03;
	rpaddr->rpda = (tn<<8) | sn;
	rpaddr->rpca = cn;
	rpaddr->rpba = bp->b_un.b_addr;
	rpaddr->rpwc = -(bp->b_bcount>>1);
	com = ((bp->b_xmem&3)<<4) | IENABLE | GO;
	if (bp->b_flags & B_READ)
		com |= RCOM; else
		com |= WCOM;
	rpaddr->rpcs.w |= com;
	el_bdact |= (1 << RP_BMAJ);
	rp_ios[dn].dk_tr = DK_T;
	rp_ios[dn].dk_busy++;	/* drive active */
	rp_ios[dn].dk_numb++;	/* count number of xfer's */
	unit = bp->b_bcount>>6;
	rp_ios[dn].dk_wds += unit;	/* count words xfer'd */
}

rpintr()
{
	register struct buf *bp;
	register struct device *rpaddr;
	register int ctr;

	rpaddr = io_csr[RP_RMAJ];
	if (rptab.b_active == NULL) {
		logsi(rpaddr);
		return;
		}
	bp = rptab.b_actf;
	ctr = (bp->b_dev >> 3) & 7;
	rp_ios[ctr].dk_busy = 0;	/* drive no longer active */
	rptab.b_active = NULL;
	if (rpaddr->rpcs.w < 0) {		/* error bit */
		if(rptab.b_errcnt == 0)
			fmtbde(bp, &rp_ebuf, rpaddr, NRPREG, RPDBOFF);
		if(rpaddr->rpds & (SUFU|SUSI|HNF)) {
			rpaddr->rpcs.c = HSEEK|GO;
			ctr = 0;
			while ((rpaddr->rpds&SUSU) && --ctr)
				;
		}
		rpaddr->rpcs.w = RESET|GO;
		ctr = 0;
		while ((rpaddr->rpcs.w&READY) == 0 && --ctr)
			;
		if (++rptab.b_errcnt <= 10) {
			rpstart();
			return;
		}
		bp->b_flags |= B_ERROR;
	}
	if(rptab.b_errcnt || bp->b_flags&B_ERROR)
	{
	    if(rp_ebuf.rp_reg[1] < 0)	/* WRT PROT VIOLATION */
		printf("RP unit %d Write Locked\n", (bp->b_dev>>3)&7);
	    else {
		rp_ebuf.rp_bdh.bd_errcnt = rptab.b_errcnt;
		if(!logerr(E_BD, &rp_ebuf, sizeof(rp_ebuf)))
			deverror(bp, rp_ebuf.rp_reg[1], rp_ebuf.rp_reg[0]);
	    }
	}
	rptab.b_errcnt = 0;
	el_bdact &= ~(1 << RP_BMAJ);
	rptab.b_actf = bp->av_forw;
	if(bp->b_flags&B_ERROR)
		bp->b_resid = bp->b_bcount;
	else
		bp->b_resid = 0;
	iodone(bp);
	rpstart();
}

rpread(dev)
{

	physio(rpstrategy, &rrpbuf, dev, B_READ);
}

rpwrite(dev)
{

	physio(rpstrategy, &rrpbuf, dev, B_WRITE);
}

rpclose(dev,flag)
dev_t dev;
{

	bflclose(dev);
}
