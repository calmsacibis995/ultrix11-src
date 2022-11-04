
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)dump.c 3.1 7/10/87";
/*
 * ULTRIX-11 file system dump program (dump)
 *
 * Modified for use with the Micro/pdp-11 and to prevent
 * accidentally overwriting the file system being dumped, due to
 * typing errors when typing the dump command.
 *
 * Also modified to allow for restarting multi-volume dumps
 * in the event of dump device errors.
 *
 * The m option is used to dump to RX50 floppy diskettes
 * on the Micro/pdp-11. The e option is used for RX33 diskettes.
 *
 * The y option causes the `continue' question not to be asked.
 * This is required when dump is used in a makefile, for
 * making distributions.
 *
 * Usage:	dump 0mf /dev/rrx0 /dev/rrd2
 *
 * Fred Canter 4/7/83
 */

#define	NI	8     	/* changed to 8 from 16 with the increase in BSIZE */
			/* (to prevent size overflow for overlay version) */
#define	DIRPB	(BSIZE/sizeof(struct direct))

/*
 * Not using stdio here, kernel printf() has been included locally.
 * WARNING: the printf version here only recognizes a subset of
 * format specifications, see printf() routine at the end of this
 * file for details.
 */
/*
#include <stdio.h>
*/
#include <sys/param.h>
#include <sys/inode.h>
#include <sys/ino.h>
#include <sys/fblk.h>
#include <sys/filsys.h>
#include <sys/dir.h>
#include <dumprestor.h>
#include <signal.h>
#include <errno.h>

#define	MWORD(m,i) (m[(unsigned)(i-1)/MLEN])
#define	MBIT(i)	(1<<((unsigned)(i-1)%MLEN))
#define	BIS(i,w)	(MWORD(w,i) |=  MBIT(i))
#define	BIC(i,w)	(MWORD(w,i) &= ~MBIT(i))
#define	BIT(i,w)	(MWORD(w,i) & MBIT(i))

struct	filsys	sblock;
struct	dinode	itab[INOPB*NI];
short	clrmap[MSIZ];
short	dirmap[MSIZ];
short	nodmap[MSIZ];

char	*disk;
char	*tape;
char	*increm;
char	incno;
int	uflag;
int	mflag;
int	kflag;
int	yflag;
int	tflag;	/* flag used to prevent rewrite on error question */
		/* see flusht(). Causes dump to exit with status 1, */
		/* allows for more control in scripts using dump */
int	fi;
int	to;
ino_t	ino;
int	nsubdir;
int	ntape;
int	nadded;
int	dadded;
int	density = 160;
int	mpid;
int	ppid;

char	*ctime();
char	*prdate();
long	atol();
int	fi;
long	tsize;
long	esize;
long	asize;
int	mark();
int	add();
int	dump();
int	tapsrec();
int	dmpspc();
int	dsrch();
int	nullf();

#define	HOUR	(60L*60L)
#define	DAY	(24L*HOUR)
#define	YEAR	(365L*DAY)

int	intr();

