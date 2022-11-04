
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)rename.c	3.0	4/22/86
 */

#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/dir.h>
#include <stdio.h>
extern errno;

/*
 * rename - change the name of a file.
 * What a beast...
 */
rename(from_p, to_p)
char *from_p, *to_p;
{
	char from[1024], to[1024];
	struct stat fromstat, tostat, pfromstat;
	register char *cp;
	register int i;
	int (*savesig[3])();

	savesig[0] = signal(SIGHUP, SIG_IGN);
	savesig[1] = signal(SIGINT, SIG_IGN);
	savesig[2] = signal(SIGQUIT, SIG_IGN);

	if (abspath(from_p, from, 1024) == NULL
	    || abspath(to_p, to, 1024) == NULL) {
		errno = ENAMETOOLONG;
		return(-1);
	}
	/* Can we access this file? */
	if (stat(from, &fromstat) < 0)
		goto bad;
	/* If it's a directory, are we root? */
	if ((fromstat.st_mode & S_IFMT) == S_IFDIR) {
		if (geteuid()) {
			errno = EPERM;
			goto bad;
		}
		/* if the source is "." or "..", we bomb out right away */
		cp = rindex(from_p, '/');
		if (cp == NULL)
			cp = from_p;
		else
			cp++;
		if ((cp[0] == '.') &&
		    ((cp[1] == '\0') ||
		     ((cp[1] == '.') && (cp[2] == '\0')))) {
			errno = EPERM;
			goto bad;
		}
	}

	if (is_parent_writeable(from, &pfromstat) < 0)
		goto bad;

	if (is_parent_writeable(to, &tostat) < 0)
		goto bad;

	/* Are the source and destination on the same device? */
	if (pfromstat.st_dev != tostat.st_dev) {
		errno = EXDEV;
		goto bad;
	}
	/* just make sure the "to" is not a "." or ".." entry... */
	cp = rindex(to_p, '/');
	if (cp == NULL)
		cp = to_p;
	if ((cp[0] == '.') &&
	    ((cp[1] == '\0') ||
	     ((cp[1] == '.') && (cp[2] == '\0')))) {
		errno = EPERM;
		goto bad;
	}

	/* does the "to file exist? "*/
	if (stat(to, &tostat) == 0) {
		if ((tostat.st_mode & S_IFMT) == S_IFDIR) {
			if ((fromstat.st_mode & S_IFMT) != S_IFDIR) {
				errno = EPERM;
				goto bad;
			}
			if (rmdir(to) < 0)
				goto bad;
		} else if ((fromstat.st_mode & S_IFMT) == S_IFDIR) {
			errno = EPERM;
			goto bad;
		} else if (unlink(to) < 0)
			goto bad;
	}
	if ((fromstat.st_mode & S_IFMT) != S_IFDIR) {
		if ((link(from, to) < 0) || (unlink(from) < 0))
			goto bad;
		goto done;
	}

	/* gag, we are re-naming a directory... */
	if (isparent(from, to)) {
		errno = EINVAL;
		goto bad;
	}
	if (mvdir(from, to) < 0)
		goto bad;
done:
	signal(SIGHUP, savesig[0]);
	signal(SIGINT, savesig[1]);
	signal(SIGQUIT, savesig[2]);
	return(0);
bad:
	signal(SIGHUP, savesig[0]);
	signal(SIGINT, savesig[1]);
	signal(SIGQUIT, savesig[2]);
	return(-1);
}

static is_parent_writeable(file, statbuf)
char *file;
register struct stat *statbuf;
{
	register char *cp;
	register int i;

	if ((cp = rindex(file, '/')) == NULL) {
		if (stat(".", statbuf) < 0)
			return(-1);
	} else {
		*cp = '\0';
		i = stat(file, statbuf);
		*cp = '/';
		if (i < 0)
			return(-1);
	}
	if (geteuid()) {
		i = S_IWRITE;
		if (statbuf->st_uid != geteuid()) {
			i >>= 3;
			if (statbuf->st_gid != getegid())
				i >>=3;
		}
		if ((statbuf->st_mode & i) == 0) {
			errno = EACCES;
			return(-1);
		}
	}
	return(0);
}

static rmdir(dir)
register char *dir;
{
	char buf[1024];
	register char *p1, *p2;

	if (isempty(dir) < 0)
		return(-1);
	if (strlen(dir) > 1020) {
		errno = EINVAL;
		return(-1);
	}
	for (p1 = buf, p2 = dir; *p2; )
		*p1++ = *p2++;
	*p1++ = '/';
	*p1++ = '.';
	*p1 = '\0';
	if (unlink(buf) < 0)
		return(-1);
	*p1++ = '.';
	*p1 = '\0';
	if (unlink(buf) || unlink(dir))
		return(-1);
}

static isempty(dir)
char *dir;
{
	register int fd;
	struct direct dent;
	register struct direct *dp = &dent;

	if ((fd = open(dir, 0)) < 0)
		return(-1);
	while (read(fd, (char *)dp, sizeof(*dp)) > 0) {
		if (dp->d_ino == 0)
			continue;
		if ((dp->d_name[0] == '.') &&
		    ((dp->d_name[1] == '\0') ||
		     ((dp->d_name[1] == '.') && (dp->d_name[2] == '\0'))))
			continue;
		errno = EPERM;
		close(fd);
		return(-1);
	}
	close(fd);
	return(0);
}

static mvdir(from, to)
char *from, *to;
{
	register char *p1, *p2;
	register val;
	char c;
	char buf[1024];

	/* make new link, the remove the old */
	if ((link(from, to) < 0) || (unlink(from) < 0))
		return(-1);

	/* fix ".." so that it points to the new director */
	for (p1 = buf, p2 = to; *p2; )
		*p1++ = *p2++;
	*p1++ = '/';
	*p1++ = '.';
	*p1++ = '.';
	*p1 = '\0';
	p1 = rindex(to, '/');
	if (p1 == to)
		p1++;
	if (p1) {
		c = *p1;
		*p1 = '\0';
	}
	if ((unlink(buf) < 0) || (link(to, buf) < 0))
		val = -1;
	else
		val = 0;
	if (p1)
		*p1 = c;
	return(val);
}

static abspath(path, buf, size)
char *path;
register char *buf;
int size;
{
	register char *s, *p;
	s = buf;
	if (*path != '/') {
		if (getcwd(buf, size) == NULL)
			return(NULL);
		while (*s)
			s++;
	}
	for (p = path; *p; ) {
		/* skip multiple slashes */
		while (*p == '/')
			p++;
		if (p[0] == '.') {
			if (p[1] == '/' || p[1] == '\0') {
				/* . entries are ignored */
				p++;
				continue;
			} else if (p[1] == '.' &&
					(p[2] == '/' || p[2] == '\0')) {
				/* .. entry, skip back in path */
				p += 2;
				while (s > buf && *--s != '/')
					;
				continue;
			}
		}
		/* add the next path unit */
		if (*p) {
			*s++ = '/';
			while (*p && *p != '/') {
				*s++ = *p++;
				if (s > &buf[size])
					return(NULL);
			}
		}
	}
	if (s == buf)
		*s++ = '/';
	*s = '\0';
	return(buf);
}

static isparent(from, to)
register char *from, *to;
{
	while (*from++ == *to++)
		;
	if (*--from == '\0')
		return(1);
	return(0);
}
