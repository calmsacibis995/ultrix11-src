/*
 * SCCSID: @(#)hp.c	3.0	5/12/86
 */
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * RP04/5/6, RM02/3/5, & ML11 standalone disk driver
 * Supports 3 RH11/RH70 controllers with 8 units:
 *
 * HP 0176700	first RH
 * HM 0176300	second RH
 * HJ 0176400	third RH
 *
 * Fred Canter
 * Jerry Brenner - Bad Block support added 12/21/82
 */

#include <sys/param.h>
#include <sys/inode.h>
#include <sys/bads.h>
#include <sys/devmaj.h>
#include "saio.h"

struct	device
{
	union {
		int	w;
		char	c[2];
	} hpcs1;		/* Control and Status register 1 */
	int	hpwc;		/* Word count register */
	caddr_t	hpba;		/* UNIBUS address register */
	int	hpda;		/* Desired address register */
	union {
		int	w;
		char	c[2];
	} hpcs2;		/* Control and Status register 2*/
	int	hpds;		/* Drive Status */
	int	hper1;		/* Error register 1 */
	int	hpas;		/* Attention Summary */
	int	hpla;		/* Look ahead */
	int	hpdb;		/* Data buffer */
	int	hpmr;		/* Maintenance register */
	int	hpdt;		/* Drive type */
	int	hpsn;		/* Serial number */
	int	hpof;		/* Offset register */
	int	hpdc;		/* Desired Cylinder address register*/
	int	hpcc;		/* Current Cylinder */
	int	hper2;		/* Error register 2 */
	int	hper3;		/* Error register 3 */
	int	hpec1;		/* Burst error bit position */
	int	hpec2;		/* Burst error bit pattern */
	int	hpbae;		/* 11/70 bus extension */
	int	hpcs3;
};

#define	RM02	025
#define	RM03	024
#define	RM05	027
#define	RP04	020
#define	RP05	021
#define	RP06	022
#define	ML11	0110
#define NRPSEC	22
#define NRPTRK	19
#define NRP5CYL	411
#define NRP6CYL	815
#define NRMSEC	32
#define NRMTRK	5
#define NRMCYL	823

#define	GO	01
#define	PRESET	020
#define DCLR	010
#define	CCLR	040
#define	WCOM	060
#define	RCOM	070
#define WHDR	062
#define RHDR	072

#define	IE	0100
#define	PIP	020000
#define	DRY	0200
#define	ERR	040000
#define	TRE	040000
#define	DCK	0100000
#define	WLE	04000
#define	ECH	0100
#define VV	0100
#define FMT22	010000
#define FER	020
#define BSE	0100000

/*
 * All of the following data structures, arrays, etc,
 * are indexed by a magic number called `dkn'.
 * The idea here is that a maximum of two drives can be
 * active at any one time (disk to disk copy).
 * So, there are only two of each type of data structure.
 * A data structure is assigned to a drive on a first come
 * first serve basis. `dkn' is set to 0 or 1 depending on
 * which structure is used.
 */
extern	int	dk_badf[];
extern struct dkbad dk_bad[];
extern struct dkres dkr[];
int	nhpsect[2];
int	nhptrac[2];
int	hpxdt[2];
long vcyl[2];
/*
 * The following are indexed by the controller number.
 */
char	hp_openf[4];	/* open flag, one char per RH, one bit per unit */
char	*hp_dct[] = {"HP", "HM", "HJ", 0};	/* Name for error messages */

hpopen(io)
register struct iob *io;
{
	register int unit, ctrl;

	ctrl = devsw[io->i_ino.i_dev].dv_cn;
	unit = io->i_unit;
	if(hp_openf[ctrl] & (1<<unit))
		return(0);
	if(hpmnt(io) == 0)
		hp_openf[ctrl] |= (1<<unit);
	else
		return(-1);
}

hpclose(io)
register struct iob *io;
{
	register int ctrl, dkn;

	ctrl = devsw[io->i_ino.i_dev].dv_cn;
	hp_openf[ctrl] &= ~(1<<io->i_unit);
	if((dkn = dkn_set(io)) >= 0)
		dk_badf[dkn] = 0;
}

