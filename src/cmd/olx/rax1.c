
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)rax1.c	3.1	3/26/87";
/*
 * ULTRIX-11 UDA50 - RA60/RA80/RA81 disk exerciser program (rax).
 *	     KDA50 - RA60/RA80/RA81
 *	     RQDX1/2 - RD31/RD32/RD51/RD52/RD53/RD54/RX50/RX33
 *	     RQDX3 - RD31/RD32/RD51/RD52/RD53/RD54/RX50/RX33
 *	     RUX1  - RX50
 *	     KLESI - RC25
 *
 * PART 1 - (rax1.c)
 *
 *	Part 1 does the initial setup and argument processing,
 *	writes the results into a file `rax_#.arg' (# = drive).
 *	and then calls part 2 of rax, which is the actual exerciser.
 *	The program is split into two sections to optimize
 *	memory usage.
 *
 * Fred Canter
 * Bill Burns 4/84
 *	added check for overlapping filesystems
 *	added -x option for rx50/rx33 (forces -w to raxr and ensures
 *		that the device is indeed an rx50/rx33.
 *	added event flag code
 *
 *	*********************************************
 *      *                                           *
 *      * This program will not function correctly  *
 *      * unless the current ULTRIX-11 monitor file *
 *      * is named "unix".                          *
 *	*					    *
 *      *********************************************
 *
 * This program exercises the following disks:
 *	RC25 RA60 RA80 RA81 RX50 RX33 RD31 RD32 RD51 RD52 RD53 RD54
 * via the ULTRIX-11 block and raw I/O interfaces, at
 * the user level.
 * It reports the occurrence of hard errors and/or data
 * compare errors on the standard output.
 * The detailed device error information must be obtained
 * the system error log (elp -ra).
 *
 *	CHANGES FOR USER SETABLE DISK PARTITIONS -- Fred Canter 7/5/85
 *
 *	The disk partition sizes table in the currently running kernel is
 *	compared with the standard sizes table, and the operation of RAX
 * 	is modified accordingly. The standard sizes table is hard coded
 *	into the RA exerciser via "#include /usr/sys/conf/dksizes.c".
 *
 *	If the sizes tables match, then RAX allows full functionality, i.e,
 *	it knows the disk layout and can protect the user from him/her self.
 *
 *	If the sizes tables don't match, it is assumed that the user changed
 *	the partition layout for some reason, and RAX operation is modified
 *	as follows:
 *
 *	1.	If the user changed partition 7, fatal error exit.
 *
 *	2.	If any partition, other than 7, overlaps the maintenance
 *		area, fatal error exit.
 *
 *	3.	If the disk is the system disk (root, swap, error log) or
 *		has any mounted file systems, then RAX will only write in
 *		the maintenance area. The rest of the disk is read only.
 *
 *	4.	If the disk is not the system disk and has no mounted file
 *		systems, then partition 7 will be a free fire zone. Only
 *		partition 7 will be used.
 *
 *	5.	The -f flag is ignored.
 *
 *	Note -	Absence of the -w flag will also limit access to the 
 *		maintenance area.
 *
 * USAGE:
 *
 *	rax -h
 *
 *		Print the help message.
 *
 *
 *	rax [-c#] [-d#] [-f#] [-m] [-s#] [-i] [-n #] [-e #] [-w] [-z #]
 *
 *		Exercise the disk(s), using random addresses,
 *		random byte counts, and alternating worst case
 *		and random data patterns.
 *
 *	[-z #] - event flag bit position, used to start/stop
 *		exercisers
 *
 *	[-c#] - MSCP controller number where drive connected (default=0)
 *
 *	[-d#] -	Select the drive to be exercised.
 *		Only one drive may be selected.
 *
 *	[-f#] -	Select a file system to be exercised.
 *		If multiple file systems are selected
 *		only the lowest numbered one will be exercised.
 *
 *	[-m]  - Print the disk file system layout.
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
 *	[-x]   - Specifies that the drive to be tested is an RX50/RX33.
 *		 Forces -w option to be asserted, and causes a check
 *		 to make sure the drive is indeed an RX50/RX33.
 *
 *	[-w]   - Allow writes to the customer area of a winchester
 *		 disks: RA60/RA80/RA81, RC25, & RD31/RD32/RD51/RD52/RD53/RD54.
 *		 Normally writes are only allowed on the maintenance area.
 *
 *	note  -	The root, swap, error log, and any mounted file
 *		systems are automatically write protected.
 *		If any file systems on the selected drive
 *		are marked read only then any overlapping
 *		file systems are also marked read only.
 *
 * EXAMPLE:
 *		rax -c0 -d1 -m
 *
 *		Print the file system map and exercise all
 *		file systems on controller 0 drive 1.
 *
 *
 */

