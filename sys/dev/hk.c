
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)hk.c	3.0	4/21/86
 */
/*
 * Unix/v7m RK06/7 disk driver
 * Fred Canter 3/10/82
 *
 * This driver supports, overlapped seek,
 * interleaved file systems, head offset positioning,
 * and full ECC recovery on block and raw I/O transfers.
 * It does NOT support the dual port option !!!!!!
 *
 * Thanks to Jerry Brenner for 95% of this driver !
 *
 * Jerry Brenner.  Bad blocking added 1/11/83
 */

/*
 * QBUS turns on code that enables rk07 look-a-likes
 * on a 22-bit Q-bus to work. -Dave Borman, 11/19/85
 */
#define	QBUS


#include <sys/param.h>
#include <sys/systm.h>
#include <sys/buf.h>
#include <sys/conf.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/seg.h>
#include <sys/errlog.h>
#include <sys/devmaj.h>
#include <sys/bads.h>
#include <sys/hkbad.h>
#include <sys/uba.h>

#define HKEOFF	      1       /* if 1 then enable offset recovery */

/*
 *
 *	The following  structure defines the order of the Control
 *	and Status registers for the RK06/07
 *
 */
struct device
{
	int	hkcs1;	/* Control and Status register 1 */
	int	hkwc;	/* Word Count Register */
	caddr_t hkba;	/* Memory Address Register */
	int	hkda;	/* Track Sector Address Register */
	int	hkcs2;	/* Control And Status Register 2 */
	int	hkds;	/* Drive Status Register */
	int	hkerr;	/* Error Register */
	union{
		int	w; /* High byte is Attention Summary */
		char c[2]; /* Low is Offset Reg. */
	}hkas;
	int	hkdc;	/* Desired Cylinder Register */
	int	dum;	/* Dummy for unused location */
	int	hkdb;	/* Data Buffer */
	int	hkmr1;	/* Maintenance Register 1 */
	int	hkec1;	/* ECC Position Register */
	int	hkec2;	/* ECC Pattern Register */
	int	hkmr2;	/* Maint. Register 2 */
	int	hkmr3;	/* Maint. Register 3 */
};

int	io_csr[];	/* CSR address now in config file (c.c) */
int	nhk;		/* number of drives, see c.c */
#define NSECT	22	/* number of sectors per pack */
#define NTRAC	3	/* number of tracks per cylinder */
#define HK6BAD	27104L	/* Bad sector info block number for RK06 */
#define HK7BAD	53768L	/* Bad sector info block number for RK07 */
#define NUMTRY 28	/* max retry count */

/*
 * Sizes table now in /usr/sys/conf/dksizes.c
 */
extern	struct {
	daddr_t	nblocks;
	int	cyloff;
} hk_sizes[];

/*
 * Block device error log buffer, holds one
 * error log record until error retry sequence
 * has been completed.
 */

struct
{
	struct elrhdr hk_hdr;	/* record header */
	struct el_bdh hk_bdh;	/* block device header */
	int hk_reg[NHKREG];	/* device registers at error time */
} hk_ebuf;

/*
 *
 * The following definitions specify the offset values
 * used during error recovery
 *
 */
#ifdef HKEOFF
#define P25	01	/* +25 Rk06, +12.5 Rk07 */
#define M25	0201	/* -25 RK06, -12.5 RK07 */
#define	P200	010	/* +200 RK06, +100 RK07 */
#define M200	0210	/* -200, RK06, -100 RK07 */
#define P400	020	/* +400  RK06 , +200  RK07 */
#define M400	0220	/* -400  RK06 , -200  RK07 */
#define P800	040	/* +800  RK06 , +400 RK07 */
#define M800	0240	/* -800  RK06 , -400  RK07 */
#define P1200	060	/* +1200  RK06 , +600  RK07 */
#define M1200	0260	/* -1200  RK06 , -600 Rk07 */
/*
 *
 *	The following array defines the order in which the offset values defined above
 *	are used during error recovery.
 *
 */
int	hk_offset[16] =
{
	P25, M25, P200, M200,
	P400, M400, P400, M400,
	P800, M800, P800, M800,
	P1200, M1200, P1200, M1200,
};
#endif

/*
*	Control and Status bit definitions for  hkcs1
*/
#define GO	01	/* GO bit */
#ifdef	SELECT
#undef	SELECT
#endif
#define SELECT	01	/* Select Function */
#define PAKACK	02	/* Pack Acknowledge Function */
#define DCLR	04	/* Drive Clear Function */
#define UNLOAD	06	/* Unload Heads Function */
#define STRSPN	010	/* Start Spindle Function */
#define RECAL	012	/* Recalibrate Function */
#define OFFSET	014	/* Offset Function */
#define SEEK	016	/* Seek Function */
#define RCOM	020	/* Read Command */
#define WCOM	022	/* Write Command */
#define RHDR	024	/* Read Header	*/
#define WHDR	026	/* Write Header */
#define WCHK	030	/* Write Check Function */
#define IEI	0100	/* Interrupt Inable bit */
#define CRDY	0200	/* Controller Ready bit */
#define SEL7	02000	/* Select RK07 bit */
#define CTO	04000	/* Controller Time Out bit */
#define CFMT	010000	/* Controller Format bit */
#define DTCPAR	020000	/* Drive to Controller Parity Error */
#define DINTR	040000	/* Drive Interrupt bit */
#define CCLR	0100000 /* Controller Clear bit */
#define CERR	0100000 /* Controller Error bit */

