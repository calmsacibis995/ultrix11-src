
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * ULTRIX-11 file system restore program (restor)
 *
 * Modified for use with the Micro/pdp-11.
 * The m option restores from RX50 diskettes.
 * The e option restores from RX33 diskettes.
 *
 * Fred Canter 5/25/83

 * 512restor program: compiled by defining REST_512,
 * used to restore from 512-byte block file system.
 * George Mathew 6/11/85
 *
 * New option (s file#) to restore individual files if there
 * is more than one dump file on a tape. It uses tape ioctls
 * to position the tape correctly.
 * George Mathew 9/4/85
 *
 * Modifications to restart restor with any floppy in the
 * middle for STANDALONE version only.
 * George Mathew 11/11/85
 */

#ifndef STANDALONE
static char Sccsid[] = "@(#)restor.c 3.2 7/11/87";
#endif STANDALONE
#ifdef	RESTOR40
#define	MAXINO	2000
#else
#define MAXINO	3000
#endif

#define BITS	8
#define MAXXTR	60
#define NCACHE	3

#ifndef STANDALONE
#define	NORMAL	0
#define	FATAL	1
#include <stdio.h>
#include <signal.h>
#else
#include "/usr/sys/sas/sa_defs.h"	/* exit status codes for SDLOAD */
#endif
#include <sys/param.h>
#include <sys/inode.h>
#include <sys/ino.h>
#include <sys/fblk.h>
#include <sys/filsys.h>
#include <sys/dir.h>
#include <sys/mtio.h>
#include <dumprestor.h>


#ifdef REST_512        /* 512restor */

/* from old ino.h */

#define	OINOPB	8	/* 8 inodes per block */

#endif REST_512

#define	MWORD(m,i) (m[(unsigned)(i-1)/MLEN])
#define	MBIT(i)	(1<<((unsigned)(i-1)%MLEN))
#define	BIS(i,w)	(MWORD(w,i) |=  MBIT(i))
#define	BIC(i,w)	(MWORD(w,i) &= ~MBIT(i))
#define	BIT(i,w)	(MWORD(w,i) & MBIT(i))

struct	filsys	sblock;

int	fi;
ino_t	ino, maxi, curino;

int	mt;
int	trn;
int	rx_trn;		/* max # of 10240 byte blocks on floppy */
char	tapename[] = "/dev/rht0";
char	*magtape = tapename;
char	*nota = "Media is not a dump volume\n";
char	*notv = "Media is not volume ";
char	*eodm = "End of dump media\n";
char	*prtbrt = "Press <RETURN> to begin restoring to";
#ifdef STANDALONE
char	mbuf[30];
char	vbuf[30];
#endif

#ifndef STANDALONE
daddr_t	seekpt;
int	df, ofile;
char	dirfile[] = "rstXXXXXX";

struct {
	ino_t	t_ino;
	daddr_t	t_seekpt;
} inotab[MAXINO];
int	ipos;

#define ONTAPE	1
#define XTRACTD	2
#define XINUSE	4
struct xtrlist {
	ino_t	x_ino;
	char	x_flags;
} xtrlist[MAXXTR];

char	name[12];

char	drblock[BSIZE];
int	bpt;
#endif

int	eflag;
int	mflag;
int	rflag;	/* load device is RC25 or RL02 */

int	volno = 1;

#ifdef STANDALONE
int	restart = 0;	/* flag indicating restarted from within the process */
#endif

struct dinode tino, dino;
daddr_t	taddr[NADDR];

daddr_t	curbno;

short	dumpmap[MSIZ];
short	clrimap[MSIZ];


int bct = NTREC+1;

#ifdef REST_512
char tbf[NTREC*OBSIZE];
#else
char tbf[NTREC*BSIZE];
#endif

struct	cache {
	daddr_t	c_bno;
	int	c_time;
	char	c_block[BSIZE];
} cache[NCACHE];
int	curcache;

#ifdef STANDALONE
int	argflag;	/* 0=interactive, 1=called by SDLOAD, see prf.c */
int	afstat;		/* save argflag during volume change */
#endif
int	seqno = 0;	/* file number if more than 1 restored on a single vol */
struct  mtop mtop;

main(argc, argv)
char *argv[];
{
	register char *cp;
	char command;
	int done();

#ifndef STANDALONE
	mktemp(dirfile);
	if (argc < 2) {
usage:
		printf("Usage: restor x file file.., restor r filesys, or restor t\n");
		exit(FATAL);
	}
	argv++;
	argc -= 2;
	for (cp = *argv++; *cp; cp++) {
		switch (*cp) {
		case '-':
			break;
		case 'f':
			magtape = *argv++;
			argc--;
			break;
		case 'r':
		case 'R':
		case 't':
		case 'x':
			command = *cp;
			break;
		case 's':
			if ((seqno = atoi(*argv++)) <= 0) {
				printf("file number must be > 0\n");
				goto usage;
			}
			argc--;
			break;
		case 'm':		/* restore from RX50 diskettes */
			mflag++;
			rx_trn = 40;
			break;
		case 'e':		/* restore from RX33 diskettes */
			mflag++;
			rx_trn = 120;
			break;
		default:
			printf("Bad key character %c\n", *cp);
			goto usage;
		}
	}
	if (command == 'x') {
		if (signal(SIGINT, done) == SIG_IGN)
			signal(SIGINT, SIG_IGN);
		if (signal(SIGTERM, done) == SIG_IGN)
			signal(SIGTERM, SIG_IGN);

		df = creat(dirfile, 0666);
		if (df < 0) {
			printf("%s - can't create directory temporary\n", dirfile);
			exit(FATAL);
		}
		close(df);
		df = open(dirfile, 2);
	}
	doit(command, argc, argv);
	/* if (command == 'x')
		unlink(dirfile);
	exit(NORMAL); */
	rstexit(NORMAL);
#else
	magtape = "tape";
	doit('r', 1, 0);
#endif
}

doit(command, argc, argv)
char	command;
int	argc;
char	*argv[];
{
	extern char *ctime();
	register i, k;
	ino_t	d;
#ifndef STANDALONE
	int	xtrfile(), skip();
#endif
	int	rstrfile(), rstrskip();
	struct dinode *ip, *ip1;
	int ret;

#ifndef STANDALONE
	if ((mt = open(magtape, 0)) < 0) {
		printf("%s: can't open\n", magtape);
		rstexit(FATAL);
	}
	if (seqno > 0) {
		mtop.mt_op = MTFSF;
		mtop.mt_count = seqno - 1;
		if (ioctl(mt,MTIOCTOP,&mtop) < 0) {
			printf("cannot position tape at file %d\n",seqno);
			rstexit(FATAL);
		}
	}
#else
	if(!restart) {
		do {
			printf("Input: ");
			gets(mbuf);
			mt = open(mbuf, 0);
			if(argflag && (mt == -1))
				rstexit(FATAL);
		} while (mt == -1);
		if((mbuf[0] == 'r') && ((mbuf[1] == 'a') || (mbuf[1] == 'x')))
		{
		    while (1) {
			printf("Diskette type <33 or 50>: ");
			gets(vbuf);
			i = atoi(vbuf);
			if(i == 50) {
			    rx_trn = 40;
			    break;
			} else if(i == 33) {
			    rx_trn = 120;
			    break;
			} else
			    continue;
		    }
		    mflag++;		/* restore is from RX50/RX33 */
		}
		if((mbuf[0] == 'r') && ((mbuf[1] == 'l') || (mbuf[1] == 'c')))
			rflag++;	/* restore from RC25/RL02 */
		magtape = mbuf;
		if(mflag) {
			printf("Starting volume number <1>: ");
			gets(vbuf);
			if(vbuf[0] == 0)
				volno = 1;
			else
				volno = atoi(vbuf);
		}
	}
#endif
	switch(command) {
#ifndef STANDALONE
	case 't':
		if (readhdr(&spcl) == 0) {
			printf("%s", nota); /* not a dump volume */
			rstexit(FATAL);
		}
		printf("Dump   date: %s", ctime(&spcl.c_date));
		printf("Dumped from: %s", ctime(&spcl.c_ddate));
		return;
	case 'x':
		if (readhdr(&spcl) == 0) {
			printf("%s", nota); /* not a dump volume */
			rstexit(FATAL);
		}
		if (checkvol(&spcl, 1) == 0) {
			printf("%s1 of the dump\n", notv);
			rstexit(FATAL);
		}
		pass1();  /* This sets the various maps on the way by */
		i = 0;
		while (i < MAXXTR-1 && argc--) {
			if ((d = psearch(*argv)) == 0 || BIT(d, dumpmap) == 0) {
				printf("%s: not on the volume\n", *argv++);
				continue;
			}
			xtrlist[i].x_ino = d;
			xtrlist[i].x_flags |= XINUSE;
			printf("%s: inode %u\n", *argv, d);
			argv++;
			i++;
		}
newvol:
		flsht();
		if (seqno > 0) {
			filcntl();
		} else
			close(mt);
		trn = 0;	/* fake rewind RX50/RX33 diskette */
getvol:
		if (seqno == 0) {
			printf("Mount desired volume: Specify volume #: ");
			fflush(stdout);
			if (gets(tbf) == NULL)
				return;
			volno = atoi(tbf);
			if (volno <= 0) {
				printf("Volume number should be > 0\n");
				goto getvol;
			}
			mt = open(magtape, 0);
		}
		if (readhdr(&spcl) == 0) {
			printf("%s", nota); /* not a dump volume */
			goto newvol;
		}
		if (seqno == 0) {
			if (checkvol(&spcl, volno) == 0) {
				printf("Wrong volume (%d)\n", spcl.c_volume);
				goto newvol;
			}
		}
rbits:
		while (gethead(&spcl) == 0)
			;
		if (checktype(&spcl, TS_INODE) == 1) {
			printf("Can't find inode mask!\n");
			goto newvol;
		}
		if (checktype(&spcl, TS_BITS) == 0)
			goto rbits;
		readbits(dumpmap);
		i = 0;
		for (k = 0; xtrlist[k].x_flags; k++) {
			if (BIT(xtrlist[k].x_ino, dumpmap)) {
				xtrlist[k].x_flags |= ONTAPE;
				i++;
			}
		}
		while (i > 0) {
again:
			if (ishead(&spcl) == 0)
				while(gethead(&spcl) == 0)
					;
			if (checktype(&spcl, TS_END) == 1) {
				printf("%s", eodm);
checkdone:
				for (k = 0; xtrlist[k].x_flags; k++)
					if ((xtrlist[k].x_flags&XTRACTD) == 0)
						goto newvol;
					return;
			}
			if (checktype(&spcl, TS_INODE) == 0) {
				gethead(&spcl);
				goto again;
			}
			d = spcl.c_inumber;
			for (k = 0; xtrlist[k].x_flags; k++) {
				if (d == xtrlist[k].x_ino) {
					printf("extract file %u\n", xtrlist[k].x_ino);
					sprintf(name, "%u", xtrlist[k].x_ino);
					if ((ofile = creat(name, 0666)) < 0) {
						printf("%s: can't create file\n", name);
						i--;
						continue;
					}
					chown(name, spcl.c_dinode.di_uid, spcl.c_dinode.di_gid);
					getfile(ino, xtrfile, skip, spcl.c_dinode.di_size);
					i--;
					xtrlist[k].x_flags |= XTRACTD;
					close(ofile);
					goto done;
				}
			}
			gethead(&spcl);
done:
			;
		}
		goto checkdone;
#endif
	case 'r':
	case 'R':
#ifndef STANDALONE
		if ((fi = open(*argv, 2)) < 0) {
			printf("%s: can't open\n", *argv);
			rstexit(FATAL);
		}
#else
		if(!restart) {
			do {
				char charbuf[50];
	
				printf("Disk: ");
				gets(charbuf);
				fi = open(charbuf, 2);
				if(argflag && (fi == -1))
					rstexit(FATAL);
			} while (fi == -1);
		}
#endif
#ifndef STANDALONE
		if (command == 'R') {
			printf("Enter starting volume number: ");
			fflush(stdout);
			if (gets(tbf) == EOF) {
				volno = 1;
				printf("\n");
			}
			else
				volno = atoi(tbf);
		}
		else
#endif
			if(mflag == 0)
				volno = 1;
#ifdef STANDALONE
		if(!restart) {
			if(argflag == 0)
				printf("%s disk. ", prtbrt);
		} else {
			printf("Restarting: Mount volume %d <type return when ready>",volno);
			/* make sure we are interactive, user hits return */
			afstat = argflag;
			argflag = 0;
		}
#else
		printf("%s %s. ", prtbrt, *argv);
		fflush(stdout);
#endif
		while (getchar() != '\n');
#ifdef STANDALONE
		if(restart)
		{
			argflag = afstat;
			mt = open(magtape,0);
		}
#endif
		dread((daddr_t)1, (char *)&sblock, sizeof(sblock));
		maxi = (sblock.s_isize-2)*INOPB;
		if (readhdr(&spcl) == 0) {
			printf("Missing volume record\n");
			rstexit(FATAL);
		}
		if (checkvol(&spcl, volno) == 0) {
			printf("%s%d\n", notv, volno);
			rstexit(FATAL);
		}
		gethead(&spcl);
		for (;;) {
ragain:
			if (ishead(&spcl) == 0) {
#ifdef STANDALONE
				if(!restart) {
					printf("Missing header block\n");
					printf("type:%d\n",spcl.c_type);
				}
#else
				printf("Missing header block\n");
				printf("type:%d\n",spcl.c_type);
#endif
				while (gethead(&spcl) == 0)
					;
				eflag++;
			}
			if (checktype(&spcl, TS_END) == 1) {
				printf("%s", eodm);
				close(mt);
				dwrite( (daddr_t) 1, (char *) &sblock);
				return;
			}
			if (checktype(&spcl, TS_CLRI) == 1) {
				readbits(clrimap);
				for (ino = 1; ino <= maxi; ino++)
					if (BIT(ino, clrimap) == 0) {
						getdino(ino, &tino);
						if (tino.di_mode == 0)
							continue;
						itrunc(&tino);
						clri(&tino);
						putdino(ino, &tino);
					}
				dwrite( (daddr_t) 1, (char *) &sblock);
				goto ragain;
			}
			if (checktype(&spcl, TS_BITS) == 1) {
				readbits(dumpmap);
				goto ragain;
			}
			if (checktype(&spcl, TS_INODE) == 0) {
#ifdef STANDALONE
				if(!restart)
					printf("Unknown header type\n");
#else
				printf("Unknown header type\n");
#endif
				eflag++;
				gethead(&spcl);
				goto ragain;
			}
			ino = spcl.c_inumber;
			if (eflag)
				printf("Resynced at inode %u\n", ino);
			eflag = 0;
			if (ino > maxi) {
				printf("%u: ilist too small\n", ino);
				gethead(&spcl);
				goto ragain;
			}
			dino = spcl.c_dinode;
			getdino(ino, &tino);
			curbno = 0;
			itrunc(&tino);
			clri(&tino);
			for (i = 0; i < NADDR; i++)
				taddr[i] = 0;
			l3tol(taddr, dino.di_addr, 1);
			getfile(ino, rstrfile, rstrskip, dino.di_size);
			ip = &tino;
			ltol3(ip->di_addr, taddr, NADDR);
			ip1 = &dino;
			ip->di_mode = ip1->di_mode;
			ip->di_nlink = ip1->di_nlink;
			ip->di_uid = ip1->di_uid;
			ip->di_gid = ip1->di_gid;
			ip->di_size = ip1->di_size;
			ip->di_atime = ip1->di_atime;
			ip->di_mtime = ip1->di_mtime;
			ip->di_ctime = ip1->di_ctime;
			putdino(ino, &tino);
		}
	}
}

/*
 * Read the tape, bulding up a directory structure for extraction
 * by name
 */
#ifndef STANDALONE
pass1()
{
	register i;
	struct dinode *ip;
	int	putdir(), null();

	while (gethead(&spcl) == 0) {
		printf("Can't find directory header!\n");
	}
	for (;;) {
		if (checktype(&spcl, TS_BITS) == 1) {
			readbits(dumpmap);
			continue;
		}
		if (checktype(&spcl, TS_CLRI) == 1) {
			readbits(clrimap);
			continue;
		}
		if (checktype(&spcl, TS_INODE) == 0) {
finish:
			flsh();
			if (seqno > 0) {
				filcntl();
			} else
				close(mt);
			return;
		}
		ip = &spcl.c_dinode;
		i = ip->di_mode & IFMT;
		if (i != IFDIR) {
			goto finish;
		}
		inotab[ipos].t_ino = spcl.c_inumber;
		inotab[ipos++].t_seekpt = seekpt;
		getfile(spcl.c_inumber, putdir, null, spcl.c_dinode.di_size);
		putent("\000\000/");
	}
}
#endif

/*
 * Do the file extraction, calling the supplied functions
 * with the blocks
 */
getfile(n, f1, f2, size)
ino_t	n;
int	(*f2)(), (*f1)();
long	size;
{
	register i;
	struct spcl addrblock;
	char buf[BSIZE];

	addrblock = spcl;
	curino = n;
	goto start;
	for (;;) {
		if (gethead(&addrblock) == 0) {
			printf("Missing address (header) block\n");
			goto eloop;
		}
		if (checktype(&addrblock, TS_ADDR) == 0) {
			spcl = addrblock;
			curino = 0;
			return;
		}
start:

#ifdef REST_512
		
		for (i = 0; i < addrblock.c_count; i += 2) {
			if (addrblock.c_addr[i])
				readtape(buf, 0);
			else
				clearbuf(buf, 0);
			if (size > OBSIZE && addrblock.c_addr[i+1])
				readtape(buf, 1);
			else
				clearbuf(buf, 1);
			if (addrblock.c_addr[i] || size > OBSIZE && addrblock.c_addr[i + 1])
				(*f1)(buf, size > BSIZE ? (long) BSIZE : size);
			else
				(*f2)(buf, size > BSIZE ? (long) BSIZE : size);
#else
		for (i = 0; i < addrblock.c_count; i++) {
			if (addrblock.c_addr[i]) {
				readtape(buf);
				(*f1)(buf, size > BSIZE ? (long) BSIZE : size);
			}
			else {
				clearbuf(buf);
				(*f2)(buf, size > BSIZE ? (long) BSIZE : size);
			}
#endif REST_512
			if ((size -= BSIZE) <= 0) {
eloop:
				while (gethead(&spcl) == 0)
					;
				if (checktype(&spcl, TS_ADDR) == 1)
					goto eloop;
				curino = 0;
				return;
			}
		}
	}
}

/*
 * Do the tape i/o, dealling with volume changes
 * etc..
 */
long	boff;
#ifdef REST_512
readtape(b,part)
#else
readtape(b)
#endif
char *b;
{
	register i;
	struct spcl tmpbuf;

	if (bct >= NTREC) {
		for (i = 0; i < NTREC; i++)
#ifdef REST_512
			((struct spcl *)&tbf[i*OBSIZE])->c_magic = 0;
#else
			((struct spcl *)&tbf[i*BSIZE])->c_magic = 0;
#endif
		bct = 0;
		if(mflag || rflag) {	/* restoring from a disk */
		    if(mflag) {		/* RX50/RX33 restore */
			if(trn >= rx_trn)
				goto newvol;
		    }
		    boff = (long) trn;
#ifdef REST_512
		    boff *= (long)(NTREC*OBSIZE);
#else
		    boff *= (long)(NTREC*BSIZE);
#endif
		    lseek(mt, boff, 0);
		}
#ifdef REST_512
		if ((i = read(mt, tbf, NTREC*OBSIZE)) < 0) {
#else
		if ((i = read(mt, tbf, NTREC*BSIZE)) < 0) {
#endif
			printf("Input read error: inode %u\n", curino);
			eflag++;
#ifdef STANDALONE
			if (mflag)  {
				if (volno >1 )
					volno -= 1;
				else
					rstexit(FATAL);
				if (++restart > 1) {
					printf("Exceeded 1 restart. Exit\n");
					rstexit(FATAL);
				}
				if( ichecks())
					rstexit(FATAL);
				close(mt);
				trn = 0;
				bct = NTREC +1;
				doit('r',1,0);
				rstexit(NORMAL);
			}
#endif
#ifdef REST_512
			rstexit(FATAL);
#else
			for (i = 0; i < NTREC; i++)
				clearbuf(&tbf[i*BSIZE]);
#endif
		}
		trn++;
		if(i == 0) {
newvol:
			bct = NTREC + 1;
			volno++;
			trn = 0;
loop:
			flsht();
			close(mt);
			printf("Mount volume %d <type return when ready>", volno);
#ifdef STANDALONE
			/* make sure we are interactive, user hits return */
			afstat = argflag;
			argflag = 0;
#else
			fflush(stdout);
#endif
			while (getchar() != '\n')
				;
#ifdef STANDALONE
			argflag = afstat;
#endif
			if ((mt = open(magtape, 0)) == -1) {
				printf("Can't open volume!\n");
				goto loop;
			}
			if (readhdr(&tmpbuf) == 0) {
				printf("Not a dump volume. Try again\n");
				goto loop;
			}
			if (checkvol(&tmpbuf, volno) == 0) {
				printf("Wrong volume. Try again\n");
				goto loop;
			}
#ifdef REST_512
			readtape(b, part);
#else
			readtape(b);
#endif
			return;
		}
	}
#ifdef REST_512
	copy(&tbf[(bct++*OBSIZE)], b + part * OBSIZE, OBSIZE);
#else
	copy(&tbf[(bct++*BSIZE)], b, BSIZE);
#endif
}

flsht()
{
	bct = NTREC+1;
}

copy(f, t, s)
register char *f, *t;
{
	register i;

	i = s;
	do
		*t++ = *f++;
	while (--i);
}

#ifdef REST_512
clearbuf(cp, part)
#else
clearbuf(cp)
#endif
register char *cp;
{
	register i;

#ifdef REST_512
	cp += part * OBSIZE;
	i = OBSIZE;
#else
	i = BSIZE;
#endif
	do
		*cp++ = 0;
	while (--i);
}

/*
 * Put and get the directory entries from the compressed
 * directory file
 */
#ifndef STANDALONE
putent(cp)
char	*cp;
{
	register i;

	for (i = 0; i < sizeof(ino_t); i++)
		writec(*cp++);
	for (i = 0; i < DIRSIZ; i++) {
		writec(*cp);
		if (*cp++ == 0)
			return;
	}
	return;
}

getent(bf)
register char *bf;
{
	register i;

	for (i = 0; i < sizeof(ino_t); i++)
		*bf++ = readc();
	for (i = 0; i < DIRSIZ; i++)
		if ((*bf++ = readc()) == 0)
			return;
	return;
}

/*
 * read/write te directory file
 */
writec(c)
char c;
{
	drblock[bpt++] = c;
	seekpt++;
	if (bpt >= BSIZE) {
		bpt = 0;
		write(df, drblock, BSIZE);
	}
}

readc()
{
	if (bpt >= BSIZE) {
		read(df, drblock, BSIZE);
		bpt = 0;
	}
	return(drblock[bpt++]);
}

mseek(pt)
daddr_t pt;
{
	bpt = BSIZE;
	lseek(df, pt, 0);
}

flsh()
{
	write(df, drblock, bpt+1);
}

/*
 * search the directory inode ino
 * looking for entry cp
 */
ino_t
search(inum, cp)
ino_t	inum;
char	*cp;
{
	register i;
	struct direct dir;

	for (i = 0; i < MAXINO; i++)
		if (inotab[i].t_ino == inum) {
			goto found;
		}
	return(0);
found:
	mseek(inotab[i].t_seekpt);
	do {
		getent((char *)&dir);
		if (direq(dir.d_name, "/"))
			return(0);
	} while (direq(dir.d_name, cp) == 0);
	return(dir.d_ino);
}

/*
 * Search the directory tree rooted at inode 2
 * for the path pointed at by n
 */
psearch(n)
char	*n;
{
	register char *cp, *cp1;
	char c;

	ino = 2;
	if (*(cp = n) == '/')
		cp++;
next:
	cp1 = cp + 1;
	while (*cp1 != '/' && *cp1)
		cp1++;
	c = *cp1;
	*cp1 = 0;
	ino = search(ino, cp);
	if (ino == 0) {
		*cp1 = c;
		return(0);
	}
	*cp1 = c;
	if (c == '/') {
		cp = cp1+1;
		goto next;
	}
	return(ino);
}

direq(s1, s2)
register char *s1, *s2;
{
	register i;

	for (i = 0; i < DIRSIZ; i++)
		if (*s1++ == *s2) {
			if (*s2++ == 0)
				return(1);
		} else
			return(0);
	return(1);
}
#endif

/*
 * read/write a disk block, be sure to update the buffer
 * cache if needed.
 */
dwrite(bno, b)
daddr_t	bno;
char	*b;
{
	register i;

	for (i = 0; i < NCACHE; i++) {
		if (cache[i].c_bno == bno) {
			copy(b, cache[i].c_block, BSIZE);
			cache[i].c_time = 0;
			break;
		}
		else
			cache[i].c_time++;
	}
	lseek(fi, bno*BSIZE, 0);
	if(write(fi, b, BSIZE) != BSIZE) {
#ifdef STANDALONE
		printf("disk write error %D\n", bno);
#else
		fprintf(stderr, "disk write error %ld\n", bno);
#endif
		rstexit(FATAL);
	}
}

dread(bno, buf, cnt)
daddr_t bno;
char *buf;
{
	register i, j;

	j = 0;
	for (i = 0; i < NCACHE; i++) {
		if (++curcache >= NCACHE)
			curcache = 0;
		if (cache[curcache].c_bno == bno) {
			copy(cache[curcache].c_block, buf, cnt);
			cache[curcache].c_time = 0;
			return;
		}
		else {
			cache[curcache].c_time++;
			if (cache[j].c_time < cache[curcache].c_time)
				j = curcache;
		}
	}

	lseek(fi, bno*BSIZE, 0);
	if (read(fi, cache[j].c_block, BSIZE) != BSIZE) {
#ifdef STANDALONE
		printf("read error %D\n", bno);
#else
		printf("read error %ld\n", bno);
#endif
		rstexit(FATAL);
	}
	copy(cache[j].c_block, buf, cnt);
	cache[j].c_time = 0;
	cache[j].c_bno = bno;
}

/*
 * the inode manpulation routines. Like the system.
 *
 * clri zeros the inode
 */
clri(ip)
struct dinode *ip;
{
	int i, *p;

	if(ip->di_mode&IFMT)
		sblock.s_tinode++;
	i = sizeof(struct dinode)/sizeof(int);
	p = (int *)ip;
	do
		*p++ = 0;
	while(--i);
}

/*
 * itrunc/tloop/bfree free all of the blocks pointed at by the inode
 */
itrunc(ip)
register struct dinode *ip;
{
	register i;
	daddr_t bn, iaddr[NADDR];

	if (ip->di_mode == 0)
		return;
	i = ip->di_mode & IFMT;
	if (i != IFDIR && i != IFREG)
		return;
	l3tol(iaddr, ip->di_addr, NADDR);
	for(i=NADDR-1;i>=0;i--) {
		bn = iaddr[i];
		if(bn == 0) continue;
		switch(i) {

		default:
			bfree(bn);
			break;

		case NADDR-3:
			tloop(bn, 0, 0);
			break;

		case NADDR-2:
			tloop(bn, 1, 0);
			break;

		case NADDR-1:
			tloop(bn, 1, 1);
		}
	}
	ip->di_size = 0;
}

tloop(bn, f1, f2)
daddr_t	bn;
int	f1, f2;
{
	register i;
	daddr_t nb;
	union {
		char	data[BSIZE];
		daddr_t	indir[NINDIR];
	} ibuf;

	dread(bn, ibuf.data, BSIZE);
	for(i=NINDIR-1;i>=0;i--) {
		nb = ibuf.indir[i];
		if(nb) {
			if(f1)
				tloop(nb, f2, 0);
			else
				bfree(nb);
		}
	}
	bfree(bn);
}

bfree(bn)
daddr_t	bn;
{
	register i;
	union {
		char	data[BSIZE];
		struct	fblk frees;
	} fbuf;

	if(sblock.s_nfree >= NICFREE) {
		fbuf.df_nfree = sblock.s_nfree;
		for(i=0;i<NICFREE;i++)
			fbuf.df_free[i] = sblock.s_free[i];
		sblock.s_nfree = 0;
		dwrite(bn, fbuf.data);
	}
	sblock.s_free[sblock.s_nfree++] = bn;
	sblock.s_tfree++;
}

/*
 * allocate a block off the free list.
 */
daddr_t
balloc()
{
	daddr_t	bno;
	register i;
	static char zeroes[BSIZE];
	union {
		char	data[BSIZE];
		struct	fblk frees;
	} fbuf;

	if(sblock.s_nfree == 0 || (bno=sblock.s_free[--sblock.s_nfree]) == 0) {
#ifdef STANDALONE
		printf("Out of space\n");
#else
		fprintf(stderr, "Out of space\n");
#endif
		rstexit(FATAL);
	}
	if(sblock.s_nfree == 0) {
		dread(bno, fbuf.data, BSIZE);
		sblock.s_nfree = fbuf.df_nfree;
		for(i=0;i<NICFREE;i++)
			sblock.s_free[i] = fbuf.df_free[i];
	}
	dwrite(bno, zeroes);
	sblock.s_tfree--;
	return(bno);
}

/*
 * map a block number into a block address, ensuring
 * all of the correct indirect blocks are around. Allocate
 * the block requested.
 */
daddr_t
bmap(iaddr, bn)
daddr_t	iaddr[NADDR];
daddr_t	bn;
{
	register i;
	int j, sh;
	daddr_t nb, nnb;
	daddr_t indir[NINDIR];

	/*
	 * blocks 0..NADDR-4 are direct blocks
	 */
	if(bn < NADDR-3) {
		iaddr[bn] = nb = balloc();
		return(nb);
	}

	/*
	 * addresses NADDR-3, NADDR-2, and NADDR-1
	 * have single, double, triple indirect blocks.
	 * the first step is to determine
	 * how many levels of indirection.
	 */
	sh = 0;
	nb = 1;
	bn -= NADDR-3;
	for(j=3; j>0; j--) {
		sh += NSHIFT;
		nb <<= NSHIFT;
		if(bn < nb)
			break;
		bn -= nb;
	}
	if(j == 0) {
		return((daddr_t)0);
	}

	/*
	 * fetch the address from the inode
	 */
	if((nb = iaddr[NADDR-j]) == 0) {
		iaddr[NADDR-j] = nb = balloc();
	}

	/*
	 * fetch through the indirect blocks
	 */
	for(; j<=3; j++) {
		dread(nb, (char *)indir, BSIZE);
		sh -= NSHIFT;
		i = (bn>>sh) & NMASK;
		nnb = indir[i];
		if(nnb == 0) {
			nnb = balloc();
			indir[i] = nnb;
			dwrite(nb, (char *)indir);
		}
		nb = nnb;
	}
	return(nb);
}

/*
 * read the tape into buf, then return whether or
 * or not it is a header block.
 */
gethead(buf)
struct spcl *buf;
{
#ifdef REST_512
	readtape((char *)buf, 0);
#else
	readtape((char *)buf);
#endif
/*
printf("\ngethead: M=%o cs=%o",buf->c_magic, checksum((int *) buf));
 */
	if (buf->c_magic != MAGIC || checksum((int *) buf) == 0)
		return(0);
	return(1);
}

/*
 * return whether or not the buffer contains a header block
 */
ishead(buf)
struct spcl *buf;
{
	if (buf->c_magic != MAGIC || checksum((int *) buf) == 0) {
/* 
printf("\nishead: M=%o cs=%o",buf->c_magic,checksum((int *) buf));
*/
		return(0);
	}
	return(1);
}

checktype(b, t)
struct	spcl *b;
int	t;
{
	return(b->c_type == t);
}


checksum(b)
int *b;
{
	register i, j;

#ifdef REST_512
	j = OBSIZE/sizeof(int);
#else
	j = BSIZE/sizeof(int);
#endif
	i = 0;
	do
		i += *b++;
	while (--j);
	if (i != CHECKSUM) {
		printf("Checksum error %o\n", i);
		return(0);
	}
	return(1);
}

checkvol(b, t)
struct spcl *b;
int t;
{
	if (b->c_volume == t)
		return(1);
	return(0);
}

readhdr(b)
struct	spcl *b;
{
	if (gethead(b) == 0)
		return(0);
	if (checktype(b, TS_TAPE) == 0)
		return(0);
	return(1);
}

/*
 * The next routines are called during file extraction to
 * put the data into the right form and place.
 */
#ifndef STANDALONE
xtrfile(b, size)
char	*b;
long	size;
{
	write(ofile, b, (int) size);
}

null() {;}

skip()
{
#ifdef REST_512
	lseek(ofile, (long) OBSIZE, 1);
#else
	lseek(ofile, (long) BSIZE, 1);
#endif
}
#endif


rstrfile(b, s)
char *b;
long s;
{
	daddr_t d;

	d = bmap(taddr, curbno);
	dwrite(d, b);
	curbno += 1;
}

rstrskip(b, s)
char *b;
long s;
{
	curbno += 1;
}

#ifndef STANDALONE
putdir(b)
char *b;
{
	register struct direct *dp;
	register i;

	for (dp = (struct direct *) b, i = 0; i < BSIZE; dp++, i += sizeof(*dp)) {
		if (dp->d_ino == 0)
			continue;
		putent((char *) dp);
	}
}
#endif

/*
 * read/write an inode from the disk
 */
getdino(inum, b)
ino_t	inum;
struct	dinode *b;
{
	daddr_t	bno;
	char buf[BSIZE];

	bno = (ino - 1)/INOPB;
	bno += 2;
	dread(bno, buf, BSIZE);
	copy(&buf[((inum-1)%INOPB)*sizeof(struct dinode)], (char *) b, sizeof(struct dinode));
}

putdino(inum, b)
ino_t	inum;
struct	dinode *b;
{
	daddr_t bno;
	char buf[BSIZE];

	if(b->di_mode&IFMT)
		sblock.s_tinode--;
	bno = ((ino - 1)/INOPB) + 2;
	dread(bno, buf, BSIZE);
	copy((char *) b, &buf[((inum-1)%INOPB)*sizeof(struct dinode)], sizeof(struct dinode));
	dwrite(bno, buf);
}

/*
 * read a bit mask from the tape into m.
 */
readbits(m)
short	*m;
{
	register i;

	i = spcl.c_count;

	while (i--) {
#ifdef REST_512
		readtape((char *) m, 0);
		m += (OBSIZE/(MLEN/BITS));
#else
		readtape((char *) m);
		m += (BSIZE/(MLEN/BITS));
#endif
	}
	while (gethead(&spcl) == 0)
		;
}

done()
{
#ifndef STANDALONE
	unlink(dirfile);
#endif
	exit(NORMAL);
}

filcntl()
{
		mtop.mt_op = MTBSF;
		mtop.mt_count = 1;
		if (ioctl(mt,MTIOCTOP,&mtop) < 0) {
			/* abbreviation like BOF for beginning of file used to squeeze the code for restor40*/
			printf("cannot move to BOF: file %d\n",seqno);
			rstexit(FATAL);
		}
		if (seqno > 1) {
			mtop.mt_op = MTFSF;
			if (ioctl(mt,MTIOCTOP,&mtop) < 0) {
				printf("cannot skip TM: file %d\n");
				rstexit(FATAL);
			}
		}
		return(0);
}

rstexit(status)
int status;
{
#ifndef STANDALONE
	unlink(dirfile);
#endif
	exit(status);
}

/****  icheck for standalone version ***/

#ifdef STANDALONE
/*
 * icheck program with -s option for standalone version
 * included here for restarting restor in the middle.
 *
 * George Mathew 11/11/85
 *
 */

#define	NI	8
/* #define	MAXFN	714 */
#define	MAXFN	170
struct	dinode	itab[INOPB*NI];
daddr_t	saiaddr[NADDR];
char	*sabmap;
/*
 * Standalone buffer used for checking dups
 * and salvaging the free list.
 */

int	nerror;
/* #define	SABUFSIZ 16384 */
#define	SABUFSIZ 1200
char	sabuf[SABUFSIZ];
ino_t	saino;
daddr_t	nfree;
long	atol();
daddr_t	alloc();

ichecks()
{
	register i, j;
	ino_t mino;
	daddr_t d;
	long n;

	printf("Wait: icheck being done\n");
	bread((daddr_t)1, (char *)&sblock, sizeof(sblock));
	mino = (sblock.s_isize-2) * INOPB;
	saino = 0;
	n = (sblock.s_fsize - sblock.s_isize + BITS-1) / BITS;
	if (n != (unsigned)n) {
		printf("Check fsize and isize: %D, %u\n",
		   sblock.s_fsize, sblock.s_isize);
		nerror = 1;
	}
	if(n <= SABUFSIZ)
		sabmap = &sabuf;
	else
		sabmap = NULL;
	if (sabmap==NULL) {
		printf("Not enough core; duplicates unchecked\n");
		return(-1);
	}
	for(i=0; i<(unsigned)n; i++)
		sabmap[i] = 0;
	for(i=2;; i+=NI) {
		if(saino >= mino)
			break;
		bread((daddr_t)i, (char *)itab, sizeof(itab));
		for(j=0; j<INOPB*NI; j++) {
			if(saino >= mino)
				break;
			saino++;
			pass2(&itab[j]);
		}
	}
	saino = 0;
	bread((daddr_t)1, (char *)&sblock, sizeof(sblock));
	makefree();
	if (nerror)
		return(-1);
	else
		return(0);
	
}

pass2(ip)
register struct dinode *ip;
{
	daddr_t ind1[NINDIR];
	daddr_t ind2[NINDIR];
	daddr_t ind3[NINDIR];
	register i, j;
	int k, l;

	i = ip->di_mode & IFMT;
	if( (i == 0) || (i == IFIFO) || (i == IFCHR) || (i == IFBLK) )
		return;
	if( (i != IFDIR) && (i != IFREG)
#ifdef  UCB_SYMLINKS
		&&(i != IFLNK)
#endif
		) {
			printf("bad mode %u\n", saino);
			nerror = 1;
			return(1);
		}
	l3tol(saiaddr, ip->di_addr, NADDR);
	for(i=0; i<NADDR; i++) {
		if(saiaddr[i] == 0)
			continue;
		if(i < NADDR-3) {
			chk(saiaddr[i], "data (small)");
			continue;
		}
		if (chk(saiaddr[i], "1st indirect"))
				continue;
		bread(saiaddr[i], (char *)ind1, BSIZE);
		for(j=0; j<NINDIR; j++) {
			if(ind1[j] == 0)
				continue;
			if(i == NADDR-3) {
				chk(ind1[j], "data (large)");
				continue;
			}
			if(chk(ind1[j], "2nd indirect"))
				continue;
			bread(ind1[j], (char *)ind2, BSIZE);
			for(k=0; k<NINDIR; k++) {
				if(ind2[k] == 0)
					continue;
				if(i == NADDR-2) {
					chk(ind2[k], "data (huge)");
					continue;
				}
				if(chk(ind2[k], "3rd indirect"))
					continue;
				bread(ind2[k], (char *)ind3, BSIZE);
				for(l=0; l<NINDIR; l++)
					if(ind3[l]) {
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
		printf("%D bad; inode=%u, class=%s\n", bno, saino, s);
		nerror =1;
		return(1);
	}
	if(duped(bno)) {
		printf("%D dup; inode=%u, class=%s\n", bno, saino, s);
		nerror = 1;
	}
	return(0);
}

duped(bno)
daddr_t bno;
{
	daddr_t d;
	register m, n;

	d = bno - sblock.s_isize;
	m = 1 << (d%BITS);
	n = (d/BITS);
	if(sabmap[n] & m)
		return(1);
	sabmap[n] |= m;
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
		nerror = 1;
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
			nerror = 1;
			return(0);
		}
		for(i=0; i<NICFREE; i++)
			sblock.s_free[i] = buf.fb.df_free[i];
	}
	return(bno);
}

sabfree(bno)
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
		nerror = 1;
			printf("No update\n");
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
		nerror = 1;
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
	sblock.s_tfree = 0;

	sabfree((daddr_t)0);
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
				sabfree(f);
	}
	bwrite((daddr_t)1, (char *)&sblock);
	return(0);
}
#endif STANDALONE
