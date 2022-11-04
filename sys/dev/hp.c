
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)hp.c	3.0	4/21/86
 */
/*
 * ULTRIX-11 General Massbus Disk Driver (hp.c).
 * Supports up to four RH11/RH70 massbus controllers,
 * (hp, hm, hl, hj). Each controller may have up to 8
 * drives of type: ML11, RP04/5/6, RM02/3/5.
 *
 * This driver contains what on the surface may appear
 * to be strangeness, HOWEVER it is really some very
 * carefully designed and tested code, which allows
 * the driver to handle dual ported drives and to recognize
 * when drives are taken off/on line via removing/replacing the LAP
 * (logical assignment plug). It also handles the case where the LAPs
 * are exchanged between unlike drive types, for example RM03 & RP06.
 * The LAP feature will not function properly for the ML11,
 * which has no LAP anyway !
 * The ML11 cannot be dual ported, GOOD !
 * WARNING !, testing of the dual port code was limited to
 * allowing the driver to work with dual ported drives having
 * the port switch in any of the three positions, A, B, or A/B.
 * Actual dynamic (A/B) operation with two systems will NOT work,
 * because the drive is treated as nonexistent if it is seized
 * by the controller on the other port.
 */

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
#include <sys/hpbad.h>
#include <sys/hp_info.h>
#include <sys/uba.h>

char	hp_index[];	/* non-semetrical array index (see conf/rh.c) */
int	io_csr[];	/* (c.c) I/O page CSR address for each cntlr */
			/*       Index is HP_RMAJ + ctrl */
char	io_bae[];	/* (c.c) BAE register offset for each cntlr,	*/
			/*       or 0 if cntlr is an RH11		*/
			/*       Index is HP_BMAJ + ctrl		*/
char	nhp[];		/* (c.c) Number of units configured for each cntlr */
char	*hp_dct[] = {"HP", "HM", "HJ"};	/* cntlr type for error msg */

/*
 * Instrumentation (iostat) structures
 */
struct	ios	hp_ios[];
#define	DK_N	0

/*
 * Disk overlapped seek definitions
 */
#define	SDIST	2
#define	RDIST	6

/*
 * HP disk information structure
 *
 * WARNING!, the order and values of things in this
 * structure are hardwired into the HPX disk exerciser.
 * DO NOT change this table without checking HPX.
 * WARNING!, same deal with /usr/sys/sas/setup.c!
 */
#define	RP456	0
#define	RM23	1
#define	RM5	2
#define	ML	3

struct	hpdi {
	int	nsect;	/* sectors per track */
	int	ntrac;	/* tracks per cylinder */
	int	nbpc;	/* blocks per cylinder */
	int	sti;	/* sizes table index */
} hpdi[] = {
	22,	19,	22*19,	RP456*8,	/* RP04/5/6 */
	32,	5,	32*5,	RM23*8,		/* RM02/3 */
	32,	19,	32*19,	RM5*8,		/* RM05 */
	1,	1,	1,	ML*8,		/* ML11 */
};

/*
 * The size structures determine the starting cylinder
 * number and length in blocks of the disk partitions.
 * For the RM02/3/5 and RP04/5/6 disks, each entry describes
 * a partition, for the ML11 each entry contains the size in
 * blocks of that unit, the ML11 is not partitioned.
 * The size of the ML11 unit is determined by reading
 * the ML maintenance register the first time the drive
 * is accessed.
 * The last two tracks of each are reserved for the bad
 * block (sector) file and can only be accessed by the driver.
 */

/*
 * Sizes table now in /usr/sys/conf/dksizes.c
 */
extern	struct {
	daddr_t	nblocks;
	int	cyloff;
} hp_sizes[];

/*
 * Block device error log buffer, holds one
 * error log record until error retry sequence
 * has been completed.
 * One for each RH11/RH70 controller.
 */
struct	hp_ebuf	hp_ebuf[];

