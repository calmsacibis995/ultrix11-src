#ifndef lint
/*
 * Based on "@(#)rlogind.c	1.4	(ULTRIX)	4/11/85";
 */
static char *sccsid = "@(#)rlogind.c	3.1	(ULTRIX-11)	7/16/87";
#endif lint

/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985.	      *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/include/COPYRIGHT" for applicable restrictions.  *
 **********************************************************************/
/*-----------------------------------------------------------------------
 *	Modification History
 *
 *	4/5/85 -- jrs
 *		Revise to allow inetd to perform front end functions,
 *		following the Berkeley model. Revisions based on
 *		BSD labeled:
 *			rlogind.c	4.22	9/13/84
 *
 *-----------------------------------------------------------------------
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/socket.h>
#ifndef	pdp11
#include <sys/wait.h>
#else	pdp11
#include <wait.h>
#endif	pdp11

#include <netinet/in.h>

#include <errno.h>
#include <pwd.h>
#include <signal.h>
#ifdef	pdp11
#define	signal	sigset
#endif	pdp11
#include <sgtty.h>
#include <stdio.h>
#include <netdb.h>
#include <syslog.h>

extern	errno;
struct	passwd *getpwnam();
char	*ntoa();
/*
 * remote login server:
 *	remuser\0
 *	locuser\0
 *	terminal type\0
 *	data
 */
main(argc, argv)
	int argc;
	char **argv;
{
	int on = 1, fromlen;
	struct sockaddr_in from;

	fromlen = sizeof (from);
	if (getpeername(0, &from, &fromlen) < 0) {
		openlog(argv[0], LOG_PID);
		syslog(LOG_ERR, "getpeername: %m");
		closelog();
		exit(1);
	}
	if (setsockopt(0, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof (on)) < 0) {
		openlog(argv[0], LOG_PID);
		syslog(LOG_WARNING, "setsockopt (SO_KEEPALIVE): %m");
		closelog();
	}
	doit(0, &from);
}

char	buf[BUFSIZ];
int	cleanup();
int	netf;
extern	errno;
char	*line;
int p, f;
doit(f, fromp)
	struct sockaddr_in *fromp;
{
	char c = 0;
	int i,  cc, t, pid;
	register struct hostent *hp;

