
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)cmx2.c	3.0	4/22/86";
/*
 *	Ultrix-11 communications exerciser (cmx).
 *
 *	DH11/DHU11/DHV11/DZ11/DZV11/DZQ11/DL11 
 *
 * PART 2 - (cmx2.c)
 *
 *	Part 2 is the actual exerciser
 *	Called from cmx, data passed in cmx_?#.arg file,
 *	? = dh, dhu, dhv, dz, dzv, dzq, dl # = unit number.
 *
 * Fred Canter 11/2/82
 * Bill Burns 4/84
 *
 * This program exercises one DH11, DHU11/DHV11, DZ11/DZV11/DZQ11,
 * or DL11 unit, (a DL11 unit is actually 16 DL11's)
 * either in maintenance loop back mode or with line
 * turnaroud connectors on the mux panel.
 *
 * Maintenance loopback mode:
 *
 * 1.	Maintenance loopback mode for DH11 and DZ11/DZV11/DZQ11
 *	automatically loops back all lines. This means that all 
 *	terminal lines on DH11 and DZ11/DZV11/DZQ11 devices must be 
 *	disabled in the `/etc/ttys' file.
 *
 * 2.	 The DHU11/DHV11 controllers allow loopback on individual lines.
 *	 When testing DHU11/DHV11 devices only the lines that are
 *	 being tested need to be disabled in the `/etc/ttys' file.
 *
 * Turnaround connectors:
 *
 *	 With turnaround connectors, only the line(s) to be
 *	 exercised must be disabled in `/etc/ttys'.
 *
 ****************************************************************
 *	WARNING WARNING WARNING WARNING WARNING WARNING		*
 *								*
 *	In maintenance loop back mode, character output on	*
 *	the transmit lines in NOT disabled !			*
 *	Prior to running this exerciser the customers		*
 *	equipment, other than terminals, must be disconnected	*
 *	from the lines to be exercised or disabled, if it	*
 *	would be affected by the test data on the lines.	*
 *	Also (cmx) will not function properly unless the	*
 *	currently running ULTRIX-11 is named `unix'.		*
 *								*
 ****************************************************************
 *
 * USAGE:
 *
 *	cmxr ?# #
 *
 *		?	Device type, dh, dhu, dhv, dz, dzv, dzq, dl
 *		#	Device unit number
 *			(for DL, unit 0 is dl 0 - 15, & unit 1 is dl 16 - 31)
 *		#	Event flag number
 *
 *		Below is gone due to event flags usage:
 *		kfn	Run/stop control file name
 *
 */

#include <stdio.h>
#include <sgtty.h>
#include <signal.h>
#include <time.h>
#include <a.out.h>
#include <sys/param.h>	/* Don't matter which one */

#define	R	0
#define	W	1
#define	RW	2
#define	OFF	0

/* tstate flags */
#define	SETUP	1
#define	WRDY	2
#define	RRDY	4
#define	TRDY	8

char	afn[12];	/* argument file name */
char	*dname;		/* device name DH11, DHU11/DHV11, 
				DZ11/DZV11/DZQ11, or DL11 */
char	*dn;		/* DH/DHU/DHV/DZ/DZV/DZQ/DL device name */
char	tdn[12];	/* tty node name `/dev/tty??' */
char	*obuf;		/* pointer to tty line output buffers */
char	*ibuf;		/* pointer to tty line input buffers */
char	tfc[16];	/* first character of test data pattern */
char	tstate[16];	/* state of each line, OFF, WRT, READ, TEST */
int	tnc[16];	/* # of characters sent on a line, 132 max */
int	twc[16];	/* write character count for a line */
int	tcc[16];	/* received character count for each line */
int	tfd[16];	/* file descriptor for each line */
int	toc[16];	/* timeout count on each line */
int	tdec[16];	/* data error count on each line */
time_t	tstime[16];	/* time output started on a line, for timeout */

		/* stuff for end of pass and stats */
unsigned 	wrtcnt[16];	/* count of write operations per line */
unsigned 	rdcnt[16];	/* count of "good" read operations per line */
unsigned	errcnt[16];	/* cout of read errors per line */
				/* NOTE: end of pass not implemented */
unsigned	passcnt[16];	/* end of pass count for each line */


int	unit;		/* DH11/DHU11/DHV11/DZ11/DZV11/DZQ11/DL11 unit number */
int	nline;		/* number of lines per unit */

