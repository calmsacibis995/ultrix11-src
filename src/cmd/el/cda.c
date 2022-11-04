
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)cda.c	3.1	7/21/87";
/*
 * ULTRIX-11 crash dump analysis program (cda).
 * Fred Canter 10/10/83
 * Bill Burns 6/84
 *	added:
 *		core and swap map prinout (-r (resource map))
 *		buffer queue option (-q)
 *		added mscp disk info dump (-u)
 *		added getu option
 *
 * Chung-Wu Lee 2/11/85
 *	added:
 *		added tmscp magtape info dump (-k)
 *
 * Fred Canter -- 8/17/85
 *
 *	Make CDA understand the ublock doubled in size.
 *	Fix up CDA's concept of the stack frame, which
 *	was really bent by the 431 and 451 kernels.
 *	Expand CPU error register bits with ascii text.
 *
 * Usage: cda -he?????? [namelist] [corefile]
 *
 *	namelist	File name containing the namelist,
 *			default is `/unix'.
 *
 *	corefile	Filename of the core dump file,
 *			default is `/usr/crash/core'.
 *			If the corefile is given then namelist
 *			must also be given.
 *
 *	-h		Print the help message.
 *
 *	-e		Locate, format, and print any unlogged errors
 *			(from the kernel error log buffer in memory).
 *			Also print last error logged by each block I/O device.
 *
 *	-t		Format and print the panic trap stack frame.
 *
 *	-m		Print the memory usage map (memstat)
 *
 *	-b		Print I/O buffer pool usage (bufstat)
 *
 *	-r		Print the resourse maps (core and swap)
 *
 *	-q		Print the block I/O buffer queues
 *
 *	-u		Print the MSCP disk data structures
 *
 *	-k		Print the TMSCP magtape data structures
 *
 *	-g[#]		Use getu to print user blocks
 *
 *	-ps		Get process status from core dump (ps -alx)
 *
 *	-pp		pstat -p
 *	-ppa		pstat -pa
 *	-pi		pstat -i
 *	-pf		pstat -f
 *	-px		pstat -x
 *	-pu#		pstat -u addr
 *	-pt		pstat -t
 *	-ppaifxtu#	all of the above
 */

#include <sys/param.h>	/* Don't matter which one */
#include <sys/tmscp.h>	/* must preceed errlog.h (EL_MAXSZ) */
#include <sys/errlog.h>
#include <sys/hp_info.h>
#include <stdio.h>
#include <a.out.h>
#include <sys/map.h>
#include "cda.h"
#include <time.h>
#define	MBSMAX	1024		/* MAX size of saved error message buffer */

struct	nlist	nl[] = {
	{ "_el_buf" },		/* 0 */
	{ "_elbsiz" },		/* 1 */
	{ "_el_bpi" },		/* 2 */
	{ "_el_bpo" },		/* 3 */
	{ "_cputype" },		/* 4 */
	{ "__ovno" },		/* 5 */
	{ "svi0" },	/* no longer used */	/* 6 */
	{ "_rn_ssr3" },		/* 7 */
	{ "_cpereg" },		/* 8 */
	{ "_mmr3" },		/* 9 */
	{ "ssr" },		/* 10 */
	{ "ova" },		/* 11 */
	{ "_msgbuf" },		/* 12 */
	{ "_msgbufs" },		/* 13 */
	{ "_mapsize" },		/* 14 */
	{ "_coremap" },		/* 15 */
	{ "_swapmap" },		/* 16 */
	{ "_msgbufp" },		/* 17 */
	{ "_nuda" },		/* 18 */
	{ "_uda_sof" },		/* 19 */
	{ "_uda" },		/* 20 */	
	{ "_nra" },		/* 21 */
	{ "_ra_drv" },		/* 22 */
	{ "_ra_ctid" },		/* 23 */
	{ "_time" },		/* 24 */
	{ "_cfree" },		/* 25 */
	{ "_sepid" },		/* 26 */
	{ "_ubmaps" },		/* 27 */
	{ "_ntk" },		/* 28 */
	{ "_tk_soft" },		/* 29 */
	{ "_tk" },		/* 30 */	
	{ "_tk_drv" },		/* 31 */
	{ "_tk_ctid" },		/* 32 */
	{ "_ra_rs" },		/* 33 */
	{ "_ra_rp" },		/* 34 */
	{ "_ra_cp" },		/* 35 */
	{ "_tk_csr" },		/* 36 */
	{ "_nhp" },		/* 37 */
	{ "_nts" },		/* 38 */
	{ "_ub_map" },		/* 39 */
	{ "" },		/* */
};

extern	int	nuda;
extern	int	ntk;
int	nts;
char	nhp[MAXRH];
int	nrh;

struct exec exe;		/* place for exec struct from a.out */


