
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#ifndef lint
static	char	*sccsid = "@(#)rsh.c	3.0	(ULTRIX-11) 4/22/86";
#endif lint

/*
 * rsh.c
 *
 *	static char sccsid[] = "rsh.c       (Berkeley)  4.8 83/06/10";
 *
 *	14-Mar-85	Added SELECT (NO) defines to allow rsh to be compiled
 *			to run as a single process (via select system
 *			call). This results in a slightly more responsive
 *			rsh especially when you interrupt it.
 *							lp@decvax
 *
 *	22-Aug-84	ma.  Changed the usage message to reflect reality.
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/file.h>

#include <netinet/in.h>

#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <pwd.h>
#include <netdb.h>

/*
 * rsh - remote shell
 */
/* VARARGS */
int	error();
char	*index(), *rindex(), *malloc(), *getpass(), *sprintf(), *strcpy();

struct	passwd *getpwuid();

int	errno;
int	options;
int	rfd2;
int	sendsig();
int	noinput = 0;

#ifndef	pdp11
#define mask(s) (1 << ((s) - 1))
#endif	pdp11

main(argc, argv0)
	int argc;
	char **argv0;
{
	int rem, pid;
	char *host, *cp, **ap, buf[BUFSIZ], *args, **argv = argv0, *user = 0;
	register int cc;
	int asrsh = 0;
	struct passwd *pwd;
#ifndef	pdp11
	int readfrom, ready;
#else	pdp11
	long readfrom, ready;
#endif	pdp11
	int one = 1;
	struct servent *sp;
#ifndef	pdp11
	int omask;
#endif	pdp11
	int bytin=0, bytout=0;

	host = rindex(argv[0], '/');
	if (host)
		host++;
	else
		host = argv[0];
	argv++, --argc;
	if (!strcmp(host, "rsh")) {
		host = *argv++, --argc;
		asrsh = 1;
	}
another:
	if (argc > 0 && !strcmp(*argv, "-l")) {
		argv++, argc--;
		if (argc > 0)
			user = *argv++, argc--;
		goto another;
	}
	if (argc > 0 && !strcmp(*argv, "-n")) {
		argv++, argc--;
		(void) close(0);
		(void) open("/dev/null", 0);
		noinput++;
		goto another;
	}
	if (argc > 0 && !strcmp(*argv, "-d")) {
		argv++, argc--;
		options |= SO_DEBUG;
		goto another;
	}
	/*
	 * Ignore the -e flag to allow aliases with rlogin
	 * to work
	 */
	if (argc > 0 && !strncmp(*argv, "-e", 2)) {
		argv++, argc--;
		goto another;
	}
	if (host == 0)
		goto usage;
	if (argv[0] == 0) {
		if (asrsh)
			*argv0 = "rlogin";
		execv("/usr/ucb/rlogin", argv0);
		perror("rsh: /usr/ucb/rlogin");
		exit(1);
	}
	pwd = getpwuid(getuid());
	if (pwd == 0) {
		fprintf(stderr, "who are you?\n");
		exit(1);
	}
	cc = 0;
	for (ap = argv; *ap; ap++)
		cc += strlen(*ap) + 1;
	cp = args = malloc(cc);
	for (ap = argv; *ap; ap++) {
		(void) strcpy(cp, *ap);
		while (*cp)
			cp++;
		if (ap[1])
			*cp++ = ' ';
	}
	sp = getservbyname("shell", "tcp");
	if (sp == 0) {
		fprintf(stderr, "rsh: shell/tcp: unknown service\n");
		exit(1);
	}
	rem = rcmd(&host, sp->s_port, pwd->pw_name,
	    user ? user : pwd->pw_name, args, &rfd2);
	if (rem < 0) {
		perror("rsh: rcmd");
		exit(1);
	}
	if (rfd2 < 0) {
		fprintf(stderr, "rsh: can't establish stderr\n");
		exit(2);
	}
	if (options & SO_DEBUG) {
		if (setsockopt(rem, SOL_SOCKET, SO_DEBUG, 0, 0) < 0)
			perror("rsh: setsockopt (stdin)");
		if (setsockopt(rfd2, SOL_SOCKET, SO_DEBUG, 0, 0) < 0)
			perror("rsh: setsockopt (stderr)");
	}
	(void) setuid(getuid());
#ifndef	pdp11
	omask = sigblock(mask(SIGINT)|mask(SIGQUIT)|mask(SIGTERM));
	signal(SIGINT, sendsig);
	signal(SIGQUIT, sendsig);
	signal(SIGTERM, sendsig);
#else	pdp11
	sigset(SIGINT, sendsig);
	sighold(SIGINT);
	sigset(SIGQUIT, sendsig);
	sighold(SIGQUIT);
	sigset(SIGTERM, sendsig);
	sighold(SIGTERM);
#endif	pdp11
#ifdef NOSELECT
	pid = fork();
	if (pid < 0) {
		perror("rsh: fork");
		exit(1);
	}
	ioctl(rfd2, FIONBIO, &one);
	ioctl(rem, FIONBIO, &one);
	if (pid == 0) {
#ifndef	pdp11
		char *bp; int rembits, wc;
#else	pdp11
		char *bp;
		long rembits;
		int wc;
#endif	pdp11
		(void) close(rfd2);
	reread:
		errno = 0;
		cc = read(0, buf, sizeof buf);
		if (cc <= 0)
			goto done;
		bp = buf;
	rewrite:
#ifndef	pdp11
		rembits = 1<<rem;
#else	pdp11
		rembits = 1L<<rem;
#endif	pdp11
		if (select(16, 0, &rembits, 0, 0) < 0) {
			if (errno != EINTR) {
				perror("rsh: select");
				exit(1);
			}
			goto rewrite;
		}
#ifndef	pdp11
		if ((rembits & (1<<rem)) == 0)
#else	pdp11
		if ((rembits & (1L<<rem)) == 0)
#endif	pdp11
			goto rewrite;
		wc = write(rem, bp, cc);
		if (wc < 0) {
			if (errno == EWOULDBLOCK)
				goto rewrite;
			goto done;
		}
		cc -= wc; bp += wc;
		if (cc == 0)
			goto reread;
		goto rewrite;
	done:
		(void) shutdown(rem, 1);
		exit(0);
	}
#ifndef	pdp11
	sigsetmask(omask);
#else	pdp11
	sigrelse(SIGINT);
	sigrelse(SIGQUIT);
	sigrelse(SIGTERM);
#endif	pdp11
#ifndef	pdp11
	readfrom = (1<<rfd2) | (1<<rem);
#else	pdp11
	readfrom = (1L<<rfd2) | (1L<<rem);
#endif	pdp11
	do {
		ready = readfrom;
		if (select(16, &ready, 0, 0, 0) < 0) {
			if (errno != EINTR) {
				perror("rsh: select");
				exit(1);
			}
			continue;
		}
#ifndef	pdp11
		if (ready & (1<<rfd2)) {
#else	pdp11
		if (ready & (1L<<rfd2)) {
#endif	pdp11
			errno = 0;
			cc = read(rfd2, buf, sizeof buf);
			if (cc <= 0) {
				if (errno != EWOULDBLOCK)
#ifndef	pdp11
					readfrom &= ~(1<<rfd2);
#else	pdp11
					readfrom &= ~(1L<<rfd2);
#endif	pdp11
			} else
				(void) write(2, buf, cc);
		}
#ifndef	pdp11
		if (ready & (1<<rem)) {
#else	pdp11
		if (ready & (1L<<rem)) {
#endif	pdp11
			errno = 0;
			cc = read(rem, buf, sizeof buf);
			if (cc <= 0) {
				if (errno != EWOULDBLOCK)
#ifndef	pdp11
					readfrom &= ~(1<<rem);
#else	pdp11
					readfrom &= ~(1L<<rem);
#endif	pdp11
			} else
				(void) write(1, buf, cc);
		}
	} while (readfrom);
	(void) kill(pid, SIGKILL);
	exit(0);
#else	NOSELECT
	ioctl(rfd2, FIONBIO, &one);
	ioctl(rem, FIONBIO, &one);
#ifndef	pdp11
	readfrom = (1<<rfd2) | (1<<rem);
#else	pdp11
	readfrom = (1L<<rfd2) | (1L<<rem);
#endif	pdp11
	if( noinput == 0)
#ifndef	pdp11
		readfrom |= (1<<0);
#else	pdp11
		readfrom |= (1L<<0);
#endif	pdp11
#ifndef	pdp11
	omask = sigsetmask(omask);	/* unblock */
#else	pdp11
	sigrelse(SIGINT);
	sigrelse(SIGQUIT);
	sigrelse(SIGTERM);
#endif	pdp11
	do {
#ifndef	pdp11
		char *bp; int rembits, wc;
#else	pdp11
		char *bp;
		long rembits;
		int wc;
#endif	pdp11
		extern int interrupt;

		ready = readfrom;
		if (select(16, &ready, 0, 0, 0) < 0) {
			if (errno != EINTR) {
				perror("rsh: select");
				exit(1);
			}
			goto done;
		}
#ifndef	pdp11
		omask = sigsetmask(omask);	/* block */
#else	pdp11
		sighold(SIGINT);
		sighold(SIGQUIT);
		sighold(SIGTERM);
#endif	pdp11
#ifndef	pdp11
		if ((ready & (1<<0))) {
#else	pdp11
		if ((ready & (1L<<0))) {
#endif	pdp11
reread:
			errno = 0;
			cc = read(0, buf, sizeof buf);
			if (cc <= 0) {
#ifndef	pdp11
				readfrom &= ~(1<<0);	
#else	pdp11
				readfrom &= ~(1L<<0);	
#endif	pdp11
				goto cont1;
			}
			bp = buf;
rewrite:
#ifndef	pdp11
			rembits = 1<<rem;
#else	pdp11
			rembits = 1L<<rem;
#endif	pdp11
			if (select(16, 0, &rembits, 0, 0) < 0) {
				if (errno != EINTR) {
					perror("rsh: select");
					exit(1);
				}
				goto rewrite;
			}
#ifndef	pdp11
			if ((rembits & (1<<rem)) == 0)
#else	pdp11
			if ((rembits & (1L<<rem)) == 0)
#endif	pdp11
				goto rewrite;
			wc = write(rem, bp, cc);
			if (wc < 0) {
				if (errno == EWOULDBLOCK)
					goto rewrite;
				goto done;
			}
			cc -= wc; bp += wc;
			if (cc == 0)
				goto cont;
			goto rewrite;
cont1:
		shutdown(rem, 1);
		}
cont:
#ifndef	pdp11
		omask = sigsetmask(omask);	/* unblock */
#else	pdp11
		sigrelse(SIGINT);
		sigrelse(SIGQUIT);
		sigrelse(SIGTERM);
#endif	pdp11

#ifndef	pdp11
		if (ready & (1<<rfd2)) {
#else	pdp11
		if (ready & (1L<<rfd2)) {
#endif	pdp11
			errno = 0;
			cc = read(rfd2, buf, sizeof buf);
			if (cc <= 0) {
				if (errno != EWOULDBLOCK) {
#ifndef	pdp11
					readfrom &= ~(1<<rfd2);
#else	pdp11
					readfrom &= ~(1L<<rfd2);
#endif	pdp11
				}
			} else
				(void) write(2, buf, cc);
		}
#ifndef	pdp11
		if (ready & (1<<rem)) {
#else	pdp11
		if (ready & (1L<<rem)) {
#endif	pdp11
			errno = 0;
			cc = read(rem, buf, sizeof buf);
			if (cc <= 0) {
				if (errno != EWOULDBLOCK) {
#ifndef	pdp11
					readfrom &= ~(1<<rem);
#else	pdp11
					readfrom &= ~(1L<<rem);
#endif	pdp11
					goto done;
				}
			} else
				(void) write(1, buf, cc);
		}
		if (interrupt)
			goto done;
	} while (readfrom);
done:
	(void) shutdown(rem, 2);
	exit(0);


#endif
usage:
	fprintf(stderr,
	    "usage: rsh host [ -l username ] [ -n ] command\n");
	exit(1);
}
int interrupt = 0;

sendsig(signo)
	int signo;
{

	(void) write(rfd2, (char *)&signo, 1);
	interrupt = 1;
}
