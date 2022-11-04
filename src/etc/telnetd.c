
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#ifndef lint
static char sccsid[] = "@(#)telnetd.c	3.0	(ULTRIX-11)	4/22/86";
/*
 * Based on "@(#)telnetd.c	1.3 (ULTRIX-32) 4/11/85";
 */
#endif

/*-----------------------------------------------------------------------
 *	Modification History
 *
 *	4/10/85 -- jrs
 *		Revise to allow inetd to perform front end functions,
 *		following the Berkeley model.  Also modify to check for
 *		more than just first sixteen ptys.
 *
 *	Based on 4.2BSD labeled:
 *		telnetd.c	4.27	84/04/11
 *
 *-----------------------------------------------------------------------
 */

/*
 * Stripped-down telnet server.
 */
#include <sys/types.h>
#include <sys/socket.h>
#ifndef	pdp11
#include <sys/wait.h>
#else	pdp11
#include <wait.h>
#endif	pdp11
#include <sys/file.h>

#include <netinet/in.h>

#include <arpa/telnet.h>

#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <sgtty.h>
#include <netdb.h>
#include <syslog.h>

#define	BELL	'\07'
#define BANNER	"%s %s"

char	hisopts[256];
char	myopts[256];

char	doopt[] = { IAC, DO, '%', 'c', 0 };
char	dont[] = { IAC, DONT, '%', 'c', 0 };
char	will[] = { IAC, WILL, '%', 'c', 0 };
char	wont[] = { IAC, WONT, '%', 'c', 0 };

/*
 * I/O data buffers, pointers, and counters.
 */
char	ptyibuf[BUFSIZ], *ptyip = ptyibuf;
char	ptyobuf[BUFSIZ], *pfrontp = ptyobuf, *pbackp = ptyobuf;
char	netibuf[BUFSIZ], *netip = netibuf;
char	netobuf[BUFSIZ], *nfrontp = netobuf, *nbackp = netobuf;
int	pcc, ncc;

int	pty, net;
int	inter;
int	reapchild();
extern	char **environ;
extern	int errno;
char	line[] = "/dev/ptyp0";


main(argc, argv)
	char *argv[];
{
	int fromlen;
	struct sockaddr_in from;

	fromlen = sizeof(from);
	if (getpeername(0, &from, &fromlen) < 0) {
		openlog(argv[0], LOG_PID);
		syslog(LOG_ERR, "getpeername: %m");
		closelog();
		exit(1);
	}
	doit(0, &from);
}

char	*envinit[] = { "TERM=network", 0 };
int	cleanup();

/*
 * Get a pty, scan input lines.
 */
doit(f, who)
	int f;
	struct sockaddr_in *who;
{
	char *cp = line, *host, *ntoa(), c;
	int i, p, t;
	struct sgttyb b;
	struct hostent *hp;

	for (c = 'p'; c <= 'z'; c++) {
		cp[strlen("/dev/pty")] = c;
		for (i = 0; i < 16; i++) {
			cp[strlen("/dev/ptyp")] = "0123456789abcdef"[i];
			p = open(cp, O_RDWR, 0);
			if (p > 0)
				goto gotpty;
		}
	}
	fatal(f, "All network ports in use");
	/*NOTREACHED*/
gotpty:
	(void) dup2(f, 0);
	cp[strlen("/dev/")] = 't';
#ifndef	pdp11
	t = open("/dev/tty", O_RDWR, 0);
	if (t >= 0) {
		ioctl(t, TIOCNOTTY, 0);
		(void) close(t);
	}
#else	pdp11
	zaptty();
#endif	pdp11
	t = open(cp, O_RDWR, 0);
	if (t < 0)
		fatalperror(f, cp, errno);
	ioctl(t, TIOCGETP, &b);
	b.sg_ispeed = B9600;
	b.sg_ospeed = B9600;
	b.sg_flags = CRMOD|XTABS|ANYP;
	ioctl(t, TIOCSETP, &b);
	ioctl(p, TIOCGETP, &b);
	b.sg_flags &= ~ECHO;
	ioctl(p, TIOCSETP, &b);
	hp = gethostbyaddr(&who->sin_addr, sizeof (struct in_addr),
		who->sin_family);
	if (hp)
		host = hp->h_name;
	else
		host = ntoa(who->sin_addr);
	if ((i = fork()) < 0)
		fatalperror(f, "fork", errno);
	if (i) {
		(void) close(t);
		telnet(f, p);
	}
	(void) close(f);
	(void) close(p);
	(void) dup2(t, 0);
	(void) dup2(t, 1);
	(void) dup2(t, 2);
	if (t > 2) {
		(void) close(t);
	}
	environ = envinit;
	execl("/bin/login", "login", "-h", host, 0);
	fatalperror(2, "/bin/login", errno);
	/*NOTREACHED*/
}

