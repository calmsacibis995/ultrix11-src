
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)w.c	3.0	4/22/86";
/*
 * w - print system status (who and what)
 *
 * This program is similar to the systat command on Tenex/Tops 10/20
 * It needs read permission on /dev/mem and /dev/swap.
 *
 * PDP-11 V7 version that does not run off ps -r.
 */
#include <whoami.h>
#include <a.out.h>
#include <core.h>
#include <stdio.h>
#include <ctype.h>
#include <utmp.h>
#include <time.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/proc.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/tty.h>

#define ARGWIDTH	33	/* # chars left on 80 col crt for args */
#define ARGLIST 1024	/* amount of stack to examine for argument list */

struct smproc {
	long	w_addr;			/* address in file for args */
	short	w_pid;			/* proc.p_pid */
	int	w_igintr;		/* INTR+3*QUIT, 0=die, 1=ign, 2=catch */
	time_t	w_time;			/* CPU time used by this process */
	time_t	w_ctime;		/* CPU time used by children */
	dev_t	w_tty;			/* tty device of process */
	char	w_comm[15];		/* user.u_comm, null terminated */
	char	w_args[ARGWIDTH+1];	/* args if interesting process */
} *pr;

struct	nlist nl[] = {
	{ "_proc" },
#define	X_PROC		0
	{ "_swapdev" },
#define	X_SWAPDEV	1
	{ "_swplo" },
#define	X_SWPLO		2
	{ "_avenrun" },
#define	X_AVENRUN	3
	{ "_boottime" },
#define	X_BOOTTIME	4
	{ "_nproc" },
#define	X_NPROC		5
	{ 0 },
};

FILE	*ps;
FILE	*ut;
FILE	*bootfd;
int	swmem;
int	mem;
int	swap;			/* /dev/mem, mem, and swap */
int	nswap;
int	file;
dev_t	tty;
char	doing[520];		/* process attached to terminal */
time_t	proctime;		/* cpu time of process in doing */
short	avenrun[3];
double	load[3];

#define	DIV60(t)	((t+30)/60)    /* x/60 rounded */ 
#define	TTYEQ		(tty == pr[i].w_tty)
#define IGINT		(1+3*1)		/* ignoring both SIGINT & SIGQUIT */

long	round();
char	*getargs();
char	*fread();
char	*ctime();
char	*rindex();
char	*getptr();
FILE	*popen();
struct	tm *localtime();

int	debug;			/* true if -d flag: debugging output */
int	header = 1;		/* true if -h flag: don't print heading */
int	lflag = 1;		/* true if -l flag: long style output */
int	login;			/* true if invoked as login shell */
time_t	idle;			/* number of minutes user is idle */
int	nusers;			/* number of users logged in now */
char *	sel_user;		/* login of particular user selected */
char firstchar;			/* first char of name of prog invoked as */
time_t	jobtime;		/* total cpu time visible */
time_t	now;			/* the current time of day */
struct	tm *nowt;		/* current time as time struct */
time_t	boottime, uptime;	/* time of last reboot & elapsed time since */
int	np;			/* number of processes currently active */
struct	utmp utmp;
struct	proc mproc;
struct	user up;
char	fill[512];

struct map {
	long	b1, e1; long f1;
	long	b2, e2; long f2;
};
struct map datmap;

