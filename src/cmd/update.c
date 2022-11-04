
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * Update the file system every 30 seconds.
 * For cache benefit, open certain system directories.
 */

static char Sccsid[] = "@(#)update.c 3.0 4/22/86";
#include <signal.h>

char *fillst[] = {
	"/bin",
	"/usr",
	"/usr/bin",
	0,
};

main()
{
	char **f;

	if(fork())
		exit(0);
	close(0);
	close(1);
	close(2);
	for(f = fillst; *f; f++)
		open(*f, 0);
	dosync();
	for(;;)
		pause();
}

dosync()
{
	sync();
	signal(SIGALRM, dosync);
	alarm(30);
}