#define	P400	020
#define	M400	0220
#define	P800	040
#define	M800	0240
#define	P1200	060
#define	M1200	0260
int	hp_offset[16] =
{
	P400, M400, P400, M400,
	P800, M800, P800, M800,
	P1200, M1200, P1200, M1200,
	0, 0, 0, 0,
};

struct	buf	hptab[];	/* Head of I/O queue for each cntlr */
struct	buf	rhpbuf[];	/* RAW I/O buffer header for each cntlr */
struct	buf	hputab[];	/* Drive queue (overlap seek) one per unit */

/*
 * Drive types, required by HPX
 * 0	= nonexistent drive
 * otherwise equals the drive type.
 */
char	hp_dt[];	/* see conf/rh.c */

/*
 * Structures for bad block revectoring
 */
struct	hpbad hp_bads[];	/* Buffer for bad block file, one per unit */
struct	dkres hp_res[];		/* Bads revector and continue structure, */
				/* one per controller. */
struct	hp_badr	hp_r[];		/* Flags & base block, one per unit (hp_info.h) */

char	hp_openf[];	/* HP open flag, one per cntlr */

#define	GO	01
#define	PRESET	020
#define	RTC	016
#define	OFFSET	014
#define	SEARCH	030
#define	RECAL	06
#define DCLR	010
#define	DPRLS	012
#define	CCLR	040
#define	WCOM	060
#define	RCOM	070

#define	IE	0100
#define	NED	010000
#define	PIP	020000
#define	DRY	0200
#define	ERR	040000
#define	TRE	040000
#define	DCK	0100000
#define	WLE	04000
#define	ECH	0100
#define VV	0100
#define	DPR	0400
#define	MOL	010000
#define FMT22	010000

#define	b_cylin	b_resid

/*
 * On the first call to hpopen only,
 * determine the existence and type of all possible drives.
 * A dual ported drive that is locked on the opposite port
 * will be treated and nonexistent until it is unlocked.
 * This code assumes that the interrupt enable in HPCS1 is off.
 */

hpopen(dev)
{
	register struct hp_regs *hpaddr;
	register int ctrl, dn;
	int fs, ind, pri;

	ctrl = hpctrl(dev);
	if(hp_openf[ctrl])
		return;
	hp_openf[ctrl]++;
	hpaddr = io_csr[HP_RMAJ+ctrl];	/* Get RH11/RH70 CSR address (HPCS1) */
	for(dn=0; dn<nhp[ctrl]; dn++) {		/* look at all drives */
		ind = hp_index[ctrl] + dn;
		hpaddr->hpcs2.w = dn;	/* select drive */
		hpaddr->hpcs1.w = GO;	/* NOP, attempt seize dual port drive */
		hp_dt[ind] = hpaddr->hpdt & 0377;  /* save drive type, 0 = NED */
		if(hp_dt[ind] == 0)	/* clear the world if NED */
			hpaddr->hpcs2.w = CCLR;
/*
 * Find the size of the ML11 unit by reading the
 * number of array modules from the ML maintenance register.
 * There are 512 blocks per array.
 */
		if(hp_dt[ind] == ML11) {
			hp_sizes[dn+(ML*8)].nblocks = (hpaddr->hpmr>>2)&037000;
/*
 * Check the ML11 transfer rate.
 * 2mb is too fast for any pdp11
 * 1mb allowed on pdp11/70 only
 * .5mb & .25mb ok on any processor.
 */
			fs = hpaddr->hpmr & 01400;
			if((fs == 0) || ((fs == 0400) && (cputype != 70))) {
				printf("\n%s unit %d ML11 too fast for CPU\n",
					hp_dct[ctrl], dn);
				hp_dt[ind] = 0;
			} else
				hp_ios[ind].dk_tr = ((fs >> 8) & 3) + 4;
		}
	}
/*
 * Initialize disk I/O instrumentation pointers
 */
	pri = spl6();
	fs = DK_N + ctrl;
	dk_iop[fs] = &hp_ios[hp_index[ctrl]];
	dk_nd[fs] = nhp[ctrl];
	splx(pri);
}

