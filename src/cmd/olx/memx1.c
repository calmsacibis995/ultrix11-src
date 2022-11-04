
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)memx1.c	3.0	4/22/86";
/*
 * ULTRIX-11 memory exerciser program (memx).
 * PART 1
 *
 *	Part 1 does the initial setup and then calls
 *	the memory exerciser (memxr).
 *	A memory buffer area size is passed to memxr.
 *	The number of copies of memxr to be run is based
 *	on the type of CPU and the amount of free memory.
 *
 * Fred Canter 11/4/82
 * Bill Burns  4/23/84
 *	added event flags
 *
 *	********************************************
 *	*                                          *
 *	* This program will not function correctly *
 *	* unless the current unix monitor file is  *
 *	* named "unix".                            *
 *	*					   *
 *	********************************************
 *
 * USAGE:
 *
 *	memx -z # #
 *
 *		-z # #	event flag bit position and identifier
 *
 */

#include <sys/param.h>	/* Does not matter which one ! */
#include <stdio.h>
#include <a.out.h>
#include <signal.h>
#include <setjmp.h>

#define	SAM	1	/* standalone mode (no SYSX script) */
#define	SCRIPT	2	/* running under SYSX as part of script */
#define	MEMXTO	15	/* MEMX timeout in minutes */

struct nlist nl[] =
{
	{ "_cputype" },
	{ "_usermem" },
	{ "_nswap" },
	{ "" },
};

char	msz[12];
char	fcopy[10];

int	mx2pid[150];	/* should never be more than 150 processes */

time_t timbuf;
int	mpid;	/* PID of master copy of memx1 */

#ifdef EFLG
#include <sys/eflg.h>
char	*efpis;
char	*efids;
int	efbit;
int	efid;
long	evntflg();
int	zflag;
#else
char	*killfn = "memx.kill";
#endif

int	stopsig;
int	qflag;
int	slflag;		/* swap limit flag */

int	almcnt;
int	almsig;
jmp_buf	savej;

