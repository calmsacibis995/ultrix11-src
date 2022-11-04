
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#ifndef lint
static	char	*sccsid = "@(#)rlogin.c	3.0	4/22/86";
#endif lint
/*
 *
 *	Revised 2/17/85 by lp@decvax.
 *	Use select to eliminate reader process. Reader process will
 *	now only awaken on ~^Y command (ie a reader subprocess will
 *	be made while main program is suspended). This process will
 *	die when the main program returns (or exits).
 *	This change can be incorporated into rlogin by compiling with
 *	with no option (-DNOSELECT gets 2 process version).
 */

/*
#ifndef lint
static char sccsid[] = "@(#)rlogin.c	4.15 (Berkeley) 83/07/02";
#endif
*/

/*
 * rlogin - remote login
 */
#include <sys/types.h>
#include <sys/socket.h>
#ifndef	pdp11
#include <sys/wait.h>
#else	pdp11
#include <wait.h>
#endif	pdp11

#include <netinet/in.h>

#include <stdio.h>
#include <sgtty.h>
#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <netdb.h>

char	*index(), *rindex(), *malloc(), *getenv();
struct	passwd *getpwuid();
char	*name;
int	rem;
char	cmdchar = '~';
int	eight;
char	*speeds[] =
    { "0", "50", "75", "110", "134", "150", "200", "300",
      "600", "1200", "1800", "2400", "4800", "9600", "19200", "38400" };
char	term[64] = "network";
extern	int errno;
int	lostpeer();
#ifdef	pdp11
#define	signal	sigset
#endif	pdp11

main(argc, argv)
	int argc;
	char **argv;
{
	char *host, *cp;
	struct sgttyb ttyb;
	struct passwd *pwd;
	struct servent *sp;
	int uid, options = 0;

	host = rindex(argv[0], '/');
	if (host)
		host++;
	else
		host = argv[0];
	argv++, --argc;
	if (!strcmp(host, "rlogin"))
		host = *argv++, --argc;
another:
	if (argc > 0 && !strcmp(*argv, "-d")) {
		argv++, argc--;
		options |= SO_DEBUG;
		goto another;
	}
	if (argc > 0 && !strcmp(*argv, "-l")) {
		argv++, argc--;
		if (argc == 0)
			goto usage;
		name = *argv++; argc--;
		goto another;
	}
	if (argc > 0 && !strncmp(*argv, "-e", 2)) {
		cmdchar = argv[0][2];
		argv++, argc--;
		goto another;
	}
	if (argc > 0 && !strcmp(*argv, "-8")) {
		eight = 1;
		argv++, argc--;
		goto another;
	}
	if (host == 0)
		goto usage;
	if (argc > 0)
		goto usage;
	pwd = getpwuid(getuid());
	if (pwd == 0) {
		fprintf(stderr, "Who are you?\n");
		exit(1);
	}
	sp = getservbyname("login", "tcp");
	if (sp == 0) {
		fprintf(stderr, "rlogin: login/tcp: unknown service\n");
		exit(2);
	}
	cp = getenv("TERM");
	if (cp)
		strcpy(term, cp);
	if (ioctl(0, TIOCGETP, &ttyb)==0) {
		strcat(term, "/");
		strcat(term, speeds[ttyb.sg_ospeed]);
	}
	signal(SIGPIPE, lostpeer);
	rem = rcmd(&host, sp->s_port, pwd->pw_name,
	    name ? name : pwd->pw_name, term, 0);
	if (rem < 0) {
		perror("rcmd");
		exit(1);
	}
	if (options & SO_DEBUG &&
	    setsockopt(rem, SOL_SOCKET, SO_DEBUG, 0, 0) < 0)
		perror("rlogin: setsockopt (SO_DEBUG)");
	uid = getuid();
	if (setuid(uid) < 0) {
		perror("rlogin: setuid");
		exit(1);
	}
	doit();
	/*NOTREACHED*/
usage:
	fprintf(stderr,
	    "usage: rlogin host [ -ex ] [ -l username ] [ -8 ]\n");
	exit(1);
}

#define CRLF "\r\n"

int	child;
int	catchild();

int	defflags, tabflag;
char	deferase, defkill;
#ifndef NOSELECT
char ctrlc;
#endif
struct	tchars deftc;
struct	ltchars defltc;
struct	tchars notc =	{ -1, -1, -1, -1, -1, -1, CMIN, CTIME };
struct	ltchars noltc = { -1, -1, -1, -1, -1, -1 };

#ifndef NOSELECT
#ifndef	pdp11
#include <sys/time.h>
#else	pdp11
#include <time.h>
#endif	pdp11
	int running = 1;
	int readonly = 0;
#endif