hpstrategy(bp)
register struct buf *bp;
{
	register struct buf *dp;
	register int	ctrl;
	long sz, bn;
	int dn, dip, fs, pri, unit, ind;

	ctrl = hpctrl(bp->b_dev);
	if(!io_bae[HP_BMAJ+ctrl])
		mapalloc(bp);
	unit = minor(bp->b_dev) & 077;
	dn = (unit >> 3) & 07;	/* get drive number */
	ind = hp_index[ctrl] + dn;
	if(hp_dt[ind] == 0)
		goto bad;	/* NED - can't even touch ! */
/*
 * Set the drive information pointer according to drive type.
 * 0 = RP04/5/6, 1 = RM02/3, 2 = RM05, 3 = ML11
 */
	dip = hp_dipset(ind);
/*
 * Set disk xfer rates for iostat
 */
	switch(dip) {
	case RP456:
	case RM23:
		hp_ios[ind].dk_tr = 1;
		if(hp_dt[ind] == RM03)
			hp_ios[ind].dk_tr++;
		break;
	case RM5:
		hp_ios[ind].dk_tr = 2;
	case ML:	/* ML rate set in hpopen() */
		break;
	default:
		goto bad;
	}
	sz = bp->b_bcount;
	sz = (sz+511) >> 9;
	fs = unit & 07;		/* For RP & RM, fs is partition */
	if(dip == ML) {		/* For ML, fs is unit number */
		fs = dn;
		if(unit & 7)
			goto bad;	/* ML11 is not partitioned */
	}
	if(dn >= nhp[ctrl] ||
	    bp->b_blkno < 0 ||
	    (bn = bp->b_blkno)+sz > hp_sizes[fs+hpdi[dip].sti].nblocks) {
	bad:
		bp->b_error = ENXIO;
		bp->b_flags |= B_ERROR;
		iodone(bp);
		return;
	}
	if(dip == ML)
	    bp->b_cylin = 0;	/* fake out dsort for ML11 */
	else
	    bp->b_cylin = bn/hpdi[dip].nbpc + hp_sizes[fs+hpdi[dip].sti].cyloff;
	dp = &hputab[ind];
	pri = spl5();
	disksort(dp, bp);
	if (dp->b_active == 0) {
		hpustart(ctrl, dn);
		if(hptab[ctrl].b_active == 0)
			hpstart(ctrl);
	}
	splx(pri);
}

