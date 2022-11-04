
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/


#ifndef lint
static char *sccsid = "@(#)dlogin.c	3.0	(ULTRIX-11)	4/21/86";
#endif

#include <sys/types.h>
#include <signal.h>
#include <sgtty.h>
#include <stdio.h>
#ifdef	pdp11
#define	signal	sigset
#endif	pdp11
#include "dgate.h"

#define CTLQ	021
#define CTLS	023
#define OFF	-1

struct	tchars	otchars;
struct	ltchars oltchars;
struct	tchars	tchars = { OFF, OFF, CTLQ, CTLS, OFF, OFF };
struct	ltchars	ltchars = { OFF, OFF, OFF, OFF, OFF, OFF };
struct	sgttyb	osgb, sgb;
int	cpid;

main(argc, argv, envp)
int	argc;
char	**argv;
char	*envp[];
{
	char	*namebuf[256];
	char	**tp;
	char	gate_way[64];
	char	gate_accnt[64];
	int	cleanup();

	if (argc < 2) {
		fprintf (stderr, "usage: %s hostname\n", argv[0]);
		exit(1);
	}
	
	getgateway(gate_way, gate_accnt);

	tp = namebuf;
	*tp++ = RSH;
	*tp++ = gate_way;
	*tp++ = "-l";
	*tp++ = gate_accnt;
	*tp++ = RDLOGIND;

	argv++;
	while(--argc)
		*tp++ = quote(*argv++);
	*tp = (char *) 0;

	if(isatty(0) != 1) {
		fprintf (stderr, "%s: stdin must be a tty\n", argv[0]);
		exit(1);
	}

	setterm();
	switch(cpid = fork()) {
		case 0 :
			execve (RSH, namebuf, envp);
			fprintf (stderr, "%s: exec of rsh failed\r\n", argv[0]);
			exit(1);
		case -1 :
			fprintf (stderr, "%s: can't fork\n\r", argv[0]);
			resetterm();
			exit(1);
	}
	signal(SIGINT, cleanup);
	signal(SIGHUP, cleanup);
	signal(SIGQUIT, cleanup);
	signal(SIGCHLD, cleanup);
	signal(SIGTSTP, SIG_IGN);

	while (wait((int *) 0) != -1)
		;
	cleanup(0);
}

setterm() {
	ioctl(0, TIOCGETP, &osgb);
	ioctl(0, TIOCGETC, &otchars);
	ioctl(0, TIOCGLTC, &oltchars);
	sgb.sg_ispeed = osgb.sg_ispeed;
	sgb.sg_ospeed = osgb.sg_ospeed;
	sgb.sg_erase = sgb.sg_kill = OFF;
	sgb.sg_flags = (XTABS | CBREAK) & ~ECHO;
	ioctl(0, TIOCFLUSH, (char *) 0);
	ioctl(0, TIOCSETP, &sgb);
	ioctl(0, TIOCSETC, &tchars);
	ioctl(0, TIOCSLTC, &ltchars);
}

resetterm () {
	ioctl(0, TIOCSETP, &osgb);
	ioctl(0, TIOCSLTC, &oltchars);
	ioctl(0, TIOCSETC, &otchars);
}

cleanup(how)
int how;
{
	resetterm();

	if (how) {
		if (how != SIGCHLD) {
			kill(cpid, SIGKILL);
			fprintf (stderr, "\007Lost connection\n");
		}
		wait((int *) 0);	
	}
	fprintf (stderr, "\n");
	exit(how & ~SIGCHLD);
}
