
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)mtx1.c	3.0	4/22/86";
/*
 * ULTRIX-11 mag tape disk exerciser program (mtx).
 *
 * PART 1 - (mtx1.c)
 *
 *	Part 1 does the initial setup and argument processing,
 *	writes the results into a file `mtx_??.arg' (?? = ts, tm, ht),
 *	and then calls part 2 of mtx, which is the actual exerciser.
 *	The mtx program is split into two sections to optimize
 *	memory usage.
 *
 * Fred Canter 6/23/82
 * Bill Burns 4/84
 *	added event flags
 * Chung-Wu Lee 2/22/85
 *	added TMSCP (TK50/TU81)
 *
 * This program exercises the tm11 - tu10/ts03, ts11/tsv05/tu80/tk25,
 * tm02/3 - tu16/te16/tu77 or tk50 - tk50/tu81 mag tape subsystems,
 * using the unix block and raw I/O interfaces at the user level.
 * Each of the four types of tape controllers requires
 * a dedicated copy of this program to exercise that
 * tape controller. There can be a maximum of four
 * copies of (mtx) running, the text segment is shared.
 * Each copy of (mtx) can exercise all drives that
 * are attached to its specified controller.
 * (mtx) can only report on errors that are detected at
 * the user level, i.e., hard errors.
 * Detailed error information must be obtained
 * from the error log (elp -ht, elp -tm, elp -ts or elp -tk).
 *
 * USAGE:
 *
 *	mtx -h		Print the help message.
 *
 *	mtx -z #	event flag bit position, used to start/stop
 *			exercisers
 *
 *	mtx -tm		Select the TM11 controller for testing.
 *	mtx -ts		Select the TS11 controller for testing.
 *	mtx -ht		Select the TM02/3 controller for testing.
 *	mtx -tk		Select the TMSCP controller for testing.
 *
 *	mtx -d#		Select drive (#) for testing.
 *			Multiple [-d#] may be specified.
 *			All drives is default.
 *
 *	mtx -n #	Print (#) data errors per tape record,
 *			default # is 5, maximum is 256.
 *
 *	mtx -e #	Stop exercising a tape drive after # errors,
 *			Default # is 100, maximum is 1000.
 *
 *	mtx -i		Inhibit the drive status message.
 *
 *	mtx -s		Suppress the end of pass tape I/O
 *			statistics printouts.
 *
 *	mtx -f#		Specify (#) feet of tape to be used
 *			for the large file test.
 *			10 feet min, 2400 feet max, & 500 feet default,
 *			10 foot minimum increments, rounded down.
 *			But for TK50, it specify (#) records) of tape to be
 *			used for the large file test.
 *			1 record min, 10000 records max, & 500 records default.
 */

#include <sys/param.h>	/* Don't matter which one ! */
#include <sys/devmaj.h>
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include <signal.h>
#include <a.out.h>
#include <sys/tk_info.h>

#define	RD	0
#define	WRT	1
#define	DMM	2
#define	MEOF	3
#define	MT_OFL	0100
#define	MT_WL	040
#define	MT_OPN	020

/*
 * Mag tape block and raw I/O file names.
 * mt & rmt are for 800 BPI.
 * ht & rht are for 1600 BPI.
 * gt & rgt are for 6250 BPI.
 * tk & rtk are TK50 only.
 */

char	mtn[]	"/dev/mt";
char	rmtn[]	"/dev/rmt";
char	htn[]	"/dev/ht";
char	rhtn[]	"/dev/rht";
char	gtn[]	"/dev/gt";
char	rgtn[]	"/dev/rgt";
char	tkn[]	"/dev/tk";
char	rtkn[]	"/dev/rtk";
char	devn[12];

/*
 * Argument file name
 */

char	afn[] = "mtx_xx.arg";

/*
 * Help message.
 */