static
hpustart(ctrl, unit)
int ctrl;
int unit;
{
	register struct buf *bp, *dp;
	register struct hp_regs *hpaddr;
	daddr_t bn;
	int sn, cn, csn, dip, ind;

	ind = hp_index[ctrl] + unit;
	hpaddr = io_csr[HP_RMAJ+ctrl];
/*
 * Check for nonexistent drive !
 * This could cause a NED error interrupt,
 * which must be handled by hpintr().
 * The drive type reads as zeros on NEDs.
 */
	hpaddr->hpcs2.w = unit;
	hpaddr->hpcs1.c[0] = IE|GO;	/* NOP attempt seize dual port drive */
	hp_dt[ind] = hpaddr->hpdt & 0377;
	if(hp_dt[ind] == 0)
		goto done;	/* NED */
	hpaddr->hpcs1.c[0] = IE;
	hpaddr->hpas = 1<<unit;
	if(unit >= nhp[ctrl])
		return;
	dip = hp_dipset(ind);
	hp_ios[ind].dk_busy = 0;	/* device not busy */
	el_bdact |= (1 << (HP_BMAJ+ctrl));	/* XXX */
	dp = &hputab[ind];
	if((bp=dp->b_actf) == NULL)
		return;
	hpaddr->hpcs2.w = unit;	/* NED interrupt could change CS2 */
	if((hpaddr->hpds & VV) == 0) {
		hpaddr->hpcs1.c[0] = IE|PRESET|GO;
		hp_r[ind].bads &= ~BAD_LOA;
	}
	if(dip != ML)
		hpaddr->hpof = FMT22;
	if(dp->b_active)
		goto done;
	if(dip == ML) {
		sn = bp->b_blkno;
		dp->b_active++;
		goto mlsrch;
		}
	if ((hpaddr->hpds & (DPR|MOL)) != (DPR|MOL))
		goto done;
	dp->b_active++;
	bn = bp->b_blkno;
	cn = bp->b_cylin;
	sn = bn%hpdi[dip].nbpc;
	sn = (sn+hpdi[dip].nsect-SDIST)%hpdi[dip].nsect;
	if(hpaddr->hpcc != cn)
		goto search;
	csn = (hpaddr->hpla>>6) - sn + SDIST - 1;
	if(csn < 0)
		csn += hpdi[dip].nsect;
	if(csn > hpdi[dip].nsect-RDIST)
		goto done;
search:
	hpaddr->hpdc = cn;
mlsrch:
	hpaddr->hpda = sn;
	hp_ios[ind].dk_busy++;		/* device active */
	el_bdact |= (1 << (HP_BMAJ+ctrl));
	hpaddr->hpcs1.c[0] = IE|SEARCH|GO;
	return;
done:
	dp->b_forw = NULL;
	if(hptab[ctrl].b_actf == NULL)
		hptab[ctrl].b_actf = dp; else
		hptab[ctrl].b_actl->b_forw = dp;
	hptab[ctrl].b_actl = dp;
}