main(argc, argv)
	char **argv;
{
	int days, hrs, mins;
	register int i, j;
	char *cp;
	register int curpid, empty;
	extern char _sobuf[];

	setbuf(stdout, _sobuf);
	login = (argv[0][0] == '-');
	cp = rindex(argv[0], '/');
	firstchar = login ? argv[0][1] : (cp==0) ? argv[0][0] : cp[1];
	cp = argv[0];	/* for Usage */

	while (argc > 1) {
		if (argv[1][0] == '-') {
			for (i=1; argv[1][i]; i++) {
				switch(argv[1][i]) {

				case 'd':
					debug++;
					break;

				case 'h':
					header = 0;
					break;

				case 'l':
					lflag++;
					break;

				case 's':
					lflag = 0;
					break;

				case 'u':
				case 'w':
					firstchar = argv[1][1];
					break;

				default:
					printf("Bad flag %s\n", argv[1]);
					exit(1);
				}
			}
		} else {
			if (!isalnum(argv[1][0]) || argc > 2) {
				printf("Usage: %s [ -hlsuw ] [ user ]\n", cp);
				exit(1);
			} else
				sel_user = argv[1];
		}
		argc--; argv++;
	}

	if ((mem = open("/dev/kmem", 0)) < 0) {
		fprintf(stderr, "No mem\n");
		exit(1);
	}
	nlist("/unix", nl);
	if (nl[0].n_type==0) {
		fprintf(stderr, "No namelist\n");
		exit(1);
	}

	if (firstchar != 'u')
		readpr();

	ut = fopen("/etc/utmp","r");
	if (header) {
		/* Print time of day */
		time(&now);
		nowt = localtime(&now);
		prtat(nowt);

		if (nl[X_BOOTTIME].n_type > 0) {
			/*
			 * Print how long system has been up.
			 * (Found by looking for "boottime" in kernel)
			 */
			lseek(mem, (long)nl[X_BOOTTIME].n_value, 0);
			read(mem, &boottime, sizeof (boottime));

			uptime = now - boottime;
			days = uptime / (60L*60L*24L);
			uptime %= (60L*60L*24L);
			hrs = uptime / (60L*60L);
			uptime %= (60L*60L);
			mins = DIV60(uptime);

			printf("  up");
			if (days > 0)
				printf(" %d day%s,", days, days>1?"s":"");
			if (hrs > 0 && mins > 0) {
				printf(" %2d:%02d,", hrs, mins);
			} else {
				if (hrs > 0)
					printf(" %d hr%s,", hrs, hrs>1?"s":"");
				if (mins > 0)
					printf(" %d min%s,", mins, mins>1?"s":"");
			}
		}

		/* Print number of users logged in to system */
		while (fread(&utmp, sizeof(utmp), 1, ut)) {
			if (utmp.ut_name[0] != '\0')
				nusers++;
		}
		rewind(ut);
		printf("  %d user%c", nusers, nusers > 1 ?  's' : '\0');

		if (nl[X_AVENRUN].n_type > 0) {
			/*
			 * Print 1, 5, and 15 minute load averages.
			 * (Found by looking in kernel for avenrun).
			 */
			printf(",  load average:");
			lseek(mem, (long)nl[X_AVENRUN].n_value, 0);
			read(mem, avenrun, sizeof(avenrun));
			for (i = 0; i < (sizeof(avenrun)/sizeof(avenrun[0])); i++) {
				load[i] = avenrun[i] / 256.0;
				if (i > 0)
					printf(",");
				printf(" %.2f", load[i]);
			}
		}
		printf("\n");
		if (firstchar == 'u')
			exit(0);

		/* Headers for rest of output */
		if (lflag)
			printf("User     tty       login@  idle   JCPU   PCPU  what\n");
		else
			printf("User     tty   idle  what\n");
		fflush(stdout);
	}


	for (;;) {	/* for each entry in utmp */
		if (fread(&utmp, sizeof(utmp), 1, ut) == NULL) {
			fclose(ut);
			exit(0);
		}
		if (utmp.ut_name[0] == '\0')
			continue;	/* that tty is free */
		if (sel_user && strncmp(utmp.ut_name, sel_user, 8) != 0)
			continue;	/* we wanted only somebody else */

		gettty();
		jobtime = 0;
		proctime = 0;
		strcpy(doing, "-");	/* default act: normally never prints */
		empty = 1;
		curpid = -1;
		idle = findidle();
		for (i=0; i<np; i++) {	/* for each process on this tty */
			if (!(TTYEQ))
				continue;
			jobtime += pr[i].w_time + pr[i].w_ctime;
			proctime += pr[i].w_time;
			if (debug) {
				printf("\t\t%d\t%s", pr[i].w_pid, pr[i].w_args);
				if ((j=pr[i].w_igintr) > 0)
					if (j==IGINT)
						printf(" &");
					else
						printf(" & %d %d", j%3, j/3);
				printf("\n");
			}
			if (empty && pr[i].w_igintr!=IGINT) {
				empty = 0;
				curpid = -1;
			}
			if(pr[i].w_pid>curpid && (pr[i].w_igintr!=IGINT || empty)){
				curpid = pr[i].w_pid;
				strcpy(doing, lflag ? pr[i].w_args : pr[i].w_comm);
				if (doing[0]==0 || doing[0]=='-' && doing[1]<=' ' || doing[0] == '?') {
					strcat(doing, " (");
					strcat(doing, pr[i].w_comm);
					strcat(doing, ")");
				}
			}
		}
		putline();
	}
}