struct	nlist	dnl[] = {
	{ "_hk_ebuf" },
	{ "_hp_ebuf" },
	{ "_ht_ebuf" },
	{ "_ra_ebuf" },
	{ "_rk_ebuf" },
	{ "_rl_ebuf" },
	{ "_rp_ebuf" },
	{ "_hx_ebuf" },
	{ "_tm_ebuf" },
	{ "_ts_ebuf" },
	{ "_tk_ebuf" },
	{ "" },
};

/*
 * flags word is used to indicate what 
 * functions have already been called.
 *
 *	bit	meaning
 *
 *	 0	cdhead called
 *	 1	rainfo called
 *	 2	bioinfo called
 */
int flags;	

#define	CD_SAVE	040	/* crash info saved at 040 on panic trap */
			/* if 040 < 0140000 or > 0144000, no info saved */
			/* 040 = aps (virtual address of PS on stack) */
			/* 042 = ka6 (physical address of U block) */
			/* 044 = current kernel stack pointer */
			/* 046 = first kernel I space PAR */

int	ssr[4];		/* saved M/M status registers */
			/* ssr0, ssr1, ssr2, ssr3 */
int	cputype;
int	rn_ssr3;	/* bits 0->5, saved M/M SSR3 (NOT USED) */
			/* bits 6->15, kernel version number (see machdep.c) */
int	cpereg;		/* -1 if no CPU error register, otherwise contents */
int	mmr3;		/* 0 if no M/M status reg 3, otherwise its address */
int	ksp;		/* current kernel stack pointer */
int	kisa0;		/* first kernel I space PAR */
int	ovno;		/* -1 if NOT overlay kernal */
int	sepid;		/* type of processor */
int	magic;		/* type of kernel  0 == 430; 1 == 431/451 */
unsigned ova[8];	/* text overlay address table */
unsigned ka6;		/* physical address of base of current U block */
unsigned aps;		/* virtual address of top of stack frame. */
int	svi0;		/* address of svi0 */

int	elbsiz;		/* size of error log buffer */
char	*el_bpi;	/* buffer input pointer */
char	*el_bpo;	/* buffer output pointer */
char	*ebuf;		/* local copy of error log buffer */
char	obuf[513] = E_EOB;	/* end of block followed by zeroes */
int	msgbufs;		/* size of kernel saved error messages buffer */
int	msgbufp;		/* pointer in msgbuf area */
int	mapsize;	/* size of core and swap maps */
int	ubmaps;		/* if unibus map present or not */
struct map coremap[300];	/* room for core map */
struct map swapmap[300];	/* room for swap map */
#define UBMAPSZ	12		/* no. of map entries, constant 12 in ubmap.c */
struct map ub_map[UBMAPSZ];	/* room for unibus map */
time_t time;		/* system time - used by red zone printout */

/*
 * Buffer to hold one block I/O device error.
 * Assumes that TMSCP (tk50/tu81) is the largest error!
 * EL_MAXSZ defined in /usr/include/sys/errlog.h
 */
union {
	struct {
		struct	elrhdr	d_hdr;	/* error record header */
		struct	el_bdh	d_bdh;	/* block device header */
		int	d_reg[sizeof(struct tmslg)/2];	/* device info */
	} d_ebuf;
	char	d_max[EL_MAXSZ];
} d_un;

char	msgbuf[MBSMAX+1];	/* console message buffer */

char	*corfil = "/usr/crash/core";
char	*nlfil = "/unix";
char	tmpfil[40];
char	syscmd[100];

char	*help[] =
{
	"\n(cda) - ULTRIX-11 crash dump analysis program",
	"\nUsage:\tcda -key [namelist] [corefile]",
	"\n\tnamelist\tFile containing namelist, default is /unix.",
	"\n\tcorefile\tCrash dump file, default is /usr/crash/core.",
	"\t\t\tIf given then namelist must also be given.",
	"\n\t-key\t\tThe operation(s) to be performed:",
	"\t\t\tMultiple keys may be given.",
	"\n\t\th\tPrint the help message.",
	"\n\t\te\tLocate, format, & print any unlogged errors.",
	"\n\t\tm\tPrint the memory usage map.",
	"\n\t\tb\tPrint the I/O buffer pool usage.",
	"\n\t\tt\tFormat & print the panic trap error stack frame.",
	"\n\t\tr\tPrint the resource maps (core and swap).",
	"\n\t\tq\tPrint the block I/O buffer queues.",
	"\n\t\tu[#]\tPrint MSCP disk information (# = controller).",
	"\n\t\tk\tPrint TMSCP magtape information.",
	"\n\t\tg\tPrint user blocks.",
	"\n\t\tps\tGet process status from crash dump (ps -alx).",
	"\n\t\tpp\tDump the process table, only active slots.",
	"\n\t\tppa\tDump process table, all slots.",
	"\n\t\tpi\tDump the incore inode table.",
	"\n\t\tpf\tDump the incore open file table.",
	"\n\t\tpx\tDump the incore text table.",
	"\n\t\tpt\tPrint the status of all terminal lines.",
	"\n\t\tpu#\tDump the U block for the process at address #.",
	"\nExample:\tcda -et /unix.crash /usr/crash/core.crash",
	"\n\t\tPrint unlogged errors and panic trap stack frame.",
	"\n\n",
	0
};

