
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)elp2.c	3.1	3/26/87";
/*
 * ULTRIX-11 error log report program (elp) - PART 2
 * Fred Canter 7/2/83
 *
 * The input for the report is taken from the current
 * error log or the optional [file].
 *
 * PART 2 of elp does the following:
 *
 * 1.	Contains the error log read buffer.
 *
 * 2.	Contains the error type heading and
 *	device name text string arrays.
 *
 * 3.	Contains all variables and arrays used
 *	for maintaining the hard and soft error counts.
 *
 * 4.	Counts the number of occurences of each type
 *	of error and counts the number of hard and
 *	soft errors for device errors.
 *
 * 5.	Prints the summary portion of the error log report.
 *
 */

#include "elp.h"	/* must be first (param.h) */
#include <sys/devmaj.h>
#include <sys/tmscp.h>	/* must preceed errlog.h (EL_MAXSZ) */
#include <sys/ra_info.h>
#include <sys/errlog.h>
#include <stdio.h>
#include <time.h>

/*
 * Error log input file read buffer.
 */

int	buf[256+1];

/*
 * Error type header messages
 */

char	*et_head[]
{
	"dummy",
	"ERROR LOGGING STARTUP",
	"ERROR LOGGING SHUTDOWN",
	"TIME CHANGE",
	"STRAY INTERRUPT",
	"STRAY VECTOR",
	"MEMORY PARITY ERROR",
	"BLOCK I/O DEVICE ERROR",
	0
};
/*
 * Device name header messages
 */

char	*dntab[]
{
	"(RK11) - RK03/RK05",
	"(RP11) - RP03",
	"(MSCP) - RC25/RX33/RX50/RD31-32/RD51-54/RA60/RA80/RA81",  /* default */
	"(RL11) - RL01/RL02",
	"(RX211) - RX02",
	"(TM11) - TU10/TE10/TS03",
	"(TMSCP) - TK50/TU81",
	"(TS11/TSV05/TSU05/TU80/TK25)",
	"(RH11/RH70 - TM02/3) - TU16/TE16/TU77",
	"(first  - RH11/RH70) - RP04/5/6, RM02/3/5, ML11",
	"(second - RH11/RH70) - RP04/5/6, RM02/3/5, ML11",
	"(RK611/RK711) - RK06/RK07",
	"(third  - RH11/RH70) - RP04/5/6, RM02/3/5, ML11",
	" CONSOLE",
	" PC11",
	" LP11",
	" DC11",
	" DH11",
	" DP11",
	" DHU11/DHV11",
	" DN11",
	" DZ11/DZV11/DZQ11",
	" DU11",
	0
};
/*
 * MSCP controller type name string,
 * setup by setrat(),
 * index is controller number.
 */
char	*ra_ctn[MAXUDA];
int	ra_cid[MAXUDA];	/* Numerical controller ID */
/*
 * MSCP controller type name strings,
 * index is UQPORT controller type ID.
 */
char	*radntab[] =
{
	"(UDA50) - RA60/RA80/RA81",
	"(KLESI) - RC25",
	"(RUX1) - RX50",
	"UNKNOWN",
	"UNKNOWN",	/*	"(KDA25) - RA60/RA80/RA81",	*/
	"UNKNOWN",
	"(UDA50A) - RA60/RA80/RA81",
	"(RQDX1) - RX33/RX50/RD31/RD32/RD51/RD52/RD53/RD54",
	"UNKNOWN",
	"UNKNOWN",
	"UNKNOWN",
	"UNKNOWN",
	"UNKNOWN",
	"(KDA50) - RA60/RA80/RA81",
	"(RQDX3) - RX33/RX50/RD31/RD32/RD51/RD52/RD53/RD54",
	"UNKNOWN",
};

/*
 * Hard and soft error count arrays.
 * The (ht) has 64 units, (rl) has 4, rest have 8.
 * The (rl) is treated as if it had 8 units to save code.
 *
 */

int	rk_hec[8];
int	rk_sec[8];
int	rp_hec[8];
int	rp_sec[8];
int	ra_hec[MAXUDA][8];
int	ra_sec[MAXUDA][8];
int	rl_hec[8];
int	rl_sec[8];
int	rx_hec[8];
int	rx_sec[8];
int	tm_hec[8];
int	tm_sec[8];
int	tk_hec[8];
int	tk_sec[8];
int	ts_hec[8];
int	ts_sec[8];
int	ht_hec[64];
int	ht_sec[64];
int	hp_hec[8];
int	hp_sec[8];
int	hm_hec[8];
int	hm_sec[8];
int	hk_hec[8];
int	hk_sec[8];
int	hj_hec[8];
int	hj_sec[8];

