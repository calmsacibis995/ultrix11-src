
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)mkdir.c	3.0	(ULTRIX-11)	4/22/86
 * mkdir(path, mode) - create a directory with the specified
 * mode.  This is done via execing /bin/mkdir.
 */
#include <sys/errno.h>

mkdir(path, mode)
char *path;
int mode;
{
	register int child, pid;
	int status;
	extern errno;
	int mask;

	switch (child = fork()) {
	case -1:
		return(-1);
	case 0:
		/*
		 * Set our real uid to our effective uid, and our
		 * real gid to our effective gid. This is because
		 * /bin/mkdir runs setuid(0), and we need to keep
		 * permissions correct.
		 */
		setregid(getegid(), -1);
		setreuid(geteuid(), -1);

		/* so we don't get any output on errors */
		close(1);
		close(2);

		execl("/bin/mkdir", "mkdir", path, 0);
		exit(2);
	default:
		while ((pid = wait(&status)) > 0)
			if (pid == child)
				break;
		if (pid < 0 || status == (2<<8)) {
			errno = ENOEXEC;
			return(-1);
		}
		if (status) {
			errno = EACCES;
			return(-1);
		}
		/*
		 * Find out the current umask, modify the mode
		 * accordingly, and set the mode on the newly
		 * created directory.  Note that we can't get
		 * the umask value without changing it, so we
		 * have to do a second umask to restore it.
		 */
		mask = umask(0);
		umask(mask);
		mode &= ~mask;
		if (chmod(path, mode) < 0)
			return(-1);
		return(0);
	}
}
