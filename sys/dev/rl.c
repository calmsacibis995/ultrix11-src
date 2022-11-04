
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)rl.c	3.0	4/21/86
 */
/*
 * ULTRIX-11 RL01/2 disk driver
 *
 * This driver is based on the standard V7 RL driver.
 *
 * 7/80 - Modified by Armando P. Stettner to support both
 *	RL01 and RL02 drives on the same controller.
 *
 * 5/81 - Modified by Fred Canter to improve drive type
 *	identification and disk block number overflow checking.
 *
 * 9/81 - Modified by Fred Canter to include the new
 *	iostat code.
 * 1/82 - Modified by Fred Canter for error logging
 *	and on-line device exercisers.
 *
 * 3/82 - Modified by Fred Canter for 11/23+ (Q22 bus)
 * 11/83 - Fred Canter, partition disk into two file systems.
 *	Places swap area in center fo disk for better performance.
 *
 * 2/84 - Bill Burns & Dave Borman 
 * 	added overlapped seeks and queue sorting
 *
 ****************************************************************
 *								*
 *	This driver cannot handle switching unit numbers	*
 *	between unlike drive types, i.e., RL01 and RL02.	*
 *	This is because the driver determines the number of	*
 *	blocks on each drive at the time of the first open	*
 *	and has no way to dynamically update that information.	*
 ****************************************************************
 *
 * Fred Canter 11/5/83
 */

#include <sys/param.h>
#include <sys/buf.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/systm.h>
#include <sys/errlog.h>
#include <sys/devmaj.h>

#define	BLKRL1	10240		/* Number of UNIX blocks for an RL01 drive */
#define BLKRL2	20480		/* Number of UNIX blocks for an RL02 drive */
#define RLCYLSZ 10240		/* bytes per cylinder */
#define RLSECSZ 256		/* bytes per sector */

#define RESET 013
#define	RL02TYP	0200	/* drive type bit */
#define STAT 03
#define GETSTAT 04
#define WCOM 012
#define RCOM 014
#define SEEK 06
#define SEEKHI 5
#define SEEKLO 1
#define RDHDR 010
#define IENABLE 0100
#define CRDY 0200
#define OPI 02000
#define CRCERR 04000
#define TIMOUT 010000
#define NXM 020000
#define DE  040000

struct device
{
	int	rlcs;	/* RL control & status register */
	int	rlba;	/* RL bus addess register */
	int	rlda;	/* RL disk address register */
	int	rlmp;	/* RL multipurpose register */
	int	rlbae;	/* RL bus address extension register */
			/*    used by 11/23+ with Q22 bus only */
};

int	io_csr[];	/* CSR address now in config file (c.c) */
char	io_bae[];	/* BAE register offset, 0 = not Q22 bus */
int	nrl;		/* number of drives, see c.c */

/*
 * Block device error log buffer, holds one error log record
 * until the error retry sequence has been completed.
 */

struct
{
	struct	elrhdr	rl_hdr;	/* record header */
	struct	el_bdh	rl_bdh;	/* block device header */
	int	rl_reg[NRLREG+2];	/* device registers at error time */
} rl_ebuf;

/*
 * Instrumentation (iostat) structures
 */

#define	DK_N	5
#define	DK_T	9	/* RL xfer rate indicator */
struct	ios	rl_ios[];
struct	buf	rrlbuf;
struct	buf	rltab;
struct	buf	rlutab[];
int	rltimer();

/*
 * RL drive types
 * -1 = non existent drive
 * otherwise contains number of blocks for drive
 * 10240 for RL01, 20480 for RL02
 */
int rl_dt[] = {-1, -1, -1, -1};	/* maintained for usep */

char	rl_openf;			/* RL open flag */

/*
 * the drive type array has been added to the
 * following structure, and the structure has been
 * expanded to one per drive. This allows overlapped
 * seeks to work.
 */
struct
{
	int	cn;		/* location of heads */
	int	dt;		/* drive type */
	int	com;		/* read or write command word */
	int	chn;		/* cylinder and head number */
	unsigned int	bleft;	/* bytes left to be transferred */
	unsigned int	bpart;	/* number of bytes transferred */
	int	sn;		/* sector number */
	union {
		int	w[2];
		long	l;
	} addr;			/* address of memory for transfer */
} rl[4] = {
	{-1,-1},
	{-1,-1},
	{-1,-1},
	{-1,-1}
};

