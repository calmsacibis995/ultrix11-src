
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * Cda -u option
 * 
 * Bill Burns 6/84
 * Fred Canter 9/22/85
 *	Modify for changes in uda communications area layout.
 *
 */
#include <sys/param.h>
#include <sys/errlog.h>
#include <sys/ra_info.h>
#include <a.out.h>
#include "cda.h"

static char Sccsid[] = "@(#)cda_u.c	3.2	7/21/87";

extern int mem;
extern int flags;
extern struct nlist nl[];
extern int magic;	/* 0 == non-split I/D kernel; 1 == split I/D kernel */

struct uda_softc *udasc;
struct uda *udap;
char xnra[MAXUDA];
struct ra_drv *drvp;
int nra;
char ra_ctid[MAXUDA];
int nuda;
char	ra_rs[MAXUDA];	/* cmd/rsp packet ring size (per mscp cntlr) */
struct	mscp	*ra_rp;	/* pointer to response packets */
struct	mscp	*ra_cp;	/* pointer to command packets */
extern int ubmaps; 	/* unibus map: 1 == yes ; 0 == no */
char *ubm_off;	/* offset for mapped kernel data structures */
int npac;

ucmd(arg)
int arg;	/* controller number plus 1; zero means all controllers */
{
	int cn, dn;
	int i;
	int ctlr;
	
	if(!(flags & RAINFO))
		rainfo();
	if(!nuda) {
		printf("\ncda: kernel not configured for MSCP disks!\n");
		exit(1);
	}
	if(arg > nuda) {
		printf("\ncda: Controller %d does not exist!\n",(arg - 1));
		printf("     will print data for all controllers\n");
		ctlr = 0;
	} else
		ctlr = arg;
	if(ctlr) { 
		ctlr--;
		prnt(ctlr);
	} else
		for(i = 0; i < nuda; i++)
			prnt(i);
/*
 * for all drives, print ra_drv structures
 */
	cn = 0;
	dn = 0;
	printf("\n********* Drive information:\n\n");
	for(i = 0; i < nra; i++) {
		if(dn == xnra[cn]) {
			dn = 0;
			cn++;
		}
		printf("Controller %d, Drive %d\n",cn,dn);
		printf("drive type = ");
		switch(drvp->ra_dt) {
		case RC25:
			printf("RC25");
			break;
		case RX33:
			printf("RX33");
			break;
		case RX50:
			printf("RX50");
			break;
		case RD31:
			printf("RD31");
			break;
		case RD32:
			printf("RD32");
			break;
		case RD51:
			printf("RD51");
			break;
		case RD52:
			printf("RD52");
			break;
		case RD53:
			printf("RD53");
			break;
		case RD54:
			printf("RD54");
			break;
		case RA60:
			printf("RA60");
			break;
		case RA80:
			printf("RA80");
			break;
		case RA81:
			printf("RA81");
			break;
		default:
			printf("unknown");
			break;
		}
		printf("\tstatus = ");
		if(drvp->ra_online)
			printf("online \t");
		else
			printf("offline\t");
		printf("unit size = %-D\n\n", drvp->d_un.ra_dsize);
		drvp++;
		dn++;
	}
}

/*
 * get mscp disk info
 */