static
hpstart(ctrl)
{
	register struct buf *bp, *dp;
	register struct hp_regs *hpaddr;
	int unit, ind;
	daddr_t bn;
	int dn, sn, tn, cn, dt, pri;
	int dip, bae;

	hpaddr = io_csr[HP_RMAJ+ctrl];
	bae = io_bae[HP_BMAJ+ctrl];
loop:
	if ((dp = hptab[ctrl].b_actf) == NULL)
		return;
	if ((bp = dp->b_actf) == NULL) {
		hptab[ctrl].b_actf = dp->b_forw;
		goto loop;
	}
	hptab[ctrl].b_active++;
	unit = minor(bp->b_dev) & 077;
	dn = unit >> 3;
	bn = 0;
/*
 * Do not attempt to access a NED !
 */
	ind = hp_index[ctrl] + dn;
	if(hp_dt[ind] == 0)
		goto bad;	/* NED */
	if ((hpaddr->hpds & (DPR|MOL)) != (DPR|MOL)) {
	bad:
		hptab[ctrl].b_active = 0;
		bp->b_error = ENXIO;	/* NED */
		hptab[ctrl].b_errcnt = 0;
		dp->b_actf = bp->av_forw;
		bp->b_flags |= B_ERROR;
		iodone(bp);
		goto loop;
	}
	dip = hp_dipset(ind);
	if((hp_r[ind].bads & BAD_LOA) == 0 && dip != ML){
		/* don't have Bad Sector Info. Need to load it. */
		bn = hp_r[ind].vcyl;
		for(pri = 0; pri < 5; pri++){
			cn = bn/hpdi[dip].nbpc;
			sn = bn%hpdi[dip].nbpc;
			tn = sn/hpdi[dip].nsect;
			sn = sn%hpdi[dip].nsect;
			hpaddr->hpcs2.w = dn;
			hpaddr->hpdc = cn;
			hpaddr->hpda = (tn<<8)+ sn;
			if(ubmaps && !bae)
			    hpaddr->hpba = (char *)&hp_bads[ind]-(char *)&cfree;
			else
			    hpaddr->hpba = &hp_bads[ind];
			hpaddr->hpwc = -((sizeof(struct hpbad)-64)>>1);
			if(bae)
				hpaddr->hpbae = 0;
			hpaddr->hpcs1.w = RCOM|GO;
			while((hpaddr->hpcs1.w & DRY) == 0)
				;
			if(hpaddr->hpcs1.w & TRE){
				if((pri==0) && (hptab[ctrl].b_errcnt==0)) {
					if(!bae)
						cn = (NHPREG - 2);
					else
						cn = NHPREG;
					fmtbde(bp, &hp_ebuf[ctrl], hpaddr, cn, HPDBOFF);
					hp_ebuf[ctrl].hp_reg[4] &= ~7;  /* correct unit # */
					hp_ebuf[ctrl].hp_reg[4] |= dn;
				}
				bn += 2;
			}
			else{
				hp_r[ind].bads |= BAD_LOA;
				break;
			}
		}
	}
	if((dip != ML) &&
	   (hptab[ctrl].b_errcnt == 0) &&
	   ((hp_r[ind].bads & BAD_LOA) == 0)) {
		hp_ebuf[ctrl].hp_bdh.bd_errcnt = 0;
		if(!logerr(E_BD,&hp_ebuf[ctrl],sizeof(struct hp_ebuf)))
		deverror(bp,hp_ebuf[ctrl].hp_reg[4],hp_ebuf[ctrl].hp_reg[6]);
	}

	hpaddr->hpcs2.w = dn;
	switch(hp_r[ind].bads & 060){
		case BAD_VEC:
			bn = hp_res[ctrl].r_vbn;
			hpaddr->hpba = hp_res[ctrl].r_vma;
			if(bae)
				hpaddr->hpbae = hp_res[ctrl].r_vxm;
			hpaddr->hpwc = -(hp_res[ctrl].r_vcc>>1);
			pri = ((hp_res[ctrl].r_vxm&03)<<8) | IE | GO;
			break;
		case BAD_CON:
			hp_r[ind].bads &= ~BAD_CON;
			bn = hp_res[ctrl].r_bn;
			hpaddr->hpba = hp_res[ctrl].r_ma;
			if(bae)
				hpaddr->hpbae = hp_res[ctrl].r_xm;
			hpaddr->hpwc = -(hp_res[ctrl].r_cc>>1);
			pri = ((hp_res[ctrl].r_xm&03)<<8) | IE | GO;
			break;
		default:
			bn = bp->b_blkno;
			hpaddr->hpba = bp->b_un.b_addr;
			if(bae)
				hpaddr->hpbae = bp->b_xmem;
			hpaddr->hpwc = -(bp->b_bcount>>1);
			pri = ((bp->b_xmem&3) << 8) | IE | GO;
			break;
	}
	if(dip != ML) {
		cn = bn/hpdi[dip].nbpc;
		if((hp_r[ind].bads & BAD_VEC) == 0)
			cn += hp_sizes[(unit&7)+hpdi[dip].sti].cyloff;
		sn = bn%hpdi[dip].nbpc;
		tn = sn/hpdi[dip].nsect;
		sn = sn%hpdi[dip].nsect;
	}
	if((hptab[ctrl].b_errcnt >= 16) && (dip != ML)) {
		hpaddr->hpof = hp_offset[hptab[ctrl].b_errcnt & 017] | FMT22;
		hpaddr->hpcs1.w = OFFSET|GO;
		while(hpaddr->hpds & PIP)
			;
	}
	if(dip == ML)
		hpaddr->hpda = bn;
	else {
		hpaddr->hpdc = cn;
		hpaddr->hpda = (tn << 8) + sn;
	}
	if(bp->b_flags & B_READ)
		pri |= RCOM; else
		pri |= WCOM;
	hpaddr->hpcs1.w = pri;
	el_bdact |= (1 << (HP_BMAJ+ctrl));
	hp_ios[ind].dk_busy++;		/* Instrumentation - disk active, */
	hp_ios[ind].dk_numb++;		/* count number of transfers, */
	unit = bp->b_bcount >> 6;	/* transfer size in 32 word chunks, */
	hp_ios[ind].dk_wds += unit;	/* count total words transferred */
}