char	*trapmsg[] =
{
	"4 - Bus error",
	"10 - Reserved instruction",
	"14 - Break point trace",
	"20 - Input/Output trap",
	"24 - Power fail",
	"30 - Emulator trap",
	"34 - Trap instruction (system call from kernel mode)",
	"240 - Programmed interrupt request",
	"244 - Floating point exception",
	"250 - Memory management violation",
};

int	mem;

main(argc, argv)
char	*argv[];
int	argc;
{
	register char	*p, *n;
	register int i;
	int f;
	int ctlr;

	if((argc > 4) || (argc == 1)) {
		phelp();
		exit(0);
	}
	if(argv[1] [0] != '-') {
		printf("\ncda: missing key\n");
		phelp();
		exit(1);
	}
	if(argv[1] [1] == 'h') {
		phelp();
		exit(0);
	}
	if(argc == 4)
		corfil = argv[3];
	if(argc >= 3)
		nlfil = argv[2];
/* find out magic number */
        f = open(nlfil, 0);
        if(f < 0)
                return(-1);
        read(f, (char *)&exe, sizeof exe);
	if((exe.a_magic == 0431) || (exe.a_magic == 0451))
		magic = 1;
	close(f);

	nlist(nlfil, nl);
	if(nl[0].n_type == 0) {
		printf("\ncda: Can't access namelist in %s\n", nlfil);
		exit(1);
	}
	if((mem = open(corfil, 0)) < 0) {
		printf("\ncda: Can't open corefile %s\n", corfil);
		exit(1);
	}
	nlist(nlfil, dnl);
	lseek(mem, (long)nl[4].n_value, 0);
	read(mem, (char *)&cputype, sizeof(cputype));
	lseek(mem, (long)nl[5].n_value, 0);
	read(mem, (char *)&ovno, sizeof(ovno));
	if(ovno >= 0) {	/* overlay kernel, get overlay addresss table */
		lseek(mem, (long)nl[11].n_value, 0);
		read(mem, (char *)&ova[0], sizeof(ova));
	}
	if(nl[37].n_value) {
		lseek(mem, (long)nl[37].n_value, 0);
		read(mem, (char *)&nhp, sizeof(nhp));
		for(nrh=0; nhp[nrh]; nrh++);
	}
	if(nl[38].n_value) {
		lseek(mem, (long)nl[38].n_value, 0);
		read(mem, (char *)&nts, sizeof(nts));
	}
	if(nl[28].n_value) {
		lseek(mem, (long)nl[28].n_value, 0);
		read(mem, (char *)&ntk, sizeof(ntk));
	}
	if(nl[18].n_value) {
		lseek(mem, (long)nl[18].n_value, 0);
		read(mem, (char *)&nuda, sizeof(nuda));
	}
	lseek(mem, (long)nl[26].n_value, 0);
	read(mem, (char *)&sepid, sizeof(sepid));
	lseek(mem, (long)nl[7].n_value, 0);
	read(mem, (char *)&rn_ssr3, sizeof(rn_ssr3));
	lseek(mem, (long)nl[8].n_value, 0);
	read(mem, (char *)&cpereg, sizeof(cpereg));
	lseek(mem, (long)nl[9].n_value, 0);
	read(mem, (char *)&mmr3, sizeof(mmr3));
	lseek(mem, (long)nl[10].n_value, 0);
	read(mem, (char *)&ssr[0], sizeof(ssr));
	lseek(mem, (long)nl[14].n_value, 0);
	read(mem, (char *)&mapsize, sizeof(mapsize));

	lseek(mem, (long)nl[15].n_value, 0);
	read(mem, (char *)&coremap, mapsize);
	lseek(mem, (long)nl[16].n_value, 0);
	read(mem, (char *)&swapmap, mapsize);
	lseek(mem, (long)nl[17].n_value, 0);
	read(mem, (char *)&msgbufp, sizeof(msgbufp));

	lseek(mem, (long)nl[27].n_value, 0);
	read(mem, (char *)&ubmaps, sizeof(ubmaps));
	if(ubmaps) {
		lseek(mem, (long)nl[39].n_value, 0);
		read(mem, (char *)&ub_map, UBMAPSZ);
	}
	lseek(mem, (long)CD_SAVE, 0);
	read(mem, (char *)&aps, sizeof(aps));
	read(mem, (char *)&ka6, sizeof(ka6));
	read(mem, (char *)&ksp, sizeof(ksp));
	read(mem, (char *)&kisa0, sizeof(kisa0));
	p = &msgbuf[0];
	for(i=0; i<(MBSMAX+1); i++)
		*p++ = 0;
	lseek(mem, (long)nl[13].n_value, 0);
	read(mem, (char *)&msgbufs, sizeof(msgbufs));
	if(msgbufs > MBSMAX)
		i = MBSMAX;
	else
		i = msgbufs;
	lseek(mem, (long)nl[12].n_value, 0);
	read(mem, (char *)&msgbuf, i);


	p = argv[1];
	for( ;;)
	{
		switch(*p++) {
		case 0:
			exit(0);
		case '-':
			break;
		case 'm':		/* memory usage map (memstat) */
			if(!(flags & CDHEAD))
				cdhead();
			printf("\n****** Map of memory usage ******\n");
			fflush(stdout);
			sprintf(&syscmd, "memstat -fn %s %s", corfil, nlfil);
			system(&syscmd);
			fflush(stdout);
			break;
		case 'r':		/* core and swap map */
			if(!(flags & CDHEAD))
				cdhead();
			printf("\n****** Resource map contents ******\n\n");
			fflush(stdout);
			rcmd();
			fflush(stdout);
			break;
		case 'u':		/* mscp disk info */
			if(!(flags & CDHEAD))
				cdhead();
			ctlr = 0;
			if((*p >= '0') && (*p <= '9')) {
				ctlr = atoi(p++);
				ctlr++;
			}
			printf("\n****** Mscp disk information ******");
			fflush(stdout);
			ucmd(ctlr);
			fflush(stdout);
			break;
		case 'k':		/* tmscp magtape info */
			if(!(flags & CDHEAD))
				cdhead();
			ctlr = 0;
			if((*p >= '0') && (*p <= '9')) {
				ctlr = atoi(p++);
				ctlr++;
			}
			printf("\n****** Tmscp magtape information ******");
			fflush(stdout);
			kcmd(ctlr);
			fflush(stdout);
			break;
		case 'q':		/* buffer queues */
			if(!(flags & CDHEAD))
				cdhead();
			qcmd();
			fflush(stdout);
			break;
		case 'g':		/* getu */
			if(!(flags & CDHEAD))
				cdhead();
			if((*p >= '0') && (*p <= '7')) {
				n = &tmpfil;
				while(*p >= '0' && *p <= '7')
					*n++ = *p++;
				*n++ = '\0';
				sprintf(&syscmd, "/usr/crash/getu -%s %s %s", tmpfil, nlfil, corfil);
			} else
				sprintf(&syscmd, "/usr/crash/getu %s %s", nlfil, corfil);
			printf("\n****** Ublock information ******\n\n");
			fflush(stdout);
			system(&syscmd);
			fflush(stdout);
			break;
		case 'b':		/* I/O buffer pool usage (bufstat) */
			if(!(flags & CDHEAD))
				cdhead();
			printf("\n****** I/O buffer pool usage ******\n");
			fflush(stdout);
			sprintf(&syscmd, "bufstat -fn %s %s", corfil, nlfil);
			system(&syscmd);
			fflush(stdout);
			break;
		case 'e':		/* unlogged errors */
			if(!(flags & CDHEAD))
				cdhead();
			printf("\n****** Analysis of unlogged errors ******\n");
			fflush(stdout);
			ecmd();
			fflush(stdout);
			break;
		case 't':		/* panic trap stack frame */
			if(!(flags & CDHEAD))
				cdhead();
			if(rzchk() == 0) {
		printf("\n****** Dump of panic trap error stack frame ******\n");
				tcmd();
			}
			fflush(stdout);
			break;
		case 'p':		/* ps -alx or pstat */
			if(!(flags & CDHEAD))
				cdhead();
			if(*p == 's') {
				p++;
		printf("\n****** Status of active processes ******\n");
printf("\nCAUTION !, if the process is swapped (F = 0) or being swapped (F = 11)");
	printf("\n         , then TTY, TIME and CMD could be erroneous !\n");
				fflush(stdout);
				sprintf(&syscmd, "ps -alx %s %s",nlfil, corfil);
				system(&syscmd);
			}
		loop:
			if(*p == 0)
				break;
			switch(*p++) {
			case 'p':
				if(*p == 'a') {
		printf("\n****** Dump of all process table slots ******\n");
					p++;
					fflush(stdout);
			sprintf(&syscmd, "pstat -pa %s %s", corfil, nlfil);
					system(&syscmd);
				} else {
	printf("\n****** Dump of only active process table slots ******\n");
					fflush(stdout);
			sprintf(&syscmd, "pstat -p %s %s", corfil, nlfil);
					system(&syscmd);
				}
				fflush(stdout);
				break;
			case 'i':
		printf("\n****** Dump of the incore inode table ******\n");
					fflush(stdout);
			sprintf(&syscmd, "pstat -i %s %s", corfil, nlfil);
				system(&syscmd);
				fflush(stdout);
				break;
			case 'f':
	printf("\n****** Dump of the incore open file table ******\n");
				fflush(stdout);
			sprintf(&syscmd, "pstat -f %s %s", corfil, nlfil);
				system(&syscmd);
				fflush(stdout);
				break;
			case 'x':
		printf("\n****** Dump of the incore text table ******\n");
				fflush(stdout);
			sprintf(&syscmd, "pstat -x %s %s", corfil, nlfil);
				system(&syscmd);
				fflush(stdout);
				break;
			case 't':
		printf("\n****** Status of all terminal lines ******\n");
				fflush(stdout);
			sprintf(&syscmd, "pstat -t %s %s", corfil, nlfil);
				system(&syscmd);
				fflush(stdout);
				break;
			case 'u':
				n = &tmpfil;
				while(*p >= '0' && *p <= '7')
					*n++ = *p++;
				*n++ = '\0';
  printf("\n****** Dump of U block for process at address %s ******\n", tmpfil);
				fflush(stdout);
		sprintf(&syscmd, "pstat -u %s %s %s ", tmpfil, corfil, nlfil);
				system(&syscmd);
				fflush(stdout);
				break;
		default:
			printf("\ncda: bad -p option\n");
			break;
		}
		goto loop;

			break;
		default:
			printf("\ncda: Unknown key\n");
			exit(1);
		}
	}
}

