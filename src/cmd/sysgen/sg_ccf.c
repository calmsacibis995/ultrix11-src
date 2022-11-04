
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)sg_ccf.c	3.5	1/6/88";
/*
 * Create an ULTRIX-11 kernel configuration file.
 *
 * Fred Canter
 */

#include "sysgen.h"

int	buflag;
int	cpu;
int	mscp_cn;

char	*no_cds = "\n\7\7\7CRASH DUMP SUPPORT OMITTED: ";
char *p;
int i, j;
FILE	*cf;
int	cc, k, yn;
int	ct, dt, sd, cod, ctn;
int	sysdisk, syscdd;
int	sysdn;
int	sysdt;
int	sysrmd;
int	syssmd;
int	sysemd;
int	ovsys;
int	kfpsim;
int	shuff, mesg, sema, flock, maus;
int	network;
int	nudev;
int	rhn;
char	nrh[4];
int	nmtc;		/* number of magtape controllers configured */
int	nssd;
int	mapszval;	/* holds "mapsize" as a function of "nproc" */
int	nb, mb;
unsigned ms, dms;
int	cdi;
int	nomap;		/* omit UB map code if set */
int	generic;	/* generate a generic kernel (see mkconf) */
int	npty;		/* number of pseudo ttys */
long	ulimitval=1024;	/* value of ulimit is long, not int.  Notice that
			  the default is hardwired here, and that the table
			  syspar[i].spval default value (1024) is not used */

int	g_cfname(), g_cpu(), g_ggk(), g_gms(), g_gnb(), g_gubm(),
	g_dct(), g_mtc(), g_cdd(), g_lpc(), g_cmc(), g_pty(), g_ctc(), g_udev(),
	g_fpsim(), g_net(), p_wcf(), g_spar(), g_shff(), g_msg(), g_smp(),
	g_flck(), g_mus(), g_hz(), g_timz(), g_dst(), p_wcf2(), p_wcf_p();

/*
 * This is a list of functions that are to be called to do the
 * system configuration.  A return value of 1 means go on to the
 * next function, a return value of -1 means back up a question.
 * The lines with the PD comment means that those functions are
 * position dependent in the table, so don't split them apart.
 */

int	(*funcs[])() = {
	g_cfname,	/* get configuration name */
	g_cpu,		/* get CPU type */
/*PD*/	g_ggk,		/* check to see if we want a generic kernel */
/*PD*/	g_gms,		/* memory size of target CPU */
	g_gnb,		/* number of buffers */
/*PD*/	g_gubm,		/* ask if UNIBUS code should be omitted */
/*PD*/	g_dct,		/* init disk controller table */
	g_mtc,		/* get magtape controller type(s) */
/*PD*/	g_cdd,		/* get crash dump device */
/*PD*/	g_lpc,		/* see if line printer to be used */
	g_cmc,		/* communications devices */
	g_pty,		/* pseudo ttys */
	g_ctc,		/* see if CAT to be used */
	g_udev,		/* number of user devices */
	g_fpsim,	/* ask include kernel floating point simulator or not */
	g_net,		/* networking code */
/*PD*/	p_wcf,		/* write data to the `?.cf' configuration file */
/*PD*/	g_spar,		/* system parameters */
	g_shff,		/* kernel shuffle routine */
	g_msg,		/* ipc messages */
	g_smp,		/* ipc semaphores */
	g_flck,		/* record file locking */
	g_mus,		/* maus */
	g_hz,		/* get line frequency */
	g_timz,		/* get timezone */
	g_dst,		/* is there daylight savings */
	p_wcf2,		/* finish writing the `?.cf' configuration file */
	p_wcf_p,	/* write out the `?.cf_p' file */
	0
};

ccf()
{
	register int	(**fp)();

	for (fp = funcs; *fp; ) {
		fp += (**fp)();
		if (fp < funcs)
			break;
	}
}

g_cfname()
{
	printf("\nFor help, answer the question with ?<RETURN>");
	printf("\nTo backup to the previous question, type <CTRL/D>\n");
	p = cname();		/* get config name */
	if (p == 0)
		return(-1);
	if(access(p, 0) == 0) {	/* see if it already exists */
		printf("\nConfiguration file exists, overwrite it");
		if(yes(NO, NOHELP) != YES)
			return(-1);
	}
	if((cf = fopen(p, "w")) == NULL) {
		printf("\nCan't create %s file\n", p);
		return(-1);
	}
	return(1);
}
g_cpu()
{
	register int i;

	for (;;) {
		do {
			printf("\nProcessor type:\n\n( ");
			for(i=0; cputyp[i].p_nam; i++)
				if((cputyp[i].p_flag&LATENT) == 0)
					printf("%s ", cputyp[i].p_nam);
			printf(") < %d", cputype);
			if((cputype == 23) && (rn_ssr3 & 020))
				printf("+");
			printf(" > ? ");
		} while((cc = getline("sg_cpu")) < 0);
		if(cc == 0) {
			fclose(cf);
			return(-1);
		}
		if(cc == 1) {
			if((cputype == 23) && (rn_ssr3 & 020))
				sprintf(&line, "23+");
			else
				sprintf(&line, "%d", cputype);
		}
		for(i=0; cputyp[i].p_nam; i++)
			if(strcmp(line, cputyp[i].p_nam) == 0)
				break;
		if(cputyp[i].p_nam != 0)
			break;
		printf("\nProcessor type `%s' not supported!\n", line);
	}
	/* this gets set again later in p_wcf(), ho hum... */
	if(cputyp[i].p_sid == NSID)
		ovsys = 1;	/* say non split I/D overlay kernel */
	else
		ovsys = 0;
	cpu = i;
	/*
	 * If the target CPU is a Q-bus processor,
	 * increase the default number of allocs and mbufs
	 * for the networking. The DEQNA has 20 mbufs permantently
	 * allocated to its receive ring. If the interface is not
	 * a DEQNA, then the user must be smart enough to deal with this.
	 */
	if(cputyp[cpu].p_bus == QBUS) {
	    for(i=0; netpar[i].spname; i++) {
		if(strcmp("mbufs", netpar[i].spname) == 0) {
		    netpar[i].spv_ov = 80;
		    netpar[i].spv_id = 80;
		}
		if(strcmp("allocs", netpar[i].spname) == 0) {
		    netpar[i].spv_ov = 1750;
		    netpar[i].spv_id = 2250;
		}
	    }
	}
	return(1);
}
g_ggk()
{
	/*
	 * If the -g option was given, ask if
	 * a generic kernel is to be generated.
	 *
	 * This option is used only by UEG, so backing
	 * up via <CTRL/D> is not supported.
	 */
	generic = 0;
	if(gkopt) {
		printf("\nGenerate a generic kernel");
		if(yes(NO, NOHELP) == YES)
			generic = 1;
	}
	return(1);
}
g_gms()
{
	for(;;) {
		if(cputyp[cpu].p_typ == cputype)
			ms = realmem;
		else if(cputyp[cpu].p_flag&A22BIT)
			ms = 256*16;
		else
			ms = 248*16;
		do {
			printf("\nMemory size in K bytes (K = 1024) < %u > ? ",
					ms/16);
		} while((cc = getline("sg_memsz")) < 0);
		if(cc == 0)
			return(-2);
		if(cc == 1)
			return(1);
		if((cputyp[cpu].p_flag&A22BIT) && (cputyp[cpu].p_bus == QBUS))
			j = 4088;	/* 4MB - 8KB I/O page */
		else if(cputyp[cpu].p_flag&A22BIT)
			j = 3840;	/* 4MB - 256KB I/O page */
		else
			j = 248;	/* 256KB - 8KB I/O page */
		i = atoi(line);
		if((i >= 248) && (i <= j))
			break;
		printf("\nInvalid memory size: min = 248, max = %d!\n", j);
	}
	ms = (unsigned)(i * 16);
	return(1);
}
g_gnb()
{
	register int i;

	/*
	 * CAUTION: these values must match the code in
	 *	    /usr/sys/sas/boot.c.
	 */
	for(;;) {
		i = ms/16;
		if(i <= 256) {
			nb = 20;
			mb = 40;
		} else if (i < 384) {
			nb = 35;
			mb = 50;
		} else if (i < 512) {
			nb = 50;
			mb = 100;
		} else if(i < 768) {
			nb = 75;
			mb = 200;
		} else if(i < 1024) {
			nb = 100;
			mb = 250;
		} else {
			nb = 150;
			mb = 300;
		}
		if((cputyp[cpu].p_flag & UBMAP) && (mb > MAXNBUF)) {
			mb = MAXNBUF;
			if(nb > MAXNBUF)
				nb = MAXNBUF;
		}
		/*
		 * Non-split I/D CPUs can't have a large number of buffers.
		 * Limited memory size takes care of this on all but 23/24.
		 */
		if((cputyp[cpu].p_typ==23) || (cputyp[cpu].p_typ==24)) {
			if(nb > 35)
				nb = 35;
			if(mb > 100)
				mb = 100;
		}
		do {
			printf("\nI/O buffer cache size ");
			printf("(NBUF: min = 16, max = %d) < %d > ? ", mb, nb);
		} while((cc = getline("sg_nbuf")) < 0);
		if(cc == 0)
			return(-1);
		if(cc != 1)
			nb = atoi(line);
		if ((nb >= 16) && (nb <= mb))
			break;
		printf("\nInvalid number of buffers!\n");
	}
	for(i=0; syspar[i].spname; i++)
		if(strcmp("nbuf", syspar[i].spname) == 0)
			break;
	syspar[i].spval = nb;
	return(1);
}
g_gubm()
{
	for(;;) {
		nomap = 0;
		if(cputyp[cpu].p_flag & UBMAP)
			return(1); /* CPU has unibus map, must include code */
		printf("\nOmit unibus map support, to save kernel space");
		if((yn = yes(YES, HELP)) != -1)
			break;
		phelp("sg_nomap");
	}

	if(buflag) {
		printf("\n");
		return(-1);
	}
	if(yn == YES)
		nomap++;
	return(1);
}
	/* Get disk controller type */