/*
 * On the first call to rlopen only,
 * then get the status of all possible drives
 * as follows:
 *
 * 1.	Execute a get status with reset up to 8 times
 *	to get a valid status from the drive.
 *
 * 2.	If an OPI error is detected then the drive
 *	is non-existent (NED).
 *
 * 3.	If a vaild status cannot be obtained after 8 attempts,
 *	then print the "can't get status" message and
 *	mark the drive non-existent.
 *
 * 4.	If a valid status is obtained, then use the drive
 *	type to set rl[unit].dt to the number of blocks 
 *	for that type drive.
 *	10240 for RL01 or 20480 for RL02
 *
 * The above sequence only occurs on the first access
 * of the RL disk driver. The drive status obtained is
 * retained until the system in rebooted.
 * Packs may be mounted and dismounted,
 * HOWEVER the disk unit number select plugs may
 * NOT be changed without rebooting the system.
 *
 ****************************************************************
 *								*
 * For some unknown reason the RL02 does not return a valid	*
 * status the first time a GET STATUS request is issued for	*
 * the drive, in fact it can take up to three or more		*
 * GET STATUS requests to obtain a valid drive status.		*
 * This is why the GET STATUS is repeated eight times		*
 * in step one above.						*
 *								*
 ****************************************************************
 */

rlopen()
{
	register struct device *rladdr;
	register int dn, ctr;

	if(rl_openf)
		return;
	rl_openf++;
	rladdr = io_csr[RL_RMAJ];
	for (dn=0; dn<nrl; dn++) {
		for(ctr=0; ctr<8; ctr++) {
			rladdr->rlda = RESET;
			rladdr->rlcs = (dn << 8) | GETSTAT;
			while ((rladdr->rlcs & CRDY) == 0) ;
			if(rladdr->rlcs & OPI)
				break;	/* NED */
			if((rladdr->rlmp & 0157400) == 0)
				break;	/* valid status */
		}
		if(rladdr->rlcs & OPI)
			continue;	/* NED */
		if(ctr >= 8) {
			printf("\nCan't get status of RL unit %d\n", dn);
			continue;
		}
		if(rladdr->rlmp & RL02TYP) {
			rl[dn].dt = BLKRL2;	/* RL02 */
			rl_dt[dn] = BLKRL2;	/* USEP */
		} else {
			rl[dn].dt = BLKRL1;	/* RL01 */
			rl_dt[dn] = BLKRL1;	/* USEP */
		}
		rl_ios[dn].dk_tr = DK_T;
	}
	ctr = spl6();
	dk_iop[DK_N] = &rl_ios[0];
	dk_nd[DK_N] = nrl;
	splx(ctr);
}

rlstrategy(bp)
register struct buf *bp;
{
	register struct buf *dp;	/* bpb */
	register int nblks;
	int dn, fs;
	long sz;

	if(!io_bae[RL_BMAJ])
		mapalloc(bp);
	dn = minor(bp->b_dev);
	fs = dn & 7;
	dn >>= 3;
	if((dn >= nrl) || (rl[dn].dt < 0) || (bp->b_blkno < 0))
		goto bad;
	if(fs == 7)
		nblks = rl[dn].dt;
	else
		nblks = 10240;
	sz = bp->b_bcount;
	sz = (sz+511) >> 9;
	if((bp->b_blkno + sz) > nblks)
		goto bad;
	if(bp->b_blkno >= nblks) {
		if(bp->b_blkno == nblks && bp->b_flags&B_READ)
			bp->b_resid = bp->b_bcount;
		else {
	bad:
			bp->b_flags |= B_ERROR;
			bp->b_error = ENXIO;
		}
		iodone(bp);
		return;
	}
	bp->av_forw = NULL;
	dp = &rlutab[dn];
	nblks = spl5();
	disksort(dp, bp);	/* sort into drive queue */

	/* if drive and controller are inactive... */
	if (dp->b_active == NULL && rltab.b_active == NULL) {
		rlustart(dn);	/* start drive seeking */
	}
	splx(nblks);
}