hpmnt(io)
register struct iob *io;
{
	register struct device *addr;
	register int ctrl;
	int cn, tn, sn, unit, cnt, dkn;
	int hpcmd;
	long bn;

	ctrl = devsw[io->i_ino.i_dev].dv_cn;
	unit = io->i_unit;
	if((dkn = dkn_set(io)) < 0) {
		printf("\n%s unit %d can't allocate bads structure!\n",
			hp_dct[ctrl], unit);
		return(-1);
	}
	addr = devsw[io->i_ino.i_dev].dv_csr;
	addr->hpcs2.w = CCLR;
	addr->hpcs2.w = unit;
	hpxdt[dkn] = addr->hpdt & 0377;
	switch(hpxdt[dkn]) {
	case ML11:
		nhpsect[dkn] = NRPSEC;
		nhptrac[dkn] = NRPTRK;
		vcyl[dkn]  = -1L;
		dk_bad[dkn].bt_mbz = -1;
		return(0);
		break;
	case RP04:
	case RP05:
		nhpsect[dkn] = NRPSEC;
		nhptrac[dkn] = NRPTRK;
		vcyl[dkn]  = ((long)NRP5CYL*(long)nhptrac[dkn]*(long)nhpsect[dkn])- nhpsect[dkn];
		break;
	case RP06:
		nhpsect[dkn] = NRPSEC;
		nhptrac[dkn] = NRPTRK;
		vcyl[dkn]  = ((long)NRP6CYL*(long)nhptrac[dkn]*(long)nhpsect[dkn])- nhpsect[dkn];
		break;
	case RM02:
	case RM03:
		nhpsect[dkn] = NRMSEC;
		nhptrac[dkn] = NRMTRK;
		vcyl[dkn]  = ((long)NRMCYL*(long)nhptrac[dkn]*(long)nhpsect[dkn])- nhpsect[dkn];
		break;
	case RM05:
		nhpsect[dkn] = NRMSEC;
		nhptrac[dkn] = NRPTRK;
		vcyl[dkn]  = ((long)NRMCYL*(long)nhptrac[dkn]*(long)nhpsect[dkn])- nhpsect[dkn];
		break;
	default:
		dk_badf[dkn] = 0;
		return(-1);
	}
	if((addr->hpds & VV) == 0)
		addr->hpcs1.c[0] = PRESET|GO;
	dk_bad[dkn].bt_mbz = -1;
	/* BAD_CMD is PHYSADR 0140000, would overwrite boot/sdload */
#ifndef	BIGKERNEL
	if(segflag == 2)	/* boot or sdload */
#else	BIGKERNEL
	if(segflag == 3)	/* boot or sdload */
#endif	BIGKERNEL
		hpcmd = 0;	/* BAD_CMD never used! */
	else {
		hpcmd = BAD_CMD->r[0]&07;
		BAD_CMD->r[0] = 0;
	}
	for(cnt = 0,bn = vcyl[dkn]; cnt < 5; cnt++){
		cn = bn/(nhpsect[dkn]*nhptrac[dkn]);
		sn = bn%(nhpsect[dkn]*nhptrac[dkn]);
		tn = sn/nhpsect[dkn];
		sn = sn%nhpsect[dkn];
		addr->hpof = FMT22;
		addr->hpba = &dk_bad[dkn];
		addr->hpwc = -(sizeof(struct dkbad)>>1);
		addr->hpdc = cn;
		addr->hpda = (tn << 8) + sn;
		addr->hpcs2.w = unit;
#ifndef	BIGKERNEL
/*
 * If this is Boot: program (segflag == 2) all I/O starts at 128 KB,
 * otherwise set reads of bad sector file to 64 KB boundry.
 */
		if(segflag == 2)
#else	BIGKERNEL
/*
 * If this is Boot: program (segflag == 3) all I/O starts at 196 KB,
 * otherwise set reads of bad sector file to 64 KB boundry.
 */
		if(segflag == 3)
#endif	BIGKERNEL
			addr->hpcs1.w = (segflag << 8)|RCOM|GO;
		else
			addr->hpcs1.w = (1 << 8)|RCOM|GO;
		while ((addr->hpcs1.w&DRY) == 0);
		if (addr->hpcs1.w & TRE) {
			if(hpcmd != BAD_CHK)
				hpeprt(ctrl, addr, unit, bn, dkn);
			addr->hpcs2.w = CCLR;
			bn += 2;
		}
		else{
			cnt = 0;
			break;
		}
	}
	if(cnt >= 5) {
		if(hpcmd == BAD_CHK)	/* dskinit reformat in progress */
			dk_bad[dkn].bt_mbz = -1; /* invalid bad sector file */
		else {
			dk_badf[dkn] = 0;
			return(-1);
		}
	}
	if(dk_bad[dkn].bt_mbz || dk_bad[dkn].bt_flag
	 ||(dk_bad[dkn].bt_csnl==0&&dk_bad[dkn].bt_csnh==0)
	 ||(dk_bad[dkn].bt_badb[0].bt_cyl==0 && dk_bad[dkn].bt_badb[0].bt_trksec==0))
		dk_bad[dkn].bt_mbz = -1;
	return(0);
}
hpstrategy(io, func)
register struct iob *io;
{
	register struct device *addr;
	register unit;
	int	ctrl, dkn;
	daddr_t bn;
	int hpcmd;
	int sn, cn, tn, j, k, errcnt, cmd, i;

	ctrl = devsw[io->i_ino.i_dev].dv_cn;
	dkn = dkn_set(io);
	/* BAD_CMD is PHYSADR 0140000, would overwrite boot/sdload */
#ifndef	BIGKERNEL
	if(segflag == 2)	/* boot or sdload */
#else	BIGKERNEL
	if(segflag == 3)	/* boot or sdload */
#endif	BIGKERNEL
		hpcmd = 0;	/* BAD_CMD never used! */
	else {
		hpcmd = BAD_CMD->r[0]&07;
		BAD_CMD->r[0] = 0;
	}
	errcnt = 0;
	unit = io->i_unit;
	addr = devsw[io->i_ino.i_dev].dv_csr;
	addr->hpcs2.w = CCLR;
	addr->hpcs2.w = unit;
	hpxdt[dkn] = addr->hpdt & 0377;
	switch(hpxdt[dkn]) {
		case ML11:
		case RP04:
		case RP05:
		case RP06:
		case RM02:
		case RM03:
		case RM05:
			break;
	default:
		printf("\n%s unit %d - unknown drive type (%o)\n",
			hp_dct[ctrl], unit, hpxdt[dkn]);
		return(-1);
	}

hpretry:
	addr->hpcs2.w = CCLR;
	addr->hpcs2.w = unit;
	if((addr->hpds & VV) == 0 || (hp_openf[ctrl] & (1<<unit))==0){
		if(hpmnt(io))
			return(-1);
	}
	while ((addr->hpcs1.w&DRY) == 0);

	cmd = (segflag << 8) | GO;

	switch(hpcmd & 070){
		case BAD_VEC:
			addr->hpwc = -(dkr[dkn].r_vcc>>1);
			addr->hpba = dkr[dkn].r_vma;
			bn = dkr[dkn].r_vbn;
			cmd |= (func == READ)?RCOM:WCOM;
			break;
		case BAD_CON:
			hpcmd &= ~BAD_CON;
			addr->hpwc = -(dkr[dkn].r_cc>>1);
			addr->hpba = dkr[dkn].r_ma;
			bn = dkr[dkn].r_bn;
			cmd |= (func == READ)?RCOM:WCOM;
			break;
		default:
			addr->hpba = io->i_ma;
			addr->hpwc = -(io->i_cc>>1);
			bn = io->i_bn;
			if(hpcmd & BAD_FMT){
				if(func == READ)
					cmd |= RHDR;
				else
					cmd |= WHDR;
			}
			else if(func == READ)
				cmd |= RCOM;
			else
				cmd |= WCOM;
			break;
	}
	if(dk_bad[dkn].bt_mbz == 0 && (hpcmd & (BAD_VEC|BAD_CHK|BAD_FMT)) == 0
	  && hpxdt[dkn] != ML11 && func != READ){
		if((bn +(io->i_cc/512)) >= (vcyl[dkn] - nhpsect[dkn]))
		{
			printf("%s %d error. Would", hp_dct[ctrl], unit);
			printf(" overwrite bad sector");
			printf(" information\n");
			return(-1);
		}
	}
	if(hpxdt[dkn] == ML11)
		addr->hpda = bn & 0177777;
	else {
		cn = bn/(nhpsect[dkn]*nhptrac[dkn]);
		sn = bn%(nhpsect[dkn]*nhptrac[dkn]);
		tn = sn/nhpsect[dkn];
		sn = sn%nhpsect[dkn];
	
		addr->hpdc = cn;
		addr->hpda = (tn << 8) + sn;
	}

	addr->hpcs1.w = cmd;
hpagain:
	while ((addr->hpcs1.w&DRY) == 0)
			;
	if (addr->hpcs1.w & TRE) {
		if((addr->hper1 & (DCK|ECH)) == DCK){
		    /* do ECC */
		    fixbad(io, addr->hpwc, 0, dkn);
		    k = addr->hpec1 - 1;
		    j = k & 017;
		    k >>= 4;
		    if((k >= 0) && (k < 256)) {
			if((hpcmd & BAD_NEC) == 0)
			{
			    fixecc(((dkr[dkn].r_ma-256)+k),(addr->hpec2 << j));
			    fixecc(((dkr[dkn].r_ma-256)+(k+1)),(addr->hpec2>>(16-j)));
				printf("%s %d ECC bn = %D\n", hp_dct[ctrl],
					unit, dkr[dkn].r_bn-1);
			}
/* TODO: call hpeprt() here, if ECC ignored (BAD_NEC set) */
			if(dkr[dkn].r_cc > 0){
				hpcmd |= BAD_CON;
				goto hpretry;
			}
			if(hpcmd & 070)
				goto hpcont;
			return(io->i_cc);
		    }
		}
		if((hpcmd & BAD_CHK) == 0 && dk_bad[dkn].bt_mbz == 0
			&& (hpcmd & BAD_FMT) == 0)
		{
			switch(hpxdt[dkn]){
				case RP04:
				case RP05:
				case RP06:
					if(addr->hper1&FER)
						if(hpvector(unit, io, vcyl[dkn])){
							hpcmd |= BAD_VEC;
							goto hpretry;
						}
					break;
				case RM02:
				case RM03:
				case RM05:
					if(addr->hper3&BSE)
						if(hpvector(unit, io, vcyl[dkn])){
							hpcmd |= BAD_VEC;
							goto hpretry;
						}
					break;
				case ML11:
				default:
					break;
			}
		}
		if(errcnt == 0) {
			switch(hpxdt[dkn]) {
			case RP04:
			case RP05:
			case RP06:
				/*
				 * Special case for ECH error while reading
				 * header during revector.
				 * Ignore FER, HCRC and HCE bad news!
				 * Fred Canter -- 8/20/85
				 */
				if((hpcmd&BAD_FMT) &&
				   ((addr->hper1&0100700) == (DCK|ECH)) &&
				   (io->i_cc == 520)) {
					hpeprt(ctrl, addr, unit, bn, dkn);
					return(520);
				}
				if((hpcmd&BAD_FMT) && (addr->hper1&FER)) {
					addr->hper1 = 0;
					if(addr->hpwc != 0){
					  addr->hpcs1.w|=(addr->hpcs1.c[0]|GO);
					  goto hpagain;
					}
					else
						goto hpcont;
				}
				if((hpcmd&BAD_CHK) && (addr->hper1&0100720)) {
					hpeprt(ctrl, addr, unit, bn, dkn);
					fixbad(io, addr->hpwc, 0, dkn);
					if(addr->hper1 &0100000)
						return(dkr[dkn].r_vcc - 512);
					return(dkr[dkn].r_vcc);
				}
				break;
			case RM02:
			case RM03:
			case RM05:
				/*
				 * Special case for ECH error while reading
				 * header during revector.
				 * HCRC and HCE bad news!
				 * Fred Canter -- 8/20/85
				 */
				if((hpcmd&BAD_FMT) &&
				   ((addr->hper1&0100700) == (DCK|ECH)) &&
				   (io->i_cc == 520)) {
					hpeprt(ctrl, addr, unit, bn, dkn);
					return(520);
				}
				if(hpcmd&BAD_FMT && addr->hper3&BSE){
					addr->hper3 = 0;
					if(addr->hpwc != 0){
					  addr->hpcs1.w|=(addr->hpcs1.c[0]|GO);
					  goto hpagain;
					}
					else
						goto hpcont;
				}
				if(hpcmd&BAD_CHK && (addr->hper1&0100700
				    || addr->hper3&BSE)){
					hpeprt(ctrl, addr, unit, bn, dkn);
					addr->hper3 = 0;
					fixbad(io, addr->hpwc, 0, dkn);
					if(addr->hper1 &0100000)
						return(dkr[dkn].r_vcc - 512);
					return(dkr[dkn].r_vcc);
				}
				break;
			default:
				break;
			}
			hpeprt(ctrl, addr, unit, bn, dkn);
		}
		if(++errcnt >= 10) {
			printf("\n(FATAL ERROR)\n");
			return(-1);
		}
		goto hpretry;
	}
hpcont:
	if(hpcmd & BAD_VEC){
		if(dkr[dkn].r_cc > 0)
		{
			hpcmd &= ~BAD_VEC;
			hpcmd |= BAD_CON;
			goto hpretry;
		}
	}
	if(errcnt)
		printf("\n(RECOVERED by retry)\n");
	return(io->i_cc);
}

