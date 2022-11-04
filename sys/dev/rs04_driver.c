/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985.	      *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/include/COPYRIGHT" for applicable restrictions.  *
 **********************************************************************/
/*
 * SCCSID: %Z%%M%	%I%	%G%
 */
/*
 * ULTRIX-11 SAMPLE DEVICE DRIVER for RS03/4 disks.
 *
 * This driver is based on the UNIX V6 RS03/4 driver.
 * It has worked, but it has been modified extensively
 * and those modifications have NOT yet been tested!
 * The changes were to improve the block number overflow
 * test in hsstrategy and to use io_bae[HS_BMAJ] in place of cputype.
 * Error logging and on-line exerciser support has
 * also been added.
 *
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/buf.h>
#include <sys/conf.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/devmaj.h>
#include <sys/errlog.h>

struct device {
	int	hscs1;	/* Control and Status register 1 */
	int	hswc;	/* Word count register */
	int	hsba;	/* UNIBUS address register */
	int	hsda;	/* Desired address register */
	int	hscs2;	/* Control and Status register 2 */
	int	hsds;	/* Drive Status */
	int	hser;	/* Error register */
	int	hsas;	/* not used */
	int	hsla;	/* not used */
	int	hsdb;	/* not used */
	int	hsmr;	/* not used */
	int	hsdt;
	int	hsbae;	/* 11/70 bus extension */
	int	hscs3;	/* 11/70 control & status 3 */
};

int	io_csr[];	/* CSR address now in c.c */
char	io_bae[];	/* BAE address offset, 0 if not rh70 */
int	nhs;		/* number of drives, see c.c */

struct	buf	hstab;
struct	buf	rhsbuf;

/*
 * Block device error log buffer, holds one
 * error log record until the error retry sequence
 * has been completed.
 */

struct
{
	struct	elrhdr	hs_hdr;	/* record header */
	struct	el_bdh	hs_bdh;	/* block device header */
	int	hs_reg[NHSREG];	/* device registers at error time */
} hs_ebuf;

/*
 * Drive types,
 *  0 = RS03
 *  1 = RS03 sector interleaved
 *  2 = RS04
 *  3 = RS04 sector interleaved
 * -1 = nonexistent drive
 */

char	hs_dt[] = {-1,-1,-1,-1,-1,-1,-1,-1};	/* size MUST be 8 */
char	hs_openf;	/* HS open flag */

#define	INTRLV	01	/* sector interleave bit */
#define ERR	040000	/* hscs1 - composite error */
#define	NED	010000	/* hscs2 - nonexistent drive */
#define GO	01
#define RCLR	010
#define	CCLR	040
#define	DRY	0200	/* hsds - Drive Ready */
#define	WCOM	060
#define	RCOM	070
#define	IENABLE	0100

/*
 * Monitoring device number
 * and iostat structure.
 */

struct	ios	hs_ios[];
#define	DK_N	7
#define	DK_T	11

/*
 * On the first call to hsopen only,
 * verify the existence and type of all drives.
 * Also print a warning message and mark the
 * drive nonexistent if the transfer rate is too
 * fast for the CPU. The RS03/4 must be sector interleaved
 * on all but 11/70 CPUs.
 */
hsopen()
{
	register struct device *hsaddr;
	register int dn, pri;

	if(hs_openf)
		return;
	hs_openf++;
	hsaddr = io_csr[HS_RMAJ];
	for(dn=0; dn<nhs; dn++) {
		hsaddr->hscs2 = dn;
		hs_dt[dn] = hsaddr->hsdt & 0377;
		if(hsaddr->hscs2 & NED) {
			hs_dt[dn] = -1;
			hsaddr->hscs2 = CCLR;
		}
		if((hs_dt[dn] >= 0) &&
		  ((hs_dt[dn] & INTRLV) == 0) &&
		  (cputype != 70)) {
			hs_dt[dn] = -1;
			printf("\nHS unit %d must be interleaved\n", dn);
		}
	pri = spl6();
	dk_iop[DK_N] = &hs_ios[0];
	dk_nd[DK_N] = nhs;
	splx(pri);
	}
}