phelp()
{
	register int i;

	for(i=0; help[i]; i++)
		printf("\n%s", help[i]);
}

ecmd()
{
	register char	*ip, *op;
	register char siz;
	char	*lim;
	int	fd;
	int	found;
	int	i, j, k;

	lseek(mem, (long)nl[1].n_value, 0);
	read(mem, (char *)&elbsiz, sizeof(elbsiz));
	lseek(mem, (long)nl[2].n_value, 0);
	read(mem, (char *)&el_bpi, sizeof(el_bpi));
	lseek(mem, (long)nl[3].n_value, 0);
	read(mem, (char *)&el_bpo, sizeof(el_bpo));
	printf("\nError log buffer at\t%06.o", nl[0].n_value);
	printf("\nBuffer size in words  =\t%06.d",elbsiz);
	printf("\nBuffer input  pointer =\t%06.o", el_bpi);
	printf("\nBuffer output pointer =\t%06.o\n",el_bpo);
	ebuf = calloc((elbsiz+1), sizeof(int));
	if(ebuf == NULL) {
		printf("\ncda: Can't get more memory for buffer\n");
		exit(1);
	}
	lseek(mem, (long)nl[0].n_value, 0);
	read(mem, (char *)ebuf, (elbsiz*2));
	sprintf(&tmpfil, "/tmp/cda_el.%d", getpid());
	if((fd = creat(&tmpfil, 0644)) < 0) {
		printf("\ncda: Can't create %s\n", &tmpfil);
		exit(1);
	}
	if(el_bpi == el_bpo) {
		printf("\n****** No unlogged errors in kernel buffer ******\n");
		close(fd);
		goto e_last;
	}
	ip = ebuf + (el_bpi - nl[0].n_value);
	op = ebuf + (el_bpo - nl[0].n_value);
	lim = ebuf + (elbsiz*2);
e_loop:
	if(ip == op)
		goto e_done;
	if(op >= lim) {
		op = ebuf;
		goto e_loop;
	}
	if(*op == 0) {
		op = ebuf;
		goto e_loop;
	}
	if(*op < 0 || *op > E_BD) {	/* bad record type */
		printf("\ncda: bad error log record - type\n");
		goto badrec;
	}
	siz = *(op + 1);
	if((siz <= 0) || (siz & 1) || (siz > EL_MAXSZ)) {	/* bad size */
		printf("\ncda: bad error log record - size\n");
	badrec:
		printf("\n     error report terminated\n");
		goto e_done;
	}
	if(write(fd, (char *)op, siz) != siz) {
	wrterr:
		printf("\ncda: write error\n");
		exit(1);
	}
	if(write(fd, (char *)obuf, (512 - siz)) != (512 - siz))
		goto wrterr;
	op += siz;
	goto e_loop;
e_done:
	if(write(fd, (char *)&obuf+1, 512) != 512)
		goto wrterr;
	close(fd);
	fflush(stdout);
	sprintf(&syscmd, "elp %s", &tmpfil);
	system(&syscmd);
/*
 * Print the last error on each block I/O device.
 * The error is located in the device's error buffer.
 * This is done because,
 * when a `panic' occurs, errors that have been
 * passed from the kernel error log buffer to
 * the error log copy process (elc) can be lost if the
 * panic crashes unix before elc can write the error to
 * the disk error log file.
 */

e_last:
	siz = 0;
	for(i=0; (dnl[i].n_name[0] != 0); i++)
		siz += dnl[i].n_value;
	if(siz == 0) {
		printf("\n****** Can't locate device error buffers ******\n");
		goto e_xit;
	}
	if((fd = open(&tmpfil, 2)) < 0) {
		printf("\ncda: Can't open %s\n", &tmpfil);
		exit(1);
	}
	found = 0;
	for(i=0; (dnl[i].n_name[0] != 0); i++) {
	    if(dnl[i].n_value == 0)
	    	continue;	/* device not configured */
	    j = 1;
	    if(nrh && (strncmp(dnl[i].n_name, "_hp_ebuf", 8) == 0))
		j = nrh;
	    if(nuda && (strncmp(dnl[i].n_name, "_ra_ebuf", 8) == 0))
		j = nuda;
	    if(nts && (strncmp(dnl[i].n_name, "_ts_ebuf", 8) == 0))
		j = nts;
	    if(ntk && (strncmp(dnl[i].n_name, "_tk_ebuf", 8) == 0))
		j = ntk;
	    lseek(mem, (long)dnl[i].n_value, 0);	/* find dev error buf */
	    for(k=0; k<j; k++) {
		read(mem, (char *)&d_un.d_ebuf, EL_MAXSZ);	/* get contents */
		if(d_un.d_ebuf.d_hdr.e_time == 0)
		    continue;	/* no error in buffer */
		found++;
		d_un.d_ebuf.d_hdr.e_type = E_BD;
		siz = sizeof(struct elrhdr) +
		    sizeof(struct el_bdh) +
		    (d_un.d_ebuf.d_bdh.bd_nreg * 2);
		    d_un.d_ebuf.d_hdr.e_size = siz;	/* actual record size */
		if(write(fd, (char *)&d_un.d_ebuf, siz) != siz)
		    goto wrterr;
		if(write(fd, (char *)&obuf, (512 - siz)) != (512 - siz))
		    goto wrterr;
	    }
	}
	if(write(fd, (char*)&obuf+1, 512) != 512)
		goto wrterr;
	close(fd);
	printf("\n****** Last error on each Block I/O device ******\n");
	if(found == 0) {
		printf("\n\n****** No errors found ******\n");
		goto e_xit;
	}
	sprintf(&syscmd, "elp %s", &tmpfil);
	fflush(stdout);
	system(&syscmd);
e_xit:
	unlink(&tmpfil);
}

