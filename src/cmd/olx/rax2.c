
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)rax2.c	3.1	3/26/87";
/*
 * ULTRIX-11 UDA50 - RA60/RA80/RA81 disk exerciser program (rax).
 *	     KDA50 - RA60/RA80/RA81
 *	     RQDX1/2 - RD31/RD32/RD51/RD52/RD53/RD54/RX50/RX33
 *	     RQDX3 - RD31/RD32/RD51/RD52/RD53/RD54/RX50/RX33
 *	     RUX1  - RX50
 *	     KLESI - RC25
 *
 * Fred Canter
 * Bill Burns 
 *	added event flags	4/19/84
 *
 * PART 2 - (rax2.c)
 *
 *	Part 2 is the actual disk exerciser
 *
 *	Usage:
 *		rax rax_#.arg # #
 *		    ^	      ^ ^
 *		    |	      | event flag id
 *		    |	      event flag bit position
 *		    argument file name
 *
 *
 ****************************************************************
 *								*
 *	The last ## blocks of each drive are reserved		*
 *	as the maintenance area. Unless the [-w] option		*
 *	is specified, this is the only area where writes	*
 *	are allowed. If [-w] is specified, then the		*
 *	normal read only rules apply.				*
 *	## = 1000 for UDA50, 32 for RQDX1/3, 102 for KLESI	*
 *								*
 ****************************************************************
 *
 *	CHANGES FOR USER SETABLE DISK PARTITIONS -- Fred Canter 7/5/85
 *
 *	Note - most of this work already done in rax1.c (rax).
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
 */

#include <sys/param.h>
#include <sys/devmaj.h>
#include <sys/ra_info.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <errno.h>

#define	NPASS	5000

/*
 * File system layout, also
 * obtained from the ULTRIX-11 kernel.
 */

int	nsdp;		/* non standard partitions layout in use */
int	dpmask;		/* partitions used by this type of disk */
			/* bitwise 1 = partition used */
struct	rasize	rasizes[8];
int	ra_ctid;
int	ra_mas;

/*
 * Unit number.
 */

int	cn;
int	dn;

struct	ra_drv	ra_drv[8];

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
 * File descriptors for read/write and random read.
 * The first 8 file descriptors are for block I/O
 * and the second 8 are for raw I/O.
 * -1,   file system not open
 * >= 0, file system is open
 */

int	fd_rw[16];

/*
 * write, read, error statistics.
 */

long	rdcnt;
long	wrtcnt;
long	hecnt;

/*
 * block and raw I/O file name.
 */

char	fn[20];

/*
 * The following three lines of code
 * are the interface to the system call
 * error return messages.
 */

int	errno;
int	sys_nerr;
char	*sys_errlist[];

/*
 * Time buffers.
 */

int	istime;
time_t	btbuf;	/* Exerciser run beginning time */
time_t	etbuf;	/* Exerciser run ending time */
time_t	stbuf;	/* I/O statistics time */
time_t	timbuf;
struct tm *tl;	/* structure for localtime */
char	btime[13];	/* ascii beg time yymmddhhmmss */
char	etime[13];	/* ascii end time yymmddhhmmss */

char	*diskn;		/* Disk type name */
char	diskdn[] = "-ra#";

/*
 * Write/read buffers,
 * size depends on amount of free memory.
 * The buffer size is 8k words for > 256 kb
 * and 4k words for < 256 kb.
 * Buffer area obtained via calloc();
 */

int	*wbuf;	/* points to write buffer */
int	*rbuf;	/* points to read buffer */
int	bufsiz;
int	bcmask;

long	randx;

/*
 * Structure for selecting test data patterns.
 * Patterns were lifted from the UDA diagnostic.
 */

#define	P_RAND	0
#define	P_SEQ1	1
#define	P_SEQ3	2
#define	P_SEQ16	3

