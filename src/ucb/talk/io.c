
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char *Sccsid = "@(#)io.c	3.0	(ULTRIX-11)	4/22/86";

/* this file contains the I/O handling and the exchange of 
   edit characters. This connection itself is established in
   ctl.c
 */

#include "talk.h"
#include <stdio.h>
#include <errno.h>
#ifdef vax
#include <sys/time.h>
#endif vax

#define A_LONG_TIME 10000000
#define STDIN_MASK (1<<fileno(stdin))	/* the bit mask for standard
					   input */
extern int errno;

/*
 * The routine to do the actual talking
 */

talk()
{
#ifdef vax
    register int read_template, sockt_mask;
    int read_set, nb;
#else pdp11
    long read_template, sockt_mask;
    long read_set;
    register int nb;
#endif
    char buf[BUFSIZ];
#ifdef vax
    struct timeval wait;
#endif vax

    message("Connection established\007\007\007");
    current_line = 0;

    sockt_mask = (1L<<sockt);

	/*
	 * wait on both the other process (sockt_mask) and 
	 * standard input ( STDIN_MASK )
	 */

    read_template = sockt_mask | STDIN_MASK;

    forever {

	read_set = read_template;

#ifdef vax
	wait.tv_sec = A_LONG_TIME;
	wait.tv_usec = 0;

	nb = select(32, &read_set, 0, 0, &wait);
#else pdp11
	nb = select(32, &read_set, 0, 0, 0);
#endif

	if (nb <= 0) {

		/* We may be returning from an interupt handler */

	    if (errno == EINTR) {
		read_set = read_template;
		continue;
	    } else {
		    /* panic, we don't know what happened */
		p_error("Unexpected error from select");
		quit();
	    }
	}

	if ( read_set & sockt_mask ) { 

		/* There is data on sockt */
	    nb = read(sockt, buf, sizeof buf);

	    if (nb <= 0) {
		message("Connection closed. Exiting");
		quit();
	    } else {
		display(&his_win, buf, nb);
	    }
	}
	
	if ( read_set & STDIN_MASK ) {

		/* we can't make the tty non_blocking, because
		   curses's output routines would screw up */

#ifdef pdp11
	    long foo;

	    ioctl(0, FIONREAD, (struct sgttyb *) &foo);
	    nb = read(0, buf, (int)foo);
#else vax
	    ioctl(0, FIONREAD, (struct sgttyb *) &nb);
	    nb = read(0, buf, nb);
#endif

	    display(&my_win, buf, nb);
	    write(sockt, buf, nb);	/* We might lose data here
					   because sockt is non-blocking
					 */

	}
    }
}

extern int	errno;
extern int	sys_nerr;
extern char	*sys_errlist[];

    /* p_error prints the system error message on the standard location
       on the screen and then exits. (i.e. a curses version of perror)
     */

p_error(string) 
char *string;
{
    char *sys;

    sys = "Unknown error";
    if(errno < sys_nerr) {
	sys = sys_errlist[errno];
    }


    wmove(my_win.x_win, current_line%my_win.x_nlines, 0);
    wprintw(my_win.x_win, "[%s : %s (%d)]\n", string, sys, errno);
    wrefresh(my_win.x_win);
    move(LINES-1, 0);
    refresh();
    quit();
}

    /* display string in the standard location */

message(string)
char *string;
{
    wmove(my_win.x_win, current_line%my_win.x_nlines, 0);
    wprintw(my_win.x_win, "[%s]\n", string);
    wrefresh(my_win.x_win);
}
