
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)rkx1.c	3.0	4/22/86";
/*
 * ULTRIX-11 RK disk exerciser program (rkx).
 *
 * PART 1 - (rkx1.c)
 *
 *	Part 1 does the initial setup and argument processing,
 *	writes the results into a file `rkx_#.arg' (# = drive).
 *	and then calls part 2 of rkx, which is the actual exerciser.
 *	The program is split into two sections to optimize
 *	memory usage.
 *
 * Fred Canter 3/2/83
 * Bill Burns 4/84
 *
 *	********************************************
 *      *                                          *
 *      * This program will not function correctly *
 *      * unless the current unix monitor file is  *
 *      * named "unix".                            *
 *	*					   *
 *      ********************************************
 *
 * This program exercises the RK05 disk
 * via the unix block and raw I/O interfaces, at
 * the user level.
 * It reports the occurrence of hard errors and/or data
 * compare errors on the standard output.
 * The detailed device error information must be obtained
 * the system error log (elp -rk).
 *
 *
 * USAGE:
 *
 *	rkx -h
 *
 *		Print the help message.
 *
 *	rkx [-d#] [-s#] [-i] [-n #] [-e #] [-z #]
 *
 *		Exercise the disk(s), using random addresses,
 *		random byte counts, and alternating worst case
 *		and random data patterns.
 *
 *	[-z #] - event flag bit position, used to start/stop
 *		exercisers
 *
 *	[-d#] -	Select the drive to be exercised.
 *		Only one drive may be selected.
 *
 *	[-s#] -	Specify time intervals for periodic I/O statistics printouts.
 *		The time interval (#) is in minutes and can range
 *		from 1 to 720 (12 hours).
 *		The default time interval is 1/2 hour.
 *		If no time interval is specified (-s only),
 *		the I/O statistics are not printed.
 *
 *	[-i]  -	Inhibit the file system status printouts.
 *
 *	[-n #] - Specify the number (#) of data mismatch errors
 *		to be printer per failing block.
 *		The maximum is 256 and the default is 5.
 *
 *	[-e #] - Drop the disk after # hard errors,
 *		 default = 100, maximum = 1000.
 *
 *
 *	note  -	The root, swap, error log, and any mounted file
 *		systems are automatically write protected.
 *
 * EXAMPLE:
 *		rkx -d1
 *
 *		Exercise drive one.
 *
 *
 */

#include <sys/param.h>	/* Don't matter which one ! */
#include <sys/devmaj.h>
#include <sys/mount.h>
#include <stdio.h>
#include <a.out.h>
#include <signal.h>

/*
 * Structure for accessing symbols in the unix kernel
 * via the nlist subroutine and the /dev/mem driver.
 */

struct nlist nl[] =
{
	{ "_rootdev" },
	{ "_swapdev" },
	{ "_swplo" },
	{ "_nswap" },
	{ "_el_dev" },
	{ "_el_sb" },
	{ "_el_nb" },
	{ "_cputype" },
	{ "_nmount" },
	{ "_usermem" },
	{ "_rk_dt" },
	{ "_mount" },
	{ "ova" },
	{ "" },
};

/*
 * The following are the symbols who's values are obtained
 * from the unix kernel.
 * THE ORDER MUST NOT BE CHANGED !
 */

int	rootdev;
int	swapdev;
int	swplo;
int	nswap;
int	el_dev;
int	el_sb;
int	el_nb;
int	cputype;
int	nmount;
int	usermem;

/*
 * Text array used by the print file
 * system read only status code.
 */

char	*fsutab[] =
{
	"dummy",
	"root device",
	"swap device",
	"error log device",
	"mounted file system",
	0
};

/*
 * Unit number.
 */

int	dn = -1;

/*
 * Drive types,
 * 0 = NED
 * 1 = RK05
 */
#define	RK05	1

char	rk_dt[8];

/*
 * drive can be opened flag.
 */

int	rk_opn;

/*
 * File system write read status.
 * zero for write/read access
 * non zero for read only access
 */

int	fswrs;

/*
 * block and raw I/O file name.
 */

char	fn[20];

/*
 * Armument file name
 */

char	afn[] = "rkx_0.arg";

/*
 * Help message.
 */