/* figure out the major/minor device # pair for this tty */
gettty()
{
	char ttybuf[20];
	struct stat statbuf;

	ttybuf[0] = 0;
	strcpy(ttybuf, "/dev/");
	strcat(ttybuf, utmp.ut_line);
	stat(ttybuf, &statbuf);
	tty = statbuf.st_rdev;
}

/*
 * putline: print out the accumulated line of info about one user.
 */
putline()
{
	register int tm;

	/* print login name of the user */
	printf("%-8.8s ", utmp.ut_name);

	/* print tty user is on */
	if (lflag)
		/* long form: all (up to) 8 chars */
		printf("%-8.8s", utmp.ut_line);
	else {
		/* short form: 4 chars, skipping 'tty' if there */
		if (utmp.ut_line[0]=='t' && utmp.ut_line[1]=='t' && utmp.ut_line[2]=='y')
			printf("%-4.4s", &utmp.ut_line[3]);
		else
			printf("%-4.4s", utmp.ut_line);
	}

	if (lflag)
		/* print when the user logged in */
		prtat(localtime(&utmp.ut_time));

	/* print idle time */
	prttime(idle," ");

	if (lflag) {
		/* print CPU time for all processes & children */
		prttime(DIV60(jobtime)," ");
		/* print cpu time for interesting process */
		prttime(DIV60(proctime)," ");
	}

	/* what user is doing, either command tail or args */
	printf(" %-.32s\n",doing);
	fflush(stdout);
}

/* find & return number of minutes current tty has been idle */
findidle()
{
	struct stat stbuf;
	long lastaction, diff;
	char ttyname[20];

	strcpy(ttyname, "/dev/");
	strncat(ttyname, utmp.ut_line, 8);
	stat(ttyname, &stbuf);
	time(&now);
	lastaction = stbuf.st_atime;
	diff = now - lastaction;
	diff = DIV60(diff);
	if (diff < 0) diff = 0;
	return(diff);
}

/*
 * prttime prints a time in hours and minutes.
 * The character string tail is printed at the end, obvious
 * strings to pass are "", " ", or "am".
 */
prttime(tim, tail)
	time_t tim;
	char *tail;
{
	register int didhrs = 0;

	if (tim >= 60) {
		printf("%3ld:", tim/60);
		didhrs++;
	} else {
		printf("    ");
	}
	tim %= 60;
	if (tim > 0 || didhrs) {
		printf(didhrs&&tim<10 ? "%02ld" : "%2ld", tim);
	} else {
		printf("  ");
	}
	printf("%s", tail);
}

/* prtat prints a 12 hour time given a pointer to a time of day */
prtat(p)
	struct tm *p;
{
	register int pm;
	register time_t t;

	t = p -> tm_hour;
	pm = (t > 11);
	if (t > 11)
		t -= 12;
	if (t == 0)
		t = 12;
	prttime(t*60 + p->tm_min, pm ? "pm" : "am");
}

/*
 * readpr finds and reads in the array pr, containing the interesting
 * parts of the proc and user tables for each live process.
 */
