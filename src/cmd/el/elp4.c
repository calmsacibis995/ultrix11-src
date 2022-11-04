
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)elp4.c	3.1	3/26/87";
/*
 * ULTRIX-11 error log report program (elp) - PART 4
 * Fred Canter 6/8/83
 *
 * The input for the report is taken from the current
 * error log or the optional [file].
 *
 * PART 4 does the following:
 *
 * 1.	The erfull() function formats and prints
 *	the full error report.
 *
 */

#include "elp.h"	/* must be first (param.h) */
#include <sys/tmscp.h>
#include <sys/ra_info.h>
#include <sys/errlog.h>
#include <sys/devmaj.h>
#include <stdio.h>

char	*rhcs1s[];
char	*hkcs1s[];
char	*tssrs[];
char	*tscoms[];
char	*hkfun[];
char	*rlfun[];
char	*rpfun[];
char	*rx2fun[];
char	*tscom1[];
char	*tscom8[];
char	*tscom9[];
char	*tscom10[];
char	*mshdr1[];
char	*mshdr2[];
char	*mlmrs[];
char	*mlees[];
char	*rlcss[];
char	*rpcss[];
char	*rx2css[];
char	*hkdss[];
char	*ra_erstat[];
char	*ra_efmc[];
char	*ra_nums[];

/*
 * Error log full report
 * Called by main() in PART 1 (elp1.c)
 */

int	setrat();	/* see elp2.c */

