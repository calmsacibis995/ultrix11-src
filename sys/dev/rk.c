
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)rk.c	3.0	4/21/86
 */
/*
 * Unix/v7m RK03/5 disk driver
 *
 * Fred Canter 10/9/82
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/buf.h>
#include <sys/conf.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/errlog.h>
#include <sys/devmaj.h>

#define	NRKBLK	4872

#define	RESET	0
#define	WCOM	2
#define	RCOM	4
#define	GO	01
#define	DRESET	014
#define	IENABLE	0100
#define	DRY	0200
#define	NED	0200
#define	ARDY	0100
#define	RK05	04000
#define	WLO	020000
#define	CTLRDY	0200

struct	device
{
	int	rkds;		/* RK drive status register */
	int	rker;		/* RK error register */
	int	rkcs;		/* RK control and status register */
	int	rkwc;		/* RK word count register */
	caddr_t	rkba;		/* RK bus address register */
	int	rkda;		/* RK disk address register */
	int	rkmr;		/* RK maintenance register */
	int	rkdb;		/* RK data buffer register */
};

int	io_csr[];	/* CSR address now in config file (c.c) */
int	nrk;		/* number of drives, see c.c */
char	rk_openf;	/* RK open flag */
char	rk_dt[8];	/* RK drive type, size MUST be 8 */
			/* 0 = NED, 1 = drive present */
			/* Required by RK disk exerciser */
/*
 * Block device error log buffer, holds one error record
 * until the error retry sequence has been completed.
 */

struct
{
	struct	elrhdr	rk_hdr;	/* record header */
	struct	el_bdh	rk_bdh;	/* block device header */
	int	rk_reg[NRKREG];	/* device registers at error time */
} rk_ebuf;

struct	buf	rktab;
struct	buf	rrkbuf;

/*
 * Monitoring device number
 * and iostat structure.
 */

struct	ios	rk_ios[];
#define	DK_N	6
#define	DK_T	10

/*
 * On the first call to rkopen only.
 * Assume that all drives are rk05 disks and
 * that they are present, even if they are not !
 * This is done because a drive sizing method,
 * consistent across the various flavors of RK11
 * controller, could not be found.
 */

rkopen()
{
	register int dn, pri;

	if(rk_openf)
		return;
	rk_openf++;
	for(dn=0; dn<nrk; dn++)
		rk_dt[dn]++;
	pri = spl6();
	dk_iop[DK_N] = &rk_ios[0];
	dk_nd[DK_N] = nrk;
	splx(pri);
}

rkstrategy(bp)
register struct buf *bp;
{
	int pri, dn;
	long sz;

	mapalloc(bp);
	dn = minor(bp->b_dev) & 7;
	sz = bp->b_bcount;
	sz = (sz+511) >> 9;
	if((dn >= nrk) ||
	  (rk_dt[dn] == 0) ||
	  (bp->b_blkno < 0) ||
	  ((bp->b_blkno + sz) > NRKBLK)) {
		bp->b_flags |= B_ERROR;
		bp->b_error = ENXIO;
		iodone(bp);
		return;
	}
	bp->av_forw = (struct buf *)NULL;
	pri = spl5();
	if(rktab.b_actf == NULL)
		rktab.b_actf = bp;
	else
		rktab.b_actl->av_forw = bp;
	rktab.b_actl = bp;
	if(rktab.b_active == NULL)
		rkstart();
	splx(pri);
}

rkstart()
{
	register struct buf *bp;
	register struct device *rkaddr;
	register com;
	daddr_t bn;
	int dn, cn, sn;

	rkaddr = io_csr[RK_RMAJ];
	if ((bp = rktab.b_actf) == NULL)
		return;
	rktab.b_active++;
	el_bdact |= (1 << RK_BMAJ);
	bn = bp->b_blkno;
	dn = minor(bp->b_dev);
	cn = bn/12;
	sn = bn%12;
	rkaddr->rkda = (dn<<13) | (cn<<4) | sn;
	if((rkaddr->rkds & DRY) == 0) {
		rktab.b_active = NULL;
		rktab.b_actf = bp->av_forw;
		bp->b_flags |= B_ERROR;
		bp->b_error = ENXIO;
		iodone(bp);
		return;
	}
	rkaddr->rkba = bp->b_un.b_addr;
	rkaddr->rkwc = -(bp->b_bcount>>1);
	com = ((bp->b_xmem&3) << 4) | IENABLE | GO;
	if(bp->b_flags & B_READ)
		com |= RCOM; else
		com |= WCOM;
	rkaddr->rkcs = com;
	rk_ios[dn].dk_tr = DK_T;
	rk_ios[dn].dk_busy++;
	rk_ios[dn].dk_numb++;
	com = bp->b_bcount >> 6;
	rk_ios[dn].dk_wds += com;
}

rkintr()
{
	register struct buf *bp;
	register struct device *rkaddr;

	rkaddr = io_csr[RK_RMAJ];
	if (rktab.b_active == NULL) {
		logsi(rkaddr);
		return;
		}
	bp = rktab.b_actf;
	rk_ios[bp->b_dev & 7].dk_busy = 0;
	if (rkaddr->rkcs < 0) {		/* error bit */
		if(rktab.b_errcnt == 0)
			fmtbde(bp, &rk_ebuf, rkaddr, NRKREG, RKDBOFF);
		rkaddr->rkcs = RESET|GO;
		while((rkaddr->rkcs&CTLRDY) == 0)
			;
		if (++rktab.b_errcnt <= 10) {
			rkstart();
			return;
		}
		bp->b_flags |= B_ERROR;
	}
	rktab.b_active = NULL;
	if(rktab.b_errcnt || bp->b_flags & B_ERROR)
	{
	    if(rk_ebuf.rk_reg[1] & 020000)
		printf("RK unit %d Write Locked\n", bp->b_dev&7);
	    else {
		rk_ebuf.rk_bdh.bd_errcnt = rktab.b_errcnt;
		if(!logerr(E_BD, &rk_ebuf, sizeof(rk_ebuf)))
			deverror(bp, rk_ebuf.rk_reg[1], rk_ebuf.rk_reg[0]);
	    }
	}
	el_bdact &= ~(1 << RK_BMAJ);
	rktab.b_errcnt = 0;
	rktab.b_actf = bp->av_forw;
	if(bp->b_flags & B_ERROR)
		bp->b_resid = bp->b_bcount;
	else
		bp->b_resid = 0;
	iodone(bp);
	rkstart();
}

rkread(dev)
dev_t dev;
{

	physio(rkstrategy, &rrkbuf, dev, B_READ);
}

rkwrite(dev)
dev_t dev;
{

	physio(rkstrategy, &rrkbuf, dev, B_WRITE);
}

rkclose(dev,flag)
dev_t dev;
{

	bflclose(dev);
}