g_dct()
{
	static int doingover = 0;
dct2:
	if (doingover)
	    printf("\n\7\7\7All disk controllers deleted, starting over!\n");
	else
		doingover = 1;
	printf("\n\7\7\7Please enter the system (ROOT) disk controller first.\n");
	for(i=0; i<(MAXDC+1); i++) {
		dcd[i].dctyp = 0;
		dcd[i].dccn = 0;
		dcd[i].dcsys = -1;
	}
	mscp_cn = 0;
	sysdisk = 0;
	rhn = 0;
	for(i=0; i<4; i++)
		nrh[i] = 0;
dct3:
	do {
		printf("\nDisk controller type:\n\n<");
		for(j=0; dctype[j].dcn; j++) {
			if(j == 8)
				printf(",\n ");
			printf(" %s", dctype[j].dcn);
		}
		printf(" > ? ");
	} while((cc = getline("sg_dc")) < 0);
	if(cc == 0) {
		doingover = 0;
		return(cputyp[cpu].p_flag & UBMAP ? -2 : -1);
	}
	if(cc == 1)	/* all disk controllers entered */
		goto sdsl;
	for(ctn=0; dctype[ctn].dcn; ctn++)
		if(strcmp(&line, dctype[ctn].dcn) == 0)
			break;
	if(dctype[ctn].dcn == 0) {
		printf("\n`%s' not supported!\n", line);
		goto dct3;
	}
/*
 * Warn user, disk does not support
 * Q22 bus, i.e., no RAW I/O.
 */
	if((dctype[ctn].dcflag & Q22WARN) &&
	   (cputyp[cpu].p_bus == QBUS) &&
	   (cputyp[cpu].p_flag & A22BIT))
		pmsg(wm_q22);
	if(dctype[ctn].dcflag & MSCP) {
		switch(mscp_cn) {
		case 0:
			printf("\nFirst");
			break;
		case 1:
			printf("\nSecond");
			break;
		case 2:
			printf("\nThird");
			break;
		default:
			goto dct4;
		}
		printf(" MSCP disk controller:\n");
	}
dct4:	/* If MASSBUS, tell for controller number */
	if(dctype[ctn].dcflag & MASSBUS) {
		if(nrh[0] && nrh[1] && nrh[2]) {
			printf("\nMaximum of 3 RH11/RH70 controllers allowed!\n");
			goto dct3;
		}
		switch(rhn) {
		case 0:
			printf("\nFirst");
			break;
		case 1:
			printf("\nSecond");
			break;
		case 2:
			printf("\nThird");
			break;
		}
		printf(" MASSBUS disk controller:\n");
		/* change name, CSR, VECTOR, according to cntlr number */
		for(i=0; ddtype[i].ddn; i++)
			if(ddtype[i].ddflag & MASSBUS)
				switch(rhn) {
				case 0:
					ddtype[i].ddun = "hp";
					if((ddtype[i].ddflag & FIXED) == 0) {
						ddtype[i].ddcsr = 0176700;
						ddtype[i].ddvec = 0254;
					}
					break;
				case 1:
					ddtype[i].ddun = "hm";
					if((ddtype[i].ddflag & FIXED) == 0) {
						ddtype[i].ddcsr = 0176300;
						ddtype[i].ddvec = 0150;
					}
					break;
				case 2:
					ddtype[i].ddun = "hj";
					if((ddtype[i].ddflag & FIXED) == 0) {
						ddtype[i].ddcsr = 0176400;
						ddtype[i].ddvec = 0204;
					}
					break;
				}
	}
	for(ct=0; dcd[ct].dctyp; ct++) {  /* find empty cont. decsriptor */
		if(((dctype[ctn].dcflag&MASSBUS) == 0) &&
		    (dcd[ct].dctyp == dctype[ctn].dct)) {
			printf("\n`%s' ", dctype[ctn].dcn);
	dc_ac:
			printf("already configured!\n");
			goto dct3;
		}
		if((dctype[ctn].dct == RUX1) && (dcd[ct].dctyp == RQDX1)) {
			printf("\nCan't have `rux1', `rqdx' ");
			goto dc_ac;
		}
		if((dctype[ctn].dct == RQDX1) && (dcd[ct].dctyp == RUX1)) {
			printf("\nCan't have `rqdx', `rux1' ");
			goto dc_ac;
		}
	}
	if(ct >= MAXDC) {
		printf("\nDisk controller limit is %d!\n", MAXDC);
		goto sdsl;
	}
	dcd[ct].dctyp = dctype[ctn].dct;	/* load cont. type ID */
	if(dctype[ctn].dcflag & MSCP)
		dcd[ct].dccn = mscp_cn++;	/* MSCP cntlr number */
	else if(dctype[ctn].dcflag & MASSBUS)
		dcd[ct].dccn = rhn;		/* RH cntlr number */
	else
		dcd[ct].dccn = 0;
dcdn:
	for(j=0; j<8; j++)
		dcd[ct].dcunit[j] = 0;
	dcd[ct].dcnd = 0;
	if(dcd[ct].dctyp == RL11)
		j = 4;
	else if(dcd[ct].dctyp == RX211)
		j = 2;
	else
		j = 8;
	for(i=0; i<j; i++) {
	dcdn1:
		do {
			printf("\nDrive %d type < ", i);
			for(cod=0; doc[cod].dtct; cod++)
				if(doc[cod].dtct == dcd[ct].dctyp)
					break;
			for(k=0; k<MAXDOC; k++) {
				if(doc[cod].dtdn[k] == 0)
					break;
				printf("%s ", ddtype[doc[cod].dtdn[k]].ddn);
			}
			printf("> ? ");
		} while((cc = getline("sg_ddt")) < 0);
		if(cc == 0) {
			dcd[ct].dctyp = 0;	/* clean out cntlr entry */
			dcd[ct].dccn = 0;
			dcd[ct].dcsys = -1;
			if(dctype[ctn].dcflag & MSCP)
				mscp_cn--;
			goto dct3;
		}
		if(cc == 1) {
			if(dcd[ct].dcnd)
				break;
			else {
			    printf("\nNo default, please enter drive type!\n");
			    goto dcdn1;
			}
		}
		for(dt=0; ddtype[dt].ddn; dt++)
			if(strcmp(&line, ddtype[dt].ddn) == 0)
				break;
		if(ddtype[dt].ddn == 0) {
			printf("\n`%s' not supported!\n", line);
			dt = dcd[ct].dcunit[(i-1)];	/* valid CSR pointer */
			goto dcdn1;
		}
		for(k=0; doc[cod].dtdn[k]; k++)
			if(dt == doc[cod].dtdn[k])
				goto dcdnok;
		printf("\n`%s' not allowed on %s controller!\n",
			ddtype[dt].ddn, dctype[ctn].dcn);
		goto dcdn1;
	dcdnok:
		dcd[ct].dcnd++;			/* count # of drives */
		dcd[ct].dcunit[i] = dt;		/* load drive type */
	}
	dt = dcd[ct].dcunit[0];	/* base name, csr, vect on type of unit 0 */
	if(dcd[ct].dctyp == RUX1)
		dcd[ct].dcname = "ru";		/* RUX1 special case */
	else
		dcd[ct].dcname = ddtype[dt].ddun;	/* drive unix name */
dcadd:
	do
		printf("\nCSR address <%o> ? ", ddtype[dt].ddcsr);
	while((cc = getline("sg_csr")) < 0);
	if(cc == 0)
		goto dcdn;
	if(cc == 1)
		dcd[ct].dcaddr = ddtype[dt].ddcsr;
	else
		dcd[ct].dcaddr = acon(&line);
	if(dcd[ct].dcaddr == -1)
		goto dcadd;
dcvec:
	do
		printf("\nVector address <%o> ? ", ddtype[dt].ddvec);
	while((cc = getline("sg_vec")) < 0);
	if(cc == 0)
		goto dcadd;
	if(cc == 1)
		dcd[ct].dcvect = ddtype[dt].ddvec;
	else
		dcd[ct].dcvect = acon(&line);
	if(dcd[ct].dcvect == -1)
		goto dcvec;
	if(sysdisk)		/* If the system disk has not been specified, */
		goto sd_done;	/* ask if this is the system disk ?	*/
dcsd:
	printf("\nIs the system disk on this controller");
	if((yn = yes(YES, HELP)) == -1) {
		phelp("sg_sd");
		goto dcsd;
	}
	if(buflag) {
		printf("\n");
		goto dcvec;
	}
	if(yn == YES) {
	dcsdun:
		if(dcd[ct].dctyp == KLESI)
			k = 1;
		else
			k = 0;
		do
			printf("\nSystem disk unit number <%d> ? ", k);
		while((cc = getline("sg_sdn")) < 0);
		if(cc == 0)
			goto dcsd;
		if(cc == 1)
			i = k;
		else {
			if((cc != 2) || (line[0] < '0') || (line[0] > '7'))
				goto dcsdun;	/* bad unit # */
			i = line[0] - '0';
		}
		sysdn = i;
		dt = dcd[ct].dcunit[i];	/* drive type */
		if(dt == 0) {
			printf("\nUnit %d not configured!\n", i);
			goto dcsdun;	/* drive not configured */
		}
		if(ddtype[dt].ddsdl == -1) {
		    printf("\n%s can't be the system disk!", ddtype[dt].ddn);
			goto dcsd;
		}
		sysdisk++;
		dcd[ct].dcsys = ddtype[dt].ddsdl;
	}
sd_done:
	if(dctype[ctn].dcflag & MASSBUS)
		nrh[rhn++]++;
	goto dct3;
	sdsl:
		if(sysdisk == 0) {
			printf("\nSystem disk not specified!\n");
			return(0);
		}
		for(ct=0; ct<MAXDC; ct++)	/* find system disk controller */
			if(dcd[ct].dcsys >= 0)
				break;
		dt = dcd[ct].dcunit[sysdn];	/* set system disk drive type */
		sysdt = dt;			/* save it for later (wcf:) */
		if(dt == ML11)
			goto sdsl1;
		printf("\nUse standard placement of root, swap, and error log");
		if((yn = yes(YES, HELP)) == -1) {
			phelp("sg_sl");
			goto sdsl;
		}
		if(buflag) {
			printf("\n");
			goto dct2;
		}
	sdsl1:
		k = dcd[ct].dcsys;
		sdfsl[k].rootdev = dcd[ct].dcname;
		sdfsl[k].pipedev = dcd[ct].dcname;
		sdfsl[k].swapdev = dcd[ct].dcname;
		sdfsl[k].eldev = dcd[ct].dcname;
		if(dt == ML11) {
			pmsg(wm_ml11);
			printf("\nReally use ML11 as the system disk");
			if(yes(NO, NOHELP) == YES)
				goto sdsl2;
			else
				goto dct2;
		}
		if(yn == YES)
			return(1);
	sdsl2:
		i = dcd[ct].dcsys;	/* save standard file system layout */
		dcd[ct].dcsys = 0;
		sdfsl[0].rootdev = dcd[ct].dcname;
	  printf("\n\n\07\07\07CHANGING PLACEMENT OF ROOT, SWAP, ERROR LOG!");
	  printf("\n\nPress <RETURN> to use the default value!");
	  printf("\nType `?' followed by <RETURN> for help!\n");
	sdroot:
		for(j=0; dctype[j].dcn; j++)	/* find controller name */
			if(dctype[j].dct == dcd[ct].dctyp)
				break;
		printf("\nROOT is on `%s' ", sdfsl[0].rootdev);
		printf("(%s-%s unit %d)\n",dctype[j].dcn,ddtype[dt].ddn,sysdn);
		sysrmd = (sysdn*ddtype[dt].ddnp)+(sdfsl[i].rootmd&7); /* root minor */
		do
			printf("\nROOT minor device number <%d> ? ", sysrmd);
		while((cc = getline("sg_md")) < 0);
		if(cc == 0) {
			dcd[ct].dcsys = k;
			goto sdsl;
		}
		if(cc == 1)
			sdfsl[0].rootmd = sysrmd;
		else
			sdfsl[0].rootmd = atoi(line);
	sdpipe:
		do
			printf("\nPIPE device <%s> ? ", sdfsl[i].pipedev);
		while((cc = getline("sg_dn")) < 0);
		if(cc == 0)
			goto sdroot;
		if(cc == 1)
			sdfsl[0].pipedev = sdfsl[i].pipedev;
		else
			sdfsl[0].pipedev = gdname(&line);
		if(sdfsl[0].pipedev == 0)
			goto sdpipe;
		do
			printf("\nPIPE minor device number <%d> ? ", sysrmd);
		while((cc = getline("sg_md")) < 0);
		if(cc == 0)
			goto sdpipe;
		if(cc == 1)
			sdfsl[0].pipemd = sysrmd;
		else
			sdfsl[0].pipemd = atoi(line);
	sdswap:
		nssd = 0;
		do
			printf("\nSWAP device <%s> ? ", sdfsl[i].swapdev);
		while((cc = getline("sg_dn")) < 0);
		if(cc == 0)
			goto sdpipe;
		if(cc == 1)
			sdfsl[0].swapdev = sdfsl[i].swapdev;
		else {
			sdfsl[0].swapdev = gdname(&line);
			nssd++;
		}
		if(sdfsl[0].swapdev == 0)
			goto sdswap;
		syssmd = (sdfsl[i].swapmd & 7) + (sysdn * ddtype[dt].ddnp);
		do
			printf("\nSWAP minor device number <%d> ? ", syssmd);
		while((cc = getline("sg_md")) < 0);
		if(cc == 0)
			goto sdswap;
		if(cc == 1)
			sdfsl[0].swapmd = syssmd;
		else {
			sdfsl[0].swapmd = atoi(line);
			nssd++;
		}
		do
		  printf("\nSWAP area start block number <%D> ? ",sdfsl[i].swaplo);
		while((cc = getline("sg_sb")) < 0);
		if(cc == 0)
			goto sdswap;
		if(cc == 1)
			sdfsl[0].swaplo = sdfsl[i].swaplo;
		else
			sdfsl[0].swaplo = atol(line);
		do
		  printf("\nSWAP area length in blocks <%d> ? ", sdfsl[i].nswap);
		while((cc = getline("sg_nb")) < 0);
		if(cc == 0)
			goto sdswap;
		if(cc == 1)
			sdfsl[0].nswap = sdfsl[i].nswap;
		else
			sdfsl[0].nswap = atoi(line);
		if(nssd) {	/* non standard swap device used */
			printf("\n\n\7\7\7WARNING - May need to remake");
			printf(" the file /dev/swap.");
			printf("\nRefer to ULTRIX-11 System Management ");
			printf("Guide, Section 2.5.5\n");
		}
	sderlog:
		do
			printf("\nERROR LOG device <%s> ? ", sdfsl[i].eldev);
		while((cc = getline("sg_dn")) < 0);
		if(cc == 0)
			goto sdswap;
		if(cc == 1)
			sdfsl[0].eldev = sdfsl[i].eldev;
		else
			sdfsl[0].eldev = gdname(&line);
		if(sdfsl[0].eldev == 0)
			goto sderlog;
		sysemd = (sdfsl[i].elmd & 7) + (sysdn * ddtype[dt].ddnp);
		do
		  printf("\nERROR LOG area minor device number <%d> ? ", sysemd);
		while((cc = getline("sg_md")) < 0);
		if(cc == 0)
			goto sderlog;
		if(cc == 1)
			sdfsl[0].elmd = sysemd;
		else
			sdfsl[0].elmd = atoi(line);
		do
	printf("\nERROR LOG area start block number <%D> ? ",sdfsl[i].elsb);
		while((cc = getline("sg_sb")) < 0);
		if(cc == 0)
			goto sderlog;
		if(cc == 1)
			sdfsl[0].elsb = sdfsl[i].elsb;
		else
			sdfsl[0].elsb = atol(line);
		do
	printf("\nERROR LOG area length in blocks <%d> ? ", sdfsl[i].elnb);
		while((cc = getline("sg_nb")) < 0);
		if(cc == 0)
			goto sderlog;
		if(cc == 1)
			sdfsl[0].elnb = sdfsl[i].elnb;
		else
			sdfsl[0].elnb = atoi(line);
	return(1);
}
g_mtc()
{
	static int doingover = 0;
	struct mttype *mtp;
	int	istsk, ntskc, tskcf;

#define	ISTS	01
#define	ISTK	02

mtc2:
	if (doingover)
		printf("\n\7\7\7All magtapes deleted, starting over!\n");
	else
		doingover = 1;
	for(i=0; mttype[i].mtct; i++) {
		mttype[i].mtnunit = 0;
		if(mttype[i].mtcd > 0)
			mttype[i].mtcd = 0;
	}
	for(i=0; mtts[i].mtct; i++) {
		mtts[i].mtnunit = -1;
		if(mtts[i].mtcd > 0)
			mtts[i].mtcd = 0;
	}
	for(i=0; mttk[i].mtct; i++) {
		mttk[i].mtnunit = -1;
		if(mttk[i].mtcd > 0)
			mttk[i].mtcd = 0;
	}
	nmtc = 0;
mtc3:
	do {
		printf("\nMagtape controller:\n\n< ");
		for(k=0; mttype[k].mtct; k++) {
			if(strcmp("tk25", mttype[k].mtct) == 0)
				continue;	/* no print tk25 */
			printf("%s ", mttype[k].mtct);
		}
		printf("> ? ");
	} while((cc = getline("sg_mtc")) < 0);
	if(cc == 0) {
		doingover = 0;
		return(-1);
	}
	if(cc == 1)
		return(1);
	for(ct=0; mttype[ct].mtct; ct++)
		if(strcmp(&line, mttype[ct].mtct) == 0)
			break;
	if(mttype[ct].mtct == 0) {
		printf("\n`%s' not supported!\n", line);
		goto mtc3;
	}
	istsk = 0;
	if (strcmp(mttype[ct].mtun,"ts") == 0) {
		istsk = ISTS;
		tskcf = 0;
	} else if (strcmp(mttype[ct].mtun,"tk") == 0) {
		istsk = ISTK;
		tskcf = 0;
	} else {
		for(j=0; mttype[j].mtct; j++) {
			if((strcmp(mttype[j].mtun, mttype[ct].mtun) == 0) &&
		  	(mttype[j].mtnunit != 0)) {
		   	printf("\n`%s' already configured, only one `%s' allowed!\n",
				mttype[j].mtct, mttype[j].mtun);
		  	goto mtc3;
			}
		}
	}
mtnu:
	if (istsk) {
		do
			printf("\nMagtape unit number < 0->3 > ? ");
		while((cc = getline("sg_mtu")) < 0);
		if(cc == 0)
			goto mtc3;
		else if (cc == 1) {
			if (tskcf == 0) {
				printf("\nNo default, please enter unit number!\n");
				goto mtnu;
			}
			goto mtc3;
		} else
			i = atoi(line);
		if (i < 0 || i > 3) {
			printf("\n'%s' unit number, 0 -> 3 only!\n", mttype[ct].mtun);
			goto mtnu;
		}
		if(istsk == ISTS)
			mtp = &mtts[0];
		else
			mtp = &mttk[0];
		mtp += i;
		if (mtp->mtnunit >= 0) {
			printf("\n'%s' unit number %d, configured already!\n",
				mttype[ct].mtun, i);
			goto mtnu;
		}
		ntskc = i;
		strcpy(mtp->mtct,mttype[ct].mtct);
	} else {
		do
			printf("\nNumber of magtape units <1> ? ");
		while((cc = getline("sg_mtn")) < 0);
		if(cc == 0)
			goto mtc2;
		if(cc == 1)
			i = 1;
		else
			i = atoi(line);
	}
	if(istsk == 0)
		mttype[ct].mtnunit = i;
	for(i=0, j=0; mttype[i].mtct; i++)
		if((strcmp("ht", mttype[i].mtun) == 0) && mttype[i].mtnunit)
			j++;
	if(j > 1) {
		printf("\nCan't have both tm02 and tm03!");
		goto mtc2;
	}
mtcsr:
	do
		printf("\nCSR address <%o> ? ", mttype[ct].mtcsr);
	while((cc = getline("sg_csr")) < 0);
	if(cc == 0)
		goto mtnu;
	if(cc != 1) {
		if((i = acon(line)) == -1)
			goto mtcsr;
		if(istsk)
			mtp->mtcsr = i;
		else
			mttype[ct].mtcsr = i;
	}
mtvec:
	do
		printf("\nVector address <%o> ? ", mttype[ct].mtvec);
	while((cc = getline("sg_vec")) < 0);
	if(cc == 0)
		goto mtcsr;
	if(cc != 1) {
		if((i = acon(line)) == -1)
			goto mtvec;
		if(istsk)
			mtp->mtvec = i;
		else
			mttype[ct].mtvec = i;
	}
	if (istsk) {
		mtp->mtnunit = ntskc;
		tskcf++;
	}
	if(++nmtc < 10) {		/* max of 10 magtapes allowed */
		if (istsk)
			goto mtnu;
		else
			goto mtc3;
	}
	return(1);
}
g_cdd()
{
	register int j;

	syscdd = -1;
	for(i=0; cdtab[i].cd_name; i++)	/* init crash dump table */
		cdtab[i].cd_flags &= ~(CD_SG);
	for(i=0; mttype[i].mtct; i++) {	/* scan for configured tapes */
		if(mttype[i].mtcd > 0)
			mttype[i].mtcd = 0;	/* clean out any left overs */
		if(mttype[i].mtnunit && (mttype[i].mtcd >= 0)) {
			for(j=0; cdtab[j].cd_name; j++) {
			    if((cdtab[j].cd_flags & CD_TAPE) == 0)
				continue;
			    if(strcmp(mttype[i].mtct, cdtab[j].cd_gtyp) == 0) {
				syscdd = 1;
				cdtab[j].cd_flags |= CD_SG;
			    }
			}
		}
	}
	if (mtts[0].mtnunit >= 0) {	/* scan for configured ts tapes */
		if(mtts[0].mtcd > 0)
			mtts[0].mtcd = 0;	/* clean out any left overs */
		for(j=0; cdtab[j].cd_name; j++) {
			if((cdtab[j].cd_flags & CD_TAPE) == 0)
				continue;
			if(strcmp(mtts[0].mtct, cdtab[j].cd_gtyp) == 0) {
				syscdd = 1;
				cdtab[j].cd_flags |= CD_SG;
			}
		}
	}
	if (mttk[0].mtnunit >= 0) {	/* scan for configured tk tapes */
		if(mttk[0].mtcd > 0)
			mttk[0].mtcd = 0;	/* clean out any left overs */
		for(j=0; cdtab[j].cd_name; j++) {
			if((cdtab[j].cd_flags & CD_TAPE) == 0)
				continue;
			if(strcmp(mttk[0].mtct, cdtab[j].cd_gtyp) == 0) {
				syscdd = 1;
				cdtab[j].cd_flags |= CD_SG;
			}
		}
	}
	for(ct=0; ct<MAXDC; ct++)	/* find system disk controller */
		if(dcd[ct].dcsys >= 0)
			break;
	for(i=0; cdtab[i].cd_name; i++) {
		if((cdtab[i].cd_flags & CD_DISK) == 0)
			continue;
		if((cdtab[i].cd_flags & CD_RX50) == 0)
			cdtab[i].cd_unit = sysdn;
		else {
			for(j=1; j<4; j++)
			    if((dcd[ct].dcunit[j] == RX50) ||
			       (dcd[ct].dcunit[j] == RX33)) {
				    cdtab[i].cd_unit = j;
				    break;
			    }
		}
		if((strcmp(dcd[ct].dcname, cdtab[i].cd_name) == 0) &&
		   (cdtab[i].cd_dtyp == dcd[ct].dcunit[cdtab[i].cd_unit])) {
			syscdd = 1;
			cdtab[i].cd_flags |= CD_SG;
		}
	}
	if(syscdd < 0) {
		printf("%s", no_cds);
		printf("no dump device available!\n");
		return(1);
	}
cdd1:
	syscdd = -1;
	do {
		j = 0;
		printf("\nCrash dump device < ");
			for(i=0; cdtab[i].cd_name; i++)
				if(cdtab[i].cd_flags & CD_SG) {
					printf("%s ", cdtab[i].cd_gtyp);
					cdi = i;
					j++;
				}
		if(j == 1)
			i = cdi;
		printf("> ? ");
	} while((cc = getline("sg_cdd")) < 0);
	if(cc == 0)
		return(-1);
	if((cc == 1) && (j == 1))
		goto cdd2;	/* typed return, use default */
	if(cc == 1) {
		printf("\nNo default, please enter crash dump device!\n");
		goto cdd1;
	}
	for(i=0; cdtab[i].cd_name; i++)
		if(strcmp(line, cdtab[i].cd_gtyp) == 0)
			break;
	if(cdtab[i].cd_name == 0) {
		printf("\n`%s' - can't be crash dump device!\n", line);
		goto cdd1;
	}
	if((cdtab[i].cd_flags & CD_SG) == 0) {
		printf("\n`%s' - not configured into kernel!\n", line);
		goto cdd1;
	}
cdd2:
	if((dcd[ct].dcsys == 0) &&
	   ((cdtab[i].cd_flags & CD_TAPE) == 0) &&
	   ((cdtab[i].cd_flags & CD_RX50) == 0)) {
		printf("%s%s%s%s", no_cds,
			"non-standard placements used!",
			"\n(must use magtape or RQDX/RX50 ",
			"as crash dump device)\n");
		return(1);
	}
	if(cdtab[i].cd_flags & CD_TAPE) {
		if (mtts[0].mtnunit >= 0 && strcmp(cdtab[i].cd_name, mtts[0].mtun) == 0)
			mtts[0].mtcd = 1;
		else if (mttk[0].mtnunit >= 0 && strcmp(cdtab[i].cd_name, mttk[0].mtun) == 0)
			mttk[0].mtcd = 1;
		else {
			for(j=0; mttype[j].mtct; j++)
				if(mttype[j].mtnunit &&
			   	(strcmp(cdtab[i].cd_gtyp, mttype[j].mtct) == 0))
					mttype[j].mtcd = 1;
		}
	}
	if(cdtab[i].cd_flags & CD_RX50) {
	    /* will always find an RX33 or RX50 */
	    for(j=1; j<4; j++)
		if((dcd[ct].dcunit[j] == RX50) || (dcd[ct].dcunit[j] == RX33))
		    break;
	    cdtab[i].cd_unit = j;
	    printf("\nCrash dump will be to floppy unit %d.\n", j);
	}
	syscdd = i;	/* save index to crash dump device info */
	return(1);
}
g_lpc()
{
top:
	printf("\nLP11/LPV11 line printer present");
	while((yn = yes(NO, NOHELP)) < 0);
	if(buflag) {
		printf("\n");
		if(syscdd >= 0)
			return(-1);
		else
			return(-2);
	}
	lptype[0].lpused = 0;
	if(yn == YES) {
		lptype[0].lpused = 1;
	lpcsr:
		do
		    printf("\nCSR address <%o> ? ", lptype[0].lpcsr);
		while((cc = getline("sg_csr")) < 0);
		if(cc == 0)
			goto top;
		if(cc != 1) {
			if((i = acon(line)) == -1)
				goto lpcsr;
			lptype[0].lpcsr = i;
		}
		if (g_lpvec() < 0)
			goto lpcsr;
	}
	return(1);
}
g_lpvec()
{
	for (;;) {
		do
			printf("\nvector address <%o> ? ", lptype[0].lpvec);
		while((cc = getline("sg_vec")) < 0);
		if(cc == 0)
			return(-1);
		if (cc == 1)
			break;
		if((i = acon(line)) != -1) {
			lptype[0].lpvec = i;
			break;
		}
	}
	return(1);
}
g_cmc()
{
	static int doingover = 0;
cmc2:
	if (doingover) {
		printf("\n\7\7\7All communications devices deleted,");
		printf(" starting over!\n");
	} else
		doingover = 1;
	for(i=0; cmtype[i].cmn; i++) {
		cmtype[i].cmnunit = 0;
	}
cmc3:
	do {
		printf("\nCommunications devices:\n\n< ");
		for(k=0; cmtype[k].cmn; k++)
			printf("%s ", cmtype[k].cmn);
		printf("> ? ");
	} while((cc = getline("sg_cmt")) < 0);
	if(cc == 0) {
		doingover = 0;
		return(-1);
	}
	if(cc == 1)
		return(1);
	for(ct=0; cmtype[ct].cmn; ct++)
		if(strcmp(&line, cmtype[ct].cmn) == 0)
			break;
	if(cmtype[ct].cmn == 0) {
		printf("\n`%s' not supported!", line);
		goto cmc3;
	}
cmcnu:
		do
			printf("\nNumber of units <1> ? ");
		while((cc= getline("sg_cmn")) < 0);
		if(cc == 0)
			goto cmc3;
		if(cc == 1)
			cmtype[ct].cmnunit = 1;
		else
			cmtype[ct].cmnunit = atoi(line);
		if((cmtype[ct].cmnunit < 0) ||
			(cmtype[ct].cmnunit > cmtype[ct].cmumax)) {
			printf("\nMaximum of %d units allowed!\n",
				cmtype[ct].cmumax);
			cmtype[ct].cmnunit = 0;
			goto cmcnu;
		}
cmcsr:
	do
		printf("\nCSR address <%o> ? ", cmtype[ct].cmcsr);
	while((cc = getline("sg_csr")) < 0);
	if(cc == 0)
		goto cmcnu;
	if(cc != 1) {
		if((i = acon(line)) == -1)
			goto cmcsr;
		cmtype[ct].cmcsr = i;
	}
cmvec:
	do
		printf("\nVector address <%o> ? ", cmtype[ct].cmvec);
	while((cc = getline("sg_vec")) < 0);
	if(cc == 0)
		goto cmcsr;
	if(cc != 1) {
		if((i = acon(line)) == -1)
			goto cmvec;
		cmtype[ct].cmvec = i;
	}
	goto cmc3;
}
g_pty()
{
	register int npt;

	if(ovsys)
		npt = 2;
	else
		npt = 4;
	if((ms/16) < 512)
		npt = 2;
	for (;;) {
		do
			printf("\nNumber of pseudo ttys <%d> ? ", npt);
		while((cc = getline("sg_pty")) < 0);
		if(cc == 0)
			return(-1);
		if (cc == 1)
			cc = npt;
		else if((cc = atoi(line)) < 0)
			continue;
		if (cc < 2) {
			printf("\nMust have at least 2 pseudo ttys!\n");
			continue;
		}
		npty = cc;
		break;
	}
	return(1);
}
g_ctc()
{
	do {
		printf("\nInclude C/A/T phototypesetter driver");
		while((yn = yes(NO, NOHELP)) < 0)
			;
		if(buflag) {
			printf("\n");
			return(-1);
		}
		if (yn != YES) {
			cttype[0].ctused = 0;
			break;
		} else
			cttype[0].ctused = 1;
	} while (g_ctcsr() < 0);
	return(1);
}
g_ctcsr()
{
	for (;;) {
		do
			printf("\nCSR address <%o> ? ", cttype[0].ctcsr);
		while((cc = getline("sg_csr")) < 0);
		if(cc == 0)
			return(-1);
		if(cc != 1) {
			if((i = acon(line)) == -1)
				continue;
			cttype[0].ctcsr = i;
		}
		if (g_ctvec() > 0)
			break;
	}
	return(1);
}
g_ctvec()
{
	for (;;) {
		do
			printf("\nvector address <%o> ? ", cttype[0].ctvec);
		while((cc = getline("sg_vec")) < 0);
		if(cc == 0)
			return(-1);
		if (cc == 1)
			break;
		if((i = acon(line)) != -1) {
			cttype[0].ctvec = i;
			break;
		}
	}
	return(1);
}
g_udev()
{
	static int doingover = 0;
	if (doingover)
		printf("\n\7\7\7All user devices deleted, starting over!\n");
	else
		doingover = 1;
	for(i=0; udtype[i].udname; i++)
		udtype[i].udused = 0;
	nudev = 0;
	for (;;) {
		do {
			printf("\nUser devices:\n\n< ");
			for(k=0; udtype[k].udname; k++)
				printf("%s ", udtype[k].udname);
			printf("> ? ");
		} while((cc = getline("sg_udev")) < 0);
		if(cc == 0) {
			doingover = 0;
			return(-1);
		}
		if (cc == 1)
			return(1);
		for (ct=0; udtype[ct].udname; ct++)
			if(strcmp(&line, udtype[ct].udname) == 0)
				break;
		if (udtype[ct].udname == 0) {
			printf("\n`%s' not supported!\n", line);
			continue;
		}
		if (udtype[ct].udused) {
			printf("\n`%s' already configured!\n", udtype[ct].udname);
			continue;
		}
		udtype[ct].udused = 1;
		if ((g_udcsr() > 0) && (++nudev >= MAXUD))
			break;
	}
	return(1);
}
g_udcsr()
{
	for (;;) {
		do
			printf("\nCSR address <%o> ? ", udtype[ct].udcsr);
		while((cc = getline("sg_csr")) < 0);
		if(cc == 0)
			return(-1);
		if(cc != 1) {
			if((i = acon(line)) == -1)
				continue;
			udtype[ct].udcsr = i;
		}
		if (g_udvec() > 0)
			break;
	}
	return(1);
}
g_udvec()
{
	for(;;) {
		do
			printf("\nVector address <%o> ? ", udtype[ct].udvec);
		while((cc = getline("sg_vec")) < 0);
		if (cc == 0)
			return(-1);
		if (cc == 1)
			break;
		if((i = acon(line)) != -1) {
			udtype[ct].udvec = i;
			break;
		}
	}
	return(1);
}
g_fpsim()
{
	register int j;

	j = YES;
	if(cputyp[cpu].p_typ == cputype) {	/* on target CPU (best guess) */
		if(fpp < 0) {		/* no FPP support code (can't happen) */
			printf("\nFloating point support code missing, ");
			printf("you need FP simulator!\n");
		}
		if(fpp == 0) {		/* support code, but no FP hardware */
			printf("\nCurrent CPU does not have floating");
			printf(" point hardware!\n");
		}
		if(fpp == 1) {		/* support code and FP hardware */
			printf("\nCurrent CPU has floating point hardware!\n");
			j = NO;
		}
	}
	do {
		printf("\nInclude kernel floating point simulator");
		if ((yn = yes(j, HELP)) == -1)
			phelp("sg_fpsim");
	} while (yn == -1);
	if(buflag) {
		printf("\n");
		return(-1);
	}
	if(yn == YES)
		kfpsim = 1;
	else
		kfpsim = 0;
	return(1);
}
p_wcf()
{
	rewind(cf);	/* make sure writing to start of file (backup !) */
	if(cputyp[cpu].p_sid == NSID) {
		fprintf(cf, "ov\n");
		ovsys = 1;
	} else
		ovsys = 0;
	if(nomap)			/* If CPU has no unibus map, */
		fprintf(cf, "nomap\n");	/* do not include code to support it. */
	if(kfpsim)
		fprintf(cf, "kfpsim\n");
	if(generic)
		fprintf(cf, "generic\n");	/* tell mkconf, generic kernel */
	for(i=0; dcd[i].dctyp; i++) {	/* find & print system disk */
		if(dcd[i].dcsys < 0)
			continue;
		sd = dcd[i].dcsys;
		fprintf(cf, "%d%s %o %o %d\n",
			dcd[i].dcnd,
			dcd[i].dcname,
			dcd[i].dcaddr,
			dcd[i].dcvect,
			dcd[i].dccn);
		j = (sysdn * ddtype[sysdt].ddnp) + (sdfsl[sd].rootmd & 7);
		j |= dcd[i].dccn << 6;
		fprintf(cf, "root %s %d\n", sdfsl[sd].rootdev, j);
		j = (sysdn * ddtype[sysdt].ddnp) + (sdfsl[sd].pipemd & 7);
		j |= dcd[i].dccn << 6;
		fprintf(cf, "pipe %s %d\n", sdfsl[sd].pipedev, j);
		j = (sysdn * ddtype[sysdt].ddnp) + (sdfsl[sd].swapmd & 7);
		j |= dcd[i].dccn << 6;
		fprintf(cf, "swap %s %d\n", sdfsl[sd].swapdev, j);
		fprintf(cf, "swplo %D\n", sdfsl[sd].swaplo);
		fprintf(cf, "nswap %d\n", sdfsl[sd].nswap);
		j = (sysdn * ddtype[sysdt].ddnp) + (sdfsl[sd].elmd & 7);
		j |= dcd[i].dccn << 6;
		fprintf(cf, "eldev %s %d\n", sdfsl[sd].eldev, j);
		fprintf(cf, "elsb %D\n", sdfsl[sd].elsb);
		fprintf(cf, "elnb %d\n", sdfsl[sd].elnb);
		if((syscdd >= 0) && (cdtab[syscdd].cd_flags & CD_DISK)) {
			if(cdtab[syscdd].cd_flags & CD_RX50)
			    fprintf(cf, "dump rx\n");
			else
			    fprintf(cf, "dump %s\n", cdtab[syscdd].cd_name);
			fprintf(cf, "dumplo %D\n", cdtab[syscdd].cd_dmplo);
			fprintf(cf, "dumphi %D\n", cdtab[syscdd].cd_dmphi);
			fprintf(cf, "dumpdn %d\n", cdtab[syscdd].cd_unit);
		}
		break;
	}
	for(i=0; dcd[i].dctyp; i++) {	/* all other disks */
		if(dcd[i].dcsys >= 0)
			continue;
		fprintf(cf, "%d%s %o %o %d\n",
			dcd[i].dcnd,
			dcd[i].dcname,
			dcd[i].dcaddr,
			dcd[i].dcvect,
			dcd[i].dccn);
	}
	for(i=0; mttype[i].mtct; i++) {	/* magtapes */
		if(mttype[i].mtnunit == 0 || strcmp(mttype[i].mtun,"ts") == 0
		 || strcmp(mttype[i].mtun,"tk") == 0)
			continue;
		fprintf(cf, "%d%s %o %o\n",
			mttype[i].mtnunit,
			mttype[i].mtun,
			mttype[i].mtcsr,
			mttype[i].mtvec);
		if(mttype[i].mtcd > 0)
			fprintf(cf, "dump %s\n", cdtab[syscdd].cd_name);
	}
	for(i=0; mtts[i].mtct; i++) {	/* ts magtapes */
		if(mtts[i].mtnunit < 0)
			continue;
		fprintf(cf, "1%s %o %o %d\n",
			mtts[i].mtun,
			mtts[i].mtcsr,
			mtts[i].mtvec,
			mtts[i].mtnunit);
		if(mtts[i].mtcd > 0)
			fprintf(cf, "dump %s\n", cdtab[syscdd].cd_name);
	}
	for(i=0; mttk[i].mtct; i++) {	/* tk magtapes */
		if(mttk[i].mtnunit < 0)
			continue;
		fprintf(cf, "1%s %o %o %d\n",
			mttk[i].mtun,
			mttk[i].mtcsr,
			mttk[i].mtvec,
			mttk[i].mtnunit);
		if(mttk[i].mtcd > 0)
			fprintf(cf, "dump %s\n", cdtab[syscdd].cd_name);
	}
	for(i=0; cmtype[i].cmn; i++) {	/* comm. devices */
		if(cmtype[i].cmnunit == 0)
			continue;
		fprintf(cf, "%d%s %o %o\n",
			cmtype[i].cmnunit,
			cmtype[i].cmnx,
			cmtype[i].cmcsr,
			cmtype[i].cmvec);
	}
	fprintf(cf, "pty %d\n", npty);
	if(lptype[0].lpused)	/* LP11 */
		fprintf(cf, "lp %o %o\n",lptype[0].lpcsr, lptype[0].lpvec);
	if(cttype[0].ctused)	/* CAT */
		fprintf(cf, "ct %o %o\n",cttype[0].ctcsr, cttype[0].ctvec);
	for(i=0; udtype[i].udname; i++) {	/* user devices */
		if(udtype[i].udused == 0)
			continue;
		fprintf(cf, "%s %o %o\n",
			udtype[i].udname, udtype[i].udcsr, udtype[i].udvec);
	}
	if (network)
		fprintf(cf, "network\n");
	for (i=0; nettype[i].ntn; i++) {	/* network device */
		if (nettype[i].ntnunit == 0)
			continue;
		for (j = 0; j < nettype[i].ntnunit; j++)
			fprintf(cf, "1if_%s %o %o %d\n",
				nettype[i].ntnx,
				nettype[i].ntcsr[j],
				nettype[i].ntvec[j],
				nettype[i].nvec < 0 ?
					- nettype[i].nvec : nettype[i].nvec);
	}
	return(1);
}
g_spar()
{
	for (;;) {
		printf("\nUse standard system parameters");
		if((yn = yes(YES, HELP)) == -1) {
			phelp("sg_sp");
			continue;
		}
		if(buflag) {	
			printf("\n");
			return(-2);
		}
		for(i=0; syspar[i].spname; i++) {
			if(strcmp("maxseg", syspar[i].spname) == 0) {
				if((cputyp[cpu].p_flag & A22BIT) &&
				   (cputyp[cpu].p_bus == QBUS)) {
					syspar[i].spv_ov = 65408;
					syspar[i].spv_id = 65408;
				} else {
					syspar[i].spv_ov = 61440;
					syspar[i].spv_id = 61440;
				}
				break;
			}
		}
		if(yn == YES) {
			for(i=0; syspar[i].spname; i++) {
				if(syspar[i].spask == -1)
					continue;
				syspar[i].spval = ovsys ?
					syspar[i].spv_ov : syspar[i].spv_id;
			}
			return(1);
		}
		if (g_cspr() > 0)
			break;
	}
	return(1);
}
long atol();
g_cspr()
{
	printf("\n\n\07\07\07CHANGING SYSTEM PARAMETERS!");
	printf("\n\nPress <RETURN> to use the default value!");
	printf("\nType ?<RETURN> for help!\n");
	for(i=0; syspar[i].spname; i++) {
		if(syspar[i].spask == -1)
			continue;
		if (strcmp("mapsize", syspar[i].spname) == 0) {
			do {
				printf("\nmapsize <%u> ? ", mapszval);
			} while((cc = getline(syspar[i].sphelp)) < 0);
		} else {
			do {
				printf("\n%s <%u> ? ", syspar[i].spname,
				   ovsys ? syspar[i].spv_ov : syspar[i].spv_id);
			} while((cc = getline(syspar[i].sphelp)) < 0);
		}
		if (cc == 0)
			return(-1);
		if ((cc == 1) && (strcmp("mapsize", syspar[i].spname) == 0))
			syspar[i].spval = mapszval;
		else if(cc == 1)
		  syspar[i].spval = ovsys ? syspar[i].spv_ov : syspar[i].spv_id;
		else {
		    if (strcmp("ulimit", syspar[i].spname) == 0) {
			ulimitval = atol(line);
			if (ulimitval <= 0L)
			    ulimitval = 1024L;
		    }
		    else
			syspar[i].spval = atoi(line);
		}
		if (strcmp("nproc", syspar[i].spname) == 0)
			mapszval = 30 + syspar[i].spval/2;
	}
	return(1);
}
g_shff()
{
	do {
		printf("\nInclude memory shuffle routine");
		if((yn = yes(YES, HELP)) == -1)
			phelp("sg_shuff");
	} while (yn == -1);
	if(buflag) {	
		printf("\n");
		return(-1);
	}
	if(yn == YES)
		shuff = 1;
	else
		shuff = 0;
	return(1);
}
g_msg()
{
	do {
		do {
	printf("\nInclude interprocess communication message facility");
			if((yn = yes(YES, HELP)) == -1)
				phelp("sg_mesg");
		} while (yn == -1);
		if(buflag) {	
			printf("\n");
			return(-1);
		}
		if(yn != YES) {
			mesg = 0;
			return(1);
		}
		mesg = 1;
	} while (g_msgpr() < 0);
	return(1);
}
g_msgpr()
{
	register int seg_size,no_segs;
	long limit;

	limit = (long)64*1024 - 63;
	do {
		printf("\nUse standard message parameters");
		if((yn = yes(YES, HELP)) == -1)
			phelp("sg_mgpar");
	} while (yn == -1);
	if(buflag) {	
		printf("\n");
		return(-1);
	}
	if(yn == NO) {
		for(;;) {
		   printf("\n\nPress <RETURN> to use the default value!");
		   printf("\nType ?<RETURN> for help!\n");
		   for(i=0; msgpar[i].spname; i++) {
again:
			do {
			  printf("\n%s <%u> ? ", msgpar[i].spname,
			   ovsys ? msgpar[i].spv_ov : msgpar[i].spv_id);
			} while((cc = getline(msgpar[i].sphelp)) < 0);
			if(cc == 0)
				return(-1);
			if(cc == 1)
				msgpar[i].spval = ovsys ? 
					msgpar[i].spv_ov : 
					msgpar[i].spv_id;
			else {
				msgpar[i].spval = atoi(line);
				if(!strcmp("msgssz",msgpar[i].spname)){
				   if(msgpar[i].spval%2) { /* msg segment size should be word size multiple */
				      printf("\nSegment size should be a multiple of 2");
				      goto again;
				   }
				}
				if(!strcmp("msgseg",msgpar[i].spname)){
				   if(msgpar[i].spval >= 32768) { 
				      printf("\nNumber of message segments should be less than 32768");
				      goto again;
				   }
				}
			}
			if(!strcmp("msgssz",msgpar[i].spname))
		   		seg_size = msgpar[i].spval;
			if(!strcmp("msgseg",msgpar[i].spname))
	   			no_segs = msgpar[i].spval;
		   }
		   if((long)seg_size*no_segs >= limit) 
			printf("\nTotal size of Message Segments (msgssz*msgseg) should be less than %ld",limit);
		   else 
			break;  /* get out of for loop */
		} /* end for */
	} else {
		for(i=0; msgpar[i].spname; i++)
			msgpar[i].spval = ovsys ?
				msgpar[i].spv_ov : msgpar[i].spv_id;
	}
	return(1);
}
g_smp()
{
	do {
		do {
	printf("\nInclude interprocess communication semaphore facility");
			if((yn = yes(YES, HELP)) == -1)
				phelp("sg_sema");
		} while (yn == -1);
		if(buflag) {	
			printf("\n");
			return(-1);
		}
		if (yn != YES) {
			sema = 0;
			break;
		}
		sema = 1;
	} while (g_smpr() < 0);
	return(1);
}
g_smpr()
{
	do {
		printf("\nUse standard semaphore parameters");
		if((yn = yes(YES, HELP)) == -1)
			phelp("sg_smpar");
	} while (yn == -1);
	if(buflag) {	
		printf("\n");
		return(-1);
	}
	if(yn == NO) {
		printf("\n\nPress <RETURN> to use the default value!");
		printf("\nType ?<RETURN> for help!\n");
		for(i=0; sempar[i].spname; i++) {
			do {
			  printf("\n%s <%u> ? ", sempar[i].spname,
			   ovsys ? sempar[i].spv_ov : sempar[i].spv_id);
			} while((cc = getline(sempar[i].sphelp)) < 0);
			if(cc == 0)
				return(-1);
			if(cc == 1)
				sempar[i].spval = ovsys ? 
					sempar[i].spv_ov : 
					sempar[i].spv_id;
			else
				sempar[i].spval = atoi(line);
		}
	} else 
		for(i=0; sempar[i].spname; i++)
			sempar[i].spval = ovsys ?
				sempar[i].spv_ov : sempar[i].spv_id;
	return(1);
}
g_flck()
{
	do {
		do {
			printf("\nInclude advisory record and file locking");
			if((yn = yes(YES, HELP)) == -1)
				phelp("sg_flock");
		} while (yn == -1);
		if(buflag) {	
			printf("\n");
			return(-1);
		}
		if (yn != YES) {
			flock = 0;
			break;
		}
		flock = 1;
	} while (g_flkpar() < 0);
	return(1);
}
g_flkpar()
{
	do {
		printf("\nUse standard locking parameters");
		if ((yn = yes(YES, HELP)) == -1)
			phelp("sg_flkpar");
	} while (yn == -1);
	if(buflag) {	
		printf("\n");
		return(-1);
	}
	if (yn == NO) {
		printf("\n\nPress <RETURN> to use the default value!");
		printf("\nType ?<RETURN> for help!\n");
		for(i=0; flckpar[i].spname; i++) {
			do {
			  printf("\n%s <%u> ? ", flckpar[i].spname,
			 ovsys ? flckpar[i].spv_ov : flckpar[i].spv_id);
			} while((cc = getline(flckpar[i].sphelp)) < 0);
			if(cc == 0)
				return(-1);
			if(cc == 1)
				flckpar[i].spval = ovsys ? 
					flckpar[i].spv_ov : 
					flckpar[i].spv_id;
			else
				flckpar[i].spval = atoi(line);
		}
	} else
		for(i=0; flckpar[i].spname; i++)
			flckpar[i].spval = ovsys ?
				flckpar[i].spv_ov : flckpar[i].spv_id;
	return(1);
}
g_mus()
{
	do {
		do {
			printf("\nInclude multiple access user space");
			if ((yn = yes(YES, HELP)) == -1)
				phelp("sg_maus");
		} while (yn == -1);
		if(buflag) {	
			printf("\n");
			return(-1);
		}
		if(yn == NO){
			maus = 0;
			break;
		}
		maus = 1;
	} while (g_mauspar() < 0);
	return(1);
}
g_mauspar()
{
	do {
		printf("\nUse standard maus parameters");
		if ((yn = yes(YES,HELP)) == -1)
			phelp("sg_mspar");
	} while (yn == -1);
	if(buflag) {
		printf("\n");
		return(-1);
	}
	if (yn == NO) {
		printf("\n\nPress <RETURN> to use the default value!");
		printf("\nType ?<RETURN> for help!\n");
		for(i=0; mauspar[i].spname; i++) {
	again:		do {
				printf("\n%s <%u> ? ",mauspar[i].spname,
				ovsys ? mauspar[i].spv_ov : mauspar[i].spv_id);
			} while((cc = getline(mauspar[i].sphelp)) < 0);
			if(cc == 0)
				return(-1);
			if(cc == 1)
				mauspar[i].spval = ovsys ?
					mauspar[i].spv_ov : mauspar[i].spv_id;
			else {
				if (i == 0) {
					if(atoi(line) <= 0 || atoi(line) >8) {
						printf("\nNumber of maus entries should be between 1 and 8");
						goto again;
					}
				} else {
					if(atoi(line) <= 0 || atoi(line) > 128) {
						printf("\nSize of each maus area should be between 1 and 128 (clicks of 64 bytes)");
						goto again;
					}
				}
				mauspar[i].spval = atoi(line);
			}
			if ( i == mauspar[0].spval)
				break;
		}
	} else {
		mauspar[0].spval = ovsys ? mauspar[0].spv_ov : mauspar[0].spv_id;
		for(i=1; i <= mauspar[0].spval;i++)
			mauspar[i].spval = ovsys ?
				mauspar[i].spv_ov : mauspar[i].spv_id;
	}
	return(1);
}
g_hz()
{
	for(;;) {
		do
			printf("\nAC power line frequency in hertz <%d> ? ", hz);
		while((cc = getline("sg_hz")) < 0);
		if(cc == 0)
			return(-1);
		for(i=0; syspar[i].spname; i++)
			if(strcmp("hz", syspar[i].spname) == 0)
				break;
		if(cc == 1)
			syspar[i].spval = hz;
		else
			syspar[i].spval = atoi(line);
		if((syspar[i].spval != 50) && (syspar[i].spval != 60)) {
			printf("\nLine frequency should be 50 or 60 hertz!\n");
			printf("\nDo you really want %d hertz", syspar[i].spval);
			if(yes(NO, NOHELP) == NO)
				continue;
		}
		break;
	}
	return(1);
}
g_timz()
{
	for(;;) {
		printf("\nCurrent timezone is %d hours west/behind GMT.\n", tz);
		do {
			printf("\nTimezone (hours west/behind GMT) ");
			printf("<5=EST 6=CST 7=MST 8=PST> ? ");
		 } while((cc = getline("sg_tz")) < 0);
		if(cc == 0)
			return(-1);
		if(cc == 1)
			continue;
		for(i=0; syspar[i].spname; i++)
			if(strcmp("timezone", syspar[i].spname) == 0)
				break;
		if(strlen(line) > 2)
			continue;
		if((line[0] < '0') || (line[0] > '9'))
			continue;
		if((strlen(line) == 2) && ((line[1] < '0') || (line[1] > '9')))
			continue;
		syspar[i].spval = atoi(line);
		if((syspar[i].spval >= 0) && (syspar[i].spval < 24))
			break;
	}
	return(1);
}
g_dst()
{
	register int j;
	register int ret;

	if(dst)
		j = YES;
	else
		j = NO;
	do {
		printf("\nDoes your area use daylight savings time");
		if ((yn = yes(j, HELP)) < 0)
			phelp("sg_dst");
	} while (yn < 0);
	if(buflag) {
		printf("\n");
		return(-1);
	}
	for(i=0; syspar[i].spname; i++)
		if(strcmp("dstflag", syspar[i].spname) == 0)
			break;
	if(yn == YES) {
		if((ret = get_dst()) == -1)
			return(-1);
		syspar[i].spval = ret;
	} else
		syspar[i].spval = 0;
	return(1);
}
get_dst()
{

	register int cc;
	register struct dst_table *dst_ptr;
again:
	printf("\nChoose the Geographic Area for daylight savings time from the table below:\n");
	printf("\n\t\tGeographic Area\tSelection\n");
	printf("\t\t---------------\t---------\n");
	for(dst_ptr=dst_table;dst_ptr->dst_area;dst_ptr++) {
		printf("\t\t%s",dst_ptr->dst_area);
		if(strlen(dst_ptr->dst_area) > 7)
			printf("\t");
		else
			printf("\t\t");
		printf("%4d\n",dst_ptr->dst_id);
	}
	do 
		printf("\nEnter the selection number <%d> ",DST_USA);
	while((cc = getline("sg_dstarea")) < 0);
	if(cc == 0) 
		return(-1);
	if(cc == 1)
		return(DST_USA);  /* USA (default) */
	cc = atoi(line);
	if((cc < DST_USA) || (cc > DST_EET)) {
		printf("Enter a number between %d and %d\n",DST_USA,DST_EET);
		goto again;
	}
	return(cc);
		

}

