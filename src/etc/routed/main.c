
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
/*
 * char copyright[] =
 * "@(#) Copyright (c) 1983 Regents of the University of California.\n\
 *  All rights reserved.\n";
 */
#endif not lint

#ifndef lint
/*
 * Based on static char sccsid[] = "@(#)main.c	5.1 (Berkeley) 6/4/85";
 */
static char *Sccsid = "@(#)main.c	3.0	(ULTRIX-11)	4/22/86";
#endif not lint

/*
 * Routing Table Management Daemon
 */
#include "defs.h"
#include <sys/ioctl.h>
#ifndef	pdp11
#include <sys/time.h>
#endif	pdp11

#include <net/if.h>

#include <errno.h>
#include <nlist.h>
#include <signal.h>
#ifdef	pdp11
#define	signal	sigset
#endif	pdp11
#include <syslog.h>

int	supplier = -1;		/* process should supply updates */
int	gateway = 0;		/* 1 if we are a gateway to parts beyond */

struct	rip *msg = (struct rip *)packet;
int	hup();

main(argc, argv)
	int argc;
	char *argv[];
{
	int cc;
	struct sockaddr from;
	u_char retry;
	
	argv0 = argv;
	openlog("routed", LOG_PID, 0);
	sp = getservbyname("router", "udp");
	if (sp == NULL) {
		fprintf(stderr, "routed: router/udp: unknown service\n");
		exit(1);
	}
	addr.sin_family = AF_INET;
	addr.sin_port = sp->s_port;
	s = getsocket(AF_INET, SOCK_DGRAM, &addr);
	if (s < 0)
		exit(1);
	argv++, argc--;
	while (argc > 0 && **argv == '-') {
		if (strcmp(*argv, "-s") == 0) {
			supplier = 1;
			argv++, argc--;
			continue;
		}
		if (strcmp(*argv, "-q") == 0) {
			supplier = 0;
			argv++, argc--;
			continue;
		}
		if (strcmp(*argv, "-t") == 0) {
			tracepackets++;
			argv++, argc--;
			continue;
		}
		if (strcmp(*argv, "-g") == 0) {
			gateway = 1;
			argv++, argc--;
			continue;
		}
		fprintf(stderr,
			"usage: routed [ -s ] [ -q ] [ -t ] [ -g ]\n");
		exit(1);
	}
#ifndef DEBUG
	if (!tracepackets) {
		int t;

		if (fork())
			exit(0);
		for (t = 0; t < 20; t++)
			if (t != s)
				(void) close(cc);
		(void) open("/", 0);
		(void) dup2(0, 1);
		(void) dup2(0, 2);
#ifndef	pdp11
		t = open("/dev/tty", 2);
		if (t >= 0) {
			ioctl(t, TIOCNOTTY, (char *)0);
			(void) close(t);
		}
#else	pdp11
		zaptty();
#endif	pdp11
	}
#endif
	/*
	 * Any extra argument is considered
	 * a tracing log file.
	 */
	if (argc > 0)
		traceon(*argv);
	/*
	 * Collect an initial view of the world by
	 * checking the interface configuration and the gateway kludge
	 * file.  Then, send a request packet on all
	 * directly connected networks to find out what
	 * everyone else thinks.
	 */
	rtinit();
	gwkludge();
	ifinit();
	if (gateway > 0)
		rtdefault();
	if (supplier < 0)
		supplier = 0;
	msg->rip_cmd = RIPCMD_REQUEST;
	msg->rip_vers = RIPVERSION;
	msg->rip_nets[0].rip_dst.sa_family = AF_UNSPEC;
	msg->rip_nets[0].rip_metric = HOPCNT_INFINITY;
	msg->rip_nets[0].rip_dst.sa_family = htons(AF_UNSPEC);
	msg->rip_nets[0].rip_metric = htonl(HOPCNT_INFINITY);
	toall(sendmsg);
	signal(SIGALRM, timer);
	signal(SIGHUP, hup);
	signal(SIGTERM, hup);
	timer();

	for (;;) {
#ifndef	pdp11
		int ibits;
#else	pdp11
		long ibits;
#endif	pdp11
		register int n;

#ifndef	pdp11
		ibits = 1 << s;
#else	pdp11
		ibits = 1L << s;
#endif	pdp11
		n = select(20, &ibits, 0, 0, 0);
		if (n < 0)
			continue;
#ifndef	pdp11
		if (ibits & (1 << s))
#else	pdp11
		if (ibits & (1L << s))
#endif	pdp11
			process(s);
		/* handle ICMP redirects */
	}
}

process(fd)
	int fd;
{
	struct sockaddr from;
	int fromlen = sizeof (from), cc, omask;

	cc = recvfrom(fd, packet, sizeof (packet), 0, &from, &fromlen);
	if (cc <= 0) {
		if (cc < 0 && errno != EINTR)
			perror("recvfrom");
		return;
	}
	if (fromlen != sizeof (struct sockaddr_in))
		return;
#ifndef	pdp11
#define	mask(s)	(1<<((s)-1))
	omask = sigblock(mask(SIGALRM));
	rip_input(&from, cc);
	sigsetmask(omask);
#else	pdp11
	sighold(SIGALRM);
	rip_input(&from, cc);
	sigrelse(SIGALRM);
#endif	pdp11
}

getsocket(domain, type, sin)
	int domain, type;
	struct sockaddr_in *sin;
{
	int retry, s, on = 1;

	retry = 1;
	while ((s = socket(domain, type, 0, 0)) < 0 && retry) {
		perror("socket");
		sleep(5 * retry);
		retry <<= 1;
	}
	if (retry == 0) {
		syslog(LOG_ERR, "socket: %m");
		return (-1);
	}
	if (setsockopt(s, SOL_SOCKET, SO_BROADCAST, &on, sizeof (on)) < 0) {
		syslog(LOG_ERR, "setsockopt SO_BROADCAST: %m");
		exit(1);
	}
	while (bind(s, sin, sizeof (*sin), 0) < 0 && retry) {
		perror("bind");
		sleep(5 * retry);
		retry <<= 1;
	}
	if (retry == 0) {
		syslog(LOG_ERR, "bind: %m");
		return (-1);
	}
	return (s);
}
