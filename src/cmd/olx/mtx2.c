
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)mtx2.c	3.0	4/22/86";
/*
 * ULTRIX-11 mag tape disk exerciser program (mtx).
 *
 * PART 2 - (mtx2.c)
 *
 *	Part 2 is called from part 1 (mtx) with arguments in the
 *	file `mtx_??.arg' (?? = ts, tm, ht, tk). This is the actual
 *	magtape exerciser. The mtx program is split into two sections
 *	to optimize memory usage.
 *
 * Fred Canter 10/15/82
 * Bill Burns 4/82
 *	added event flag usage
 * Chung-Wu Lee 2/22/85
 *	added TMSCP
 * George Mathew 7/2/85
 *	changes for 1K file system
 *
 * This program exercises the tm11 - tu10/ts03, ts11/tsv05/tu80/tk25,
 * tm02/3 - tu16/te16/tu77, or tk50 - tk50/tu81 mag tape subsystems,
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
 *	mtx mtx_??.arg efpis efids
 *		^      ^
 *		|      event flag bit position and id
 *		argument passing file name; ht, mt, or ts
 *
 */

#include <sys/param.h>	/* Don't matter which one ! */
#include <sys/devmaj.h>
#include <sys/tk_info.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>

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
 * tk & rtk are for tk50 only.
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

char	*afp;	/* Argument file pointer */

char	eplmsg[] "\n\n[error printout limit exceeded]";
char	noeof[] "\n\n[missing EOF or extra record(s) at end of file]";

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
 * Tape write, read, hard error counts.
 */

long	rdcnt[64];
long	wrtcnt[64];
long	hecnt[64];

/*
 * The following three lines of code
 * are the interface to the system call
 * error return messages.
 */

int	errno;
int	sys_nerr;
char	*sys_errlist[];

time_t	stbuf;	/* I/O statistics time */
time_t	btbuf;	/* Exerciser run beginning time */
time_t	etbuf;	/* Exerciser run ending time */
time_t	timbuf;
struct tm *tl;	/* structure for localtime */
char	btime[13];	/* ascii beg time yymmddhhmmss */
char	etime[13];	/* ascii end time yymmddhhmmss */

char	*tapen;	/* name of tape controller */

int	wrterr;
int	werec;
int	rderr;
long	randx;
int	nfeet[MAXTK];
int	pat[] =
{
	0000000,	/* lo & hi byte all 0's */
	0177777,	/* lo & hi byte all 1's */
	0000377,	/* lo byte all 1's, hi byte all 0's */
	0177400,	/* lo byte all 0's, hi byte all 1's */
	0052525,	/* lo & hi byte alt 1's & 0's */
	0125252,	/* lo & hi byte alt 0's & 1's */
	0125125,	/* lo byte alt 1's & 0's, hi byte comp. */
	0052652,	/* lo byte alt 0's & 1's, hi byte comp. */
	0177001,	/* lo byte walking 1, hi byte walking 0 */
	0176402,	/* "	*/
	0175404,	/* "	*/
	0173410,	/* "	*/
	0167420,	/* "	*/
	0157440,	/* "	*/
	0137500,	/* "	*/
	0077600,	/* "	*/
	0125220,	/* address pattern, in case of  */
	0052521,	/* write buffer overrun	*/
	0166422,
	0177423,
};

/*
 * Write and read buffer address offsets.
 */

int	w_off, r_off;
int	testno;

/*
 * Write/Read buffer,
 * large enough for 20 512-byte records.
 */

#ifdef UCB_NKB
char	buf[20*1024];
#else
char	buf[20*512];
#endif

int	sflag, tmflag, htflag, tsflag, tkflag;

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

int	stopsig;

int	ndrop, ndep;

int	rbc;	/* byte count returned from read/write calls */

main(argc, argv)
char *argv[];
int argc;
{
	int	stop(), intr();
	register *wp, *rp;
	register int k;
	FILE	*argf;
	int	*wba, *rba;
	int	*wbp, *rbp;
	int 	i, j;
	int	dn, fd, md, maxdrvs;
	int	nrec, nbytes;
	int	cnt, iomode, density;
	int	fsterr, neb;
	int	dpat;
	char	*p;
	char	c;
#ifdef UCB_NKB
	int     maxrec, recsize;
#endif

/*
 * Read needed data from the `mtx_??.arg' file,
 * and start the exerciser.
 */

	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, intr);
	signal(SIGQUIT, stop);
	close(stdin);
	if((argc < 2) || (argc > 4))
		exit(1);
	afp = argv[1];
	if((fd = open(afp, RD)) < 0) {
		fprintf(stderr, "\nmtx: Can't open %s\n", afp);
		exit(1);
		}
	if((rbc = read(fd, (char *)&buf, 512)) != 512) {
		fprintf(stderr, "\nmtx: %s read error\n", afp);
		exit(1);
		}
	p = &buf;
	for(i=0; i<64; i++)
		mt_dt[i] = *p++;
	rbp = p;
	sflag = *rbp++;
	tsflag = *rbp++;
	tmflag = *rbp++;
	htflag = *rbp++;
	tkflag = *rbp++;
	for(i=0; i<MAXTK; i++)
		nfeet[i] = *rbp++;
	ndep = *rbp++;
	ndrop = *rbp++;
	maxdrvs = *rbp++;
