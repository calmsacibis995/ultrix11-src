
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)rpx2.c	3.0	4/22/86";
/*
 * ULTRIX-11 RP disk exerciser program (rpx).
 * Fred Canter 5/10/83
 * Bill Burns 4/84
 *	fixed closing of arg file
 *	added event flags
 *
 * PART 2 - (rpx2.c)
 *
 *	Part 2 is the actual disk exerciser
 *
 *	Usage:
 *		rpx rpx_#.arg #
 *		    ^	      ^
 *		    |	      event flag bit position
 *		    argument file name
 *
 *
 *	CHANGES FOR USER SETABLE DISK PARTITIONS -- Fred Canter 7/6/85
 *
 *	The disk partition sizes table in the currently running kernel is
 *	compared with the standard sizes table, and the operation of RPX
 * 	is modified accordingly. The standard sizes table is hard coded
 *	into the RP exerciser via "#include /usr/sys/conf/dksizes.c".
 *
 *	If the sizes tables match, then RPX allows full functionality, i.e,
 *	it knows the disk layout and can protect the user from him/her self.
 *
 *	If the sizes tables don't match, it is assumed that the user changed
 *	the partition layout for some reason, and RPX operation is modified
 *	as follows:
 *
 *	1.	If the user changed partition 6 or 7, fatal error exit.
 *		(6 for RP02, 7 for RP03)
 *
 *	2.	If the disk is the system disk (root, swap, error log) or
 *		has any mounted file systems, then the entire disk will be
 *		treated as read only.
 *
 *	3.	If the disk is not the system disk and has no mounted file
 *		systems, then partition 6 or 7 will be a free fire zone. Only
 *		partition 6 or 7 will be used.
 *
 *	4.	The -f flag is ignored.
 *
 */

#include <sys/param.h>
#include <sys/devmaj.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>

#define	NPASS	5000

/*
 * File system layout, also
 * obtained from the unix kernel.
 */

int	nsdp;	/* non-standard disk partition layout being used */
int	dpmask;	/* bitwise 1 = partition used by drive being exericsed */

struct size
{
	daddr_t nblocks;
	int	cyloff;
} rpsizes[8];

/*
 * Unit number.
 */

int	dn;

/*
 * Drive types,
 *	0	= NED
 *	1	= RP02
 *	2	= RP03
 */
#define	RP02	01
#define	RP03	02

char	rp_dt[8];

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
char	diskdn[] = "-rp#";

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
 * Worst case data patterns.
 */