int	brate[] =	/* bit rates (sgtty) */
{
	3,		/*  110 baud */
	7,		/*  300 baud */
	9,		/* 1200 baud */
	11,		/* 2400 baud */
	12,		/* 4800 baud */
	13,		/* 9600 baud */
	13,
	13
};

/*
 * Line activity table,
 * -1 = line not selected
 *  0 = line deselected via -u
 *  1 = line selected via -l (without bit rate)
 * >1 = line selected via -l (with bit rate)
 * for active lines, set to bit rate (sgtty) number.
 */

int	lnact[] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
int	bdrsav[16];	/* baud rate save */
struct	sgttyb	tsgtty = { 0, 0, '#', '@', (RAW+ANYP) };
struct	sgttyb	t_etq;	/* used by ETQ ioctl to get char count */

int	dzflag;
int	dzvflag;
int	dzqflag;
int	dhuflag;
int	dhvflag;
int	dlflag;
int	nmflag, brflag, lsflag;
int	ndep;
int	ndel;
long	randx;
time_t	btbuf;
time_t	etbuf;
time_t	timbuf;
time_t	ptbuf;	/* time buf for stat printouts */

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
int	errno;
int	sys_nerr;
char	*sys_errlist[];
int	istime;

main(argc, argv)
char *argv[];
int argc;
{
	int	stop(), intr();
	FILE	*argf;
	register int i, j;
	register char *p;
	char	*n;
	int	*ap;
	int	fi, ln, opnerr;
	int	first, cc, sum;
	int	dcerr, cserr;
	int	wcc, dec, fde;
	int	burst, maxcc, bufsiz;
	int	maj;
	int	sflag;
	int	oldrate;

	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, intr);
	signal(SIGQUIT, stop);
	if((argc < 2) || (argc > 4))
		exit(1);
	dn = argv[1];	/* device name, dz#, dh# or uh# */
	sprintf(&afn, "cmx_%s.arg", dn);
	argf = fopen(afn, "r");
	if(argf == NULL)
		exit(1);
	maj = getw(argf);
	unit = getw(argf);
	dzflag = getw(argf);
	dzvflag = getw(argf);
	dzqflag = getw(argf);
	dhuflag = getw(argf);
	dhvflag = getw(argf);
	dlflag = getw(argf);
	sflag = getw(argf);
	istime = getw(argf);
	ndep = getw(argf);
	ndel = getw(argf);
	bufsiz = getw(argf);
	burst = getw(argf);
	maxcc = getw(argf);
	brflag = getw(argf);
	nmflag = getw(argf);
#ifdef EFLG
	zflag = getw(argf);
#endif
	for(i=0; i<16; i++)
		lnact[i] = getw(argf);
	fclose(argf);
	unlink(afn);
#ifdef EFLG
	if(zflag) {
		efpis = argv[2];
		efids = argv[3];
	}
#else
	killfn = argv[2];	/* run/stop control file name */
#endif
	if(dzflag) {
		dname = "DZ11";
		nline = 8;
	} else if(dzvflag) {
		dname = "DZV11";
		nline = 4;
	} else if(dzqflag) {
		dname = "DZQ11";
		nline = 4;
	} else if(dlflag) {
		dname = "DL11";
		nline = 16;
	} else if(dhvflag) {
		dname = "DHV11";
		nline = 8;
	} else if(dhuflag) {
		dname = "DHU11";
		nline = 16;
	} else {
		dname = "DH11";
		nline = 16;
	}
/*
 * Allocate memory for 4 input & output
 * buffers if DZV11 or DZQ11,
 * 8 if DHV11 or DZ11, or 16 if DH11, DHU11, or DL11.
 */
	ibuf = calloc((bufsiz*nline), sizeof(char));
	if(ibuf == NULL) {
		fprintf(stderr, "\ncmx: Can't calloc input buffers\n");
		exit(1);
	}
	obuf = calloc((bufsiz*nline), sizeof(char));
	if(obuf == NULL) {
		fprintf(stderr, "\ncmx: Can't calloc output buffers\n");
		exit(1);
	}

	time(&btbuf);
	ptbuf = btbuf;
	randx = btbuf & 0777;	/* initialize random number generator */
	printf("\n\n%s exerciser started - %s",dname, ctime(&btbuf));
	fflush(stdout);
#ifdef EFLG
	if(zflag) {
		efbit = atoi(efpis);
		efid = atoi(efids);
		evntflg(EFCLR, efid, (long)efbit);
	}
#else
	unlink(killfn);		/* tell SYSX cmx started */