/*
 * Fred Canter -- 8/17/85
 *
 * Stack frame for all type kernels currently used
 * by ULTRIX-11 (430, 431, 450, 451), 411 is dead.
 *
 *	Offset sf[#]	Contents
 *	------------	--------
 *		13	PS
 *		12	PC
 *		11	R0
 *		10	NPS
 *		 9	OV (at error time)
 *		 8	R1
 *		 7	SP (previous space)
 *		 6	DEV
 *		 5	TPC
 *		 4	R5
 *		 3	OV (current)
 *		 2	R4
 *		 1	R3
 *		 0	R2
 */
int	sf[14];	/* copy of panic trap error stack frame, reverse order */
int	pco[] = {-6, -4, -2, 0, 2, 4};
int	pcdata[6];

tcmd()
{
	register int i;
	long	sfa;
	int	sfs;
	char	ov;
	int	dev;
	unsigned	vpc;
	long	ppc;

	if((aps < 0140000) || (aps > 0144000)) {
		printf("\nNO panic trap error status saved !");
		printf("\nAre you sure the error was a panic trap ?\n");
		return;
	}
	sfa = ka6;
	sfa = sfa << 6;
	sfa += (aps - 0140000);
	if(ovno < 0) {		/* sep I&D kernel - w/o overlays */
		ov = 1;			/* was 0 */
		/*sfa -= 026; */
		sfs = 26;		/* was 24 */
	} else {
		ov = 1;
		/*sfa -= 030; */
		sfs = 28;
	}
	sfa -= 032;

	lseek(mem, (long)sfa, 0);
	read(mem, (char *)&sf[0], sfs);
	printf("\nGeneral Registers at error time:");
	printf("\n\tR0\t%06.o", ov ? sf[11] : sf[9]);
	printf("\n\tR1\t%06.o", ov ? sf[8] : sf[7]);
	printf("\n\tR2\t%06.o", sf[0]);
	printf("\n\tR3\t%06.o", sf[1]);
	printf("\n\tR4\t%06.o", sf[2]);
	printf("\n\tR5\t%06.o", ov ? sf[4] : sf[3]);
	printf("\n\tSP\t%06.o", ov ? sf[7] : sf[6]);
	printf("\t(from previous space)");
	printf("\n\tSP\t%06.o\t(after panic trap)", ksp);
	printf("\n\nMemory Management Status Registers:");
	printf("\n\tMMR0\t%06.o", ssr[0]);
	if(sepid) /* only separate I & D CPUs have MMR1 */
		printf("\n\tMMR1\t%06.o", ssr[1]);
	printf("\n\tMMR2\t%06.o", ssr[2]);
	if(mmr3 == 0172516)
		printf("\n\tMMR3\t%06.o", ssr[3]);
	if(cpereg != -1) {
		printf("\n\nCPU Error Register = %06.o", cpereg);
		if(cpereg&0200)
			printf("\n\tIllegal halt");
		if(cpereg&0100)
			printf("\n\tAddress error");
		if(cpereg&040)
			printf("\n\tNon-existent memory");
		if(cpereg&020)
			printf("\n\tI/O bus time-out");
		if(cpereg&010)
			printf("\n\tYellow zone stack limit");
		if(cpereg&04)
			printf("\n\tRed zone stack limit");
	}
	printf("\n\nka6 = %o aps = %o", ka6, aps);
	vpc = ov ? sf[12] : sf[10];
	printf("\nUpdated program counter\t= %06.o", vpc);
	printf("\nProcessor status word\t= %06.o", ov ? sf[13] : sf[11]);
	if(ovno != -1) {
		printf("\nCurrent kernel overlay\t= %d", sf[3]);
		printf("\nOverlay at error time\t= %d", sf[9]); 
	}
	dev = ov ? sf[6] : sf[5];
	if(dev & 020)
		goto nopc;	/* can't do user pc */
/*
 * sepid kernel now also overlaid using last i page 
 * -- Bill Burns 8/15/84
 */
	if(magic) {		/* 431/451 kernel */
		if(vpc < 0160000) 
			ppc = kisa0 + (((vpc >> 13) & 07) * 0200);
		else
			ppc = ova[sf[9]];
	} else if(vpc < 040000) {  /* 430 kernel */
		ppc = vpc;	/* in root text segment */
		goto tcmd1;
	} else
		ppc = ova[sf[9]];	/* was sf[3] */
/****************************** old stuff for non overlaid sepid
 *	if(ovno < 0)
 *		 ppc = kisa0 + (((vpc >> 13) & 07) * 0200);
 *	else if(vpc < 040000) {
 *		ppc = vpc;	
 *		goto tcmd1;
 *	} else
 *		ppc = ova[sf[9]];	
 *********************************************************************/
	ppc = ppc << 6;
	ppc += (vpc & 017777);
tcmd1:
	lseek(mem, (long)ppc+pco[0], 0);
	read(mem, (char *)&pcdata[0], sizeof(pcdata));
	printf("\n\nPC\tPhysical");
	printf("\noffset\taddress  Contents");
	for(i=0; i<6; i++) {
		printf("\n");
		if(pco[i] > 0)
			printf("+");
		printf("%d",pco[i]);
		printf("\t%06.O",(ppc+pco[i]));
		printf("   %06.o", pcdata[i]);
	}
nopc:
	printf("\n\nCPU was in ");
	if(dev & 020)
		printf("USER");
	else
		printf("KERNEL");
	printf(" mode at the time of the trap");
	printf("\nThe error type was (");
	if((dev & 017) > 9)
		printf("UNKNOWN)\n");
	else
		printf("trap thru location %s)", trapmsg[dev&017]);
	if(((dev & 017) == 2) && (vpc == 2)) {	/* jump -> 0 */
		printf("\n(trap type = 2, pc = 2 - most likely");
		printf(" a jump to location zero !)");
	}
	if(((dev & 017) == 0) && (!magic) && (vpc == 5)) {   /* vector -> 0 */
		printf("\n(trap type = 0, pc = 5 - most likely");
		printf(" a vector through location zero !)");
	}
	printf("\n");
}