main(argc, argv)
char *argv[];
{
	char *arg;
	register i;

	time(&spcl.c_date);

	tsize = 2300L*12L*10L;
	tape = "/dev/rht0";
	disk = "/dev/null";
	increm = "/etc/ddate";
	incno = '9';
	uflag = 0;
	mflag = 0;
	kflag = 0;
	yflag = 0;
	arg = "u";
	if(argc > 1) {
		argv++;
		argc--;
		arg = *argv;
	}
	while(*arg)
	switch (*arg++) {

	case 'f':
		if(argc > 1) {
			argv++;
			argc--;
			tape = *argv;
		}
		break;

	case 'd':
		if (argc > 1) {
			argv++;
			argc--;
			if(strlen(*argv) > 4) {
			denerr:
				printf("bad density\n");
				exit(1);
			}
			density = atoi(*argv)/10;
			if(density == 0)
				goto denerr;
		}
		break;

	case 's':
		if(argc > 1) {
			argv++;
			argc--;
			tsize = atol(*argv);
			tsize *= 12L*10L;
		}
		break;

	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		incno = arg[-1];
		break;

	case 'u':
		uflag++;
		break;
	case 'm':		/* dump to RX50 diskettes */
		mflag++;
#ifdef UCB_NKB
		tsize = 400L;
#else
		tsize = 800L;
#endif
		break;
	case 'e':		/* dump to RX33 diskettes */
		mflag++;
#ifdef UCB_NKB
		tsize = 1200L;
#else
		tsize = 2400L;
#endif
		break;
	case 'k':
		kflag++;
		tsize = 8000L;
		tape = "/dev/rtk0";
		break;
	case 'y':
		yflag++;
		break;
	case 't':
		tflag++;
		break;

	default:
		printf("bad key '%c%'\n", arg[-1]);
		exit(1);
	}
	if(argc > 1) {
		argv++;
		argc--;
		disk = *argv;
	}

	mpid = getpid();
	signal(SIGTERM, SIG_IGN);
	getitime();
	printf("     date = %s\n", prdate(spcl.c_date));
	printf("dump date = %s\n", prdate(spcl.c_ddate));
	printf("dumping %s to %s\n", disk, tape);
	if(yflag == 0) {
		printf("\n\7\7\7Last chance before writing on %s\n", tape);
		printf("\nContinue <y or n> ? ");
		if(getchar() != 'y') {
			printf("\nDump aborted\n");
			exit(1);
		}
		while(getchar() != '\n') ;	/* flush input */
	}
	fi = open(disk, 0);
	if(fi < 0) {
		printf("dump: cannot open %s\n", disk);
		exit(1);
	}
	otape();
	printf("I\n");
	esize = 0;
	CLR(clrmap);
	CLR(dirmap);
	CLR(nodmap);

	pass(mark, (short *)NULL);
	do {
		printf("II\n");
		nadded = 0;
		pass(add, dirmap);
	} while(nadded);

	bmapest(clrmap);
	bmapest(nodmap);
	if(mflag) {
		printf("estimated %D blocks on %D diskette(s)\n",
			esize, (esize/tsize)+1);
	} else {
		printf("estimated %D tape blocks on %d tape(s)\n",
			esize, 0);
	}

	printf("III\n");
	bitmap(clrmap, TS_CLRI);
	pass(dump, dirmap);
	printf("IV\n");
	pass(dump, nodmap);
	putitime();
	printf("DONE\n");
	spcl.c_type = TS_END;
	for(i=0; i<NTREC; i++)
		spclrec();
	if(mflag) {
		printf("%D blocks on %d diskette(s)\n",
			spcl.c_tapea, spcl.c_volume);
	} else {
		printf("%D tape blocks on %d tape(s)\n",
			spcl.c_tapea, spcl.c_volume);
	}
	close(to);
	kill(ppid, SIGTERM);
	kill(mpid, SIGINT);	/* kill master copy of dump */
}

pass(fn, map)
int (*fn)();
short *map;
{
	register i, j;
	int bits;
	ino_t mino;
	daddr_t d;

	sync();
	bread((daddr_t)1, (char *)&sblock, sizeof(sblock));
	mino = (sblock.s_isize-2) * INOPB;
	ino = 0;
	for(i=2;; i+=NI) {
		if(ino >= mino)
			break;
		d = (unsigned)i;
		for(j=0; j<INOPB*NI; j++) {
			if(ino >= mino)
				break;
			if((ino % MLEN) == 0) {
				bits = ~0;
				if(map != NULL)
					bits = *map++;
			}
			ino++;
			if(bits & 1) {
				if(d != 0) {
					bread(d, (char *)itab, sizeof(itab));
					d = 0;
				}
				(*fn)(&itab[j]);
			}
			bits >>= 1;
		}
	}
}

icat(ip, fn1, fn2)
struct	dinode	*ip;
int (*fn1)(), (*fn2)();
{
	register i;
	daddr_t d[NADDR];

	l3tol(&d[0], &ip->di_addr[0], NADDR);
	(*fn2)(d, NADDR-3);
	for(i=0; i<NADDR; i++) {
		if(d[i] != 0) {
			if(i < NADDR-3)
				(*fn1)(d[i]); else
				indir(d[i], fn1, fn2, i-(NADDR-3));
		}
	}
}

indir(d, fn1, fn2, n)
daddr_t d;
int (*fn1)(), (*fn2)();
{
	register i;
	daddr_t	idblk[NINDIR];

	bread(d, (char *)idblk, sizeof(idblk));
	if(n <= 0) {
		spcl.c_type = TS_ADDR;
		(*fn2)(idblk, NINDIR);
		for(i=0; i<NINDIR; i++) {
			d = idblk[i];
			if(d != 0)
				(*fn1)(d);
		}
	} else {
		n--;
		for(i=0; i<NINDIR; i++) {
			d = idblk[i];
			if(d != 0)
				indir(d, fn1, fn2, n);
		}
	}
}

mark(ip)
struct dinode *ip;
{
	register f;

