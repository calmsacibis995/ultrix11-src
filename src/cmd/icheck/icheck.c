
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * ULTRIX-11 old file system check program (icheck)
 *
 * Modified to allow the standalone version to check
 * dups and salvage the free list for file systems
 * up to approximately 128K blocks in size.
 * This change is required for the Micro/pdp-11.
 * Standalone icheck is used to salvage the free list
 * prior to resuming an interrupted standalone restor
 * at a given volume, other than volume one.
 *
 * Fred Canter 3/6/83
 */

static char Sccsid[] = "@(#)icheck.c 3.0 4/21/86";
#define	NI	16
#define	NB	40
#define	BITS	8
#define	MAXFN	714

#ifndef STANDALONE
#include <stdio.h>
#else
#include "/usr/sys/sas/sa_defs.h"	/* exit status codes for SDLOAD */
#endif
#include <sys/param.h>
#include <sys/inode.h>
#include <sys/ino.h>
#include <sys/fblk.h>
#include <sys/filsys.h>

struct	filsys	sblock;
struct	dinode	itab[INOPB*NI];
daddr_t	iaddr[NADDR];
daddr_t	blist[NB+1];
char	*bmap;
/*
 * Standalone buffer used for checking dups
 * and salvaging the free list.
 */
#ifdef	STANDALONE
#define	SABUFSIZ 16384
char	sabuf[SABUFSIZ];
#endif

int	sflg;
int	mflg;
int	dflg;
int	fi;
ino_t	ino;

ino_t	nrfile;
ino_t	ndfile;
ino_t	nbfile;
ino_t	ncfile;
ino_t	npfile;
#ifdef	UCB_SYMLINKS
ino_t	nlfile;
#endif	UCB_SYMLINKS

daddr_t	ndirect;
daddr_t	nindir;
daddr_t	niindir;
daddr_t	niiindir;
daddr_t	nfree;
daddr_t	ndup;

int	nerror;

long	atol();
daddr_t	alloc();
#ifndef STANDALONE
char	*malloc();
#endif

main(argc, argv)
char *argv[];
{
	register i;
	long n;

	blist[0] = -1;
#ifndef STANDALONE
	while (--argc) {
		argv++;
		if (**argv=='-')
		switch ((*argv)[1]) {
		case 'd':
			dflg++;
			continue;


		case 'm':
			mflg++;
			continue;

		case 's':
			sflg++;
			continue;

		case 'b':
			for(i=0; i<NB; i++) {
				n = atol(argv[1]);
				if(n == 0)
					break;
				blist[i] = n;
				argv++;
				argc--;
			}
			blist[i] = -1;
			continue;

		default:
			printf("Bad flag\n");
		}
		check(*argv);
	}
	return(nerror);
#else
	{
		static char fname[60];
		static char yn[60];

		printf("File: ");
		gets(fname);
		printf("Salvage free list <y or n> ? ");
		gets(yn);
		if((yn[0] == 'y') && (yn[1] == 0))
			sflg++;
		check(fname);
	}
	if(nerror)
		exit(FATAL);
#endif
}

