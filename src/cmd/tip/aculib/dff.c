
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#ifndef lint
static char sccsid[] = "@(#)dff.c	3.0	4/22/86";
#endif

/*
 * Dial the DF112/DF224
 */

#include "tip.h"

static jmp_buf Sjbuf;
static timeout();


dff_dialer(num, mod, sta)
	char *num, *mod, *sta;
{
	register int f = FD;
	struct sgttyb buf;
	int rw = 2;
	char c = '\0';
	int twice, i, j;
#ifdef TIOCMSET
	int st = TIOCM_ST;	/* secondary Transmit flag */
#endif TIOCMSET

	/* translate telephone number for df112/df224 */
	for (j = i = 0; i < strlen(num); ++i) {
		switch(num[i]) {
		case '-':	/* df112/df224 doesn't like dash */
		case ' ':	/* df112/df224 doesn't like space */
			break;
		default:
			num[j++] = num[i];
			break;
		}
	}
	num[j] = '\0';

	ioctl(f, TIOCHPCL, 0);		/* make sure it hangs up when done */
	if (setjmp(Sjbuf)) {
		printf("connection timed out\r\n");
		dff_dconnect();
		return (0);
	}
	if (boolean(value(VERBOSE)))
		printf("\ndialing...");
	fflush(stdout);
#ifdef TIOCMSET
	ioctl(f, TIOCGETP, &buf);
	if (buf.sg_ospeed != B1200) {	/* must dial at 1200 baud */
		buf.sg_ospeed = buf.sg_ispeed = B1200;
		ioctl(f, TIOCSETP, &buf);
		ioctl(f, TIOCMBIC, &st); /* clear ST for 300 baud */
	} else
		ioctl(f, TIOCMBIS, &st); /* set ST for 1200 baud */
#endif
/*
 * Must try the dial twice so the df112/df224 dialer can
 * determine the proper speed to dial at.  Bill Burns 11/6/84
 */
	for(twice = 2; twice; twice--) {
		signal(SIGALRM, timeout);
		alarm(5 * strlen(num) + 10);
		ioctl(f, TIOCFLUSH, &rw);
 		/*cntrl A = burst mode, P*/
		write(f, "\001", 1);
		/*write(f, "P", 1);  */
		write(f,mod,1); 
		write(f, num, strlen(num));
		write(f, sta, 1);
 		read(f, &c, 1);		/* avoid carrige return */
 		read(f, &c, 1);		/* and line feed */
 		read(f, &c, 1);
 		sleep(2);	/* allow time for the rest of the modem */
 				/* "Attached" message to be recieved */
		if(c == 'A') {	   /* if we got an A, ok */
 			ioctl(f, TIOCFLUSH, &rw); /* get rid of modem garbage */
		 	return (1);	/* success */
		}
	}
 	ioctl(f, TIOCFLUSH, &rw);	/* get rid of modem garbage */
 	return (0);	/* fail */
}

dff_dconnect()
{
	int rw = 2;

/*
	write(FD, "\002", 1);
*/
	sleep(1);
	ioctl(FD, TIOCFLUSH, &rw);
}


dff_abort()
{

	dff_dconnect();
}


static
timeout()
{

	longjmp(Sjbuf, 1);
}


d112p_dialer(num,acu)
	char *num, *acu;
{

	return (dff_dialer(num,"P","#"));
}

d224p_dialer(num,acu)
	char *num, *acu;
{

	return (dff_dialer(num,"P","!"));
}

d112t_dialer(num,acu)
	char *num, *acu;
{

	return (dff_dialer(num,"T","#"));
}

d224t_dialer(num,acu)
	char *num, *acu;
{

	return (dff_dialer(num,"T","!"));
}