struct	{
	int	p_type;		/* type of data pattern */
	int	p_data[16];	/* actual data pattern */
} pat[16] =  {
	{ P_RAND },
	{ P_SEQ1, 0105613 },
	{ P_SEQ1, 031463 },
	{ P_SEQ1, 030221 },
	{ P_SEQ16, 1, 3, 7, 017, 037, 077, 0177, 0377, 0777, 01777,
		03777, 07777, 017777, 037777, 077777, 0177777 },
	{ P_SEQ16, 0177776, 0177774, 0177770, 0177760, 0177740, 0177700,
		0177600, 0177400, 0177000, 0176000, 0174000, 0170000,
		0160000, 0140000, 0100000, 0 },
	{ P_SEQ16, 0, 0, 0, -1, -1, -1, 0, 0, -1, -1, 0, -1, 0, -1, 0, -1 },
	{ P_SEQ1, 0133331 },
	{ P_SEQ16, 052525, 052525, 052525, 0125252, 0125252,
		0125252, 052525, 052525, 0125252, 0125252,
		052525, 0125252, 052525, 0125252, 052525, 0125252 },
	{ P_SEQ1, 0155554 },
	{ P_SEQ16, 026455, 026455, 026455, 0151322, 0151322,
		0151322, 026455, 026455, 0151322, 0151322,
		026455, 0151322, 026455, 0151322, 026455, 0151322 },
	{ P_SEQ1, 066666 },
	{ P_SEQ16, 1, 2, 4, 010, 020, 040, 0100, 0200, 0400, 01000,
		02000, 04000, 010000, 020000, 040000, 0100000 },
	{ P_SEQ16, 0177776, 0177775, 0177773, 0177767, 0177757,
		0177737, 0177677, 0177577, 0177377, 0176777,
		0175777, 0173777, 0167777, 0157777, 0137777, 077777 },
	{ P_SEQ3, 0155555, 0133333, 0155555 },
	{ P_SEQ16, 0133331, 0133331, 0133331, 0155554, 0155554,
		0155554, 0133331, 0133331, 0155554, 0155554,
		0133331, 0155554, 0133331, 0155554, 0133331, 0155554 },
};

#ifdef EFLG
#include <sys/eflg.h>
char	*efpis;
char	*efids;
int	efbit;
int	efid;
long	evntflg();
int	zflag;
#else
char	*killfn;
#endif

int	ndep, ndrop, wflag;

int	rbc;	/* byte count returned from read/write calls */

int	rootdev;

main(argc, argv)
char *argv[];
int argc;
{
	register int	*wbp, *rbp;
	register int	ctr;
	int	stop(), intr();
	FILE	*argf;
	int fflag, sflag;
	int i, j, k;
	int fd;
	int *ap;
	char *p, *n;
	int a, b;
	int	rderr, count, cnt, nbytes;
	int	w_off, r_off, rr_off;
	int	fsterr, feb, neb;
	int	nwlb, rba, nw;
	daddr_t	ebn, bn;
	int	nfb, rbn;
	int	ebc, woff, roff;
	daddr_t	bn_rw, bn_rr;
	int	fs_rw, fs_rr, efd;
	int	ronly;
	long	fsz, lbt;

	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, intr);
	signal(SIGQUIT, stop);
	close(stdin);
	if((argc < 2) || (argc > 4))
		exit(1);
/*
 * Read needed date from the `rax_#.arg' file (# = drive).
 */
	argf = fopen(argv[1], "r");
	if(argf == NULL)
		exit(1);
	cn = getw(argf);
	dn = getw(argf);
	ndep = getw(argf);
	ndrop = getw(argf);
	fflag = getw(argf);
	sflag = getw(argf);
	wflag = getw(argf);
	istime = getw(argf);
	ronly = getw(argf);
	bufsiz = getw(argf);
	bcmask = getw(argf);
	ra_mas = getw(argf);
	ra_ctid = getw(argf);
	nsdp = getw(argf);
	dpmask = getw(argf);
#ifdef EFLG
	zflag = getw(argf);
#endif
	n = &rasizes;
	for(i=0; i<sizeof(rasizes); i++)
		*n++ = getc(argf);
	n = &ra_drv;
	for(i=0; i<sizeof(ra_drv); i++)
		*n++ = getc(argf);
	for(i=0; i<sizeof(fsact); i++)
		fsact[i] = getc(argf);
	for(i=0; i<sizeof(fswrs); i++)
		fswrs[i] = getc(argf);
	fclose(argf);
	unlink(argv[1]);
#ifdef EFLG
	if(zflag) {
		efpis = argv[2];
		efids = argv[3];
	}
#else
	killfn = argv[2];	/* run/stop control file name */
#endif
/*
 * If this is the system disk, do not allow any
 * block mode I/O other than random reads.
 * Any disk with read only file systems !
 * This is because bdflush() will interfere with
 * normal buffer cache operations.
 */
	for(i=0; i<8; i++)
		if(fswrs[i] != 0)
			rootdev = 1;