hpvector(unit, io, vcyl)
int unit;
struct iob *io;
long vcyl;
{
	register struct device *addr;
	int cn, tn, sn, j, dkn;

	addr = devsw[io->i_ino.i_dev].dv_csr;
	dkn = dkn_set(io);
	fixbad(io, addr->hpwc, 1, dkn);
	cn = addr->hpdc;
	tn = addr->hpda >> 8;
	sn = addr->hpda & 0377;
/*
 * HARDWARE HACK - backup disk address by one sector.
 */
	if(--sn < 0) {
		sn = nhpsect[dkn] - 1;
		if(--tn < 0) {
			tn = nhptrac[dkn] - 1;
			cn--;
		}
	}
	if((j = isbad(&dk_bad[dkn], cn, tn, sn)) >= 0){
		dkr[dkn].r_vbn = (vcyl-1) - j;
		return(1);
	}
	return(0);
}

hpeprt(ctrl, addr, unit, bn, dkn)
struct device *addr;
int unit, ctrl, dkn;
long bn;
{
	printf("%s unit %d disk error:\n", hp_dct[ctrl], unit);
	printf("cs1=%o cs2=%o ds=%o er1=%o ", addr->hpcs1,
		addr->hpcs2, addr->hpds, addr->hper1);
	if(hpxdt[dkn] == ML11)
		printf("bn=%d ", bn);
	else
		printf("cyl=%d track=%d sect=%d ", addr->hpdc
		  , addr->hpda>>8, addr->hpda&037);
	switch(hpxdt[dkn]) {
	case ML11:
		printf("ee=%o\n", addr->hper3);
		break;
	case RP04:
	case RP05:
	case RP06:
		printf("er2=%o er3=%o\n", addr->hper2, addr->hper3);
		break;
	case RM02:
	case RM03:
	case RM05:
		printf("er2=%o\n", addr->hper3);
		break;
	}
}