#include <sys/param.h>
#include <sys/devmaj.h>
#include <sys/mount.h>
#include <sys/ra_info.h>
#define	USEP	1
#define	NUDA	1
#include "/usr/sys/conf/dksizes.c"
#include <stdio.h>
#include <a.out.h>
#include <signal.h>

/*
 * Structure for accessing symbols in the ULTRIX-11 kernel
 * via the nlist subroutine and the /dev/mem driver.
 */

struct nlist nl[] =
{
	{ "_ra_size" },
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
	{ "_ra_drv" },
	{ "_mount" },
	{ "ova" },
	{ "_ra_ctid" },
	{ "_ra_mas" },
	{ "_nuda" },
	{ "_ra_inde" },
	{ "" },
};

/*
 * The following are the symbols who's values are obtained
 * from the ULTRIX-11 kernel.
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
	"not maintenance",
	0
};

/*
 * Control variables for dealing with user
 * setable disk partitions.
 */

int	nsdp;	/* non-standard disk partition layout being used */
int	dpmask;	/* bitwise 1 = partition used by drive being exericsed */

/*
 * File system layout, also
 * obtained from the ULTRIX-11 kernel.
 *
 * Prototype sizes tables now in /usr/sys/conf/dksizes.c
 */

struct rasize rasizes[8];

int	ra_saddr[MAXUDA];	/* address of driver sizes table */
int	nuda;		/* Number of MSCP controllers configured */
daddr_t	ra_mas[MAXUDA];	/* maint area size UDA=1000, RQDX=32, KLESI=102 */
char	ra_ctid[MAXUDA]; /* RA controller type IDs (see ra.c) */
char	ra_index[MAXUDA];	/* index into non-semetrical arrays */

/*
 * Overlapping partition array,
 * set dynamically by examining the sizes table.
 */

int	ovp[8];

/*
 * Controller and unit number.
 */

int	cn = 0;
int	dn = -1;
struct	ra_drv	ra_drv[8];

/*
 * drive can be opened flag.
 */

int	ra_opn;

/*
 * Selected file system array.
 * Element = 0 for file system not selected.
 * Element = 1 for file system selected.
 */

char fsact[] {0, 0, 0, 0, 0, 0, 0, 0};

/*
 * File system write read status array.
 * element is zero for write/read access
 * element is non zero for read only access
 */

char	fswrs[8];

/*
 * block and raw I/O file name.
 */

char	fn[20];

/*
 * Armument file name
 */

char	afn[] = "rax0_0.arg";

/*
 * Help message.
 */

