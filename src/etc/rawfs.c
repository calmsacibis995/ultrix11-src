
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char *sccsid =  "@(#)rawfs.c	3.0	(ULTRIX-11)	4/22/86";
/*
 * rawfs - copy files from a raw file system.
 *	6/85	-Dave Borman
 *
 * Usage:
 *	See usage() function below.
 * Options:
 *	-n	No clobber. Existing files are not overwritten
 *	-s	Special. Copy character/block special files.
 *	-o #	File system starts at a block offset of #.
 *	-d	Debug mode. Multiple references increases debug info.
 *	-t	just do a listing, no copy.
 *	  -l	long listing
 *	  -i	print inode number
 *	  -g	print gid instead of uid
 *	  -R	Recursive, descend directories
 *	  -u	print last accessed time instead of modified time
 *	  -c	print creation time instead of modified time
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/file.h>	/* for open() flags */

/*
 * Variables dealing with the file system.  These are the default
 * values, which are for a 512byte block file system.
 */
#define	BSIZE	512		/* 512 byte block */
#define	BMASK	0777		/* BSIZE-1 */
#define	NINDIR	(BSIZE/sizeof(long))
#define	NMASK	0177		/* NINDIR-1 */
#define	NSHIFT	7		/* LOG2(NINDIR) */
#define	INOPB	8		/* 8 inodes per block */
#define	NADDR	13		/* number of address entries used in inode */

#define	DIRSIZ	14		/* name length if directory entry */
#define	ROOTINO	((short)2)	/* i number of all roots */
#define	SUPERB	((long)1)	/* block number of the super block */
/*
 * These two don't matter much, because the only thing that we look
 * at in the superblock is s_fsize and s_isize, the first two entries.
 */
#define	NICINOD	100		/* number of inode in superblock */
#define	NICFREE	50		/* number of free blocks in superblock */
/*
 * Inode structure as it appears on
 * a disk block.
 */
struct dinode
{
	unsigned short	di_mode;/* mode and type of file */
	short	di_nlink;    	/* number of links to file */
	short	di_uid;      	/* owner's user id */
	short	di_gid;      	/* owner's group id */
	off_t	di_size;     	/* number of bytes in file */
	char  	di_addr[40];	/* disk block addresses */
	time_t	di_atime;   	/* time last accessed */
	time_t	di_mtime;   	/* time last modified */
	time_t	di_ctime;   	/* time created */
};
/* modes */
#define	IFMT	0170000		/* type of file */
#define		IFIFO	0010000	/* fifo special */
#define		IFREE	0000000	/* free */
#define		IFDIR	0040000	/* directory */
#define		IFCHR	0020000	/* character special */
#define		IFBLK	0060000	/* block special */
#define		IFREG	0100000	/* regular */
#define		IFLNK	0120000	/* symbolic link */
#define		IFSOCK	0140000	/* socket */
#define	ISUID	04000		/* set user id on execution */
#define	ISGID	02000		/* set group id on execution */
#define ISVTX	01000		/* save swapped text even after use */
#define	IREAD	0400		/* read, write, execute permissions */
#define	IWRITE	0200
#define	IEXEC	0100

/*
 * Structure of the super-block
 */
struct	filsys {
	unsigned short s_isize; /* size in blocks of i-list */
#ifndef	vax
	long	s_fsize;   	/* size in blocks of entire volume */
#define	SB_S_FSIZE	sb->s_fsize
#else
	/* Grrr, the vax will longword align s_fsize if it is long... */
	short	s_fsize1;
	short	s_fsize2;
#define	SB_S_FSIZE	(*((long *)&sb->s_fsize1))
#endif
	short  	s_nfree;   	/* number of addresses in s_free */
	long	s_free[NICFREE];/* free block list */
	short  	s_ninode;  	/* number of i-nodes in s_inode */
	short  	s_inode[NICINOD];/* free i-node list */
	char   	s_flock;   	/* lock during free list manipulation */
	char   	s_ilock;   	/* lock during i-list manipulation */
	char   	s_fmod;    	/* super block modified flag */
	char   	s_ronly;   	/* mounted read-only flag */
	time_t 	s_time;    	/* last super block update */
	/* set by mkfs, updated by fsck, now used by system */
	long	s_tfree;   	/* total free blocks*/
	short  	s_tinode;  	/* total free inodes */
	/* set by mkfs, used by fsck to salvage free lists */
	short   s_m;	/* interleave - free list spacing */
	short	s_n;	/* interleave - blocks per cylinder */
	/* set by mkfs and/or labelit, printed by fsck and labelit */
	char   	s_fname[6];	/* file system name */
	char   	s_fpack[6];	/* file system pack name */
	short	s_lasti;	/* start place for circular search */
	short	s_nbehind;	/* est # free inodes before s_lasti */
} *sb;