#endif
/*
 * Open all selected lines and set their
 * state to write ready.
 */
	for(i=0; i<nline; i++) {
		toc[i] = 0;	/* zero timeout counter */
		if(lnact[i] < 0) {	/* line not selected */
			tfd[i] = -1;
			tstate[i] = OFF;
		} else {
			sprintf(&tdn, "/dev/%s%02d", dn, i);
			ttlocl((maj|(i+(unit*nline))),1); /* set local mode */
			tfd[i] = open(tdn, RW);
			if(tfd[i] < 0) {
				perror("cmx");
				fprintf(stderr, "\ncmx: Can't open %s\n", tdn);
				exit(1);
			}
			tstate[i] = SETUP;
			twc[i] = 0;
		}
	}
/*
 * Write, Read, compare data loop as follows:
 *
 * 1.	Do a stty() to set RAW mode and bit rate on the tty line.
 *	The bit rate is randomly set to one of the following:
 *	110, 300, 1200, 2400, 4800, 9600,
 *	or is fixed at the rate specified by [-b] or [-l].
 *	DL11 must use fixed bit rate, checked by cmx1.
 *
 * 2.	On DH11, and DZ11/DZV11/DZQ11 set the controller
 *	to maintenance loop back mode, unless [-m] was specified.
 *	On DHU11/DHV11 and DL11 set maintenance loopback
 *	on individual lines, unless [-m] was specified.
 *
 * 3.	Generate a test data packet for the tty line
 *	and load it into the output buffer.
 *	The data packet contains a sequential binary count
 *	pattern starting with a random character and is a
 *	random number of characters in length.
 *	Save the first character of the test data pattern
 *	for use by the data compare test.
 *	Write the output buffer to the tty line.
 *
 *	CHAR	CONTENTS
 *
 *	0	DH/DHU/DHV/DZ/DZV/DZQ/DL unit number
 *	1	DH/DHU/DHV/DZ/DZV/DZQ/DL line number
 *	2 - N	test data pattern
 *		( N = random number of sequential characters)
 *	N	16 bit checksum over entire packet
 *
 * 4.	Scan all selected lines for input characters pending.
 *	For any that have some, read all waiting characters
 *	into the input buffer for that line.
 *	When a line has read an entire packet,
 *	do the data compare test and print any errors.
 *	If the entire packet has not been received within
 *	2 minutes after the output was started on the line
 *	a timeout error message is printed.
 *
 * 5.	After the data has been compared, mark the line
 *	ready for output and go back to step 1.
 */
	first = 0;
loop:
/*	intr();	/* poll for stop cmx (signals unreliable) */
	if(sflag)
		goto nostat;
	time(&timbuf);
	if(((timbuf - ptbuf) / 60) < istime)
		goto nostat;
	else {
		ptbuf = timbuf;
		printf("\n\n CMX I/O Statistics at %s \n\n", ctime(&ptbuf));
		printf("line\t\t write\t\t  read\t\t error\n");
		printf("\t\t count\t\t count\t\t count\n");
		for(i=0; i<16; i++)
			if(tstate[i] != OFF)
				printf("%2.d\t\t%6.u\t\t%6.u\t\t%6.u\n", i, wrtcnt[i], rdcnt[i], errcnt[i]);
		
	}