p_wcf2()
{
	/*
	 * The below increment of mapsize is due to the SYSTEM V
	 * compatability mods required for malloc/mfree. One extra
	 * location is all that is needed.
	 */
	for(i=0; syspar[i].spname; i++)
		if(strcmp("mapsize", syspar[i].spname) == 0)
			syspar[i].spval += 1;;
	for(i=0; syspar[i].spname; i++) {
		if (strcmp("ulimit", syspar[i].spname) == 0)
		    fprintf(cf, "%s %ld\n",syspar[i].spname, ulimitval);
		else
		    fprintf(cf, "%s %u\n", syspar[i].spname, syspar[i].spval);
	}
	if(shuff)
		fprintf(cf, "shuffle\n");
	if(mesg) {
		fprintf(cf, "mesg\n");
		for(i=0; msgpar[i].spname; i++)
			fprintf(cf, "%s %u\n", msgpar[i].spname, 
				msgpar[i].spval);
	}
	if(sema) {
		fprintf(cf, "sema\n");
		for(i=0; sempar[i].spname; i++)
			fprintf(cf, "%s %u\n", sempar[i].spname, 
				sempar[i].spval);
	}
	if(flock) {
		fprintf(cf, "flock\n");
		for(i=0; flckpar[i].spname; i++)
			fprintf(cf, "%s %u\n", flckpar[i].spname, 
				flckpar[i].spval);
	}
	if(maus) {
		fprintf(cf, "maus\n");
		for(i=0; i <= mauspar[0].spval; i++)
			fprintf(cf, "%s %u\n", mauspar[i].spname,
				mauspar[i].spval);
	}
	if (network) {
		for(i=0; netpar[i].spname; i++)
			fprintf(cf, "%s %u\n", netpar[i].spname, 
				netpar[i].spval);
	}
	fclose(cf);
	return(1);
}
/*
 * Create the ?.cf_p file containing a formatted
 * printout of the configuration file ?.cf.
 */
