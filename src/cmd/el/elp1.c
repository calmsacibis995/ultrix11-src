
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)elp1.c	3.0	4/21/86";
/*
 * ULTRIX-11 error log report program (elp) - PART 1
 * Fred Canter 3/16/83
 * Chung-Wu Lee 2/15/85  -  add tk50
 *
 * USAGE: elp [-h] [-s] [-f] [-b] [-r] [-u] [-d sd ed] [-et[#]] [file]
 *
 * The input for the report is taken from the current error
 * log on /dev/errlog or from [file].
 *
 * PART 1 is the main() of elp, which does the following:
 *
 * 1.	Prints the help message, if requested.
 *
 * 2.	Processes the argument list.
 *
 * 3.	Find the location and size of the error log and
 *	open it for reading.
 *
 * 4.	Calls the ersum() function (elp2.c) for the summary report.
 *
 * 5.	Calls the erfull() function (elp4.c) for the
 *	full report, unless only a summary was requested.
 *
 */

#include "elp.h"	/* must be first (param.h) */
#include <sys/errlog.h>
#include <sys/devmaj.h>
#include <sys/ra_info.h>
#include <a.out.h>
#include <stdio.h>

/*
 * System error log information,
 * obtained from unix via the errlog system call (EL_INFO).
 */
struct	el_data	el_data;
daddr_t	el_sb;		/* error log start block number */
int	el_nb;		/* error log length in blocks */
int	nblkdev;	/* maximum number of block devices */
int	nchrdev;	/* maximum number of character devices */


/* Name of error type, for selective */

char	etn[3];

/*
 * The el_info structure is used throughout elp,
 * it contains all data, or pointers to it, needed
 * to describe each type of error and is set up so that
 * if something like the order of the major device numbers
 * is changed, only a recompile of elp should be required.
 *
 * Not all error types use all of the info in el_data,
 * for pointers 0 denotes unused, and for all other data
 * types -1 indicates that they should be ignored.
 *
 */

struct el_info elc[] =
{
	"dummy",0,-1,0,0,-1,-1,0,0,0,-1,

	"su",		/* error type name */
	E_SU,		/* index to header message, et_head[] */
	-1,		/* index to device name, dntab[] */
	0,		/* pointer to device reg. control, ??_rbp */
	0,		/* pointer to device reg name text, ??_reg */
	-1,		/* BLOCK major device number */
	-1,		/* RAW major device number */
	0,		/* total error count */
	0,		/* pointer to hard error count */
	0,		/* pointer to soft error count */
	-1,		/* error retry count */

	"sd",E_SD,-1,0,0,-1,-1,0,0,0,-1,
	"tc",E_TC,-1,0,0,-1,-1,0,0,0,-1,
	"si",E_SI,-1,0,0,-1,-1,0,0,0,-1,
	"sv",E_SV,-1,0,0,-1,-1,0,0,0,-1,
	"mp",E_MP,-1,0,0,-1,-1,0,0,0,-1,

