
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)rlx1.c	3.0	4/22/86";
/*
 * ULTRIX-11 RL disk exerciser program (rlx).
 *
 * PART 1 - (rlx1.c)
 *
 *	Part 1 does the initial setup and argument processing,
 *	writes the results into a file `rlx_#.arg' (# = drive).
 *	and then calls part 2 of rlx, which is the actual exerciser.
 *	The program is split into two sections to optimize
 *	memory usage.
 *
 * Fred Canter 3/2/83
 * Bill Burns 4/84
 *	added multiple filesystem code
 *	added event flag code
 *
 *	********************************************
 *      *                                          *
 *      * This program will not function correctly *
 *      * unless the current unix monitor file is  *
 *      * named "unix".                            *
 *	*					   *
 *      ********************************************
 *
 * This program exercises the RL01/2 disks
 * via the unix block and raw I/O interfaces, at
 * the user level.
 * It reports the occurrence of hard errors and/or data
 * compare errors on the standard output.
 * The detailed device error information must be obtained
 * the system error log (elp -rl).
 *
 *
 * USAGE:
 *
 *	rlx -h
 *
 *		Print the help message.
 *
 *
 *	rlx [-d#] [-s#] [-i] [-n #] [-e #] [-z #]
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
 *	[-f#] -	Select a file system to be exercised.
 *		If multiple file systems are selected
 *		only the lowest numbered one will be exercised.
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
 *		 default = 100, maximum is 1000.
 *
 *
 *	note  -	The root, swap, error log, and any mounted file
 *		systems are automatically write protected.
 *
 * EXAMPLE:
 *		rlx -d1
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
	{ "_rl_dt" },
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
	"root",
	"swap",
	"error log",
	"mounted",
	"overlapping",
	0
};

/*
 * Unit number.
 */

int	dn = -1;

/*
 * Drive types,
 * 0 = NED
 * otherwise contains number of blocks on drive,
 * 10240 for RL01, 20480 for RL02.
 */

#define	RL01	10240
#define	RL02	20480

int	rl_dt[4];

/*
 * drive can be opened flag.
 */

int	rl_opn;

/*
 * overlapping partition array
 */

int ovp[]  {0200,0200,0,0,0,0,0,03};

/*
 * Selected file system array.
 * Element = 0 for file system not selected.
 * Element = 1 for file system selected.
 */

char fsact[] {0, 0, 0, 0, 0, 0, 0, 0};

/*
 * File system write read status.
 * zero for write/read access
 * non zero for read only access
 */

int	fswrs[8];	/* for drive partitioning */

/*
 * block and raw I/O file name.
 */

char	fn[20];

/*
 * Armument file name
 */

char	afn[] = "rlx_0.arg";

/*
 * Help message.
 */

char *help[]
{
	"\n\n(rlx) - ULTRIX-11 RL01/2 disk exerciser.",
	"\nUsage:\n",
	"\trlx [-h] [-d#] [-s#] [-i] [-n #] [-e #]",
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

int	sflag, iflag, fflag;
int	ndep = 5;
int	ndrop = 100;

#ifdef EFLG
char	*efbit;
char	*efids;
int	zflag;
#else
char	*killfn = "rlx.kill";
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
	char c;
	char *mp;
	int mem;
	int a, b;
	int x, y;
	int ronly;
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
			fprintf(stderr,"\nrlx: bad arg\n");
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
			if(*p < '0' || *p > '3')
				goto aderr;
			dn = *p - '0';
			afn[4] = *p;	/* argument file name */
			break;
		case 'f':	/* select file system */
			fflag++;
			p++;
			if(*p < '0' || *p > '7')
				goto aderr;
			fsact[*p - '0']++;
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
			fprintf(stderr,"rlx: detaching... type \"sysxstop\" to stop\n");
			fflush(stderr);
		}
		if((i = fork()) == -1) {
			printf("rlx: Can't fork new copy !\n");
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
		fprintf(stderr, "\nrlx: must select drive\n");
		exit(1);
		}
	if(!fflag) {
		for(j=0; j<8; j++)
			fsact[j]++;
		}