char *help[]
{
	"\n\n(rax) - ULTRIX-11 UDA50/KDA50/RQDX1/RQDX3/RUX1/KLESI disk exerciser.",
	"\nUsage:\n",
	"\trax [-h] [-c#] [-d#] [-f#] [-m] [-s#] [-i] [-n #] [-e #]",
	"\n-h\tPrint this help message",
	"\n-c#\tMSCP controller number `#' (default = 0) ",
	"\n-d#\tSelect drive number `#' ",
	"\n-f#\tExercise only file system `#'",
	"\n-m\tPrint file system layout",
	"\n-s#\tPrint I/O statistics every `#' minutes (default = 30 minutes)",
	"\n-s\tInhibit I/O statistics printout",
	"\n-i\tInhibit file system write/read status printout",
	"\n-n #\tLimit number of data compare error printouts to `#'",
	"\n-e #\tDrop the disk after # errors, default = 100, maximum = 1000",
	"\n-w\tCAUTION! - Allows writes to customer area of fixed media disks.",
	"\n\n",
	0
};

/*
 * Time buffers.
 */

int	istime = 30;

char	argbuf[512];

int	fflag, mflag, sflag, iflag, wflag, xflag;
int	ndep = 5;
int	ndrop = 100;

#ifdef EFLG
char	*efbit;
char	*efids;
int	zflag;
#else
char	*killfn = "rax.kill";
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
	int	ronly;
	int	bufsiz;
	int	bcmask;
	struct rasize *spp;
	daddr_t	i_sb, i_eb, j_sb, j_eb;

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
			fprintf(stderr,"\nrax: bad arg\n");
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
			afn[5] = *p;	/* argument file name */
			break;
		case 'c':	/* select controller */
			p++;
			if(*p < '0' || *p > '3')
				goto aderr;
			cn = *p - '0';
			afn[3] = *p;	/* argument file name */
			break;
		case 'f':	/* select file system */
			fflag++;
			p++;
			if(*p < '0' || *p > '7')
				goto aderr;
			fsact[*p - '0']++;
			break;
		case 'm':	/* print file system map */
			mflag++;
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
		case 'x':
			xflag++;
			wflag++;
			break;
		case 'w':	/* write to customer area */
			wflag++;
			break;
		default:	/* bad argument */
			goto aderr;
		}
	}
	if(!zflag) {
		if(isatty(2)) {
			fprintf(stderr,"rax: detaching... type \"sysxstop\" to stop\n");
			fflush(stderr);
		}
		if((i = fork()) == -1) {
			printf("rax: Can't fork new copy !\n");
			exit(1);
		}
		if(i != 0)
			exit(0);
	}
	setpgrp(0, 31111);
/*
 * Check for invalid option combinatons & defaults.
 */

	if((dn < 0) && ((argc != 2) && (!mflag))) {
		fprintf(stderr, "\nrax: must select drive\n");
		exit(1);
	}
	if(!fflag) {
		for(j=0; j<8; j++)
			fsact[j]++;
	}

/*
 * Use the nlist subroutine & /dev/mem to set the values
 * to obtain needed data from the ULTRIX-11 kernel.
 */

	nlist("/unix", nl);
	for(i=1; i<13; i++) {	/* don't check size table */
		if(i == 11)	/* don't check drive type */
		    continue;
		if(nl[i].n_type == 0) {
		    fprintf(stderr,"\nrax: Can't access namelist in /unix!\n");
		    exit(1);
		}
	}
	if((nl[11].n_type == 0) || (nl[0].n_type == 0) ||
	   (nl[16].n_type = 0) ||
	   (nl[17].n_type = 0) ||
	   (nl[14].n_type == 0) || (nl[15].n_type == 0)) {
no_uda:
		fprintf(stderr, "\nrax: /unix not configured for MSCP disks!\n");
		exit(1);
	}
	if((mem = open("/dev/mem", 0)) < 0) {
		fprintf(stderr,"\nrax: Can't open /dev/mem\n");
		exit(1);
	}
	mp = &rootdev;
	for(i=1; i<11; i++) {
		lseek(mem, (long)nl[i].n_value, 0);
		read(mem, (char *)mp, sizeof(int));
		mp += sizeof(int);
	}
