
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)lpx.c	3.0	4/22/86";
/*
 * ULTRIX-11 line printer exerciser program (lpx).
 * Fred Canter 10/31/82
 * Bill Burns 4/84
 *	added event flag code
 *
 ************************************************
 *                                              *
 * The line printer driver must be set up for   *
 * NO indent, so that all columns will be used. *
 *                                              *
 ************************************************
 *
 * This program exercises the lp11 line printer
 * and interface via the unix character
 * I/O mechanism. It prints test patterns that
 * must be visually verified by user.
 * If an error occurs a message is printed and the
 * print operation is retried at one minute intervals.
 * In order to save paper the program prints 12 pages
 * (11 pages if 80 column) and then "pauses" for a time,
 * specified by -p option. Durring the pause, non printing
 * characters are sent to the printer to simulate
 * constant use of the line printer and controller.
 * This program is a system level exerciser, it is NOT
 * intended to be a device diagnostic.
 *
 * USAGE:
 *
 *	lpx	-h	Print the help message.
 *
 *	lpx	-p#	Specify the paper saving "pause"
 *			time as (#) minutes.
 *			The default pause time is 15 minutes.
 *
 *	lpx	-p0	Specify continuous printing (no pause).
 *
 *	lpx	-p	Specify continuous pause (no printing).
 *
 */

#include <sys/param.h>	/* Don't matter which one */
#include <stdio.h>
#include <signal.h>

/*
 * Help message text
 */

char	*help[] =
{
	"\n\n\nUSAGE:",
	"\n\n\tlpx [-h] [-p#]",
	"\n\n\t-h\tPrint the help message.",
	"\n\n\t-p#\tSpecify the paper saving \"pause\"",
	"\n\t\tas (#) minutes, default is 15 minutes.",
	"\n\t\tSet pause time to zero for continuous printing.",
	"\n\t\tUse -p without (#) for continuous pause (no print).",
	"\n\n\n\n",
	0
};

time_t	timbuf;
time_t	stbuf;
char	lptpat;
char	lptsv;
char	lpform = 014;
char	lpbuf[134];
int	ptime = 15;
char	*lock = "/usr/spool/lpd/lock";

#ifdef EFLG
#include <sys/eflg.h>
char	*efpis;
char	*efids;
int	efbit;
int	efid;
long	evntflg();
int	zflag;
#else
char	*killfn = "lpx.kill";	/* name of kill file */
#endif

#define	NPASS	66 * 100	/* pass count - 100 pages @ 66 lines each */

int	lc;			/* line count */
int	termsig;
int	stopsig;