/*
*	Control and Status bit definitions for  hkcs2
*/
#define DLT	0100000 /* Data Late error */
#define WCE	040000	/* Write Check Error */
#define UPE	020000	/* Unibus Parity Error */
#define NED	010000	/* Nonexistent Drive error */
#define NEM	04000	/* Nonexistent Memory error */
#define PGE	02000	/* Programming error */
#define MDS	01000	/* Multiple Drive Select error */
#define UFE	0400	/* Unit Field Error */
#define SCLR	040	/* Subsystem Clear bit */
#define BAI	020	/* Bus Address Increment Inhibit bit */
#define RLS	010	/* Release bit	*/

/*
*	Control and Status bit definitions for  hkds
*/
#define SVAL	0100000 /* Status Valid bit */
#define CDA	040000	/* Current Drive Attention bit */
#define PIP	020000	/* Position In Progress bit */
#define WRL	04000	/* Write Lock bit */
#define DDT	0400	/* Disk Drive Type bit */
#define DRDY	0200	/* Drive Ready bit */
#define VV	0100	/* Volume Valid bit */
#define DROT	040	/* Drive Off Track Error */
#define SPLS	020	/* Speed Loss Error	*/
#define ACLO	010	/* Drive AC Low */
#define DOFST	04	/* Drive Offset bit */
#define DRA	01	/* Drive Available bit */

/*
*	Control and Status bit definitions for  hkerr
*/
#define DCK	0100000 /* Data Check error */
#define DUNS	040000	/* Drive Unsafe error */
#define OPI	020000	/* Operation Incomplete error */
#define DTE	010000	/* Drive Timing error */
#define DWLE	04000	/* Drive Write Lock error */
#define IDAE	02000	/* Invalid Disk Address Error */
#define COE	01000	/* Cylinder Overflow Error	*/
#define HRVC	0400	/* Header Vertical Redundance Check Error */
#define BSE	0200	/* Bad Sector Error */
#define ECH	0100	/* Error Correction Hard error */
#define DTYE	040	/* Drive Type error */
#define FMTE	020	/* Format Error */
#define CDPAR	010	/* Controller To Driver Parity Error */
#define NXF	04	/* Nonexecutable Function Error */
#define SKI	02	/* Seek Incomplete error */
#define ILF	01	/* Illegal Function Error */

/*
 * HK drive types,
 * the size of this array MUST be 8
 * because it is used by the HK disk exerciser
 * as well as by this driver.
 *
 *	0	= RK06
 *	02000	= RK07
 *	-1	= NED
 */

int	hk_dt[8] = { -1,-1,-1,-1,-1,-1,-1,-1,};
char	hk_openf;	/* HK open flag */

struct
{
	char recal;	/* recalibrate flag */
	char bads;	/* bad block proccessing active flag */
	int  ccyl;	/* stores drives current cylinder  */
} hk_r[8];	/* size MUST be 8 */

struct buf	hktab;	/* This is the buffer header used exclusivly by the RK06/07 for block I/O */
			/* the structure is the same as buf.h but useage is slightly different */

struct buf	rhkbuf; /* This buffer header is used by the RK06/07 for raw I/O. struct. same as hktab */

struct buf	hkutab[];	 /* Unit headers for overlapped seek driver */

struct hkbad hk_bads[];

struct dkres hk_res;	/* Bads revector and continue structure */

#define trksec	av_back
#define cylin	b_resid
/*
 * Instrumentation (iostat) structures
 */

struct	ios	hk_ios[];
#define DK_N	3
#define	DK_T	3	/* RK06/7 transfer rate indicator */

#define b_cylin b_resid

/********   flag field defines for hk tables   **********/
#define HKBUSY	01	/* drive or controller is busy */
#define HKSEEK	02	/* drive is seeking */
#define HKOFFS	04	/* drive in offset mode */

#define HKRECAL 01	/* drive recal in progress */
#define HKRRECAL 02	/* drive recal request */

/*
 * On the first call to hkopen only,
 * attempt to mount all drives to determine
 * if they exist and what type they are.
 * Also set up the iostat interface to the clock().
 */

hkopen()
{
	register dn, pri;

	if(hk_openf == 0) {
		hk_openf++;
		for(dn=0; dn<nhk; dn++) 
			hkmnt(dn);
	pri = spl6();
	dk_iop[DK_N] = &hk_ios[0];
	dk_nd[DK_N] = nhk;
	splx(pri);
	}
}