fatal(f, msg)
	int f;
	char *msg;
{
	char buf[BUFSIZ];

	(void) sprintf(buf, "telnetd: %s.\r\n", msg);
	(void) write(f, buf, strlen(buf));
	exit(1);
}

fatalperror(f, msg, errno)
	int f;
	char *msg;
	int errno;
{
	char buf[BUFSIZ];
	extern char *sys_errlist[];

	(void) sprintf(buf, "%s: %s", msg, sys_errlist[errno]);
	fatal(f, buf);
}

/*
 * Main loop.  Select from pty and network, and
 * hand data to telnet receiver finite state machine.
 */
telnet(f, p)
{
	int on = 1;
	char hostname[32];

	net = f, pty = p;
	ioctl(f, FIONBIO, &on);
	ioctl(p, FIONBIO, &on);
	signal(SIGTSTP, SIG_IGN);
	signal(SIGCHLD, cleanup);

	/*
	 * Request to do remote echo.
	 */
	dooption(TELOPT_ECHO);
	myopts[TELOPT_ECHO] = 1;
	/*
	 * Show banner that getty never gave.
	 */
	gethostname(hostname, sizeof (hostname));
	(void) sprintf(nfrontp, BANNER, hostname, "");
	nfrontp += strlen(nfrontp);
	for (;;) {
#ifndef	pdp11
		int ibits = 0, obits = 0;
		register int c;

		/*
		 * Never look for input if there's still
		 * stuff in the corresponding output buffer
		 */
		if ((nfrontp - nbackp) || pcc > 0)
			obits |= (1 << f);
		else
			ibits |= (1 << p);
		if ((pfrontp - pbackp) || ncc > 0)
			obits |= (1 << p);
		else
			ibits |= (1 << f);
		if (ncc < 0 && pcc < 0)
			break;
#else	pdp11
		long ibits = 0, obits = 0;
		register int c;

		/*
		 * Never look for input if there's still
		 * stuff in the corresponding output buffer
		 */
		if ((nfrontp - nbackp) || pcc > 0)
			obits |= (1L << f);
		else
			ibits |= (1L << p);
		if ((pfrontp - pbackp) || ncc > 0)
			obits |= (1L << p);
		else
			ibits |= (1L << f);
		if (ncc < 0 && pcc < 0)
			break;
#endif	pdp11
		select(16, &ibits, &obits, 0, 0);
		if (ibits == 0 && obits == 0) {
			sleep(5);
			continue;
		}

		/*
		 * Something to read from the network...
		 */
#ifndef	pdp11
		if (ibits & (1 << f)) {
#else	pdp11
		if (ibits & (1L << f)) {
#endif	pdp11
			ncc = read(f, netibuf, BUFSIZ);
			if (ncc < 0 && errno == EWOULDBLOCK)
				ncc = 0;
			else {
				if (ncc <= 0)
					break;
				netip = netibuf;
			}
		}

		/*
		 * Something to read from the pty...
		 */
#ifndef	pdp11
		if (ibits & (1L << p)) {
#else	pdp11
		if (ibits & (1 << p)) {
#endif	pdp11
			pcc = read(p, ptyibuf, BUFSIZ);
			if (pcc < 0 && errno == EWOULDBLOCK)
				pcc = 0;
			else {
				if (pcc <= 0)
					break;
				ptyip = ptyibuf;
			}
		}

		while (pcc > 0) {
			if ((&netobuf[BUFSIZ] - nfrontp) < 2)
				break;
			c = *ptyip++ & 0377, pcc--;
			if (c == IAC)
				*nfrontp++ = c;
			*nfrontp++ = c;
		}
		if ((obits & (1 << f)) && (nfrontp - nbackp) > 0)
			netflush();
		if (ncc > 0)
			telrcv();
		if ((obits & (1 << p)) && (pfrontp - pbackp) > 0)
			ptyflush();
	}
	cleanup();
}
	
/*
 * State for recv fsm
 */
#define	TS_DATA		0	/* base state */
#define	TS_IAC		1	/* look for double IAC's */
#define	TS_CR		2	/* CR-LF ->'s CR */
#define	TS_BEGINNEG	3	/* throw away begin's... */
#define	TS_ENDNEG	4	/* ...end's (suboption negotiation) */
#define	TS_WILL		5	/* will option negotiation */
#define	TS_WONT		6	/* wont " */
#define	TS_DO		7	/* do " */
#define	TS_DONT		8	/* dont " */

telrcv()
{
	register int c;
	static int state = TS_DATA;
	struct sgttyb b;

	while (ncc > 0) {
		if ((&ptyobuf[BUFSIZ] - pfrontp) < 2)
			return;
		c = *netip++ & 0377, ncc--;
		switch (state) {

		case TS_DATA:
			if (c == IAC) {
				state = TS_IAC;
				break;
			}
			if (inter > 0)
				break;
			*pfrontp++ = c;
			if (!myopts[TELOPT_BINARY] && c == '\r')
				state = TS_CR;
			break;

		case TS_CR:
			if (c && c != '\n')
				*pfrontp++ = c;
			state = TS_DATA;
			break;

		case TS_IAC:
			switch (c) {

			/*
			 * Send the process on the pty side an
			 * interrupt.  Do this with a NULL or
			 * interrupt char; depending on the tty mode.
			 */
			case BREAK:
			case IP:
				interrupt();
				break;

			/*
			 * Are You There?
			 */
			case AYT:
				*nfrontp++ = BELL;
				break;

			/*
			 * Erase Character and
			 * Erase Line
			 */
			case EC:
			case EL:
				ptyflush();	/* half-hearted */
				ioctl(pty, TIOCGETP, &b);
				*pfrontp++ = (c == EC) ?
					b.sg_erase : b.sg_kill;
				break;

			/*
			 * Check for urgent data...
			 */
			case DM:
				break;

			/*
			 * Begin option subnegotiation...
			 */
			case SB:
				state = TS_BEGINNEG;
				continue;

			case WILL:
			case WONT:
			case DO:
			case DONT:
				state = TS_WILL + (c - WILL);
				continue;

			case IAC:
				*pfrontp++ = c;
				break;
			}
			state = TS_DATA;
			break;

		case TS_BEGINNEG:
			if (c == IAC)
				state = TS_ENDNEG;
			break;

		case TS_ENDNEG:
			state = c == SE ? TS_DATA : TS_BEGINNEG;
			break;

		case TS_WILL:
			if (!hisopts[c])
				willoption(c);
			state = TS_DATA;
			continue;

		case TS_WONT:
			if (hisopts[c])
				wontoption(c);
			state = TS_DATA;
			continue;

		case TS_DO:
			if (!myopts[c])
				dooption(c);
			state = TS_DATA;
			continue;

		case TS_DONT:
			if (myopts[c]) {
				myopts[c] = 0;
				(void) sprintf(nfrontp, wont, c);
				nfrontp += sizeof (wont) - 2;
			}
			state = TS_DATA;
			continue;

		default:
			printf("telnetd: panic state=%d\n", state);
			exit(1);
		}
	}
}

willoption(option)
	int option;
{
	char *fmt;

	switch (option) {

	case TELOPT_BINARY:
/*		mode(RAW, 0);		*/
		goto common;

	case TELOPT_ECHO:
		mode(0, ECHO|CRMOD);
		/*FALL THRU*/

	case TELOPT_SGA:
	common:
		hisopts[option] = 1;
		fmt = doopt;
		break;

	case TELOPT_TM:
		fmt = dont;
		break;

	default:
		fmt = dont;
		break;
	}
	(void) sprintf(nfrontp, fmt, option);
	nfrontp += sizeof (dont) - 2;
}

wontoption(option)
	int option;
{
	char *fmt;

	switch (option) {

	case TELOPT_ECHO:
		mode(ECHO|CRMOD, 0);
		goto common;

	case TELOPT_BINARY:
/*		mode(0, RAW);		*/
		/*FALL THRU*/

	case TELOPT_SGA:
	common:
		hisopts[option] = 0;
		fmt = dont;
		break;

	default:
		fmt = dont;
	}
	(void) sprintf(nfrontp, fmt, option);
	nfrontp += sizeof (doopt) - 2;
}

dooption(option)
	int option;
{
	char *fmt;

	switch (option) {

	case TELOPT_TM:
		fmt = wont;
		break;

	case TELOPT_ECHO:
		mode(ECHO|CRMOD, 0);
		goto common;

	case TELOPT_BINARY:
/*		mode(RAW, 0);		*/
		/*FALL THRU*/

	case TELOPT_SGA:
	common:
		fmt = will;
		break;

	default:
		fmt = wont;
		break;
	}
	(void) sprintf(nfrontp, fmt, option);
	nfrontp += sizeof (doopt) - 2;
}

mode(on, off)
	int on, off;
{
	struct sgttyb b;

	ptyflush();
	ioctl(pty, TIOCGETP, &b);
	b.sg_flags |= on;
	b.sg_flags &= ~off;
	ioctl(pty, TIOCSETP, &b);
}

/*
 * Send interrupt to process on other side of pty.
 * If it is in raw mode, just write NULL;
 * otherwise, write intr char.
 */
interrupt()
{
	struct sgttyb b;
	struct tchars tchars;

	ptyflush();	/* half-hearted */
	ioctl(pty, TIOCGETP, &b);
	if (b.sg_flags & RAW) {
		*pfrontp++ = '\0';
		return;
	}
	*pfrontp++ = ioctl(pty, TIOCGETC, &tchars) < 0 ?
		'\177' : tchars.t_intrc;
}

ptyflush()
{
	int n;

	if ((n = pfrontp - pbackp) > 0)
		n = write(pty, pbackp, n);
	if (n < 0)
		return;
	pbackp += n;
	if (pbackp == pfrontp)
		pbackp = pfrontp = ptyobuf;
}

netflush()
{
	int n;

	if ((n = nfrontp - nbackp) > 0)
		n = write(net, nbackp, n);
	if (n < 0) {
		if (errno == EWOULDBLOCK)
			return;
		/* should blow this guy away... */
		return;
	}
	nbackp += n;
	if (nbackp == nfrontp)
		nbackp = nfrontp = netobuf;
}

cleanup()
{

	rmut();
#ifndef	pdp11
	vhangup();	/* XXX */
#endif	pdp11
	(void) shutdown(net, 2);
	(void) kill(0, SIGKILL);
	exit(1);
}

#include <utmp.h>

struct	utmp wtmp;
char	wtmpf[]	= "/usr/adm/wtmp";
char	utmp[] = "/etc/utmp";
#define SCPYN(a, b)	(void) strncpy(a, b, sizeof (a))
#define SCMPN(a, b)	strncmp(a, b, sizeof (a))

rmut()
{
	register f;
	int found = 0;

	f = open(utmp, O_RDWR, 0);
	if (f >= 0) {
		while(read(f, (char *)&wtmp, sizeof (wtmp)) == sizeof (wtmp)) {
			if (SCMPN(wtmp.ut_line, line+5) || wtmp.ut_name[0]==0)
				continue;
			(void) lseek(f, -(long)sizeof (wtmp), 1);
			SCPYN(wtmp.ut_name, "");
			SCPYN(wtmp.ut_host, "");
			(void) time(&wtmp.ut_time);
			write(f, (char *)&wtmp, sizeof (wtmp));
			found++;
		}
		(void) close(f);
	}
	if (found) {
		f = open(wtmpf, O_WRONLY|O_APPEND, 0);
		if (f >= 0) {
			SCPYN(wtmp.ut_line, line+5);
			SCPYN(wtmp.ut_name, "");
			SCPYN(wtmp.ut_host, "");
			(void) time(&wtmp.ut_time);
			write(f, (char *)&wtmp, sizeof (wtmp));
			(void) close(f);
		}
	}
	(void) chmod(line, 0666);
	(void) chown(line, 0, 0);
	line[strlen("/dev/")] = 'p';
	(void) chmod(line, 0666);
	(void) chown(line, 0, 0);
}

/*
 * Convert network-format internet address
 * to base 256 d.d.d.d representation.
 */
char *
ntoa(in)
	struct in_addr in;
{
	static char b[18];
	register char *p;

	p = (char *)&in;
#define	UC(b)	(((int)b)&0xff)
	(void) sprintf(b, "%d.%d.%d.%d", UC(p[0]), UC(p[1]), UC(p[2]), UC(p[3]));
	return (b);
}