rainfo()
{
	register int uda_size;
	register int i;
	int	npkt;

	if(flags & RAINFO)
		return;
	flags |= RAINFO;
	lseek(mem, (long)nl[18].n_value, 0);
	read(mem, (char *)&nuda, sizeof(nuda));
	if(nuda == 0)
		return;
	lseek(mem, (long)nl[33].n_value, 0);
	if(read(mem, (char *)&ra_rs, MAXUDA) != MAXUDA)
		rai_re();
	for(npkt=0, i=0; i<nuda; i++)
		npkt += ra_rs[i];
	ra_rp = calloc(npkt, sizeof(struct mscp));
	if(ra_rp == NULL)
		rai_me();
	ra_cp = calloc(npkt, sizeof(struct mscp));
	if(ra_cp == NULL)
		rai_me();
	lseek(mem, (long)nl[34].n_value, 0);
	i = npkt * sizeof(struct mscp);
	if(read(mem, (char *)ra_rp, i) != i)
		rai_re();
	lseek(mem, (long)nl[35].n_value, 0);
	if(read(mem, (char *)ra_cp, i) != i)
		rai_re();
	udasc = calloc(nuda, sizeof(struct uda_softc));
	if(udasc == NULL)
		rai_me();
	lseek(mem, (long)nl[19].n_value, 0);
	i = sizeof(struct uda_softc) * nuda;
	if(read(mem, (char *)udasc, i) != i)
		rai_re();
	/*
	 * Determine size of UDA communications area (in memory)
	 * based on type of kernel. Ring size = MAX_RS for SID kernel,
	 * or MAX_RS/2 for NSID kernel.
	 */
	if(magic)
		uda_size = sizeof(struct uda);
	else
		uda_size = sizeof(struct uda) - (MAX_RS * sizeof(long));
	udap = calloc(nuda, sizeof(struct uda));
	if(udap == NULL)
		rai_me();
	lseek(mem, (long)nl[20].n_value, 0);
	for(i=0; i<nuda; i++)
		if(read(mem, (char *)&udap[i], uda_size) != uda_size)
			rai_re();
	lseek(mem, (long)nl[21].n_value, 0);
	if(read(mem, &xnra, sizeof(xnra)) != sizeof(xnra))
		rai_re();
	nra = (xnra[0] + xnra[1] + xnra[2] + xnra[3]);
	drvp = calloc(nra, sizeof(struct ra_drv));
	lseek(mem, (long)nl[22].n_value, 0);
	i = nra * sizeof(struct ra_drv);
	if(read(mem, (char *)drvp, i) != i)
		rai_re();
	lseek(mem, (long)nl[23].n_value, 0);
	if(read(mem, &ra_ctid, sizeof(ra_ctid)) != sizeof(ra_ctid))
		rai_re();
	/*lseek(mem, (long)nl[27].n_value, 0);
	if(read(mem, &ubmaps, sizeof(ubmaps)) != sizeof(ubmaps))
		rai_re();*/
	if(ubmaps == 1)
		ubm_off = nl[25].n_value;
	else
		ubm_off = 0;
}

rai_me()
{
	printf("\ncda: can't get memory for uda structures!\n");
	exit(1);
}

rai_re()
{
	printf("\ncda: memory read error!\n");
	exit(1);
}

