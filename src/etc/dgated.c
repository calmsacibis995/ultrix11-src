
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#ifndef lint
static char Sccsid[] = "@(#)dgated.c	3.0	(ULTRIX-11)	4/22/86";
/*
 * Based on:
 *	"@(#)dlogind.c	1.5	(ULTRIX-32)	5/3/85";
 */
#endif lint

#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include <netinet/in.h>

#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <sgtty.h>
#include <stdio.h>
#include <netdb.h>
#include <syslog.h>

extern	errno;
int pid, Pfd, netf;
char *line;
/*
 * remote dlogin gateway server:
 */
main(argc, argv)
	int argc;
	char **argv;
{
	int f = 0;
	int on = 1;
	char buf[BUFSIZ];
	int cleanup();
	int child;
	int i, p, cc, t, ppid;
	char c;
	int stop = TIOCPKT_DOSTOP;

	
	for (c = 'p'; c <= 'z'; c++) {
		struct stat stb;
		line = "/dev/ptyXX";
		line[strlen("/dev/pty")] = c;
		line[strlen("/dev/ptyp")] = '0';
		if (stat(line, &stb) < 0)
			break;
		for (i = 0; i < 16; i++) {
			line[strlen("/dev/ptyp")] = "0123456789abcdef"[i];
			p = open(line, 2);
			if (p > 0)
				goto gotpty;
		}
	}
	openlog("dgated", LOG_PID);
	syslog(LOG_ERR, "All network ports in use");
	closelog();
	exit(1);
	/*NOTREACHED*/
gotpty:
	Pfd = p;
	netf = f;
	line[strlen("/dev/")] = 't';
#ifdef DEBUG
	{ int tt = open("/dev/tty", 2);
	  if (tt > 0) {
		ioctl(tt, TIOCNOTTY, 0);
		(void) close(tt);
	  }
	}
#endif
	t = open(line, 2);
	if (t < 0) {
		openlog("dgated", LOG_PID);
		syslog (LOG_CRIT, "cannot reopen %s", line);
		closelog();
		exit(1);
	}
	{ struct sgttyb b;
	  gtty(t, &b); b.sg_flags = RAW|ANYP; stty(t, &b);
	}
	pid = fork();
	if (pid < 0) {
		openlog("dgated", LOG_PID);
		syslog(LOG_CRIT, "cannot fork");
		closelog();
		exit(1);
	}
	if (pid) {
		char fibuf[1024], *pbp, *fbp;
		register int fcc = 0;
		int on = 1;

		ioctl(f, FIONBIO, &on);
		ioctl(p, FIONBIO, &on);
		ioctl(p, TIOCPKT, &on);
		signal(SIGTSTP, SIG_IGN);
		signal(SIGCHLD, cleanup);
		for (;;) {
			int ibits = 0;
			
			ibits = (1 << f) | (1 << p);
			select(16, &ibits, (int *) 0, (int *) 0, 0);
			if (!ibits)
				continue;
			if (ibits & (1 << f)) {
				fcc = read(f, fibuf, sizeof(fibuf));
				if(fcc)
					write(p, fibuf, fcc);
				else break;
			}
			if (ibits & (1 << p)) {
				fcc = read(p, fibuf, sizeof(fibuf));
				if(fcc)
					write(f, fibuf, fcc);
				else break;
			}
		}
		cleanup();
	}
	(void) close(f);
	(void) close(p);
	(void) dup2(t, 0);
	(void) dup2(t, 1);
	(void) dup2(t, 2);
	(void) close(t);
	argv[0] = "dlogin";
	execve("/usr/bin/dlogin", argv, (char **) 0);
	openlog("dgated", LOG_PID);
	syslog(LOG_ERR, "cannot exec /usr/bin/dlogin");
	closelog();
	exit(1);
	/*NOTREACHED*/
}

cleanup()
{

	rmut();
	signal(SIGHUP, SIG_IGN);
	close(Pfd);
	vhangup();		/* XXX */
	(void) shutdown(netf, 2);
	(void) kill(0, SIGKILL);
	exit(1);
}

#include <utmp.h>

struct	utmp wtmp;
char	wtmpf[]	= "/usr/adm/wtmp";
char	utmp[] = "/etc/utmp";
#define SCPYN(a, b)	(void) strncpy(a, b, sizeof(a))
#define SCMPN(a, b)	strncmp(a, b, sizeof(a))

rmut()
{
	register f;
	int found = 0;

	f = open(utmp, 2);
	if (f >= 0) {
		while(read(f, (char *)&wtmp, sizeof(wtmp)) == sizeof(wtmp)) {
			if (SCMPN(wtmp.ut_line, line+5) || wtmp.ut_name[0]==0)
				continue;
			(void) lseek(f, -(long)sizeof(wtmp), 1);
			SCPYN(wtmp.ut_name, "");
			SCPYN(wtmp.ut_host, "");
			(void) time(&wtmp.ut_time);
			write(f, (char *)&wtmp, sizeof(wtmp));
			found++;
		}
		(void) close(f);
	}
	if (found) {
		f = open(wtmpf, O_WRONLY|O_APPEND);
		if (f >= 0) {
			SCPYN(wtmp.ut_line, line+5);
			SCPYN(wtmp.ut_name, "");
			SCPYN(wtmp.ut_host, "");
			(void) time(&wtmp.ut_time);
			write(f, (char *)&wtmp, sizeof(wtmp));
			(void) close(f);
		}
	}
	(void) chmod(line, 0666);
	(void) chown(line, 0, 0);
	line[strlen("/dev/")] = 'p';
	(void) chmod(line, 0666);
	(void) chown(line, 0, 0);
}