struct	direct
{
	short	d_ino;
	char	d_name[DIRSIZ];
};

int	diskio;
extern int errno;

struct diskp {
	char	*name;
	long	offset[8];
};
/*
 * Known diskp partion offsets for ULTRIX-11 V2.0.
 * All offsets are in 512 byte blocks.
 */
struct diskp  u11v2dp[] = {
	"rl01", 0L,    -1L,    -1L,    -1L,     -1L,     -1L,     -1L,  0L,
	"rl02", 0L, 10240L,    -1L,    -1L,     -1L,     -1L,     -1L,  0L,
	"rp02", 0L,  9600L, 15000L,    -1L,  25600L,     -1L,      0L, -1L,
	"rp03", 0L,  9600L, 15000L, 25600L,     -1L,     -1L,     -1L,  0L,
	"rk06", 0L,  9636L, 15840L,    -1L,     -1L,     -1L,      0L, -1L,
	"rk07", 0L,  9636L, 15840L, 27126L,     -1L,     -1L,      0L,  0L,
	"rp04", 0L,  9614L, 15884L, 27170L,     -1L,     -1L,      0L, -1L,
	"rp05", 0L,  9614L, 15884L, 27170L,     -1L,     -1L,      0L, -1L,
	"rp06", 0L,  9614L, 15884L, 27170L,  27170L, 171380L,      0L,  0L,
	"rm02", 0L,  9600L, 15840L, 26400L,  26400L,  79040L,     -1L,  0L,
	"rm03", 0L,  9600L, 15840L, 26400L,  26400L,  79040L,     -1L,  0L,
	"rm05", 0L,  9728L, 16416L, 27968L, 133152L, 133152L, 316768L,  0L,
	"rd51", 0L,  8468L,    -1L,    -1L,     -1L,     -1L,     -1L,  0L,
	"rd52", 0L,  8468L, 21568L,    -1L,     -1L,     -1L,     -1L,  0L,
	"ra60", 0L,  9600L, 15870L, 27156L,  27156L, 213156L,     -1L,  0L,
	"ra80", 0L,  9600L, 15870L, 27156L,     -1L,     -1L,     -1L,  0L,
	"ra81", 0L,  9600L, 15870L, 27156L,  27156L, 213156L, 399156L,  0L,
	"rc25", 0L,  9600L, 15870L, 27156L,     -1L,     -1L,     -1L,  0L,
	0,
};

char *me;
int debug = 0;
int Sflag = 0;
int dospecial = 0;
int noclobber = 0;
int recursive = 0;
int verbose = 0;
int xtract = 0;
int dolist = 0;
int cflag = 0;
int gflag = 0;
int iflag = 0;
int lflag = 0;
int uflag = 0;
long offset = 0L;
long year;
int Bsize = BSIZE;
int Bmask = BMASK;
int Nindir = NINDIR;
int Naddr = NADDR;
int Nshift = NSHIFT;
int Nmask = NMASK;
int Inopb = INOPB;
long		*sdatabuf;
long		*ddatabuf;
long		*tdatabuf;
char		*databuf;
char		*malloc();
#define	NAMSIZE	512