nostat:
	for(i=0; i<nline; i++) {
		if(tstate[i] & SETUP) {
			ioctl(tfd[i], TIOCFLUSH, &tsgtty);
			if(tdec[i] >= ndel) {	/* data error limit exceeded */
				printf("\n\n****** DATA ERROR LIMIT EXCEEDED ******\n");
				printf("\n     %s unit %d line %02d deselected\n", dname, unit, i);
				tstate[i] = OFF;
				continue;
			}
			if(brflag) {
				tsgtty.sg_ispeed = brflag;
				tsgtty.sg_ospeed = brflag;
			} else if(lnact[i] > 1) {
				tsgtty.sg_ispeed = lnact[i];
				tsgtty.sg_ospeed = lnact[i];
			} else {
				tsgtty.sg_ispeed = brate[rng()&7];
				tsgtty.sg_ospeed = tsgtty.sg_ispeed;
			}
			oldrate = bdrsav[i];		/* note old rate for dzv11 */
			bdrsav[i] = tsgtty.sg_ispeed;	/* save bit rate */
			stty(tfd[i], &tsgtty);
			if((dlflag || dhuflag || dhvflag) && !nmflag) {
				ioctl(tfd[i], TIOCSMLB, &tsgtty);
			} else if(!first && !nmflag) {
				first++;
				ioctl(tfd[i], TIOCSMLB, &tsgtty);
			}
			tnc[i] = (rng() & maxcc) + 1;
			p = obuf + (i*bufsiz);
			sum = 0;
			tfc[i] = rng() & 0377;
			*p = unit;
			sum += *p++;
			*p = i;
			sum += *p++;
			for(j=0; j<tnc[i]; j++) {
				*p = (tfc[i] + j) & 0377;
				sum += *p++;
			}
			sum = -sum;
			*p++ = sum & 0377;
			*p++ = (sum >> 8) & 0377;
			tnc[i] += 4;
			wrtcnt[i]++;
		/* 110 baud special case for dzv11 */
			if((oldrate == 3) && (dzvflag || dzqflag))
				sleep(1);

			ioctl(tfd[i], TIOCFLUSH, &tsgtty);
			tstate[i] = WRDY;
			continue;
		}
		if((tstate[i] & WRDY) && (twc[i] < tnc[i])) {
			if((tnc[i] - twc[i]) >= burst)
				wcc = burst;
			else
				wcc = tnc[i] - twc[i];
			time(&tstime[i]);
			p = obuf + (i*bufsiz);
			p += twc[i];
			if(write(tfd[i], (char *)p, wcc) != wcc) {
	/* In case QUIT signal received during system call */
	/* NOTE - access will mess up errno! */
#ifdef EFLG
				if(zflag) {
					if(stopsig || checkflg())
						stop();
				} else {
					if(stopsig)
						stop();
				}
#else
				if(stopsig || (access(killfn, 0) == 0))
					stop();
#endif
				printf("\n\n****** WRITE ERROR on %s unit %d line %02d ******\n",dname,unit,i);
				perror("cmxr");
				tdec[i]++;
				tstate[i] = SETUP;
				twc[i] = 0;
				continue;
			}
			twc[i] += wcc;	
			/* first write only */
			if(twc[i] && (*(p+2) == tfc[i]) &&
			    (*(p+1) == i) && (*p == unit)) {
				tstate[i] |= RRDY;
				tcc[i] = 0;
			}
		}
	}
/*	intr()	*/
	for(i=0; i<nline; i++)
		if(tstate[i] & RRDY) {
			t_etq.sg_flags = 0;
			ioctl(tfd[i], TIOCETQ, &t_etq);
			if(cc = t_etq.sg_flags) {	/* char count */
				if(tcc[i] == 0) {	/* first read only */
					p = ibuf + (i*bufsiz);
					for(j=0; j<bufsiz; j++)
						*p++ = 0;
				}
				for(j=0; j<cc; j++)
				if(read(tfd[i], (char *)(ibuf+(i*bufsiz)+tcc[i]+j),1) != 1) {
#ifdef EFLG
					if(zflag) {
						if(stopsig || checkflg())
							stop();
					} else {
						if(stopsig)
							stop();
					}
#else
					if(stopsig || (access(killfn, 0) == 0))
						stop();
#endif
					printf("\n\n****** READ ERROR on %s unit %d line %02d ******\n",dname,unit,i);
					tstate[i] = TRDY;
					goto nxtrd;
				}
				tcc[i] += cc;
				if(tcc[i] == tnc[i]) {
					tstate[i] = TRDY;
					continue;
				}
			}
			time(&timbuf);
			if((timbuf - tstime[i]) > 120) {
				printf("\n\nRead data timeout on ");
				printf("%s unit %d line %02d", dname, unit, i);
				if(tcc[i])
					printf("\nComplete data packet not received within 2 minutes");
				else
					printf("\nNo data packet received within 2 minutes");
				toc[i]++;
				if(toc[i] >= 5) {
					printf("\n\n****** TIMEOUT LIMIT EXCEEDED ******");
					printf("\n   %s unit %d line %02d deselected\n",dname, unit, i);
					ioctl(tfd[i], TIOCFLUSH, &tsgtty);
					tstate[i] = OFF;
				} else {
					if(tcc[i] > 0) {
						tstate[i] = TRDY;
						continue;
					}
					tstate[i] = SETUP;
					twc[i] = 0;
				}
			}
		nxtrd:
			continue;
		}