/*
 * The EXERCISE mode exercises all selected file systems
 * on each selected drive by writing, reading , and
 * comparing the data.
 * A random read of a single block is also done.
 * The root, swap , error log , and any mounted file
 * systems are treated as read only.
 * The disk activity occurs on a four count cycle, i.e.,
 *
 *	count	io mode	 data	byte count
 *
 *	0	block	pat[?]	512
 *	1	block	pat[?]	random, 2048 max
 *	2	raw	pat[?]	512
 *	3	raw	pat[?]	8 count cycle
 *
 *	8 count cycle
 *
 *	random byte count - 8190 max,
 *	1024, 2048, 3072, 4096, 5120, 6144, 7168 bytes
 *
 *
 * Each count consists of the following sequence:
 *
 *  1.	Randumly select the file system and block number
 *	for the write/read/compare and the random read.
 *
 *  2.	Open the selected file systems if necessary.
 *
 *  3.	Select the data pattern to be used,
 *	from the pattern structure pat[].
 *
 *  4.	Load the write buffer with a data pattern.
 *
 *  5.	Write to the disk.
 *
 *  6.	Read a randomly selected block from the disk.
 *
 *  7.	Clear the read buffer.
 *
 *  8.	Read the data written in step 5. from the disk.
 *
 *  9.	Compare the data in the read buffer with the
 *	data pattern and report any mismatches.
 *
 * 10.	Increment the count and goto step one.
 */

/*
 * Allocate memory for Write/Read buffers.
 */
	wbuf = calloc(bufsiz, sizeof(int));
	rbuf = calloc(bufsiz, sizeof(int));
	if(wbuf == NULL || rbuf == NULL) {
		fprintf(stderr, "\nrax: Can't calloc R/W buffers\n");
		exit(1);
		}

/*
 * Set all file descriptors to -1 to
 * indicate all files closed.
 */
	for(i=0; i<16; i++)
		fd_rw[i] = -1;
	time(&btbuf);
	randx = btbuf & 0777;	/* initialize random number generator */
	switch(ra_drv[dn].ra_dt) {
	case RC25:
		diskn = "RC25";
		break;
	case RX33:
		diskn = "RX33";
		break;
	case RX50:
		diskn = "RX50";
		break;
	case RD31:
		diskn = "RD31";
		break;
	case RD32:
		diskn = "RD32";
		break;
	case RD51:
		diskn = "RD51";
		break;
	case RD52:
		diskn = "RD52";
		break;
	case RD53:
		diskn = "RD53";
		break;
	case RD54:
		diskn = "RD54";
		break;
	case RA60:
		diskn = "RA60";
		break;
	case RA80:
		diskn = "RA80";
		break;
	case RA81:
		diskn = "RA81";
		break;
	}
	printf("\n\n%s disk exerciser started - %s",diskn,ctime(&btbuf));
#ifdef EFLG
	if(zflag) {
		efbit = atoi(efpis);
		efid = atoi(efids);
		evntflg(EFCLR, efid, (long)efbit);
	}
#else
	unlink(killfn);
#endif
	time(&stbuf);
	fflush(stdout);
	nice(0);
/*
 * If this is the system disk, i.e.,
 * if the root and/or swap are on this drive,
 * run at lower priority.
 * This is done because the system disk already
 * gets a certain amount of activity even without
 * the exerciser running.
 */

/* No!
	for(i=0; i<8; i++)
		if((fswrs[i] == 1) || (fswrs[i] == 2)) {
			nice(20);
			break;
			}
*/
e_loop:
	cnt = count & 03;
	if(rootdev && ((cnt == 0) || (cnt == 1)))
		goto next;
	if(cnt & 1) {
		nbytes = rng() & bcmask;
		if(cnt == 1) {
			nbytes =& 03776;
			nbytes =+ 2;
		} else {
			a = (count >> 2) & 07;
			b = a * 1024;
			if(a)
				nbytes = b;
			if(nbytes < 4)
				nbytes = 8192;
			}
		}
	else
		nbytes = 512;
	rr_off = (count & 017) * 256;
	w_off = offset(nbytes);
	r_off = offset(nbytes);