prnt(ctrl)
int	ctrl;
{
	register struct uda_softc *sc;
	register struct uda *ud;
	int i, j, k, o;
	int *ip;
	union {
		long	big;
		struct {
			int	lit1;
			int	lit2;
		} little
	} muck;

	sc = (struct uda_softc *)udasc + ctrl;
	ud = (struct uda *)udap + ctrl;
	printf("\n\nController number %d:", ctrl);
	printf("\tmicrocode revision = %d",ra_ctid[ctrl]&017);
	printf("\ttype = ");
	switch((ra_ctid[ctrl] >> 4) & 017) {
	case UDA50:
		printf("UDA50\n");
		break;
	case KDA50:
		printf("KDA50\n");
		break;
/* NO KDA25 support -- Fred
	case KDA25:
		printf("KDA25\n");
		break;
*/
	case UDA50A:
		printf("UDA50A\n");
		break;
	case KLESI:
		printf("KLESI\n");
		break;
	case RQDX1:
		printf("RQDX1\n");
		break;
	case RQDX3:
		printf("RQDX3\n");
		break;
	case RUX1:
		printf("RUX1\n");
		break;
	default:
		printf("UNKNOWN\n");
		break;
	}
	printf("\nstate = %d credits = %d tcmax = %d ",sc->sc_state,sc->sc_credits,sc->sc_tcmax);
	printf("lastcmd = %d lastrsp = %d\n",sc->sc_lastcmd,sc->sc_lastrsp);
	printf("command queue transition interrupt flag = %d\n",ud->uda_ca.ca_cmdint);
	printf("response queue transition interrupt flag = %d\n",ud->uda_ca.ca_rspint);
	printf("\n********* command descriptors: \n\n");
	if(ubmaps == 1) {	/* unibus map machines */
		printf("%16s %13s %11s\n","","virtual","physical");
		printf("%16s %13s %11s\n","UDA_INT  UDA_OWN","address","address");
		printf("------------------------------------------------------\n");
		k = ra_rs[ctrl];
		for(j = 0; j < ra_rs[ctrl]; j++) {
			if(ud->uda_ca.ca_dscptr[j+k].rh & UDA_INT)
				printf("   1     ");
			else
				printf("   0     ");
			if(ud->uda_ca.ca_dscptr[j+k].rh & UDA_OWN)
				printf("   1       ");
			else
				printf("   0       ");
	   		muck.little.lit1 = (ud->uda_ca.ca_dscptr[j+k].rh & ~0140000);
		    muck.little.lit2 = ud->uda_ca.ca_dscptr[j+k].rl;
			printf("%011O\t", muck.big);
	   		muck.little.lit1 = (ud->uda_ca.ca_dscptr[j+k].rh & ~0140000);
		    	muck.little.lit2 = (ud->uda_ca.ca_dscptr[j+k].rl + ubm_off);
			printf("%011O\n", muck.big);
		}
	} else {	/* non - unibus map machines */
		printf("%16s %13s\n","UDA_INT  UDA_OWN","address");
		printf("---------------------------------------------\n");
		k = ra_rs[ctrl];
		for(j = 0; j < ra_rs[ctrl]; j++) {
			if(ud->uda_ca.ca_dscptr[j+k].rh & UDA_INT)
				printf("   1     ");
			else
				printf("   0     ");
			if(ud->uda_ca.ca_dscptr[j+k].rh & UDA_OWN)
				printf("   1       ");
			else
				printf("   0       ");
	   		muck.little.lit1 = (ud->uda_ca.ca_dscptr[j+k].rh & ~0140000);
			muck.little.lit2 = ud->uda_ca.ca_dscptr[j+k].rl;
			printf("%011O\n", muck.big);
		}
	}
	printf("\n********* response descriptors: \n\n");
	if(ubmaps == 1) {	/* unibus map machines */
		printf("%16s %13s %11s\n","","virtual","physical");
		printf("%16s %13s %11s\n","UDA_INT  UDA_OWN","address","address");
		printf("------------------------------------------------------\n");
		for(j = 0; j < ra_rs[ctrl]; j++) {
			if(ud->uda_ca.ca_dscptr[j].rh & UDA_INT)
				printf("   1     ");
			else
				printf("   0     ");
			if(ud->uda_ca.ca_dscptr[j].rh & UDA_OWN)
				printf("   1       ");
			else
				printf("   0       ");
	   		muck.little.lit1 = (ud->uda_ca.ca_dscptr[j].rh & ~0140000);
		    muck.little.lit2 = ud->uda_ca.ca_dscptr[j].rl;
			printf("%011O\t", muck.big);
	   		muck.little.lit1 = (ud->uda_ca.ca_dscptr[j].rh & ~0140000);
		    	muck.little.lit2 = (ud->uda_ca.ca_dscptr[j].rl + ubm_off);
			printf("%011O\n", muck.big);
		}
	} else {	/* non - unibus map machines */
		printf("%16s %13s\n","UDA_INT  UDA_OWN","address");
		printf("---------------------------------------------\n");
		for(j = 0; j < ra_rs[ctrl]; j++) {
			if(ud->uda_ca.ca_dscptr[j].rh & UDA_INT)
				printf("   1     ");
			else
				printf("   0     ");
			if(ud->uda_ca.ca_dscptr[j].rh & UDA_OWN)
				printf("   1       ");
			else
				printf("   0       ");
	   	muck.little.lit1 = (ud->uda_ca.ca_dscptr[j].rh & ~0140000);
	    	muck.little.lit2 = ud->uda_ca.ca_dscptr[j].rl;
			printf("%011O\n", muck.big);
		}
	}
	printf("\n********** Command packets:\n");
	for(o=0, i=0; i<ctrl; i++)
		o += ra_rs[i];
	for(j = 0; j < ra_rs[ctrl]; j++) {
		printf("\nPacket #%d:\n",(j+1));
		ip = &ra_cp[o+j];
		for(k = 0; k < 4; k++,ip += 8) {
	printf("%06o %06o %06o %06o ",*(ip),*(ip+1),*(ip+2),*(ip+3));
	printf("%06o %06o %06o %06o \n",*(ip+4),*(ip+5),*(ip+6),*(ip+7));
		}
	}
	printf("\n********** Response packets:\n");
	for(j = 0; j < ra_rs[ctrl]; j++) {
		printf("\nPacket #%d:\n",(j+1));
		ip = &ra_rp[o+j];
		for(k = 0; k < 4; k++,ip += 8) {
printf("%06o %06o %06o %06o ",*(ip),*(ip+1),*(ip+2),*(ip+3));
printf("%06o %06o %06o %06o \n",*(ip+4),*(ip+5),*(ip+6),*(ip+7));
		}
	}
}