#ifdef EFLG
	zflag = *rbp++;
#endif
	close(fd);
	unlink(afp);
#ifdef EFLG
	if(zflag) {
		efpis = argv[2];
		efids = argv[3];
	}
#else
	killfn = argv[2];	/* run/stop control file name */
#endif

/*
 * Print the mtx started message.
 */
	time(&btbuf);
	randx = btbuf & 0777;
	if(tmflag)
		tapen = "TM11";
	if(tsflag)
		tapen = "TS11/TSV05/TSU05/TU80/TK25";
	if(htflag)
		tapen = "TM02/3";
	if(tkflag)
		tapen = "TK50/TU81";
	printf("\n\n%s magtape exerciser started - %s\n", tapen, ctime(&btbuf));
	fflush(stdout);
#ifdef EFLG
	if(zflag) {
		efbit = atoi(efpis);
		efid = atoi(efids);
		evntflg(EFCLR, efid, (long)efbit);
	}
#else
	unlink(killfn);
#endif
/*
 * TEST 1 - SHORT FILE TEST
 *
 * The following test is done on each of the selected tape
 * units that is available. 
 * The test is done in block and raw I/O modes for 800, 1600
 * 6250 BPI density or just tk50
 * The test consists of writing a number of 512 byte records
 * to tape from the write buffer.
 * Reading the same number of records into the read buffer
 * and comparing the data in the write and read buffers.
 * A pass is 32 of the above write/read/compare tests.
 * The number of records starts at one and increases to 16,
 * and then decreases from 16 back down to one.
 * The test insures that the correct number of tape records
 * were written by attempting to read one extra record
 * and checking for an end of file error.
 * The write address is rotated from the end of the
 * write buffer towards the beginning write buffer.
 * The read address is rotated, within a very narrow range
 * ( 32 words), from the start towards the end of
 * the read buffer.
 * The word before the start of the read data in the read
 * buffer and the word after the read data are tested
 * to insure that the data was transferred only to the
 * correct addresses in the read buffer.
 * The data patterns in pat[0] thru pat[19] are loaded
 * into the write buffer at the start of the test and
 * are constant throughout the test 1.
 */