int	pat[] =
{
	0163126,
	0052525,
	0125252,
	0031463,
	0007417,
	0010421,
	0021042,
	0042104,
	0404210,
	0167356,
	0156735,
	0135673,
	0073567,
	0177777,	/* alternating 1's & 0's */
	0000000,	/* alternating 0's & 1's */
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

int	ndep, ndrop;

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
 * Read needed date from the `rpx_#.arg' file (# = drive).
 */
	argf = fopen(argv[1], "r");
	if(argf == NULL)
		exit(1);
	dn = getw(argf);
	ndep = getw(argf);
	ndrop = getw(argf);
	fflag = getw(argf);
	sflag = getw(argf);
	istime = getw(argf);
	ronly = getw(argf);
	bufsiz = getw(argf);
	bcmask = getw(argf);
	nsdp = getw(argf);
	dpmask = getw(argf);
#ifdef EFLG
	zflag = getw(argf);
#endif
	n = &rpsizes;
	for(i=0; i<sizeof(rpsizes); i++)
		*n++ = getc(argf);
	n = &rp_dt;
	for(i=0; i<sizeof(rp_dt); i++)
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
 *	0	block	random	512
 *	1	block	random	random, 2048 max
 *	2	raw	random	512
 *	3	raw	wc pat	8 count cycle
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
 *  3.	Select the data pattern to be used.
 *	Count 3 use the worst case data pattern.
 *	Counts 0, 1, 2 use a random data pattern.
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
		fprintf(stderr, "\nrpx: Can't calloc R/W buffers\n");
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
	switch(rp_dt[dn]) {
	case RP02:
		diskn = "RP02";
		break;
	case RP03:
		diskn = "RP03";
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
/*

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
	if(nsdp) {
		fs_rw = fsok(dn, 7);
		fs_rr = fs_rr;
	} else if(fflag) {
		for(fs_rw=0; fsact[fs_rw]==0; fs_rw++)
		{}
		fs_rr = fs_rw;
		if(fs_rw != fsok(dn, fs_rw)) {
			fprintf(stderr,"\nrpx: invalid file system selected\n");
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
	fsz = rpsizes[fs_rw].nblocks;
	bn_rw = (fsz / 8) * (rng() & 7);
	bn_rw += (unsigned int)rng();
	lbt = bn_rw + ((nbytes + 511) / 512);
	if(lbt > fsz)
		bn_rw -= (lbt - fsz);
	bn_rr = (unsigned int)rng();	/* bias toward lower numbers */
	lbt = bn_rr + ((nbytes + 511) / 512);
	fsz = rpsizes[fs_rr].nblocks;
	if(lbt > fsz)
		bn_rr -= (lbt - fsz);
/*
 * Set the file descriptor (efd) for
 * the correct I/O mode, i.e., block or raw.
 */
	a = count & 02;
	if(a) {
		if((efd = fsopen(dn, fs_rw+8)) < 0) {	/* raw I/O */
		opnerr:
			fprintf(stderr,"\nrpx: Can't open %s\n", fn);
			exit(1);
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
 * The worst case data pattern is
 * selected form pat[] at random.
 */
	wbp = wbuf;
	wbp += w_off;
	j = nbytes/2;
	if(cnt == 3) {
		a = pat[rng() & 017];
		if((a == 0) || (a == -1))
			for(ctr=0; ctr<j; ctr++) {
				if(ctr & 1)
					*wbp++ = ~a;
				else
					*wbp++ = a;
				}
		else
			for(ctr=0; ctr<j; ctr++)
				*wbp++ = a;
	} else {
		for(ctr=0; ctr<j; ctr++) {
			if((ctr & 0377) == 0)
				rbp = rng();
			*wbp++ = rbp;
			}
		}
/*
 * Write to disk and count write operations.
 *(only if file system is write/read)
 */
e_erc:	/* Entry point for hard error xfer restart */
	if(fswrs[fs_rw] == 0) {
		lseek(efd, (long)(bn_rw * 512), 0);
		wrtcnt++;
		wbp = wbuf;
		wbp += w_off;
		if((rbc = write(efd, (char *)wbp, nbytes)) != nbytes)
			dskse((cnt | 010), dn, fs_rw, bn_rw, nbytes);
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
			}
		fsterr = 1; /* first error flag */
		if(fswrs[fs_rw] == 0)
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
 * If a hard read error occurs on a multi block transfer
 * , all blocks after the bad block will fail the data
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
		bdflush((RP_BMAJ<<8)|(dn<<3)|fs_rw);
next:
	if(++count >= NPASS) {
		count = 0;
		time(&timbuf);
	    printf("\n%s disk exerciser end of pass - %s",diskn,ctime(&timbuf));
		fflush(stdout);
		if((i = fork()) > 0)
			exit(0);
		if(i == -1)
			fprintf(stderr,"\nrpx: Can't fork new copy of rpx !\n");
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

	switch(rp_dt[dn]) {
	case RP02:
		if((dpmask & (1 << filsys)) == 0)
			return(6);
		break;
	case RP03:
		if((dpmask & (1 << filsys)) == 0)
			return(7);
		break;
	}
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
		if(fso < 8)			/* generate file name */
			sprintf(&fn, "/dev/rp%o%o", dn, j);
		else
			sprintf(&fn, "/dev/rrp%o%o", dn, j);
		if(fswrs[j] == 0)		/* open the file system */
			fd_rw[fso] = open(fn, 2);	/* write/read */
		else
			fd_rw[fso] = open(fn, 0);	/* read only */
		}
	return(fd_rw[fso]);
}

/*
 * This function converts the time from a time_t
 * ,as returned by time(), to ascii in the form of
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