/*
 * These arrays hold the ECC recovered and retry 
 * recovered soft error counts for the hp, hm, hj, hk disks.
 */

int	hp_ecce[8];
int	hp_rtce[8];
int	hm_ecce[8];
int	hm_rtce[8];
int	hj_ecce[8];
int	hj_rtce[8];
int	hk_ecce[8];
int	hk_rtce[8];

/*
 * Various time buffers & pointers.
 */

time_t	timbuf;
long	lotim, hitim;
struct tm *tl, *th;

/*
 * Error log summary report
 * Called by main() in PART 1 (elp1.c)
 */

ersum(fi, filen)
{

	register struct el_rec *erp;
	register char *ebp;
	int	*ec;
	daddr_t bn;
	int	lim, rn;
	int	elfull;
	int	i, j, f, ft;
	int	rtc;
	int	bde_ec;
	int	etyp, maj, dn, cn;
	char	sz;
	int	dskip;

/*
 * Read thru the error log and count
 * the number of occurrences of each error type.
 */

	elfull = bn = 0;
	hitim = 0;
	ft = 0;
	bde_ec = 0;
	badrn = -1;
	if(!fnflag)
		lseek(fi, (long)((bn+el_sb)*512), 0);
ecloop:
	if(!fnflag)
		if((bn + el_sb) >= (el_sb + el_nb)) {
			elfull++;
			bn--;
			goto eclend;
			}
	if(read(fi, (char *)&buf, 512) != 512) {
	if(fnflag) {
		elfull++;
		bn--;
		goto eclend;
		}
	rderr:
		fprintf(stderr,"\nelp: read error bn = %D\n", bn+el_sb);
		exit(1);
		}
	ebp = &buf;
	erp = ebp;
	rn = 0;
	while((ebp < &buf[256]) && ((etyp = erp->er_hdr.e_type) != E_EOB)) {
/*
 * If record size is odd, zero, or
 * greater than the maximum size
 * treat it is end of file.
 */
		if(etyp != E_EOF)
		{
		sz = erp->er_hdr.e_size;
		if((sz == 0) || (sz & 1) | (sz > EL_MAXSZ)) {
		fprintf(stderr, "\nelp: Error record with invalid size\n     ");
		  fprintf(stderr, "bn = %D, rn = %d, size = %d bytes",bn,rn,sz);
			fprintf(stderr, "\n     Assuming end of file\n");
			badbn = bn;
			badrn = rn;
			badet = etyp;
			badsz = sz;
			goto eclend;
			}
		}
/*
 * If date flag is set use specified date/time limits,
 * otherwise use first to last date.
 */

		dskip = 0;
		if(dflag) {
			timbuf = erp->er_hdr.e_time;
			tl = localtime(&timbuf);
			elrtim[0] = tl->tm_sec;
			elrtim[1] = tl->tm_min;
			elrtim[2] = tl->tm_hour;
			elrtim[3] = tl->tm_mday;
			elrtim[4] = tl->tm_mon;
			elrtim[5] = tl->tm_year;
			elrtim[5] += 1900;
	
		/* if error log record date/time is out of range, ignore it */
			if(gte(&elrtim, &timl) && lte(&elrtim, &timh)) {
				if(!ft) {	/* set range to time of first error */
					lotim = timbuf;
					hitim = timbuf;
					ft++;
					}
				} else
					dskip++;
			}
		switch(etyp) {
		case E_EOF:	/* end of error log */
			if(bn == 0 && rn == 0) {
			fprintf(stderr,"\nelp: no errors have been logged\n");
				exit(1);
				}
			goto eclend;
		case E_SU:	/* startup */
				/* First record will always be a startup, */
				/* save its time. */
			if(bn == 0 && rn == 0 && !dflag)
				lotim = erp->er_hdr.e_time;
			/*
			 * Set MSCP disk controller type ID,
			 * UDA50/UDA50A/RQDX1/RQDX3/RUX1/KLESI message selection.
			 */
			setrat(erp->er_body.su.e_mccw);
		case E_SD:	/* shutdown */
		case E_TC:	/* time change */
		case E_SI:	/* stray interrupt */
		case E_SV:	/* stray vector */
		case E_MP:	/* memory parity */
			if(!dskip)
				elc[etyp].tec++;
			break;
		case E_BD:	/* block I/O device */
			if(dskip)
				break;
			bde_ec++;
			maj = (erp->er_body.bd.bd_dev >> 8) & 077; /* maj dev */
			dn = erp->er_body.bd.bd_dev;
			cn = (dn >> 6) & 3;	/* used with MSCP disks only */
			if(maj >= nblkdev) { 	/* RAW device */
				for(etyp=E_BD; etyp<(E_BD+nblkdev); etyp++)
					if(elc[etyp].rmaj == maj)
						break;
				if(maj == HT_RMAJ)
					dn &= 077;	/* unit number */
				else if(maj == RP_RMAJ
					|| maj == HP_RMAJ
					|| maj == HM_RMAJ
					|| maj == HJ_RMAJ
					|| maj == RA_RMAJ
					|| maj == RL_RMAJ
					|| maj == HK_RMAJ)
						dn = (dn >> 3) & 7;
				else if(maj == HX_RMAJ)
					dn &= 1;
				else
					dn &= 7;
			} else {				/* BLOCK dev */
				for(etyp=E_BD; etyp<(E_BD+nblkdev); etyp++)
					if(elc[etyp].bmaj == maj)
						break;
				if(maj == HT_BMAJ)
					dn &= 077;
				else if(maj == RP_BMAJ
					|| maj == HP_BMAJ
					|| maj == HM_BMAJ
					|| maj == HJ_BMAJ
					|| maj == RL_BMAJ
					|| maj == RA_BMAJ
					|| maj == HK_BMAJ)
						dn = (dn >> 3) & 7;
				else if(maj == HX_BMAJ)
					dn &= 1;
				else
					dn &= 7;
				}
			rtc = erp->er_body.bd.bd_errcnt;	/* retry count */
			/* special case for MSCP disks (like everything else) */
			if(elc[etyp].bmaj == RA_BMAJ) {
				if((rtc==0) || (rtc > elc[etyp].rtc))
					ra_hec[cn][dn]++;  /* hard error */
				else
					ra_sec[cn][dn]++;  /* soft error */
				elc[etyp].tec++;	/* total error count */
				break;
			}
			if((rtc == 0) || (rtc > elc[etyp].rtc)) { /* hard err */
				ec = elc[etyp].hec;
				*(ec + dn) += 1;
			} else {				/* soft err */
				ec = elc[etyp].sec;
				*(ec + dn) += 1;
				if(elc[etyp].bmaj == HP_BMAJ) {	/* HP soft */
				    if ((erp->er_body.bd.bd_flags & E_READ)
				      && ((erp->devreg[6] & (DCK|ECH)) == DCK))
					  hp_ecce[dn]++;  /* ecc recovered */
				    else
					  hp_rtce[dn]++;  /* retry recovered */
				}
				if(elc[etyp].bmaj == HM_BMAJ) {	/* HM soft */
				    if ((erp->er_body.bd.bd_flags & E_READ)
				      && ((erp->devreg[6] & (DCK|ECH)) == DCK))
					  hm_ecce[dn]++;  /* ecc recovered */
				    else
					  hm_rtce[dn]++;  /* retry recovered */
				}
				if(elc[etyp].bmaj == HJ_BMAJ) {	/* HJ soft */
				    if ((erp->er_body.bd.bd_flags & E_READ)
				      && ((erp->devreg[6] & (DCK|ECH)) == DCK))
					  hj_ecce[dn]++;  /* ecc recovered */
				    else
					  hj_rtce[dn]++;  /* retry recovered */
				}
				if(elc[etyp].bmaj == HK_BMAJ) {	/* HK soft */
				    if ((erp->er_body.bd.bd_flags & E_READ)
				      && ((erp->devreg[6] & (DCK|ECH)) == DCK))
					  hk_ecce[dn]++;  	/* ecc recovered */
				    else
					  hk_rtce[dn]++;  	/* retry recovered */
				}
			}
			elc[etyp].tec++;	/* total error count */
			break;
		default:
	   fprintf(stderr,"\nelp: Error record with invalid error type\n     ");
	      fprintf(stderr,"bn = %D, rn = %d, error type = %d", bn,rn,etyp);
		fprintf(stderr, "\n     Assuming end of file\n");
			badbn = bn;
			badrn = rn;
			badet = etyp;
			badsz = erp->er_hdr.e_size;
			goto eclend;
		}

	if(!dflag || !dskip) {
		timbuf = erp->er_hdr.e_time;
		if(timbuf < lotim)
			lotim = timbuf;
		if(timbuf > hitim)
			hitim = timbuf;
		}
around:
	ebp += erp->er_hdr.e_size;	/* move pointer to next record */
	erp = ebp;
	rn++;
	}
	bn++;
	goto ecloop;
eclend:

/* get the current time & print the report header msg's */

	time(&timbuf);
	printf("\nULTRIX-11 SYSTEM ERROR REPORT, taken on - %s\n",ctime(&timbuf));
	if(elfull)
		printf("\n****** This error log is full. ******\n\n");
	if(badrn >= 0) {
	    printf("\n****** Report terminated by a bad error record ! ******");
	    printf("\n\n****** Block number	%D	******", badbn);
	    printf("\n****** Record number	%d	******", badrn);
	    printf("\n****** Error type	%d	******", badet);
	    printf("\n****** Record size	%d	******\n", badsz);
		}
	printf("\nError type limitations: [ ");
	if(!etflag)
		printf("NONE ]");
	else if(et >= E_BD) {
		if((etcn < 0) || (etcn == MAXUDA))
			printf("%s ]", dntab[elc[et].edn]);
		else
			printf("%s ]", ra_ctn[etcn]);
		if(etdn >= 0)
			printf("\n\t\t\t[ unit %d only ]", etdn);
	} else
		printf("%s ]", et_head[elc[et].ehm]);
	if(rflag)
		printf("\n\t\t\t[ For block I/O devices, recovered errors only ]");
	if(uflag)
		printf("\n\t\t\t[ For block I/O devices, unrecovered errors only ]");
	printf("\n");
	printf("\nInput taken from\t%s\n", filen);
	if(!fnflag) {
		printf("Error log start block\t%D\n", el_sb);
		printf("Length in blocks\t%d\n", el_nb);
	printf("\nCurrent error log has %D of %d blocks used.\n", (bn+1), el_nb);
		}
	printf("\nREPORTED DATE/TIME RANGE: ");
	if(dflag)
		printf("[ only errors within specified limits ]\n\n");
	else
		printf("[ first error thru last error ]\n\n");
	printf("\t%.24s thru ", ctime(&lotim));
	printf("%s\n", ctime(&hitim));
	if(fflag)
		return;
	if(!etflag || (etflag && et == E_SU))
		printf("\nTOTAL startup\t\t= %d\n",elc[E_SU].tec);
	if(!etflag || (etflag && et == E_SD))
		printf("\nTOTAL shutdown\t\t= %d\n",elc[E_SD].tec);
	if(!etflag || (etflag && et == E_TC))
		printf("\nTOTAL time change\t= %d\n",elc[E_TC].tec);
	if(!etflag || (etflag && et == E_SI))
		printf("\nTOTAL stray interrupts\t= %d\n",elc[E_SI].tec);
	if(!etflag || (etflag && et == E_SV))
		printf("\nTOTAL stray vectors\t= %d\n",elc[E_SV].tec);
	if(!etflag || (etflag && et == E_MP))
		printf("\nTOTAL memory parity\t= %d\n",elc[E_MP].tec);
	if(!etflag || (etflag && et >= E_BD))
		printf("\nTOTAL block I/O errors\t= %d\n\n",bde_ec);
	for(i=0; i<nblkdev; i++) {
	    if(i == RA_BMAJ) {	/* yet another MSCP special case */
		for(cn=0; cn<MAXUDA; cn++) {
		    if((etcn >= 0) && (etcn != cn))
			continue;
		    f = 0;
		    for(j=0; j<8; j++) {
			if(!etflag || (etflag && et == (E_BD + i)))
			    if((etdn < 0) || (j == etdn))
			    {
			      if(ra_hec[cn][j] || ra_sec[cn][j]) {
				if(f==0) {
				      printf("\n%s\n", ra_ctn[cn]);
				      f++;
				}
			      printf("\n UNIT %d\n", j);
			      printf("\n\tHard errors\t\t= %d", ra_hec[cn][j]);
			      printf("\n\tSoft errors\t\t= %d", ra_sec[cn][j]);
			      printf("\n\n");
			      }
			    }
		    }
		}
		continue;
	    }
	    f = 0;
	    if(i == HT_BMAJ)
	    	lim = 64;
	    else
	    	lim = 8;
	    for(j=0; j<lim; j++) {
	        if(!etflag || (etflag && et == (E_BD + i)))
	    	if((etdn < 0) || (j == etdn))
	    	{
	    	if(elc[E_BD+i].hec[j] || elc[E_BD+i].sec[j]) {
	    	if(f == 0) {
	    		printf("\n%s\n", dntab[elc[E_BD+i].edn]);
	    		f++;
	    		}
	    	printf("\n UNIT %d\n", j);
	    	printf("\n\tHard errors\t\t= %d",elc[E_BD+i].hec[j]);
	    	printf("\n\tSoft errors\t\t= %d",elc[E_BD+i].sec[j]);
	    	if(i == HP_BMAJ)	/* hp ECC vs retry counts */
	    	{
	    	printf("\n\t(ECC   recovered)\t= %d",hp_ecce[j]);
	    	printf("\n\t(RETRY recovered)\t= %d",hp_rtce[j]);
	    	}
	    	if(i == HM_BMAJ)	/* hm ECC vs retry counts */
	    	{
	    	printf("\n\t(ECC   recovered)\t= %d",hm_ecce[j]);
	    	printf("\n\t(RETRY recovered)\t= %d",hm_rtce[j]);
	    	}
	    	if(i == HJ_BMAJ)	/* hj ECC vs retry counts */
	    	{
	    	printf("\n\t(ECC   recovered)\t= %d",hj_ecce[j]);
	    	printf("\n\t(RETRY recovered)\t= %d",hj_rtce[j]);
	    	}
	    	if(i == HK_BMAJ)	/* hk ECC vs retry counts */
	    	{
	    	printf("\n\t(ECC   recovered)\t= %d",hk_ecce[j]);
	    	printf("\n\t(RETRY recovered)\t= %d",hk_rtce[j]);
	    	}
	    	printf("\n\n");
	    	}
	        }
	    }
	}
}