/*
 * Get number and type of MSCP controllers.
 */
	lseek(mem, (long)nl[16].n_value, 0);
	read(mem, (char *)&nuda, sizeof(nuda));
	if(nuda == 0)
		goto no_uda;
	if(cn >= nuda) {
	    printf("\nrax: /unix only configured for %d MSCP controller(s)\n",
			nuda);
		exit(1);
	}
	lseek(mem, (long)nl[17].n_value, 0);
	read(mem, (char *)&ra_index, MAXUDA);
	lseek(mem, (long)nl[14].n_value, 0);
	read(mem, (char *)&ra_ctid, MAXUDA);
	if(ra_ctid[cn] == 0) {
		printf("\nrax: MSCP controller %d not initialized!\n", cn);
		exit(1);
	}
	ra_ctid[cn] = (ra_ctid[cn] >> 4) & 017;
/*
 * Get the disk file system layout
 * and maintenance area size.
 */
	lseek(mem, (long)nl[0].n_value, 0);
	read(mem, (char *)&ra_saddr, sizeof(int) * nuda);
	lseek(mem, (long)ra_saddr[cn], 0);
	read(mem, (char *)&rasizes, sizeof(rasizes));
	lseek(mem, (long)nl[15].n_value, 0);
	read(mem, (char *)&ra_mas, sizeof(long) * nuda);
/*
 * Get the drive types.
 * Force open, then get them again.
 */

	i = ra_index[cn] + dn;
	lseek(mem, (long)(nl[11].n_value+(i * sizeof(struct ra_drv))), 0);
	read(mem, (char *)&ra_drv[dn], sizeof(struct ra_drv));
/*
 * Attempt to open the first file system on selected drive,
 * if successful attempt to read first block of the file system.
 * This is done in order to force the disk driver to update
 * the ra_drv[] array with the unit size and on-line status of each unit.
 * This allows for drives to be enabled/disabled via the LAP
 * without requiring a reboot of the system.
 * This should not cause any errors to be logged.
 */

	switch(ra_ctid[cn]) {
	case UDA50:
	case UDA50A:
	case KDA50:
		sprintf(&fn, "/dev/ra%o7", dn);		/* file name */
		if((a = open(fn, 0)) >= 0)
			if(read(a, (char *) &argbuf, 512) == 512)
				ra_opn++;
		close(a);
		break;
	case RQDX1:	/* RD or RX */
	case RQDX3:	/* RD or RX */
		sprintf(&fn, "/dev/rd%o7", dn);		/* file name */
		if((a = open(fn, 0)) >= 0)
			if(read(a, (char *) &argbuf, 512) == 512)
				ra_opn++;
		close(a);
	case RUX1:
		sprintf(&fn, "/dev/rx%o", dn);		/* file name */
		if((a = open(fn, 0)) >= 0)
			if(read(a, (char *) &argbuf, 512) == 512)
				ra_opn++;
		close(a);
		break;
	case KLESI:
		sprintf(&fn, "/dev/rc%o7", dn);		/* file name */
		if((a = open(fn, 0)) >= 0)
			if(read(a, (char *) &argbuf, 512) == 512)
				ra_opn++;
		close(a);
		break;
	default:
		break;		/* unknown cntlr, ra_opn will be zero */
				/* cntlr type validated later */
	}
	i = ra_index[cn] + dn;
	lseek(mem, (long)(nl[11].n_value+(i * sizeof(struct ra_drv))), 0);
	read(mem, (char *)&ra_drv[dn], sizeof(struct ra_drv));
	if(ra_drv[dn].ra_online == 0)
		ra_opn = 0;	/* did not come on-line */
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
 * Compare the prototype file system sizes table
 * with the real one, to insure that RAX really
 * does know about the disk layout.
 * Set the non-standard partitions flag if the sizes mismatch.
 */
	switch(ra_ctid[cn]) {
	case RQDX1:
	case RQDX3:
	case RUX1:
		spp = &rq_sizes;
		break;
	case UDA50:
	case UDA50A:
	case KDA50:
		spp = &ud_sizes;
		break;
	case KLESI:
		spp = &rc_sizes;
		break;
	default:
		printf("\nrax: (%d) - unknown controller type !\n", ra_ctid[cn]);
		exit(1);
	}
	nsdp = 0;
	for(i=0; i<8; i++)
		if((rasizes[i].nblocks != spp[i].nblocks)
		  ||  (rasizes[i].blkoff != spp[i].blkoff))
			nsdp++;
