
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)memx2.c	3.0	4/22/86";
/*
 * ULTRIX-11 memory exerciser program (memxr).
 *
 * PART 2 - (memx2.c)
 *
 *	Part 2 is the actual memory exerciser.
 *	Writes data into memory, reads it back and
 *	matches it against the test pattern written.
 *	The patterns are; checkerboard, walking one,
 *	and walking zero. Memory is addressed both as
 *	bytes and words.
 *	Obtains an area of free memory via a call to
 *	malloc(), memory area size is passed to memxr
 *	from memx as an argument.
 *	WARNING !, The total size of memxr must be
 *	less than 8k bytes, to allow for 48k bytes of
 *	memory buffer space.
 *
 * USAGE:
 *		memxr size # fcopy
 *		      ^    ^ ^
 *		      |    | first copy flag
 *		      |    event flag bit position
 *		      memory size to malloc
 *
 *
 * Fred Canter 11/4/82
 * Bill Burns  4/23/84
 *	added event flags
 *
 */

#include <stdio.h>
#include <signal.h>

int	termsig;

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

main(argc, argv)
char *argv[];
int argc;
{
	int	stop(), intr();
	int	fizz();
	register char *cp;
	register int *ip;
	register unsigned int c;
	char	*ptr;
	unsigned int memsiz;
	int	first;
	char	cdp0, cdp1, cdp;
	int	idp;
	int	fcopy;
	FILE	*fi;

	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, stop);
	signal(SIGTERM, intr);
	signal(SIGPIPE, fizz);
#ifdef EFLG
	if((argc < 3) || (argc > 5))
		exit(0);
	if(argc == 5) {
		zflag++;
		efpis = argv[2];
		efbit = atoi(efpis);
		efids = argv[3];
		efid = atoi(efids);
		fcopy = atoi(argv[4]);
	} else
		fcopy = atoi(argv[2]);
#else
	if(argc != 5)
		exit(0);
	killfn = argv[2];
	fcopy = atoi(argv[3]);
#endif
	memsiz = atoi(argv[1]);
	memsiz = memsiz << 6;
	ptr = malloc(memsiz);
	if(ptr == 0) {
		fprintf(stderr, "\nmemxr: malloc failed, pid = %d\n", getpid());
		exit(SIGKILL);
		}
/*
 * Wait for parent process (memx1) to start
 * this process via SIGTERM.
 */
	if(fcopy != 0) {	/* first copy starts itself */
	ploop:
		termsig = 0;
		pause();
		if(!termsig)
			goto ploop;
	}
/*
 * Character (byte) mode test.
 * Checkerboard and anti-checkerboard data pattern.
 */
	first = 0;
ctest:
	cp = ptr;
	if(first == 0) {
		cdp0 = 0252;
		cdp1 = 0125;
	} else {
		cdp0 = 0125;
		cdp1 = 0252;
		}
	for(c=0; c<memsiz; c++) {	/* write memory, bytes */
		if(c & 2)
			*cp++ = cdp1;
		else
			*cp++ = cdp0;
		}
	cp = ptr;
	for(c=0; c<memsiz; c++) {	/* check memory data */
		if(c & 2)
			cdp = cdp1;
		else
			cdp = cdp0;
		if(*cp++ != cdp) {
			cp--;
			mderr(0, cp, cdp, *cp);
			exit(SIGKILL);
			}
		}
	if(first++ == 0)
		goto ctest;
/*
 * Integer (word) mode test.
 * Walking one and walking zero data patterns.
 */
	first = 0;
itest:
	ip = ptr;
	for(c=0; c<(memsiz/2); c += 2) {
		idp = (1 << (c & 017));
		if(!first)
			idp = ~idp;
		*ip++ = idp;
		}
	ip = ptr;
	for(c=0; c<(memsiz/2); c += 2) {
		idp = (1 << (c & 017));
		if(!first)
			idp = ~idp;
		if(*ip++ != idp) {
			ip--;
			mderr(1, ip, idp, *ip);
			exit(SIGKILL);
			}
		}
	if(first++ == 0)
		goto itest;
	fflush(stdout);
	exit(0);
}

/*
 * Memory data error routine
 *
 * mode = 0, byte access
 * mode = 1, word access
 *
 * addr = virtual address of failing byte,word,double
 * exp  = good data
 * act  = bad data
 */

mderr(mode, addr, exp, act)
int mode, exp, act;
char *addr;
{
	int	pid;

	pid = getpid();
	printf("\n\nMemory data error ");
	if(mode == 0)
		printf("[byte access]");
	else
		printf("[word access]");
	printf("\nMemory exerciser process I.D. = %d",pid);
	printf("\nVirtual address = %u",addr);
	printf("\nExpected data   = %6.o",exp);
	printf("\nActual data     = %6.o",act);
	printf("\n\n");
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
fizz()
{
	signal(SIGPIPE, SIG_IGN);
	termsig++;
}

stop()
{
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