/*
*
*	The following routine determines the drive type of the selected unit,
*	saves drive type in hk_dt[].
*
*/
static
hkmnt(dn)
{
	register struct device *hkaddr;
	register int pri;

	hkaddr = io_csr[HK_RMAJ];
	hk_dt[dn] = 0;	/* set default drive type to RK06 */
	pri = spl5();	/* can't allow HK interrupts durring mount */
			/* hkcmd() does spl5() also, but it is still */
			/* needed here to prevent an NED interrupt ! */
	hkcmd(dn, GO);	   /* select drive */
	if((hkaddr->hkcs1 & CERR) && (hkaddr->hkerr & DTYE))
	{
		hk_dt[dn] = SEL7;	/* Drive type is RK07 */
		hkcmd(dn, (DCLR|GO));	/* clear error bits */
		hkcmd(dn, GO);		/* get status */
	}
	if((hkaddr->hkcs1 & CERR) == 0)
		hk_ios[dn].dk_tr = DK_T;   /* (iostat) xfer rate indicator */
	else {
		hkcmd(dn, IEI);		/* clear controller */
		hk_dt[dn] = -1;		/* nonexistent drive */
	}
	splx(pri);
}

/*
*
*	hkstrategy checks for overflow errors
*
*/
hkstrategy(bp)
register struct buf *bp;
{
	register struct buf *dp;
	register struct device *hkaddr;
	int unit;
	long sz, bn;
	int dn, pri;

	hkaddr = io_csr[HK_RMAJ];
	mapalloc(bp);	/* phys I/O. see about allocating ubmap */
	unit = minor(bp->b_dev) & 077;	/* unit number from buffer header */
	dn = (unit >> 3) & 07;		/* get drive number	*/
	sz = bp->b_bcount;		/* size from buffer header */
	sz = (sz+511)>>9;		/* convert to number of blocks */

	/* if unit requested is greater than number available 
	* or
	* block number is < 0
	* or
	* block number +size is > number of blocks in file system
	* or
	* mount flag is less than 0. Drive not available or no drive
	*
	* ERROR
	*/
	if(unit >= (nhk<<3)
		|| bp->b_blkno < 0
		|| (bn = bp->b_blkno)+sz > hk_sizes[unit&007].nblocks)
	{
		bp->b_error = ENXIO;
		bp->b_flags |= B_ERROR;		/* set ERROR flag */
		iodone(bp);	/* call iodone to return buffer to pool */
		return; 	/* bye bye */
	}

/*
 *	no error so continue 
 */
	bp->b_cylin = bn/(NSECT*NTRAC) + hk_sizes[unit&07].cyloff;
	dp = &hkutab[dn];	/* get a pointer to the drives table */
	pri = spl5();
	disksort(dp, bp);	/* sort the drives queue in ascending
				   positional order and load this
				   buffer on the drives queue	*/

/*	If drive is not active and no I/O pending
*	then start it up.
*/
	if(dp->b_active == 0 && hktab.b_active == 0)
	{
		hkustart(dn);	 	/* start a drive seeking if needed */
		hkstart();		/* start I/O	*/
	}
	splx(pri);
}

