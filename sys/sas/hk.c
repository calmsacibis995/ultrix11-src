/*
 * SCCSID: @(#)hk.c	3.0	5/12/86
 */
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * RK06/7 standalone disk driver
 * Jerry Brenner
 * Fred Canter - modified for ECC & error recovery
 * 10/20/82
 * Jerry Brenner - Bad block support added
 * 12/21/82
 * Fred Canter - modified to get CSR address from devsw[].dv_csr
 * 7/4/84
 */

#include <sys/param.h>
#include <sys/inode.h>
#include <sys/bads.h>
#include "saio.h"


struct device
{
	int	hkcs1;
	int	hkwc;
	caddr_t	hkba;
	int	hkda;
	int	hkcs2;
	int	hkds;
	int	hkerr;
	int	hkas;
	int	hkdc;
	int	dum;
	int	hkdb;
	int	hkmr1;
	int	hkec1;
	int	hkec2;
	int	hkmr2;
	int	hkmr3;
};

#define	NHK	8
#define	NHKSEC	22
#define	NHKTRK	3
#define	BHK6BN	(22L*3L*411L) - 22L
#define	BHK7BN	(22L*3L*815L) - 22L

#define P400	020
#define M400	0220
#define P800	040
#define M800	0240
#define P1200	060
#define M1200	0260


#define GO	01
#ifdef	SELECT
#undef	SELECT
#endif
#define SELECT	01
#define PAKACK	02
#define DCLR	04
#define RECAL	012
#define OFFSET	014
#define RCOM	020
#define WCOM	022
#define RHDR	024
#define WHDR	026
#define IEI	0100
#define CRDY	0200
#define CCLR	0100000
#define CERR	0100000

#define DLT	0100000
#define UPE	020000
#define NED	010000
#define PGE	02000
#define SCLR	040

#define SVAL	0100000
#define CDA	040000
#define PIP	020000
#define WLE	010000
#define DDT	0400
#define DRDY	0200
#define VV	0100
#define DOFST	04
#define DRA	01

#define DCK	0100000
#define DUNS	040000
#define OPI	020000
#define DTE	010000
#define DWLE	04000
#define BSE	0200
#define ECH	0100
#define SKI	02
#define DRTER 040

extern	int	dk_badf[];
extern struct dkbad dk_bad[];
extern struct dkres dkr[];
int hk_drvtyp[NHK];
char hk_openf;

hkopen(io)
register struct iob *io;
{
	register struct device *hkaddr;
	register int unit;
	int cn, tn, sn, cnt, dkn;
	int hkcmd;
	daddr_t bn;

	hkaddr = devsw[io->i_ino.i_dev].dv_csr;
	unit = io->i_unit;
	if(hk_openf&(1<<unit))
		return(0);
	hkaddr->hkcs2 = SCLR;
	while((hkaddr->hkcs1 & CRDY) == 0);
	hk_drvtyp[unit] = 0;
	hkaddr->hkcs2 = unit;
	hkaddr->hkcs1 = SELECT;
	while((hkaddr->hkcs1 & CRDY) == 0);
	if(hkaddr->hkcs1 & CERR && hkaddr->hkerr & DRTER)
	{
		hk_drvtyp[unit] = 02000;
	}
	hkaddr->hkcs2 = SCLR;
	while((hkaddr->hkcs1 & CRDY) == 0);
	hkaddr->hkcs2 = unit;
	hkaddr->hkcs1 = (hk_drvtyp[unit]|SELECT);
	while((hkaddr->hkcs1 & CRDY) == 0);
	if((hkaddr->hkds & VV) == 0)
	{
		hkaddr->hkcs1 = hk_drvtyp[unit]|PAKACK|GO;
		while((hkaddr->hkcs1 & CRDY) == 0);
	}
	if((dkn = dkn_set(io)) < 0) {
		printf("\nHK unit %d can't allocate bads structure!\n", unit);
		return(-1);
	}
	dk_bad[dkn].bt_mbz = -1;
	/* BAD_CMD is PHYSADR 0140000, would overwrite boot/sdload */
#ifndef	BIGKERNEL
	if(segflag == 2)	/* boot or sdload */
#else	BIGKERNEL
	if(segflag == 3)	/* boot or sdload */
#endif	BIGKERNEL
		hkcmd = 0;	/* BAD_CMD never used! */
	else {
		hkcmd = BAD_CMD->r[0]&07;
		BAD_CMD->r[0] = 0;
	}
	bn = hk_drvtyp[unit]?BHK7BN:BHK6BN;
	for(cnt = 0; cnt < 5; cnt++){
		cn = bn/(NHKSEC*NHKTRK);
		sn = bn%(NHKSEC*NHKTRK);
		tn = sn/NHKSEC;
		sn = sn%NHKSEC;
		hkaddr->hkba = &dk_bad[dkn];
		hkaddr->hkwc = -(sizeof(struct dkbad)>>1);
		hkaddr->hkdc = cn;
		hkaddr->hkda = (tn<<8) | sn;
		hkaddr->hkcs2 = unit;
#ifndef	BIGKERNEL
/*
 * If this is Boot: program (segflag == 2) all I/O starts at 128 KB,
 * otherwise set reads of bad sector file to 64 Kb boundry.
 */
		if(segflag == 2)
#else	BIGKERNEL
/*
 * If this is Boot: program (segflag == 3) all I/O starts at 196 KB,
 * otherwise set reads of bad sector file to 64 Kb boundry.
 */
		if(segflag == 3)
#endif	BIGKERNEL
		    hkaddr->hkcs1 = hk_drvtyp[unit]|(segflag << 8)|RCOM|GO;
		else
		    hkaddr->hkcs1 = hk_drvtyp[unit]|(1 << 8)|RCOM|GO;
		while((hkaddr->hkcs1 & CRDY) == 0);
		if(hkaddr->hkcs1 & CERR){
			if(hkcmd != BAD_CHK)
				hkeprt(hkaddr, unit);
			bn += 2;
			hkaddr->hkcs2 = SCLR;
			while((hkaddr->hkcs1 & CRDY) == 0);
			continue;
		}
		else{
			cnt = 0;
			break;
		}
	}
	if(cnt >= 5) {
		if(hkcmd == BAD_CHK)	/* dskinit reformating disk */
			dk_bad[dkn].bt_mbz = -1;/* invalid bad sector file */
		else {
			dk_badf[dkn] = 0;
			return(-1);
		}
	}
	if(dk_bad[dkn].bt_mbz || dk_bad[dkn].bt_flag
	  || (dk_bad[dkn].bt_csnl == 0 && dk_bad[dkn].bt_csnh == 0)
	  || (dk_bad[dkn].bt_badb[0].bt_cyl==0 && dk_bad[dkn].bt_badb[0].bt_trksec==0))
		dk_bad[dkn].bt_mbz = -1;
	hk_openf |= 1<<unit;
	return(0);
}