/*
 * Attempt to open the selected drive,
 * if successful attempt to read first block on the drive.
 * This is done in order to force the disk driver to update
 * the rl_dt[] array with the drive type of each available unit.
 * The RL nuit number plugs must not be switched between
 * unlike drive types, because the RL driver can only
 * check drive sizes on first open.
 * This should not cause any errors to be logged.
 */

	sprintf(&fn, "/dev/rl%o0", dn);		/* file name */
	if((fd = open(fn, 0)) >= 0)
		if(read(fd, (char *) &argbuf, 512) == 512)
			rl_opn++;
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
		      fprintf(stderr,"\nrlx: Can't access namelist in /unix\n");
			exit(1);
			}
		}
	if(nl[10].n_type == 0) {
		fprintf(stderr, "\nrlx: /unix not configured for RL01/2\n");
		exit(1);
		}
	if((mem = open("/dev/mem", 0)) < 0) {
		fprintf(stderr,"\nrlx: Can't open /dev/mem\n");
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
	read(mem, (char *)&rl_dt, sizeof(rl_dt));
	for(j=0; j<3; j++)
		if(rl_dt[j] == -1)
			rl_dt[j] = 0;

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
	switch(rl_dt[dn]) {
	case RL01:
		printf("RL01 - ");
		break;
	case RL02:
		printf("RL02 - ");
		break;
	default:
		break;
	}
	if(rl_dt[dn] && rl_opn) {
		printf("accessible");
		j++;
	} else if(rl_dt[dn] && !rl_opn)
		printf("not accesible");
	else
		printf("non existent\n\n");
	if(j == 0)
		exit(1);

/*
 * Initialize the file system write/read status array.
 * Mark as read only any file system that is the root, swap,
 * error log device or is mounted.
 * Check overlapping partitions.
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

	for(i=0; i<8; i++) {	/* check file systems for selected unit */
		a = (dn << 3) | i;		/* minor device number */
		fswrs[i] = cfs(a);		/* check file system status */
		if(fswrs[i])			/* root, swap, error log ? */
			continue;		/* yes, then read only ! */
		b = (RL_BMAJ << 8) | a;		/* maj/min device */
		for(mtp = &argbuf, j=0; j<nmount; mtp++, j++)
			if((mtp->m_bufp != NULL) && (mtp->m_dev == b)) {
				fswrs[i] = 4;	/* file system mounted */
				break;
			}
	}

/*
 * Loop to check for overlapping disk partitions
 */
	a = 0;
	for(j = 0; j < 8; j++) {
		a =+ fswrs[j];
		if((fswrs[j] > 0) && (fswrs[j] < 5)) 
			for(x = ovp[j], y = 0; y < 8; y++)
				if((x & (1 << y)) && (fswrs[y] == 0))
					fswrs[y] = 5;
	}
	ronly = a;
/*
 * Unless the iflag is set, print the list
 * of read only file systems.
 */
	if(!iflag) {
	c = 0;
	for(i=0; i<8; i++) {
		if(fswrs[i]) {
			if(!c++) {
				printf("\n\nRead only file systems.");
				printf("\n\nunit\tfile");
				printf("\nnumber\tsystem\treason");
				}
			printf("\n%d\t%d\t", dn, i);
			printf("%s",fsutab[fswrs[i]]);
			printf(" file system");
			}
		}
	}
/*
 * Write needed data into a file `rlx_#.arg' (# = drive),
 * and call part 2 of the RL exerciser (rlxr).
 */

	ap = &argbuf;
	*ap++ = dn;
	*ap++ = ndep;
	*ap++ = ndrop;
	*ap++ = fflag;
	*ap++ = sflag;
	*ap++ = istime;
	*ap++ = bufsiz;
	*ap++ = bcmask;
	*ap++ = ronly;		/* was ----->  *ap++ = fswrs; */
#ifdef EFLG
	*ap++ = zflag;
#endif
	p = ap;
	n = &rl_dt;
	for(i=0; i<sizeof(rl_dt); i++)
		*p++ = *n++;
	for(i=0; i<sizeof(fsact); i++)
		*p++ = fsact[i];
	for(i=0; i<sizeof(fswrs); i++)
		*p++ = fswrs[i];
	if((fd = creat(afn, 0644)) < 0) {
		fprintf(stderr, "\nrlx: Can't create %s file\n", afn);
		exit(1);
		}
	if(write(fd, (char *)&argbuf, 512) != 512) {
		fprintf(stderr, "\nrlx: %s write error\n", afn);
		exit(1);
		}
	close(fd);
	fflush(stdout);
	signal(SIGQUIT, SIG_IGN);
#ifdef EFLG
	if(zflag)
		execl("rlxr", "rlxr", afn, efbit, efids, (char *)0);
	else
		execl("rlxr", "rlxr", afn, (char *)0);
#else
	execl("rlxr", "rlxr", afn, killfn, (char *)0);
#endif
	fprintf(stderr, "\nrlx: Can't exec rlxr\n");
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
	if((rm == RL_BMAJ) && (dev == (rootdev & 0377)))
		return(1);
	if((sm == RL_BMAJ) && (dev == (swapdev & 0377)))
		return(2);
	if((em == RL_RMAJ) && (dev == (el_dev & 0377)))
		return(3);
	return(0);
}

stop()
{
	exit(0);
}