static
rlustart(unit)
int unit;
{
	register struct buf *bp, *dp;
	register struct device *rladdr;
	int dif, head, bn, mts;

	rladdr = io_csr[RL_RMAJ];
	dp = &rlutab[unit];

	mts = 0;
	if (( bp = dp->b_actf) == NULL)
		return;
    
	if(dp->b_active)
		mts++;		/* mid transfer seek */
	else
		dp->b_active++;		/* flag device queue as busy */

	if (rl[unit].cn < 0) {		/* find where the heads are */
		rladdr->rlcs = (unit << 8) | RDHDR;
		while ((rladdr->rlcs&CRDY) == 0)
			;
		rl[unit].cn = (rladdr->rlmp >> 6) & 01777;
	}
	if(mts == 0) {				/* not a mid-transfer seek */
		bn = bp->b_blkno;
		if((bp->b_dev & 7) == 1)
			bn += 10240;
		rl[unit].chn = bn/20;
		rl[unit].sn = (bn%20) << 1;
		rl[unit].bleft = bp->b_bcount;
	}

	dif = (rl[unit].cn >> 1) - (rl[unit].chn >> 1);
	head = (rl[unit].chn & 1) << 4;
	if (dif < 0) {
		rladdr->rlda = (-dif << 7) | SEEKHI | head;
		dif = -dif;			/* make dif positive */
	} else
		rladdr->rlda = (dif << 7) | SEEKLO | head;
	rladdr->rlcs = (unit << 8) | SEEK;
	while ((rladdr->rlcs&CRDY) == 0) 	/* wait for command start */
		;
	rl[unit].cn = rl[unit].chn;
	if(mts) {
		rlio(unit);
		return;
	}
	head =(56*dif)/(rl[unit].dt/4)+1;
	if (head < 1 || head > 6)
		head = 2;
	timeout(rltimer, (caddr_t)dp, head);
}

rltimer(dp)
register struct buf *dp;
{
	/* seek is (probably) complete on unit
	   let's link the drive queue to the rltab queue */

	dp->b_forw = NULL;
	if (rltab.b_actf == NULL)	/* link drive queue into i/o queue... */
		rltab.b_actf = dp;		/* at beginning of queue */
	else
		rltab.b_actl->b_forw = dp;	/* at end of queue */
	rltab.b_actl = dp;
	if (rltab.b_active == NULL)	/* if no i/o in progress lets go */
		rlstart();
}

static
rlstart()
{

	register struct buf *bp, *dp;
	register int unit;

	if ((dp = rltab.b_actf) == NULL)   /* dp points to drive queue head */
		return;
	bp = dp->b_actf;		/* bp points to actual buffer */
	rltab.b_active++;		/* mark i/o queue busy */
	unit = minor(bp->b_dev) >> 3;
	rl[unit].addr.w[0] = bp->b_xmem;
	rl[unit].addr.w[1] = (int)bp->b_un.b_addr;
	rl[unit].com = (unit << 8) | IENABLE;
	if (bp->b_flags & B_READ)
		rl[unit].com |= RCOM;
	else
		rl[unit].com |= WCOM;
	rl_ios[unit].dk_busy++;
	rl_ios[unit].dk_numb++;
	rl_ios[unit].dk_wds += (bp->b_bcount >> 6);
	el_bdact |= (1 << RL_BMAJ);
	rlio(unit);
}

static
rlio(unit)
int unit;
{

	register struct device *rladdr;
	register dif;
	register int head;

	rladdr = io_csr[RL_RMAJ];

	if(rl[unit].bleft < (rl[unit].bpart = RLCYLSZ - (rl[unit].sn * RLSECSZ)))
		rl[unit].bpart = rl[unit].bleft;
	rladdr->rlda = (rl[unit].chn << 6) | rl[unit].sn;
	rladdr->rlba = rl[unit].addr.w[1];
	if(io_bae[RL_BMAJ])	/* for 11/23+ with Q22 bus */
		rladdr->rlbae = rl[unit].addr.w[0];
	rladdr->rlmp = -(rl[unit].bpart >> 1);
	rladdr->rlcs = rl[unit].com | ((rl[unit].addr.w[0] & 3) << 4);
}