/*
 * If non-standard partitions being used,
 * do some checks to insure minimum sanity level
 * of the sizes table.
 *
 * Partition 7 must not have changed.
 * No partition, other than 7, may overlap the maint. area.
 */
	if(nsdp) {
	    printf("\n****** cntlr %d unit %d ", cn, dn);
	    printf("non standard partition layout ******\n");
	    if((rasizes[7].blkoff != 0L) || (rasizes[7].nblocks != -1L)) {
		fprintf(stderr, "\nrax: fatal error - partition 7 changed!\n");
		exit(1);
	    }
	}
	if(nsdp && (ra_drv[dn].ra_dt != RX50) && (ra_drv[dn].ra_dt != RX33)) {
	    i_sb = ra_drv[dn].d_un.ra_dsize - ra_mas[cn]; /* maint. area size */
	    for(j=0; j<7; j++) {	/* 7 not checked */
		j_eb = rasizes[j].blkoff + rasizes[j].nblocks;
		if(j_eb > i_sb) {	/* partition overlaps maint. area */
		    fprintf(stderr, "\nrax: fatal error - partition %d ", j);
		    fprintf(stderr, "overlaps maintenance area!\n");
		    exit(1);
		}
	    }
	}
/*
 * Set dpmask to show which partitions are used
 * by this type of disk. The dpmask is, for the most part,
 * ignored if non standard partitions in use.
 */
	switch(ra_drv[dn].ra_dt) {
	case RC25:
		dpmask = 0237;
		break;
	case RX33:
		dpmask = 0200;
		break;
	case RX50:
		dpmask = 0200;
		break;
	case RD31:
		dpmask = 0341;
		break;
	case RD51:
		dpmask = 0221;
		break;
	case RD32:
	case RD52:
	case RD53:
	case RD54:
		dpmask = 0217;
		break;
	case RA60:
		dpmask = 0277;
		break;
	case RA80:
		dpmask = 0217;
		break;
	case RA81:
		dpmask = 0377;
		break;
	default:
		dpmask = 0;
		break;
	}
/*
 * Check that the selected unit is an RX50/RX33
 * if the xflag is set
 */
	if(xflag && (ra_drv[dn].ra_dt != RX50) && (ra_drv[dn].ra_dt != RX33)) {
		fprintf(stderr, "\nrax: controller %d ", cn);
		fprintf(stderr, "unit %d not an RX50 or RX33!\n", dn);
		exit(1);
	}
/*
 * Change all partitions with length of -1,
 * to actual size, i.e., size of entire volume
 * minus block offset of partition.
 *
 * If size is -2 also subtract maint. area size.
 */
	for(i=0; i<8; i++) {
		if(rasizes[i].nblocks == -1L)
			rasizes[i].nblocks = 
			    ra_drv[dn].d_un.ra_dsize - rasizes[i].blkoff;
		else if(rasizes[i].nblocks == -2L)
			rasizes[i].nblocks = ((ra_drv[dn].d_un.ra_dsize -
			  rasizes[i].blkoff) - ra_mas[cn]);
	}