	"rk",E_BD,0,&rk_rbp,&rk_reg,RK_BMAJ,RK_RMAJ,0,&rk_hec,&rk_sec,10,
	"rp",E_BD,1,&rp_rbp,&rp_reg,RP_BMAJ,RP_RMAJ,0,&rp_hec,&rp_sec,10,
/* rx, rd, and rc get converted to ra ! */
	"ra",E_BD,2,&ra_rbp,&ra_reg,RA_BMAJ,RA_RMAJ,0,0,0,2,
	"rl",E_BD,3,&rl_rbp,&rl_reg,RL_BMAJ,RL_RMAJ,0,&rl_hec,&rl_sec,10,
/* rx changed to hx because of RX50 */
	"hx",E_BD,4,&rx_rbp,&rx_reg,HX_BMAJ,HX_RMAJ,0,&rx_hec,&rx_sec,10,
	"tm",E_BD,5,&tm_rbp,&tm_reg,TM_BMAJ,TM_RMAJ,0,&tm_hec,&tm_sec,9,
/* tc changed to dt because of time change */
	"tk",E_BD,6,&tk_rbp,&tk_reg,TK_BMAJ,TK_RMAJ,0,&tk_hec,&tk_sec,2,
	"ts",E_BD,7,&ts_rbp,&ts_reg,TS_BMAJ,TS_RMAJ,0,&ts_hec,&ts_sec,9,
	"ht",E_BD,8,&ht_rbp,&ht_reg,HT_BMAJ,HT_RMAJ,0,&ht_hec,&ht_sec,9,
/*
 * erp & ernp are zero for HP because it has three
 * drive types to contend with, RP RM ML.
 * The code in elp4.c checks the drive type register
 * and sets the correct values into erp & ernp.
 */
	"hp",E_BD,9,/*&hp_rbp*/ 0,/*&hp_reg*/ 0,HP_BMAJ,HP_RMAJ,0,&hp_hec,&hp_sec,28,
	"hm",E_BD,10,/*&hp_rbp*/ 0,/*&hp_reg*/ 0,HM_BMAJ,HM_RMAJ,0,&hm_hec,&hm_sec,28,
	"hj",E_BD,13,/*&hp_rbp*/ 0,/*&hp_reg*/ 0,HJ_BMAJ,HJ_RMAJ,0,&hj_hec,&hj_sec,28,
	"hk",E_BD,11,&hk_rbp,&hk_reg,HK_BMAJ,HK_RMAJ,0,&hk_hec,&hk_sec,28,
/*
 * The following are the character devices.
 * They are not real error types, these entries are
 * only used to map to the device name array dntab[].
 * Their values are changed dynamically.
 */
	"console",E_EOF,0,0,-1,-1,CO_RMAJ,0,0,0,-1,
	"pc",E_EOF,1,0,-1,-1,-1,0,0,0,-1,
	"lp",E_EOF,2,0,-1,-1,LP_RMAJ,0,0,0,-1,
	"dc",E_EOF,3,0,-1,-1,DC_RMAJ,0,0,0,-1,
	"dh",E_EOF,4,0,-1,-1,DH_RMAJ,0,0,0,-1,
	"dp",E_EOF,5,0,-1,-1,DP_RMAJ,0,0,0,-1,
	"uh",E_EOF,6,0,-1,-1,UH_RMAJ,0,0,0,-1,
	"dn",E_EOF,7,0,-1,-1,DN_RMAJ,0,0,0,-1,
	"dz",E_EOF,8,0,-1,-1,DZ_RMAJ,0,0,0,-1,
	"du",E_EOF,9,0,-1,-1,DU_RMAJ,0,0,0,-1,
	0
};

/*
 * This is the help message text.
 */

char	*help[] =
{
	"\n\n(elp) - ULTRIX-11 error log report generator.",
	"\nThis command formats and prints error reports based",
	"on the error data captured by the error logger.",
	"\nUsage:\n",
	"\telp [-h] [-s] [-f] [-b] [-r] [-u] [-d sd ed] [-et[#]] [file]",
	"\nelp\tWith no arguments,",
	"\ta summary of all errors is printed followed",
	"\tby a detailed report on each error.",
	"\n-h\tPrint this help message.",
	"\n-s\tPrint only the error summary report.",
	"\n-f\tPrint only the detailed error report.",
	"\n-b\tPrint only breif decsriptions of each error.",
	"\n-r\tFor block device errors, print only recovered errors.",
	"\n-u\tFor block device errors, print only unrecovered errors.",
	"\n-d\tPrint only the errors within the specified",
	"\tdate/time range.",
	"\n\t(sd) -\tStarting date/time (yymmddhhmmss).",
	"\t(ed) -\tEnding date/time (yymmddhhmmss).",
	"\n\tnote -\tAll digits must be present in date/time.",
	"\n-et[#]\tPrint only the error for the specified error type.",
	"\tOnly one error type may be specified.",
	"\t`#' - optional unit number for block device errors only.",
	"\nfile\tTake the input from the specified file",
	"\tinstead of the current error log.",
	0
};