static
hkustart(unit)
int unit;
{
	register struct buf *bp, *dp;
	register struct device *hkaddr;
	daddr_t bn;
	int sn, tn, cn, dn;

	hkaddr = io_csr[HK_RMAJ];
	dn = unit;			/* get drive number	*/
	hk_ios[dn].dk_busy = 0;		/* device not busy */
	el_bdact &= ~(1 << HK_BMAJ);
	dp = &hkutab[dn];		/* grab pointer to unit table	*/
	if(( bp = dp->b_actf) == NULL)	/* nothing pending here so return */
		return;
	hkmnt(dn);		/* make sure we know drive type */
	if(hk_dt[dn] < 0 || (hkaddr->hkds & DRDY)== 0)
	{	/* error from mount. drive not available ? */
		fmtbde(bp,&hk_ebuf,hkaddr,NHKREG,HKDBOFF);
		bp->b_flags |= B_ERROR; /* set error */
		bp->b_error = ENXIO;
		hk_ebuf.hk_bdh.bd_errcnt = 0;
		if (!logerr(E_BD, &hk_ebuf, sizeof(hk_ebuf)))
			deverror(bp, hk_ebuf.hk_reg[4], hk_ebuf.hk_reg[6]);
		hk_r[dn].ccyl = 0;	/* reset these */
		dp->b_actf = bp->av_forw; /* get next buffer */
		iodone(bp);		/* return buffer */
		return; 		/* bye bye */
	} else if((hkaddr->hkds & VV) == 0) {
		hkcmd(dn, (PAKACK|GO));	       /* set volume valid */
		hk_r[dn].ccyl = 0;		/* set current cyl = 0 */
		hk_r[dn].bads &= ~BAD_LOA;	/* don't have bse info */
	}
	if((hk_r[dn].bads & BAD_LOA) == 0){
		/* Don't have bad sector information */
		bn = hk_dt[dn]?HK7BAD:HK6BAD;
		for(sn = 0; sn < 5; sn++){
			hkaddr->hkdc = bn/(NSECT*NTRAC);
			cn = bn%(NSECT*NTRAC);
			hkaddr->hkda = ((cn/NSECT)<<8)|cn%NSECT;
			hkaddr->hkcs2 = dn;
#ifdef HKBAD
			hkaddr->hkwc = -((sizeof(struct hkbad)- 44)>>1);
#else
			hkaddr->hkwc = -(sizeof(struct hkbad)>>1);
#endif
			if(ubmaps)
			   hkaddr->hkba = (char*)&hk_bads[dn] - (char *)&cfree;
			else
			    hkaddr->hkba = &hk_bads[dn];
			hkaddr->hkcs1 = (hk_dt[dn]|RCOM|GO);
			while((hkaddr->hkcs1 & CRDY) == 0);
			if(hkaddr->hkcs1 & CERR){
				hkcmd(dn, (DCLR|GO));
				bn += 2;
			}
			else{
				hk_r[dn].bads |= BAD_LOA;
				break;
			}
		}
	}
	if(dp->b_active & HKSEEK || hktab.b_actf == NULL)
		goto done;
	bn = bp->b_blkno;		/* get block number     */
	cn = bp->b_cylin;		/* get cylinder number	*/
	sn = bn%(NSECT*NTRAC);		/* calculate the */
	tn = sn/NSECT;			/* track number */
	sn = sn%NSECT;			/* get the sector number */

	if(hk_r[dn].ccyl != cn && hktab.b_actf != NULL)
	{		/* not on cylinder so seek */

		dp->b_active = HKSEEK|HKBUSY;	/* say drive is seeking */
		hkaddr->hkdc = cn;		/* desired cylinder */
		hkaddr->hkda = (tn <<8)| sn;	/* so no illegal addr */
		hkaddr->hkcs2 = dn;		/* select drive */
		hk_ios[dn].dk_busy++;		/* drive is active (seeking) */
		el_bdact |= (1 << HK_BMAJ);
		hkaddr->hkcs1 = (hk_dt[dn]|IEI|SEEK|GO);	/* do it */
		return;
	}
done:
	hk_r[dn].ccyl = bp->b_cylin;	/* save current cylinder number */
	dp->b_active = HKBUSY;		/* say drive is active	*/
	dp->b_forw = NULL;		/* clear this forward link */
	if(hktab.b_actf == NULL)	/* if I/O table is 0 then */
		hktab.b_actf = dp;    /* load to I/O table (first in queue)*/
	else
		hktab.b_actl->b_forw = dp;	/* link to end of queue */
	hktab.b_actl = dp;			/* new end of queue */
}