/*
 * Generate a random file system, unless the [-f#]
 * option is specified. In that case use the 
 * first selected file system.
 * Also generate a random block number, check to insure
 * that it is within the bounds of the file system,
 * and adjust it if neccessary.
 */
	if(nsdp) {		/* non standard disk partitions in use */
		fs_rw = fsok(dn, 7);	/* use only file system 7 */
		fs_rr = fs_rw;		/* & possibly maint. area */
	} else if(fflag) {
		for(fs_rw=0; fsact[fs_rw]==0; fs_rw++)
		{}
		fs_rr = fs_rw;
		if(fs_rw != fsok(dn, fs_rw)) {
			fprintf(stderr,"\nrax: invalid file system selected\n");
			exit(1);
		}
	} else if(!ronly) {
		fs_rw = fsok(dn, 7);
		fs_rr = fs_rw;
	} else {
		fs_rw = (rng() >> 8) & 07;
		fs_rr = fsok(dn, 7);
		fs_rw = fsok(dn, fs_rw);
	}
	if(wflag == 0) {	/* write maint area only */
		fsz = rasizes[7].nblocks;
		fs_rw = 7;
		bn_rw = rasizes[7].nblocks - (long)ra_mas;
		if((ra_ctid == RQDX1) || (ra_ctid == RQDX3) || (ra_ctid == RUX1))
			bn_rw += (rng() & 037);
		else if(ra_ctid == KLESI)
			bn_rw += (rng() & 077);
		else
			bn_rw += (rng() & 01777);
	} else {
		fsz = rasizes[fs_rw].nblocks;
		bn_rw = (fsz / 8) * (rng() & 7);
		bn_rw += (unsigned int)rng();
	}
	lbt = bn_rw + ((nbytes + 511) / 512);
	if(lbt > fsz)
		bn_rw -= (lbt - fsz);
	fsz = rasizes[fs_rr].nblocks;
	switch(ra_ctid) {
	case KLESI:
		bn_rr = (unsigned int)rng() & 037777;
		lbt = bn_rr + ((nbytes + 511) / 512);
		if(lbt > fsz)
			bn_rr -= (lbt - fsz);
		break;
	case UDA50:
	case UDA50A:
	case KDA50:
		bn_rr = (unsigned int)rng();	/* bias toward lower numbers */
		lbt = bn_rr + ((nbytes + 511) / 512);
		if(lbt > fsz)
			bn_rr -= (lbt - fsz);
		break;
	case RUX1:
	case RQDX1:
	case RQDX3:
		/*
		 * TODO: Should check unit size and adjust bn_rr mask
		 *	 according to media type (size=800 for rx50
		 *	 size=2400 for rx33. RX33 drive can access both
		 *	 RX33 and RX50 media.
		 */
		if((ra_drv[dn].ra_dt == RX50) || (ra_drv[dn].ra_dt == RX33))
			bn_rr = (unsigned int)rng() & 0777;
		else {
			bn_rr = (unsigned int)rng() & 037777;
			lbt = bn_rr + ((nbytes + 511) / 512);
			if(lbt > fsz)
				bn_rr -= (lbt - fsz);
		}
		break;
	}
/*
 * Set the file descriptor (efd) for
 * the correct I/O mode, i.e., block or raw.
 */
	a = count & 02;
	if(a) {
		if((efd = fsopen(dn, fs_rw+8)) < 0) {	/* raw I/O */
		opnerr:
			fprintf(stderr,"\nrax: Can't open %s\n", fn);
		fatal:
			fprintf(stderr,"\nrax: FATAL ERROR, unit %d dropped !\n", dn);
			fflush(stderr);
			for( ;; )
				sleep(3600);
			}
	} else
		if((efd = fsopen(dn, fs_rw)) < 0)	/* block I/O */
			goto opnerr;
/*
 * Open the file system for 
 * the random read.
 */

	if(fsopen(dn, fs_rr) < 0)
		goto opnerr;
/*
 * Select the data pattern and load the write
 * buffer.
 * The random data pattern is a random number
 * which changes for each sector written.
 * The other patterns are selected from
 * the pattern structure.
 */
	wbp = wbuf;
	wbp += w_off;
	j = nbytes/2;
	k = rng() & 017;		/* select pattern type */
	switch(pat[k].p_type) {
	case P_RAND:			/* random data */
		for(ctr=0; ctr<j; ctr++) {
			if((ctr & 0377) == 0)
				rbp = rng();
			*wbp++ = rbp;
		}
		break;
	case P_SEQ1:			/* single word sequence */
		for(ctr=0; ctr<j; ctr++)
			*wbp++ = pat[k].p_data[0];
		break;
	case P_SEQ3:			/* three word sequence */
		for(ctr=0; ctr<j; ctr++) {
			if((ctr % 3) == 0)
				i = 0;
			*wbp++ = pat[k].p_data[i++];
		}
		break;
	case P_SEQ16:			/* sixteen word sequence */
		for(ctr=0; ctr<j; ctr++)
			*wbp++ = pat[k].p_data[ctr&017];
		break;
	}