	f = ip->di_mode & IFMT;
	if(f == 0)
		return;
	BIS(ino, clrmap);
	if(f == IFDIR)
		BIS(ino, dirmap);
	if(ip->di_mtime >= spcl.c_ddate ||
	   ip->di_ctime >= spcl.c_ddate) {
		BIS(ino, nodmap);
#ifdef	UCB_SYMLINKS
		if (f != IFREG && f != IFLNK)
#else
		if (f != IFREG)
#endif
			return;
		est(ip);
	}
}

add(ip)
struct dinode *ip;
{

	if(BIT(ino, nodmap))
		return;
	nsubdir = 0;
	dadded = 0;
	icat(ip, dsrch, nullf);
	if(dadded) {
		BIS(ino, nodmap);
		est(ip);
		nadded++;
	}
	if(nsubdir == 0)
		if(!BIT(ino, nodmap))
			BIC(ino, dirmap);
}

dump(ip)
struct dinode *ip;
{
	register i;

	if(ntape) {
		ntape = 0;
		bitmap(nodmap, TS_BITS);
	}
	BIC(ino, nodmap);
	spcl.c_dinode = *ip;
	spcl.c_type = TS_INODE;
	spcl.c_count = 0;
	i = ip->di_mode & IFMT;
#ifdef	UCB_SYMLINKS
	if(i != IFDIR && i != IFREG && i != IFLNK)
#else
	if(i != IFDIR && i != IFREG)
#endif
	{
		spclrec();
		return;
	}
	icat(ip, tapsrec, dmpspc);
}

dmpspc(dp, n)
daddr_t *dp;
{
	register i, t;

	spcl.c_count = n;
	for(i=0; i<n; i++) {
		t = 0;
		if(dp[i] != 0)
			t++;
		spcl.c_addr[i] = t;
	}
	spclrec();
}

bitmap(map, typ)
short *map;
{
	register i, n;
	char *cp;

	n = -1;
	for(i=0; i<MSIZ; i++)
		if(map[i])
			n = i;
	if(n < 0)
		return;
	spcl.c_type = typ;
	spcl.c_count = (n*sizeof(map[0]) + BSIZE)/BSIZE;
	spclrec();
	cp = (char *)map;
	for(i=0; i<spcl.c_count; i++) {
		taprec(cp);
		cp += BSIZE;
	}
}

spclrec()
{
	register i, *ip, s;

	spcl.c_inumber = ino;
	spcl.c_magic = MAGIC;
	spcl.c_checksum = 0;
	ip = (int *)&spcl;
	s = 0;
	for(i=0; i<BSIZE/sizeof(*ip); i++)
		s += *ip++;
	spcl.c_checksum = CHECKSUM - s;
	taprec((char *)&spcl);
}

dsrch(d)
daddr_t d;
{
	register char *cp;
	register i;
	register ino_t in;
	struct direct dblk[DIRPB];

	if(dadded)
		return;
	bread(d, (char *)dblk, sizeof(dblk));
	for(i=0; i<DIRPB; i++) {
		in = dblk[i].d_ino;
		if(in == 0)
			continue;
		cp = dblk[i].d_name;
		if(cp[0] == '.') {
			if(cp[1] == '\0')
				continue;
			if(cp[1] == '.' && cp[2] == '\0')
				continue;
		}
		if(BIT(in, nodmap)) {
			dadded++;
			return;
		}
		if(BIT(in, dirmap))
			nsubdir++;
	}
}

nullf()
{
}

bread(da, ba, c)
daddr_t da;
char *ba;
{
	register n;

#ifndef	UCB_NKB
	lseek(fi, da*512, 0);
#else
	lseek(fi, da*BSIZE, 0);
#endif	UCB_NKB
	n = read(fi, ba, c);
	if(n != c) {
		printf("\n\7\7\7Read error on %s, bn = %D\n", disk, da);
		printf("asked for %d; got %d bytes\n", c, n);
	}
}

CLR(map)
register short *map;
{
	register n;

	n = MSIZ;
	do
		*map++ = 0;
	while(--n);
}


char	tblock[NTREC][BSIZE];
daddr_t	tdaddr[NTREC];
int	trecno;

taprec(dp)
char *dp;
{
	register i;

	for(i=0; i<BSIZE; i++)
		tblock[trecno][i] = *dp++;
	tdaddr[trecno] = 0;
	trecno++;
	spcl.c_tapea++;
	if(trecno >= NTREC)
		flusht();
}