doit()
{
	int exit();
	struct sgttyb sb;
#ifndef NOSELECT
#ifndef	pdp11
	int *readfd, *writefd=0, *exfd=0, maski, masko, mask, found;
#else	pdp11
	long *readfd, *writefd=0, *exfd=0, maski, masko, mask;
	int found;
#endif	pdp11
	int oob();
	struct timeval *time=0;
	maski = (1<<0);
	masko = (1<<rem);
#endif

	ioctl(0, TIOCGETP, (char *)&sb);
	defflags = sb.sg_flags;
	tabflag = defflags & TBDELAY;
	defflags &= ECHO | CRMOD;
	deferase = sb.sg_erase;
	defkill = sb.sg_kill;
	ioctl(0, TIOCGETC, (char *)&deftc);
	notc.t_startc = deftc.t_startc;
	notc.t_stopc = deftc.t_stopc;
#ifndef NOSELECT
	ctrlc = deftc.t_intrc;
#endif
	ioctl(0, TIOCGLTC, (char *)&defltc);
	signal(SIGINT, exit);
	signal(SIGHUP, exit);
	signal(SIGQUIT, exit);
#ifdef NOSELECT
	child = fork();
	if (child == -1) {
		perror("rlogin: fork");
		done();
	}
	signal(SIGINT, SIG_IGN);
	mode(1);
	if (child == 0) {
		reader();
		sleep(1);
		prf("\007Connection closed.");
		exit(3);
	}
	signal(SIGCHLD, catchild);
	writer();
#else
	signal(SIGINT, SIG_IGN);
	signal(SIGURG, oob);
	mode(1);
	{ int pid = -getpid();
	  ioctl(rem, SIOCSPGRP, (char *)&pid); }
	while (running) {
		mask = maski|masko;
		if (readonly)
			mask = masko;
		readfd = &mask;
		found = select(32, readfd, writefd, exfd, time);
		if(found == -1) {     /* Ignore if select is interrupted */
			errno = 0;
			continue;
		}
		if (*readfd == masko || *readfd == (maski|masko)) {
			reader();
			if (!running) {
				sleep(1);
				prf("\007Connection closed.");
				mode(0);
				exit(3);
			}
		}
		if (!readonly && (*readfd == maski || *readfd == (maski|masko))) {
			if (!writer())
				break;
		}
	}
#endif
	prf("Closed connection.");
	done();
}

done()
{

	mode(0);
#ifdef NOSELECT
	if (child > 0 && kill(child, SIGKILL) >= 0)
		wait((int *)0);
#endif
	exit(0);
}

catchild()
{
	union wait status;
	int pid;

again:
#ifndef	pdp11
	pid = wait3(&status, WNOHANG|WUNTRACED, 0);
#else	pdp11
	pid = wait2(&status, WNOHANG|WUNTRACED);
#endif	pdp11
	if (pid == 0)
		return;
	/*
	 * if the child (reader) dies, just quit
	 */
	if (pid < 0 || pid == child && !WIFSTOPPED(status))
		done();
	goto again;
}

/*
 * writer: write to remote: 0 -> line.
 * ~.	terminate
 * ~^Z	suspend rlogin process.
 * ~^Y	suspend rlogin process, but leave reader alone.
 */
#ifndef NOSELECT
#define MAXLOOP 8	/* Simulate an 8 character typeahead else select might eat your system */
#define MAXBUF 600	/* Must see a \r or \n after this many characters */
static char b[MAXBUF];
       int local = 0;
       char *p = &b[0];
#endif

writer()
{
#ifdef NOSELECT
	char b[600], c;
	register n;
	register char *p;
#else
#ifndef	pdp11
	register char c;
#else	pdp11
	char c;
#endif	pdp11
	register n;
#ifndef	pdp11
	caddr_t inchars;
#else	pdp11
	long	inchars;
#endif	pdp11
	int numloop = 0;

	if(ioctl(0, FIONREAD, (caddr_t) &inchars) == -1)
		return(1);
#endif

top:
#ifdef NOSELECT
	p = b;
	for (;;) {
		int local;
#else
reread:
#endif

		n = read(0, &c, 1);
		if (n == 0)
#ifdef NOSELECT
			break;
#else
			return (0);
#endif
		if (n < 0)
			if (errno == EINTR)
#ifdef NOSELECT
				continue;
			else
				break;
#else

				goto reread;
			else
				return (0);
#endif

		if (eight == 0)
			c &= 0177;
		/*
		 * If we're at the beginning of the line
		 * and recognize a command character, then
		 * we echo locally.  Otherwise, characters
		 * are echo'd remotely.  If the command
		 * character is doubled, this acts as a
		 * force and local echo is suppressed.
		 */
		if (p == b)
			local = (c == cmdchar);
		if (p == b + 1 && *b == cmdchar)
			local = (c != cmdchar);
		if (!local) {
			if (write(rem, &c, 1) == 0) {
				prf("line gone");
#ifdef NOSELECT
				return;
#else
				return (0);
#endif
			}
		} else {
			if (c == '\r' || c == '\n') {
				char cmdc = b[1];

				if (cmdc == '.' || cmdc == deftc.t_eofc) {
					write(0, CRLF, sizeof(CRLF));
#ifdef NOSELECT
					return;
#else
					return (0);
#endif
				}
				if (cmdc == defltc.t_suspc ||
				    cmdc == defltc.t_dsuspc) {
					write(0, CRLF, sizeof(CRLF));
#ifdef NOSELECT
					mode(0);
					signal(SIGCHLD, SIG_IGN);
					kill(cmdc == defltc.t_suspc ?
					  0 : getpid(), SIGTSTP);
					signal(SIGCHLD, catchild);
					mode(1);
					goto top;
#else
					if(cmdc == defltc.t_dsuspc) {
						child = fork();
						if (child == -1)
							perror("rlogin: can't fork reader");
						if (child == 0) {
							readonly = 1;
							return(1);
						}
					}
					mode(0);
	     /* suspend writer */	kill(getpid(), SIGTSTP);
					if(cmdc == defltc.t_dsuspc) {
		/* Kill reader */		kill(child, SIGKILL);
						(void) wait(0);
					}
					readonly = 0;
					mode(1);
					p = b;
					return (1);
#endif

				}
				*p++ = c;
				write(rem, b, p - b);
#ifdef NOSELECT
				goto top;
#else
				p = b;
				return(1);
#endif
			}
			write(1, &c, 1);
		}
		*p++ = c;
		if (c == deferase) {
			p -= 2;
			if (p < b) {
#ifdef NOSELECT
				goto top;
#else
				p = b;
				return (1);
#endif
			}
		}
		if (c == defkill || c == deftc.t_eofc ||
		    c == '\r' || c == '\n' || c == ctrlc) {
#ifdef NOSELECT
			goto top;
#else
			p = b;
			return(1);
#endif
		}
		if (p >= &b[sizeof b])
			p--;
#ifdef NOSELECT
	}
#else
	/* If there are more input chars go get them */
	if(--inchars > 0 && numloop++ < MAXLOOP)
		goto top;
#endif
	return (1);
}