check(file)
char *file;
{
	register i, j;
	ino_t mino;
	daddr_t d;
	long n;

	fi = open(file, sflg?2:0);
	if (fi < 0) {
		printf("cannot open %s\n", file);
#ifdef STANDALONE
		nerror = 1;
#else
		nerror |= 04;
#endif
		return;
	}
	printf("%s:\n", file);
	nrfile = 0;
	ndfile = 0;
	ncfile = 0;
	nbfile = 0;
	npfile = 0;
#ifdef	UCB_SYMLINKS
	nlfile = 0;
#endif	UCB_SYMLINKS

	ndirect = 0;
	nindir = 0;
	niindir = 0;
	niiindir = 0;

	ndup = 0;
#ifndef STANDALONE
	sync();
#endif
	bread((daddr_t)1, (char *)&sblock, sizeof(sblock));
	mino = (sblock.s_isize-2) * INOPB;
	ino = 0;
	n = (sblock.s_fsize - sblock.s_isize + BITS-1) / BITS;
	if (n != (unsigned)n) {
		printf("Check fsize and isize: %D, %u\n",
		   sblock.s_fsize, sblock.s_isize);
#ifdef STANDALONE
		nerror = 1;
#endif
	}
#ifdef STANDALONE
	if(n <= SABUFSIZ)
		bmap = &sabuf;
	else
		bmap = NULL;
#else
	bmap = malloc((unsigned)n);
#endif
	if (bmap==NULL) {
		printf("Not enough core; duplicates unchecked\n");
		dflg++;
		sflg = 0;
	}
	if(!dflg)
	for(i=0; i<(unsigned)n; i++)
		bmap[i] = 0;
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
	ino = 0;
#ifndef STANDALONE
	sync();
#endif
	bread((daddr_t)1, (char *)&sblock, sizeof(sblock));
	if (sflg) {
		makefree();
		close(fi);
#ifndef STANDALONE
		if (bmap)
			free(bmap);
#endif
		return;
	}
	nfree = 0;
	while(n = alloc()) {
		if (chk(n, "free"))
			break;
		nfree++;
	}
	close(fi);
#ifndef STANDALONE
	if (bmap)
		free(bmap);
#endif

	i = nrfile + ndfile + ncfile + nbfile + npfile;
#ifdef	UCB_SYMLINKS
	i += nlfile;
#endif	UCB_SYMLINKS
#ifndef STANDALONE
#ifdef	UCB_SYMLINKS
	printf("files %6u (r=%u,d=%u,b=%u,c=%u,p=%u,l=%u)\n",
		i, nrfile, ndfile, nbfile, ncfile, npfile, nlfile);
#else	UCB_SYMLINKS
	printf("files %6u (r=%u,d=%u,b=%u,c=%u,p=%u)\n",
		i, nrfile, ndfile, nbfile, ncfile, npfile);
#endif	UCB_SYMLINKS
#else
#ifdef	UCB_SYMLINKS
	printf("files %u (r=%u,d=%u,b=%u,c=%u,p=%u,l=%u)\n",
		i, nrfile, ndfile, nbfile, ncfile, npfile, nlfile);
#else	UCB_SYMLINKS
	printf("files %u (r=%u,d=%u,b=%u,c=%u,p=%u)\n",
		i, nrfile, ndfile, nbfile, ncfile, npfile);
#endif	UCB_SYMLINKS
#endif
	n = ndirect + nindir + niindir + niindir;
#ifdef STANDALONE
	printf("used %D (i=%D,ii=%D,iii=%D,d=%D)\n",
		n, nindir, niindir, niiindir, ndirect);
	printf("free %D\n", nfree);
#else
	printf("used %7D (i=%D,ii=%D,iii=%D,d=%D)\n",
		n, nindir, niindir, niiindir, ndirect);
	printf("free %7D\n", nfree);
#endif
	if(!dflg) {
		n = 0;
		for(d=sblock.s_isize; d<sblock.s_fsize; d++)
			if(!duped(d)) {
				if(mflg)
					printf("%D missing\n", d);
				n++;
			}
#ifndef STANDALONE
		printf("missing%5D\n", n);
#else
		printf("missing %D\n", n);
		if(n)
			nerror = 1;
#endif
	}
}

pass1(ip)
register struct dinode *ip;
{
	daddr_t ind1[NINDIR];
	daddr_t ind2[NINDIR];
	daddr_t ind3[NINDIR];
	register i, j;
	int k, l;

	i = ip->di_mode & IFMT;
	if(i == 0)
		return;
	if(i == IFIFO) {
		npfile++;
		return;
	}
	if(i == IFCHR) {
		ncfile++;
		return;
	}
	if(i == IFBLK) {
		nbfile++;
		return;
	}
	if(i == IFDIR)
		ndfile++; else
#ifdef	UCB_SYMLINKS
	if(i == IFLNK)
		nlfile++; else
#endif	UCB_SYMLINKS
	if(i == IFREG)
		nrfile++;
	else {
		printf("bad mode %u\n", ino);
#ifdef STANDALONE
		nerror = 1;
#endif
		return;
	}
	l3tol(iaddr, ip->di_addr, NADDR);
	for(i=0; i<NADDR; i++) {
		if(iaddr[i] == 0)
			continue;
		if(i < NADDR-3) {
			ndirect++;
			chk(iaddr[i], "data (small)");
			continue;
		}
		nindir++;
		if (chk(iaddr[i], "1st indirect"))
				continue;
		bread(iaddr[i], (char *)ind1, BSIZE);
		for(j=0; j<NINDIR; j++) {
			if(ind1[j] == 0)
				continue;
			if(i == NADDR-3) {
				ndirect++;
				chk(ind1[j], "data (large)");
				continue;
			}
			niindir++;
			if(chk(ind1[j], "2nd indirect"))
				continue;
			bread(ind1[j], (char *)ind2, BSIZE);
			for(k=0; k<NINDIR; k++) {
				if(ind2[k] == 0)
					continue;
				if(i == NADDR-2) {
					ndirect++;
					chk(ind2[k], "data (huge)");
					continue;
				}
				niiindir++;
				if(chk(ind2[k], "3rd indirect"))
					continue;
				bread(ind2[k], (char *)ind3, BSIZE);
				for(l=0; l<NINDIR; l++)
					if(ind3[l]) {
						ndirect++;
						chk(ind3[l], "data (garg)");
					}
			}
		}
	}
}

chk(bno, s)
daddr_t bno;
char *s;
{
	register n;

	if (bno<sblock.s_isize || bno>=sblock.s_fsize) {
		printf("%D bad; inode=%u, class=%s\n", bno, ino, s);
#ifdef STANDALONE
		nerror =1;
#endif
		return(1);
	}
	if(duped(bno)) {
		printf("%D dup; inode=%u, class=%s\n", bno, ino, s);
		ndup++;
#ifdef STANDALONE
		nerror = 1;
#endif
	}
	for (n=0; blist[n] != -1; n++)
		if (bno == blist[n]) {
			printf("%D arg; inode=%u, class=%s\n", bno, ino, s);
#ifdef	STANDALONE
			nerror = 1;
#endif
		}
	return(0);
}