/*
 * Prints crash dump heading message
 */

cdhead()
{
	register char rn, rnd;
	register int i;
	char	rnv;
	int	diff;
	
	flags |= CDHEAD;
	rn = ((rn_ssr3 >> 12) & 017) + '0';
	rnd = ((rn_ssr3 >> 8) & 017) + '0';
	if(rn_ssr3 & 0100)
		rnv = 'X';
	else if(rn_ssr3 & 0200)
		rnv = 'Y';
	else
		rnv = 'V';
	printf("\n\nCrash Dump Analysis of ULTRIX-11 ");
	printf("%c%c.%c Kernel\n", rnv, rn, rnd);
	if(magic == 1)
		printf("Split I/D space overlay version");
	else
		printf("Non-split I/D space overlay version");
	printf(" on 11/%d CPU\n", cputype);
	if(msgbufs > MBSMAX)
		i = MBSMAX;
	else
		i = msgbufs;
    printf("\n****** Last %d characters of console messages ******", i);
	if(msgbufs > MBSMAX) {
	    printf("\n(kernel message buffer size exceeds %d bytes, ", MBSMAX);
	   printf("last %d bytes lost !)\n", (msgbufs - MBSMAX));
	} else
		printf("\n");
/* print out mesg buf in order - Bill 6/84 */	
	diff = (msgbufp - nl[12].n_value);
	printf("\n%s", (msgbuf + diff));
	*(msgbuf + diff) = '\0';
	printf("%s\n", msgbuf);
}