hkclose(io)
register struct iob *io;
{
	int unit, dkn;

	if((dkn = dkn_set(io)) >= 0)
		dk_badf[dkn] = 0;
	unit = io->i_unit;
	hk_openf &= ~(1<<unit);
}

hkstrategy(io, func)
register struct iob *io;
{
	register struct device *hkaddr;
	register com;
	int i, unit, dkn;
	daddr_t bn, vbn;
	int sn, cn, tn, j, k, hkcmd, errcnt;
	int *rhdrpnt;

	hkaddr = devsw[io->i_ino.i_dev].dv_csr;
	/* BAD_CMD is PHYSADR 0140000, would overwrite boot/sdload */
#ifndef	BIGKERNEL
	if(segflag == 2)	/* boot or sdload */
#else	BIGKERNEL
	if(segflag == 3)	/* boot or sdload */
#endif	BIGKERNEL
		hkcmd = 0;	/* BAD_CMD never used! */
	else {
		hkcmd = BAD_CMD->r[0]&07;
		BAD_CMD->r[0] = 0;
	}
	unit = io->i_unit;
	errcnt = 0;
hkretry:
	hkaddr->hkcs2 = SCLR;
	while((hkaddr->hkcs1 & CRDY) == 0);
	hkaddr->hkcs2 = unit;
	hkaddr->hkcs1 = hk_drvtyp[unit]|SELECT;
	while((hkaddr->hkcs1 & CRDY) == 0);
	if((hkaddr->hkds & VV) == 0 || (hk_openf&(1<<unit)) == 0){
		hk_openf &= ~(1<<unit);
		if(hkopen(io))
			return(-1);
	}

	dkn = dkn_set(io);
	com = hk_drvtyp[unit]|(segflag << 8) | GO;

	switch(hkcmd & 070)
	{
		case BAD_VEC:
			hkaddr->hkwc = -(dkr[dkn].r_vcc>>1);
			hkaddr->hkba = dkr[dkn].r_vma;
			bn = dkr[dkn].r_vbn;
			if(func == READ)
				com |= RCOM;
			else
				com |= WCOM;
			break;
		case BAD_CON:
			hkcmd &= ~BAD_CON;
			hkaddr->hkwc = -(dkr[dkn].r_cc>>1);
			hkaddr->hkba = dkr[dkn].r_ma;
			bn = dkr[dkn].r_bn;
			if(func == READ)
				com |= RCOM;
			else
				com |= WCOM;
			break;
		default:
			bn = io->i_bn;
			hkaddr->hkba = io->i_ma;
			hkaddr->hkwc = -(io->i_cc>>1);
			if(hkcmd & BAD_FMT)
			{
				if(func == READ)
					com |= RHDR;
				else
					com |= WHDR;
			}
			else{
				if(func == READ)
					com |= RCOM;
				else
					com |= WCOM;
			}
			break;
	}

	if(dk_bad[dkn].bt_mbz == 0 && (hkcmd&(BAD_CHK|BAD_VEC|BAD_FMT)) == 0
	    && func != READ)
	{
		if((bn + (io->i_cc/512)) >=
		  (hk_drvtyp[unit]?BHK7BN:BHK6BN)- NHKSEC)
		{
		    printf("HK %d error. bn = %D cc = %d",unit ,bn, io->i_cc);
			printf(" Would overwrite bad sector information\n");
			return(-1);
		}
	}

	cn = bn/(NHKSEC*NHKTRK);
	sn = bn%(NHKSEC*NHKTRK);
	tn = sn/NHKSEC;
	sn = sn%NHKSEC;

	hkaddr->hkdc = cn;
	hkaddr->hkda = (tn<<8) | sn;
	hkaddr->hkcs2 = unit;
	if(hkcmd & BAD_FMT && func == READ){
		rhdrpnt = io->i_ma;
		j = io->i_cc>>1;
		hkaddr->hkwc = -(j*2);
		for(k = 0; j > 0; ){
			hkaddr->hkcs1 = com;
			while((hkaddr->hkcs1 & CRDY) == 0) ;
			if(hkaddr->hkcs1 & CERR){
				hkeprt(hkaddr, unit);
				return(-1);
			}
/*
			*(rhdrpnt++) = hkaddr->hkdb;
			sn = hkaddr->hkdb;
			*(rhdrpnt++) = sn;
			*(rhdrpnt++) = hkaddr->hkdb;
*/
			fixhdr(rhdrpnt, hkaddr->hkdb);
			rhdrpnt++;
			sn = hkaddr->hkdb;
			fixhdr(rhdrpnt, sn);
			rhdrpnt++;
			fixhdr(rhdrpnt, hkaddr->hkdb);
			rhdrpnt++;
			j -= 3;
			if(k == 0 && (sn &037) != 0){
				rhdrpnt = io->i_ma;
				j = io->i_cc >> 1;
				continue;
			}
			k++;
		}
	}
	else
		hkaddr->hkcs1 = com;

	while((hkaddr->hkcs1 & CRDY) == 0);

	if (hkaddr->hkcs1 & CERR)
	{
		if((hkaddr->hkerr & (DCK|ECH)) == DCK)
		{
		    fixbad(io, hkaddr->hkwc, 0, dkn);
			/* do ECC */
		    k = hkaddr->hkec1 - 1;
		    j = k & 017;
		    k >>= 4;
		    if((k >= 0) && (k < 256))
		    {
			if((hkcmd&BAD_NEC) == 0)
			{
			    printf("HK %d ECC bn = %D\n",unit,dkr[dkn].r_bn - 1);
			    fixecc(((dkr[dkn].r_ma-256)+k),(hkaddr->hkec2<<j));
			    fixecc(((dkr[dkn].r_ma-256)+(k+1)),(hkaddr->hkec2>>(16-j)));
			}
/* TODO: call hkeprt() if ECC ignored (BAD_NEC set) */
			if(dkr[dkn].r_cc > 0)
			{
				hkcmd |= BAD_CON;
				goto hkretry;
			}
			if(hkcmd & 070)
				goto hkcont;
			return(io->i_cc);
		    }
		}
		if((hkaddr->hkerr & BSE) && (hkcmd & BAD_CHK) == 0
 		   && dk_bad[dkn].bt_mbz == 0)
		{
			fixbad(io, hkaddr->hkwc, 1, dkn);
			cn = hkaddr->hkdc;
			tn = (hkaddr->hkda >> 8) &07;
			sn = hkaddr->hkda &037;
			if((j = isbad(&dk_bad[dkn],cn,tn,sn)) >= 0)
			{
			     hkcmd |= BAD_VEC;
			     dkr[dkn].r_vbn=((hk_drvtyp[unit]?BHK7BN:BHK6BN)-1)-j;
			     goto hkretry;
			}
		}
		if(errcnt == 0)
		{
			hkeprt(hkaddr, unit);
			if((hkcmd & BAD_CHK) && (hkaddr->hkerr & 0100700))
			{
	    			fixbad(io, hkaddr->hkwc, 0, dkn);
				if(hkaddr->hkerr &0100000)
					return(dkr[dkn].r_vcc - 512);
				return(dkr[dkn].r_vcc);
			}
		}
		if(++errcnt >= 10)
		{
			printf("\n(FATAL ERROR)\n");
			return(-1);
		}
		goto hkretry;
	}
hkcont:
	if(hkcmd&BAD_VEC)
	{
		if(dkr[dkn].r_cc > 0)
		{
			hkcmd &= ~BAD_VEC;
			hkcmd |= BAD_CON;
			goto hkretry;
		}
	}
	if(errcnt)
		printf("\n(RECOVERED by retry)\n");
	return(io->i_cc);
}

hkeprt(hkaddr, unit)
register struct device *hkaddr;
{
	printf("HK unit %d disk error:\n", unit);
	printf("cs1=%o cs2=%o ds=%o err=%o ", hkaddr->hkcs1,
		hkaddr->hkcs2, hkaddr->hkds, hkaddr->hkerr);
	printf("hkdc=%d track=%d sect=%d\n", hkaddr->hkdc
		, (hkaddr->hkda&03400)>>8, hkaddr->hkda&037);
}