p_wcf_p()
{
	p = strcpy(&cpf, &config);
	p = strcat(p, "_p");
	if((cf = fopen(p, "w")) == NULL) {
		printf("\nCan't create %s file\n", p);
		return;
	}
	p = strcpy(&line, &config);
	while(*++p != '.');
	*p = '\0';
	fprintf(cf, "\n\n(%s): ", line);
	if(ovsys) {
		k = NSID;
		fprintf(cf, "overlay kernel for < ");
	} else {
		k = SID;
		fprintf(cf, "split I & D kernel for < ");
	}
	for(i=0; cputyp[i].p_nam; i++)
		if((cputyp[i].p_sid == k) && ((cputyp[i].p_flag&LATENT) == 0))
			fprintf(cf, "%s ", cputyp[i].p_nam);
	fprintf(cf, "> processors.\n");
	for(i=0; dcd[i].dctyp; i++) {
		for(j=0; dctype[j].dcn; j++)
			if(dcd[i].dctyp == dctype[j].dct)
				break;
		fprintf(cf, "\n`%s'\t%-12s- %2d unit(s)  ",
			dcd[i].dcname, dctype[j].dcn, dcd[i].dcnd);
		for(j=0; dcd[i].dcunit[j]; j++)
			fprintf(cf, "%s ", ddtype[dcd[i].dcunit[j]]);
	}
	for(i=0; mttype[i].mtct; i++)
		if(mttype[i].mtnunit) {
			fprintf(cf, "\n`%s'\t%-12s- %2d unit(s)",
			    mttype[i].mtun, mttype[i].mtct, mttype[i].mtnunit);
		}
	for(i=0; mtts[i].mtct; i++)
		if(mtts[i].mtnunit >= 0) {
			fprintf(cf, "\n`%s'\t%-12s- unit %2d",
		    	    mtts[i].mtun, mtts[i].mtct, mtts[i].mtnunit);
		}
	for(i=0; mttk[i].mtct; i++)
		if(mttk[i].mtnunit >= 0) {
			fprintf(cf, "\n`%s'\t%-12s- unit %2d",
		    	    mttk[i].mtun, mttk[i].mtct, mttk[i].mtnunit);
		}
	for(i=0; cmtype[i].cmn; i++)
		if(cmtype[i].cmnunit)
			fprintf(cf, "\n`%s'\t%-12s- %2d unit(s)",
			    cmtype[i].cmn, cmtype[i].cmdn, cmtype[i].cmnunit);
	for(i=0; nettype[i].ntn; i++)
		if(nettype[i].ntnunit)
			fprintf(cf, "\n`%s'\t%-12s- %2d unit(s)",
			   nettype[i].ntnx, nettype[i].ntn, nettype[i].ntnunit);
	if(lptype[0].lpused)
		fprintf(cf, "\n`lp'\tLP11        -  1 unit(s)");
	if(cttype[0].ctused)
		fprintf(cf, "\n`ct'\tCAT         -  1 unit(s)");
	if(syscdd >= 0) {
		fprintf(cf, "\n\nCrash Dump Device: %s unit %d",
			cdtab[syscdd].cd_gtyp, cdtab[syscdd].cd_unit);
		if(cdtab[syscdd].cd_flags & CD_DISK)
			fprintf(cf, "  dumplo = %D  dumphi = %D",
				cdtab[syscdd].cd_dmplo, cdtab[syscdd].cd_dmphi);
		if(cdtab[syscdd].cd_flags & CD_RX50)
			fprintf(cf, " blocks/diskette");
	}
	fprintf(cf, "\n");
	for(i=0; dcd[i].dctyp; i++)
		if(dcd[i].dcsys >= 0)
			break;
	k = dcd[i].dccn << 6;
	i = dcd[i].dcsys;
	fprintf(cf, "\nROOT      on `%s' minor device %2d",
	    sdfsl[i].rootdev,(sysdn*ddtype[sysdt].ddnp)+(sdfsl[i].rootmd&7)|k);
	fprintf(cf, "\nPIPE      on `%s' minor device %2d",
	    sdfsl[i].pipedev,(sysdn*ddtype[sysdt].ddnp)+(sdfsl[i].pipemd&7)|k);
	fprintf(cf, "\nSWAP      on `%s' minor device %2d ",
	    sdfsl[i].swapdev,(sysdn*ddtype[sysdt].ddnp)+(sdfsl[i].swapmd&7)|k);
	fprintf(cf, "Start block = %6D  Length = %6d blocks",
			sdfsl[i].swaplo, sdfsl[i].nswap);
	fprintf(cf, "\nERROR LOG on `%s' minor device %2d ",
	    sdfsl[i].eldev,(sysdn*ddtype[sysdt].ddnp)+(sdfsl[i].elmd&7)|k);
	fprintf(cf, "Start block = %6D  Length = %6d blocks",
		sdfsl[i].elsb, sdfsl[i].elnb);
	fprintf(cf, "\n");
	for(i=0; syspar[i].spname; i++) {
		if((i%4) == 0)
			fprintf(cf, "\n");
		fprintf(cf, "%-8s%6u    ",syspar[i].spname, syspar[i].spval);
	}
	j = i;
	if(mesg) {
	    for(i=0; msgpar[i].spname; i++, j++) {
		if((j%4) == 0)
			fprintf(cf, "\n");
		fprintf(cf, "%-8s%6u    ",msgpar[i].spname, msgpar[i].spval);
	    }
	}
	if(sema) {
	    for(i=0; sempar[i].spname; i++, j++) {
		if((j%4) == 0)
			fprintf(cf, "\n");
		fprintf(cf, "%-8s%6u    ",sempar[i].spname, sempar[i].spval);
	    }
	}
	if(flock) {
	    for(i=0; flckpar[i].spname; i++, j++) {
		if((j%4) == 0)
			fprintf(cf, "\n");
		fprintf(cf, "%-8s%6u    ",flckpar[i].spname, flckpar[i].spval);
	    }
	}
	if(shuff) {
		if(((j++)%4) == 0)
			fprintf(cf, "\n");
		fprintf(cf, "%-13s     ","shuffle");
	}
	if(maus) {
		if(((j++)%4) == 0)
			fprintf(cf, "\n");
		fprintf(cf, "%-13s     ","maus");
	}
	if(network) {
	    for(i=0; netpar[i].spname; i++, j++) {
		if((j%4) == 0)
			fprintf(cf, "\n");
		fprintf(cf, "%-8s%6u    ",netpar[i].spname, netpar[i].spval);
	    }
	}
	for(i=0; syspar[i].spname; i++) {
		if(strcmp("timezone", syspar[i].spname) == 0)
			j = syspar[i].spval;
		if(strcmp("dstflag", syspar[i].spname) == 0)
			k = syspar[i].spval;
	}
	fprintf(cf,  "\n\nTIMEZONE - %d hours ahead of GMT, with", j);
	if(k == 0)
		fprintf(cf, "out");
	fprintf(cf, " daylight savings time.");
	if(kfpsim)
		fprintf(cf, "\nFloating point simulator included.");
	fprintf(cf, "\n");
	fclose(cf);
	return(1);
}