main(argc, argv)
char	*argv[];
int	argc;
{
/*
 * NO register variables are used because they get
 * modified by setjmp, longjmp, and alarm.
 */
	int	intr1(), stop1();
	int	xrmon();
	int	intr(), stop();
	int	i, j;
	FILE	*fi, *argf;
	int	cputype;
	unsigned int usermem;	/* amount of memory after unix */
	int	nswap;
	unsigned int memsiz;
	unsigned int oddms;
	int	nmemx;
	int	mem;
	int	count, cnt;
	int	mxstat;
	int	mxs;
	int	mxpid;
	int	ncbx;	/* # of compute bound exercisers in script */
	int	ncix;	/* # of char I/O intensive exercisers in script */
	int	nbix;	/* # of block I/O intensive exercisers in script */
	int	opmode;

	signal(SIGTTOU, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, stop);
	signal(SIGTERM, intr);
#ifdef EFLG
	if(argc == 4) {
		zflag++;
		efpis = argv[2];
		efids = argv[3];
	}
	if(!zflag) {
	    if(isatty(2)) {
	       fprintf(stderr, "memx: detaching... type \"sysxstop\" to stop\n");
	       fflush(stdout);
	    }
	    if((i = fork()) == -1) {
		    printf("memx: Can't fork new copy !\n");
		    exit(1);
	    }
	    if(i != 0)
		    exit(0);
	}
	setpgrp(0, 31111);
#else
	if(argc == 3)
		killfn = argv[2];
#endif
	mpid = getpid();
	close(stdin);
	nlist("/unix", nl);
	for(i=0; i<3; i++)
		if(nl[i].n_type == 0) {
			fprintf(stderr,"\nmemx: Can't access /unix namelist\n");
			exit(1);
		}
	if((mem = open("/dev/mem", 0)) < 0) {
		fprintf(stderr, "\nmemx: Can't open /dev/mem\n");
		exit(1);
	}
	lseek(mem, (long)nl[0].n_value, 0);
	read(mem, (char *)&cputype, sizeof(cputype));
	lseek(mem, (long)nl[1].n_value, 0);
	read(mem, (char *)&usermem, sizeof(usermem));
	lseek(mem, (long)nl[2].n_value, 0);
	read(mem, (char *)&nswap, sizeof(nswap));
	close(mem);
	time(&timbuf);
	printf("\n\nMemory exerciser started - %s", ctime(&timbuf));
	fflush(stdout);
#ifdef EFLG
	if(zflag) {
		efbit = atoi(efpis);
		efid = atoi(efids);
		evntflg(EFCLR, efid, (long)efbit);
	}
#else
	unlink(killfn);		/* tell SYSX memx started */
#endif
/*
 * Determine operating mode
 */
	if(argc != 4)
		opmode = SAM;	/* no script */
	else {	/* script, see how many and what type exers */
		fi = fopen("sysxr.nx", "r");
		if(fi == NULL) {
			fprintf(stderr, "\nmemx: Can't open sysxr.nx !\n");
			exit(1);
		}
		ncbx = getw(fi);
		ncix = getw(fi);
		nbix = getw(fi);
		fclose(fi);
		if(ncbx+ncix+nbix > 1)
			opmode = SCRIPT;
		else
			opmode = SAM;
	}
	nmemx = 2;
	while((memsiz = usermem/nmemx) > 768)
		nmemx++;
/*
 * Adjust number of memx subprocesses according to
 * system load, this makes an effort to regulate swapping.
 */
	if(opmode == SAM)
		nmemx += 3;
	/* else {		patch for swap problem on Micro/11 (FIXED)
		if(nbix)
			nmemx += 1;
		else if(ncix)
			nmemx += 2;
		else if(ncbx <= 4)
			nmemx += 3;
	} */
/*
 * Limit swap activity under certain cercumstances,
 * fix for panic: out of swap space on micro/pdp-11.
 */
	if((nswap < 5000) &&
	   (opmode == SCRIPT) &&
	   ((ncbx+ncix+nbix) > 3))
		slflag = 1;
	else
		slflag = 0;
	cnt = 0;
	count = 0;
xloop:
	for(j=0; j<nmemx; j++)
		mx2pid[j] = 0;
	signal(SIGTERM, intr1);	/* protect during fork/exec */
	signal(SIGQUIT, stop1);
	for(j=0; j<nmemx; j++) {
/*
 * For the last memxr, the memory buffer size
 * is a multiple of 512 bytes up to and
 * including the maximum buffer size (memsiz).
 */
		if(slflag)
			sprintf(&msz, "%u", 768 - (128 * (j%6)));
		else if(j == (nmemx - 1)) {
			cnt++;
			oddms = cnt * 8;
			if(oddms > memsiz) {
				cnt = 0;
				oddms = memsiz;
			}
			sprintf(&msz, "%u", oddms);
		} else
			sprintf(&msz, "%u", memsiz);
		if((i = fork()) == 0) {
			sprintf(&fcopy, "%d", j);	/* copy number */
#ifdef EFLG
			if(zflag)
			  execl("memxr", "memxr", msz, efpis, efids, fcopy, (char *)0);
			else
			  execl("memxr", "memxr", msz, fcopy, (char *)0);
#else
			execl("memxr", "memxr", msz, killfn, fcopy, (char *)0);
#endif
			exit();
		}
		if(i == -1) {
			fprintf(stderr,"\nmemx: Can't create memxr # %d\n", j);
			exit(1);
		}
		mx2pid[j] = i;
		sleep(5);	/* slow down process creation rate */
	}
	if(qflag) {
		for(j=0; j<nmemx; j++)
			if(mx2pid[j])
				kill(mx2pid[j], SIGKILL);
		stop();
	}
	signal(SIGTERM, intr);
	signal(SIGQUIT, stop);
	almsig = 0;
	signal(SIGALRM, xrmon);
	alarm(60);
	for(j=0; j<nmemx; j++) {
		almcnt = 0;
		if(mx2pid[j] == 0)
			continue;	/* in case memx killed */
		if(j != 0)	/* first copy starts itself */
			kill(mx2pid[j], SIGPIPE);	/* start memx2 */
/*
 * Use alarm signal to detect memx
 * subprocess timeouts.
 */
		setjmp(savej);
		if(almsig) {
		      almsig = 0;
			if(++almcnt < MEMXTO) {
/*				printf("\ndebug %d %d", mx2pid[j], j);	*/
				if(mx2pid[j])
					kill(mx2pid[j], SIGPIPE);
				goto wloop;
			}
		      printf("\nmemx: memxr (pid %d) timed out !\n",mx2pid[j]);
		      kill(mx2pid[j], SIGKILL);
		      fflush(stdout);
		      almcnt = 0;
		      continue;
		}
wloop:
		while((mxpid = wait(&mxstat)) == -1) ;
		if(stopsig)
			stop();
		mxs = mxstat & 0377;
		if((mxs != 0) && (mxs != 3) && (mxs != 9)) {
			time(&timbuf);
		printf("\nMemory exerciser (pid %d status %d) ", mxpid, mxstat);
		printf("terminated abnormally !");
	printf("\nCheck error log for error at about - %s\n", ctime(&timbuf));
		}
	}
	alarm(0);
	if(((opmode == SAM) && ((count & 7) == 7))
	  || ((opmode == SCRIPT) && (count & 1))) {	/* End of pass */
		time(&timbuf);
		printf("\nMemory exerciser end of pass - %s", ctime(&timbuf));
		fflush(stdout);
	}
	count++;
	if(opmode == SCRIPT)
		sleep(60);	/* delay to allow for process table cleanup */
	else
		sleep(30);
	goto xloop;
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
	signal(SIGTERM, SIG_IGN);	/* so elp will ignore SIGTERM ! */
	signal(SIGQUIT, SIG_IGN);
	alarm(0);
	if(getpid() == mpid) {
		time(&timbuf);
		printf("\n\nMemory exerciser stopped - %s\n", ctime(&timbuf));
		fflush(stdout);
		if(fork() == 0)
			execl("/bin/elp", "elp", "-s", "-mp", (char *)0);
		else
			while(wait() != -1) ;
#ifdef EFLG
		if(zflag)
			evntflg(EFCLR, efid, (long)efbit);
#else
		unlink(killfn);
#endif
	}
	exit();
}

xrmon()
{
	almsig++;
	signal(SIGALRM, xrmon);
	alarm(60);
	longjmp(savej, 1);
}

intr1()
{
	signal(SIGTERM, intr1);
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
	qflag++;
}

stop1()
{
	signal(SIGQUIT, stop1);
	qflag++;
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
