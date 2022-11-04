
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * Stat - print out vital STATistics on a file
 *
 * stat file1 [ file2 ... ]
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

static char Sccsid[] = "@(#)stat.c	3.0	4/22/86";

char buf[BUFSIZ];

main(argc, argv)
int argc;
char *argv[];
{
	struct stat statb;
	register struct stat *st = &statb;
	struct passwd *getpwuid(), *pw;
	struct group *getgrgid(), *gr;
	char *getmode(), *ctime();
	int i;
	char linknm[512];
	int errflag = 0;

	setbuf(stdout, buf);

	if (argc < 2) {
		fprintf(stderr, "usage: stat file1 [ file2 ... ]\n");
		++errflag;
	}
	while (--argc > 0) {
#ifdef	S_IFLNK
		if (lstat(*++argv, st) == -1) {
#else
		if (stat(*++argv, st) == -1) {
#endif	S_IFLNK
			fprintf(stderr,"%s: status unavailable\n", *argv);
			++errflag;
			continue;
		}
		printf("\nName: %s", *argv);
#ifdef	S_IFLNK
		if ((st->st_mode & S_IFMT) == S_IFLNK) {
			i = readlink(*argv, linknm, 512);
			if (i != -1)
				printf(" -> %*s", i, linknm);
			else
				printf(" -> ????");
		}
#endif	S_IFLNK
		printf("\nDev: %d/%d\tInode: %d", major(st->st_dev),
			minor(st->st_dev), st->st_ino);
		switch(st->st_mode & S_IFMT) {
		case S_IFDIR:
#ifdef S_IFIFO
		case S_IFIFO:
#endif S_IFIFO
		case S_IFREG:
			printf("\tSize: %lu\n",st->st_size);
			break;
		default:
			printf("\tRdev: %d/%d\n",
				major(st->st_rdev), minor(st->st_rdev));
		}
		printf("Mode: %04.4o %s\tLinks: %d\n", st->st_mode,
			getmode(st->st_mode), st->st_nlink);
		printf("Owner: %d", st->st_uid);
		if ((pw = getpwuid(st->st_uid)) != NULL)
			printf(" %s", pw->pw_name);
		printf("\tGroup: %d", st->st_gid);
		if ((gr = getgrgid(st->st_gid)) != NULL)
			printf(" %s", gr->gr_name);
		printf("\nAccessed: %s", ctime(&st->st_atime));
		printf("Modified: %s", ctime(&st->st_mtime));
		printf("Created:  %s", ctime(&st->st_ctime));
		fflush(stdout);
	}
	exit(errflag);
}

char	*
getmode(p_mode)
unsigned short p_mode;
{
	static char a_mode[16];
	register char *p = a_mode;
	register int	i = 0;

	*p++ = ' ';

	switch (p_mode & S_IFMT) {
	case S_IFDIR:	*p++ = 'd'; break;
#ifdef S_IFIFO		/* defined is stats.h if you have fifo's */
	case S_IFIFO:	*p++ = 'p'; break;
#endif S_IFIFO
#ifdef	S_IFLNK		/* defined is stats.h if you have symbolic links */
	case S_IFLNK:	*p++ = 'l'; break;
#endif	S_IFLNK
#ifdef	S_IFSOCK	/* defined in stats.h if you have sockets */
	case S_IFSOCK:	*p++ = 's'; break;
#endif	S_IFSOCK
#ifdef	S_IFMPC		/* defined in stats.h if you have MPX files */
	case S_IFMPC:	*(p-1) = 'm'; *p++ = 'c'; break;
#endif	S_IFMPC
#ifdef	S_IFMPB		/* defined in stats.h if you have MPX files */
	case S_IFMPB:	*(p-1) = 'm'; *p++ = 'b'; break;
#endif	S_IFMPB
	case S_IFCHR:	*p++ = 'c'; break;
	case S_IFBLK:	*p++ = 'b'; break;
	case S_IFREG:
	default:	*p++ = (p_mode & S_ISVTX) ? 't' : ' '; break;
	}
	*p++ = ' ';
	for (i = 0; i < 3; i++) {
		*p++ = (p_mode<<(3*i) & S_IREAD) ? 'r' : '-';
		*p++ = (p_mode<<(3*i) & S_IWRITE) ? 'w' : '-';
		*p++ = (i<2 && (p_mode<<i & S_ISUID)) ? 's' :
			      ((p_mode<<(3*i) & S_IEXEC ) ? 'x' : '-');
		*p++ = ' ';
	}
	*p = '\0';
	return(a_mode);
}