char *help[]
{
	"\n\n(rkx) - ULTRIX-11 RK05 disk exerciser.",
	"\nUsage:\n",
	"\trkx [-h] [-d#] [-s#] [-i] [-n #] [-e #]",
	"\n-h\tPrint this help message",
	"\n-d#\tSelect drive number `#' ",
	"\n-s#\tPrint I/O statistics every `#' minutes (default = 30 minutes)",
	"\n-s\tInhibit I/O statistics printout",
	"\n-i\tInhibit file system write/read status printout",
	"\n-n #\tLimit number of data compare error printouts to `#'",
	"\n-e #\tDrop the disk after # errors, default = 100, maximum = 1000",
	"\n\n",
	0
};

/*
 * Time buffers.
 */

int	istime = 30;

char	argbuf[512];

int	sflag, iflag;
int	ndep = 5;
int	ndrop = 100;

#ifdef EFLG
char	*efbit;
char	*efids;
int	zflag;
#else
char	*killfn = "rkx.kill";
#endif

main(argc, argv)
char *argv[];
int argc;
{
	int	stop(), intr();
	register struct mount *mtp;
	register char *p;
	register int i;
	int *ap;
	int j, k;
	int fd;
	char *n;
	char *mp;
	int mem;
	int	bufsiz;
	int	bcmask;

	setpgrp(0, 31111);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	signal(SIGQUIT, stop);
	if(argc < 2)
		goto aderr;
	for(i=1; i < argc; i++) {	/* decode arg's */
		p = argv[i];
		if(*p++ != '-') {
		aderr:
			fprintf(stderr,"\nrkx: bad arg\n");
			exit(1);
			}
		switch(*p) {
		case 'h':	/* print help message */
			for(j=0; help[j]; j++)
				fprintf(stderr,"\n%s",help[j]);
			exit();
#ifdef EFLG
		case 'z':
			zflag++;
			i++;
			efbit = argv[i++];
			efids = argv[i];
			break;
#else
		case 'r':	/* kill filename */
			i++;
			killfn = argv[i];
			break;
#endif
		case 'd':	/* select drive */
			p++;
			if(*p < '0' || *p > '7')
				goto aderr;
			dn = *p - '0';
			afn[4] = *p;	/* argument file name */
			break;
		case 'n':	/* # of data errors to print */
			i++;
			ndep = atoi(argv[i]);
			if((ndep <= 0) || (ndep > 256))
				ndep = 5;
			break;
		case 'e':	/* drop disk after # errors */
			i++;
			ndrop = atoi(argv[i]);
			if((ndrop <= 0) || (ndrop > 1000))
				ndrop = 100;
			break;
		case 'i':	/* Inhibit file system status printouts */
			iflag++;
			break;
		case 's':
			sflag++;
			p++;
			if(*p < '0' || *p >'9')
				break;
			sflag = 0;
			istime = atoi(p);
			if(istime <= 0)
				istime = 30;
			if(istime > 720)
				istime = 720;
			break;
		default:	/* bad argument */
			goto aderr;
		}
	}
	if(!zflag) {
		if(isatty(2)) {
			fprintf(stderr,"rkx: detaching... type \"sysxstop\" to stop\n");
			fflush(stderr);
		}
		if((i = fork()) == -1) {
			printf("rkx: Can't fork new copy !\n");
			exit(1);
		}
		if(i != 0)
			exit(0);
	}
	setpgrp(0, 31111);
/*
 * Check for invalid option combinatons & defaults.
 */

	if(dn < 0) {
		fprintf(stderr, "\nrkx: must select drive\n");
		exit(1);
		}

/*
 * Attempt to open the selected drive,
 * if successful attempt to read first block on the drive.
 * This is done in order to force the disk driver to update
 * the rk_dt[] array with the drive type of each available unit.
 * This should not cause any errors to be logged.
 */

	sprintf(&fn, "/dev/rk%o", dn);		/* file name */
	if((fd = open(fn, 0)) >= 0)
		if(read(fd, (char *) &argbuf, 512) == 512)
			rk_opn++;
	close(fd);

/*
 * Use the nlist subroutine & /dev/mem to set the values
 * to obtain needed data from the unix kernel.
 */

	nlist("/unix", nl);
	for(i=0; i<12; i++) {
		if(i == 10)	/* don't check drive type */
			continue;
		if(nl[i].n_type == 0) {
		      fprintf(stderr,"\nrkx: Can't access namelist in /unix\n");
			exit(1);
			}
		}
	if(nl[10].n_type == 0) {
		fprintf(stderr, "\nrkx: /unix not configured for RK05\n");
		exit(1);
		}
	if((mem = open("/dev/mem", 0)) < 0) {
		fprintf(stderr,"\nrkx: Can't open /dev/mem\n");
		exit(1);
		}
	mp = &rootdev;
	for(i=0; i<10; i++) {
		lseek(mem, (long)nl[i].n_value, 0);
		read(mem, (char *)mp, sizeof(int));
		mp += sizeof(int);
		}
/*
 * Get the drive types.
 */

	lseek(mem, (long)nl[10].n_value, 0);
	read(mem, (char *)&rk_dt, sizeof(rk_dt));
/*
 * Set the write and read buffer sizes
 * and transfer size limits, based on
 * the amount of user memory.
 */
	if(usermem >= 4096) {	/* 256 kb */
		bufsiz = 8192;
		bcmask = 037776;
	} else {
		bufsiz = 4096;
		bcmask = 017776;
		}
/*
 * Print the status of drive.
 */

	j = 0;
	printf("\nUnit %d - ", dn);
	if(rk_dt[dn] == RK05)
		printf("RK05 - ");
	if(rk_dt[dn] && rk_opn) {
		printf("accessible");
		j++;
	} else if(rk_dt[dn] && !rk_opn)
		printf("not accesible");
	else
		printf("non existent\n\n");
	if(j == 0)
		exit(1);

/*
 * Initialize the file system write/read status.
 * Mark the drive read only if it is the root, swap,
 * error log device or is mounted.
 */

	/*
	 * Read the mount table from /unix.
	 *
	 * New stuff - mount table is now consistent between
	 * kernels (ov and sep I&D)  -  Bill Burns 8/13/84
	 */
	mtp = nl[12].n_value;
	lseek(mem, (long)mtp, 0);
	for(i=0; i<nmount; i++, mtp++) {
		p = &argbuf;
		p += (sizeof(struct mount) * i);
		read(mem, (char *)p, sizeof(struct mount));
	}
	close(mem);
	fswrs = cfs(dn);	/* check file system status */
	if(fswrs == 0)		/* root, swap, error log ? */
	{			/* no, check if mounted */
		i = (RK_BMAJ << 8) | dn;	/* maj/min device */
		for(mtp = &argbuf, j=0; j<nmount; mtp++, j++)
			if((mtp->m_bufp != NULL) && (mtp->m_dev == i)) {
				fswrs = 4;	/* file system mounted */
				break;
				}
	}
/*
 * Unless the iflag is set, print the list
 * of read only file systems.
 */
	if(!iflag && fswrs)
	{
		if(rk_dt[dn] == RK05)
			printf("\nRK05");
		printf(" unit %d is read only - %s\n", dn, fsutab[fswrs]);
	}
/*
 * Write needed date into a file `rkx_#.arg' (# = drive),
 * and call part 2 of the RK exerciser (rkxr).
 */

	ap = &argbuf;
	*ap++ = dn;
	*ap++ = ndep;
	*ap++ = ndrop;
	*ap++ = sflag;
	*ap++ = istime;
	*ap++ = bufsiz;
	*ap++ = bcmask;
	*ap++ = fswrs;
#ifdef EFLG
	*ap++ = zflag;
#endif
	p = ap;
	n = &rk_dt;
	for(i=0; i<sizeof(rk_dt); i++)
		*p++ = *n++;
	if((fd = creat(afn, 0644)) < 0) {
		fprintf(stderr, "\nrkx: Can't create %s file\n", afn);
		exit(1);
		}
	if(write(fd, (char *)&argbuf, 512) != 512) {
		fprintf(stderr, "\nrkx: %s write error\n", afn);
		exit(1);
		}
	close(fd);
	fflush(stdout);
	signal(SIGQUIT, SIG_IGN);
#ifdef EFLG
	if(zflag)
		execl("rkxr", "rkxr", afn, efbit, efids, (char *)0);
	else
		execl("rkxr", "rkxr", afn, (char *)0);
#else
	execl("rkxr", "rkxr", afn, killfn, (char *)0);
#endif
	fprintf(stderr, "\nrkx: Can't exec rkxr\n");
	exit(1);
}

/*
 * Check file system and return 0 if it is writable,
 * otherwise return 1 if it is the root file system,
 * 2 if it is the swap file system, and 3 if it
 * is the error log file system.
 */

cfs(dev)
{

	register int rm, sm, em;

	rm = (rootdev >> 8) & 0377;
	sm = (swapdev >> 8) & 0377;
	em = (el_dev >> 8) & 0377;
	if((rm == RK_BMAJ) && (dev == (rootdev & 0377)))
		return(1);
	if((sm == RK_BMAJ) && (dev == (swapdev & 0377)))
		return(2);
	if((em == RK_RMAJ) && (dev == (el_dev & 0377)))
		return(3);
	return(0);
}

stop()
{
	exit(0);
}