static
hkstart()
{
	register struct buf *bp, *dp;
	register struct device *hkaddr;
	int unit;
	int com,cn,tn,sn,dn;
	daddr_t bn;

	hkaddr = io_csr[HK_RMAJ];
loop:
	if ((dp = hktab.b_actf) == NULL)	/* get a Unit table ptr */
	{
		hkaddr->hkcs1 = IEI;		/* nothing queued so make */
		return; 			/* sure IEI is set and bye */
	}
	if(dp->b_active & HKSEEK)
	{
		hkaddr->hkcs1 = IEI;
		return; 			/* drive is seeking. later */
	}
	if ((bp = dp->b_actf) == NULL)
	{					/* no buffer for this drive */
		hktab.b_actf = dp->b_forw;	/* get next drive from queue*/
		goto loop;			/* look some more */
	}
	hktab.b_active |= HKBUSY;	/* say that we're active */
	unit = minor(bp->b_dev)&077;	/* get the unit and file sys number */
	dn = unit >> 3;			/* strip file sys #, get drive # */

#if HKEOFF
		/* try offset recovery. Offset only good on read */
	if((hktab.b_active & HKOFFS) == 0
		&& hktab.b_errcnt >= 16
		&& bp->b_flags & B_READ)
	{
		hktab.b_active = HKOFFS;
		hkcmd(dn, GO);
		hkaddr->hkas.w = hk_offset[hktab.b_errcnt &017];
		hkaddr->hkcs2 = dn;
		hkaddr->hkcs1 = (hk_dt[dn]|IEI|OFFSET|GO);
		return;
	}
	hktab.b_active &= ~HKOFFS;	/* clear offset flag */
#endif
	switch(hk_r[dn].bads&060){
		case BAD_VEC:
			bn = hk_res.r_vbn;
			hkaddr->hkba = ((int)hk_res.r_vma & 0177777);
			hkaddr->hkwc = -(hk_res.r_vcc>>1);
#ifdef	QBUS
			hkaddr->dum = 022000|(hk_res.r_vxm&077);
#endif	QBUS
			com = (hk_res.r_vxm&03)<<8;
			break;
		case BAD_CON:
			hk_r[dn].bads &= ~BAD_CON;
			bn = hk_res.r_bn;
			hkaddr->hkba = ((int)hk_res.r_ma & 0177777);
			hkaddr->hkwc = -(hk_res.r_cc>>1);
#ifdef	QBUS
			hkaddr->dum = 022000|(hk_res.r_xm&077);
#endif	QBUS
			com = (hk_res.r_xm&03)<<8;
			break;
		default:
			bn = bp->b_blkno;	/* get the block number */
			hkaddr->hkba = bp->b_un.b_addr; /* get memory address */
			hkaddr->hkwc = -(bp->b_bcount>>1);
#ifdef	QBUS
			hkaddr->dum = 022000|(bp->b_xmem&077);
#endif	QBUS
			com = (bp->b_xmem&3)<<8;
			break;
	}
	cn =(bn/(NSECT*NTRAC));
	if((hk_r[dn].bads&BAD_VEC) == 0)
		cn += hk_sizes[unit&07].cyloff;/* calc cylinder # */
	sn = bn%(NSECT*NTRAC);
	tn = sn/NSECT;			/* calculate track number */
	sn = sn%NSECT;			/* get the sector number */
	com |= (IEI|GO); /* IEI, and go bits */
	if(bp->b_flags & B_READ)
		com |= RCOM;		/* Read function */
	else
		com |= WCOM;		/* write function */
	com |= hk_dt[dn];	 	/* set drive type */
	hkaddr->hkdc = cn;		/* load the desired cylinder */
	hkaddr->hkda = (tn<<8)|sn;	/* and the track , sector */
	hkaddr->hkcs2 = dn;		/* select drive number */
	hkaddr->hkcs1 = com;		/* and load it all into hkcs1 */
	hk_ios[dn].dk_busy++;		/* Instrumentation - disk active, */
	hk_ios[dn].dk_numb++;		/* count number of transfers, */
	unit = bp->b_bcount >> 6;	/* transfer size in 32 word chunks */
	hk_ios[dn].dk_wds += unit;	/* count total words transferred */
	el_bdact |= (1 << HK_BMAJ);	/* error log block device activity */
}

