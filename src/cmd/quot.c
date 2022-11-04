
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * ULTIRX-11 disk usage by user
 *
 * Modified to use the disk name,
 * from /etc/fstab, that is mounted on /usr
 * as the default file name.
 *
 * Fred Canter 3/14/82
 *
 * Modified for new /etc/fstab, and to use the fstab
 * library routines. Dave Borman 2/3/85
 */


static char Sccsid[] = "@(#)quot.c 3.0 4/22/86";
#include <stdio.h>
#include <ctype.h>
#include <pwd.h>
#include <sys/param.h>
#include <sys/ino.h>
#include <sys/inode.h>
#include <sys/filsys.h>
#include <fstab.h>

#define	ITABSZ	256
#define	ISIZ	(BSIZE/sizeof(struct dinode))
#define	NUID	1000
struct	filsys	sblock;
struct	dinode	itab[ITABSZ];
struct du
{
	long	kbytes;
	long	nfiles;
	char	*name;
} du[NUID];
#define	TSIZE	500
int	sizes[TSIZE];
long	overflow;

int	nflg;
int	fflg;
int	cflg;
int	checked;

int	fi;
unsigned	ino;
unsigned	nfiles;

struct	passwd	*getpwent();
char	*malloc();
char	*copy();

char	*fstab = FSTAB;
char	fsname[50];
char	dn[32];

main(argc, argv)
char **argv;
{
	register int n;
	register struct passwd *lp;
	register char *p;
	FILE *fp;
	struct fstab *fsp;

	while((lp=getpwent()) != 0) {
		n = lp->pw_uid;
		if (n>NUID)
			continue;
		if(du[n].name)
			continue;
		du[n].name = copy(lp->pw_name);
	}
	while (--argc) {
		argv++;
		if (argv[0][0]=='-') {
			if (argv[0][1]=='n')
				nflg++;
			else if (argv[0][1]=='f')
				fflg++;
			else if (argv[0][1]=='c')
				cflg++;
		} else {
			check(*argv);
			report();
			checked = 1;
		}
	}
	if (!checked) {
		/*
		 * We haven't checked anything yet, so try
		 * for the default file system.
		 */
		if (setfsent() == NULL) {
			fprintf(stderr, "cannot open %s\n", fstab);
			exit(1);
		}
		while ((fsp = getfsent()) != NULL) {
			if (strcmp(fsp->fs_file, "/usr"))
				continue;
			if (!strcmp(fsp->fs_type, FSTAB_RW) &&
			    !strcmp(fsp->fs_type, FSTAB_RO))
				continue;
			sprintf(dn, "/dev/r%s", fsp->fs_spec + 5);
			break;
		}
		endfsent();
		if(fsp == NULL) {	/* /usr not mounted on */
			printf("\n/usr not mounted, must give file system\n");
			exit(1);
		}
		check(dn);
		report();
	}
	return(0);
}

check(file)
char *file;
{
	register unsigned i, j;
	register c;

	fi = open(file, 0);
	if (fi < 0) {
		printf("cannot open %s\n", file);
		return;
	}
	printf("%s:\n", file);
	sync();
	bread(1, (char *)&sblock, sizeof sblock);
	nfiles = (sblock.s_isize-2)*(BSIZE/sizeof(struct dinode));
	ino = 0;
	if (nflg) {
		if (isdigit(c = getchar()))
			ungetc(c, stdin);
		else while (c!='\n' && c != EOF)
			c = getchar();
	}
	for(i=2; ino<nfiles; i += ITABSZ/ISIZ) {
		bread(i, (char *)itab, sizeof itab);
		for (j=0; j<ITABSZ && ino<nfiles; j++) {
			ino++;
			acct(&itab[j]);
		}
	}
}

acct(ip)
register struct dinode *ip;
{
	register n;
	register char *np;
	static fino;

	if ((ip->di_mode&IFMT) == 0)
		return;
	if (cflg) {
		if ((ip->di_mode&IFMT)!=IFDIR && (ip->di_mode&IFMT)!=IFREG)
			return;
		n = (ip->di_size+1023)/1024;
		if (n >= TSIZE) {
			overflow += n;
			n = TSIZE-1;
		}
		sizes[n]++;
		return;
	}
	if (ip->di_uid >= NUID)
		return;
	du[ip->di_uid].kbytes += (ip->di_size+1023)/1024;
	du[ip->di_uid].nfiles++;
	if (nflg) {
	tryagain:
		if (fino==0)
			if (scanf("%d", &fino)<=0)
				return;
		if (fino > ino)
			return;
		if (fino<ino) {
			while ((n=getchar())!='\n' && n!=EOF)
				;
			fino = 0;
			goto tryagain;
		}
		if (np = du[ip->di_uid].name)
			printf("%.7s	", np);
		else
			printf("%d	", ip->di_uid);
		while ((n = getchar())==' ' || n=='\t')
			;
		putchar(n);
		while (n!=EOF && n!='\n') {
			n = getchar();
			putchar(n);
		}
		fino = 0;
	}
}

bread(bno, buf, cnt)
unsigned bno;
char *buf;
{

	lseek(fi, (long)bno*BSIZE, 0);
	if (read(fi, buf, cnt) != cnt) {
		printf("read error %u\n", bno);
		exit(1);
	}
}

qcmp(p1, p2)
register struct du *p1, *p2;
{
	if (p1->kbytes > p2->kbytes)
		return(-1);
	if (p1->kbytes < p2->kbytes)
		return(1);
	if (p1->name && p2->name)
		return(strcmp(p1->name, p2->name));
	if (p1 < p2)
		return(-1);
	return (p1 > p2);
}

report()
{
	register i;

	if (nflg)
		return;
	if (cflg) {
		long t = 0;
		for (i=0; i<TSIZE-1; i++)
			if (sizes[i]) {
				t += i*sizes[i];
				printf("%d	%d	%D\n", i, sizes[i], t);
			}
		printf("%d	%d	%D\n",
			TSIZE-1, sizes[TSIZE-1], overflow+t);
		return;
	}
	qsort(du, NUID, sizeof(du[0]), qcmp);
	for (i=0; i<NUID; i++) {
		if (du[i].kbytes==0)
			return;
		printf("%5D\t", du[i].kbytes);
		if (fflg)
			printf("%5D\t", du[i].nfiles);
		if (du[i].name)
			printf("%s\n", du[i].name);
		else
			printf("#%d\n", i);
	}
}

/*
 * This copy routine used to just call malloc for
 * each string. Since malloc has overhead for each
 * space allocated, and since we'll never free
 * this space, this is more effecient for lots of
 * small strings.
 */
char *
copy(s)
char *s;
{
	register char *p;
	register n;
	static	freec = 0;
	static	freep = NULL;

	n = strlen(s)+1;
	if (freec < n) {
		if ((freep = malloc((unsigned)512)) != NULL)
			freec = 512;
	}
	if (p = freep) {
		freep += n;
		freec -= n;
		strcpy(p, s);
	}
	return(p);
}
