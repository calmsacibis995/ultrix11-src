
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char *Sccsid = "@(#)msgs.c	3.0	(ULTRIX-11)	4/22/86";

/* 
 * a package to display what is happening every MSG_INTERVAL seconds
 * if we are slow connecting.
 */

#include <signal.h>
#include <stdio.h>
#ifdef vax
#include <sys/time.h>
#endif
#include "talk.h"

#define MSG_INTERVAL 4
#define LONG_TIME 100000

char *current_state;
int current_line = 0;

#ifdef vax
static struct itimerval itimer;
static struct timeval wait = { MSG_INTERVAL , 0};
static struct timeval undo = { LONG_TIME, 0};
#else pdp11
static int wait = MSG_INTERVAL;
static int undo = LONG_TIME;
#endif
    

disp_msg()
{
    message(current_state);
#ifdef pdp11
    signal(SIGALRM, disp_msg);
    alarm(wait);
#endif pdp11
}

start_msgs()
{
    message(current_state);
    signal(SIGALRM, disp_msg);
#ifdef vax
    itimer.it_value = itimer.it_interval = wait;
    setitimer(ITIMER_REAL, &itimer, (struct timerval *)0);
#else pdp11
    alarm(wait);
#endif
}

end_msgs()
{
    signal(SIGALRM, SIG_IGN);
#ifdef vax
    timerclear(&itimer.it_value);
    timerclear(&itimer.it_interval);
    setitimer(ITIMER_REAL, &itimer, (struct timerval *)0);
#else pdp11
    alarm(0);
#endif
}