rlintr()
{
	register struct buf *bp, *dp;
	register struct device *rladdr;
	int status, unit;

	rladdr = io_csr[RL_RMAJ];
	if (rltab.b_active == NULL) {
		logsi(rladdr);
		return;
		}
	dp = rltab.b_actf;			/* head of drive queue */
	bp = dp->b_actf;			/* actual i/o buffer */
	unit = minor(bp->b_dev) >> 3;
	rl_ios[unit].dk_busy = 0;
	if (rladdr->rlcs < 0) {			/* error bit */
		if(rltab.b_errcnt == 0) {
			fmtbde(bp, &rl_ebuf, rladdr, NRLREG, RLDBOFF);
			rl_ebuf.rl_bdh.bd_nreg++;	/* fake 5 device regs */
			if(io_bae[RL_BMAJ]) {
				rl_ebuf.rl_reg[5] = rladdr->rlbae; /* Q22 bus */
				rl_ebuf.rl_bdh.bd_nreg++;	/* 6 reg's */
			}
		}
		rladdr->rlda = STAT;
		rladdr->rlcs = (unit << 8) | GETSTAT;
		while ((rladdr->rlcs & CRDY) == 0)
			;
		status = rladdr->rlmp;
		if(rltab.b_errcnt == 0)
			rl_ebuf.rl_reg[4] = status;    /* update rlmp */
/*
		if (rladdr->rlcs & 036000) {
			if(rltab.b_errcnt > 2)
				{
				printf("rlcs, rlda  ") ;
				deverror(bp, rladdr->rlcs, rladdr->rlda);
				}
		}
*/
		if (rladdr->rlcs & 040000) {
/*
			if(rltab.b_errcnt > 2)
				{
				printf("rlmp, rlda  ") ;
				deverror(bp, status, rladdr->rlda);
				}
*/
			rladdr->rlda = RESET;
			rladdr->rlcs = (unit << 8) | GETSTAT;
			while ((rladdr->rlcs & CRDY) == 0)
				;
			if(status & 01000) {
				rlstart();
				return;
			}
		}
		if (++rltab.b_errcnt <= 10) {
			rl[unit].cn = -1;
			rlustart(unit);	    /* treat like a mid-xfer seek */
			return;
		} else {
			bp->b_flags |= B_ERROR;
			rl[unit].bpart = rl[unit].bleft;
		}
	}

	if ((rl[unit].bleft -= rl[unit].bpart) > 0) {
		rl[unit].addr.l += rl[unit].bpart;
		rl[unit].sn=0;
		rl[unit].chn++;
		rlustart(unit);		/* mid transfer seek */
		return;
	}
	rltab.b_active = NULL;
	if(rltab.b_errcnt || bp->b_flags & B_ERROR)
	{
	    if((rl_ebuf.rl_reg[4] & 022000) == 022000)	/* WL * WGE */
		printf("RL unit %d Write Locked\n", unit);
	    else {
		rl_ebuf.rl_bdh.bd_errcnt = rltab.b_errcnt;
		if(!logerr(E_BD, &rl_ebuf, sizeof(rl_ebuf)))
		{
			deverror(bp, rl_ebuf.rl_reg[0], rl_ebuf.rl_reg[1]);
			deverror(bp, rl_ebuf.rl_reg[2], rl_ebuf.rl_reg[3]);
		}
	    }
	}
	el_bdact &= ~(1 << RL_BMAJ);
	rltab.b_errcnt = 0;
	rltab.b_actf = dp->b_forw;	/* get next drive from i/o queue */
	if (rltab.b_actf == NULL)	/* XXX */
		rltab.b_actl = NULL;	/* XXX */
	if(bp->b_flags & B_ERROR)
		bp->b_resid = bp->b_bcount;
	else
		bp->b_resid = 0;
	
	dp->b_actf = bp->av_forw;		/* clean up drive queue */
	dp->b_active = NULL;
	dp->b_errcnt = NULL;
	iodone(bp);
	for (unit = 0; unit < nrl; unit++) {	/* start drives seeking */
		dp = &rlutab[unit];
		if (dp->b_active == NULL && dp->b_actf != NULL)
			rlustart(unit);
	}
	rlstart();
}

rlread(dev)
{

	physio(rlstrategy, &rrlbuf, dev, B_READ);
}

rlwrite(dev)
{

	physio(rlstrategy, &rrlbuf, dev, B_WRITE);
}

rlclose(dev,flag)
dev_t dev;
{

	bflclose(dev);
}