	(void) alarm(60);
	read(f, &c, 1);
	if (c != 0)
		exit(1);
	(void) alarm(0);
	fromp->sin_port = htons((u_short)fromp->sin_port);
	hp = gethostbyaddr(&fromp->sin_addr, sizeof (struct in_addr),
		fromp->sin_family);
	if (hp == 0) {
		char buf[BUFSIZ];

		fatal(f, sprintf(buf, "Host name for your address (%s) unknown",
			ntoa(fromp->sin_addr)));
	}
	if (fromp->sin_family != AF_INET ||
	    fromp->sin_port >= IPPORT_RESERVED ||
	    hp == 0)
		fatal(f, "Permission denied");
	write(f, "", 1);
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
	fatal(f, "All network ports in use");
	/*NOTREACHED*/
gotpty:
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
	if (t < 0)
		fatalperror(f, line, errno);
	{ struct sgttyb b;
	  gtty(t, &b); b.sg_flags = RAW|ANYP; stty(t, &b);
	}
	pid = fork();
	if (pid < 0)
		fatalperror(f, "", errno);
	if (pid) {
		char pibuf[1024], fibuf[1024], *pbp, *fbp;
		register int pcc = 0, fcc = 0;
		int on = 1;
/* FILE *console = fopen("/dev/console", "w");  */
/* setbuf(console, 0); */

/* fprintf(console, "f %d p %d\r\n", f, p); */
		ioctl(f, FIONBIO, &on);
		ioctl(p, FIONBIO, &on);
		ioctl(p, TIOCPKT, &on);
		signal(SIGTSTP, SIG_IGN);
		signal(SIGINT, SIG_IGN);
		signal(SIGQUIT, SIG_IGN);
		signal(SIGCHLD, cleanup);
		for (;;) {
#ifndef	pdp11
			int ibits = 0, obits = 0;
#else	pdp11
			long ibits = 0, obits = 0;
#endif	pdp11

			if (fcc)
				obits |= (1L<<p);
			else
				ibits |= (1L<<f);
			if (pcc >= 0)
				if (pcc)
					obits |= (1L<<f);
				else
					ibits |= (1L<<p);
			if (fcc < 0 && pcc < 0)
				break;
/* fprintf(console, "ibits from %d obits from %d\r\n", ibits, obits); */
			select(16, &ibits, &obits, 0, 0);
/* fprintf(console, "ibits %d obits %d\r\n", ibits, obits); */
			if (ibits == 0 && obits == 0) {
				sleep(5);
				continue;
			}
			if (ibits & (1L<<f)) {
				fcc = read(f, fibuf, sizeof (fibuf));
/* fprintf(console, "%d from f\r\n", fcc); */
				if (fcc < 0 && errno == EWOULDBLOCK)
					fcc = 0;
				else {
					if (fcc <= 0)
						break;
					fbp = fibuf;
				}
			}
			if (ibits & (1L<<p)) {
				pcc = read(p, pibuf, sizeof (pibuf));
/* fprintf(console, "%d from p, buf[0] %x, errno %d\r\n", pcc, buf[0], errno); */
				pbp = pibuf;
				if (pcc < 0 && errno == EWOULDBLOCK)
					pcc = 0;
				else if (pcc <= 0)
					pcc = -1;
				else if (pibuf[0] == 0)
					pbp++, pcc--;
				else {
					if (pibuf[0]&(TIOCPKT_FLUSHWRITE|
						      TIOCPKT_NOSTOP|
						      TIOCPKT_DOSTOP)) {
						pibuf[0] &= (TIOCPKT_NOSTOP|
						     TIOCPKT_DOSTOP);
						send(f,&pibuf[0],1,MSG_OOB);
					}
					pcc = 0;
				}
			}
			if ((obits & (1L<<f)) && pcc > 0) {
				cc = write(f, pbp, pcc);
/* fprintf(console, "%d of %d to f\r\n", cc, pcc); */
				if (cc > 0) {
					pcc -= cc;
					pbp += cc;
				}
			}
			if ((obits & (1L<<p)) && fcc > 0) {
				cc = write(p, fbp, fcc);
/* fprintf(console, "%d of %d to p\r\n", cc, fcc); */
				if (cc > 0) {
					fcc -= cc;
					fbp += cc;
				}
			}
		}
		cleanup();
	}
	/* Make ourselves the process group leader */
	pid = getpid();
	setpgrp(0, pid);
	ioctl(t, TIOCSPGRP, &pid);

	(void) close(f);
	(void) close(p);
	(void) dup2(t, 0);
	(void) dup2(t, 1);
	(void) dup2(t, 2);
	(void) close(t);
	execl("/bin/login", "login", "-r", hp->h_name, 0);
	fatalperror(2, "/bin/login", errno);
	/*NOTREACHED*/
}

cleanup()
{

	char pibuf[1024];
	int pcc;
	rmut();
#ifndef	pdp11
	vhangup();		/* XXX */
#endif	pdpd11
	/*
	 * check to see if there is any output pending for the
	 * network before we close everything down. (Sometimes
	 * there is data that hasn't been read from the pseudo tty
	 * and written out to the net.
	 * David Roberts 05/10/85.
	 */
	if ((pcc = read(p, pibuf, sizeof (pibuf))) > 0)
		write(f, pibuf, pcc);
	(void) shutdown(netf, 2);
	(void) kill(0, SIGKILL);
	exit(1);
}

fatal(f, msg)
	int f;
	char *msg;
{
	char buf[BUFSIZ];

	buf[0] = '\01';		/* error indicator */
	(void) sprintf(buf + 1, "rlogind: %s.\r\n", msg);
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
	sprintf(b, "%d.%d.%d.%d", UC(p[0]), UC(p[1]), UC(p[2]), UC(p[3]));
	return (b);
}