loop:
	wba = &buf;
	rba = wba + (18*256);
	wp = wba;
	*wp++ = pat[0];		/* load first word of write buffer */
	for(j=0; j<17; j++)	/* fill write buffer with data patterns */
		for(k=0; k<256; k++)
			*wp++ = pat[j];
	testno = 1;
	for(cnt=0; cnt<256; cnt++)
	{			/* BIG loop, tests all units in all modes */

	dn = cnt & 077;		/* unit # to be tested */
	iomode = cnt & 0100;	/* block or raw I/O mode */
/*
 * density 0 -  800 BPI (tm11, tm02/3), 1600 BPI (tu81) or tk50
 *         1 - 1600 BPI (ts11, tm02/3), 6250 BPI (tu81) but no tk50
 */
	density = cnt & 0200;	/* 800 ro 1600 BPI density */
	if(!mt_dt[dn])
		continue;	/* unit not selected */
	if(!density && ((mt_dt[dn] & 07) == 3))
		continue;	/* no 800 BPI on ts11 */
	if(density && ((mt_dt[dn] & 07) == 1 || (mt_dt[dn] & 07) == 5))
		continue;	/* no 1600 BPI if unit is tm11 */
	mtname(dn, density, iomode);	/* generate magtape filename */
#ifndef UCB_NKB
	for(i=0; i<32; i++) {	/* one test pass */
	fd = mtopen(WRT);
	if(i < 16)	/* set up record count & write offset */
		nrec = i + 1;
	else
		nrec = 32 - i;
	w_off = (16 - nrec)*256+1;
	wbp = wba + w_off;
	for(j=0; j<nrec; j++) {
		wrterr = 0;
		wrtcnt[dn]++;
		wp = wbp + (j*256);
		/* intr();	OLD: poll for stop */
		if((rbc = write(fd, (char *)wp, 512)) != 512) {
			trwse(WRT,dn,density,iomode,512,nrec,j+1);
/*
 * WRITE errors are fatal, the "tuready"
 * function waits a minimum of 15 minutes
 * for the tape unit to be made ready.
 */
			fd = tuready(fd);
			break;
			}
		}
		close(fd);
		if(wrterr) {	/* write error, so read must be adjusted */
			printf("\n\n******\n");
			if(werec == 0) {
				printf("READ ABORTED - no records written");
				printf("\n******\n");
				continue;
			} else {
				nrec = werec;
			printf("READ SHORTENED - %d records will be read",nrec);
				}
			printf("\n******\n");
			}
		fd = mtopen(RD);
		for(j=0; j<nrec; j++) {
/*
 * The read buffer offset is random
 * and in the range of 1 - 4096.
 */
			rderr = 0;
			r_off = i + j;
			rbp = rba + r_off;
			for(k= -1; k<257; k++)
				*(rbp + k) = ~*(wbp + (j*256) + k);
			rdcnt[dn]++;
			/* intr(); */
			if((rbc = read(fd, (char *)rbp, 512)) != 512)
				trwse(RD,dn,density,iomode,512,nrec,j+1);
			fsterr = 1;
			neb = 0;
/*
 * Buffer over/under flow test.
 * Check the words before and after the buffer
 * to insure that data was read into the
 * assigned buffer area and no where else.
 */
	if((*(rbp - 1) != ~*(wbp + (j*256) - 1)) ||
	(*(rbp + 256) != ~*(wbp + (j*256) + 256))) {
		if(!rderr)
			trwse(DMM,dn,density,iomode,512,nrec,j+1);
		rderr++;
		printf("\n\nREAD BUFFER OVERRUN ERROR");
		printf("\n\nWord before read buffer");
		pgb(-1, *(wbp + (j*256) - 1), *(rbp - 1));
		printf("\n\nWord after read buffer");
		pgb(-1, *(wbp + (j*256) + 256), *(rbp + 256));
		}
			for(k=0; k<256; k++) {
				if(*(rbp + k) != *(wbp + (j*256) + k)) {
					if(fsterr && !rderr)
						trwse(DMM,dn,density,iomode,512,nrec,j+1);
					if(fsterr)
		printf("\n\nDATA COMPARE ERROR - RECORD %u",j+1);
					fsterr = 0;
					if(++neb > ndep) {
						printf("%s",&eplmsg);
						break;
						}
			pgb(k, *(wbp + (j*256) + k), *(rbp + k));
					}
				}
				if(!fsterr)
					printf("\n******\n");
				if(rderr && (errno == ETPL)) {
					printf("\n\n[READ ABORTED - ");
					printf("fatal read error]\n******\n");
					fd = tuready(fd); /* wait for tape ready */
					goto fte1;
					}
				if(rderr && fsterr)
					printf("\n******\n");
			}
#else UCB_NKB
	if (iomode) {
		maxrec = 16;
		recsize = 512;
	} else {
		maxrec = 8;
		recsize = 1024;
	}
	for(i=0; i<(maxrec*2); i++) {	/* one test pass */
	fd = mtopen(WRT);
	if(i < maxrec)	/* set up record count & write offset */
		nrec = i + 1;
	else
		nrec = maxrec*2  - i;
	w_off = (maxrec - nrec)*(recsize/2)+1;
	wbp = wba + w_off;
	for(j=0; j<nrec; j++) {
		wrterr = 0;
		wrtcnt[dn]++;
		wp = wbp + (j*(recsize/2));
		/* intr();	OLD: poll for stop */
		/*printf("before write: stopsig= %d\n",stopsig); */
		if((rbc = write(fd, (char *)wp, recsize)) != recsize) {
			trwse(WRT,dn,density,iomode,recsize,nrec,j+1);
/*
 * WRITE errors are fatal, the "tuready"
 * function waits a minimum of 15 minutes
 * for the tape unit to be made ready.
 */
			fd = tuready(fd);
			break;
			} /* else */
			/*printf("\n*** write successful, record no=%d, nrec=%d\n",j+1,nrec); */
		}
		close(fd);
		if(wrterr) {	/* write error, so read must be adjusted */
			printf("\n\n******\n");
			if(werec == 0) {
				printf("READ ABORTED - no records written");
				printf("\n******\n");
				continue;
			} else {
				nrec = werec;
			printf("READ SHORTENED - %d records will be read",nrec);
				}
			printf("\n******\n");
			}
		fd = mtopen(RD);
		for(j=0; j<nrec; j++) {
/*
 * The read buffer offset is random
 * and in the range of 1 - 4096.
 */
			rderr = 0;
			r_off = i + j;
			/* printf("r_off=%d\t",r_off); */
			rbp = rba + r_off;
			/* printf("read buffer ptr: %d\n",rbp);*/
			/* printf("before loop: stopsig= %d\t",stopsig); */
			for(k= -1; k<(recsize/2 +1) ; k++)
				*(rbp + k) = ~*(wbp + (j*(recsize/2)) + k);
			/* printf("after loop: stopsig= %d\n",stopsig); */
			rdcnt[dn]++;
			/* intr(); */
			if((rbc = read(fd, (char *)rbp, recsize)) != recsize) {
				trwse(RD,dn,density,iomode,recsize,nrec,j+1);
				/* printf("\n** no. of chars read=%d\n",rbc); */
			} else {
				/* printf("\n** read successful, record no=%d, nrec=%d",j+1,nrec); */
			}
			/* printf("after read: stopsig= %d\n", stopsig); */
			fsterr = 1;
			neb = 0;
/*
 * Buffer over/under flow test.
 * Check the words before and after the buffer
 * to insure that data was read into the
 * assigned buffer area and no where else.
 */
	if((*(rbp - 1) != ~*(wbp + (j*(recsize/2)) - 1)) ||
	(*(rbp + (recsize/2)) != ~*(wbp + (j*(recsize/2)) + (recsize/2)))) {
		if(!rderr)
		/* printf("\ncalling trwse after read, stopsig=%d\n",stopsig); */
			trwse(DMM,dn,density,iomode,recsize,nrec,j+1);
		rderr++;
		printf("\n\nREAD BUFFER OVERRUN ERROR");
		printf("\n\nWord before read buffer");
		pgb(-1, *(wbp + (j*(recsize/2)) - 1), *(rbp - 1));
		printf("\n\nWord after read buffer");
		pgb(-1, *(wbp + (j*(recsize/2)) + (recsize/2)), *(rbp + (recsize/2)));
		}
			for(k=0; k<(recsize/2); k++) {
				if(*(rbp + k) != *(wbp + (j*(recsize/2)) + k)) {
					if(fsterr && !rderr)
						trwse(DMM,dn,density,iomode,recsize,nrec,j+1);
					if(fsterr)
		printf("\n\nDATA COMPARE ERROR - RECORD %u",j+1);
					fsterr = 0;
					if(++neb > ndep) {
						printf("%s",&eplmsg);
						break;
						}
			pgb(k, *(wbp + (j*(recsize/2)) + k), *(rbp + k));
					}
				}
				if(!fsterr)
					printf("\n******\n");
				if(rderr && (errno == ETPL)) {
					printf("\n\n[READ ABORTED - ");
					printf("fatal read error]\n******\n");
					fd = tuready(fd); /* wait for tape ready */
					goto fte1;
					}
				if(rderr && fsterr)
					printf("\n******\n");
			}
#endif
/*
 * Check for end of file (EOF).
 * Attempt to read one more record, if an error is
 * returned then EOF is present. If read is successful
 * then the EOF is missing or an extra record was written.
 */
		if(iomode)	/* only RAW mode for now */
		{
		/* intr(); */
		if((rbc = read(fd, (char *)rba, 512)) != 0) {
			trwse(MEOF,dn,density,iomode,512,nrec,(nrec+1));
			printf("%s", &noeof);
			if(wrterr) {
				printf("\n[Most likely due to previous");
				printf(" fatal write error]\n******\n");
				}
			}
		}
	fte1:
		fflush(stdout);
		close(fd);
		}
/*
 * The following is necessary because the tm03 tape controller
 * will not allow the density to be changed unless the
 * tape is at load point.
 * This causes the first write after a density change to fail.
 * To avoid this problem, the tape is opened at the previous
 * density, the first record is read, the tape is closed, and
 * then the process sleeps for 2 seconds ( or so ) while the
 * tape drive is rewound to load point. 
 */
		if(htflag && ((!density && iomode) || (density && iomode))) {
			fd = mtopen(RD);
			read(fd, (char *)rba, 512);
			close(fd);
			sleep(1);
			sleep(1);
			}
	}

/*
 * TEST 2 - VARIABLE LENGTH RECORD TEST
 *
 * This test writes one record with a length that varies
 * from 512 bytes to 10240 bytes in 512 byte increments.
 * The record is read and the data comapared with
 * the test pattern written.
 * The write/read address is the start of the buffer.
 * The data patterns are the complement of those
 * in test 1, pat[0] thru pat[19].
 * The test is done for all selected and available drives,
 * in raw I/O mode, at 800 BPI and 1600 BPI
 * (if rm02/3) densities.
 */

	wba = &buf;
	rba = &buf;
	w_off = 0;
	r_off = 0;
	iomode = 1;	/* RAW I/O mode only ! */
	testno = 2;
	for(cnt=0; cnt<128; cnt++)
	{			/* BIG loop, tests all units */

	dn = cnt & 077;		/* unit # to be tested */
/*
 * density 0 -  800 BPI (tm11, tm02/3), 1600 BPI (tu81) or tk50
 *         1 - 1600 BPI (ts11, tm02/3), 6250 BPI (tu81) but no tk50
 */
	density = cnt & 0100;	/* 800 ro 1600 BPI density */
	if(!mt_dt[dn])
		continue;	/* unit not selected */
	if(!density && ((mt_dt[dn] & 07) == 3))
		continue;	/* no 800 BPI on ts11 */
	if(density && ((mt_dt[dn] & 07) == 1 || (mt_dt[dn] & 07) == 5))
		continue;	/* no 1600 BPI if unit is tm11 */
	mtname(dn, density, iomode);	/* generate magtape filename */
	for(i=0; i<20; i++) {	/* one test pass */
	fd = mtopen(WRT);
	nbytes = 512 + (i*512);
	wp = wba;			/* load write buffer with pattern */
	for(j=0; j<(i+1); j++)
		for(k=0; k<256; k++)
			*wp++ = ~pat[j];
	nrec = 1;
	wrterr = 0;
	wrtcnt[dn]++;
	/* intr(); */
	if((rbc = write(fd, (char *)wba, nbytes)) != nbytes) {
		trwse(WRT,dn,density,iomode,nbytes,nrec,1);
		fd = tuready(fd);	/* wait for tape unit ready */
		}
	close(fd);
	if(wrterr) {	/* fatal write error, no read */
		printf("\n\n******\nREAD ABORTED - no records");
		printf(" written\n******\n");
		continue;
		}
	fd = mtopen(RD);
	rderr = 0;
	rp = rba;		/* load read buffer with complement pattern */
	for(j=0; j<(i+1); j++)
		for(k=0; k<256; k++)
			*rp++ = pat[j];
	rdcnt[dn]++;
	/* intr(); */
	if((rbc = read(fd, (char *)rba, nbytes)) != nbytes)
		trwse(RD,dn,density,iomode,nbytes,nrec,1);
	fsterr = 1;
	neb = 0;
	rp = rba;
	for(k=0; k<(nbytes/2); k++) {
		if(*rp++ != ~pat[k/256]) {
			if(fsterr && !rderr)
				trwse(DMM,dn,density,iomode,nbytes,nrec,1);
			if(fsterr)
		printf("\n\nDATA COMPARE ERROR - RECORD %u",1);
			fsterr = 0;
			if(++neb > ndep) {
				printf("%s",&eplmsg);
				break;
				}
			pgb(k, ~pat[k/256], *(rba + k));
			}
		}
		if(!fsterr)
			printf("\n******\n");
		if(rderr && (errno == ETPL)) {
			printf("\n\n[READ ABORTED - ");
			printf("fatal read error]\n******\n");
			fd = tuready(fd);
			goto fte2;
			}
		if(rderr && fsterr)
			printf("\n******\n");
/*
 * Check for end of file (EOF).
 * Attempt to read one more record, if an error is
 * returned then EOF is present. If read is successful
 * then the EOF is missing or an extra record was written.
 */
		/* intr(); */
		if((rbc = read(fd, (char *)rba, 512)) != 0) {
			trwse(MEOF,dn,density,iomode,512,nrec,(nrec+1));
			printf("%s", &noeof);
			}
fte2:
	fflush(stdout);
	close(fd);
	}
/*
 * The following is necessary because the tm03 tape controller
 * will not allow the density to be changed unless the
 * tape is at load point.
 * This causes the first write after a density change to fail.
 * To avoid this problem, the tape is opened at the previous
 * density, the first record is read, the tape is closed, and
 * then the process sleeps for 2 seconds ( or so ) while the
 * tape drive is rewound to load point. 
 */
	if(htflag) {
		fd = mtopen(RD);
		read(fd, (char *)rba, 10240);
		close(fd);
		sleep(1);
		sleep(1);
		}
	}

/*
 * TEST 3 - LARGE FILE TEST
 *
 * This test simulates very large files, such as dump/restore
 * tapes. It writes enough records to fill the
 * number of feet of tape specified by "nfeet".
 * The value of "nfeet" can be specifed by the [-f#] option,
 * the default value is 500 feet. The minimum value is 10 feet and
 * the maximum is 2400 feet.
 * The test insures that the correct number of tape records
 * were written by attempting to read an extra record
 * and checking for an end of file error.
 * The test is done in block and raw I/O modes at 800
 * and 1600 BPI density.
 * The record size is 1024 bytes for block mode and
 * 10240 bytes for raw mode.
 * For the 1024 bytes records the write/read addresses are
 * rotated thru the buffer on block boundries.
 * For the 10240 byte records the write/read address
 * is the start of the buffer.
 * Test data patterns (pat[0] -> pat[19]) are used
 * for this test.
 */

	testno = 3;
	wba = &buf;
	rba = &buf;
	for(cnt=0; cnt<256; cnt++)
	{			/* BIG loop, tests all units in all modes */

	dn = cnt & 077;		/* unit # to be tested */
	iomode = cnt & 0100;	/* block or raw I/O mode */
/*
 * density 0 -  800 BPI (tm11, tm02/3), 1600 BPI (tu81) or tk50
 *         1 - 1600 BPI (ts11, tm02/3), 6250 BPI (tu81) but no tk50
 */
	density = cnt & 0200;	/* 800 ro 1600 BPI density */
	if(!mt_dt[dn])
		continue;	/* unit not selected */
	if(!density && ((mt_dt[dn] & 07) == 3))
		continue;	/* no 800 BPI on ts11 */
	if(density && ((mt_dt[dn] & 07) == 1 || (mt_dt[dn] & 07) == 5))
		continue;	/* no 1600 BPI if unit is tm11 */
/*
 * Use the density and iomode to construct the name of the
 * mag tape special file to be opened, the number of records
 * to write , and the record size as follows:
 *
 *	density	mode	size	# rec's per 10 feet
 *	800	block	512	55
 *	800	raw	10240	8
 *	1600	block	512	70
 *	1600	raw	10240	15
 *	1600	block	1024	70  - tu81
 *	1600	raw	10240	15  - tu81
 *	6250	block	1024	160
 *	6250	raw	10240	35
 *	tk50	block	512	95
 *	tk50	raw	10240	15
 */
	if(!density && !iomode) {		/* low density & block mode */
		if (mt_dt[dn] == 5) {		/* tk50 */
			nbytes = 1024;
			nrec = nfeet[dn];
			}
		else if (mt_dt[dn] == 4) {	/* 1600 BPI for tu81 */
			nbytes = 1024;
			nrec = (nfeet[dn]/10)*70;
			}
		else {				/* 800 BPI for the rest */
#ifdef UCB_NKB
			nbytes = 1024;
			nrec = (nfeet[0]/10)*55;
#else
			nbytes = 512;
			nrec = (nfeet[0]/10)*76;
#endif
			}
		}
	if(!density && iomode) {		/* low density & raw mode */
		if (mt_dt[dn] == 5) {		/* tk50 */
			nbytes = 10240;
			nrec = nfeet[dn];
			}
		else if (mt_dt[dn] == 4) {	/* 1600 BPI for tu81 */
			nbytes = 10240;
			nrec = (nfeet[dn]/10)*15;
			}
		else {				/* 800 BPI for the rest */
			nbytes = 10240;
			nrec = (nfeet[0]/10)*8;
			}
		}
	if(density && !iomode) {		/* high density & block mode */
		if (mt_dt[dn] == 4) {		/* 6250 BPI for tu81 */
			nbytes = 1024;
			nrec = (nfeet[dn]/10)*160;
			}
		else {				/* 1600 BPI for the resr */
#ifdef UCB_NKB
			nbytes = 1024;
			nrec = (nfeet[0]/10)*70;
#else
			nbytes = 512;
			nrec = (nfeet[0]/10)*95;
#endif
			}
		}
	if(density && iomode) {			/* high density & raw mode */
		if (mt_dt[dn] == 4) {		/* 6250 BPI for tu81 */
			nbytes = 10240;
			nrec = (nfeet[dn]/10)*35;
			}
		else {				/* 1600 BPI for the rest */
			nbytes = 10240;
			nrec = (nfeet[0]/10)*15;
			}
		}
	mtname(dn, density, iomode);	/* generate magtape filename */
	fd = mtopen(WRT);
	wp = wba;		/* load write buffer with test pattern */
	for(j=0; j<5120; j++)
		*wp++ = pat[j/256];
	for(j=0; j<nrec; j++) {
#ifdef UCB_NKB
		if((nbytes == 512) || (nbytes == 1024))
#else
		if(nbytes == 512)
#endif
			w_off = (j&017)*256;
		else
			w_off = 0;
		wrterr = 0;
		wrtcnt[dn]++;
		wbp = wba + w_off;
		/* intr(); */
		if((rbc = write(fd, (char *)wbp, nbytes)) != nbytes) {
			trwse(WRT,dn,density,iomode,nbytes,nrec,j+1);
			fd = tuready(fd);	/* wait for tape unit ready */
			break;
			}
		}
		close(fd);
		if(wrterr) {	/* write error, so read must be adjusted */
			printf("\n\n******\n");
			if(werec == 0) {
				printf("READ ABORTED - no records written");
				printf("\n******\n");
				continue;
			} else {
				nrec = werec;
				printf("READ SHORTENED - ");
				printf("%d records will be read",nrec);
				}
			printf("\n******\n");
			}
		fd = mtopen(RD);
		for(j=0; j<nrec; j++) {
#ifdef UCB_NKB
			if((nbytes == 512) || (nbytes == 1024))
#else
			if(nbytes == 512)
#endif
				r_off = (j&017)*256;
			else
				r_off = 0;
			w_off = r_off;
			rderr = 0;
			rp = rba + r_off;
			for(k=0; k<(nbytes/2); k++)
				*rp++ = 0;
			rdcnt[dn]++;
			rbp = rba + r_off;
			/* intr(); */
			if((rbc = read(fd, (char *)rbp, nbytes)) != nbytes)
				trwse(RD,dn,density,iomode,nbytes,nrec,j+1);
			fsterr = 1;
			neb = 0;
			rp = rba + r_off;
			for(k=0; k<(nbytes/2); k++) {
				dpat = (r_off/256) + (k/256);
				if(*rp++ != pat[dpat]) {
					if(fsterr && !rderr)
					trwse(DMM,dn,density,iomode,nbytes,nrec,j+1);
					if(fsterr)
		printf("\n\nDATA COMPARE ERROR - RECORD %u",j+1);
					fsterr = 0;
					if(++neb > ndep) {
						printf("%s",&eplmsg);
						break;
						}
			pgb(k, pat[dpat], *(rbp + k));
					}
				}
				if(!fsterr)
					printf("\n******\n");
				if(rderr && (errno == ETPL)) {
					printf("\n[READ ABORTED - ");
					printf("fatal read error]\n******\n");
					fd = tuready(fd);
					goto fte3;
					}
				if(rderr && fsterr)
					printf("\n******\n");
			}
/*
 * Check for end of file (EOF).
 * Attempt to read one more record, if an error is
 * returned the EOF is present. If read is successful
 * then EOF is missing or an extra record was written.
 */
		if(iomode)	/* only in RAW mode for now ! */
		{
		/* intr(); */
		if((rbc = read(fd, (char *)rba, nbytes)) != 0) {
			trwse(MEOF,dn,density,iomode,nbytes,nrec,(nrec+1));
			printf("%s", &noeof);
			if(wrterr) {
				printf("\n[Most likely due to previous");
				printf(" fatal write error]\n******\n");
				}
			}
		}
	fte3:
		fflush(stdout);
		close(fd);
/*
 * The following is necessary because the tm03 tape controller
 * will not allow the density to be changed unless the
 * tape is at load point.
 * This causes the first write after a density change to fail.
 * To avoid this problem, the tape is opened at the previous
 * density, the first record is read, the tape is closed, and
 * then the process sleeps for 2 seconds ( or so ) while the
 * tape drive is rewound to load point. 
 */
		if(htflag && ((!density && iomode) || (density && iomode))) {
			fd = mtopen(RD);
			read(fd, (char *)rba, 10240);
			close(fd);
			sleep(1);
			sleep(1);
			}
		}
	time(&timbuf);
	printf("\n%s exerciser end of pass - %s", tapen, ctime(&timbuf));
	if(!sflag)
		pios();	/* print I/O stats */
	fflush(stdout);
	if((i = fork()) == 0)
		goto loop;
	if(i == -1) {
		fprintf(stderr, "\nmtx: Can't fork new copy of mtx !\n");
		goto loop;
	}
	exit(0);
}

