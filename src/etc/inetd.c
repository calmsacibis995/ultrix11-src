
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#ifndef lint
/*
 * Based on "@(#)inetd.c	1.1	(ULTRIX)	4/11/85";
 * and "@(#)inetd.c	5.1 (Berkeley) 5/28/85";
 */
static char *Sccsid = "@(#)inetd.c	3.0	(ULTRIX-11)	4/22/86";
#endif

/*-----------------------------------------------------------------------
 *	Modification History
 *
 *	4/10/85 -- jrs
 *		Clean up little nits in code.  Also add timer to select
 *		call so we can momentarily ignore multithreaded datagram
 *		connections in order to give the server time to pick up
 *		the initial packet before we try to listen to the socket again.
 *
 *	Based on 4.2BSD labeled:
 *		inetd.c	4.2	84/05/18
 *
 *-----------------------------------------------------------------------
 */

/*
 * Inetd - Internet super-server
 *
 * This program invokes all internet services as needed.
 * connection-oriented services are invoked each time a
 * connection is made, by creating a process.  This process
 * is passed the connection as file descriptor 0 and is
 * expected to do a getpeername to find out the source host
 * and port.
 *
 * Datagram oriented services are invoked when a datagram
 * arrives; a process is created and passed a pending message
 * on file descriptor 0.  Datagram servers may either connect
 * to their peer, freeing up the original socket for inetd
 * to receive further messages on, or ``take over the socket'',
 * processing all arriving datagrams and, eventually, timing
 * out.	 The first type of server is said to be ``multi-threaded'';
 * the second type of server ``single-threaded''. 
 *
 * Inetd uses a configuration file which is read at startup
 * and, possibly, at some later time in response to a hangup signal.
 * The configuration file is ``free format'' with fields given in the
 * order shown below.  Continuation lines for an entry must being with
 * a space or tab.  All fields must be present in each entry.
 *
 *	service name			must be in /etc/services
 *	socket type			stream/dgram/raw/rdm/seqpacket
 *	protocol			must be in /etc/protocols
 *	wait/nowait			single-threaded/multi-threaded
 *	user				user to run daemon as
 *	server program			full path name
 *	server program arguments	maximum of MAXARGS (5)
 *
 * Comment lines are indicated by a `#' in column 1.
 */

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/file.h>
#ifdef	vax
#include <sys/wait.h>
#else	pdp11
#include <wait.h>
#define	getdtablesize()	NOFILE
#endif	vax

#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>
#include <stdio.h>
#include <signal.h>
#ifdef	pdp11
#define	signal sigset
#endif	pdp11
#include <netdb.h>
#include <syslog.h>
#include <pwd.h>

extern	int errno;

int	reapchild();
char	*index();
char	*malloc();

int	debug = 0;
#ifndef	pdp11
int	allsock;
int	delaysock = 0;
#else	pdp11
long	allsock;
long	delaysock = 0;
#endif	pdp11
#ifndef	pdp11
struct	timeval deltime = { 1, 0 };
struct	timeval *seltimer = NULL;
#else	pdp11
int	deltime = 1;
int	*seltimer = NULL;
#endif	pdp11
int	options;
struct	servent *sp;

struct	servtab {
	char	*se_service;		/* name of service */
	int	se_socktype;		/* type of socket to use */
	char	*se_proto;		/* protocol used */
	short	se_wait;		/* single threaded server */
	short	se_checked;		/* looked at during merge */
	char	*se_user;		/* user name to run as */
	char	*se_server;		/* server program */
#define MAXARGV 5
	char	*se_argv[MAXARGV+1];	/* program arguments */
	int	se_fd;			/* open descriptor */
	struct	sockaddr_in se_ctrladdr;/* bound address */
	struct	servtab *se_next;
} *servtab;

char	*CONFIG = "/etc/inetd.conf";