hpintr(dev)
{
	register struct buf *bp, *dp;
	register struct hp_regs *hpaddr;
	int unit, ctrl, ind;
	int seg, uba;
	int as;
	int tpuis[2], tpbad[2], tppos, tpofst;
	int bsp;
	int pri, ctr, dn;

	ctrl = dev & 3;
	ind = hp_index[ctrl];
	hpaddr = io_csr[HP_RMAJ+ctrl];
/*
 * Must handle the NED error interrupt which could
 * be generated by the NED check in hpustart above.
 */
	if(hptab[ctrl].b_active == 0) {
		ctr = hpaddr->hpcs2.w & 7;
		if(hpaddr->hpcs2.w & NED) {
			printf("\n%s unit %d nonexistent\n", hp_dct[ctrl], ctr);
/* XXX */		hpaddr->hpcs2.w = 0; /* assumes drive 0 exists !!! */
			hpaddr->hpcs1.w = TRE|IE;
			hp_dt[ind+ctr] = 0;		/* NED */
			return;
		}
		as = hpaddr->hpas & 0377;
		if(as == 0) {
			logsi(hpaddr);
			return;
		}
	} else
		as = hpaddr->hpas & 0377;
	if(hptab[ctrl].b_active) {
		dp = hptab[ctrl].b_actf;
		bp = dp->b_actf;
		unit = (bp->b_dev >> 3) & 7;
		dn = unit;
		if(hpaddr->hpcs2.w & NED) {
			ctr = hpaddr->hpcs2.w & 7;
			if(ctr != unit) {
				hp_dt[ind+ctr] = 0;	/* NED */
				dn = ctr;
			}
		}
		ind += unit;
		hpaddr->hpcs2.c[0] = unit;
		hp_ios[ind].dk_busy = 0;	/* device no longer active */
		if (hpaddr->hpcs1.w & TRE) {		/* error bit */
			while((hpaddr->hpds & DRY) == 0)
				;
			if(hptab[ctrl].b_errcnt == 0) {
				if(!io_bae[HP_BMAJ+ctrl])
					ctr = (NHPREG - 2);
				else
					ctr = NHPREG;
				fmtbde(bp, &hp_ebuf[ctrl], hpaddr, ctr, HPDBOFF);
				hp_ebuf[ctrl].hp_reg[4] &= ~7;  /* correct unit # */
				hp_ebuf[ctrl].hp_reg[4] |= dn;
				}
			if((hp_dt[ind] == ML11) && (hptab[ctrl].b_errcnt == 0))
				hptab[ctrl].b_errcnt = 18;
			if(++hptab[ctrl].b_errcnt > 28 || hpaddr->hper1&WLE) {
				if(hpaddr->hper1 & WLE)
					hptab[ctrl].b_errcnt = 0;
				bp->b_flags |= B_ERROR;
			} else
				hptab[ctrl].b_active = 0;
/* Start of ECC code, please excuse the improper indentation */
	if(((hpaddr->hper1&(DCK|ECH)) == DCK) && (hp_dt[ind] != ML11))
	{
		fixbad(&hp_res[ctrl], bp, hpaddr->hpwc, 0);
		pri = spl7();
		tpuis[0] = UISA->r[0];	/* save User I space PAR */
		tpuis[1] = UISD->r[0];	/* save User I space PDR */
		if(io_bae[HP_BMAJ+ctrl])
			uba = (hpaddr->hpbae << 10) & 0176000;
		else
			uba = (hpaddr->hpcs1.w & 01400) << 2;
		uba += ((((int)hpaddr->hpba >> 1) & ~0100000) >> 5);
		uba -= 8;
		if(bp->b_flags & B_MAP) {
			ctr = ((uba >> 7) & 037);
			seg = (UBMAP + ctr)->ub_hi << 10;
			seg |= ((UBMAP + ctr)->ub_lo >> 6) & 01777;
			UISA->r[0] = seg + (uba & 0177);
/* ECC FIX */		bsp = (UBMAP + ctr)->ub_lo & 077;
		} else
			UISA->r[0] = uba;
		UISD->r[0] = 077406;	/* Set User I space PDR */
		tppos = (hpaddr->hpec1 - 1)/16;	/* word addr of error burst */
		tpofst = (hpaddr->hpec1 - 1)%16;	/* bit in word */
/* ECC FIX */	if((bp->b_flags&B_MAP) == 0)
			bsp = ((int)hpaddr->hpba & 077);
		bsp += (tppos*2);
		tpbad[0] = fuiword(bsp);
		tpbad[1] = fuiword(bsp+2);
		tpbad[0] ^= (hpaddr->hpec2 << tpofst); /* xor first word */
		if(tpofst > 5) {	/* must fix second word */
			ctr = (hpaddr->hpec2 >> 1) & ~0100000;
			if(tpofst < 15)
				ctr = ctr >> (15 - tpofst);
			tpbad[1] ^= ctr;	/* xor second word */
			}
		suiword(bsp,tpbad[0]);
		suiword((bsp+2),tpbad[1]);
		UISD->r[0] = tpuis[1];	/* restore User PDR */
		UISA->r[0] = tpuis[0];	/* restore User PAR */
		splx(pri);
		/* now see if transfer finished. */
		/* If not then reset;track,sector,etc. */
		if(hpaddr->hpwc != 0)
			{
			hpaddr->hpcs1.w = TRE|DCLR|GO;
			hp_r[ind].bads |= BAD_CON;
			hpstart(ctrl);	/* Continue transfer */
			return;
			}
		hptab[ctrl].b_active++;
	}
/* End of ECC code					*/

			switch(hp_dt[ind]){
				case RP04:
				case RP05:
				case RP06:
					if(hpaddr->hper1 & 020)/* Fmt Error */
						if(hpvector(bp, ctrl, ind, hpaddr->hpwc)){
						    hp_r[ind].bads |= BAD_VEC;
						    hptab[ctrl].b_errcnt--;
						    hpaddr->hper1 = 0;
						    hpstart(ctrl);
						    return;
						}
						break;
				case RM02:
				case RM03:
				case RM05:
					if(hpaddr->hper3 & DCK)	/* BSE Error */
						if(hpvector(bp, ctrl, ind, hpaddr->hpwc)){
						    hp_r[ind].bads |= BAD_VEC;
						    hptab[ctrl].b_errcnt--;
						    hpaddr->hper3 = 0;
						    hpstart(ctrl);
						    return;
						}
						break;
				default:
					break;
			}
			hpaddr->hpcs1.w = TRE|IE|DCLR|GO;
			if(((hptab[ctrl].b_errcnt&07) == 4)&& (hp_dt[ind] != ML11)) {
				hpaddr->hpcs1.w = RECAL|IE|GO;
				while(hpaddr->hpds & PIP)
					;
			}
		}
		if(hptab[ctrl].b_active) {
			if((hptab[ctrl].b_errcnt) && (hp_dt[ind] != ML11)) {
				hpaddr->hpcs1.w = RTC|GO;
				while(hpaddr->hpds & PIP)
					;
			}
			if(hptab[ctrl].b_errcnt || (bp->b_flags & B_ERROR))
			{
			  if(hp_ebuf[ctrl].hp_reg[6] & WLE)
				printf("\n%s unit %d Write Locked\n",
					hp_dct[ctrl], unit);
			  else {
			    hp_ebuf[ctrl].hp_bdh.bd_errcnt = hptab[ctrl].b_errcnt;
			    if(!logerr(E_BD, &hp_ebuf[ctrl], sizeof(struct hp_ebuf)))
		     deverror(bp, hp_ebuf[ctrl].hp_reg[4], hp_ebuf[ctrl].hp_reg[6]);
			  }
			}
			if(hp_r[ind].bads&BAD_VEC &&(bp->b_flags&B_ERROR)==0){
				hp_r[ind].bads &= ~BAD_VEC;
				if(hp_res[ctrl].r_cc > 0){
					hp_r[ind].bads |= BAD_CON;
					hpstart(ctrl);
					return;
				}
			}
			el_bdact &= ~(1 << (HP_BMAJ+ctrl));
			hp_r[ind].bads &= ~(BAD_VEC|BAD_CON);
			hptab[ctrl].b_active = 0;
			hptab[ctrl].b_errcnt = 0;
			hptab[ctrl].b_actf = dp->b_forw;
			dp->b_active = 0;
			dp->b_errcnt = 0;
			dp->b_actf = bp->av_forw;
			bp->b_resid = -(hpaddr->hpwc<<1);
			iodone(bp);
			hpaddr->hpcs1.w = DPRLS|GO; /* usseize dual port drive*/
			hpaddr->hpcs1.w = IE;
			if(dp->b_actf)
				hpustart(ctrl, unit);
		}
		as &= ~(1<<unit);
	} else {
		if(as == 0)
			hpaddr->hpcs1.w = IE;
		hpaddr->hpcs1.c[1] = TRE>>8;
	}
	for(unit=0; unit<8; unit++)
		if(as & (1<<unit)) {
			hpaddr->hpcs2.w = unit;
			hpaddr->hpcs1.w = (DCLR|GO);
/*
 * In case the attention came from
 * a drive that is seized by the other
 * port, clear the attention summary bit.
 */
			hpaddr->hpas = (1<<unit);	/* clear attn bit */
			if(unit >= nhp[ctrl])
				hpaddr->hpcs1.c[0] = IE;
			else
				hpustart(ctrl, unit);
			}
	hpstart(ctrl);
}