g_net()
{
	do {
		do {
			/* check memory size, if < 512Kbytes, assume NO net */
			if (ms/16 < 512) 
				j = NO;
			else
				j = YES;
			printf("\nInclude TCP/IP ethernet support");
			if((yn = yes(j, HELP)) == -1)
				phelp("sg_network");
		} while (yn == -1);
		if(buflag) {	
			printf("\n");
			return(-1);
		}
		if (yn != YES) {
			network = 0;
			break;
		}
		network = 1;
	} while (g_net2() < 0);
	return(1);
}
g_net2()
{
	do {
		if (g_netdev() < 0)
			return(-1);
	} while (g_netpar() < 0);
	return(1);
}
g_netdev()
{
	register int i, ct;
	static int doingover = 0;
	int  ubus = 0;
	int qbus = 0;

	if (doingover) 
		printf("\n\7\7\7All network devices deleted, starting over!\n");
	else
		doingover = 1;
	for(i=0; nettype[i].ntn; i++) {
		nettype[i].ntnunit = 0;
		if (nettype[i].nvec > 0)
			nettype[i].nvec = 0;
	}
	nudev = 0;
	for (;;) {
		do {
back1:
			printf("\nEthernet interfaces:\n\n< ");
			for(k=0; nettype[k].ntn; k++)
				printf("%s ", nettype[k].ntn);
			printf("> ? ");
		} while((cc = getline("sg_netdev")) < 0);
		if(cc == 0) {
			doingover = 0;
			return(-1);
		}
		if (cc == 1)
			break;
		for (ct=0; nettype[ct].ntn; ct++)
			if(strcmp(&line, nettype[ct].ntn) == 0) {
				if (strcmp(&line,"deuna") == 0) {
					if(qbus) {
						printf("\n`%s' should not be configured along with  deqna!\n",line); 
						goto back1;
					}
					else {
						if(ubus) {
							printf("\n`%s' already configured!\n",line);
							goto back1;
						}
						else ubus++;
					}
				} else 
				if (strcmp(&line,"deqna") == 0) {
					if(ubus) {
						printf("\n`%s' should not be configured along with deuna!\n",line);
						goto back1;
					}
					else {
						if(qbus) {
							printf("\n`%s' already configured!\n",line);
							goto back1;
						}
						else qbus++;
					}
				}
				break;
			}
		if (nettype[ct].ntn == 0) {
			printf("\n`%s' not supported!\n", line);
			continue;
		}
		g_netunits(ct);
	}
	return(1);
}
g_netunits(ct)
register int ct;
{
	register int cc;

	for (;;) {
	back1:
		nettype[ct].ntnunit = 0;
		do
			printf("\nNumber of units <1> ? ");
		while((cc= getline("sg_netn")) < 0);
		if(cc == 0)
			return(-1);
		if (cc != 1)
			cc = atoi(line);
		if (cc < 0 || cc > nettype[ct].ntumax) {
			printf("\nMaximum of %d unit(s) allowed!\n",
				nettype[ct].ntumax);
			continue;
		}
		nettype[ct].ntnunit = cc;

		if (nettype[ct].nvec >= 0) {
			for(;;) {
			back2:
				do {
					printf("\nNumber of interrupt ");
					printf("vectors per unit <1> ? ");
				} while ((cc= getline("sg_netv")) < 0);
				if (cc == 0)
					goto back1;
				if (cc != 1)
					cc = atoi(line);
				if (cc != 1 && cc != 2) {
					printf("\nSpecify 1 or 2 vectors\n");
					continue;
				}
				nettype[ct].nvec = cc;
				break;
			}
		}

		if (g_netcsr(ct) >= 0)
			break;
		if (nettype[ct].nvec >= 0)
			goto back2;
	}
	return(1);
}
g_netcsr(ct)
register ct;
{
	register int cc;
	register int i;

	for (i = 0; i < nettype[ct].ntnunit;) {
		do {
			printf("\nUnit %d CSR address <", i);
			if (nettype[ct].ntcsr[i])
				printf("%o> ? ", nettype[ct].ntcsr[i]);
			else
				printf("no default> ? ");
		} while((cc = getline("sg_csr")) < 0);
		if(cc == 0) {
			if (i == 0)
				return(-1);
			--i;
			continue;
		}
		if(cc != 1) {
			if((cc = acon(line)) == -1)
				continue;
			nettype[ct].ntcsr[i] = cc;
		} else if (!nettype[ct].ntcsr[i]) {
			printf("\nNo default, try again.");
			continue;
		}
		if (g_netvec(ct, i) < 0)
			continue;
		i++;
	}
	return(1);
}
g_netvec(ct, i)
register int ct;
int i;
{
	register int cc;

	for(;;) {
		do {
			printf("\nUnit %d Vector address <", i);
			if (nettype[ct].ntvec[i])
				printf("%o> ? ", nettype[ct].ntvec[i]);
			else
				printf("no default> ? ");
		} while((cc = getline("sg_vec")) < 0);
		if (cc == 0)
			return(-1);
		if (cc == 1) {
			if (nettype[ct].ntvec[i])
				break;
			printf("\nNo default, try again.");
		}
		if ((cc = acon(line)) != -1) {
			nettype[ct].ntvec[i] = cc;
			break;
		} 
	}
	return(1);
}
g_netpar()
{
	register int cc;

	do {
		printf("\nUse standard network parameters");
		if ((yn = yes(YES, HELP)) == -1)
			phelp("sg_netpar");
	} while (yn == -1);
	if(buflag) {	
		printf("\n");
		return(-1);
	}
	if (yn == NO) {
		printf("\n\nPress <RETURN> to use the default value!");
		printf("\nType ?<RETURN> for help!\n");
		for(i=0; netpar[i].spname; i++) {
			do {
			  printf("\n%s <%u> ? ", netpar[i].spname,
			 ovsys ? netpar[i].spv_ov : netpar[i].spv_id);
			} while((cc = getline(netpar[i].sphelp)) < 0);
			if(cc == 0)
				return(-1);
			if(cc == 1)
				netpar[i].spval = ovsys ? 
					netpar[i].spv_ov : 
					netpar[i].spv_id;
			else
				netpar[i].spval = atoi(line);
		}
	} else
		for(i=0; netpar[i].spname; i++)
			netpar[i].spval = ovsys ?
				netpar[i].spv_ov : netpar[i].spv_id;
	return(1);
}
