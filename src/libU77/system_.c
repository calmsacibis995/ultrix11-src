
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)system_.c	3.0	4/22/86
char id_system[] = "(2.9BSD)  system_.c  1.5";
 *
 * execute a unix command
 *
 * calling sequence:
 *	iexit = system(command)
 * where:
 *	iexit will return the exit status of the command
 *	command is a character string containing the command to be executed
 */

#include	"../libI77/fiodefs.h"
#include	<sys/param.h>
#ifndef	NCARGS
#define NCARGS	256
#endif

 
ftnint system_(s, n)
char *s;
ftnlen n;
{
	char buf[NCARGS - 50];
	ftnint i;

	if (n >= sizeof buf)
		return(-(ftnint)(errno=F_ERARG));
	for (i = 0; i < MXUNIT; i++)
		flush_(&i);
	g_char(s, n, buf);
	return((ftnint)system(buf));
}

/*
 * this is a sane version of the libc/stdio routine.
 */

#include	<signal.h>

char	*getenv();
char	*rindex();

system(s)
char *s;
{
	register int (*istat)(), (*qstat)();
	int status, pid, w;
	char	*shname, *shell;

	if ((shell = getenv("SHELL")) == NULL)
		shell = "/bin/sh";

	if (shname = rindex(shell, '/'))
		shname++;
	else
		shname = shell;

	if ((pid = fork()) == 0) {
		execl(shell, shname, "-c", s, 0);
		_exit(127);
	}
	istat = signal(SIGINT, SIG_IGN);
	qstat = signal(SIGQUIT, SIG_IGN);
	while ((w = wait(&status)) != pid && w != -1)
		;
	if (w == -1)
		status = -1;
	signal(SIGINT, istat);
	signal(SIGQUIT, qstat);
	return((ftnint) status);
}