/*	intr();	*/
	for(i=0; i<nline; i++)
		if(tstate[i] == TRDY) {
			cserr = 0;
			dcerr = 0;
			p = ibuf + (i*bufsiz);	/* checksum */
			for(sum=0, j=0; j<(tcc[i] - 2); j++)
				sum += *p++;
			if((sum == 0) && (tcc[i] != tnc[i]))
				cserr++;	/* all zeroes in ibuf */
			if((sum += ((*p++ & 0377) | (*p++ << 8))) != 0)
				cserr++;
			if(tcc[i] != tnc[i])
				dcerr++;
			p = ibuf + (i*bufsiz);
			n = obuf + (i*bufsiz);
			for(j=0; j<(tnc[i] - 2); j++)	/* packet data */
				if((*p++ & 0377) != (*n++ & 0377)) {
					dcerr++;
					break;
				}
			if(!cserr && !dcerr) {
				tstate[i] = SETUP;
				twc[i] = 0;
				rdcnt[i]++;
				continue;
			}
			/* Print data error message */

			errcnt[i]++;
			fde = 0;
			tdec[i]++;
			p = ibuf + (i*bufsiz);
			n = obuf + (i*bufsiz);
			time(&timbuf);
			printf("\n\n************ READ DATA ERROR *************\n");
			printf("\t%s", ctime(&timbuf));
			printf("\nData transmitted on %s unit %3d line %3d", dname, *n, *(n+1));
			if(tcc[i])	/* only print if some data received */
				printf("\nData received    on %s unit %3d line %3d", dname, *p, *(p+1));
			switch(bdrsav[i]) {
			case 3:
				j = 110;
				break;
			case 7:
				j = 300;
				break;
			case 9:
				j = 1200;
				break;
			case 11:
				j = 2400;
				break;
			case 12:
				j = 4800;
				break;
			case 13:
				j = 9600;
				break;
			default:
				j = 0;
				break;
			}
			printf("\nData transmitted at %d bits/second", j);
			printf("\nData packet checksum was ");
			if(cserr)
				printf("BAD");
			else
				printf("GOOD");
			printf("\n%03d characters transmitted", tnc[i]);
			printf("\n%03d characters received", tcc[i]);
			p = ibuf + (i*bufsiz);
			n = obuf + (i*bufsiz);
			dec = 0;
			for(j=0; j<(tnc[i] - 2); j++) {
				if((*p & 0377) != (*n & 0377)) {
					if(dec++ >= ndep) {
						printf("\n\n[ data error print limit exceeded ]");
						break;
					}
					if(fde==0) {
						fde++;
						printf("\n\nCHAR #\tGOOD\tBAD\n");
					}
					printf("\n%6d\t%03o\t%03o", j, (*n & 0377), (*p & 0377));
				}
				p++;
				n++;
			}
			printf("\n\n******************************************\n");
			tstate[i] = SETUP;
			twc[i] = 0;
		}
		fflush(stdout);
		goto loop;
}
/*
 * Random number generator
 */

rng()
{
	return(((randx = randx * 1103515245 + 12345) >> 16) & 077777);
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
	register int i;

	stopsig++;
	signal(SIGTERM, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	printf("\n\n CMX Final I/O Statistics \n\n");
	printf("line\t\t write\t\t  read\t\t error\n");
	printf("\t\t count\t\t count\t\t count\n");
	for(i=0; i<16; i++)
		if(tstate[i] != OFF) {
			ioctl(tfd[i], TIOCFLUSH, &tsgtty);
			ioctl(tfd[i], TIOCCMLB, &tsgtty); /* clear loop back */
			ttlocl((i+(unit*nline)), 0); /* clear line local mode */
			printf("%2.d\t\t%6.u\t\t%6.u\t\t%6.u\n", i, wrtcnt[i], rdcnt[i], errcnt[i]);
			close(tfd[i]);
			sprintf(&tdn, "/dev/%s%02d", dn, i);
			unlink(tdn);
		}
	time(&etbuf);
	if(dzflag)
		printf("\n\nDZ11");
	else if(dzvflag)
		printf("\n\nDZV11");
	else if(dzqflag)
		printf("\n\nDZQ11");
	else if(dlflag)
		printf("\n\nDL11");
	else if(dhuflag)
		printf("\n\nDHU11");
	else if(dhvflag)
		printf("\n\nDHV11");
	else
		printf("\n\nDH11");
	printf(" exerciser stopped - %s\n", ctime(&etbuf));
	fflush(stdout);
#ifdef EFLG
	if(zflag)
		evntflg(EFCLR, efid, (long)efbit);
#else
	unlink(killfn);	/* tell SYSX cmx stopped */
#endif
	exit(0);
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
