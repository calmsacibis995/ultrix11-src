
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#ifndef lint
static char sccsid[] = "@(#)ventel.c	3.0	4/22/86";
#endif

/*
 * Routines for calling up on a Ventel Modem
 * The Ventel is expected to be strapped for "no echo".
 */
#include "tip.h"

#define	MAXRETRY	5

static	int sigALRM();
static	int timeout = 0;
static	jmp_buf t_outbuf;

ven_dialer(num, acu)
	register char *num;
	char *acu;
{
	register char *cp;
	register int connected = 0;
#ifdef ACULOG
	char line[80];
#endif
	/*
	 * Get in synch with a couple of carriage returns
	 */
	if (!vensync(FD)) {
		printf("can't synchronize with ventel\n");
#ifdef ACULOG
		logent(value(HOST), num, "ventel", "can't synch up");
#endif
		return (0);
	}
	if (boolean(value(VERBOSE)))
		printf("\ndialing...");
	fflush(stdout);
	ioctl(FD, TIOCHPCL, 0);
	echo("#k$\r$\n$D$I$A$L$:$ ");
	for (cp = num; *cp; cp++) {
		sleep(1);
		write(FD, cp, 1);
	}
	echo("\r$\n");
	if (gobble('\n'))
		connected = gobble('!');
	ioctl(FD, TIOCFLUSH);
#ifdef ACULOG
	if (timeout) {
		sprintf(line, "%d second dial timeout",
			number(value(DIALTIMEOUT)));
		logent(value(HOST), num, "ventel", line);
	}
#endif
	if (timeout)
		ven_disconnect();	/* insurance */
	return (connected);
}

ven_disconnect()
{

	close(FD);
}

ven_abort()
{

	write(FD, "\03", 1);
	close(FD);
}

static int
echo(s)
	register char *s;
{
	char c;

	while (c = *s++) switch (c) {

	case '$':
		read(FD, &c, 1);
		s++;
		break;

	case '#':
		c = *s++;
		write(FD, &c, 1);
		break;

	default:
		write(FD, &c, 1);
		read(FD, &c, 1);
	}
}

static int
sigALRM()
{

	printf("\07timeout waiting for reply\n");
	timeout = 1;
	longjmp(t_outbuf, 1);
}

static int
gobble(match)
	register char match;
{
	char c;
	int (*f)();

	f = signal(SIGALRM, sigALRM);
	timeout = 0;
	do {
		if (setjmp(t_outbuf)) {
			signal(SIGALRM, f);
			return (0);
		}
		alarm(number(value(DIALTIMEOUT)));
		read(FD, &c, 1);
		alarm(0);
		c &= 0177;
#ifdef notdef
		if (boolean(value(VERBOSE)))
			putchar(c);
#endif
	} while (c != '\n' && c != match);
	signal(SIGALRM, SIG_DFL);
	return (c == match);
}

#define min(a,b)	((a)>(b)?(b):(a))
/*
 * This convoluted piece of code attempts to get
 * the ventel in sync.  If you don't have FIONREAD
 * there are gory ways to simulate this.
 */
static int
vensync(fd)
{
	int already = 0, nread;
	char buf[60];
#ifndef	FIONREAD
	int	(*f)();
	int	sig2ALRM();
#endif

	/*
	 * Toggle DTR to force anyone off that might have left
	 * the modem connected, and insure a consistent state
	 * to start from.
	 *
	 * If you don't have the ioctl calls to diddle directly
	 * with DTR, you can always try setting the baud rate to 0.
	 */
	ioctl(FD, TIOCCDTR, 0);
	sleep(2);
	ioctl(FD, TIOCSDTR, 0);
	while (already < MAXRETRY) {
		/*
		 * After reseting the modem, send it two \r's to
		 * autobaud on. Make sure to delay between them
		 * so the modem can frame the incoming characters.
		 */
		write(fd, "\r", 1);
		sleep(1);
		write(fd, "\r", 1);
		sleep(3);
#ifdef	FIONREAD
		if (ioctl(fd, FIONREAD, (caddr_t)&nread) < 0) {
			perror("tip: ioctl");
			continue;
		}
		while (nread > 0) {
			read(fd, buf, min(nread, 60));
			if ((buf[nread - 1] & 0177) == '$')
				return (1);
			nread -= min(nread, 60);
		}
#else
		f = signal(SIGALRM, sig2ALRM);
		if (setjmp(t_outbuf)) {
			signal(SIGALRM, f);
			already++;
			continue;
		}
		if ((nread = read(fd, buf, 60)) < 0) {
			alarm(0);
			signal(SIGALRM, f);
			perror("tip: read");
			continue;
		}
		alarm(0);
		signal(SIGALRM, f);
		if ((buf[nread - 1] & 0177) == '$')
			return (1);
#endif
		sleep(1);
		already++;
	}
	return (0);
}

#ifndef	FIONREAD
static int
sig2ALRM()
{

	printf("\07timeout waiting for reply\n");
	longjmp(t_outbuf, 1);
}
#endif	FIONREAD
