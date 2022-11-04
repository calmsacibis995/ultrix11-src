
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/


/*
 * SCCSID: @(#)rmdir.c	3.0	(ULTRIX-11)	4/22/86
 * rmdir(path) - remove a directory.
 * This is done via execing /bin/rmdir.
 */
#include <sys/errno.h>

rmdir(path)
char *path;
{
	register int child, pid;
	int status;
	extern errno;

	switch (child = fork()) {
	case -1:
		return(-1);
	case 0:
		/*
		 * Set our real uid to our effective uid, and our
		 * real gid to our effective gid. This is because
		 * /bin/rmdir runs setuid(0), and we need to keep
		 * permissions correct.
		 */
		setregid(getegid(), -1);
		setreuid(geteuid(), -1);

		/* so that we don't get output on errors */
		close(1);
		close(2);

		execl("/bin/rmdir", "rmdir", path, 0);
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
		return(0);
	}
}