duped(bno)
daddr_t bno;
{
	daddr_t d;
	register m, n;

	if(dflg)
		return(0);
	d = bno - sblock.s_isize;
	m = 1 << (d%BITS);
	n = (d/BITS);
	if(bmap[n] & m)
		return(1);
	bmap[n] |= m;
	return(0);
}

daddr_t
alloc()
{
	int i;
	daddr_t bno;
	union {
		char	data[BSIZE];
		struct	fblk fb;
	} buf;

	sblock.s_tfree--;
	if (sblock.s_nfree<=0)
		return(0);
	if (sblock.s_nfree>NICFREE) {
		printf("Bad free list, s.b. count = %d\n", sblock.s_nfree);
#ifdef STANDALONE
		nerror = 1;
#endif
		return(0);
	}
	bno = sblock.s_free[--sblock.s_nfree];
	sblock.s_free[sblock.s_nfree] = (daddr_t)0;
	if(bno == 0)
		return(bno);
	if(sblock.s_nfree <= 0) {
		bread(bno, buf.data, BSIZE);
		sblock.s_nfree = buf.fb.df_nfree;
		if (sblock.s_nfree<0 || sblock.s_nfree>NICFREE) {
			printf("Bad free list, entry count of block %D = %d\n",
				bno, sblock.s_nfree);
			sblock.s_nfree = 0;
#ifdef STANDALONE
			nerror = 1;
#endif
			return(0);
		}
		for(i=0; i<NICFREE; i++)
			sblock.s_free[i] = buf.fb.df_free[i];
	}
	return(bno);
}

bfree(bno)
daddr_t bno;
{
	union {
		char	data[BSIZE];
		struct	fblk fb;
	} buf;
	int i;

	if(bno != 0)
		sblock.s_tfree++;
	if(sblock.s_nfree >= NICFREE) {
		for(i=0; i<BSIZE; i++)
			buf.data[i] = 0;
		buf.fb.df_nfree = sblock.s_nfree;
		for(i=0; i<NICFREE; i++)
			buf.fb.df_free[i] = sblock.s_free[i];
		bwrite(bno, buf.data);
		sblock.s_nfree = 0;
	}
	sblock.s_free[sblock.s_nfree] = bno;
	sblock.s_nfree++;
}

bread(bno, buf, cnt)
daddr_t bno;
char *buf;
{
	register i;

	lseek(fi, bno*BSIZE, 0);
	if (read(fi, buf, cnt) != cnt) {
		printf("read error %D\n", bno);
#ifdef STANDALONE
		nerror = 1;
#endif
		if (sflg) {
			printf("No update\n");
			sflg = 0;
		}
		for(i=0; i<BSIZE; i++)
			buf[i] = 0;
	}
}

bwrite(bno, buf)
daddr_t bno;
char	*buf;
{

	lseek(fi, bno*BSIZE, 0);
	if (write(fi, buf, BSIZE) != BSIZE) {
		printf("write error %D\n", bno);
#ifdef STANDALONE
		nerror = 1;
#endif
	}
}

makefree()
{
	char flg[MAXFN];
	int adr[MAXFN];
	register i, j;
	daddr_t f, d;
	int m, n;

	n = sblock.s_n;
	if(n <= 0 || n > MAXFN)
		n = MAXFN;
	sblock.s_n = n;
	m = sblock.s_m;
	if(m <= 0 || m > sblock.s_n)
		m = 9;
	sblock.s_m = m;

	for(i=0; i<n; i++)
		flg[i] = 0;
	i = 0;
	for(j=0; j<n; j++) {
		while(flg[i])
			i = (i+1)%n;
		adr[j] = i+1;
		flg[i]++;
		i = (i+m)%n;
	}

	sblock.s_nfree = 0;
	sblock.s_ninode = 0;
	sblock.s_flock = 0;
	sblock.s_ilock = 0;
	sblock.s_fmod = 0;
	sblock.s_ronly = 0;
#ifndef STANDALONE
	time(&sblock.s_time);
#endif
	sblock.s_tfree = 0;

	bfree((daddr_t)0);
	d = sblock.s_fsize-1;
	if(d % sblock.s_n == 0)
		d++;	/* so last block will not be missing (see mkfs.c) */
	while(d%sblock.s_n)
		d++;
	for(; d > 0; d -= sblock.s_n)
	for(i=0; i<sblock.s_n; i++) {
		f = d - adr[i];
		if(f < sblock.s_fsize && f >= sblock.s_isize)
			if(!duped(f))
				bfree(f);
	}
	bwrite((daddr_t)1, (char *)&sblock);
#ifndef STANDALONE
	sync();
#endif
	return;
}
