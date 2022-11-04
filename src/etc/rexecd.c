
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#ifndef lint
static char sccsid[] = "@(#)rexecd.c	3.0	(ULTRIX-11)	4/22/86";
/*
 * Based on "@(#)rexecd.c	1.2	(ULTRIX)	4/11/85";
 */
#endif

/*-----------------------------------------------------------------------
 *	Modification History
 *
 *	4/5/85 -- jrs
 *		Revise to allow inetd to perform front end functions,
 *		following the Berkeley model.
 *
 *	Based on 4.2BSD labeled:
 *		rexecd.c	4.10	83/07/02
 *
 *-----------------------------------------------------------------------
 */

#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/socket.h>
#ifndef	pdp11
#include <sys/wait.h>
#else	pdp11
#include <wait.h>
#define	NCARGS	5120
#endif	pdp11

#include <netinet/in.h>

#include <stdio.h>
#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <netdb.h>
#include <syslog.h>

extern	errno;
struct	passwd *getpwnam();
char	*crypt(), *rindex(), *sprintf();
/* VARARGS 1 */
int	error();
/*
 * remote execute server:
 *	username\0
 *	password\0
 *	command\0
 *	data
 */
main(argc, argv)
	int argc;
	char **argv;
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
	(void) dup2(0, 3);
	(void) close(0);
	doit(3, &from);
}

char	username[20] = "USER=";
char	homedir[64] = "HOME=";
char	shell[64] = "SHELL=";
char	*envinit[] =
	    {homedir, shell, "PATH=:/usr/ucb:/bin:/usr/bin", username, 0};
char	**environ;

struct	sockaddr_in asin = { AF_INET };

doit(f, fromp)
	int f;
	struct sockaddr_in *fromp;
{
	char cmdbuf[NCARGS+1], *cp, *namep;
	char user[16], pass[16];
	struct passwd *pwd;
	int s;
	short port;
#ifndef	pdp11
	int pv[2], pid, ready, readfrom, cc;
#else	pdp11
	int pv[2], pid, cc;
	long ready, readfrom;
#endif	pdp11
	char buf[BUFSIZ], sig;
	int one = 1;

	(void) signal(SIGINT, SIG_DFL);
	(void) signal(SIGQUIT, SIG_DFL);
	(void) signal(SIGTERM, SIG_DFL);
#ifdef DEBUG
	{ int t = open("/dev/tty", 2);
	  if (t >= 0) {
		ioctl(t, TIOCNOTTY, (char *)0);
		(void) close(t);
	  }
	}
#endif
	(void) dup2(f, 0);
	(void) dup2(f, 1);
	(void) dup2(f, 2);
	(void) alarm(60);
	port = 0;
	for (;;) {
		char c;
		if (read(f, &c, 1) != 1)
			exit(1);
		if (c == 0)
			break;
		port = port * 10 + c - '0';
	}
	(void) alarm(0);
	if (port != 0) {
		s = socket(AF_INET, SOCK_STREAM, 0);
		if (s < 0)
			exit(1);
		if (bind(s, &asin, sizeof (asin)) < 0)
			exit(1);
		(void) alarm(60);
		fromp->sin_port = htons((u_short)port);
		if (connect(s, fromp, sizeof (*fromp)) < 0)
			exit(1);
		(void) alarm(0);
	}
	getstr(user, sizeof(user), "username");
	getstr(pass, sizeof(pass), "password");
	getstr(cmdbuf, sizeof(cmdbuf), "command");
	(void) setpwent();
	pwd = getpwnam(user);
	if (pwd == NULL) {
		error("Login incorrect.\n");
		exit(1);
	}
	(void) endpwent();
	if (*pwd->pw_passwd != '\0') {
		namep = crypt(pass, pwd->pw_passwd);
		if (strcmp(namep, pwd->pw_passwd)) {
			error("Password incorrect.\n");
			exit(1);
		}
	}
	if (chdir(pwd->pw_dir) < 0) {
		error("No remote directory.\n");
		exit(1);
	}
	(void) write(2, "\0", 1);
	if (port) {
		(void) pipe(pv);
		pid = fork();
		if (pid == -1)  {
			error("Try again.\n");
			exit(1);
		}
		if (pid) {
			(void) close(0); (void) close(1); (void) close(2);
			(void) close(f); (void) close(pv[1]);
#ifndef	pdp11
			readfrom = (1<<s) | (1<<pv[0]);
#else	pdp11
			readfrom = (1L<<s) | (1L<<pv[0]);
#endif	pdp11
			ioctl(pv[1], FIONBIO, (char *)&one);
			/* should set s nbio! */
			do {
				ready = readfrom;
				(void) select(16, &ready, 0, 0, 0);
#ifndef	pdp11
				if (ready & (1<<s)) {
#else	pdp11
				if (ready & (1L<<s)) {
#endif	pdp11
					if (read(s, &sig, 1) <= 0)
#ifndef	pdp11
						readfrom &= ~(1<<s);
#else	pdp11
						readfrom &= ~(1L<<s);
#endif	pdp11
					else
						(void) killpg(pid, sig);
				}
#ifndef	pdp11
				if (ready & (1<<pv[0])) {
#else	pdp11
				if (ready & (1L<<pv[0])) {
#endif	pdp11
					cc = read(pv[0], buf, sizeof (buf));
					if (cc <= 0) {
						(void) shutdown(s, 1+1);
#ifndef	pdp11
						readfrom &= ~(1<<pv[0]);
#else	pdp11
						readfrom &= ~(1L<<pv[0]);
#endif	pdp11
					} else
						(void) write(s, buf, cc);
				}
			} while (readfrom);
			exit(0);
		}
		(void) setpgrp(0, getpid());
		(void) close(s); (void)close(pv[0]);
		(void) dup2(pv[1], 2);
	}
	if (*pwd->pw_shell == '\0')
		pwd->pw_shell = "/bin/sh";
	(void) close(f);
#ifndef	pdp11
	initgroups(pwd->pw_name, pwd->pw_gid);
#endif	pdp11
	(void) setuid(pwd->pw_uid);
	(void) setgid(pwd->pw_gid);
	environ = envinit;
	(void) strncat(homedir, pwd->pw_dir, sizeof(homedir)-6);
	(void) strncat(shell, pwd->pw_shell, sizeof(shell)-7);
	(void) strncat(username, pwd->pw_name, sizeof(username)-6);
	cp = rindex(pwd->pw_shell, '/');
	if (cp)
		cp++;
	else
		cp = pwd->pw_shell;
	execl(pwd->pw_shell, cp, "-c", cmdbuf, 0);
	perror(pwd->pw_shell);
	exit(1);
}

/* VARARGS 1 */
error(fmt, a1, a2, a3)
	char *fmt;
	int a1, a2, a3;
{
	char buf[BUFSIZ];

	buf[0] = 1;
	(void) sprintf(buf+1, fmt, a1, a2, a3);
	(void) write(2, buf, strlen(buf));
}

getstr(buf, cnt, err)
	char *buf;
	int cnt;
	char *err;
{
	char c;

	do {
		if (read(0, &c, 1) != 1)
			exit(1);
		*buf++ = c;
		if (--cnt == 0) {
			error("%s too long\n", err);
			exit(1);
		}
	} while (c != 0);
}
