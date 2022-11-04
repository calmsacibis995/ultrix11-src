
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * ncheck -- obtain file names from reading filesystem
 */

static char Sccsid[] = "@(#)ncheck.c 3.0 4/21/86";

/*
 * WARNING:
 * ncheck uses it's own internal version of printf. An
 * additional argument is used to indicate stdout or stderr.
 * It does not accept all the formats of standard printf.
 * This is done to fit the process within size limits for
 * non-separate I&D machines.
 */

/* #include <stdio.h> */
#include <sys/param.h>
#include <sys/inode.h>
#include <sys/ino.h>
#include <sys/dir.h>
#include <sys/filsys.h>
#include <sys/fblk.h>


#ifdef	UCB_NKB
#define	NI	8
#else
#define	NI	16
#endif  UCB_NKB

#define	NB	100

#ifdef	NCHECK40
#define	HSIZE	2350
#else
#define	HSIZE	2503
#endif 	NCHECK40
#define	NDIR	(BSIZE/sizeof(struct direct))

struct	filsys	sblock;
struct	dinode	itab[INOPB*NI];
daddr_t	iaddr[NADDR];
ino_t	ilist[NB];
struct	htab
{
	ino_t	h_ino;
	ino_t	h_pino;
	char	h_name[DIRSIZ];
} htab[HSIZE];

int	aflg;
int	sflg;
int	fi;
ino_t	ino;
int	nhent;
int	nxfile;

int	nerror;
daddr_t	bmap();
long	atol();
struct htab *lookup();
char tmpbuf[14];

main(argc, argv)
char *argv[];
{
	register i;
	long n;

	while (--argc) {
		argv++;
		if (**argv=='-')
		switch ((*argv)[1]) {

		case 'a':
			aflg++;
			continue;

		case 'i':
			for(i=0; i<NB; i++) {
				n = atol(argv[1]);
				if(n == 0)
					break;
				ilist[i] = n;
				nxfile = i;
				argv++;
				argc--;
			}
			continue;

		case 's':
			sflg++;
			continue;

		default:
			/* fprintf(stderr, "ncheck: bad flag %c\n", (*argv)[1]);*/
			printf(2, "ncheck: bad flag %c\n", (*argv)[1]);
			nerror++;
		}
		check(*argv);
	}
	return(nerror);
}

check(file)
char *file;
{
	register i, j;
	ino_t mino;

	fi = open(file, 0);
	if(fi < 0) {
		/* fprintf(stderr, "ncheck: cannot open %s\n", file); */
		printf(2, "ncheck: cannot open %s\n", file);
		nerror++;
		return;
	}
	nhent = 0;
	/* printf("%s:\n", file); */
	printf(1,"%s:\n", file);
	sync();
	bread((daddr_t)1, (char *)&sblock, sizeof(sblock));
	mino = (sblock.s_isize-2) * INOPB;
	ino = 0;
	for(i=2;; i+=NI) {
		if(ino >= mino)
			break;
		bread((daddr_t)i, (char *)itab, sizeof(itab));
		for(j=0; j<INOPB*NI; j++) {
			if(ino >= mino)
				break;
			ino++;
			pass1(&itab[j]);
		}
	}
	ilist[nxfile+1] = 0;
	ino = 0;
	for(i=2;; i+=NI) {
		if(ino >= mino)
			break;
		bread((daddr_t)i, (char *)itab, sizeof(itab));
		for(j=0; j<INOPB*NI; j++) {
			if(ino >= mino)
				break;
			ino++;
			pass2(&itab[j]);
		}
	}
	ino = 0;
	for(i=2;; i+=NI) {
		if(ino >= mino)
			break;
		bread((daddr_t)i, (char *)itab, sizeof(itab));
		for(j=0; j<INOPB*NI; j++) {
			if(ino >= mino)
				break;
			ino++;
			pass3(&itab[j]);
		}
	}
}

pass1(ip)
register struct dinode *ip;
{
	if((ip->di_mode & IFMT) != IFDIR) {
		if (sflg==0 || nxfile>=NB)
			return;
		if ((ip->di_mode&IFMT)==IFBLK || (ip->di_mode&IFMT)==IFCHR
		  || ip->di_mode&(ISUID|ISGID))
			ilist[nxfile++] = ino;
			return;
	}
	lookup(ino, 1);
}

pass2(ip)
register struct dinode *ip;
{
	struct direct dbuf[NDIR];
	long doff;
	struct direct *dp;
	register i, j;
	int k;
	struct htab *hp;
	daddr_t d;
	ino_t kno;

	if((ip->di_mode&IFMT) != IFDIR)
		return;
	l3tol(iaddr, ip->di_addr, NADDR);
	doff = 0;
	for(i=0;; i++) {
		if(doff >= ip->di_size)
			break;
		d = bmap(i);
		if(d == 0)
			break;
		bread(d, (char *)dbuf, sizeof(dbuf));
		for(j=0; j<NDIR; j++) {
			if(doff >= ip->di_size)
				break;
			doff += sizeof(struct direct);
			dp = dbuf+j;
			kno = dp->d_ino;
			if(kno == 0)
				continue;
			hp = lookup(kno, 0);
			if(hp == 0)
				continue;
			if(dotname(dp))
				continue;
			hp->h_pino = ino;
			for(k=0; k<DIRSIZ; k++)
				hp->h_name[k] = dp->d_name[k];
		}
	}
}