char *help[]
{
	"\n\n(mtx) - ULTRIX-11 mag tape exerciser.",
	"\nUsage:\n",
	"\tmtx [-h] [-tm -ts -ht -tk] [-d#] [-n #] [-i] [-s] [-f#]",
	"\n\t-h\tPrint this help message.",
	"\n\t-tm\tSelect TM11 controller for testing.",
	"\t-ts\tSelect TS11/TSV05/TU80/TK25 controller for testing.",
	"\t-ht\tSelect TM02/3 controller for testing.",
	"\t-tk\tSelect TMSCP controller for testing.",
	"\t\tMust select one and only one controller.",
	"\n\t-d#\tSelect drive (#) for testing.",
	"\t\tMultiple drives may be selected.",
	"\t\tThe default is all drives selected.",
	"\n\t-n #\tPrint a maximum of (#) data compare errors",
	"\t\tper tape record. The default is 5 errors printed.",
	"\n\t-i\tInhibit the drive status printout.",
	"\n\t-s\tSuppress the end of pass tape I/O statistics.",
	"\n\t-f#\tSpecify (#) feet of tape to be used",
	"\t\tfor the large file test (test 3).",
	"\t\t10 feet minimum, 2400 feet maximum, 500 feet is default.",
	"\t\t(#) must be specified in 10 foot increments.",
	"\t\tBut for TK50, it specify (#) records of tape to be used",
	"\t\tfor the large file test (test 3).",
	"\t\t1 record min, 10000 records maximum, 500 records is default.",
	"\n\n\n\n",
	0
};

/*
 * Mag tape drive type array.
 * 0 = drive is not selected or special file does not exist.
 * 1 = drive is on tm11 controller & special file exists.
 * 2 = drive is on tm02/3 controller & special file exists.
 * 3 = drive is on ts11 controller & special file exists.
 * 4 = drive is tu81 and is on TMSCP controller & special file exists.
 * 5 = drive is tk50 and is on TMSCP controller & special file exists.
 * For each of the mtx processes the drive types will
 * all be the same.
 */

char	mt_dt[64];

/*
 * Buffer for information returned from stat system call.
 * Used to find the major device number of the tape, in order
 * to know what type of tape controller is on the system.
 */

struct stat statb;

struct nlist nl[] =
{
	{ "_ntk" },
	{ "_tk_ctid" },
	{ "_tk_csr" },
	{ "" },
};

char	tk_ctid[MAXTK];
int	tk_csr[MAXTK];
int	ntk;
int	mem;

int	errno;	/* external error number variable */

int	nfeet[MAXTK];

char	argbuf[512];

int	dflag, iflag, fflag, sflag, tmflag, tsflag, htflag, tkflag;

#ifdef EFLG
char	*efbit;
char	*efids;
int	zflag;
#else
char	*killfn = "mtx.kill";
#endif

int	ndep = 5;
int	ndrop = 100;