main(argc, argv)
int	argc;
register char	**argv;
{
	register int	err;
	register char	*cp;
	char		tbuf[NAMSIZE];
	struct stat	statb;
	long		atol();

	me = *argv;
	++argv; --argc;
	for (; argc > 1 && **argv == '-'; ++argv, --argc) {
	    for (cp = &(*argv)[1]; *cp; cp++)
		switch(*cp) {
		/*
		 * Flags for listing and extracting
		 */
		default:
			fprintf(stderr, "unknown switch %c\n", *cp);
			usage(NULL);
		case 'd':
			++debug;
			break;
		case 'S':
			/* swap words in longs */
			++Sflag;
			break;
		case 'k':
			/*
			 * Set up paramaters for a 1K filesystem, ala
			 * 2.9 BSD.
			 */
			Bsize = 1024;
			Bmask = 01777;
			Nindir = 256;
			Inopb = 16;
			Nshift = 8;
			Nmask = 0377;
			Naddr = 7;
			break;
		case 'b':
			Bsize *= 2;		/* double the buffer size */
			Bmask = (Bmask<<1)|1;	/* add a bit to the mask */
			Nindir *= 2;		/* double the # on indirects */
			Inopb *= 2;
			Nshift++;
			Nmask = (Nmask<<1)|1;
			break;
		case 'o':
		    {
			register struct diskp	*dp;
			register int		pt;
			if (++argv, --argc <= 0) {
				fprintf(stderr, "missing argument for -o\n");
				usage(NULL);
			}
			if (**argv >= '0' && **argv <= '9') {
				offset = atol(*argv)*512L;
				continue;
			}
			if (strlen(*argv) != 6) {
				fprintf(stderr, "%s: unknown disk\n", *argv);
				exit(1);
			}
			for (dp = u11v2dp; dp->name; dp++) {
				if (strncmp(dp->name, *argv, 4) == 0)
					break;
			}
			pt = (*argv)[5] - '0';
			if (dp->name == NULL || pt > 7 || pt < 0 ||
			    dp->offset[pt] == -1) {
				fprintf(stderr, "%s: unknown disk\n", *argv);
				exit(1);
			}
			offset = dp->offset[pt]*512L;
			if (debug)
				printf("Offset is %D\n", offset);
			break;
		    }
		/*
		 * Flags only for extracting
		 */
		case 'x':
			xtract++;
			break;
		case 'n':
			noclobber++;
			break;
		case 's':
			dospecial = 1;
			break;
		case 'v':
			verbose++;
			break;
		/*
		 * Flags only for listing
		 */
		case 't':
			dolist++;
			break;
		case 'g':
			gflag++;
			break;
		case 'c':
			cflag++;
			break;
		case 'i':
			iflag++;
			break;
		case 'l':
			lflag++;
			break;
		case 'R':
			recursive++;
			break;
		case 'u':
			uflag++;
			break;
		}
	}
	if (cflag || iflag || lflag || recursive || uflag)
		++dolist;
	if (noclobber || dospecial || verbose)
		++xtract;
	if (!dolist && !xtract) {
		fprintf(stderr, "Either the -x or -t flag is required\n");
		usage(NULL);
	}
	if (dolist && xtract) {
		fprintf(stderr, "Cannot mix t,c,g,i,l,R and u with x,n,s and v\n");
		usage(NULL);
	}
	if (dolist) {
		if (argc < 1)
			usage(NULL);
	} else if (argc < 3)
		usage(NULL);
	if ((sb = (struct filsys *)malloc(Bsize)) == NULL ||
	    (databuf = malloc(Bsize)) == NULL ||
	    (sdatabuf = (long *)malloc(Bsize)) == NULL ||
	    (ddatabuf = (long *)malloc(Bsize)) == NULL ||
	    (tdatabuf = (long *)malloc(Bsize)) == NULL) {
		fprintf(stderr, "Out of memory\n");
		exit(1);
	}
	if ((diskio = open(*argv, O_RDONLY)) < 0)
		usage(*argv);
	--argc; ++argv;
	SB_S_FSIZE = SUPERB+1; /* so that getblock won't fail */
	if (getblock((long)SUPERB, sb) < 0)
		usage(*argv);
	verify();
	if (dolist) {
		struct dinode dis;
		long time();

		year = time(0) - 6L*60L*60L*24L*30L; /* ~6 months */
		if (argc == 0) {
			if (getinode(ROOTINO, &dis) < 0) {
				errno = ENOENT;
				perror("/");
				exit(1);
			}
			if ((dis.di_mode&IFMT) != IFDIR) {
				errno = ENOTDIR;
				perror("/");
				exit(1);
			}
			list(&dis, "");
			exit(0);
		}
		while (argc > 0) {
			unsigned inum;

			if ((inum = namei(*argv, &dis, ROOTINO)) == 0) {
				errno = ENOENT;
				perror(*argv);
				err++;
			} else {
				if ((dis.di_mode&IFMT) == IFDIR)
					list(&dis, *argv);
				else {
					if (iflag)
						printf("%5u ", inum);
					if (lflag)
						dostats(&dis);
					printf("%s\n", *argv);
				}
			}
			--argc; ++argv;
		}
		exit(err);
	}
	--argc;
	err = stat(argv[argc], &statb);
	if (err == -1) {
		if (argc != 1)
			usage(argv[argc]);
		sprintf(tbuf, "%s", argv[1]);
	} else if ((statb.st_mode&S_IFMT) != S_IFDIR) {
		if (argc == 1 && ((statb.st_mode&S_IFMT) == S_IFREG)) {
			sprintf(tbuf, "%s", argv[1]);
		} else {
			fprintf(stderr, "%s: not a directory\n", argv[argc]);
			usage(NULL);
		}
	} else {
		if (cp = rindex(*argv, '/'))
			cp++;
		else
			cp = *argv;
		sprintf(tbuf, "%s/%s", argv[argc], cp);
	}
	for (;;) {
		struct dinode dis;
		if (namei(*argv, &dis, ROOTINO) == 0) {
			errno = ENOENT;
			perror(*argv);
			err++;
		} else if (copyfile(&dis, tbuf) < 0) {
			fprintf(stderr, "%s:can't copy\n", *argv);
			err++;
		}
		if (--argc <= 0)
			break;
		++argv;
		if (cp = rindex(*argv, '/'))
			cp++;
		else
			cp = *argv;
		sprintf(tbuf, "%s/%s", argv[argc], cp);
	}
	exit(err);
}

