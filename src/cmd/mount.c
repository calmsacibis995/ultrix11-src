
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * Based on "@(#)mount.c	4.10 (Berkeley) 5/28/83";
 */

static char Sccsid[] = "@(#)mount.c	3.0	4/21/86";
/*
 * mount
 *
 * Fred Canter 7/7/85
 *
 *	Don't mount any file system that does not have the correct
 *	magic numbers in the superblock (see <sys/filsys.h>). Only
 *	ULTRIX-11 1K block file systems can be mounted. The -d flag
 *	will override this check (emergency use only!).
 *
 *	The mount system call now does sanity checking on the file
 *	system size values (s_isize and s_fsize) in the superblock.
 *	This helps prevent mount corrupted an non unix file systems.
 */

#include <sys/param.h>
#include <sys/filsys.h>
#include <stdio.h>
#include <fstab.h>
#include <mtab.h>

#define	DNMAX	(sizeof (mtab[0].m_dname) - 1)
#define	PNMAX	(sizeof (mtab[0].m_path) - 1)

struct	mtab mtab[NMOUNT];

union {
	struct	filsys	fs;
	char	pad2blk[BSIZE];
} filsys;

int	all;
int	ro;
int	fake;
int	verbose;
int	danger;
char	*index(), *rindex();

main(argc, argv)
	int argc;
	char **argv;
{
	register struct mtab *mp;
	register char *np;
	int mf;
	char *type = FSTAB_RW;

	mf = open("/etc/mtab", 0);
	read(mf, (char *)mtab, sizeof (mtab));
	--argc; ++argv;
	if (argc == 0) {
		for (mp = mtab; mp < &mtab[NMOUNT]; mp++)
			if (mp->m_path[0] != '\0')
				prmtab(mp, 0);
		exit(0);
	}
	while (argc && **argv == '-') {
		register char *cp = *argv;
		while (*++cp) {
			switch (*cp) {
			case 'a':
				all++;
				break;
			case 'r':
				type = FSTAB_RO;
				break;
			case 'f':
				fake++;
				break;
			case 'v':
				verbose++;
				break;
			case 'd':
				danger++;
				break;
			default:
				fprintf(stderr, "Unknown flag: %c\n", *cp);
				usage();
			}
		}
		argc--, argv++;
	}
	/*
	 * This is for backwards compatabilty.  We aren't
	 * exactly the same, because the old code didn't
	 * care what the third argument was.
	 */
	if (!strcmp(argv[argc-1], "-r")) {
		type = FSTAB_RO;
		argv[argc-1] = NULL;
		--argc;
	}
	if (all) {
		struct fstab *fsp;

		if (argc)
			usage();
		close(2); dup(1);
		if (setfsent() == 0)
			perror(FSTAB), exit(1);
		while ((fsp = getfsent()) != 0) {
			if (strcmp(fsp->fs_file, "/") == 0)
				continue;
			if (strcmp(fsp->fs_type, FSTAB_RO) &&
			    strcmp(fsp->fs_type, FSTAB_RW))
				continue;
			mountfs(fsp->fs_spec, fsp->fs_file, fsp->fs_type);
		}
		exit(0);
	}
	if (argc == 1) {
		struct fstab *fs;

		if (setfsent() == 0)
			perror(FSTAB), exit(1);
		fs = getfsfile(argv[0]);
		if (fs == NULL)
			usage();
		mountfs(fs->fs_spec, fs->fs_file, type);
		exit(0);
	}
	if (argc != 2)
		usage();
	mountfs(argv[0], argv[1], type);
}

usage()
{
	fprintf(stderr,
	    "usage: mount [ -r ] [ -f ] [ -v ] [ -d ] [ [ special ] dir ]\n");
	fprintf(stderr,
	    "       mount -a [ -r ] [ -f ] [ -v ] [ -d ] \n");
	exit(1);
}

prmtab(mp, flag)
	register struct mtab *mp;
{

	if (flag)
		printf("Mounted /dev/");
	printf("%s on %s", mp->m_dname, mp->m_path);
	if (strcmp(mp->m_type, FSTAB_RO) == 0)
		printf("\t(read-only)");
	putchar('\n');
}

mountfs(spec, name, type)
	char *spec, *name, *type;
{
	register char *np;
	register struct mtab *mp;
	int mf;

	/*
	 * Don't mount unless superblock magic numbers ok.
	 */

	if(!danger) {
	    if((mf = open(spec, 0)) < 0) {
		fprintf(stderr, "\nmount: %s open failed!\n", spec);
		return;
	    }
	    lseek(mf, (long)(SUPERB*BSIZE), 0);
	    if(read(mf, (char *)&filsys, BSIZE) != BSIZE) {
		fprintf(stderr, "\nmount: %s superblock read failed!\n", spec);
		close(mf);
		return;
	    }
	    if((filsys.fs.s_magic[0] != S_0MAGIC) ||
	       (filsys.fs.s_magic[1] != S_1MAGIC) ||
	       (filsys.fs.s_magic[2] != S_2MAGIC) ||
	       (filsys.fs.s_magic[3] != S_3MAGIC)) {
		    fprintf(stderr, "\nmount: not ULTRIX-11 1K file system");
		    fprintf(stderr, " -- use rawfs(8) to access files!\n");
		    close(mf);
		    return;
	    }
	}
	if (!fake) {
		if (mount(spec, name, strcmp(type, FSTAB_RO) == 0) < 0) {
			fprintf(stderr, "%s on ", spec);
			perror(name);
			return;
		}
	}
	np = index(spec, '\0');
	while (*--np == '/')
		*np = '\0';
	np = rindex(spec, '/');
	if (np) {
		*np++ = '\0';
		spec = np;
	}
	for (mp = mtab; mp < &mtab[NMOUNT]; mp++)
		if (strcmp(mp->m_dname, spec) == 0)
			goto replace;
	for (mp = mtab; mp < &mtab[NMOUNT]; mp++)
		if (mp->m_path[0] == '\0')
			goto replace;
	return;
replace:
	strncpy(mp->m_dname, spec, DNMAX);
	mp->m_dname[DNMAX] = '\0';
	strncpy(mp->m_path, name, PNMAX);
	mp->m_path[PNMAX] = '\0';
	strcpy(mp->m_type, type);
	if (verbose)
		prmtab(mp, 1);
	mp = mtab + NMOUNT - 1;
	while (mp > mtab && mp->m_path[0] == '\0')
		--mp;
	mf = creat("/etc/mtab", 0644);
	write(mf, (char *)mtab, (mp - mtab + 1) * sizeof (struct mtab));
	close(mf);
	return;
}