/* return 1 if time1 is >= time2 */

gte(t1,t2)
int *t1, *t2;
{
	register int q;

	for(q=5; q >= 0; q--) {
		if(*(t1 + q) < *(t2 + q))
			return(0);
		else if(*(t1 + q) > *(t2 + q))
			return(1);
			}
	return(1);
}

/* return 1 if time1 is <= time2 */

lte(t1, t2)
int *t1, *t2;
{
	register int q;

	for(q=5; q >= 0; q--) {
		if(*(t1 + q) > *(t2 + q))
			return(0);
		else if(*(t1 + q) < *(t2 + q))
			return(1);
			}
	return(1);
}

/*
 * Setup the MSCP controller type name strings
 * from the config word in the startup record.
 */

setrat(mct)
{
	register int i, j;

	for(i=0; i<MAXUDA; i++) {
		j = (mct >> (i * 4)) & 017;
		ra_cid[i] = j;	/* save cntlr ID */
		if(j == 017)
			ra_ctn[i] = "";
		else
			ra_ctn[i] = radntab[j];
	}
/*
 * Set the MSCP error type controller number
 * from the saved controller type.
 * etct - 0=ra, 1=rc, 2=rd, 3=rx
 */
	if(etct >= 0) {
		etcn = MAXUDA;
		for(i=0; i<MAXUDA; i++) {
			switch(etct) {
			case 0:
				if((ra_cid[i]==UDA50) || (ra_cid[i]==UDA50A) ||
/*				   (ra_cid[i]==KDA50) || (ra_cid[i]==KDA25)) */
				   (ra_cid[i]==KDA50))
					etcn = i;
				break;
			case 1:
				if(ra_cid[i] == KLESI)
					etcn = i;
				break;
			case 2:
				if((ra_cid[i] == RQDX1) || (ra_cid[i] == RQDX3))
					etcn = i;
				break;
			case 3:
				if((ra_cid[i] == RQDX1) ||
				   (ra_cid[i] == RQDX3) ||
				   (ra_cid[i] == RUX1))
					etcn = i;
				break;
			}
		}
	} else
		etcn = -1;
}