usage(s)
char *s;
{
	if (s != NULL)
		perror(s);
	fprintf(stderr,
	 "usage: %s -x[Skbsnv] [-o offset] special file file\n", me);
	fprintf(stderr,
	 "       %s -x[Skbsnv] [-o offset] special file ... dir\n", me);
	fprintf(stderr,
	 "       %s -t[SkbcgilRu] [-o offset] special [file ...]\n", me);
	fprintf(stderr,
	 "         offset: 512byte block number or diskname/partition\n");
	exit(1);
}

swab(p)
register short *p;
{
	register short t;
	t = *p;
	*p = *(p+1);
	*(p+1) = t;
}

/*
 * Verify a filesystem by doing a few sanity checks on
 * the superblock and the root inode.
 */
verify()
{
	struct dinode		dis;
	register struct dinode	*dp = &dis;

	if (Sflag)
		swab(&SB_S_FSIZE);
	if (SB_S_FSIZE < 0 ||
	    SB_S_FSIZE > 077777777L ||
	    getblock(SB_S_FSIZE-1, databuf) < 0) {
		fprintf(stderr,
			"Bad file system size (%D blocks)\n", SB_S_FSIZE);
		exit(1);
	}
	if (sb->s_isize >= SB_S_FSIZE) {
		fprintf(stderr,
			"inode list larger than filesystem size (%u > %D)\n",
			sb->s_isize, SB_S_FSIZE);
		exit(1);
	}
	if (getinode(ROOTINO, dp) < 0) {
		fprintf(stderr, "Can't get root inode\n");
		exit(1);
	}
	switch(dp->di_mode&IFMT) {
	case IFDIR:
		break;
	case IFCHR:
		fprintf(stderr, "Root inode is a character special file!\n");
		exit(1);
	case IFBLK:
		fprintf(stderr, "Root inode is a block special file!\n");
		exit(1);
	case IFREG:
		fprintf(stderr, "Root inode is a regular file!\n");
		exit(1);
	default:
		fprintf(stderr, "Root inode is of unknown type! (%o)\n",
								dp->di_mode);
		exit(1);
	}
	if ((off_t)dp->di_nlink*(off_t)sizeof(struct direct) > dp->di_size) {
		fprintf(stderr, "Root inode has %u links and only %D entries",
			dp->di_nlink, dp->di_size/sizeof(struct direct));
		exit(1);
	}
}

copyfile(dp, to)
register struct dinode	*dp;
register char		*to;
{

	if (debug)
		printf("copyfile(%o, %s)\n", dp, to);
	switch (dp->di_mode&IFMT) {
	case IFREG:
		{
			register int fio;

			if (noclobber)
				fio = open(to, O_CREAT|O_WRONLY|O_EXCL, dp->di_mode);
			else
				fio = open(to, O_CREAT|O_WRONLY, dp->di_mode);
			if (fio < 0) {
				perror(to);
				return(-1);
			}
			if (copyi(dp, fio) < 0) {
				close(fio);
				unlink(to);
				return(-1);
			} else {
				close(fio);
				chown(to, dp->di_uid, dp->di_gid);
				chmod(to, dp->di_mode);
				utime(to, &dp->di_atime);
				if (verbose)
					printf("%s\n", to);
				return(0);
			}
		}
	case IFCHR:
	case IFBLK:
		if (!dospecial) {
			if (verbose)
				printf("%s: special file not copied\n", to);
		} else {
			register char *p1, *p2;
			long zarf;

			p1 = (char *)&zarf;
			p2 = (char *)&dp->di_addr[0];
			*p1++ = *p2++;
			*p1++ = 0;
			*p1++ = *p2++;
			*p1 = *p2;
			if (mknod(to, dp->di_mode, (int)zarf) < 0) {
				perror(to);
				return(-1);
			}
			chown(to, dp->di_uid, dp->di_gid);
			chmod(to, dp->di_mode);
			utime(to, &dp->di_atime);
			if (verbose)
				printf("%s\n", to);
			return(0);
		}
	case IFDIR:
		return(copydir(dp, to));
	/*
	 *	fprintf(stderr, "%s: can't copy directories (yet)\n", from);
	 */
	default:
		fprintf(stderr, "bad file type (%o)\n", dp->di_mode);
		break;
	}
	return(-1);
}

