
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * Based on "@(#)df.c	4.6 (Berkeley) 7/8/81";
 */

static char Sccsid[] = "@(#)df.c	3.0	4/21/86";

#include <sys/param.h>
#include <stdio.h>
#include <fstab.h>
#include <mtab.h>
#include <sys/filsys.h>
#include <sys/ino.h>
#include <sys/fblk.h>
#include <sys/stat.h>
/*
 * df
 */

#define NFS	20	/* Max number of filesystems */

struct mtab mtab[NFS];
char root[32];

char *mpath();

daddr_t	blkno	= 1;

int	lflag;
int	iflag;

struct	filsys sblock;

int	fi;
daddr_t	alloc();

main(argc, argv)
register char **argv;
{
	register i;

	while (argc >= 1 && argv[1][0]=='-') {
		switch(argv[1][1]) {

		case 'l':
			lflag++;
			break;

		case 'i':
			iflag++;
			break;

		default:
			fprintf(stderr, "usage: df [ -il ] [ filsys... ]\n");
			exit(0);
		}
		argc--, argv++;
	}

	if ((i=open("/etc/mtab", 0)) >= 0) {
		read(i, (char *) mtab, sizeof mtab);	/* Probably returns short */
		close(i);
	}
	/*
	 * print top part of title.
	 */
	printf("Filesystem    total    kbytes  kbytes");
	if (lflag)
		printf("        ");
	printf("  percent");
	if (iflag)
		printf(" inodes  inodes percent");
	printf("\n");
	/*
	 * print second part of titles.
	 */
	printf("   node       kbytes    used    free");
	if (lflag)
		printf(" hardway");
	printf("   used  ");
	if (iflag)
		printf("   used    free   used");
	printf("  Mounted on\n");
	if(argc <= 1) {
		struct	fstab	*fsp;
		if (setfsent() == 0)
			perror(FSTAB), exit(1);
		while( (fsp = getfsent()) != 0){
			if (  (strcmp(fsp->fs_type, FSTAB_RW) != 0)
			    &&(strcmp(fsp->fs_type, FSTAB_RO) != 0) )
				continue;
			if (root[0] == 0)
				strcpy(root, fsp->fs_spec);
			dfree(fsp->fs_spec, 1);
		}
		endfsent();
		exit(0);
	}

	for(i=1; i<argc; i++) {
		dfree(argv[i], 0);
	}
}

dfree(file, infsent)
char *file;
int infsent;
{
	long	blocks;
	long	free;
	long	used;
	long	hardway;
	char	*mp;
	struct	stat stbuf;

	/*
	 * If this isn't a block or character device, then do the df on
	 * the device that the file resided on.  Determine this by finding
	 * the device in /etc/mtab, if we can't find it there then check
	 * to see if it is listed in /etc/fstab.
	 */
	if (stat(file, &stbuf) == 0 &&
	    (stbuf.st_mode&S_IFMT) != S_IFCHR &&
	    (stbuf.st_mode&S_IFMT) != S_IFBLK) {
		int mt, len;
		char *str = "/dev/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
		struct mtab mtb;
		struct stat stb;

		if ((mt = open("/etc/mtab", 0)) < 0) {
			perror("/etc/mtab");
			return;
		}
		while ((len = read(mt, &mtb, sizeof(mtb))) == sizeof(mtb)) {
			strcpy(&str[5], mtb.m_dname);
			if(stat(str, &stb) == 0 && stb.st_rdev == stbuf.st_dev) {
				file = str;
				break;
			}
		}
		close(mt);
		if(len == 0) {
			struct	fstab	*fsp;
			if (infsent) {
				fprintf(stderr, "%s: screwy /etc/fstab entry\n", file);
				return;
			}
			setfsent();
			while (fsp = getfsent()) {
				if (stat(fsp->fs_spec, &stb) == 0 &&
				    stb.st_rdev == stbuf.st_dev) {
					file = fsp->fs_spec;
					break;
				}
			}
			endfsent();
			if (fsp == NULL) {
				fprintf(stderr, "%s: mounted on unknown device\n", file);
				return;
			}
		}
	}
	fi = open(file, 0);
	if(fi < 0) {
		fprintf(stderr,"cannot open %s\n", file);
		perror(file);
		return;
	}
	if (lflag)
		sync();
	bread(1L, (char *)&sblock, sizeof(sblock));
	printf("%-12.12s", file);

	blocks = (long) sblock.s_fsize - (long)sblock.s_isize;
	free = sblock.s_tfree;
	used = blocks - free;

	printf("%8ld", (blocks*BSIZE)/1024);
	printf("%8ld", (used*BSIZE)/1024);
	printf("%8ld", (free*BSIZE)/1024);
	if (lflag) {
		hardway = 0;
		while(alloc())
			hardway++;
		printf("%8ld", ((free=hardway)*BSIZE)/1024);
	}
	printf("%6.0f%%", 
	    blocks == 0L ? 0.0 : (double) used / (double)blocks * 100.0);
	if (iflag) {
		long inodes = (long)(sblock.s_isize-2)*((long)INOPB);
		used = inodes - sblock.s_tinode;
		printf(" %8ld%8u%6.0f%%", used, sblock.s_tinode, 
		    inodes == 0L ? 0.0 : (double)used/(double)inodes*100.0);
	} else
		printf("  ");
	printf("  %s\n", mp = mpath(file));
	close(fi);
}

daddr_t
alloc()
{
	int i;
	daddr_t b;
	struct fblk buf;

	i = --sblock.s_nfree;
	if(i<0 || i>=NICFREE) {
		printf("bad free count, b=%D\n", blkno);
		return(0);
	}
	b = sblock.s_free[i];
	if(b == 0)
		return(0);
	if(b<sblock.s_isize || b>=sblock.s_fsize) {
		printf("bad free block (%D)\n", b);
		return(0);
	}
	if(sblock.s_nfree <= 0) {
		bread(b, (char *)&buf, sizeof(buf));
		blkno = b;
		sblock.s_nfree = buf.df_nfree;
		for(i=0; i<NICFREE; i++)
			sblock.s_free[i] = buf.df_free[i];
	}
	return(b);
}

bread(bno, buf, cnt)
daddr_t bno;
char *buf;
{
	int n;
	extern errno;

	lseek(fi, bno<<BSHIFT, 0);
	if((n=read(fi, (char *) buf, cnt)) != cnt) {
		printf("\nread error bno = %ld\n", bno);
		printf("count = %d; errno = %d\n", n, errno);
		exit(0);
	}
}

/*
 * Given a name like /dev/rrp0h, returns the mounted path, like /usr.
 */
char *mpath(file)
char *file;
{
	register int i;

	if (eq(file, root))
		return "/";
	for (i=0; i<NFS; i++)
		if (eq(file, mtab[i].m_dname))
			return mtab[i].m_path;
	return "";
}

eq(f1, f2)
char *f1, *f2;
{
	if (strncmp(f1, "/dev/", 5) == 0)
		f1 += 5;
	if (strncmp(f2, "/dev/", 5) == 0)
		f2 += 5;
	if (strcmp(f1, f2) == 0)
		return 1;
	if (*f1 == 'r' && strcmp(f1+1, f2) == 0)
		return 1;
	if (*f2 == 'r' && strcmp(f1, f2+1) == 0)
		return 1;
	if (*f1 == 'r' && *f2 == 'r' && strcmp(f1+1, f2+1) == 0)
		return 1;
	return 0;
}
