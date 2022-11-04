
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#ifndef lint
static char sccsid[] = "@(#)df.c	3.0	4/22/86";
#endif

/*
 * Dial the DF02-AC or DF03-AC
 */

#include "tip.h"

static jmp_buf Sjbuf;
static timeout();

df02_dialer(num, acu)
	char *num, *acu;
{

	return (df_dialer(num, acu, 0));
}

df03_dialer(num, acu)
	char *num, *acu;
{

	return (df_dialer(num, acu, 1));
}

df_dialer(num, acu, df03)
	char *num, *acu;
	int df03;
{
	register int f = FD;
	struct sgttyb buf;
	int bspeed = 0, rw = 2;
	char c = '\0';

	ioctl(f, TIOCHPCL, 0);		/* make sure it hangs up when done */
	if (setjmp(Sjbuf)) {
		printf("connection timed out\r\n");
		df_disconnect();
		return (0);
	}
	if (boolean(value(VERBOSE)))
		printf("\ndialing...");
	fflush(stdout);
	if (df03) {
		/*
		 * The dial speed and the speed of the modem might be
		 * different for a df03.  This code takes this into
		 * account. ds should be in /etc/remote if this is the case.
		 */
		int	dspeed = speed(number(value(DIALSPEED)));
#ifdef TIOCMSET
		int st = TIOCM_ST;	/* secondary Transmit flag */
#endif

		ioctl(f, TIOCGETP, &buf);
		if (buf.sg_ospeed != dspeed) {	/* dialspeed != baudrate */
			if (dspeed == NULL) {
				printf("bad dialspeed, dialing at baudrate...");
			} else {
				bspeed = buf.sg_ospeed;
				buf.sg_ospeed = buf.sg_ispeed = dspeed;
				ioctl(f, TIOCSETP, &buf);
			}
#ifdef	TIOCMSET
			ioctl(f, TIOCMBIC, &st); /* clear ST for 300 baud */
		} else {
			ioctl(f, TIOCMBIS, &st); /* set ST for 1200 baud */
#endif
		}
	}
	signal(SIGALRM, timeout);
	alarm(5 * strlen(num) + 10);
	ioctl(f, TIOCFLUSH, &rw);
	write(f, "\001", 1);
	sleep(1);
	write(f, "\002", 1);
	write(f, num, strlen(num));
	read(f, &c, 1);
	if (df03 && bspeed) {
		buf.sg_ispeed = buf.sg_ospeed = bspeed;
		ioctl(f, TIOCSETP, &buf);
	}
	return (c == 'A');
}

df_disconnect()
{
	int rw = 2;

	write(FD, "\001", 1);
	sleep(1);
	ioctl(FD, TIOCFLUSH, &rw);
}


df_abort()
{

	df_disconnect();
}


static
timeout()
{

	longjmp(Sjbuf, 1);
}