main(argc, argv)
char *argv[];
int argc;
{
	int	stop(), intr();
	FILE	*argf;
	char	*p;
	int	i, j;
	int	first, fd;

	signal(SIGTTOU, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, intr);
	signal(SIGQUIT, stop);
	for(i=1; i<argc; i++) {	/* decode option arg's */
		p = argv[i];
		if(*p++ != '-') {
		argerr:
			fprintf(stderr,"\nlpx: bad arg\n");
			exit(1);
			}
		switch(*p) {
		case 'h':	/* print help message */
			for(j=0; help[j]; j++)
				printf("%s", help[j]);
			exit(0);
		case 'p':	/* pause time */
			p++;
			if(*p == 0) {
				ptime = -1;
				break;
			}
			ptime = atoi(p);
			ptime &= 077777; /* make it positive */
			if(ptime > 480)	/* 8 hours max */
				ptime = 480;
			break;
#ifdef EFLG
		case 'z':
			zflag++;
			i++;
			efpis = argv[i++];
			efids = argv[i];
			break;
#else
		case 'r':	/* kill file name */
			i++;
			killfn = argv[i];
			break;
#endif
		default:
			goto argerr;
		}
	}
	if(!zflag) {
		if(isatty(2)) {
		    fprintf(stderr, "lpx: detaching... type \"sysxstop\" to stop\n");
		    fflush(stderr);
		}
		if((i = fork()) == -1) {
			printf("lpx: Can't fork new copy !\n");
			exit(1);
		}
		if(i != 0)
			exit(0);
	}
	setpgrp(0, 31111);
/*
 * TEST DESCRIPTION
 *
 *  1.	Open the line printer (/dev/lp) and,
 *	do a form feed on the first open only.
 *
 *  2.	Print 5 pages of alternating ones and zeroes
 *	(U*U*) pattern. (4 pages if 80 column)
 *
 *  3.	Do a form feed.
 *
 *  4.	Print 3/4 page of the test pattern, then a form feed.
 *
 *  5.	Print 1/2 page of the test pattern, then a form feed.
 *
 *  6.	Print 1/4 page of the test pattern, then a form feed.
 *
 *  7.	Print 3 full pages of the test pattern.
 *
 *  8.	Print 5 lines short of a full page
 *	of the test pattern. The form feed on close
 *	should align the paper to the top of the next page.
 *
 *  9.	"pause", print non printing characters, for the
 *	specified time.
 *
 * 10	Close the line printer, should cause a form feed.
 *
 * 11.	Repeat steps 1 thru 10, until the cows come home.
 *
 */

	if(access(lock, 0) == 0) {
		fprintf(stderr, "\07\07\07");
		fprintf(stderr, "\nlpx: Printer in use by spooler,");
		fprintf(stderr, "\n     waiting for print job to finish !\n");
	}
	while(access(lock, 0) == 0)
		sleep(1);
	time(&timbuf);
	printf("\n\nLine printer exerciser started - %s", ctime(&timbuf));
	fflush(stdout);
#ifdef EFLG
	if(zflag) {
		efbit = atoi(efpis);
		efid = atoi(efids);
		evntflg(EFCLR, efid, (long)efbit);
	}
#else
	unlink(killfn);		/* tell SYSX lpx started */
#endif
lploop:
	fd = open("/dev/lp", 1);
	if(fd < 0) {
		fprintf(stderr, "\07\07\07");
		fprintf(stderr,"\nlpx: Can't open LP, check for LP off-line.");
		fprintf(stderr,"\n     Will retry at one minute intervals.\n");
		}
	while(fd < 0) {
		sleep(60);
		fd = open("/dev/lp", 1);
		}
	if(ptime == -1)
		goto noprnt;
	if(first == 0) {
/*
 * A dummy line must be printed in order to
 * make the first and only the first form feed
 * work. Not sure why !!!
 */
		lpbuf[0] = 040;
		lpbuf[1] = '\n';
		lpwrt(fd, 0);
		lpwrt(fd, 1);	/* form fead */
		first++;
		}
	for(i=1; i<133; i++) {
		p = &lpbuf;
		for(j=0; j<132; j++) {
			if(j < (132-i))
				*p++ = ' ';
			else {
				if(j & 1)
					*p++ = 'U';
				else
					*p++ = '*';
				}
			}
		*p++ = '\n';
		lpwrt(fd, 0);	/* print from lp buffer */
		}
	for(i=1; i<133; i++) {
		p = &lpbuf;
		for(j=0; j<132; j++) {
			if(j < (133-i)) {
				if(j & 1)
					*p++ = '*';
				else
					*p++ = 'U';
			} else
				*p++ = ' ';
			}
		*p++ = '\n';
		lpwrt(fd, 0);
		}
	lpwrt(fd, 1);	/* form feed */
	lptp(fd, 45);	/* 45 lines of test pat */
	lpwrt(fd, 1);
	lptp(fd, 30);
	lpwrt(fd, 1);
	lptp(fd, 15);
	lpwrt(fd, 1);
	lptp(fd, 235);
	if(ptime == 0) {	/* continuous printing */
		close(fd);
		if(lc >= NPASS)
			goto lp_eop;	/* end of pass */
		else
			goto lploop;
		}
noprnt:
	time(&stbuf);	/* time of pause start */
	p = &lpbuf;
	for(j=0; j<132; j++)
		*p++ = 0;
/*		*p++= ' ' + (i & 077);	 ascii char set */
/*
 * NULL is the only non printing character that works on all LP
 */
	*p++ = '\r';			/* NO PRINT */
ploop:
	if(lc >= NPASS)
		goto lp_eop;	/* end of pass */
	for(j=0; j<5; j++)
		lpwrt(fd, 0);
	sleep(1);
	time(&timbuf);	/* current time */
	if(ptime == -1) {	/* no print */
		if((timbuf - stbuf) < (15 * 60))
			goto ploop;
		else {
			close(fd);
			goto lploop;
		}
	}
	if((timbuf - stbuf) < (ptime * 60))
		goto ploop;
	close(fd);
	goto lploop;
lp_eop:				/* LPX end of pass */
	lc = 0;
	time(&timbuf);
	printf("\nLP exerciser end of pass - %s", ctime(&timbuf));
	fflush(stdout);
	if((i = fork()) == 0)
	lp_eop1:
		if(ptime == 0)
			goto lploop;
		else
			goto ploop;
	if(i == -1) {
		fprintf(stderr, "\nlpx: Can't fork new copy of lpx !\n");
		goto lp_eop1;	/* continue running this copy */
	}
	exit(0);
}

/*
 * Write to the line printer and  delay
 * and retry on error.
 * flag = 1 for form feed
 * flag = 0 for write from lp buffer.
 */

lpwrt(fd, flag)
{
	int err;

	lc++;		/* increment line count */
retry:
	termsig = 0;
	if(flag) {
		lptpat = 040;
		err = write(fd, &lpform, 1);
	} else
		err = write(fd, &lpbuf, 133);
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
	if(!termsig && (err < 0)) {
		printf("\nlpx: LP error, will retry at one minute intervals");
		fflush(stdout);
		sleep(60);
		goto retry;
			}
}

/*
 * Print the specified number (lc) of lines of
 * the test pattern.
 */

lptp(fd, lc)
{
	int	i,j;
	char	*p;

	for(i=0; i<lc; i++) {
		lptsv = lptpat;
		p = &lpbuf;
		for(j=0; j<132; j++) {
			*p++ = lptpat++;
			if(lptpat >= 0177)
				lptpat = 040;
			}
		*p++ = '\n';
		lpwrt(fd, 0);
		lptpat = lptsv + 1;
		if(lptpat >= 0177)
			lptpat = 040;
		}
}

intr()
{
	termsig++;
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
	if(stopsig++)
		exit(0);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	time(&timbuf);
	printf("\n\nLine printer exerciser stopped - %s", ctime(&timbuf));
	fflush(stdout);
#ifdef EFLG
	if(zflag)
		evntflg(EFCLR, efid, (long)efbit);
#else
	unlink(killfn);
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