readpr()
{
	int pn, mf, c, nproc;
	int szpt, pfnum, i;
	long addr;
#ifdef	VIRUS_VFORK
	long daddr, saddr;
#endif
	daddr_t swplo;
	long txtsiz, datsiz, stksiz;
	int septxt;

	if((swmem = open("/dev/mem", 0)) < 0) {
		perror("/dev/mem");
		exit(1);
	}
	if ((swap = open("/dev/swap", 0)) < 0) {
		perror("/dev/swap");
		exit(1);
	}
	/*
	 * read mem to find swap dev.
	 */
	lseek(mem, (long)nl[X_SWAPDEV].n_value, 0);
	read(mem, &nl[X_SWAPDEV].n_value, sizeof(nl[X_SWAPDEV].n_value));
	/*
	 * Find base of swap
	 */
	lseek(mem, (long)nl[X_SWPLO].n_value, 0);
	read(mem, &swplo, sizeof(swplo));
	if (nl[X_NPROC].n_value == 0) {
		fprintf(stderr, "nproc not in namelist\n");
		exit(1);
	}
	lseek (mem, (off_t) nl[X_NPROC].n_value, 0);
	read(mem, (char *)&nproc, sizeof(nproc));
	pr = (struct smproc *) malloc(nproc * sizeof(struct smproc));
	if (pr == (struct smproc *)NULL) {
		fprintf("Not enough memory for proc table\n");
		exit(1);
	}
	/*
	 * Locate proc table
	 */
	np = 0;
	for (pn=0; pn<nproc; pn++) {
		lseek(mem, (long)(nl[X_PROC].n_value + pn*(sizeof mproc)), 0);
		pread(mem, &mproc, sizeof mproc, (long)(nl[X_PROC].n_value + pn*(sizeof mproc)));
		/* decide if it's an interesting process */
		if (mproc.p_stat==0 || mproc.p_pgrp==0)
			continue;

#ifdef notdef
		/*
		 * The following improves speed on systems with lots of ttys
		 * by skipping gettys and inits, but loses when root logs in.
		 */
		if (mproc.p_ppid == 1 && mproc.p_uid == 0)
			continue;
#endif
		/* find & read in the user structure */
		if (mproc.p_flag&SLOAD) {
			addr = ctob((long)mproc.p_addr);
#ifdef	VIRUS_VFORK
			daddr = ctob((long)mproc.p_daddr);
			saddr = ctob((long)mproc.p_saddr);
#endif
			file = swmem;
		} else {
			addr = (mproc.p_addr+swplo)<<9;
#ifdef	VIRUS_VFORK
			daddr = (mproc.p_daddr+swplo)<<9;
			saddr = (mproc.p_saddr+swplo)<<9;
#endif
			file = swap;
		}
		lseek(file, addr, 0);
		if (pread(file, (char *)&up, sizeof(up), addr) != sizeof(up))
			continue;
		if (up.u_ttyp == NULL)
			continue;

		/* set up address maps for user pcs */
		txtsiz = ctob(up.u_tsize);
		datsiz = ctob(up.u_dsize);
		stksiz = ctob(up.u_ssize);
		septxt = up.u_sep;
		datmap.b1 = (septxt ? 0 : round(txtsiz,TXTRNDSIZ));
		datmap.e1 = datmap.b1+datsiz;
#ifdef	VIRUS_VFORK
		datmap.f1 = daddr;
#else
		datmap.f1 = ctob(USIZE)+addr;
#endif
		datmap.b2 = stackbas(stksiz);
		datmap.e2 = stacktop(stksiz);
#ifdef	VIRUS_VFORK
		datmap.f2 = saddr;
#else
		datmap.f2 = ctob(USIZE)+(datmap.e1-datmap.b1)+addr;
#endif

		/* save the interesting parts */
#ifdef	VIRUS_VFORK
		pr[np].w_addr = saddr + ctob((long)mproc.p_ssize) - ARGLIST;
#else
		pr[np].w_addr = addr + ctob((long)mproc.p_size) - ARGLIST;
#endif
		pr[np].w_pid = mproc.p_pid;
		pr[np].w_igintr = ((up.u_signal[2]==1) + 2*(up.u_signal[2]>1) + 3*(up.u_signal[3]==1)) + 6*(up.u_signal[3]>1);
		pr[np].w_time = up.u_utime + up.u_stime;
		pr[np].w_ctime = up.u_cutime + up.u_cstime;
		pr[np].w_tty = up.u_ttyd;
		up.u_comm[14] = 0;	/* Bug: This bombs next field. */
		strcpy(pr[np].w_comm, up.u_comm);
		/*
		 * Get args if there's a chance we'll print it.
		 * Cant just save pointer: getargs returns static place.
		 * Cant use strncpy: that crock blank pads.
		 */
		pr[np].w_args[0] = 0;
		strncat(pr[np].w_args,getargs(&pr[np]),ARGWIDTH);
		if (pr[np].w_args[0]==0 || pr[np].w_args[0]=='-' && pr[np].w_args[1]<=' ' || pr[np].w_args[0] == '?') {
			strcat(pr[np].w_args, " (");
			strcat(pr[np].w_args, pr[np].w_comm);
			strcat(pr[np].w_args, ")");
		}
		np++;
	}
}