namei(name, inode, inum)
char			*name;
struct dinode		*inode;
register unsigned	inum;
{
	register char		*p1, *cp;
	struct dinode		dis;
	char			nbuf[DIRSIZ+1];
	unsigned		search();

	if (debug)
		printf("namei(%s, %o, %u)\n", name, inode, inum);
	for (p1 = name; *p1 == '/'; p1++)
		;
	for(;;) {
		if (getinode(inum, &dis) < 0)
			return(0);
		if (*p1 == '\0') {
			*inode = dis;
			return (inum);
		}
		if ((dis.di_mode&IFMT) != IFDIR)
			return(0);
		cp = nbuf;
		for (; *p1 && *p1 != '/'; p1++)
			if (cp < &nbuf[DIRSIZ])
				*cp++ = *p1;
		*cp = '\0';
		for (; *p1 == '/'; p1++)
			;
		inum = search(&dis, nbuf);
		if (inum == 0)
			return(0);
	}
	/*NOTREACHED*/
}

/*
 * laddr converts a 3 character block number as is
 * stored in a disk inode, and returns it as a long.
 */
long laddr(p2)
register char *p2;
{
	register char	*p1;
	long		t;

	if (debug>4)
		printf("laddr(%o %o %o) = ", p2[0], p2[1], p2[2]);
	p1 = (char *)&t;
	if (Sflag) {
		/* swap the words for VAX/PDP conflict */
		register char t;
		t = *p2++;
		*p1++ = *p2++;
		*p1++ = *p2;
		*p1++ = t;
		*p1 = 0;
	} else {
		*p1++ = *p2++;
		*p1++ = 0;
		*p1++ = *p2++;
		*p1 = *p2;
	}
	if (debug>4)
		printf("%D\n", t);
	return(t);
}

/*
 * Bmap defines the structure of file system storage
 * by returning the physical block number on a device given the
 * inode and the logical block number in a file.
 */
long
bmap(dp, bn)
register struct dinode	*dp;
long			bn;
{
	register	i;
	int		j, sh;
	long		nb;

	if (bn < 0)
		return((long)0);

	/*
	 * blocks 0..Naddr-4 are direct blocks
	 */
	if (bn < Naddr-3) {
		i = bn;
		nb = laddr(&dp->di_addr[i*3]);
		if (nb == 0)
			return((long)-1);
		return(nb);
	}

	/*
	 * addresses Naddr-3, Naddr-2, and Naddr-1
	 * have single, double, triple indirect blocks.
	 * the first step is to determine
	 * how many levels of indirection.
	 */
	sh = 0;
	nb = 1;
	bn -= Naddr-3;
	for (j=3; j>0; j--) {
		sh += Nshift;
		nb <<= Nshift;
		if (bn < nb)
			break;
		bn -= nb;
	}
	if (j == 0)
		return((long)0);

	/*
	 * fetch the address from the inode
	 */
	nb = laddr(&(dp->di_addr[(Naddr-j)*3]));
	if (nb == 0)
		return((long)-1);

	/*
	 * fetch through the indirect blocks
	 */
	for (; j<=3; j++) {
		if (getblock(nb, databuf) < 0) {
			nb = (long)0;
			break;
		}
		sh -= Nshift;
		i = (bn>>sh) & Nmask;
		nb = ((long *)databuf)[i];
		if (nb == 0)
			return((long)-1);
	}
	return(nb);
}

