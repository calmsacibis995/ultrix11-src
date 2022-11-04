
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * uuunlock - remove a uucp lockfile
 * usage: uuunlock lockfile
 * It will not remove the lockfile if
 *	1) "lockfile" does not have LOCKPRE as a prefix.
 *	2) "lockfile contains /'s after LOCKPRE.
 *	3) The pid in "lockfile" != our parents pid.
 */
#include <stdio.h>

#define LOCKPRE "/usr/spool/uucp/LCK."
#define SAME 0

main(argc, argv)
int	argc;
char	**argv;
{
	int	pid;
	char	*t;
	FILE	*fd;

	if (argc != 2)
		exit(1);
	++argv;
	if (strncmp(LOCKPRE, *argv, strlen(LOCKPRE)) != SAME)
		exit(2);
	if ((t = index(*argv, 'L')) == NULL)
		exit(3);	/* this can't happen... */
	if (index(t, '/') != NULL)
		exit(4);
	if ((fd = fopen(*argv, "r")) == NULL)
		exit(5);
	if (fread(&pid, sizeof(pid), 1, fd) == 1) {
		if (pid != getppid())
			exit(6);
	}
	if (unlink(*argv) == -1)
		exit(7);
	exit(0);
}