tapsrec(d)
daddr_t d;
{

	if(d == 0)
		return;
	tdaddr[trecno] = d;
	trecno++;
	spcl.c_tapea++;
	if(trecno >= NTREC)
		flusht();
}

flusht()
{
	char place[100];
	register i, si;
	daddr_t d;

	while(trecno < NTREC)
		tdaddr[trecno++] = 1;

loop:
	d = 0;
	for(i=0; i<NTREC; i++)
		if(tdaddr[i] != 0)
		if(d == 0 || tdaddr[i] < d) {
			si = i;
			d = tdaddr[i];
		}
	if(d != 0) {
		bread(d, tblock[si], BSIZE);
		tdaddr[si] = 0;
		goto loop;
	}
	trecno = 0;
	if(write(to, tblock[0], sizeof(tblock)) != sizeof(tblock)) {
		close(to);
		printf("dump: write error on %s \ndump: ", tape);
		switch(errno) {
		    case ENOSPC: printf("No space left on device");
				break;
		    case ETPL: printf("Fatal error - tape position lost");
				break;
		    default:
				break;
		}
		printf(" (errno = %d)\n",errno);

	/*  below is used to cause parent to exit with status 1
	     and let a controlling script/process decide what to
	     do (helps out dumps with TK25 where tape repositioning
	     must be done) */

		if(!tflag) {
			printf("\nRewrite this volume <y or n> ? ");
			read(0, place, sizeof(place));
			if((place[0] == 'y') || (place[0] == '\n'))
				exit(0);	/* tells parent to restart */
			else {
				kill(mpid, SIGINT); /* causes parent to exit */
				exit(1);	   /* with 0 status */
			}
		} else
			exit(1); /* tells parent to exit with status 1 */
	}
	if(mflag)
		asize += sizeof(tblock)/BSIZE;
	else if(kflag)
		asize++;
	else {
		asize += sizeof(tblock)/density;
		asize += 7;
	}
	if(asize >= tsize) {
		close(to);
		printf("\nMount next ");
		if(mflag)
			printf("diskette");
		else
			printf("tape");
		printf(" <type return when ready>");
		read(0, place, sizeof(place));
		kill(ppid, SIGTERM);
		otape();
	}
}

otape()
{
	int	cpid;
	int	status;
	char	c;

redump:
	ppid = getpid();
	if(ppid != mpid)
		signal(SIGTERM, SIG_DFL);
	cpid = fork();
	if(cpid < 0) {
		printf("Can't fork (pid = %d)\n", ppid);
		exit(1);
	}
	if(cpid != 0) {	/* parent */
		if(getpid() == mpid)
			signal(SIGINT, intr);
		wait(&status);
		if(status == 0)
			goto redump;
	/* tflag added to allow dump to exit with status 1 */
		if((getpid() == mpid) && (!tflag))
			for( ;;) ;
		exit(1);
	} else {	/* child */
	reopen:
		to = creat(tape, 0666);
		if(to < 0) {
			printf("dump: cannot create %s \ndump: ", tape);
			switch (errno) {
			    case ENODEV:
			    case ENXIO: printf("No such device or address");
					break;
			    case EACCES: printf("Permission denied");
					break;
			    case EROFS: printf("Read-only device");
					break;
			    case ETOL:	printf("Tape unit offline");
					break;
			    case ETWL:	printf("Tape unit write locked");
					break;
			    case ETO:	printf("Tape unit already open");
					break;
			    default:
					break;
			}
			printf(" (errno = %d)\n",errno);
		ask_rt:
			printf("\nretry <y or n> ? ");
			c = getchar();
			if(c == '\n')
				goto ask_rt;
			while(getchar() != '\n') ;
			if(c == 'y')
				goto reopen;
			if(c != 'n')
				goto ask_rt;
			kill(mpid, SIGINT);
			exit(1);
		}
		asize = 0;
		ntape++;
		spcl.c_volume++;
		spcl.c_type = TS_TAPE;
		spclrec();
	}
}

char *
prdate(d)
time_t d;
{
	char *p;

	if(d == 0)
		return("the epoch");
	p = ctime(&d);
	p[24] = 0;
	return(p);
}