/*
 * Write to disk and count write operations.
 *(only if file system is write/read)
 */
e_erc:	/* Entry point for hard error xfer restart */
	if((fswrs[fs_rw] == 0) || (wflag == 0)) {
		lseek(efd, (long)(bn_rw * 512), 0);
		wrtcnt++;
		wbp = wbuf;
		wbp += w_off;
		if((rbc = write(efd, (char *)wbp, nbytes)) != nbytes)
			dskse((cnt | 010), dn, fs_rw, bn_rw, nbytes);
			if(errno == ENXIO)	/* unit went off-line */
				goto fatal;
		}
/*
 * Read a random block from disk
 * in block I/O mode.
 * Count the read operation.
 */
	lseek(fd_rw[fs_rr], (long)(bn_rr * 512), 0);
	rdcnt++;
	rbp = rbuf;
	rbp += rr_off;
	if((rbc = read(fd_rw[fs_rr], (char *)rbp, 512)) != 512) {
		dskse(0, dn, fs_rr, bn_rr, 512);
		printf("\n******\n");
		if(errno == ENXIO)	/* unit went off-line */
			goto fatal;
		}
/*
 * Real read from disk.
 *
 * The block number, byte count , & buffer offset
 * are buffered because they are changed durring a
 * hard read error continuation operation.
 */

		woff = w_off;
		roff = r_off;
		ebc = nbytes;
		ebn = bn_rw;
/*
 * Zero the portion of the read buffer
 * that will hold the next read data.
 * The read buffer pointer (rbp) and the loop counter
 * (ctr) are registers for speed !
 */
	rbp = rbuf;
	rbp += roff;
	for(ctr=0; ctr<(nbytes/2); ctr++)
		*rbp++ = 0;

		rderr = 0;
		lseek(efd, (long)(ebn * 512), 0);
		rdcnt++;
		rbp = rbuf;
		rbp += roff;
		if((rbc = read(efd, (char *)rbp, ebc)) != ebc) {
			dskse(cnt, dn, fs_rw, ebn, ebc);
			rderr++;
			if(errno == ENXIO)	/* unit went off-line */
				goto fatal;
			}
		fsterr = 1; /* first error flag */
		if((fswrs[fs_rw] == 0) || (wflag == 0))
		{	/* only check data on r/w file systems */
		nfb = ebc/512;	/* # of full blocks */
		nwlb = (ebc%512)/2;	/* # of words in last block */

		rbn = 0;		/* relative block # */
	e_dcl:
		if(rbn > nfb)
			goto e_dclend;	/* all blocks checked */
		if(rbn == nfb) {	/* last block ? */
			nw = nwlb;	/* yes, change of words */
			if(nw == 0)
				goto e_dclend; /* no partial block at end */
		} else
			nw = 256;	/* # words for full block */

/*
 * If a hard read error occurs on a multi block transfer, 
 * all blocks after the bad block will fail the data
 * compare test because the transfer aborts on a hard read error.
 * In order to insure that all blocks are checked the transfer
 * is restarted at bad block plus one and the data compare test
 * is completed. The block number, byte count and buffer address
 * are adjusted accordingly.
 */

		if(rderr && !fsterr) {
			bn_rw =+ rbn;
			w_off =+ (rbn*256);
			r_off =+ (rbn*256);
			nbytes =- (rbn*512);
			printf("\n******\n");
			goto e_erc;
			}
		feb = 1;	/* first error in block flag */
		neb = 0;	/* # of errors in block */
		wbp = wbuf;
		wbp += (woff + (rbn*256));
		rbp = rbuf;
		rbp += (roff + (rbn*256));
		for(ctr=0; ctr<nw; ctr++) {	/* check data in 1 block */
			if(*rbp++ != *wbp++) {	/* data compare */
				if(!rderr && fsterr) {
					time(&timbuf);
printf("\n\n******\nDATA MISMATCH WITHOUT I/O ERROR - %s", ctime(&timbuf));
	dskse((cnt|020), dn, fs_rw, ebn, ebc);
					}
				if(fsterr) {
	printf("\n\nWrite was from word %4.d of write buffer", woff);
	printf("\nRead  was to   word %4.d of read  buffer", roff);
					}
				if(feb) {
	printf("\n\nDATA COMPARE ERROR - BLOCK %D ",(ebn+rbn));
	printf("\n\nWrite buffer address = %4.d", woff+(rbn*256));
	printf("\nRead  buffer address = %4.d", roff+(rbn*256));
					feb = 0;
					}
				if(++neb > ndep) {
				printf("\n\n[error printout limit exceeded]");
					break;
					}
				printf("\n\nWORD = %d",ctr);
			printf("\nGOOD = %06.o",*(wbuf+woff+(rbn*256)+ctr));
			printf("\nBAD  = %06.o",*(rbuf+roff+(rbn*256)+ctr));
				fsterr = 0;
				}
			}
		if(hecnt >= ndrop) {
	    printf("\n\nTotal error limit exceeded, unit %d dropped !\n", dn);
			fflush(stdout);
			for( ;; )
				sleep(3600);
		}
		rbn++;
		goto e_dcl;
		}
	e_dclend:
		if(!fsterr) {
			printf("\n******\n");
			fflush(stdout);
		}
		if(rderr && fsterr) {
			printf("\n******\n");
			fflush(stdout);
		}