pass3(ip)
register struct dinode *ip;
{
	struct direct dbuf[NDIR];
	long doff;
	struct direct *dp;
	register i, j;
	int k;
	daddr_t d;
	ino_t kno;

	if((ip->di_mode&IFMT) != IFDIR)
		return;
	l3tol(iaddr, ip->di_addr, NADDR);
	doff = 0;
	for(i=0;; i++) {
		if(doff >= ip->di_size)
			break;
		d = bmap(i);
		if(d == 0)
			break;
		bread(d, (char *)dbuf, sizeof(dbuf));
		for(j=0; j<NDIR; j++) {
			if(doff >= ip->di_size)
				break;
			doff += sizeof(struct direct);
			dp = dbuf+j;
			kno = dp->d_ino;
			if(kno == 0)
				continue;
			if(aflg==0 && dotname(dp))
				continue;
			if(ilist[0] == 0)
				goto pr;
			for(k=0; ilist[k] != 0; k++)
				if(ilist[k] == kno)
					goto pr;
			continue;
		pr:
			/* printf("%u	", kno); */
			printf(1,"%u	", kno);
			pname(ino, 0);
			/* printf("/%.14s", dp->d_name); */
			strncpy(tmpbuf,dp->d_name,14);
			tmpbuf[14] = NULL;
			printf(1,"/%s", tmpbuf);
			if (lookup(kno, 0))
				/* printf("/."); */
				printf(1,"/.");
			/*printf("\n");*/
			printf(1,"\n");
		}
	}
}

dotname(dp)
register struct direct *dp;
{

	if (dp->d_name[0]=='.')
		if (dp->d_name[1]==0 || (dp->d_name[1]=='.' && dp->d_name[2]==0))
			return(1);
	return(0);
}

pname(i, lev)
ino_t i;
{
	register struct htab *hp;

	if (i==ROOTINO)
		return;
	if ((hp = lookup(i, 0)) == 0) {
		/*printf("???");*/
		printf(1,"???");
		return;
	}
	if (lev > 10) {
		/*printf("...");*/
		printf(1,"...");
		return;
	}
	pname(hp->h_pino, ++lev);
	/*printf("/%.14s", hp->h_name);*/
	strncpy(tmpbuf,hp->h_name,14);
	tmpbuf[14] = NULL;
	printf(1,"/%s", tmpbuf);
}

struct htab *
lookup(i, ef)
ino_t i;
{
	register struct htab *hp;

	for (hp = &htab[i%HSIZE]; hp->h_ino;) {
		if (hp->h_ino==i)
			return(hp);
		if (++hp >= &htab[HSIZE])
			hp = htab;
	}
	if (ef==0)
		return(0);
	if (++nhent >= HSIZE) {
		/*fprintf(stderr, "ncheck: out of core-- increase HSIZE\n");*/
		printf(2, "ncheck: out of core-- increase HSIZE\n");
		exit(1);
	}
	hp->h_ino = i;
	return(hp);
}

bread(bno, buf, cnt)
daddr_t bno;
char *buf;
{
	register i;

	lseek(fi, bno*BSIZE, 0);
	if (read(fi, buf, cnt) != cnt) {
		/*fprintf(stderr, "ncheck: read error %d\n", bno);*/
		printf(2, "ncheck: read error %d\n", bno);
		for(i=0; i<BSIZE; i++)
			buf[i] = 0;
	}
}

daddr_t
bmap(i)
{
	daddr_t ibuf[NINDIR];

	if(i < NADDR-3)
		return(iaddr[i]);
	i -= NADDR-3;
	if(i > NINDIR) {
		/*fprintf(stderr, "ncheck: %u - huge directory\n", ino);*/
		printf(2, "ncheck: %u - huge directory\n", ino);
		return((daddr_t)0);
	}
	bread(iaddr[NADDR-3], (char *)ibuf, sizeof(ibuf));
	return(ibuf[i]);
}
getchar()
{
	char c;
	if (read(0, &c, 1) < 0)
		return(-1);
	else
		return(c);
}
putchar(c,fd)
char c;
int fd;
{
	write(fd, &c, 1);
}

/*
 * Scaled down version of C Library printf.
 * Only %s %u %d (==%u) %o %x %D are recognized.
 */
/* VARARGS 1 */
printf(fd,fmt, x1)
register char *fmt;
unsigned x1;
int fd;
{
	register c;
	register unsigned int *adx;
	char *s;

	adx = &x1;
loop:
	while((c = *fmt++) != '%') {
		if(c == '\0')
			return;
		putchar(c,fd);
	}
	c = *fmt++;
	if(c == 'd' || c == 'u' || c == 'o' || c == 'x')
		printn((long)*adx, c=='o'? 8: (c=='x'? 16:10),fd);
	else if(c == 's') {
		s = (char *)*adx;
		while(c = *s++)
			putchar(c,fd);
	} else if (c == 'c') {
		putchar(*(char *)adx,fd);
	} else if (c == 'D') {
		printn(*(long *)adx, 10,fd);
		adx += (sizeof(long) / sizeof(int)) - 1;
	}
	adx++;
	goto loop;
}

/*
 * Print an unsigned integer in base b.
 */
printn(n, b,fd)
long n;
int fd;
{
	register long a;

	if (n<0) {	/* shouldn't happen */
		putchar('-',fd);
		n = -n;
	}
	if(a = n/b)
		printn(a, b,fd);
	putchar("0123456789ABCDEF"[(int)(n%b)],fd);
}