getitime()
{
	register i, df;
	struct idates idbuf;
	char *fname;

	fname = disk;
l1:
	for(i=0; fname[i]; i++)
		if(fname[i] == '/') {
			fname += i+1;
			goto l1;
		}

	spcl.c_ddate = 0;
	df = open(increm, 0);
	if(df < 0) {
		printf("cannot open %s\n", increm);
		exit(1);
	}

l2:
	i = read(df, (char *)&idbuf, sizeof(idbuf));
	if(i != sizeof(idbuf)) {
		close(df);
		return;
	}
	for(i=0;; i++) {
		if(fname[i] != idbuf.id_name[i])
			goto l2;
		if(fname[i] == '\0')
			break;
	}
	if(idbuf.id_incno >= incno)
		goto l2;
	if(idbuf.id_ddate <= spcl.c_ddate)
		goto l2;
	spcl.c_ddate = idbuf.id_ddate;
	goto l2;
}

putitime()
{
	register i, n, df;
	struct idates idbuf;
	char *fname;

	if(uflag == 0)
		return;
	fname = disk;
l1:
	for(i=0; fname[i]; i++)
		if(fname[i] == '/') {
			fname += i+1;
			goto l1;
		}

	spcl.c_ddate = 0;
	df = open(increm, 2);
	if(df < 0) {
		printf("cannot open %s\n", increm);
		exit(1);
	}
	n = 0;
l2:
	i = read(df, (char *)&idbuf, sizeof(idbuf));
	if(i != sizeof(idbuf)) {
		lseek(df, (long)n*sizeof(idbuf), 0);
		goto l3;
	}
	n++;
	for(i=0;; i++) {
		if(fname[i] != idbuf.id_name[i])
			goto l2;
		if(fname[i] == '\0')
			break;
	}
	if(idbuf.id_incno != incno)
		goto l2;
	lseek(df, (long)(n-1)*sizeof(idbuf), 0);
l3:
	for(i=0;; i++) {
		idbuf.id_name[i] = fname[i];
		if(fname[i] == '\0') {   /* fill the rest of name with blanks: 
					  * George Mathew 6/20/85) */
			for (i++; i < (sizeof(idbuf.id_name) -1); i++)
				idbuf.id_name[i] = ' ';
			idbuf.id_name[i] = '\0';
			break;
		}
	}
	idbuf.id_incno = incno;
	idbuf.id_ddate = spcl.c_date;
	if (write(df, (char *)&idbuf, sizeof(idbuf)) != sizeof(idbuf)) {
		close(df);
		printf("dump: /etc/ddate write error\n");
		exit(-1);
	}
	close(df);
	printf("level %c dump on %s\n", incno, prdate(spcl.c_date));
}

est(ip)
struct dinode *ip;
{
	long s;

	esize++;
	s = (ip->di_size + BSIZE-1) / BSIZE;
	esize += s;
	if(s > NADDR-3) {
		s -= NADDR-3;
		s = (s + (BSIZE/sizeof(daddr_t))-1) / (BSIZE/sizeof(daddr_t));
		esize += s;
	}
}

bmapest(map)
short *map;
{
	register i, n;

	n = -1;
	for(i=0; i<MSIZ; i++)
		if(map[i])
			n = i;
	if(n < 0)
		return;
	esize++;
	esize += (n + (BSIZE/sizeof(short))-1) / (BSIZE/sizeof(short));
}

intr()
{
	exit(0);	/* to keep make happy */
}

getchar()
{
	char c;
	if (read(0, &c, 1) < 0)
		return(-1);
	else
		return(c);
}
putchar(c)
char c;
{
	write(1, &c, 1);
}

/*
 * Scaled down version of C Library printf.
 * Only %s %u %d (==%u) %o %x %D are recognized.
 */
/* VARARGS 1 */
printf(fmt, x1)
register char *fmt;
unsigned x1;
{
	register c;
	register unsigned int *adx;
	char *s;

	adx = &x1;
loop:
	while((c = *fmt++) != '%') {
		if(c == '\0')
			return;
		putchar(c);
	}
	c = *fmt++;
	if(c == 'd' || c == 'u' || c == 'o' || c == 'x')
		printn((long)*adx, c=='o'? 8: (c=='x'? 16:10));
	else if(c == 's') {
		s = (char *)*adx;
		while(c = *s++)
			putchar(c);
	} else if (c == 'c') {
		putchar(*(char *)adx);
	} else if (c == 'D') {
		printn(*(long *)adx, 10);
		adx += (sizeof(long) / sizeof(int)) - 1;
	}
	adx++;
	goto loop;
}

/*
 * Print an unsigned integer in base b.
 */
printn(n, b)
long n;
{
	register long a;

	if (n<0) {	/* shouldn't happen */
		putchar('-');
		n = -n;
	}
	if(a = n/b)
		printn(a, b);
	putchar("0123456789ABCDEF"[(int)(n%b)]);
}