oob()
{
	int out = 1+1, atmark;
	char waste[BUFSIZ], mark;
	int hiwat;

	ioctl(1, TIOCFLUSH, (char *)&out);
	(void) ioctl(rem, SIOCGHIWAT, &hiwat);
	for (;;) {
		if (ioctl(rem, SIOCATMARK, &atmark) < 0) {
			perror("ioctl");
			break;
		}
		if (atmark)
			break;
		(void) read(rem, waste, sizeof (waste));
	}
	recv(rem, &mark, 1, MSG_OOB);
	if (mark & TIOCPKT_NOSTOP) {
		notc.t_stopc = -1;
		notc.t_startc = -1;
		ioctl(0, TIOCSETC, (char *)&notc);
	}
	if (mark & TIOCPKT_DOSTOP) {
		notc.t_stopc = deftc.t_stopc;
		notc.t_startc = deftc.t_startc;
		ioctl(0, TIOCSETC, (char *)&notc);
	}
}

/*
 * reader: read from remote: line -> 1
 */
reader()
{
	char rb[BUFSIZ];
	register int cnt;

#ifdef NOSELECT
	signal(SIGURG, oob);
	{ int pid = -getpid();
	  ioctl(rem, SIOCSPGRP, (char *)&pid); }
	for (;;) {
#endif
		cnt = read(rem, rb, sizeof (rb));
		if (cnt == 0)
#ifdef NOSELECT
			break;
#else
			return (running = 0);
#endif
		if (cnt < 0) {
			if (errno == EINTR)
#ifdef NOSELECT
				continue;
			break;
#else
				return (running = 1);
			return(running = 0);
#endif
		}
		write(1, rb, cnt);
#ifdef NOSELECT
	}
#else
	return (running = 1);
#endif
}

mode(f)
{
	struct tchars *tc;
	struct ltchars *ltc;
	struct sgttyb sb;

	ioctl(0, TIOCGETP, (char *)&sb);
	switch (f) {

	case 0:
		sb.sg_flags &= ~(CBREAK|RAW|TBDELAY);
		sb.sg_flags |= defflags|tabflag;
		tc = &deftc;
		ltc = &defltc;
		sb.sg_kill = defkill;
		sb.sg_erase = deferase;
		break;

	case 1:
		sb.sg_flags |= (eight ? RAW : CBREAK);
		sb.sg_flags &= ~defflags;
		/* preserve tab delays, but turn off XTABS */
		if ((sb.sg_flags & TBDELAY) == XTABS)
			sb.sg_flags &= ~TBDELAY;
		tc = &notc;
		ltc = &noltc;
		sb.sg_kill = sb.sg_erase = -1;
		break;

	default:
		return;
	}
	ioctl(0, TIOCSLTC, (char *)ltc);
	ioctl(0, TIOCSETC, (char *)tc);
	ioctl(0, TIOCSETN, (char *)&sb);
}

/*VARARGS*/
prf(f, a1, a2, a3)
	char *f;
{
	fprintf(stderr, f, a1, a2, a3);
	fprintf(stderr, CRLF);
}

lostpeer()
{
	signal(SIGPIPE, SIG_IGN);
	prf("\007Connection closed.");
	done();
}