unsigned search(dp, name)
register struct dinode	*dp;
register char		*name;
{
	register struct direct	*dir, *edir;
	char			*buf;
	long			bn, lbn, llbn;

	if (debug)
		printf("search(%o, %s)\n", dp, name);
	if ((dp->di_mode&IFMT) != IFDIR) {
		if (debug)
			printf("   not a directory\n");
		return(0);
	}
	if ((buf = malloc(Bsize)) == NULL) {
		fprintf(stderr, "Out of memory\n");
		return(0);
	}
	edir = (struct direct *)&buf[Bsize];
	llbn = dp->di_size/Bsize;
	for (lbn = 0; lbn <= llbn; lbn++) {
		bn = bmap(dp, lbn);
		if ((bn == -1L) || (getblock(bn, buf) == -1))
			continue;
		if (lbn == llbn)
			edir = (struct direct *)&buf[dp->di_size&Bmask];
		for (dir = (struct direct *)buf; dir < edir; dir++) {
			if (debug>1)
				printf("    %5u %.14s\n", dir->d_ino, dir->d_name);
			if (dir->d_ino && !strncmp(dir->d_name, name, DIRSIZ)) {
				int inum = dir->d_ino;
				free(buf);
				return(inum);
			}
		}
	}
	free(buf);
	return(0);
}

copydir(dp, to)
register struct dinode	*dp;
register char		*to;
{
	int			statval;
	struct direct		*edir;
	long			bn, lbn, llbn;
	struct stat		statb;
	char			*buf, nbuf[NAMSIZE];
	char			*rindex();

	if (debug)
		printf("copydir(%o, %s)\n", dp, to);
	if ((statval = stat(to, &statb)) == 0) {
		if ((statb.st_mode&S_IFMT) != S_IFDIR) {
			fprintf(stderr, "%s: not a directory\n", to);
			return(-1);
		}
	} else {
		register char *cp;

		if (debug)
			printf("making empty directory %s\n", to);
		if (mknod(to, dp->di_mode, 0) < 0) {
			perror(to);
			return(-1);
		}
		sprintf(nbuf, "%s/.", to);
		link(to, nbuf);
		sprintf(nbuf, "%s/..", to);
		cp = rindex(to, '/');
		if (cp) {
			if (cp != to) {
				*cp = '\0';
				link(to, nbuf);
				*cp = '/';
			} else
				link("/", nbuf);
		} else
			link(".", nbuf);
	}
	if ((buf = malloc(Bsize)) == NULL) {
		fprintf(stderr, "Out of memory\n");
		return(-1);
	}
	edir = (struct direct *)&buf[Bsize];
	llbn = dp->di_size/Bsize;
	for (lbn = 0; lbn <= llbn; lbn++) {
		register struct direct	*dir;

		bn = bmap(dp, lbn);
		if (bn == -1L)
			continue;
		if (getblock(bn, buf) == -1)
			continue;
		if (lbn == llbn)
			edir = (struct direct *)&buf[dp->di_size&Bmask];
		for (dir = (struct direct *)buf; dir < edir; dir++) {
			struct dinode dis;
			if (debug>1)
				printf("    %5u %.14s\n", dir->d_ino, dir->d_name);
			if (!dir->d_ino)
				continue;
			if (dir->d_name[0] == '.' &&
			    (dir->d_name[1] == '\0' ||
			     (dir->d_name[1] == '.' &&
			      dir->d_name[2] == '\0')))
				continue;
			sprintf(nbuf, "%s/%.14s", to, dir->d_name);
			if (getinode(dir->d_ino, &dis) < 0) {
				printf("bad inode: %o %.14s\n",
						dir->d_ino, dir->d_name);
				continue;
			}
			if (copyfile(&dis, nbuf) < 0) {
				fprintf(stderr, "can't copy to %s\n", nbuf);
			}
		}
	}
	if (statval < 0) {
		chown(to, dp->di_uid, dp->di_gid);
		chmod(to, dp->di_mode);
		utime(to, &dp->di_atime);
		if (verbose)
			printf("%s\n", to);
	}
	free(buf);
	return(0);
}