erfull(fi, filen)
{

	register struct el_rec *erp;
	char *ebp;
	register int i, j;
	int	k;
	int *rbp, *rgp;

	extern bd_rc[];
	char *p;
	daddr_t bn;
	int	lim, rn;
	int	l, num;
	int	eccerr;
	int	sn, dev;
	int	numa;
	int	rtc;
	int	etyp;
	int	maj, min, dn, dt, cn;
	int	bpcnt, rwflag;
	int	raflag, ra_emt, ra_et;	/* raflag = 1 (ra), = 2 (tk) */
	struct tmscp *mp;
	struct tmslg *msp;
	union {
		daddr_t	rabn;
		struct {
			int	rabn_l;
			int	rabn_h;
		} ra_str;
	} ra_lbn;

	bn = 0;
	sn = 0;
	bpcnt = 5;
	if(!fnflag)
		lseek(fi, (long)(el_sb * 512), 0);
	else
		lseek(fi, (long)0, 0);
eploop:
	if(!fnflag)
		if((bn + el_sb) >= (el_sb + el_nb)) {
			bn += el_sb;
			goto rderr;
			}
	if(read(fi, (char *)&buf, 512) != 512) {
	rderr:
		fprintf(stderr,"\nelp: read error bn = %D\n", bn);
		exit(1);
		}
	ebp = &buf;
	erp = ebp;
	rn = 0;

	while((ebp < &buf[256]) && ((etyp = erp->er_hdr.e_type) != E_EOB)) {

/*
 * Terminate the report if this is the bad record
 */

		if((badrn >= 0) && (bn == badbn) && (rn >= badrn))
			goto eplend;

/*
 * If the dflag ( date/time limits were specified by -d) is set
 * and the date/time of the error log record is out of
 * range, then do not print the record.
 */

	if(dflag) {
		if(etyp == E_EOF)
			goto eplend;
		timbuf = erp->er_hdr.e_time;
		if((timbuf < lotim) || (timbuf > hitim)) {
			if(etyp == E_SU)
				setrat(erp->er_body.su.e_mccw);	/* RA cntlr type */
			goto elnext;
		}
	}
	switch(etyp) {
	case E_EOF:		/* end of error log */
		goto eplend;
	case E_SU:		/* startup */
		setrat(erp->er_body.su.e_mccw);	/* RA controller type */
		if(etflag && (et != E_SU))
			break;
		if(!bflag)
			printf("\014");
		seq(sn);
		printf("%s", et_head[elc[E_SU].ehm]);
		pdt(erp->er_hdr.e_time);
		if(bflag)
			break;
		printf("\tSystem Profile:\n");
		i = erp->er_body.su.e_mmr3;
		j = ((i >> 8) & 017) + '0';
		l = ((i >> 12) & 017) + '0';
		if(i & 0100)
			k = 'X';
		else if(i & 0200)
			k = 'Y';
		else
			k = 'V';
		printf("\n\t\tULTRIX-11 (%c%c.%c) Operating System", k, l, j);
		printf("\n\t\t11/%d Processor", erp->er_body.su.e_cpu & 0377);
		if(i & 040)
			printf("\n\t\tUnibus Map Enabled");
		if(i & 020)
			printf("\n\t\t22 Bit Mapping Enabled");
		if(i & 04)
			printf("\n\t\tKernel D Space Enabled");
		if(i & 02)
			printf("\n\t\tSupervisor D Space Enabled");
		if(i & 01)
			printf("\n\t\tUser D Space Enabled");
		printf("\n\n\tConfigured with:\n");
		for(i=0; i<nblkdev; i++) {	/* block dev's */
		    if(erp->er_body.su.e_bdcw & (1 << i)) {
			k = sizeof(struct elrhdr) + sizeof(struct el_su);
			if((elc[E_BD+i].bmaj != RA_BMAJ) ||
			   (erp->er_hdr.e_size != k)) /* old SU record format */
				printf("\n\t\t%s",dntab[elc[E_BD+i].edn]);
			else {
				k = erp->er_body.su.e_mccw;
				for(l=0; l<MAXUDA; l++) {
					j = (k >> (l*4)) & 017;
					if(j != 017)
					    printf("\n\t\t%s", radntab[j]);
				}
			}
		    }
		}
		for(i=0; i<nchrdev; i++) {	/* char devices */
			if(elc[i+E_BD+nblkdev].et == NULL)
				break;	/* out of char dev's */
			if(erp->er_body.su.e_cdcw & (1 << i))
			printf("\n\t\t%s", dntab[elc[i+E_BD+nblkdev].edn]);
		}
		break;
	case E_SD:		/* shutdown */
		if(etflag && (et != E_SD))
			break;
		seq(sn);
		printf("%s", et_head[elc[E_SD].ehm]);
		pdt(erp->er_hdr.e_time);
		break;
	case E_TC:		/* time change */
		if(etflag && (et != E_TC))
			break;
		seq(sn);
		printf("TIME CHANGE ***** FROM ");
		timbuf = erp->er_hdr.e_time;
		printf("%s", ctime(&timbuf));
		timbuf = erp->er_body.tc.e_ntime;
		printf("\t\t\t\t   TO  %s\n", ctime(&timbuf));
		break;
	case E_SI:		/* stray interrupt */
		if(etflag && (et != E_SI))
			break;
		seq(sn);
		printf("%s", et_head[elc[E_SI].ehm]);
		pdt(erp->er_hdr.e_time);
		printf("\tFrom Controller at\t%o",erp->er_body.si.e_csr);
		if(bflag)
			break;
		/* special case for MSCP cntlr activity */
		k = sizeof(struct elrhdr) + sizeof(struct el_si);
		if(erp->er_hdr.e_size == k)
			j = erp->er_body.si.e_mcact;
		else
			j = 0;	/* for old format */
		pbda(erp->er_body.si.e_bdact,j);    /* block device activity */
		printf("\n");
		break;
	case E_SV:	/* stray vector */
		if(etflag && (et != E_SV))
			break;
		seq(sn);
		printf("%s", et_head[elc[E_SV].ehm]);
		pdt(erp->er_hdr.e_time);
		printf("\tFrom Vector Address\t%o",erp->er_body.sv.e_vectr);
		if(!bflag) {
		    /* special case for MSCP cntlr activity */
		    k = sizeof(struct elrhdr) + sizeof(struct el_sv);
		    if(erp->er_hdr.e_size == k)
		    	j = erp->er_body.sv.e_mcact;
		    else
		    	j = 0;	/* for old format */
		    pbda(erp->er_body.sv.e_bdact,j); /* block device activity */
		}
		printf("\n");
		break;
	case E_MP:		/* Memory parity error */
		if(etflag && (et != E_MP))
			break;
		seq(sn);
		printf("%s", et_head[elc[E_MP].ehm]);
		pdt(erp->er_hdr.e_time);
		if(!bflag)
		{
		if(erp->er_body.mp.e_nmsr == 0)
			break;	/* no reg.'s to print */
		printf("\n\tMemory System Register Contents\n");
		if(erp->er_body.mp.e_nmsr == 4) {
			printf("\n\tMLEA\t%o", erp->er_body.mp.e_mlea);
			printf("\n\tMHEA\t%o", erp->er_body.mp.e_mhea);
			}
		printf("\n\tMSER\t%o", erp->er_body.mp.e_mser);
		printf("\n\tMSCR\t%o", erp->er_body.mp.e_mscr);
		}
		printf("\n");
		break;
	case E_BD:		/* Block I/O device error */
		eccerr = 0;
		if(etflag && (et < E_BD))
			break;
		maj = erp->er_body.bd.bd_dev;
		min = maj & 0377;		/* real minor device number */
		maj >>= 8;			/* real major device number */
		cn = (min >> 6) & 3;	/* only used for MSCP disks */
		dn = min;	/* convert minor device to unit number */
				/* also set etyp equal to real error type */
		if(maj >= nblkdev) { /* RAW device */
			for(etyp=E_BD; etyp<(E_BD+nblkdev); etyp++)
				if(elc[etyp].rmaj == maj)
					break;
			if(maj == HT_RMAJ)
				dn &= 077;	/* unit number */
			else if(maj == RP_RMAJ
				|| maj == HP_RMAJ
				|| maj == HM_RMAJ
				|| maj == HJ_RMAJ
				|| maj == RL_RMAJ
				|| maj == RA_RMAJ
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
		if(etflag && (et != etyp))
			break;
		if((etcn >= 0) && (etcn != cn))
			break;
		if(etflag && (etdn >= 0) && (dn != etdn))
			break;
		rtc = erp->er_body.bd.bd_errcnt;	/* error retry count */
		if(rflag && ((rtc == 0) || (rtc > elc[etyp].rtc)))
			break;
		if(uflag && ((rtc != 0) && (rtc <= elc[etyp].rtc)))
			break;
		if(bflag) {
			if(++bpcnt > 5) {
				printf("\014");
				bpcnt = 1;
			}
		} else
			printf("\014");	/* form feed */
		if(strcmp("ra", elc[etyp].et) == 0)
			raflag = 1;	/* UDA50 error log record is special */
		else if(strcmp("tk", elc[etyp].et) == 0)
			raflag = 2;	/* TK50 error log record is special */
		else
			raflag = 0;
		seq(sn);
		printf("%s", et_head[elc[etyp].ehm]);
		pdt(erp->er_hdr.e_time);
/*
 * The UDA50 can log errors which do not have a
 * valid buffer header !
 * The buffer header flags word is set to 0100000 in this case.
 * Some of the buffer header information is filled in
 * by the driver when it logs the error.
 */
		printf("\tMajor/Minor Device\t%d/%d",maj ,min);
		printf("\n\tDevice Type\t\t");
		if(elc[etyp].bmaj == RA_BMAJ)
			printf("%s", ra_ctn[cn]);
		else
			printf("%s", dntab[elc[etyp].edn]);
		printf("\n\tUnit Number\t\t%d", dn);
		/*
		 * Print drive type for UDA50/RQDX1/RQDX3/RUX1/KLESI
		 * or TK50/TU81 if present.
		 */
		j = (erp->er_body.bd.bd_xmem >> 8) & 0377;
		if (raflag == 2) {	/* TK50 */
			switch(j) {
			case 50:
				printf(" (TK50)");
				break;
			case 81:
				printf(" (TU81)");
				break;
			default:
				break;
			}
		}
		else {
			switch(j) {
			case 25:
				printf(" (RC25)");
				break;
			case 33:
				printf(" (RX33)");
				break;
			case 50:
				printf(" (RX50)");
				break;
			case 31:
				printf(" (RD31)");
				break;
			case 32:
				printf(" (RD32)");
				break;
			case 51:
				printf(" (RD51)");
				break;
			case 52:
				printf(" (RD52)");
				break;
			case 53:
				printf(" (RD53)");
				break;
			case 54:
				printf(" (RD54)");
				break;
			case 60:
				printf(" (RA60)");
				break;
			case 80:
				printf(" (RA80)");
				break;
			case 81:
				printf(" (RA81)");
				break;
			default:
				break;
			}
		}
		if(erp->er_body.bd.bd_flags < 0)
			goto uda_s1;
	if(!bflag) {
		printf("\n\tDevice CSR Address\t%o", erp->er_body.bd.bd_csr);
		if(raflag == 0)
		  printf("\n\n\tRetry Count\t\t%d", erp->er_body.bd.bd_errcnt);
		}
		printf("\n\tError Diagnosis\t\t");
/*
 * If the retry count exceeds the device retry
 * count or is zero, then the error was fatal.
 * If the error was a DCK and
 * the retry count is less than the maximum,
 * the error was ECC recovered.
 * Otherwise the error was recovered via straight retry.
 *
 * For the UDA50, the retry count will be zero if the error
 * was fatal or two for a soft error.
 */
		if((rtc == 0) || (rtc > elc[etyp].rtc))
			printf("NOT RECOVERED");
		else {
			if(elc[etyp].bmaj == HP_BMAJ
			&& (erp->er_body.bd.bd_flags & E_READ)
			&& ((erp->devreg[6] & (DCK|ECH)) == DCK))
				eccerr++;
			if(elc[etyp].bmaj == HM_BMAJ
			&& (erp->er_body.bd.bd_flags & E_READ)
			&& ((erp->devreg[6] & (DCK|ECH)) == DCK))
				eccerr++;
			if(elc[etyp].bmaj == HJ_BMAJ
			&& (erp->er_body.bd.bd_flags & E_READ)
			&& ((erp->devreg[6] & (DCK|ECH)) == DCK))
				eccerr++;
			if(elc[etyp].bmaj == HK_BMAJ
			&& (erp->er_body.bd.bd_flags & E_READ)
			&& ((erp->devreg[6] & (DCK|ECH)) == DCK))
				eccerr++;
		if(eccerr)
			printf("RECOVERED via ECC");
		else
			printf("RECOVERED via RETRY");
		}
	uda_s1:
		if(bflag)
			goto bflagbn;
						/* block device activity */
		pbda(erp->er_body.bd.bd_act, erp->er_body.bd.bd_mcact);
		lim = erp->er_body.bd.bd_nreg;	/* number of device registers */
		if(raflag)	/* UDA50 gets special treatment */
			goto udapass;
		printf("\n\n\tDevice Register Contents at time of First Error\n");
		for(j=0; j<lim; j++) {
/*
 * If the reg name pointer (ernp) is 0,
 * use the drive type to select the correct
 * reg name text.
 */
			if(elc[etyp].ernp)
				p = *(elc[etyp].ernp + j);
			else {
				dt = erp->devreg[11] & 0377;
				switch(dt) {
				case RM02:
				case RM03:
				case RM05:
					p = rm_reg[j];
					break;
				case ML11A:
				case ML11B:
					p = ml_reg[j];
					break;
				default:
				case RP04:
				case RP05:
				case RP06:
					p = hp_reg[j];
					break;
				}
			}
			printf("\n\t%s\t\t", p);
			num = erp->devreg[j];
			printf("%06.o  ", erp->devreg[j]);
	/* print details of bits set in register */

	rbp = elc[etyp].erp;	/* reg. print control array pointer */
/*
 * If the reg print control array pointer is 0,
 * use the drive type to select the correct one.
 */
	if(rbp == 0)
		switch(dt) {
		case RM02:
		case RM03:
		case RM05:
			rbp = &rm_rbp;
			break;
		case ML11A:
		case ML11B:
			rbp = &ml_rbp;
			break;
		default:
		case RP04:
		case RP05:
		case RP06:
			rbp = &hp_rbp;
			break;
		}
	if(*(rbp + j)) {
		rbp = *(rbp + j);
		p = *rbp;
		num = erp->devreg[j];	/* device register contents */
		switch(*p) {		/* decide what to print */
	case 'a':	/* rhwc */
		printf("word count %d",num);
		break;
	case 'b':	/* rkda */
		printf("unit %d cyl %d ",((num>>13)&07),((num>>5)&0377));
		printf("surface %d sector %d",((num>>4)&01),(num&017));
		break;
	case 'c':	/* rpda */
	printf("tar %d sot %d sar %d",((num>>8)&037),((num>>4)&017),(num&017));
		break;
	case 'd':	/* rfda (NO LONGER USED !) */
/*		printf("track %d word %d",((num>>11)&037),(num&03777));  */
		break;
/*	case 'e':	/* hsda */
/*		printf("track %d sector %d",((num>>6)&077),(num&077));	*/
/*		break;	*/
	case 'f':	/* hpda */
		printf("track %d sector %d",((num>>8)&0377),(num&0377));
		break;
/*	case 'g':	/* hsdt */
/*
		if(num & 02)
			printf("RS04");
		else
			printf("RS03");
		if(num & 01)
			printf(" sector interleaved");
		break;
*/
	case 'h':	/* hpdt */
		if(num & 020000)
			printf("MOH ");
		if(num & 04000)
			printf("dual ported ");
	switch(num & 0377) {
	case RP04:
		printf("RP04");
		break;
	case RP05:
		printf("RP05");
		break;
	case RP06:
		printf("RP06");
		break;
	case RM03:
		printf("RM03");
		break;
	case RM02:
		printf("RM02");
		break;
	case RM05:
		printf("RM05");
		break;
	case ML11A:
		printf("ML11A");
		break;
	case ML11B:
		printf("ML11B");
		break;
	default:
		printf("unknown");
		break;
		}
	break;
	case 'i':	/* htdt */
		if(num & 02000)
			printf("SPR ");
		if(num & 040)
			printf("TM03 ");
		else
			printf("TM02 ");
		switch(num & 07) {
		case 0:
			printf("(unselected ");
			break;
		case 1:
			printf("(45 IPS ");
			break;
		case 2:
			printf("(75 IPS ");
			break;
		case 4:
			printf("(125 IPS ");
			break;
		}
		printf("slave)");
		break;
	case 'j':	/* rhsn */
		printf("serial number ");
		putchar(((num >> 12) & 017) + '0');
		putchar(((num >> 8) & 017) + '0');
		putchar(((num >> 4) & 017) + '0');
		putchar((num & 017) + '0');
		break;
	case 'k':	/* rhca */
		printf("cylinder %d",(num & 01777));
		break;
	case 'l':	/* hpcs1 */
			/* Print text for the set bits 15 -> 6 */
			/* Print the decoded function bits */
			/* Print the GO bit if set */

		for(l=0; l<10; l++) {
			if(num & (1 << (15 - l)))
				printf("%s ",rhcs1s[l]);
			}
		printf("<");
		switch((num >> 1) & 037) {
		case 0:
			printf("NOP");
			break;
		case 1:
			printf("UNLOAD");
			break;
		case 2:
			printf("SEEK");
			break;
		case 3:
			printf("RECAL");
			break;
		case 4:
			printf("DRIVE CLR");
			break;
		case 5:
			printf("D.P. REL");
			break;
		case 6:
			printf("OFFSET");
			break;
		case 7:
			printf("R.T.C.");
			break;
		case 8:
			printf("PRESET");
			break;
		case 9:
			printf("PACK ACK");
			break;
		case 12:
			printf("SEARCH");
			break;
		case 20:
			printf("WRT CK DATA");
			break;
		case 21:
			printf("WRT CK HDR & DATA");
			break;
		case 24:
			printf("WRT DATA");
			break;
		case 25:
			printf("WRT HDR & DATA");
			break;
		case 28:
			printf("READ DATA");
			break;
		case 29:
			printf("READ HDR & DATA");
			break;
		default:
			printf("ILLEGAL FUNCTION");
			break;
		}
		printf(">");
		if(num & 1)
			printf(" GO");
		break;
	case 'm':	/* hkcs1 */
			/* Print text for set bits 15 -> 11 */
			/* Print controller drive type */
			/* Print text for set bits 9 -> 6 */
			/* Print decoded function bits */
			/* Print GO bit if set */

		for(l=0; l<5; l++) {
			if(num & (1 << (15 - l)))
				printf("%s ",hkcs1s[l]);
			}
		if(num & 02000)
			printf("RK07 ");
		else
			printf("RK06 ");
		for(l=6; l<10; l++) {
			if(num & (1 << (15 - l)))
				printf("%s ",hkcs1s[l]);
			}
		l = ((num >> 1) & 017);
		printf("<");
		if(l > 12)
			printf("ILLEGAL FUNCTION");
		else
			printf("%s",hkfun[l]);
		printf(">");
		if(num & 1)
			printf(" GO");
		break;
/*	case 'n':	/* hscs1 */
			/* Print text for set bits 15 -> 6 */
			/* Print decoded function bits */
			/* Print GO if bit set */
/*
		for(l=0; l<10; l++) {
			if(num & (1 << (15 - l)))
				printf("%s ",rhcs1s[l]);
			}
		printf("<");
		switch((num >> 3) & 07) {
		case 0:
			printf("NOP");
			break;
		case 1:
			printf("DRIVE CLR");
			break;
		case 3:
			printf("SEARCH");
			break;
		case 5:
			printf("WRT CK");
			break;
		case 6:
			printf("WRITE");
			break;
		case 7:
			printf("READ");
			break;
		default:
			printf("ILLEGAL FUNCTION");
			break;
		}
		printf(">");
		if(num & 1)
			printf(" GO");
		break;
*/
	case 'o':	/* htcs1 */
			/* Print text for each set bit */
			/* Print decoded function bits */
			/* Print Go bit if set */
		for(l=0; l<10; l++) {
			if(num & (1 << (15 - l)))
				printf("%s ",rhcs1s[l]);
			}
		printf("<");
		switch((num >> 1) & 037) {
		case 0:
			printf("NOP");
			break;
		case 1:
			printf("REW OFF-LINE");
			break;
		case 3:
			printf("REWIND");
			break;
		case 4:
			printf("DRIVE CLR");
			break;
		case 10:
			printf("ERASE");
			break;
		case 11:
			printf("WRT TAPE MARK");
			break;
		case 12:
			printf("SPACE FWD");
			break;
		case 13:
			printf("SPACE REV");
			break;
		case 20:
			printf("WRT CK FWD");
			break;
		case 23:
			printf("WRT CK REV");
			break;
		case 24:
			printf("WRT FWD");
			break;
		case 28:
			printf("READ FWD");
			break;
		case 31:
			printf("READ REV");
			break;
		default:
			printf("ILLEGAL FUNCTION");
			break;
		}
		printf(">");
		if(num & 1)
			printf(" GO");
		break;
	case 'p':	/* tsba */
		printf("<Command packet address>");
		break;
	case 'q':	/* tssr */
			/* print bit set 15 -> 6 */
			/* FC, fatal class codes */
			/* TCC, termination class codes */
		for(l=0; l<10; l++) {
			if(num & (1 << (15 - l)))
				printf("%s ",tssrs[l]);
			}
		printf("<FC = %o TCC = %o>",((num>>4)&3),((num>>1)&7));
		break;
	case 'r':	/* tscom */
			/* print bits set 15 -> 12 & 7 */
			/* expand function bits */
		numa = num & 0170200;	/* remove func bits */
		for(l=0; l<9; l++) {
			if(numa & (1 << (15 - l)))
				printf("%s ",tscoms[l]);
			}
		numa = (num >> 8) & 7;	/* comm. mode bits */
		printf("<");
		switch(num & 017) {	/* comm code */
		case 1:	/* READ */
			printf("%s ",tscom1[numa]);
			break;
		case 4:	/* WRITE CHARACTERISTICS */
			if(numa)
				goto cmerr;
			else
				printf("Write Characteristics");
			break;
		case 5:	/* WRITE */
			if(numa == 0)
				printf("Write data (text)");
			else if(numa == 2)
			   printf("Write data retry (sp rev, erase, wrt data)");
			else
				goto cmerr;
			break;
		case 6:	/* WRITE SUBSYSTEM MEMORY */
			if(numa == 0)
				printf("Write Subsystem Memory");
			else
				goto cmerr;
			break;
		case 8:	/* POSITION */
			if(numa > 4)
				goto cmerr;
			else
				printf("%s ",tscom8[numa]);
			break;
		case 9:	/* FORMAT */
			if(numa > 2)
				goto cmerr;
			else
				printf("%s ",tscom9[numa]);
			break;
		case 10:	/* CONTROL */
			if(numa > 2)
				goto cmerr;
			else
				printf("%s ",tscom10[numa]);
			break;
		case 11:	/* INITIALIZE */
			if(numa)
				goto cmerr;
			else
				printf("Drive initialize");
			break;
		case 15:	/* GET STATUS IMMEDIATE */
			if(numa)
				goto cmerr;
			else
				printf("Get status (END message only)");
			break;
		cmerr:
		default:
			printf("UNKNOWN FUNCTION");
			break;
		}
		printf(">");
		break;
	case 's':	/* tsbc */
			/* initial byte count */
		printf("%u <Initial byte count>", num);
		break;
	case 't':	/* msbptr */
		printf("<Message buffer address lo>");
		break;
	case 'u':	/* msbae */
		printf("<Message buffer address hi>");
		break;
	case 'v':	/* mshdr */
			/* message buffer header word *
			/* ACK + class & message code expansion */
		if(num < 0)
			printf("ACK ");
		printf("<");
		numa = (num >> 8) & 3;	/* class code */
		switch(num & 017) {	/* message code */
		case 0:	/* END */
			printf("END");
			break;
		case 1:	/* FAIL */
			printf("FAIL ");
			if(numa > 3) {
			ccerr:
				printf("UNKNOWN CLASS CODE");
				break;
				}
			else
				printf("%s ",mshdr1[numa]);
			break;
		case 2:	/* ERROR */
			printf("ERROR");
			break;
		case 3:	/* ATTENTION */
			printf("ATTN ");
			if(numa > 1)
				goto ccerr;
			else
		printf("%s ",mshdr2[numa]);
			break;
		default:
			printf("UNKNOWN MESSAGE CODE");
			break;
		}
	printf(">");
	break;
	case 'w':	/* mssiz */
		printf("%u <Message size in bytes>", num);
		break;
	case 'x':	/* msresid */
		printf("%u <Residual byte count>", num);
		break;
	case 'y':	/* mlda */
		printf("Block number %u", num);
		break;
	case 'z':	/* mlmr */
		printf("%d ", (num >> 11) & 037);
		if(num & 02000)
			printf("64k");
		else
			printf("16k");
		printf(" arrays TRT = ");
		printf("%d us/word ",(1 << ((num >> 8) & 3)));
		for(l=0; l<8; l++) {	/* expand other bits */
			if(num & (1 << (7 - l)))
				printf("%s ",mlmrs[l]);
			}
		break;
	case '0':	/* mlee */
		for(l=0; l<3; l++) {	/* expand bits 15->13 */
			if(num & (1 << (15 - l)))
				printf("%s ", mlees[l]);
			}
		printf("Error channel %d ", (num >> 6) & 077);
		printf("function %d", (num & 077));
		break;
	case '1':	/* rlcs */
			/* print text for set bits 15 & 14 */
			/* print decoded error bits 13 -> 10 */
			/* print decoded unit number bits 9 & 8 */
			/* print text for set bits 7 -> 4 */
			/* print decoded function bits 3 -> 1 */
			/* print text for bit 0 */
		for(l=0; l<2; l++)
			if(num & (1 << (15 - l)))
				printf("%s ", rlcss[l]);
		l = ((num >> 10) & 017);
		switch(l) {
		case 0:
			break;
		case 1:
			printf("<OPI> ");
			break;
		case 2:
			printf("<RDCRC> ");
			break;
		case 3:
			printf("<HCRC> ");
			break;
		case 4:
			printf("<DLT> ");
			break;
		case 5:
			printf("<HNF> ");
			break;
		case 010:
			printf("<NXM> ");
			break;
		case 011:
			printf("<MPE> ");
			break;
		default:
			printf("<??> ");
			break;
		}
		printf("<unit %d> ", (num >> 8) & 3);
		for(l=8; l<12; l++)
			if(num & (1 << (15 - l)))
				printf("%s ", rlcss[l]);
		l = ((num >> 1) & 7);
		printf("<%s>", rlfun[l]);
		if(num & 1)
			printf(" DRDY");
		break;
	case '2':	/* rpcs */
			/* print text for set bits 15 -> 4 */
			/* print decoded function bits 3 -> 1 */
			/* print text for bit 0 */
		for(l=0; l<12; l++)
			if(num & (1 << (15 - l)))
				printf("%s ", rpcss[l]);
		l = ((num >> 1) & 7);
		printf("<%s>", rpfun[l]);
		if(num & 1)
			printf(" GO");
		break;
	case '3':	/* rx2cs */
			/* print text for set bits 15 -> 4 */
			/* print decoded function bits 3 -> 1 */
			/* print text for bit 0 */
		for(l=0; l<12; l++)
			if(num & (1 << (15 - l)))
				printf("%s ", rx2css[l]);
		l = ((num >> 1) & 7);
		printf("<%s>", rx2fun[l]);
		if(num & 1)
			printf(" GO");
		break;
	case '4':	/* hkds */
			/* print text for set bits 15 -> 9 */
			/* print drive type RK06 or RK07 */
			/* print text for set bits 7 -> 0 */
		for(l=0; l<7; l++)
			if(num & (1 << (15 - l)))
				printf("%s ", hkdss[l]);
		if(num & 0400)
			printf("RK07 ");
		else
			printf("RK06 ");
		for(l=8; l<16; l++)
			if(num & (1 << (15 - l)))
				printf("%s ", hkdss[l]);
		break;
	default:	/* print text for each set bit in reg. */
		for(l=0; l<16; l++) {
			if(num & (1 << (15 -l))) {
				printf("%s ",*(rbp + l));
					}
				}
		break;
		}
	}
			}

	udapass:
		if(erp->er_body.bd.bd_flags < 0) {
			printf("\n");
			goto uda_s2;
		}
		printf("\n\n\tPhysical Buffer Start Address\t");
		num = (erp->er_body.bd.bd_xmem & 077) << 1;
		if((int)erp->er_body.bd.bd_addr & 0100000)
			num++;
		printf("%o",num);
		num = (int)erp->er_body.bd.bd_addr & 077777;
		printf("%05.o",num);
		printf("\n\tUnibus Map used for transfer ?\t");
		if(erp->er_body.bd.bd_flags & E_MAP)
			printf("YES");
		else
			printf("NO");
	printf("\n\tTransfer size in bytes\t\t%u",erp->er_body.bd.bd_bcount);
bflagbn:
		printf("\n\tTransfer Type\t\t\t");
		if(erp->er_body.bd.bd_flags & E_READ)
			printf("READ");
		else
			printf("WRITE");
	printf("\n\tBlock in logical file system\t%D",erp->er_body.bd.bd_blkno);
	if(bflag)
		break;
		printf("\n\tI/O Operation type\t\t");
		if(erp->er_body.bd.bd_flags & E_PHYS)
			printf("PHYSICAL");
		else
			printf("BUFFERED");
		printf("\n\n");
	uda_s2:
		if(raflag) {	/* UDA50 - print message packet information */
			mp = &erp->devreg[0];
			if((mp->m_header.uda_credits & 0360) == 020) {
				ra_emt = 0;
				printf("\n\tError Log Packet ");
				printf("Contents:\n");
			} else {
				ra_emt = 1;
				printf("\tEnd Message Packet ");
				printf("Contents:\n");
			}
			for(j=0; j<lim; j++) {
				if((j&3) == 0)
					printf("\n\t\t");
				printf("%06.o  ", erp->devreg[j]);
			}
			printf("\n\n\tCommand Reference Number\t%o",
				mp->m_cmdref);
			printf("\n\tError Log Reference Number\t%o",
				mp->m_elref);
			printf("\n\tUnit Number\t\t\t%d",mp->m_unit);
			if(ra_emt == 0)
				goto uda_dg;
			rwflag = 0;
			printf("\n\tOperation Type\t\t\t");
			if((mp->m_opcode&0377) == (M_O_WRITE | M_O_END)) {
				printf("WRITE");
				rwflag = 1;
			}
			else if((mp->m_opcode&0377) == (M_O_READ | M_O_END)) {
				printf("READ");
				rwflag = 1;
			}
			else {
				if (raflag == 2) {
					if((mp->m_opcode&0377) == (M_O_REPOS | M_O_END)) {
						printf("REPOSITION");
						if (mp->m_lbn_l == 0)
							printf(" (REWIND)");
					}
					else if((mp->m_opcode&0377) == (M_O_WRITM | M_O_END))
						printf("WRITE TAPE MARK");
					else
						printf("????");
				}
				else
					printf("????");
			}
			printf("\n\tStatus Code\t\t\t");
			ra_scp(mp->m_status,raflag);	/* decode & print status code */
			if (rwflag)
				printf("\n\tReturned Byte Count\t\t%u", mp->m_bytecnt);
			printf("\n\tEnd Message Flags:");
			if((mp->m_flags&0377) == 0) {
				printf("\t\tNONE\n");
				break;
			}
			if(mp->m_flags&M_E_SEREX)
				printf("\n\n\t\t\t\t\tSerious exception");
			if(mp->m_flags&M_E_ERLOG) {
			    printf("\n\n\t\t\t\t\tError log generated");
			    printf("\n\t\t\t\t\t(Check for error log ");
			    printf("packets with same)\n\t\t\t\t\t");
			    printf("(command and error log reference numbers)");
			}
			if (raflag == 1) {
				if(mp->m_flags&M_E_BBLR) {
			    	ra_lbn.ra_str.rabn_h = mp->m_lbn_l;
			    	ra_lbn.ra_str.rabn_l = mp->m_lbn_h;
			    	printf("\n\n\t\t\t\t\tBad block reported");
			    	printf("\n\t\t\t\t\t(First bad block = %D)", ra_lbn.rabn);
				}
				if(mp->m_flags&M_E_BBLU)
					printf("\n\n\t\t\t\t\tBad blocks unreported");
			}
			else {
				if(mp->m_flags&M_E_EOT)
			    		printf("\n\n\t\t\t\t\tEOT encountered");
				if(mp->m_flags&M_E_PLS)
					printf("\n\n\t\t\t\t\tPosition lost");
			}
			printf("\n");
			break;
		uda_dg:		/* complete datagram printout */
			msp = mp;	/* error log message pointer */
			if((msp->me_flags & M_LF_SQNRS) == 0)
				printf("\n\tError Packet Sequence Number\t%d",
					msp->me_seqnum);
			printf("\n\tError Type\t\t\t");
			j = msp->me_format & 0377;
			if(raflag == 1 && j > 4 && j != 9)
				printf("Unknown");
			else if(raflag == 2 && ((j > 1 && j < 5) || j == 9))
				printf("Unknown");
			else
				printf("%s", ra_efmc[j]);
			printf("\n\tOperation Status\t\t");
			if(msp->me_flags & M_LF_SUCC)
				printf("Operation successful");
			else if(msp->me_flags & M_LF_CONT)
				printf("Operation continuing");
			else
				printf("Unrecoverable error");
			printf("\n\tEvent Code\t\t\t");
			ra_scp(msp->me_event,raflag);	/* decode & print event code */
			ra_et = msp->me_format & 0377;
			if(ra_et == M_F_BUSADDR) {
				ra_lbn.ra_str.rabn_l = msp->me_busaddr[1];
				ra_lbn.ra_str.rabn_h = msp->me_busaddr[0];
				printf("\n\tHost Memory Address\t\t%O",
					ra_lbn.rabn);
			}
			if (raflag == 2) {	/* TK50 */
				if(ra_et == M_F_TAPETRN) {
					i = msp->me_group;
					j = (i >> 8) & 0377;
					i &= 0377;
					printf("\n\tRetry (count/level)\t\t%d/%d",j,i);
				}
			}
			else {
				if(ra_et == M_F_DISKTRN) {
					i = msp->me_group;
					j = (i >> 8) & 0377;
					i &= 0377;
					printf("\n\tRetry (count/level)\t\t%d/%d",j,i);
				}
				if((ra_et == M_F_DISKTRN) ||
					(ra_et == M_F_SDI) ||
					(ra_et == M_F_SMLDSK)) {
					ra_lbn.ra_str.rabn_l = msp->me_volser[1];
					ra_lbn.ra_str.rabn_h = msp->me_volser[0];
					printf("\n\tVolume Serial Number\t\t%D",
						ra_lbn.rabn);
				}
				if((ra_et == M_F_DISKTRN) || (ra_et == M_F_SDI)) {
					ra_lbn.ra_str.rabn_l = msp->me_hdr[1] & 07777;
					ra_lbn.ra_str.rabn_h = msp->me_hdr[0];
					if((msp->me_hdr[1] & 0170000) == 060000)
						printf("\n\tReplacement Block Number\t");
					else
						printf("\n\tLogical Block Number\t\t");
					printf("%D", ra_lbn.rabn);
				}
				if(ra_et == M_F_SMLDSK) {
					printf("\n\tCylinder Number\t\t\t%d",
						msp->me_sdecyl);
				}
			}
		}
		break;
	default:
		fprintf(stderr,"\nelp: switch: unrecognized error type (this can't happen).\n");
		exit(1);
	}
elnext:
	sn++;
	ebp += erp->er_hdr.e_size;
	erp = ebp;
	rn++;
	}
	bn++;
	goto eploop;
eplend:
	close(fi);

	printf("\n\n\n\n\n\n\n\n");
}

pdt(time)
time_t time;
{


	printf(" - %s\n\n", ctime(&time));
}

seq(sn)
{
	printf("\n\nSequence %d\t", sn);
}

/*
 * Print the block device activity message 
 */

pbda(devact, mcact)
{

	register int l;

	printf("\n\tBlock Device Activity:");
	if(devact) {
		for(l=0; l<nblkdev; l++)
			if(devact & (1 << l))
				printf("\n\t\t\t\t%s",dntab[elc[E_BD+l].edn]);
	}
	if(mcact) {
		for(l=0; l<MAXUDA; l++)
			if(mcact & (1 << l))
				printf("\n\t\t\t\t%s", ra_ctn[l]);
	}
	if((devact == 0) && (mcact == 0))
		printf("\tNONE");
}

/*
 * Print UDA50 event subcodes
 */

ra_scp(code,flag)
{
	register int sc, cd;

	cd = code & M_S_MASK;
	if(cd == M_S_DIAG) {
		printf("Message from an internal diagnostic");
		return;
	} else if(cd == 017 || cd > 023) {
		printf("Unknown code");
		return;
	} else {
		if(cd == 05 && flag == 2) {
			printf("Unknown code");
			return;
		}
		if(cd > 013 && flag == 1) {
			printf("Unknown code");
			return;
		}
		printf("%s", ra_erstat[cd]);
	}
	sc = (code >> 5) & 03777;
	printf("\n\tSub-code\t\t\t");
	switch(code & M_S_MASK) {
	case M_S_SUCC:
		if (flag == 1) {
			switch(sc) {
			case 0:
				printf("Normal");
				break;
			case 1:
				printf("Spin-down ignored");
				break;
			case 2:
				printf("Still connected");
				break;
			case 4:
				printf("Duplicate unit number");
				break;
			case 8:
				printf("Already online");
				break;
			case 16:
				printf("Still online");
				break;
			case 32:
				printf("Incomplete Replacement");
				break;
			case 64:
				printf("Invalid RCT");
				break;
			default:
				goto ra_usc;
			}
		} else {
			switch(sc) {
			case 0:
				printf("Normal");
				break;
			case 8:
				printf("Already online");
				break;
			case 32:
				printf("EOT encountered");
				break;
			default:
				goto ra_usc;
			}
		}
		break;
	case M_S_ICMD:
		if(sc == 0)
			printf("Invalid message length");
		else {
/*
 * Refer to MSCP spec table B-2,
 * invalid command sub-codes.
 */
			printf("Field in error is offset %d bytes",
				((code & ~M_S_MASK)/256)+4);
			printf("\n\t\t\t\t\tfrom start of end message packet");
		}
		break;
	case M_S_ABRTD:
		goto ra_snu;
	case M_S_OFFLN:
		switch(sc) {
		case 0:
			printf("Unit unknown or\n\t\t\t\t\t");
			printf("online to another controller");
			break;
		case 1:
			printf("No volume mounted or\n\t\t\t\t\t");
			printf("drive disabled via RUN/STOP switch");
			break;
		case 2:
			printf("Unit is inoperative");
			break;
		case 4:
			if (flag == 2)
				goto ra_usc;
			printf("Duplicate unit number");
			break;
		case 8:
			if (flag == 2)
				goto ra_usc;
			printf("Unit disabled by field service or");
			printf("\n\t\t\t\t\tinternal diagnostic");
			break;
		default:
			goto ra_usc;
		}
		break;
	case M_S_AVLBL:
		goto ra_snu;
	case M_S_MFMTE:
		/* sub-codes shared with DATA ERROR */
		switch(sc) {
		case 0:
		case 2:
		case 3:
		case 7:
		      printf("\"Data Error\" accessing RCT or FCT\n\t\t\t\t\t");
		      break;
		default:
		      break;
		}
		switch(sc) {
		case 0:
			printf("Sector was written with \"Force Error\" modifier");
			break;
		case 2:
			printf("Invalid header");
			break;
		case 3:
			printf("Data Sync not found (Data Sync timeout)");
			break;
		case 5:
			printf("Disk isn't formatted with 512 byte sectors");
			break;
		case 6:
			printf("Disk isn't formatted or FCT corrupted");
			break;
		case 7:
			printf("Uncorrectable ECC Error");
			break;
		case 8:
			printf("RCT corrupted");
			break;
		default:
			goto ra_usc;
		}
		break;
	case M_S_WRTPR:
		if(sc == 256)
			printf("Unit is hardware write protected");
		else if(sc == 128)
			printf("Unit is software write protected");
		else if(sc == 8)
			printf("Unit is Data Safety Write Protected");
		else
			goto ra_usc;
		break;
	case M_S_COMP:
		goto ra_snu;
	case M_S_DATA:
		if (flag == 1) {
			switch(sc) {
			case 0:
				printf("Sector was written with \"Force Error\" modifier");
				break;
			case 2:
				printf("Invalid header");
				break;
			case 3:
				printf("Data sync not found (data sync timeout)");
				break;
			case 4:
				printf("Correctable error in ECC field");
				break;
			case 7:
				printf("Uncorrectable ECC error");
				break;
			case 8:
			case 9:
			case 10:
			case 11:
			case 12:
			case 13:
			case 14:
			case 15:
				printf("%s symbol ECC error", ra_nums[(sc-8)&7]);
				break;
			default:
				goto ra_usc;
			}
		}
		else {
			switch(sc) {
			case 0:
				printf("Long gap encountered");
				break;
			case 1:
				printf("Data sync not found");
				break;
			case 2:
				printf("Write lost data error");
				break;
			case 3:
				printf("Read data error");
				break;
			case 7:
				printf("Unrecoverable read error");
				break;
			default:
				goto ra_usc;
			}
		}
		break;
	case M_S_HSTBF:
		switch(sc) {
		case 0:
			printf("Host buffer access error, cause not available");
			break;
		case 1:
			if (flag == 2)
				goto ra_usc;
			printf("Odd transfer address");
			break;
		case 2:
			if (flag == 2)
				goto ra_usc;
			printf("Odd byte count");
			break;
		case 3:
			printf("Non-existent memory error");
			break;
		case 4:
			printf("Host memory parity error");
			break;
		case 5:
			if (flag == 1)
				goto ra_usc;
			printf("Invalid page table entry");
			break;
		default:
			goto ra_usc;
		}
		break;
	case M_S_CNTLR:
		if (flag == 1) {
			switch(sc) {
			case 1:
				printf("SERDES overrun or underrun error");
				break;
			case 2:
				printf("EDC error");
				break;
			case 3:
				printf("Inconsistent internal control structure");
				break;
			case 4:
				printf("Internal EDC error");
				break;
			case 8:
				printf("Controller overrun or underrun");
				break;
			case 9:
				printf("Controller memory error");
				break;
			default:
				goto ra_usc;
			}
		} else {
			switch(sc) {
			case 1:
				printf("Communications channel timeout");
				break;
			case 3:
				printf("Internal inconsistency");
				break;
			default:
				goto ra_usc;
			}
		}
		break;
	case M_S_DRIVE:
		if (flag == 1) {
			switch(sc) {
			case 1:
				printf("Drive command time out");
				break;
			case 2:
				printf("Controller detected transmission error");
				break;
			case 3:
				printf("Positioner error (mis-seek)");
				break;
			case 4:
				printf("Lost read/write ready during or");
				printf("\n\t\t\t\t\tbetween transfers");
				break;
			case 5:
				printf("Drive clock dropout");
				break;
			case 6:
				printf("Lost receiver ready for transfer");
				break;
			case 7:
				printf("Drive detected error");
				break;
			case 8:
				printf("Controller detected pulse or");
				printf("\n\t\t\t\t\tstate parity error");
				break;
			case 10:
				printf("Controller detected protocol error");
				break;
			case 11:
				printf("Drive failed initialization");
				break;
			case 12:
				printf("Drive ingored initialization");
				break;
			case 13:
				printf("Receiver Ready collision");
				break;
			default:
				goto ra_usc;
			}
		} else {
			switch(sc) {
			case 1:
				printf("Drive command time out");
				break;
			case 2:
				printf("Controller detected transmission error");
				break;
			case 3:
				printf("Recoverable drive fault");
				break;
			case 4:
				printf("Unrecoverable drive fault");
				break;
			default:
				goto ra_usc;
			}
		}
		break;
	case M_S_BBR:
		switch(sc) {
		case 0:
			printf("Bad block successfully replaced");
			break;
		case 1:
			printf("Block verified OK -- not a bad block");
			break;
		case 2:
			printf("Replacement failure -- REPLACE command");
			printf("\n\t\t\t\t\tor its analogue failed");
			break;
		case 3:
			printf("Replacement failure -- Inconsistent RCT");
			break;
		case 4:
		    printf("Replacement failure -- Drive access failure");
		    printf("\n\t\t\t\t\tOne or more transfers specified by the");
		    printf("\n\t\t\t\t\treplacement algorithm failed");
			break;
		default:
			goto ra_usc;
		}
		break;
	case M_S_FMTER:
	case M_S_BOT:
	case M_S_TAPEM:
	case M_S_RDTRN:
	case M_S_PLOST:
	case M_S_SEX:
	case M_S_LED:
	default:
		return;
	}
	return;
ra_snu:
	printf("This code has no sub-codes");
	return;
ra_usc:
	printf("Unknown sub-code < %d >", sc);
	return;
}