/*
 * This function is called when a exerciser run is
 * terminated via the delete key. This is done by
 * catching the interrupt signal.
 * The final I/O statistics are printed and the
 * the error log printout program is called via execl.
 * `elp' will print only the errors for the selected
 * tape controller which occurred durring the exerciser run.
 */

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
	register char *tn;

	stopsig++;
	signal(SIGTERM, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	time(&etbuf);	/* ending time */
	printf("\n\n%s magtape exerciser stopped - %s\n", tapen, ctime(&etbuf));
	pios();		/* print I/O stats */
	tconv(&btbuf, &btime);	/* convert beg time to ascii */
	tconv(&etbuf, &etime);	/* convert end time to ascii */
	fflush(stdout);
	if(tsflag)
		tn = "-ts";
	else if(tmflag)
		tn = "-tm";
	else if(htflag)
		tn = "-ht";
	else
		tn = "-tk";
	if(fork() == 0)
	  	execl("/bin/elp","elp","-s",tn,"-d",&btime,&etime,(char *)0);
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
 * Tape read/write status error printout function.
 * rw = type of function being performed.
 *  0 = read, 1 = write, 2 = data mismatch, 3 = missing EOF
 * un = tape unit number
 * den = 0 for 800 BPI, den = !0 for 1600 BPI
 * iom = 0 for block I/O mode , iom = !0 for raw I/O mode
 * rl = record length in bytes
 * nr = number of records
 * rn = record number
 */

trwse(rw, un, den, iom, rl, nr, rn)
{
	char tp;
	register int i, j;

	if(stopsig)
		stop();	/* in case of signal durring system call */
	tp = mt_dt[un];
	hecnt[un]++;
	time(&timbuf);
	printf("\n\n******\n");
	if(rw == DMM)
		printf("TEST %d - DATA MISMATCH WITHOUT READ", testno);
	else {
		printf("TEST %d - HARD TAPE ", testno);
		if(rw == WRT)
			printf("WRITE");
		else
			printf("READ");
		}
	printf(" ERROR - %s",ctime(&timbuf));
	if(rw < DMM) {
		printf("Returned byte = %d (-1 = error)", rbc);
		printf("\nError type: ");
		if(errno < sys_nerr)
			printf("%s\n", sys_errlist[errno]);
		else
			printf("Unknown error\n");
		}
	printf("\nunit\tdensity\tI/O\trecord\t# of\trecord");
	printf("\nnumber\tBPI\tmode\tnumber\trecords\tlength\n");
	printf("\n%d\t",un);
	if(den) {
		if (tp == 2 || tp == 3)
			printf("1600\t");
		else
			printf("6250\t");
		}
	else {
		if (tp == 1 || tp == 2)
			printf("800\t");
		else if (tp == 4)
			printf("1600\t");
		else
			printf("tk50\t");
		}
	if(iom)
		printf("raw\t");
	else
		printf("block\t");
	printf("%d\t%d\t%d bytes",rn,nr,rl);
	if(rw != MEOF)
		printf("\n\nWrite buffer address = %o", w_off);
	if(rw == RD || rw == DMM)
		printf("\nRead  buffer address = %o", r_off);
	if(rw == WRT) {
		printf("\n\n[FATAL ERROR - write termintated");
		printf(" at record %d of %d]", rn, nr);
		wrterr++;
		werec = rn - 1;
		printf("\n******\n");
		}
	if(rw == RD)
		rderr++;
	if(hecnt[un] >= ndrop) {
	    printf("\n\nTotal error limit exceeded, unit %d dropped !\n", un);
		mt_dt[un] = 0;
	}
	fflush(stdout);
	for(i=0, j=0; i<64; i++)
		j += mt_dt[i];
	if(j == 0) {
		for( ;; )
			sleep(3600);
	}
}

/*
 * Print the tape read/write/hard error
 * statistics.
 */

pios()
{
	register j;

	time(&stbuf);
	randx = stbuf & 0777;
	printf("\n\nI/O statistics - %s", ctime(&stbuf));
	printf("\ndrive   write       read        hard");
	printf("\nnumber  operations  operations  errors\n");
	for(j=0; j<64; j++)
		if(mt_dt[j]) {
			printf("\n%6.d  %10.D", j, wrtcnt[j]);
			printf("  %10.D  %6.D", rdcnt[j], hecnt[j]);
			}
	printf("\n");
	fflush(stdout);
}

/*
 * Wait for tape unit ready.
 * The tape is closed, then approximately
 * once per second an atttempt is made to
 * open the tape again. If after 15 minutes
 * the tape can't be opened, a fatal error
 * message is printed and the tape exerciser
 * exits. If the tape can be opened then
 * the file descriptor is returned.
 *
 * The 15 minute wait is necessary to allow
 * for manual intervention to make the tape
 * unit ready after such things as opening the
 * door on a TS11 or taking the drive off-line.
 */

tuready(fd)
{
	register int i;

	close(fd);
	for(i=0; i<900; i++) {
		if((fd = open(&devn, RD)) >= 0)
			return(fd);
		sleep(1);
		}
	fprintf(stderr,"\n\nmtx: FATAL TAPE ERROR - can't open %s", &devn);
	printf(" after 15 minutes\n\n");
	exit(1);
}

/*
 * The "mtopen" function opens the magtape for
 * actual read/write operations.
 * If the open fails, mtopen prints the reason
 * for the failure and the calls "tuready"
 * which retries the open once per second for 15
 * minutes, and prints a fatal error message if
 * the open can't be completed by that time.
 */

mtopen(rw)
{

	int fd;

	while((fd = open(&devn, rw)) < 0) {
		fprintf(stderr,"\n\nmtx: can't open %s ", &devn);
		if(errno == ETOL)
			fprintf(stderr,"[off-line]");
		else if(errno == ETWL)
			fprintf(stderr,"[write locked]");
		else if(errno == ETO)
			fprintf(stderr,"[already open]");
		else
			fprintf(stderr,"[unknown error]");
   		fprintf(stderr,"\n\nmtx: [will retry for at least 15 minutes, then quit !]");
		fd = tuready(fd);
		close(fd);
	}
	return(fd);
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
 * Generate the special filename for the
 * selected magtape unit, depending upon
 * the density and iomode arguments.
 */

mtname(dn, den, mode)
{
	char tp;

	tp = mt_dt[dn];
	if(!den && !mode) {
		if (tp == 1 || tp == 2)
			sprintf(&devn, "%s%d", mtn, dn);
		else if (tp == 4)
			sprintf(&devn, "%s%d", htn, dn);
		else
			sprintf(&devn, "%s%d", tkn, dn);
		}
	if(!den && mode) {
		if (tp == 1 || tp == 2)
			sprintf(&devn, "%s%d", rmtn, dn);
		else if (tp == 4)
			sprintf(&devn, "%s%d", rhtn, dn);
		else
			sprintf(&devn, "%s%d", rtkn, dn);
		}
	if(den && !mode) {
		if (tp == 2 || tp == 3)
			sprintf(&devn, "%s%d", htn, dn);
		else
			sprintf(&devn, "%s%d", gtn, dn);
		}
	if(den && mode) {
		if (tp == 2 || tp == 3)
			sprintf(&devn, "%s%d", rhtn, dn);
		else
			sprintf(&devn, "%s%d", rgtn, dn);
		}
}

/*
 * Print the address of the word in error
 * ( if word not equal to -1)
 * followed by the GOOD/BAD data.
 */

pgb(word, good, bad)
{
	if(word != -1)
		printf("\n\nWORD = %6.d", word);
	printf("\nGOOD = %06.o", good);
	printf("\nBAD  = %06.o", bad);
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
