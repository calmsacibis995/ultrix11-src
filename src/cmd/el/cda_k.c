
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * Cda -k option
 * 
 * Chung-Wu Lee 2/11/85
 *
 */
#include <sys/param.h>	/* Don't matter which one */
#include <sys/errlog.h>
#include <sys/tk_info.h>
#include <a.out.h>
#include "cda.h"

static char Sccsid[] = "@(#)cda_k.c	3.0	4/21/86";

extern int mem;
extern int flags;
extern struct nlist nl[];

struct tk_softc *tksc;
struct tk *tkp;
struct tk_drv *drvp;
char tk_ctid[MAXTK];
int tk_csr[MAXTK];
int ubmaps; 	/* unibus map: 1 == yes ; 0 == no */
int ntk;
char *ubm_off;	/* offset for mapped kernel data structures */

kcmd(arg)	/* unit + 1, 0 means all drives */
int	arg;
{
	int i, unit;
	
	if(!(flags & TKINFO))
		tkinfo();

	if (ntk <= 0) {
		printf("\ncda: kernel not configured for TMSCP magtapes !\n");
		exit(1);
	}
	if(arg > ntk) {
		printf("\ncda: Unit %d does not exist!\n",(arg - 1));
		printf("     will print data for all unit\n");
		unit = 0;
	} else
		unit = arg;
	if(unit) { 
		unit--;
		if(tk_csr[unit] == 0)
			printf("\ncda: Unit %d is not configured!\n", unit);
		else
			tkprnt(unit);
	} else
		for(i = 0; i < ntk; i++)
			tkprnt(i);
}

/*
 * get tmscp magtape info
 */

tkinfo()
{
	int i;

	if(flags & TKINFO)
		return;
	flags |= TKINFO;
	if (nl[28].n_value == 0) {
		ntk = 0;
		return;
	}
	lseek(mem, (long)nl[28].n_value, 0);
	if(read(mem, &ntk, sizeof(ntk)) == -1)
		tk_re();
	if (ntk <= 0)
		return;
	lseek(mem, (long)nl[36].n_value, 0);
	if(read(mem, &tk_csr, sizeof(tk_csr)) == -1)
		tk_re();
	tksc = calloc(ntk, sizeof(struct tk_softc));
	if(tksc == NULL)
		tk_me();
	lseek(mem, (long)nl[29].n_value, 0);
	i = sizeof(struct tk_softc) * ntk;
	if(read(mem, (char *)tksc, i) == -1)
		tk_re();
	tkp = calloc(ntk, sizeof(struct tk));
	if(tkp == NULL)
		tk_me();
	lseek(mem, (long)nl[30].n_value, 0);
	i = sizeof(struct tk) * ntk;
	if(read(mem, (char *)tkp, i) == -1)
		tk_re();
	drvp = calloc(ntk, sizeof(struct tk_drv));
	if(drvp == NULL)
		tk_me();
	lseek(mem, (long)nl[31].n_value, 0);
	i = sizeof(struct tk_drv) * ntk;
	if(read(mem, (char *)drvp, i) == -1)
		tk_re();
	lseek(mem, (long)nl[32].n_value, 0);
	if(read(mem, &tk_ctid, sizeof(tk_ctid)) == -1)
		tk_re();
	lseek(mem, (long)nl[27].n_value, 0);
	if(read(mem, &ubmaps, sizeof(ubmaps)) == -1)
		tk_re();
	if(ubmaps == 1)
		ubm_off = nl[25].n_value;
	else
		ubm_off = 0;
}

tk_me()
{
	printf("cda: read error, exiting...\n");
	exit(1);
}

tk_re()
{
	printf("cda: read error, exiting...\n");
	exit(1);
}