copyi(dp, fio)
register struct dinode	*dp;
int			fio;
{
	register i, resid;
	long bn, max;
	long t;

	if (debug)
		printf("copyi(%o, %d)\n", dp, fio);
	max = dp->di_size/Bsize;
	resid = dp->di_size & Bmask;

	/*
	 * Copy direct blocks first
	 */
	for (i = 0, bn = 0; i < Naddr-3 && bn <= max; i++, bn++)
	    if (cp_data(laddr(&dp->di_addr[i*3]),fio,(bn<max)?Bsize:resid) < 0)
		return(-1);

	/*
	 * Single indirect blocks.
	 */
	if (bn <= max+1)
	    if (cp_sindir(laddr(&dp->di_addr[(Naddr-3)*3]),&bn,max,fio,resid)<0)
		return(-1);

	/*
	 * Double indirect blocks.
	 */
	if (bn <= max+1)
	    if (cp_dindir(laddr(&dp->di_addr[(Naddr-2)*3]),&bn,max,fio,resid)<0)
		return(-1);

	/*
	 * Triple indirect blocks
	 */
	if (bn <= max) {
		if (laddr(&dp->di_addr[(Naddr-1)*3]) != 0L) {
		    if (getblock(laddr(&dp->di_addr[(Naddr-1)*3]),tdatabuf) < 0)
			return(-1);
		    if (Sflag) {
			for (i = 0; i < Nindir && bn <= max; i++) {
			    swab(&tdatabuf[i]);
			    if (cp_dindir(tdatabuf[i], &bn, max, fio, resid)<0)
				return(-1);
			}
		    } else {
			for (i = 0; i < Nindir && bn <= max; i++)
			    if (cp_dindir(tdatabuf[i], &bn, max, fio, resid)<0)
				return(-1);
		    }
		}
	}
	lseek(fio, dp->di_size, 0);
	return(0);
}

cp_dindir(block, bnp, max, fio, resid)
long		block, max;
register long	*bnp;
register int	fio;
int		resid;
{
	register int	i;

	if (debug>2)
		printf("cp_dindir(%D, %o, %D, %d %d)\n", block, bnp, max, fio, resid);
	if (block == 0) {
		*bnp += (long)Nindir*(long)Nindir;
		return(lseek(fio, (long)Nindir*(long)Nindir*(long)Bsize, 1));
	}
	if (getblock(block, ddatabuf) < 0)
		return(0);
	if (Sflag) {
		for (i = 0; i < Nindir && *bnp <= max; i++) {
			swab(&ddatabuf[i]);
			if (cp_sindir(ddatabuf[i], bnp, max, fio, resid) < 0)
				return(-1);
		}
		return(0);
	}
	for (i = 0; i < Nindir && *bnp <= max; i++)
		if (cp_sindir(ddatabuf[i], bnp, max, fio, resid) < 0)
			return(-1);
	return(0);
}

cp_sindir(block, bnp, max, fio, resid)
long		block, max;
register long	*bnp;
register int	fio;
int		resid;
{
	register int	i;

	if (debug>2)
		printf("cp_sindir(%D, %o, %D, %d)\n", block, bnp, max, fio);
	if (block == 0) {
		*bnp += Nindir;
		return(lseek(fio, (long)Nindir*(long)Bsize, 1));
	}
	if (getblock(block, sdatabuf) < 0)
		return(-1);
	if (Sflag) {
		for (i = 0; i < Nindir && *bnp <= max; i++, (*bnp)++) {
			swab(&sdatabuf[i]);
			if (cp_data(sdatabuf[i], fio,
					(*bnp < max) ? Bsize : resid) < 0)
				return(-1);
		}
		return(0);
	}
	for (i = 0; i < Nindir && *bnp <= max; i++, (*bnp)++)
		if (cp_data(sdatabuf[i], fio, (*bnp < max) ? Bsize : resid) < 0)
			return(-1);
	return(0);
}

cp_data(block, fio, resid)
long	block;
int	fio;
int	resid;
{
	if (debug>2)
		printf("cp_data(%D, %d)\n", block, fio);
	if (block == 0)
		return(lseek(fio, (long)Bsize, 1));
	if (getblock(block, databuf) < 0)
		return(-1);
	write(fio, databuf, resid);
	return(0);
}

getblock(block, buf)
long	block;
char	*buf;
{
	long	lseek();

	if (debug>3)
		printf("getblock(%D, %o)\n", block, buf);
	if (debug>5)
		printf("SB_S_FSIZE == %D\n", SB_S_FSIZE);
	if (block < 0L || block > SB_S_FSIZE) {
		fprintf(stderr, "getblock(%D): block out of range\n", block);
		fprintf(stderr, "    %D > %D\n", block, SB_S_FSIZE);
		return(-1);
	}
	if (lseek(diskio, offset + Bsize*block, 0) == -1L ||
	    read(diskio, buf, Bsize) != Bsize) {
		fprintf(stderr, "getblock(%lu): %s\n", block, perror(NULL));
		return(-1);
	}
	return(0);
}

getinode(inum, dp)
register unsigned	inum;
struct dinode		*dp;
{
	register int	i;