hkintr()
{
	register struct buf *bp, *dp;
	register struct device *hkaddr;
	int ctr;
	int seg, uba, dn;
	int tpuis[2], tpbad[2], tppos, tpofst, bsp, pri;
	long bn;

	hkaddr = io_csr[HK_RMAJ];
	if((hkaddr->hkcs1 & CRDY) == 0) /* controller not ready. I/O active */
		return; 		/* wait for next interrupt */
	if(!hktab.b_active && (hkaddr->hkas.c[1] == 0)) {
		logsi(hkaddr);	/* log stray interrupt */
		return;
	}

	if(hktab.b_active)
	{
		dp = hktab.b_actf;	/* Doing I/O. get drive pointer */
		bp = dp->b_actf;	/* get buffer pointer */
		dn = (bp->b_dev >> 3) & 7;	/* get drive number */
		if(hk_r[dn].recal & HKRRECAL)
		{
			hkcmd(dn, GO);
			if(hkaddr->hkds & DRDY == 0)
				if(hkaddr->hkcs1 & CERR)
					hkcmd(dn, (IEI|DCLR|GO));
				else
					hkaddr->hkcs1 = IEI;
			else
			{
				hk_r[dn].recal = HKRECAL;
				hkcmd(dn, (IEI|RECAL|GO));
			}
			return;
		}
		if(hk_r[dn].recal & HKRECAL || hktab.b_active & HKOFFS)
		{	/* recal in progress or offset active. see if done */
			hkcmd(dn, GO);   /* select the drive */
			if((hkaddr->hkds & DRDY)==0)
			{
				if(hkaddr->hkcs1 & CERR)
					hkcmd(dn, (IEI|DCLR|GO));
				else
					hkaddr->hkcs1 = IEI;
				return; 	/* recal still in prog */
			}
			hk_r[dn].recal = 0;	/* reset flag and */
			hktab.b_active &= ~HKBUSY;  /* retry I/O function */
			hkcmd(dn, GO);   /* select the drive */
		}
		if(hkaddr->hkcs1 & CERR)
		{
			if(hktab.b_errcnt == 0)
				fmtbde(bp,&hk_ebuf,hkaddr,NHKREG,HKDBOFF);

			/* if exceed retry count or Non-existent mem or */
			/* drive write lock error or cylinder overflow	*/
			/* or format error */
			/* or error during offset */
			/* then FATAL error */

			if(hktab.b_errcnt++ > NUMTRY
			   || hkaddr->hkcs2 & NED
			   || hkaddr->hkcs2 & NEM
			   || hkaddr->hkerr & (DWLE|COE|FMTE)
			   || hktab.b_active & HKOFFS)
			{
				if(hktab.b_errcnt <= NUMTRY)
					hktab.b_errcnt = 0;	/* no retries */
				hktab.b_active |= HKBUSY; /* make sure set */
				bp->b_flags |= B_ERROR; /* Fatal error */
			}
			else if(hkaddr->hkds & DROT
				|| hkaddr->hkerr & (OPI|SKI|DUNS))
			{				/* needs a recal   */
				hkcmd(dn, (DCLR|GO));   /* clr drive */
				if(hkaddr->hkds & DRDY == 0)
				{
					hk_r[dn].recal = HKRRECAL;
					hkaddr->hkcs1 = IEI;
				}
				else
				{
					hk_r[dn].recal = HKRECAL;
					hkcmd(dn, (IEI|RECAL|GO));
				}
				return; 		/* later */
			}

			else if((hkaddr->hkerr & (DCK|ECH))== DCK)
			{
				/* ECC error . recoverable */
			    fixbad(&hk_res, bp, hkaddr->hkwc, 0);
			    pri = spl7();	  /* need lots of priority */
			    tpuis[0] = UISA->r[0];  /* save User I PAR */
			    tpuis[1] = UISD->r[0];  /* save User I PDR */
#ifdef	QBUS
			    uba = (int)(hk_res.r_xm&077)<<10; /* exmem */
#else	QBUS
			    uba = (int)(hk_res.r_xm&03)<<10; /* exmem */
#endif	QBUS
			    uba += (((int)(hk_res.r_ma)>>1)&~0100000)>>5;
			    uba -= 8;
			    if(bp->b_flags&B_MAP)
			    {
				ctr = (uba >> 7) & 037;
				seg = (UBMAP + ctr)->ub_hi << 10;
				seg |= ((UBMAP + ctr)->ub_lo >> 6) & 01777;
				UISA->r[0] = seg + (uba & 0177);
/* ECC FIX */			bsp = (UBMAP + ctr)->ub_lo & 077;
			    } else
				UISA->r[0] = uba;
			    UISD->r[0] = 077406;	  /* Set User I PDR */
			    tppos = (hkaddr->hkec1-1)/16;   /* word addr */
			    tpofst = (hkaddr->hkec1-1)%16;  /* bit in word */
/* ECC FIX */		    if((bp->b_flags&B_MAP) == 0)
			        bsp = (int)hk_res.r_ma & 077;	    /* 64 byte range*/
			    bsp += (tppos*2);
			    tpbad[0] = fuiword(bsp);	/* first bad word */
			    tpbad[1] = fuiword(bsp+2);	/* second bad word */
			    tpbad[0] ^= (hkaddr->hkec2<<tpofst); /* xor first word */

			    if(tpofst > 5)
			    {			/* must fix second word */
				    ctr = (hkaddr->hkec2 >> 1) & ~0100000;
				    if(tpofst < 15)
					    ctr = ctr >> (15 - tpofst);
				    tpbad[1] =^ ctr;	/* xor second word */
			    }
			    suiword(bsp,tpbad[0]);	/* set first word */
			    suiword((bsp+2),tpbad[1]);	/* set second word */
			    UISD->r[0] = tpuis[1];  /* restore User PDR */
			    UISA->r[0] = tpuis[0];  /* restore User PAR */
			    splx(pri);			/* back to old pri */

			    /* now see if transfer finished. */
			    /* If not then reload xfer making sure */
			    /* the CCLR bit does not get set too */

			    if(hk_res.r_cc > 0)
			    {
				    hkaddr->hkcs1 = CCLR;
				    while((hkaddr->hkcs1 & CRDY) == 0);
				    hk_r[dn].bads |= BAD_CON;
				    hkstart();
				    return;
			    }
			}
	
			else if(hkaddr->hkerr & ECH)
			{				/* soft error. retry */
				hktab.b_active&=~HKBUSY;/* retry I/O funct */
				hkcmd(dn , (DCLR|GO)); /* clear drive's attn */
			}

			else if(hkaddr->hkerr & BSE && hk_r[dn].bads & BAD_LOA)
			{	/* BAD SECTOR Error */
				fixbad(&hk_res, bp, hkaddr->hkwc, 1);
				if((ctr=isbad(&hk_bads[dn],hkaddr->hkdc,hkaddr->hkda,NSECT))>=0)
				{
#ifdef HKBAD
					hk_bads[dn].kbt_vec[ctr]++;
#endif
				    bn = ((hk_dt[dn]?HK7BAD:HK6BAD)-1)-ctr;
				    hkcmd(dn, (DCLR|GO));
				    hk_r[dn].bads |= BAD_VEC;
				    hk_res.r_vbn = bn;
				    hktab.b_errcnt--;
				    hkstart(bp);
				    return;
				}
				else
					bp->b_flags |= B_ERROR;
			}
			else if(hkaddr->hkcs2 & (UPE|DLT)
				|| hkaddr->hkerr & (HRVC|DTE)
				|| hkcomer(dn))
			{				/* soft error. retry */
				hkcmd(dn , (DCLR|GO)); /* clear drive's attn */
				hktab.b_active&=~HKBUSY;  /* retry I/O funct */
			}

			else
			{
				/* must be fatal error */
				hktab.b_active |= HKBUSY;
				bp->b_flags |= B_ERROR;
			}
		}
		if(hktab.b_active & HKBUSY) /* need to return a buffer */
		{
			if(hktab.b_errcnt || bp->b_flags & B_ERROR)
			{
			  if(hk_ebuf.hk_reg[6] & DWLE)
			    printf("HK unit %d Write Locked\n", dn);
			  else {
			    hk_ebuf.hk_bdh.bd_errcnt = hktab.b_errcnt;
			    if (!logerr(E_BD, &hk_ebuf, sizeof(hk_ebuf)))
			     deverror(bp, hk_ebuf.hk_reg[4], hk_ebuf.hk_reg[6]);
			  }
			}
			if(hk_r[dn].bads&BAD_VEC&&(bp->b_flags&B_ERROR)==0){
				hk_r[dn].bads &= ~BAD_VEC;
				if(hk_res.r_cc > 0){
					hk_r[dn].bads |= BAD_CON;
					hkstart();
					return;
				}
			}
			hk_r[dn].bads &= ~(BAD_VEC|BAD_CON);
			hktab.b_active = 0;		/* no I/O active */
			dp->b_active = 0;		/* no drive active */
			hktab.b_errcnt = 0;		/* clr I/O error count */
			dp->b_errcnt = 0;		/* clr drive error count */
			hktab.b_actf = dp->b_forw;	/* get next drive link */
			dp->b_actf = bp->av_forw;	/* get next I/O link */
			if(bp->b_flags & B_ERROR)
			    bp->b_resid = bp->b_bcount; /* nothing xfered */
			else
				bp->b_resid = 0;	/* xfer complete */
			iodone(bp);			/* return buffer */
#if HKEOFF
			if(hktab.b_errcnt >16 && bp->b_flags & B_READ)
			{
				hkcmd(dn, (DCLR|GO));	/* make sure attn is off */
				hkcmd(dn, OFFSET|GO);	/* return to zero */
			}
#endif
			hk_ios[dn].dk_busy = 0;	/* drive no longer active */
			el_bdact &= ~(1 << HK_BMAJ);	/* device activity */
		}
		hkcmd(dn, (DCLR|GO));	/* make sure attn is off */
#if HKEOFF
		if(hktab.b_active & HKOFFS)
		{
			hkstart();
			return;
		}
#endif
	}
	hkattn();	/* go check attention lines */
}			/* and start any pending I/O */
	
