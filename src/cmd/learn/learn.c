
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)learn.c	3.0	4/21/86";
#include "stdio.h"
#include "lrndef"
#include "lrnref"
#include "signal.h"

main(argc,argv)
char *argv[];
{
	extern hangup(), intrpt();
	extern char * getlogin();
	char *malloc();

	speed = 0;
	more = 1;
	pwline = getlogin();
	setbuf(stdout, malloc(BUFSIZ));
	selsub(argc, argv);
	signal(SIGHUP, hangup);
	signal(SIGINT, intrpt);
	umask(0);
	while (more) {
		selunit();
		dounit();
		whatnow();
	}
	wrapup(0);
}

hangup()
{
	wrapup(1);
}

intrpt()
{
	char response[20], *p;

	signal(SIGINT, hangup);
	write(2, "\nInterrupt.\nWant to go on?  ", 28);
	p = response;
	*p = 'n';
	while (read(0, p, 1) == 1 && *p != '\n')
		p++;
	if (response[0] != 'y')
		wrapup(1);
	ungetc('\n', stdin);
	signal(SIGINT, intrpt);
}