tkprnt(unit)
{
	int j, k;
	int *ip;
	struct tk_softc *sp;
	struct tk *kp;
	struct tk_drv *vp;
	union {
		long	big;
		struct {
			int	lit1;
			int	lit2;
		} little
	} muck;

	if (tk_csr[unit] == 0)
		return;
	printf("\n\nUnit number %d:", unit);
	printf("\ttype = ");
	switch((tk_ctid[unit] >> 4) & 017) {
	case TK50:
		printf("TK50\n");
		break;
	case TU81:
		printf("TU81\n");
		break;
	default:
		printf("unknown\n");
		break;
	}
	sp = tksc + unit;
	kp = tkp + unit;
	vp = drvp + unit;
	printf("\nstate = %d credits = %d tcmax = %d ",sp->sc_state,sp->sc_credits,sp->sc_tcmax);
	printf("lastcmd = %d lastrsp = %d\n",sp->sc_lastcmd,sp->sc_lastrsp);
	printf("command queue transition interrupt flag = %d\n",kp->tk_ca.ca_cmdint);
	printf("response queue transition interrupt flag = %d\n",kp->tk_ca.ca_rspint);
	printf("\n********* command descriptors: \n\n");
	if(ubmaps == 1) {	/* unibus map machines */
		printf("%16s %13s %11s\n","","virtual","physical");
		printf("%16s %13s %11s\n","TK_INT   TK_OWN","address","address");
		printf("------------------------------------------------------\n");
		for(j = 0; j < NCMD; j++) {
			if(kp->tk_ca.ca_cmddsc[j].ch & TK_INT)
				printf("   1     ");
			else
				printf("   0     ");
			if(kp->tk_ca.ca_cmddsc[j].ch & TK_OWN)
				printf("   1       ");
			else
				printf("   0       ");
		   	muck.little.lit1 = (kp->tk_ca.ca_cmddsc[j].ch & ~0140000);
			muck.little.lit2 = kp->tk_ca.ca_cmddsc[j].cl;
			printf("%011O\t", muck.big);
		   	muck.little.lit1 = (kp->tk_ca.ca_cmddsc[j].ch & ~0140000);
			muck.little.lit2 = (kp->tk_ca.ca_cmddsc[j].cl + ubm_off);
			printf("%011O\n", muck.big);
		}
	} else {	/* non - unibus map machines */
		printf("%16s %13s\n","TK_INT   TK_OWN","address");
		printf("---------------------------------------------\n");
		for(j = 0; j < NCMD; j++) {
			if(kp->tk_ca.ca_cmddsc[j].ch & TK_INT)
				printf("   1     ");
			else
				printf("   0     ");
			if(kp->tk_ca.ca_cmddsc[j].ch & TK_OWN)
				printf("   1       ");
			else
				printf("   0       ");
		   	muck.little.lit1 = (kp->tk_ca.ca_cmddsc[j].ch & ~0140000);
			muck.little.lit2 = kp->tk_ca.ca_cmddsc[j].cl;
			printf("%011O\n", muck.big);
		}
	}
	printf("\n********* response descriptors: \n\n");
	if(ubmaps == 1) {	/* unibus map machines */
		printf("%16s %13s %11s\n","","virtual","physical");
		printf("%16s %13s %11s\n","TK_INT   TK_OWN","address","address");
		printf("------------------------------------------------------\n");
		for(j = 0; j < NRSP; j++) {
			if(kp->tk_ca.ca_rspdsc[j].rh & TK_INT)
				printf("   1     ");
			else
				printf("   0     ");
			if(kp->tk_ca.ca_rspdsc[j].rh & TK_OWN)
				printf("   1       ");
			else
				printf("   0       ");
		   	muck.little.lit1 = (kp->tk_ca.ca_rspdsc[j].rh & ~0140000);
			muck.little.lit2 = kp->tk_ca.ca_rspdsc[j].rl;
			printf("%011O\t", muck.big);
		   	muck.little.lit1 = (kp->tk_ca.ca_rspdsc[j].rh & ~0140000);
			muck.little.lit2 = (kp->tk_ca.ca_rspdsc[j].rl + ubm_off);
			printf("%011O\n", muck.big);
		}
	} else {	/* non - unibus map machines */
		printf("%16s %13s\n","TK_INT   TK_OWN","address");
		printf("---------------------------------------------\n");
		for(j = 0; j < NRSP; j++) {
			if(kp->tk_ca.ca_rspdsc[j].rh & TK_INT)
				printf("   1     ");
			else
				printf("   0     ");
			if(kp->tk_ca.ca_rspdsc[j].rh & TK_OWN)
				printf("   1       ");
			else
				printf("   0       ");
			muck.little.lit1 = (kp->tk_ca.ca_rspdsc[j].rh & ~0140000);
			muck.little.lit2 = kp->tk_ca.ca_rspdsc[j].rl;
			printf("%011O\n", muck.big);
		}
	}
	printf("\n********** Command packets:\n");
	for(j = 0; j < NCMD; j++) {
		printf("\nPacket #%d:\n",(j+1));
		ip = &kp->tk_cmd[j];
		for(k = 0; k < 6; k++,ip += 8) {
printf("%06o %06o %06o %06o ",*(ip),*(ip+1),*(ip+2),*(ip+3));
printf("%06o %06o %06o %06o \n",*(ip+4),*(ip+5),*(ip+6),*(ip+7));
		}
printf("%06o %06o %06o %06o ",*(ip),*(ip+1),*(ip+2),*(ip+3));
printf("%06o %06o %06o \n",*(ip+4),*(ip+5),*(ip+6));
	}
	printf("\n********** Response packets:\n");
	for(j = 0; j < NRSP; j++) {
		printf("\nPacket #%d:\n",(j+1));
		ip = &kp->tk_rsp[j];
		for(k = 0; k < 6; k++,ip += 8) {
printf("%06o %06o %06o %06o ",*(ip),*(ip+1),*(ip+2),*(ip+3));
printf("%06o %06o %06o %06o \n",*(ip+4),*(ip+5),*(ip+6),*(ip+7));
		}
printf("%06o %06o %06o %06o ",*(ip),*(ip+1),*(ip+2),*(ip+3));
printf("%06o %06o %06o \n",*(ip+4),*(ip+5),*(ip+6));
	}
/*
 * print tk_drv structures
 */
	printf("\n********* drive information\n\n");
	printf("drive type = ");
	switch((tk_ctid[unit] >> 4) & 017) {
	case TK50:
		printf("TK50");
		break;
	case TU81:
		printf("TU81");
		break;
	default:
		printf("unknown");
		break;
	}
	printf("\tstatus = ");
	if(vp->tk_online)
		printf("online");
	else
		printf("offline");
	printf("\n\n");
}