/*
 * getargs: given a pointer to a proc structure, this looks at the swap area
 * and tries to reconstruct the arguments. This is straight out of ps.
 */
char *
getargs(p)
	struct smproc *p;
{
	int c, nbad;
	static char abuf[ARGLIST];
	register int *ip;
	register char *cp, *cp1;
	char **ap;
	long addr;

	addr = p->w_addr;

	/* look for sh special */
	lseek(file, addr+ARGLIST-sizeof(char **), 0);
	if (read(file, (char *)&ap, sizeof(char *)) != sizeof(char *))
		return(NULL);
	if (ap) {
		char *b = (char *) abuf;
		char *bp = b;
		while((cp=getptr(ap++)) && cp && (bp<b+ARGWIDTH) ) {
			nbad = 0;
			while((c=getbyte(cp++)) && (bp<b+ARGWIDTH)) {
				if (c<' ' || c>'~') {
					if (nbad++>3)
						break;
					continue;
				}
				*bp++ = c;
			}
			*bp++ = ' ';
		}
		*bp++ = 0;
		return(b);
	}

	lseek(file, addr, 0);
	if (pread(file, abuf, sizeof(abuf), addr) != sizeof(abuf))
		return(1);
	for (ip = (int *) &abuf[ARGLIST]-2; ip > (int *) abuf;) {
		/* Look from top for -1 or 0 as terminator flag. */
		if (*--ip == -1 || *ip == 0) {
			cp = (char *)(ip+1);
			if (*cp==0)
				cp++;
			nbad = 0;	/* up to 5 funny chars as ?'s */
			for (cp1 = cp; cp1 < (char *)&abuf[ARGLIST]; cp1++) {
				c = *cp1&0177;
				if (c==0)  /* nulls between args => spaces */
					*cp1 = ' ';
				else if (c < ' ' || c > 0176) {
					if (++nbad >= 5) {
						*cp1++ = ' ';
						break;
					}
					*cp1 = '?';
				} else if (c=='=') {	/* Oops - found an
							 * environment var, back
							 * over & erase it. */
					*cp1 = 0;
					while (cp1>cp && *--cp1!=' ')
						*cp1 = 0;
					break;
				}
			}
			while (*--cp1==' ')	/* strip trailing spaces */
				*cp1 = 0;
			return(cp);
		}
	}
	return (p->w_comm);
}

min(a, b)
{

	return (a < b ? a : b);
}

char *
getptr(adr)
char **adr;
{
	char *ptr;
	register char *p, *pa;
	register i;

	ptr = 0;
	pa = (char *)adr;
	p = (char *)&ptr;
	for (i=0; i<sizeof(ptr); i++)
		*p++ = getbyte(pa++);
	return(ptr);
}

getbyte(adr)
char *adr;
{
	register struct map *amap = &datmap;
	char b;
	long saddr;

	if(!within(adr, amap->b1, amap->e1)) {
		if(within(adr, amap->b2, amap->e2)) {
			saddr = (unsigned)adr + amap->f2 - amap->b2;
		} else
			return(0);
	} else
		saddr = (unsigned)adr + amap->f1 - amap->b1;
	if(lseek(file, saddr, 0)==-1
		   || read(file, &b, 1)<1) {
		return(0);
	}
	return((unsigned)b);
}


within(adr,lbd,ubd)
char *adr;
long lbd, ubd;
{
	return((unsigned)adr>=lbd && (unsigned)adr<ubd);
}

long
round(a, b)
	long		a, b;
{
	long		w = ((a+b-1)/b)*b;

	return(w);
}

/*
 * pread is like read, but if it's /dev/mem we use the phys
 * system call for speed.  (On systems without phys we have
 * to use regular read.)
 */
pread(fd, ptr, nbytes, loc)
char *ptr;
long loc;
{
	int rc;
	extern int errno;

	if (fd == swmem) {
		rc=phys(6, nbytes/64+1, (short)(loc/64));
		if (rc>=0) {
			memcpy(ptr, 0140000, nbytes);
			return nbytes;
		} else {
			return read(fd, ptr, nbytes);
		}
	} else {
		return read(fd, ptr, nbytes);
	}
}

/*
memcpy(dest, src, nbytes)
register char *dest, *src;
register int nbytes;
{
	while (nbytes--)
		*dest++ = *src++;
}
*/