/*
 * Set up the overlapping partitions table.
 * Table is only used with standard partition layout.
 */
	for(i=0; i<8; i++) {		/* find overlapping partitions */
	    ovp[i] = 0;
	    if((dpmask & (1 << i)) == 0)
		continue;		/* non usable partition */
	    i_sb = rasizes[i].blkoff;
	    i_eb = rasizes[i].blkoff + rasizes[i].nblocks - 1;
	    for(j=0; j<8; j++) {
		if((dpmask & (1 << j)) == 0)
		    continue;		/* non usable partition */
		if(j == i)
		    continue;		/* can't overlap itself */
		j_sb = rasizes[j].blkoff;
		j_eb = rasizes[j].blkoff + rasizes[j].nblocks - 1;
		if(((i_sb >= j_sb) && (i_sb <= j_eb)) ||
		   ((i_eb >= j_sb) && (i_eb <= j_eb)) ||
		   ((j_sb >= i_sb) && (j_sb <= i_eb)) ||
		   ((j_eb >= i_sb) && (j_eb <= i_eb)) ||
		   ((j_sb <= i_eb) && (j_eb >= i_sb)))
			ovp[i] |= (1 << j);
	    }
	}
/*
 * debug:
	fprintf(stderr, "\nDEBUG: ovp = ");
	for(i=0; i<8; i++)
		fprintf(stderr, "%o ", ovp[i]);
	fprintf(stderr, "\n");
*/
/*
 * Print the file system map if [-m] was specified.
 * Only print file systems used by this type of disk, unless
 * non standard partitions in use, then print all file systems.
 * Always, only print file system 7 for RX50/RX33.
 */

	if(mflag) {
		switch(ra_ctid[cn]) {
		case UDA50:
		case UDA50A:
			fprintf(stderr, "\n\nUDA50 - RA60/RA80/RA81");
			break;
		case KDA50:
			fprintf(stderr, "\n\nKDA50 - RA60/RA80/RA81");
			break;
		case KLESI:
			fprintf(stderr, "\n\nKLESI - RC25");
			break;
		case RQDX1:
		    fprintf(stderr, "\n\nRQDX1 - ");
		    fprintf(stderr, "RD31/RD32/RD51/RD52/RD53/RD54/RX50/RX33");
			break;
		case RQDX3:
		    fprintf(stderr, "\n\nRQDX3 - ");
		    fprintf(stderr, "RD31/RD32/RD51/RD52/RD53/RD54/RX50/RX33");
			break;
		case RUX1:
			fprintf(stderr, "\n\nRUX1 - RX50");
			break;
		}
		fprintf(stderr, " file system layout\n");
		fprintf(stderr, "\nfilsys\toffset\tlength\n");
		for(i=0; i<8; i++) {
			if((nsdp == 0) && ((dpmask & (1 << i)) == 0))
				continue;	/* partition not used */
			if(nsdp && (ra_drv[dn].ra_dt == RX50) && (i != 7))
				continue;
			if(nsdp && (ra_drv[dn].ra_dt == RX33) && (i != 7))
				continue;
			if(rasizes[i].nblocks)
				fprintf(stderr, "\n%d\t%D\t%D",
			   i,rasizes[i].blkoff,rasizes[i].nblocks);
		}
		fprintf(stderr,"\n\n");
		exit(0);
	}
/*
 * Print the status of drives.
 */

	j = 0;
	printf("\nController %d Unit %d - ", cn, dn);
	switch(ra_drv[dn].ra_dt) {
	case RC25:
		printf("RC25 - ");
		break;
	case RX33:
		printf("RX33 - ");
		break;
	case RX50:
		printf("RX50 - ");
		break;
	case RD31:
		printf("RD31 - ");
		break;
	case RD32:
		printf("RD32 - ");
		break;
	case RD51:
		printf("RD51 - ");
		break;
	case RD52:
		printf("RD52 - ");
		break;
	case RD53:
		printf("RD53 - ");
		break;
	case RD54:
		printf("RD54 - ");
		break;
	case RA60:
		printf("RA60 - ");
		break;
	case RA80:
		printf("RA80 - ");
		break;
	case RA81:
		printf("RA81 - ");
		break;
	default:
		break;
	}
	if(ra_drv[dn].ra_dt && ra_opn) {
		printf("accessible");
		j++;
	} else if(ra_drv[dn].ra_dt && !ra_opn)
		printf("not accessible");
	else
		printf("non existent\n\n");
	if(j == 0)
		exit(1);