/*
 * Error messages
 */

char	*errmsg[] =
{
	"arg count",
	"bad arg",
	"bad error type",
	"bad date limit",
	0,
};


char	sflag;	/* Summary report only */
char	fflag;	/* Full report, skip summary */
char	dflag;	/* Date selective option specified */
char	etflag;	/* Error type selective option specified */
char	fnflag;	/* Input is from specified file, not error log device */

char	*cbp;

int	et;
int	etdn = -1;
int	etct = -1;
int	etcn = -1;

int	timl[6];	/* sec		start date limit */
			/* min */
			/* hour */
			/* day */
			/* month */
			/* year */
int	timh[6];	/* sec		end date limit */
			/* min */
			/* hour */
			/* day */
			/* month */
			/* year */
int	elrtim[6];	/* sec		time from error log record */
			/* min */
			/* hour */
			/* day */
			/* month */
			/* year */

struct	nlist	nl[] =
{
	{ "_ra_ctid" },
	{ "" },
};
char	ra_ctid[MAXUDA];

main (argc, argv)
char	*argv[];
int	argc;
{

	register int i, j;
	register char *p;
	char *filen;
	char *n;
	int mem, fi;

/*
 * Get the error log start block, length
 * and the number of block devices
 * from /unix kernel via the errlog system call.
 */
	errlog(EL_INFO, &el_data);
	el_sb = el_data.el_sb;
	el_nb = el_data.el_nb;
	nblkdev = el_data.nblkdev;
	if(nblkdev > MAXNBD)
		nblkdev = MAXNBD;

/*
 * Set up the character device name index
 */
	nchrdev = 0;
	for(i=(E_BD+nblkdev); elc[i].et; i++) {
		j = elc[i].edn + nblkdev;
		elc[i].edn = j;
		nchrdev++;
		}


	if(argc > 9)
		emsg(0);	/* arg count */
/*
 * Get initial idea of MSCP cntlr types
 * from current kernel ra_ctid[] table.
 * This is needed because to print unlogged errors
 * CDA passes error records to elp, but does not
 * pass any startup records. Without the startup records
 * elp can't tell how many and what type MSCP cntlrs there are.
 */
	nlist("/unix", nl);
	mem = open("/dev/mem", 0);
	if((mem >= 0) && (nl[0].n_value)) {
		lseek(mem, (long)nl[0].n_value, 0);
		read(mem, (char *)&ra_ctid, MAXUDA);
		for(i=0; i<MAXUDA; i++) {
			j = (ra_ctid[i] >> 4) & 017;
			ra_cid[i] = j;
			if(j == 017)
				ra_ctn[i] = "";
			else
				ra_ctn[i] = radntab[j];
		}
	}
	if(argc == 1)		/* no arguments */
		goto elp_go;
	for(i=1; i < argc; i++) {
		p = argv[i];
		if(*p != '-') {	/* arg is a filename */
			fnflag++;
			fi = i;
			continue;
			}
		*p++;			/* skip "-" in front of arg */
		if(strlen(p) > 4)
			emsg(1);	/* bad arg */
		if(strlen(p) >= 2) {	/* arg is an error type */
			etn[0] = *p++;	/* get error type name */
			etn[1] = *p++;
			etn[2] = 0;
			if(*p)
				etdn = atoi(p);	/* get unit number */
			if(etdn >= 64)
				emsg(1);	/* bad arg */
			/*
			 * If the requested error type is rd, rx, or rc
			 * change it to ra, but save the controller type
			 * ID as follows:
			 * ra=0, rc=1, rd=2, rx=3
			 * Can't set the real cntlr ID yet, because it is
			 * not known until after the first startup record
			 * is read in, see setrat().
			 */
			if(etn[0] == 'r') {
				switch(etn[1]) {
				case 'a':
					etct = 0;
					break;
				case 'c':
					etct = 1;
					break;
				case 'd':
					etct = 2;
					break;
				case 'x':
					etct = 3;
					break;
				default:
					etct = -1;
					break;
				}
				if(etct >= 0)
					etn[1] = 'a';
			}
			p = &etn;
			for(j=0; elc[j].et; j++)
				if(strcmp(p, elc[j].et) == 0) {
					et = j;
					etflag++;
					break;
					}
			if(!etflag)
				emsg(2);	/* bad error type */
			if(et < E_BD)
				etdn = -1;
			continue;
			}
		/* arg is a flag */

		if(*p == 'h') {	/* print the help message */
			for(j=0; help[j]; j++)
				printf("\n%s", help[j]);
			printf("\n\n\n");
			exit();
			}
		else if(*p == 's')
			sflag++;
		else if(*p == 'f')
			fflag++;
		else if(*p == 'r')
			rflag++;
		else if(*p == 'u')
			uflag++;
		else if(*p == 'b')
			bflag++;
		else if(*p == 'd') {
			dflag++;
		/* get start/end date from next 2 arg's */

		if((argc - i) < 3)	/* must be min of 2 arg's remaining */
			emsg(3);	/* bad date limit */
		cbp = argv[++i];	/* set up lo date limit */
		timl[5] = gtd();	/* year */
		timl[4] = gtd();	/* month */
		timl[3] = gtd();	/* day */
		timl[2] = gtd();	/* hour */
		timl[1] = gtd();	/* min */
		timl[0] = gtd();	/* sec */
		for(j=0;j<6;j++) {
			if(timl[j] < 0)
				emsg(3);	/* bad date limit */
			}
		timl[5] =+ 1900;	/* add 1900 to year */
		timl[4]--;		/* month - 1 */
		cbp = argv[++i];	/* set up end date limit */
		timh[5] = gtd();
		timh[4] = gtd();
		timh[3] = gtd();
		timh[2] = gtd();
		timh[1] = gtd();
		timh[0] = gtd();
		for(j=0;j<6;j++) {
			if(timh[j] < 0)
				emsg(3);	/* bad date limit */
			}
		timh[5] =+ 1900;
		timh[4]--;
		}
		else
			emsg(1);	/* bad arg */
	}

/*
 * Open the error log device or
 * the optional [file] for reading.
 */

elp_go:
	if(fnflag)
		filen = argv[fi];
	else
		filen = "/dev/errlog";
	if((fi = open(filen, R)) < 0) {
		fprintf(stderr, "\nelp: Can't open %s\n", filen);
		exit(1);
		}

/*
 * This is the mainline of the program.
 */

	ersum(fi, filen);
	if(!sflag)
		erfull(fi, filen);
	close(fi);
	printf("\n\n\n\n\n\n\n\n");
}

/*
 * Get 2 digits from arg & check them 
 */

gtd()
{
	register char c1, c2;
	register char *cp;

	cp = cbp;
	if(*cp == 0)
		return(-1);
	c1 = (*cp++ - '0') * 10;
	if(c1 < 0 || c1 > 100)
		return(-1);
	if(*cp == 0)
		return(-1);
	if((c2 = *cp++ - '0') < 0 || c2 > 9)
		return(-1);
	cbp = cp;
	return(c1 + c2);
}

/*
 * Print an error message and exit.
 */

emsg(m)
{
	fprintf(stderr,"\nelp: ");
	fprintf(stderr,"%s\n", errmsg[m]);
	fprintf(stderr,"\nUsage:");
fprintf(stderr,"\n\telp [-h] [-s] [-f] [-b] [-r] [-u] [-d sd ed] [-et[#]] [file]\n");
	fprintf(stderr,"\nType `elp -h' for help !\n");
	exit(1);
}