	if (debug>1)
		printf("getinode(%u, %o)\n", inum, dp);
	if (inum > (sb->s_isize - 2)*Inopb) {
		printf("getinode: bad inumber %u\n", inum);
		return(-1);
	}
#define ITOD(x)	((long)((((unsigned short)(x)+2*Inopb-1)/Inopb)))
	if (getblock(ITOD(inum), (char *)databuf) < 0)
		return(-1);

	i = ((inum - 1) % Inopb);
	*dp = ((struct dinode *)databuf)[i];
	if (Sflag) {
		swab(&dp->di_size);
	}
	return(0);
}

list(dp, prefix)
register struct dinode	*dp;
register char		*prefix;
{
	int			statval;
	struct direct		*edir;
	long			bn, lbn, llbn;
	char			*buf, nbuf[NAMSIZE];

	if (debug)
		printf("list(%o, %s)\n", dp, prefix);
	if ((buf = malloc(Bsize)) == NULL) {
		fprintf(stderr, "Out of memory\n");
		return(0);
	}
	edir = (struct direct *)&buf[Bsize];
	llbn = dp->di_size/Bsize;
	for (lbn = 0; lbn <= llbn; lbn++) {
		register struct direct	*dir;

		bn = bmap(dp, lbn);
		if (bn == -1L)
			continue;
		if (getblock(bn, buf) == -1)
			continue;
		if (lbn == llbn)
			edir = (struct direct *)&buf[dp->di_size&Bmask];
		for (dir = (struct direct *)buf; dir < edir; dir++) {
			struct dinode dis;
			if (debug>1)
				printf("    %5u %.14s\n", dir->d_ino, dir->d_name);
			if (!dir->d_ino)
				continue;
			if (dir->d_name[0] == '.' &&
			    (dir->d_name[1] == '\0' ||
			     (dir->d_name[1] == '.' &&
			      dir->d_name[2] == '\0')))
				continue;
			sprintf(nbuf, "%s/%.14s", prefix, dir->d_name);
			if (getinode(dir->d_ino, &dis) < 0) {
				printf("bad inode: %o %.14s\n",
						dir->d_ino, dir->d_name);
				continue;
			}
			if (iflag)
				printf("%5u ", dir->d_ino);
			if (lflag)
				dostats(&dis);
			printf("%s\n", nbuf);
			if (recursive && (dis.di_mode&IFMT)==IFDIR)
				list(&dis, nbuf);
		}
	}
	free(buf);
	return(0);
}

dostats(dp)
struct dinode *dp;
{
	register char	*cp;
	char		*ctime(), *getmode();
	long		mtime;

	printf("%s%2d ", getmode(dp->di_mode), dp->di_nlink);
	if (gflag)
		printf("%-8d", dp->di_gid);
	else
		printf("%-8d", dp->di_uid);
	switch(dp->di_mode&IFMT) {
	case IFBLK:
	case IFCHR:
		mtime = laddr(dp->di_addr);
		printf("%3d,%3d", (int)major(mtime), (int)minor(mtime));
		break;
	default:
		printf("%7ld", dp->di_size);
		break;
	}
	mtime = uflag ? dp->di_atime : (cflag ? dp->di_ctime : dp->di_mtime);
	if (Sflag)
		swab(&mtime);
	cp = ctime(&mtime);
	if (mtime < year)
		printf(" %-7.7s %-4.4s ", cp+4, cp+20);
	else
		printf(" %-12.12s ", cp+4);
}

char * getmode(p_mode)
register unsigned short p_mode;
{
	static char a_mode[16];
	register char *p = a_mode;
	register int	i = 0;

	switch (p_mode & IFMT) {
	case IFIFO:	*p++ = 'f'; break;
	case IFDIR:	*p++ = 'd'; break;
	case IFLNK:	*p++ = 'l'; break;
	case IFSOCK:	*p++ = 's'; break;
	case IFCHR:	*p++ = 'c'; break;
	case IFBLK:	*p++ = 'b'; break;
	case IFREG:
	default:	*p++ = (p_mode & ISVTX) ? 't' : '-'; break;
	}
	for (i = 0; i < 3; i++) {
		*p++ = (p_mode<<(3*i) & IREAD) ? 'r' : '-';
		*p++ = (p_mode<<(3*i) & IWRITE) ? 'w' : '-';
		*p++ = (i<2 && (p_mode<<i & ISUID)) ? 's' :
			      ((p_mode<<(3*i) & IEXEC ) ? 'x' : '-');
	}
	*p = '\0';
	return(a_mode);
}