/*
 * Check for red zone stack violation.
 * Saved stack pointer equal to zero indicates red zone.
 * Print CPU error register if it exists.
 */

rzchk()
{
	int	pc, ps;

	if(ksp)
		return(0);
	lseek(mem, 0L, 0);
	read(mem, (char *)&pc, sizeof(int));
	read(mem, (char *)&ps, sizeof (int));

	lseek(mem, (long)nl[24].n_value, 0);
	read(mem, (char *)&time, sizeof(time));

	printf("\n\n****** RED ZONE STACK VIOLATION ******\n");
	printf("\nUpdated Program Counter\t= %06.o", pc);
	printf("\nProcessor Status Word\t= %06.o", ps);
	if(cpereg >= 0)
		printf("\nCPU Error Register\t= %06.o", cpereg);
	printf("\nError date and time\t= %s", ctime(&time));
	printf("\n\n");
	return(1);
}

/*
 * print core and swap maps
 */

rcmd()
{
	int i, j, k, x, y, xall, yall;
	struct map *cmp, *smp, *ump;

	j = k = x = y = xall = yall = 0;
 	for(i = 0; i < mapsize; i++) {		/* size the maps */
		if(x) {
			if(coremap[i].m_size || coremap[i].m_addr)
				xall++;
		} else if(coremap[i].m_size) {
			j++;
		} else {
			x++;
		}
		if(y) {
			if(swapmap[i].m_size || swapmap[i].m_addr)
				yall++;
		} else if(swapmap[i].m_size) {
			k++;
		} else {
			y++;
		}
	}
	printf("Mapsize = %d\n\n", mapsize);
	printf("%16s%15s%16s%15s\n\n", "Core Map", "", "Swap Map", "");
	printf("%10s%11s%10s%10s%11s\n", "Size", "Address", "", "Size", "Address");

	if(xall || yall)
 		for(i = 0; i < mapsize; i++) {
			cmp = &coremap[i];
			smp = &swapmap[i];
			printf("%10u%5s%06o", cmp->m_size, "", cmp->m_addr);
			printf("%10s", "");
			printf("%10u%5s%06o\n", smp->m_size, "", smp->m_addr);
		}
	else
		for(i = 0; i < mapsize ; i++, j--, k--) {
			cmp = &coremap[i];
			smp = &swapmap[i];
			if(j >= 0)
				printf("%10u%5s%06o", cmp->m_size, "", cmp->m_addr);
			else if (j == -1)
				printf("%3d %17s", (mapsize - i), "more zero entries");
			else
				printf("%21s", "");
			printf("%10s", "");
			if(k >= 0)
				printf("%10u%5s%06o\n", smp->m_size, "", smp->m_addr);
			else if (k == -1)
				printf("%3d %17s\n", (mapsize - i), "more zero entries");
			else
				printf("%21s\n", "");
			if((j < -1) && (k < -1))
				break;
		}
	if(!ubmaps)
		return;
	j = x = xall = 0;
 	for(i = 0; i < UBMAPSZ; i++) {		/* size the maps */
		if(x) {
			if(ub_map[i].m_size || ub_map[i].m_addr)
				xall++;
		} else if(ub_map[i].m_size) {
			j++;
		} else {
			x++;
		}
	}
	printf("%16s\n\n", "Unibus Map");
	printf("%10s%11s\n", "Size", "Address");

	if(xall )
 		for(i = 0; i < UBMAPSZ; i++) {
			ump = &ub_map[i];
			printf("%10u%5s%06o\n", ump->m_size, "", ump->m_addr);
		}
	else
		for(i = 0; i < UBMAPSZ ; i++, j--) {
			ump = &ub_map[i];
			if(j >= 0)
				printf("%10u%5s%06o\n", ump->m_size, "", ump->m_addr);
			else if (j == -1)
				printf("%3d %17s\n", (UBMAPSZ - i), "more zero entries");
			if((j < -1) && (k < -1))
				break;
		}
}