/*
 * If the last operation was in block mode,
 * cancel delayed write on any buffers assosiated
 * with this major/minor device so that they will
 * not be flushed out to the disk at some time later
 * and overwrite the following raw I/O test.
 */
	if((cnt == 0) || (cnt == 1))
		bdflush((RA_BMAJ<<8)|(cn<<6)|(dn<<3)|fs_rw);
next:
	if(++count >= NPASS) {
		count = 0;
		time(&timbuf);
	    printf("\n%s disk exerciser end of pass - %s",diskn,ctime(&timbuf));
		fflush(stdout);
		if((i = fork()) > 0)
			exit(0);
		if(i == -1)
			fprintf(stderr,"\nrax: Can't fork new copy of rax !\n");
	}
	if(sflag)
		goto e_loop;
	time(&timbuf);
	if(((timbuf - stbuf) / 60) < istime)
		goto e_loop;
	pios();	/* Print I/O statistics */
	goto e_loop;
}

/*
 * Randum number generator.
 */

rng()
{
	return(((randx = randx * 1103515245 + 12345) >> 16) & 0177777);
}

/*
 * Disk status error printout function.
 */

dskse(et, drv, fs, blk, bc)
daddr_t blk;
{

	hecnt++;
	if((et & 020) == 0) {
		time(&timbuf);
		printf("\n\n******\nHARD DISK ERROR - %s",ctime(&timbuf));
		printf("Returned byte count = %d (-1 = error)", rbc);
		printf("\nError type: ");
		if(errno < sys_nerr)
			printf("%s\n", sys_errlist[errno]);
		else
			printf("Unknown error\n");
		}
	printf("\nunit  filesys  block    xfer size  xfer type");
	printf("\n%d     %d       %6.D  %5.u bytes  ",drv,fs,blk,bc);
	if(et & 2)
		printf("RAW I/O ");
	else
		printf("BLOCK I/O ");
	if(et & 010)
		printf("WRITE\n******\n");
	else
		printf("READ\n");
}

/*
 * Print I/O statistics
 */

pios()
{

	time(&stbuf);
	randx = stbuf & 0777;
	printf("\n\nI/O statistics - %s",ctime(&stbuf));
	printf("\ndrive   write       read        hard");
	printf("\nnumber  operations  operations  errors\n");
	printf("\n%6.d  %10.D", dn, wrtcnt);
	printf("  %10.D  %6.D", rdcnt, hecnt);
	printf("\n");
	fflush(stdout);
}

intr()
{
	signal(SIGTERM, intr);
#ifdef EFLG
	if(zflag) {
		if(!checkflg())
			return;
	} else
		return;
#else
	if(access(killfn, 0) != 0)
		return;
#endif
	stop();
}