static
hpvector(bp, ctrl, ind, wc)
struct buf *bp;
{
	register int dt, cn, tn;
	int sn, j;
	long bn;

	fixbad(&hp_res[ctrl], bp, wc, 1);
	dt = hp_dipset(ind);
	bn = bp->b_blkno;
	bn += ((bp->b_bcount - hp_res[ctrl].r_cc)/512)-1;
	cn = bn/hpdi[dt].nbpc
		+ hp_sizes[(minor(bp->b_dev)&7)+hpdi[dt].sti].cyloff;
	sn = bn%hpdi[dt].nbpc;
	tn = (sn/hpdi[dt].nsect)<<8;
	tn |= sn%hpdi[dt].nsect;
	if((j = isbad(&hp_bads[ind], cn, tn, hpdi[dt].nsect)) >= 0){
		hp_res[ctrl].r_vbn = (hp_r[ind].vcyl - 1) - j;
#ifdef HPBAD
		hp_bads[ind].pbt_vec[j]++;
#endif
		return(1);
	}
	return(0);
}

hpread(dev)
{

	physio(hpstrategy, &rhpbuf[hpctrl(dev)], dev, B_READ);
}

hpwrite(dev)
{

	physio(hpstrategy, &rhpbuf[hpctrl(dev)], dev, B_WRITE);
}

/*
 * Set the drive information pointer
 * according to the drive type.
 */

static
hp_dipset(ind)
{
	switch(hp_dt[ind]) {
	case RP04:
	case RP05:
		hp_r[ind].vcyl = 171798L - 22;
		return(0);
	case RP06:
		hp_r[ind].vcyl = 340670L - 22;
		return(0);
	case RM02:
	case RM03:
		hp_r[ind].vcyl = 131680L - 32;
		return(1);
	case RM05:
		hp_r[ind].vcyl = 500384L - 32;
		return(2);
	case ML11:
		return(3);
	}
}

/*
 * Determine the RH11/RH70 controller number
 * based on the major device.
 */
static
hpctrl(dev)
{
	register int maj;

	maj = (dev>>8) & 077;
	if(maj < nblkdev)
		return(maj - HP_BMAJ);
	else
		return(maj - HP_RMAJ);
}

hpclose(dev,flag)
dev_t dev;
{

	bflclose(dev);
}