main(argc, argv)
	int argc;
	char *argv[];
{
	int ctrl;
	register struct servtab *sep;
	register struct passwd *pwd;
	char *cp, buf[50];
	int pid, i;

	argc--, argv++;
	while (argc > 0 && *argv[0] == '-') {
		for (cp = &argv[0][1]; *cp; cp++) switch (*cp) {

		case 'd':
			debug = 1;
			options |= SO_DEBUG;
			break;

		default:
			fprintf(stderr,
			    "inetd: Unknown flag -%c ignored.\n", *cp);
			break;
		}
nextopt:
		argc--, argv++;
	}
	if (argc > 0)
		CONFIG = argv[0];
#ifndef DEBUG
	if (fork())
		exit(0);
	{ int s;
	for (s = 0; s < 10; s++)
		(void) close(s);
	}
	(void) open("/", O_RDONLY);
	(void) dup2(0, 1);
	(void) dup2(0, 2);
	{ int tt = open("/dev/tty", O_RDWR);
	  if (tt > 0) {
		(void) ioctl(tt, TIOCNOTTY, 0);
		(void) close(tt);
	  }
	}
#endif
	openlog("inetd", LOG_PID, 0);
	config();
	(void) signal(SIGHUP, config);
	(void) signal(SIGCHLD, reapchild);
	for (;;) {
#ifndef	pdp11
		int readable, s, ctrl;
#else	pdp11
		long readable;
		int s, ctrl;
#endif	pdp11

		while (allsock == 0)
			sigpause(0);
		readable = allsock;
		if (select(32, &readable, 0, 0, seltimer) <= 0) {
			if (readable == 0 && seltimer != NULL) {
				allsock |= delaysock;
				delaysock = 0;
				seltimer = NULL;
			}
			continue;
		}
		s = ffs(readable)-1;
		if (s < 0)
			continue;
		for (sep = servtab; sep; sep = sep->se_next)
			if (s == sep->se_fd)
				goto found;
		abort(1);
	found:
		if (debug)
			fprintf(stderr, "someone wants %s\n", sep->se_service);
		if (sep->se_socktype == SOCK_STREAM) {
			ctrl = accept(s, 0, 0);
			if (debug)
				fprintf(stderr, "accept, ctrl %d\n", ctrl);
			if (ctrl < 0) {
				if (errno == EINTR)
					continue;
				syslog(LOG_WARNING, "accept: %m");
				continue;
			}
		} else
			ctrl = sep->se_fd;
#ifndef	pdp11
#define mask(sig)	(1L << (sig - 1))
		(void) sigblock(mask(SIGCHLD)|mask(SIGHUP));
#else	pdp11
		sighold(SIGCHLD);
		sighold(SIGHUP);
#endif	pdp11
		pid = fork();
		if (pid < 0) {
			if (sep->se_socktype == SOCK_STREAM)
				(void) close(ctrl);
			sleep(1);
			continue;
		}
		if (sep->se_wait) {
			sep->se_wait = pid;
			allsock &= ~(1L << s);
		} else {
			if (sep->se_socktype == SOCK_DGRAM) {
				delaysock |= (1L << s);
				allsock &= ~(1L << s);
				seltimer = &deltime;
			}
		}
#ifndef	pdp11
		(void) sigsetmask(0);
#else	pdp11
		sigrelse(SIGCHLD);
		sigrelse(SIGHUP);
#endif	pdp11
		if (pid == 0) {
#ifdef	DEBUG
			int tt = open("/dev/tty", O_RDWR);
			if (tt > 0) {
				(void) ioctl(tt, TIOCNOTTY, 0);
				(void) close(tt);
			}
#endif
			(void) dup2(ctrl, 0);
			(void) close(ctrl);
			(void) dup2(0, 1);
			for (i = getdtablesize(); --i > 2; )
				(void) close(i);
			if ((pwd = getpwnam(sep->se_user)) == NULL) {
				syslog(LOG_ERR, "getpwnam: %s: No such user"
					,sep->se_user);
				exit(1);
			}
			(void) setgid(pwd->pw_gid);
#ifndef	pdp11
			initgroups(pwd->pw_name, pwd->pw_gid);
#endif	pdp11
			(void) setuid(pwd->pw_uid);
			if (debug)
				fprintf(stderr, "%d execl %s\n",
				    getpid(), sep->se_server);
			execv(sep->se_server, sep->se_argv);
			if (sep->se_socktype != SOCK_STREAM)
				(void) recv(0, buf, sizeof (buf), 0);
			syslog(LOG_ERR, "execv %s: %m", sep->se_server);
			_exit(1);
		}
		if (sep->se_socktype == SOCK_STREAM)
			(void) close(ctrl);
	}
}

