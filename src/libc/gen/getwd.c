
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*	SCCSID: @(#)getwd.c	3.0	4/22/86	*/
/*	(2.9BSD)  getwd.c  4.8  3/2/83	*/

/*
 * getwd() returns the pathname of the current working directory. On error
 * an error message is copied to pathname and null pointer is returned.
 */
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <sys/file.h>

#ifndef	MAXPATHLEN
#define	MAXPATHLEN	128
#endif

#define CURDIR		"."
#define GETWDERR(s)	strcpy(pathname, (s));
#define PARENTDIR	".."
#define PATHSEP		"/"
#define ROOTDIR		"/"

char *strcpy();
static int pathsize;			/* pathname length */

char *
getwd(pathname)
	char *pathname;
{
	char pathbuf[MAXPATHLEN];		/* temporary pathname buffer */
	char *pnptr = &pathbuf[(sizeof pathbuf)-1]; /* pathname pointer */
	char *prepend();		/* prepend dirname to pathname */
	dev_t rdev;			/* root device number */
	int file;			/* directory file descriptor */
	ino_t rino;			/* root inode number */
	struct direct dir;		/* directory entry struct */
	struct stat d ,dd;		/* file status struct */

	pathsize = 0;
	*pnptr = '\0';
	stat(ROOTDIR, &d);
	rdev = d.st_dev;
	rino = d.st_ino;
	for (;;) {
		stat(CURDIR, &d);
		if (d.st_ino == rino && d.st_dev == rdev)
			break;		/* reached root directory */
		if ((file = open(PARENTDIR, O_RDONLY)) < 0) {
			GETWDERR("getwd: can't open ..");
			goto fail;
		}
		if (chdir(PARENTDIR) < 0) {
			GETWDERR("getwd: can't chdir to ..");
			goto fail;
		}
		fstat(file, &dd);
		if (d.st_dev == dd.st_dev) {
			if (d.st_ino == dd.st_ino) {
				/* reached root directory */
				close(file);
				break;
			}
			do {
				if (read(file, (char *) &dir, sizeof(dir)) < sizeof(dir)) {
					close(file);
					GETWDERR("getwd: read error in ..");
					goto fail;
				}
			} while (dir.d_ino != d.st_ino);
		} else
			do {
				if (read(file, (char *) &dir, sizeof(dir)) < sizeof(dir)) {
					close(file);
					GETWDERR("getwd: read error in ..");
					goto fail;
				}
				stat(dir.d_name, &dd);
			} while(dd.st_ino != d.st_ino || dd.st_dev != d.st_dev);
		close(file);
		pnptr = prepend(PATHSEP, prepend(dir.d_name, pnptr));
	}
	if (*pnptr == '\0')		/* current dir == root dir */
		strcpy(pathname, ROOTDIR);
	else {
		strcpy(pathname, pnptr);
		if (chdir(pnptr) < 0) {
			GETWDERR("getwd: can't change back to .");
			return (NULL);
		}
	}
	return (pathname);

fail:
	chdir(prepend(CURDIR, pnptr));
	return (NULL);
}

/*
 * prepend() tacks a directory name onto the front of a pathname.
 */
static char *
prepend(dirname, pathname)
	register char *dirname;
	register char *pathname;
{
	register int i;			/* directory name size counter */

	for (i = 0; *dirname != '\0'; i++, dirname++)
		continue;
	if ((pathsize += i) < MAXPATHLEN)
		while (i-- > 0)
			*--pathname = *--dirname;
	return (pathname);
}