stop()
{
	register int	i;

	signal(SIGTERM, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	for(i=0; i<16; i++)
		close(fd_rw[i]);
	time(&etbuf);	/* ending time */
	printf("\n\n%s disk exerciser stopped - %s\n", diskn, ctime(&etbuf));
	pios();		/* print I/O stats */
	tconv(&btbuf, &btime);	/* convert beg time to ascii */
	tconv(&etbuf, &etime);	/* convert end time to ascii */
	fflush(stdout);
	diskdn[3] = dn + '0';
	if(fork() == 0)
	   execl("/bin/elp","elp","-s",diskdn,"-d",&btime,&etime,(char *)0);
	else
		while(wait() != -1) ;
#ifdef EFLG
	if(zflag)
		evntflg(EFCLR, efid, (long)efbit);
#else
	unlink(killfn);
#endif
	exit(0);
}

/*
 * This function returns a random write of read
 * buffer word offset, which assures
 * that the transfer will not overflow the buffer.
 * The buffer size (bufsiz) is based on free memory.
 */

offset(nb)
{
	register int	off, lim, wc;

	wc = nb/2;	/* convert byte count to word count */
	lim = bufsiz;	/* buffer size is xfer limit */
	off = rng() & (bufsiz - 2);	/* random offset */
	while((off + wc) > lim)	/* make xfer fit in buffer */
		off =- 256;
	if(off < 0)
		off = 0;
	return(off);
}

/*
 * Check to verify that the selected file system exists
 * on the selected drive type, adjust if necessary.
 */

fsok(dn, filsys)
{

	if((dpmask & (1 << filsys)) == 0)
		return(7);
	else
		return(filsys);
}

/*
 * File system open function.
 * Return a file descriptor for the
 * requested file system.
 * Open the file if necessary.
 *
 * dn	Disk drive number.
 * fso	File system to be opened.
 */

fsopen(dn, fso)
{

	register int j;

	j = fso & 07;
	if(fd_rw[fso] < 0) {
		switch(ra_drv[dn].ra_dt) {
		case RA60:
		case RA80:
		case RA81:
			if(fso < 8)			/* generate file name */
				sprintf(&fn, "/dev/ra%o%o", dn, j);
			else
				sprintf(&fn, "/dev/rra%o%o", dn, j);
			break;
		case RC25:
			if(fso < 8)			/* generate file name */
				sprintf(&fn, "/dev/rc%o%o", dn, j);
			else
				sprintf(&fn, "/dev/rrc%o%o", dn, j);
			break;
		case RD31:
		case RD32:
		case RD51:
		case RD52:
		case RD53:
		case RD54:
			if(fso < 8)			/* generate file name */
				sprintf(&fn, "/dev/rd%o%o", dn, j);
			else
				sprintf(&fn, "/dev/rrd%o%o", dn, j);
			break;
		case RX33:
		case RX50:
			if(fso < 8)			/* generate file name */
				sprintf(&fn, "/dev/rx%o", dn);
			else
				sprintf(&fn, "/dev/rrx%o", dn);
			break;
		}
		if((fswrs[j]==0) || (wflag==0))	/* open the file system */
			fd_rw[fso] = open(fn, 2);	/* write/read */
		else
			fd_rw[fso] = open(fn, 0);	/* read only */
		}
	return(fd_rw[fso]);
}

/*
 * This function converts the time from a time_t,
 * as returned by time(), to ascii in the form of
 * yymmddhhmmss.
 */

tconv(tim, timbuf)
time_t	*tim;
char	*timbuf;
{
	register int i;
	register char *p;
	int tb[6];

	tl = localtime(tim);
	tb[0] = tl->tm_year;
	tb[1] = tl->tm_mon + 1;
	tb[2] = tl->tm_mday;
	tb[3] = tl->tm_hour;
	tb[4] = tl->tm_min;
	tb[5] = tl->tm_sec;
	p = timbuf;
	for(i=0; i<6; i++) {
		*p++ = (tb[i]/10) + '0';
		*p++ = (tb[i]%10) + '0';
		}
	*p++ = 0;
}

/*
 * Check eventflags to stop
 * return 0 for continuation
 * return 1 to stop
 */
extern int errno;
checkflg()
{
	union efrt {
		long	efret;
		struct {
			int	a;
			int	b;
		} retval
	} ef;
	errno = 0;
	ef.efret = evntflg(EFRD, efid, (long)0);
	if(errno && ef.retval.a == -1) {
		zflag = 0;
		return(0);
	}
	if(ef.efret & (1L << efbit))
		return(1);
	return(0);
}