hsstrategy(bp)
register struct buf *bp;
{
	register int mblks;
	int dn, pri;
	long sz;


	if(!io_bae[HS_BMAJ])
		mapalloc(bp);
	dn = minor(bp->b_dev) & 7;
	sz = bp->b_bcount;
	sz = (sz+511) >> 9;
	if(hs_dt[dn] & 2)
		mblks = 2048;	/* RS04 */
	else
		mblks = 1024;	/* RS03 */
	if((hs_dt[dn] < 0) ||
	  (dn >= nhs) ||
	  (bp->b_blkno < 0) ||
	  ((bp->b_blkno + sz) > mblks)) {
		bp->b_error = ENXIO;
		bp->b_flags |= B_ERROR;
		iodone(bp);
		return;
	}
	bp->av_forw = 0;
	pri = spl5();
	if (hstab.b_actf==0)
		hstab.b_actf = bp; else
		hstab.b_actl->av_forw = bp;
	hstab.b_actl = bp;
	if (hstab.b_active==0)
		hsstart();
	splx(pri);
}

hsstart()
{
	register struct device *hsaddr;
	register struct buf *bp;
	register addr;
	int unit, com;

	hsaddr = io_csr[HS_RMAJ];
	if ((bp = hstab.b_actf) == 0)
		return;
	hstab.b_active++;
	addr = bp->b_blkno;
	unit = minor(bp->b_dev) & 7;
	if((hs_dt[unit] & 2) == 0)
		addr <<= 1; /* RS03 */
	hsaddr->hscs2 = unit;
	hsaddr->hsda = addr << 1;
	hsaddr->hsba = bp->b_un.b_addr;
	if(io_bae[HS_BMAJ])
		hsaddr->hsbae = bp->b_xmem;
	hsaddr->hswc = -(bp->b_bcount>>1);
	com = ((bp->b_xmem&3) << 8) | IENABLE | GO;
	if(bp->b_flags & B_READ)
		com |= RCOM; else
		com |= WCOM;
	hsaddr->hscs1 = com;
	el_bdact |= (1 << HS_BMAJ);
	hs_ios[unit].dk_tr = DK_T + hs_dt[unit];
	hs_ios[unit].dk_busy++;
	hs_ios[unit].dk_numb++;
	addr = bp->b_bcount >> 6;
	hs_ios[unit].dk_wds += addr;
}

hsintr()
{
	register struct device *hsaddr;
	register struct buf *bp;
	register int nreg;
	int as;

	hsaddr = io_csr[HS_RMAJ];
	as = hsaddr->hsas & 0377;
	hsaddr->hsas = as;
	if ((hstab.b_active == NULL) && (as == 0)) {
		logsi(hsaddr);
		return;
		}
	bp = hstab.b_actf;
	hs_ios[bp->b_dev & 7].dk_busy = 0;
	if(hsaddr->hscs1 & ERR){	/* error bit */
		if(hstab.b_errcnt == 0) {
			if(!io_bae[HS_BMAJ])
				nreg = NHSREG - 2;
			else
				nreg = NHSREG;
			fmtbde(bp, &hs_ebuf, hsaddr, nreg, HSDBOFF);
			}
		hsaddr->hscs1 = RCLR|GO;
		if (++hstab.b_errcnt <= 10) {
			hsstart();
			return;
		}
		bp->b_flags |= B_ERROR;
	}
	if(hstab.b_errcnt || (bp->b_flags & B_ERROR))
	{
		hs_ebuf.hs_bdh.bd_errcnt = hstab.b_errcnt;
		if(!logerr(E_BD, &hs_ebuf, sizeof(hs_ebuf)))
			deverror(bp, hs_ebuf.hs_reg[4], hs_ebuf.hs_reg[6]);
	}
	el_bdact &= ~(1 << HS_BMAJ);
	hstab.b_active = 0;
	hstab.b_errcnt = 0;
	hstab.b_actf = bp->av_forw;
	bp->b_resid = -(hsaddr->hswc<<1);
	iodone(bp);
	hsstart();
}

hsread(dev)
{

	physio(hsstrategy, &rhsbuf, dev, B_READ);
}

hswrite(dev)
{

	physio(hsstrategy, &rhsbuf, dev, B_WRITE);
}