reapchild()
{
	union wait status;
	int pid;
	register struct servtab *sep;

	for (;;) {
#ifndef	pdp11
		pid = wait3(&status, WNOHANG, 0);
#else	pdp11
		pid = wait2(&status, WNOHANG);
#endif	pdp11
		if (pid <= 0)
			break;
		if (debug)
			fprintf(stderr, "%d reaped\n", pid);
		for (sep = servtab; sep; sep = sep->se_next)
			if (sep->se_wait == pid) {
				if (status.w_status)
					syslog(LOG_WARNING,
					    "%s: exit status 0x%x",
					    sep->se_server, status);
				if (debug)
					fprintf(stderr, "restored %s, fd %d\n",
					    sep->se_service, sep->se_fd);
				allsock |= 1L << sep->se_fd;
				sep->se_wait = 1;
			}
	}
}

config()
{
	register struct servtab *sep, *cp, **sepp;
	struct servtab *getconfigent(), *enter();
	int omask, on = 1;

	if (!setconfig()) {
		syslog(LOG_ERR, "%s: %m", CONFIG);
		return;
	}
	for (sep = servtab; sep; sep = sep->se_next)
		sep->se_checked = 0;
	while (cp = getconfigent()) {
		for (sep = servtab; sep; sep = sep->se_next)
			if (strcmp(sep->se_service, cp->se_service) == 0 &&
			    strcmp(sep->se_proto, cp->se_proto) == 0)
				break;
		if (sep != 0) {
			int i;

#ifndef	pdp11
			omask = sigblock(mask(SIGCHLD));
#else	pdp11
			sighold(SIGCHLD);
#endif	pdp11
			sep->se_wait = cp->se_wait;
#define SWAP(a, b) { char *c = a; a = b; b = c; }
			if (cp->se_server)
				SWAP(sep->se_server, cp->se_server);
			for (i = 0; i < MAXARGV; i++)
				SWAP(sep->se_argv[i], cp->se_argv[i]);
#ifndef	pdp11
			(void) sigsetmask(omask);
#else	pdp11
			sigrelse(SIGCHLD);
#endif	pdp11
			freeconfig(cp);
		} else
			sep = enter(cp);
		sep->se_checked = 1;
		if (sep->se_fd != -1)
			continue;
		sp = getservbyname(sep->se_service, sep->se_proto);
		if (sp == 0) {
			syslog(LOG_ERR, "%s/%s: unknown service",
			    sep->se_service, sep->se_proto);
			continue;
		}
		sep->se_ctrladdr.sin_port = sp->s_port;
		if ((sep->se_fd = socket(AF_INET, sep->se_socktype, 0)) < 0) {
			syslog(LOG_ERR, "%s/%s: socket: %m",
			    sep->se_service, sep->se_proto);
			continue;
		}
#define	turnon(fd, opt) \
	setsockopt(fd, SOL_SOCKET, opt, &on, sizeof (on))
		if (strcmp(sep->se_proto, "tcp") == 0 && (options & SO_DEBUG) &&
		    turnon(sep->se_fd, SO_DEBUG) < 0)
			syslog(LOG_ERR, "setsockopt (SO_DEBUG): %m");
		if (turnon(sep->se_fd, SO_REUSEADDR) < 0)
			syslog(LOG_ERR, "setsockopt (SO_REUSEADDR): %m");
#undef turnon
		if (bind(sep->se_fd, &sep->se_ctrladdr,
		    sizeof (sep->se_ctrladdr), 0) < 0) {
			syslog(LOG_ERR, "%s/%s: bind: %m",
			    sep->se_service, sep->se_proto);
			continue;
		}
		if (sep->se_socktype == SOCK_STREAM)
			(void) listen(sep->se_fd, 10);
		allsock |= 1L << sep->se_fd;
	}
	endconfig();
	/*
	 * Purge anything not looked at above.
	 */
#ifndef	pdp11
	omask = sigblock(mask(SIGCHLD));
#else	pdp11
	sighold(SIGCHLD);
#endif	pdp11
	sepp = &servtab;
	while (sep = *sepp) {
		if (sep->se_checked) {
			sepp = &sep->se_next;
			continue;
		}
		*sepp = sep->se_next;
		if (sep->se_fd != -1) {
			allsock &= ~(1L << sep->se_fd);
			if ((delaysock & (1L << sep->se_fd)) != 0) {
				delaysock &= ~(1L << sep->se_fd);
				if (delaysock == 0) {
					seltimer = NULL;
				}
			}
			(void) close(sep->se_fd);
		}
		freeconfig(sep);
		free((char *)sep);
	}
#ifndef	pdp11
	(void) sigsetmask(omask);
#else	pdp11
	sigrelse(SIGCHLD);
#endif	pdp11
}