/*
 * Initialize the file system write/read status array.
 * Mark the root, swap, error log, & any mount file systems
 * read only.
 * Also mark file system 7 read only if any other
 * file systems on the drive are read only.
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
		a = (cn << 6) | (dn << 3) | i;	/* minor device number */
		fswrs[i] = cfs(a);	/* check file system status */
		if(fswrs[i])	/* root, swap, error log ? */
			continue;	/* yes, then read only ! */
		b = (RA_BMAJ << 8) | a;		/* maj/min device */
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
	ronly = a;		/* save number of read only file systems */
 
/*
 * If non standard partitions in use and any read only
 * file systems, for wflag = 0 (write only in maint. area).
 */
	if(nsdp && ronly)
		wflag = 0;
/*
 * If the wflag is not set, all file systems read only.
 */
	if(wflag == 0) {
		for(j=0; j<8; j++)
			fswrs[j] = 6;
		ronly = 8;
	}

/*
 * Unless the iflag is set, print the list
 * of read only file systems.
 */
	if(!iflag)
	{
	c = 0;
	for(i=0; i<8; i++) {
		if(nsdp)
			break;
		if((dpmask & (1 << i)) == 0)
			continue;	/* partition not used */
		if(rasizes[i].nblocks == 0)
			continue;
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
	if(wflag == 0)
		printf("\n\n****** Writing to maintenance area only ******\n");
	if(nsdp && (wflag != 0)) {
		printf("\n\n****** Writing to file system 7 only ");
		printf("(non standard partitions used) ******\n");
	}
/*
 * Write needed date into a file `rax?_#.arg' (? = ctlr, # = drive),
 * and call part 2 of the RA exerciser (raxr).
 */

	ap = &argbuf;
	*ap++ = cn;
	*ap++ = dn;
	*ap++ = ndep;
	*ap++ = ndrop;
	*ap++ = fflag;
	*ap++ = sflag;
	*ap++ = wflag;
	*ap++ = istime;
	*ap++ = ronly;
	*ap++ = bufsiz;
	*ap++ = bcmask;
	*ap++ = (int)ra_mas[cn];
	*ap++ = ra_ctid[cn];
	*ap++ = nsdp;
	*ap++ = dpmask;
#ifdef EFLG
	*ap++ = zflag;
#endif
	p = ap;
	n = &rasizes;
	for(i=0; i<sizeof(rasizes); i++)
		*p++ = *n++;
	n = &ra_drv;
	for(i=0; i<sizeof(ra_drv); i++)
		*p++ = *n++;
	for(i=0; i<sizeof(fsact); i++)
		*p++ = fsact[i];
	for(i=0; i<sizeof(fswrs); i++)
		*p++ = fswrs[i];
	if((fd = creat(afn, 0644)) < 0) {
		fprintf(stderr, "\nrax: Can't create %s file\n", afn);
		exit(1);
		}
	if(write(fd, (char *)&argbuf, 512) != 512) {
		fprintf(stderr, "\nrax: %s write error\n", afn);
		exit(1);
		}
	close(fd);
	fflush(stdout);
	signal(SIGQUIT, SIG_IGN);
#ifdef EFLG
	if(zflag)
		execl("raxr", "raxr", afn, efbit, efids, (char *)0);
	else
		execl("raxr", "raxr", afn, (char *)0);
#else
	execl("raxr", "raxr", afn, killfn, (char *)0);
#endif
	fprintf(stderr, "\nrax: Can't exec raxr\n");
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
	if((rm == RA_BMAJ) && (dev == (rootdev & 0377)))
		return(1);
	if((sm == RA_BMAJ) && (dev == (swapdev & 0377)))
		return(2);
	if((em == RA_RMAJ) && (dev == (el_dev & 0377)))
		return(3);
	return(0);
}

stop()
{
	exit(0);
}