static
hkattn()
{
	register struct buf *bp, *dp;
	register struct device *hkaddr;
	int dn, as;

	hkaddr = io_csr[HK_RMAJ];
	as = hkaddr->hkas.c[1]; 	/* get attn summary reg */
	for( dn = 0; dn < nhk; dn++)
	{
		if((as & (1 << dn)) == 0)
			continue;	/* find attn bit(s) */
		dp = &hkutab[dn];	/* get pointer to utable */
		bp = dp->b_actf;	/* get active buffer pointer */
		if(hk_r[dn].recal & HKRRECAL)
		{
			hkcmd(dn, (DCLR|GO));
			if(hkaddr->hkds & DRDY)
			{
				hk_r[dn].recal = HKRECAL;
				hkcmd(dn, (IEI|RECAL|GO));
			}
			continue;
		}
		if(hk_r[dn].recal & HKRECAL)	/* recal in progress */
		{
			hkcmd(dn, GO);	/* select drive */
			if((hkaddr->hkds & DRDY) == 0)
				continue;
			hk_r[dn].recal = 0;	/* reset recal flag */
			dp->b_active = 0;	/* try again */
			continue;
		}
		if(hk_dt[dn] < 0)
		{	/* attn from NED, try to mount it */
			hkcmd(dn, DCLR|GO);
			hkmnt(dn);
			if(hk_dt[dn] >= 0){
				hk_r[dn].bads &= ~BAD_LOA;
				hkcmd(dn, DCLR|GO);
			}
			continue;
		}
		hkcmd(dn , GO);	   /* select drive */
		if(hkaddr->hkds & PIP)
			continue;
		if(hkaddr->hkcs1 & CERR)
		{
			if(dp->b_errcnt == 0){
				if(bp == 0)
					dp->b_dev = ((HK_BMAJ << 8)| (dn << 3));
				fmtbde(bp?bp:dp,&hk_ebuf,hkaddr,NHKREG,HKDBOFF);
			}
			if(dp->b_errcnt++ > NUMTRY
			     || (hkcomer(dn)) == 0)
			{
				if(!logerr(E_BD,&hk_ebuf,sizeof(hk_ebuf))){
					printf("\nHK attention error\n");
					printf("cs2   ds    err\n");
					printf("%o  %o  %o\n", hkaddr->hkcs2
						, hkaddr->hkds, hkaddr->hkerr);
				}
				if(dp->b_active){
					dp->b_actf = bp->av_forw;/* next link*/
					bp->b_flags |= B_ERROR; /* say bad */
					dp->b_errcnt = 0;/* not recoverable */
					iodone(bp);	      /* return buf */
				}
			}
			if(hkaddr->hkds & DROT
				|| hkaddr->hkerr & (DUNS|OPI|SKI))
			{
				hkcmd(dn, (DCLR|GO));	/* drive clear */
				if(hkaddr->hkds & DRDY)
				{
					hk_r[dn].recal = HKRECAL;
					hkcmd(dn, (IEI|RECAL|GO));
				}
				else
					hk_r[dn].recal = HKRRECAL;
				continue;
			}
			dp->b_active = 0;	/* drive not active */
			hk_r[dn].ccyl = 0;
		}

		if((hkaddr->hkds & DRDY) == 0)
		{	/* drive dropped off line */
			hk_r[dn].bads &= ~BAD_LOA;
			if(dp->b_active)
			{	/* something active */
				dp->b_active = 0;	/* say no more */
				bp->b_flags |= B_ERROR; /* error */
				dp->b_actf = bp->av_forw; /* get next buffer */
				iodone(bp);		/* return error */
			}
		}
		dp->b_active &= ~HKBUSY;	   /* make sure clear */
		hkcmd(dn, (DCLR|GO));	   /* clear drive attn */
	}
	for(dn = 0; dn < nhk; dn++)
	{
		dp = &hkutab[dn];	/* get utable pointer */
		if((dp->b_active & HKBUSY) == 0 && dp->b_actf)
			hkustart(dn);	/* if drive not busy and request
					   pending then start the drive */
	}
	if((hktab.b_active & HKBUSY) == 0)
		hkstart();		/* If no I/O active then start it */
}

