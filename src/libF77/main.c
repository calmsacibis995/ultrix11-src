
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)main.c	3.0	4/22/86
 */
/* STARTUP PROCEDURE FOR UNIX FORTRAN PROGRAMS */

#include <stdio.h>
#include <signal.h>

int xargc;
char **xargv;

main(argc, argv, arge)
int argc;
char **argv;
char **arge;
{
int sigfdie(), sigidie(), sigqdie(), sigindie(), sigtdie();

xargc = argc;
xargv = argv;
signal(SIGFPE, sigfdie);	/* ignore underflow, enable overflow */
signal(SIGIOT, sigidie);
if( (int)signal(SIGQUIT,sigqdie) & 01) signal(SIGQUIT, SIG_IGN);
if( (int)signal(SIGINT, sigindie) & 01) signal(SIGINT, SIG_IGN);
signal(SIGTERM,sigtdie);

#ifdef pdp11
	ldfps(01200); /* detect overflow as an exception */
#endif

f_init();
MAIN__();
f_exit();
}


static sigfdie()
{
sigdie("Floating Exception", 1);
}



static sigidie()
{
sigdie("IOT Trap", 1);
}


static sigqdie()
{
sigdie("Quit signal", 1);
}



static sigindie()
{
sigdie("Interrupt", 0);
}



static sigtdie()
{
sigdie("Killed", 0);
}



static sigdie(s, kill)
register char *s;
int kill;
{
/* print error message, then clear buffers */
/* fflush(stderr); 	not needed */
fprintf(stderr, "%s\n", s);
fflush(stderr);
if(kill)
    signal(SIGIOT, 0);	/* set in case we get an IOT before returning */
f_exit();

if(kill)
	{
	/* now get a core */
	signal(SIGIOT, 0);
	abort();
	}
else
	exit(1);
}