main(argc, argv)
char *argv[];
int argc;
{
	int	stop(), intr();
	register int *ap;
	register char *p;
	register int i;
	int 	j, k, fn, gtflag;
	int	dn, fd, md, maxdrvs;
	char	*n;
	char	c;

	setpgrp(0, 31111);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	signal(SIGQUIT, stop);
	fn = 0;
	for(i=1; i < MAXTK; i++)
		nfeet[i] = 0;
	for(i=1; i < argc; i++) {	/* decode arg's */
		p = argv[i];
		if(*p++ != '-') {
		argerr:
			fprintf(stderr,"\nmtx: bad arg\n");
			exit(1);
			}
		switch(*p) {
		case 'h':	/* print the help message */
				/* or select controller type */
			if(*++p == 't') {
				htflag++;	/* tm02/3 */
				break;
				}
			for(j=0; help[j]; j++)
				printf("\n%s",help[j]);
			exit();
#ifdef EFLG
		case 'z':
			zflag++;
			i++;
			efbit = argv[i++];
			efids = argv[i];
			break;
#else
		case 'r':	/* kill file name */
			i++;
			killfn = argv[i];
			break;
#endif
		case 'i':	/* inhibit drive status */
			iflag++;
			break;
		case 'n':	/* # of errors to print */
			i++;
			ndep = atoi(argv[i]);
			if((ndep <= 0) || (ndep > 256))
				ndep = 5;
			break;
		case 'e':	/* drop drive after # errors */
			i++;
			ndrop = atoi(argv[i]);
			if((ndrop <= 0) || (ndrop > 1000))
				ndrop = 100;
			break;
		case 'd':	/* drive select */
			dflag++;
			p++;
			dn = atoi(p);
			if(dn >= 64)
				goto argerr;
			mt_dt[dn] = 1;
			break;
		case 'f':	/* # of feet of tape */
			fflag++;
			p++;
			nfeet[fn++] = atoi(p);
			break;
		case 's':	/* suppress I/O stats */
			sflag++;
			break;
		case 't':	/* select controller type */
			if(*++p == 'm')
				tmflag++;
			if(*p == 's')
				tsflag++;
			if(*p == 'k')
				tkflag++;
			break;
		default:	/* bad argument */
			goto argerr;
		}
	}
	if(!zflag) {
		if(isatty(2)) {
			fprintf(stderr,"mtx: detaching... type \"sysxstop\" to stop\n");
			fflush(stderr);
		}
		if((i = fork()) == -1) {
			printf("mtx: Can't fork new copy !\n");
			exit(1);
		}
		if(i != 0)
			exit(0);
	}
	setpgrp(0, 31111);
/*
 * Verify that one and only one type of tape
 * controller has been selected for testing.
 */

	if((i = tmflag + tsflag + htflag + tkflag) != 1) {
		fprintf(stderr,"\nmtx: Controller specification error\n");
		exit(1);
		}

/*
 * In order not to waste time
 * opening drives that can't exist,
 * the variable "maxdrvs" is set to the maximum
 * number of drives that can exist for the type
 * of controller to exercised.
 * TS = 4, TM = 8, HT = 64, TK = 4
 */
	if(tsflag) {
		maxdrvs = 4;
		afn[4] = 't';
		afn[5] = 's';
	} else if(tmflag) {
		maxdrvs = 8;
		afn[4] = 't';
		afn[5] = 'm';
	} else if(htflag) {
		maxdrvs = 64;
		afn[4] = 'h';
		afn[5] = 't';
	} else {
		nlist("/unix", nl);
		if(nl[0].n_type == 0) {
			fprintf(stderr,"\nmtx: kernel not configured for TMSCP magtape\n");
			exit(1);
		}
		if((mem = open("/dev/mem", 0)) < 0) {
			fprintf(stderr,"\nmtx: can't open /dev/mem\n");
			exit(1);
		}
		lseek(mem, (long)nl[0].n_value, 0);
		read(mem, &ntk, sizeof(ntk));
		if(ntk <= 0) {
			fprintf(stderr,"\nmtx: kernel not configured for TMSCP magtape\n");
			exit(1);
		}
		lseek(mem, (long)nl[1].n_value, 0);
		read(mem, &tk_ctid, sizeof(tk_ctid));
		lseek(mem, (long)nl[2].n_value, 0);
		read(mem, &tk_csr, sizeof(tk_csr));
		maxdrvs = MAXTK;
		afn[4] = 't';
		afn[5] = 'k';
		}

/*
 * IF no drives were selected with -d#,
 * then select all drives.
 */
	if(!dflag) {
		for(i=0; i<maxdrvs; i++)
			mt_dt[i] = 1;
		}

/*
 * Set up the number of feet of tape
 * to be used for large file test.
 *
 * for TMSCP, it is number of blocks
 * to be used for large file test.
 */
	if (!tkflag) {
		if(!fflag)
			nfeet[0] = 500;
		if(nfeet[0] < 10)
			nfeet[0] = 10;
		if(nfeet[0] > 2400)
			nfeet[0] = 2400;
		}
	else {
		for (i=0; i<ntk; i++) {
			if (((tk_ctid[i] >> 4)&017) == TK50) {
				if(!fflag)
					nfeet[i] = 500;
				if(nfeet[i] < 1)
					nfeet[i] = 1;
				if(nfeet[i] > 10000)
					nfeet[i] = 10000;
			} else if (((tk_ctid[i] >> 4)&017) == TU81) {
				if(!fflag)
					nfeet[i] = 500;
				if(nfeet[i] < 10)
					nfeet[i] = 10;
				if(nfeet[i] > 2400)
					nfeet[i] = 2400;
			}
		}
	}

/*
 * Use the stat system call to determine how many of the
 * selected drives have special files and what drive type
 * they are.
 * The special file for the selected unit is checked.
 * It must be a block mode special file.
 * The major device number of the special file is
 * compared with the major device numbers in the
 * devmaj.h header file to determine the drive type.
 * For the tm11 & ts11 the minor device number of the
 * special file must match the unit number.
 * For the tm02/3 at 800 BPI the minor device 
 * number in the special file must match the unit
 * number plus 64.
 * For the tm02/3 at 1600 BPI it must match the unit number.
 */

	for(i=0; i<maxdrvs; i++) {
		if(!mt_dt[i])
			continue;
		if (!tkflag) {
			sprintf(&devn, "%s%d", mtn, i);
			if(tsflag)
				dn = -1;
			else
				dn = stat(&devn, &statb);
			if(dn < 0) {
				sprintf(&devn, "%s%d", htn, i);
				dn = stat(&devn, &statb);
				if(dn < 0) {
					mt_dt[i] = 0;
					continue;
					}
				}
			if((statb.st_mode & S_IFMT) != S_IFBLK)
				goto sferr;	/* not block mode special file */
			md = (statb.st_rdev >> 8) & 077;	/* major device */
			dn = statb.st_rdev & 0377;	/* minor device */
			if(tmflag && md == TM_BMAJ && dn == i)
				mt_dt[i] = 1;	/* tm11 */
			else if(tsflag && md == TS_BMAJ && dn == i)
				mt_dt[i] = 3;	/* ts11 */
			else if(htflag && md == HT_BMAJ && dn == (i + 64)) {
				sprintf(&devn, "%s%d", htn, i);
				dn = stat(&devn, &statb);
				if(dn < 0)
					goto sferr;
				md = (statb.st_rdev >> 8) & 077;
				dn = statb.st_rdev & 0377;
				if(md != HT_BMAJ || dn != i)
					goto sferr;
				mt_dt[i] = 2;	/* tm02/3 */
			} else {
			sferr:
		    	fprintf(stderr,"\nmtx: %s - special file mismatch\n",devn);
				exit(1);
				}
			}
		else {
			if (tk_csr[i]) {
				if (((tk_ctid[i] >> 4)&017) == TK50) {
					sprintf(&devn, "%s%d", tkn, i);
					dn = stat(&devn, &statb);
					if(dn < 0) {
						mt_dt[i] = 0;
						continue;
						}
					mt_dt[i] = 5;
					}
				else if (((tk_ctid[i] >> 4)&017) == TU81) {
					gtflag = 0;
					sprintf(&devn, "%s%d", htn, i);
					dn = stat(&devn, &statb);
					if(dn < 0) {
						sprintf(&devn, "%s%d", gtn, i);
						dn = stat(&devn, &statb);
						gtflag = 1;
						}
					if(dn < 0 && gtflag == 1) {
						mt_dt[i] = 0;
						continue;
						}
					else if (dn < 0 || gtflag == 1)
						goto sferr;
					mt_dt[i] = 4;
					}
				else
					mt_dt[i] = 0;
				}
			else
				mt_dt[i] = 0;
			}
/*
 * Deselect any drive that has a special file
 * but cannot actually be opened, i.e. , is
 * nonexistent, off-line, write locked,
 * or is already open.
 */
		if(mt_dt[i] == 1 || mt_dt[i] == 2) {
			sprintf(&devn, "%s%d", mtn, i);
			if((fd = open(&devn, WRT)) < 0) {
				if(errno == ETOL)
					mt_dt[i] |= MT_OFL;
				else if(errno == ETWL)
					mt_dt[i] |= MT_WL;
				else if(errno == ETO)
					mt_dt[i] |= MT_OPN;
				}
			close(fd);
			}
		if(mt_dt[i] >= 2 && mt_dt[i] <= 4) {
			sprintf(&devn, "%s%d", htn, i);
			if((fd = open(&devn, WRT)) < 0) {
				if(errno == ETOL)
					mt_dt[i] |= MT_OFL;
				else if(errno == ETWL)
					mt_dt[i] |= MT_WL;
				else if(errno == ETO)
					mt_dt[i] |= MT_OPN;
				}
			close(fd);
			}
		if(mt_dt[i] == 4) {
			sprintf(&devn, "%s%d", gtn, i);
			if((fd = open(&devn, WRT)) < 0) {
				if(errno == ETOL)
					mt_dt[i] |= MT_OFL;
				else if(errno == ETWL)
					mt_dt[i] |= MT_WL;
				else if(errno == ETO)
					mt_dt[i] |= MT_OPN;
				}
			close(fd);
			}
		if(mt_dt[i] == 5) {
			sprintf(&devn, "%s%d", tkn, i);
			if((fd = open(&devn, WRT)) < 0) {
				if(errno == ETOL)
					mt_dt[i] |= MT_OFL;
				else if(errno == ETWL)
					mt_dt[i] |= MT_WL;
				else if(errno == ETO)
					mt_dt[i] |= MT_OPN;
				}
			close(fd);
			}
		}
/*
 * Print the drive status.
 */
	j = 0;
	if(!iflag) {
		for(i=0; i<maxdrvs; i++) {
			if(mt_dt[i] == 0)
				continue;
			k = mt_dt[i] & 7;
			if(mt_dt[i] <= 5)
				j++;
			printf("\nUnit %d - ",i);
			if(k == 1)
				printf("tm11 - tu10/ts03");
			else if(k == 2)
				printf("tm02/3 - tu16/te16");
			else if(k == 3)
				printf("ts11/tsv05/tsu05/tu80/tk25");
			else if(k == 4)
				printf("tu81");
			else
				printf("tk50");
			if(mt_dt[i] & MT_OFL) {
				printf(" [off-line]");
				j++;
			}
			else if(mt_dt[i] & MT_WL)
				printf(" [write locked]");
			else if(mt_dt[i] & MT_OPN)
				printf(" [already open]");
		}
		if(j == 0) {
			printf("\n\nNo drives available\n");
			exit(1);
		}
	}

/*
 * Write data needed by mtx2 into the `argbuf',
 * write it out to a file and call mtx2.
 */

	p = &argbuf;
	for(i=0; i<64; i++)
		*p++ = mt_dt[i];
	ap = p;
	*ap++ = sflag;
	*ap++ = tsflag;
	*ap++ = tmflag;
	*ap++ = htflag;
	*ap++ = tkflag;
	for(i=0; i<MAXTK; i++)
		*ap++ = nfeet[i];
	*ap++ = ndep;
	*ap++ = ndrop;
	*ap++ = maxdrvs;
#ifdef EFLG
	*ap++ = zflag;
#endif
	if((fd = creat(afn, 0644)) < 0) {
		fprintf(stderr, "\nmtx: Can't create %s file\n", afn);
		exit(1);
		}
	if(write(fd, (char *)&argbuf, 512) != 512) {
		fprintf(stderr, "\nmtx: %s write error\n", afn);
		exit(1);
		}
	close(fd);
	fflush(stdout);
	signal(SIGQUIT, SIG_IGN);
#ifdef EFLG
	if(zflag)
		execl("mtxr", "mtxr", afn, efbit, efids, (char *)0);
	else
		execl("mtxr", "mtxr", afn, (char *)0);
#else
	execl("mtxr", "mtxr", afn, killfn, (char *)0);
#endif
	fprintf(stderr, "\nmtx: Can't exec mtxr\n");
	exit(1);
}

stop()
{
	exit(0);
}