static
hkcmd(dn, comm)
{
	register struct device *hkaddr;
	int pri;

	hkaddr = io_csr[HK_RMAJ];
	while((hkaddr->hkcs1 & CRDY) == 0);	/* wait for ready */
	pri = spl5();				/* no interrupts */
	hkaddr->hkcs1 |= CCLR;			/* clear controller */
	while((hkaddr->hkcs1 & CRDY) == 0);	/* wait for contr ready */
	hkaddr->hkcs2 = dn;			/* load drive # */
	hkaddr->hkcs1 = (hk_dt[dn]|comm); /* load command */
	if((comm & IEI) == 0)
		while((hkaddr->hkcs1 & CRDY) == 0); /* need to wait here */
	splx(pri);
}

static
hkcomer(dn)
int dn;
{
	/** some errors are common to seek as well as i/o **/
	/** this routine checks for them.		  **/

	register struct buf *dp;
	register struct device *hkaddr;
	int soft, cnt;

	/* Check for Controller Type Errors first */

	hkaddr = io_csr[HK_RMAJ];
	if(hkaddr->hkcs1 & (CTO|DTCPAR) 	/* Controller timeout */
		|| hkaddr->hkcs2 & (UFE|PGE)	/* Drive to Cont. Parity */
		|| hkaddr->hkerr & CDPAR)	/* Cont. to Drive Parity */
						/* Unit Field Error */
						/* Programming Error */
		return(1);			/* are all soft errors */

	else if(hkaddr->hkcs2 & NED)		/* Non-Exist Drive */
		return(0);			/* return non-recoverable */

	else if(hkaddr->hkcs2 & MDS)		/* Multiple Drive Select */
	{					/* Have to clear the entire */
		hkaddr->hkcs2 = SCLR;		/* disk system to recover */
		while((hkaddr->hkcs1 & CRDY) == 0)
			;			/* wait for controller rdy */
		hktab.b_active = 0;		/* no I/O active */
		for(cnt = 0; cnt < nhk; cnt++)
		{
			dp = &hkutab[cnt];	/* no drive active */
			dp->b_active = 0;
		}
		return(0);			/* return Hard Error */
	}

	/* Now we can check for drive errors */

	else
		hkcmd(dn, GO);		   /* select drive */

	if(hkaddr->hkds & (ACLO|SPLS)		/* ACLO or Speed Loss */
	  || hkaddr->hkerr & (ILF|NXF|IDAE))	/* Illegal Function */
						/* Nonexecutable Function */
							
		return(0);
	else
		return(1);		/* anything else is soft error?? */
}

hkread(dev)
dev_t dev;
{
	physio(hkstrategy, &rhkbuf, dev, B_READ);
}

hkwrite(dev)
dev_t dev;
{
	physio(hkstrategy, &rhkbuf, dev, B_WRITE);
}

hkclose(dev,flag)
dev_t dev;
{

	bflclose(dev);
}