struct servtab *
enter(cp)
	struct servtab *cp;
{
	register struct servtab *sep;
	int omask, i;
	char *strdup();

	sep = (struct servtab *)malloc(sizeof (*sep));
	if (sep == (struct servtab *)0) {
		syslog(LOG_ERR, "Out of memory.");
		exit(-1);
	}
	*sep = *cp;
	sep->se_fd = -1;
#ifndef	pdp11
	omask = sigblock(mask(SIGCHLD));
	sep->se_next = servtab;
	servtab = sep;
	(void) sigsetmask(omask);
#else	pdp11
	sighold(SIGCHLD);
	sep->se_next = servtab;
	servtab = sep;
	(void) sigrelse(SIGCHLD);
#endif	pdp11
	return (sep);
}

FILE	*fconfig = NULL;
struct	servtab serv;
char	line[256];
char	*skip(), *nextline();

setconfig()
{

	if (fconfig != NULL) {
		(void) fseek(fconfig, 0L, L_SET);
		return (1);
	}
	fconfig = fopen(CONFIG, "r");
	return (fconfig != NULL);
}

endconfig()
{

	if (fconfig == NULL)
		return;
	(void) fclose(fconfig);
	fconfig = NULL;
}

struct servtab *
getconfigent()
{
	register struct servtab *sep = &serv;
	char *cp, *arg;
	int argc;

	while ((cp = nextline(fconfig)) && *cp == '#')
		;
	if (cp == NULL)
		return ((struct servtab *)0);
	sep->se_service = strdup(skip(&cp));
	arg = skip(&cp);
	if (strcmp(arg, "stream") == 0)
		sep->se_socktype = SOCK_STREAM;
	else if (strcmp(arg, "dgram") == 0)
		sep->se_socktype = SOCK_DGRAM;
	else if (strcmp(arg, "rdm") == 0)
		sep->se_socktype = SOCK_RDM;
	else if (strcmp(arg, "seqpacket") == 0)
		sep->se_socktype = SOCK_SEQPACKET;
	else if (strcmp(arg, "raw") == 0)
		sep->se_socktype = SOCK_RAW;
	else
		sep->se_socktype = -1;
	sep->se_proto = strdup(skip(&cp));
	arg = skip(&cp);
	sep->se_wait = strcmp(arg, "wait") == 0;
	sep->se_user = strdup(skip(&cp));
	sep->se_server = strdup(skip(&cp));
	argc = 0;
	for (arg = skip(&cp); cp; arg = skip(&cp))
		if (argc < MAXARGV)
			sep->se_argv[argc++] = strdup(arg);
	while (argc <= MAXARGV)
		sep->se_argv[argc++] = NULL;
	return (sep);
}

freeconfig(cp)
	register struct servtab *cp;
{
	int i;

	if (cp->se_service)
		free(cp->se_service);
	if (cp->se_proto)
		free(cp->se_proto);
	if (cp->se_server)
		free(cp->se_server);
	for (i = 0; i < MAXARGV; i++)
		if (cp->se_argv[i])
			free(cp->se_argv[i]);
}

char *
skip(cpp)
	char **cpp;
{
	register char *cp = *cpp;
	char *start;

again:
	while (*cp == ' ' || *cp == '\t')
		cp++;
	if (*cp == '\0') {
		char c;

		c = getc(fconfig);
		(void) ungetc(c, fconfig);
		if (c == ' ' || c == '\t')
			if (cp = nextline(fconfig))
				goto again;
		*cpp = (char *)0;
		return ((char *)0);
	}
	start = cp;
	while (*cp && *cp != ' ' && *cp != '\t')
		cp++;
	if (*cp != '\0')
		*cp++ = '\0';
	*cpp = cp;
	return (start);
}

char *
nextline(fd)
	FILE *fd;
{
	char *cp;

	if (fgets(line, sizeof (line), fd) == NULL)
		return ((char *)0);
	cp = index(line, '\n');
	if (cp)
		*cp = '\0';
	return (line);
}

char *
strdup(cp)
	char *cp;
{
	char *new;

	if (cp == NULL)
		cp = "";
	new = malloc(strlen(cp) + 1);
	if (new == (char *)0) {
		syslog(LOG_ERR, "Out of memory.");
		exit(-1);
	}
	(void) strcpy(new, cp);
	return (new);
}
