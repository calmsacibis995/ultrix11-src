
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)du.c	3.0	4/21/86";
#include <stdio.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/dir.h>
#define EQ(x,y)	(strcmp(x,y)==0)
#define ML	1000
#define	NAMSIZ	256

struct stat Statb;
char	path[NAMSIZ], name[NAMSIZ], cwdpath[NAMSIZ];
int	Aflag = 0,
	Sflag = 0,
	Xflag = 0,
	Noarg = 0;
dev_t	startdev;
struct {
	int	dev,
		ino;
} ml[ML];

long	descend();
char	*rindex();
char	*strcpy();
FILE	*cwdfd;

main(argc, argv)
char **argv;
{
	register	i = 1;
	long	blocks = 0;
	register char	*np;

	if (argc>1) {
		if(EQ(argv[i], "-s")) {
			++i;
			++Sflag;
		} else if(EQ(argv[i], "-x")) {
			++i;
			++Xflag;
		} else if(EQ(argv[i], "-a")) {
			++i;
			++Aflag;
		}
	}
	if(i == argc)
		++Noarg;

	if ((cwdfd = popen("pwd", "r")) == NULL) {
		perror("du: pwd"); /* no more processes, no access to shell, etc */
		exit(1);
	}
	if (fgets(cwdpath, NAMSIZ, cwdfd) == NULL) {
		perror("du: pwd");		/* unexpected EOF, or error */
		exit(1);
	}
	if (pclose(cwdfd) < 0)
		perror("du: pwd");
	if ((np = rindex(cwdpath, '\n')) != NULL)	/* zap off '/n' */
		*np = '\0';
	do {
		if (chdir(cwdpath) == -1) {
			fprintf(stderr, "\ncannot chdir() to %s\n",cwdpath);
			exit(1);
		}
		strcpy(path, Noarg? ".": argv[i]);
		strcpy(name, path);
		if(np = rindex(name, '/')) {
			*np++ = '\0';
			if(chdir(*name? name: "/") == -1) {
				fprintf(stderr, "cannot chdir()\n");
				exit(1);
			}
		} else
			np = path;
		if (Xflag) {
			struct stat statb;
#ifdef	UCB_SYMLINKS
			if (lstat(*np ? np : ".", &statb) < 0)
#else
			if (stat(*np ? np : ".", &statb) < 0)
#endif
			{
				perror(*np ? np : ".");
				exit(1);
			}
			startdev = statb.st_dev;
		}
		blocks = descend(path, *np? np: ".");
		if(Sflag)
			printf("%ld	%s\n", blocks, path);
	} while(++i < argc);

	exit(0);
}

long
descend(np, fname)
char *np, *fname;
{
	int dir = 0, /* open directory */
		offset,
		dsize,
		entries,
		dirsize;

#ifdef	UCB_NKB
	struct direct dentry[BSIZE/sizeof(struct direct)];
#else
	struct direct dentry[32];
#endif	UCB_NKB
	register  struct direct *dp;
	register char *c1, *c2;
	int i;
	char *endofname;
	long blocks = 0;

#ifdef	UCB_SYMLINKS
	if(lstat(fname,&Statb)<0)
#else
	if(stat(fname,&Statb)<0)
#endif
	{
		fprintf(stderr, "--bad status < %s >\n", name);
		return 0L;
	}
	if(Statb.st_nlink > 1 && (Statb.st_mode&S_IFMT)!=S_IFDIR) {
		static linked = 0;

		for(i = 0; i <= linked; ++i) {
			if(ml[i].ino==Statb.st_ino && ml[i].dev==Statb.st_dev)
				return 0;
		}
		if (linked < ML) {
			ml[linked].dev = Statb.st_dev;
			ml[linked].ino = Statb.st_ino;
			++linked;
		}
	}
	blocks = (Statb.st_size + BSIZE-1) >> BSHIFT;

	if((Statb.st_mode&S_IFMT)!=S_IFDIR) {
		if(Aflag)
			printf("%ld	%s\n", blocks, np);
		return(blocks);
	}
	if (Xflag && Statb.st_dev != startdev) {
		/*
		 * Assume file system is mounted on an empty directory
		 * so the directory only takes up 1 disk block
		 */
		if (!Sflag)
			printf("%ld\t%s -- mounted file system\n", 1L, np);
		return (1L);
	}

	for(c1 = np; *c1; ++c1);
	if(*(c1-1) == '/')
		--c1;
	endofname = c1;
	dirsize = Statb.st_size;
	if(chdir(fname) == -1)
		return 0;
#ifdef	UCB_NKB
	for(offset=0; offset < dirsize; offset += BSIZE) { /* each block */
		dsize = BSIZE<(dirsize-offset)? BSIZE: (dirsize-offset);
#else
	for(offset=0; offset < dirsize; offset += 512) { /* each block */
		dsize = 512<(dirsize-offset)? 512: (dirsize-offset);
#endif	UCB_NKB
		if(!dir) {
			if((dir=open(".",0))<0) {
				fprintf(stderr, "--cannot open < %s >\n",
					np);
				goto ret;
			}
			if(offset) lseek(dir, (long)offset, 0);
			if(read(dir, (char *)dentry, dsize)<0) {
				fprintf(stderr, "--cannot read < %s >\n",
					np);
				goto ret;
			}
			if(dir > 10) {
				close(dir);
				dir = 0;
			}
		} else 
			if(read(dir, (char *)dentry, dsize)<0) {
				fprintf(stderr, "--cannot read < %s >\n",
					np);
				goto ret;
			}
		for(dp=dentry, entries=dsize>>4; entries; --entries, ++dp) {
			/* each directory entry */
			if(dp->d_ino==0
			|| EQ(dp->d_name, ".")
			|| EQ(dp->d_name, ".."))
				continue;
			c1 = endofname;
			*c1++ = '/';
			c2 = dp->d_name;
			for(i=0; i<DIRSIZ; ++i)
				if(*c2)
					*c1++ = *c2++;
				else
					break;
			*c1 = '\0';
			if(c1 == endofname) /* ?? */
				return 0L;
			blocks += descend(np, endofname+1);
		}
	}
	*endofname = '\0';
	if(!Sflag)
		printf("%ld	%s\n", blocks, np);
ret:
	if(dir)
		close(dir);
	if(chdir("..") == -1) {
		*endofname = '\0';
		fprintf(stderr, "Bad directory <%s>\n", np);
		while(*--endofname != '/');
		*endofname = '\0';
		if(chdir(np) == -1)
			exit(1);
	}
	return(blocks);
}
