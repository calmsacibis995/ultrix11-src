
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#ifndef lint
static	char	*sccsid = "@(#)rdate.c	3.0	(ULTRIX-11)	4/22/86";
#endif lint

/*-----------------------------------------------------------------------
 *	Modification History
 *
 *	4/5/85 -- jrs
 *		Created to allow machines to set time from network.
 *		Based on a concept by Marshall Rose of UC Irvine
 *		and the internet specifications for time server.
 *
 *-----------------------------------------------------------------------
 */

/*
 *	The syntax for this client is:
 *	rdate [-sv] [network]
 *
 *	where: 
 *		-s	Set time from network median
 *		-v	Print time for each responding network host
 *		network	The network is poll for time
 *
 *	If no switches are set, rdate will just report the network median time
 *	If no network is specified, rdate will use the host's primary network.
 *
 *	It is intended that rdate will normally be used in the /etc/rc file
 *	with the -s switch to set the system date.  This is especially useful
 *	on machines such as MicroVax I's that have no t.o.y. clock.
 */

#include <netdb.h>
#include <stdio.h>
#include <utmp.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/socket.h>
#ifndef	pdp11
#include <sys/time.h>
#endif	pdp11
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <net/if.h>

#define	WTMP	"/usr/adm/wtmp"
#define	RESMAX	100

struct	utmp wtmp[2] = { { "|", "", "", 0 }, { "{", "", "", 0 } };

struct	servent *getservbyname();
struct	netent *getnetbyname();
struct	hostent *gethostbyname();
#ifndef	pdp11
struct	in_addr inet_makeaddr();
#else	pdp11
struct	in_addr inet_mkaddr();
#endif	pdp11
int	tcomp();

main(argc, argv)
int	argc;
char	**argv;
{
	int	set = 0;
	int	verbose = 0;
	char	*net = NULL;
	int on = 1;
	struct	servent	*tserv;
	struct	netent	*tnet;
	struct	hostent	*thost;
	struct	sockaddr_in netaddr;
#ifndef	pdp11
	struct	timeval baset, nowt, timeout;
	struct	timezone basez, nowz;
#else	pdp11
	long	baset, nowt;
	int	timeout;
#endif	pdp11
	char	hostnam[32], resbuf[16], *swtp;
#ifndef	pdp11
	int	argp, wtmpfd, tsock, readsel, writesel, selmax, selvalue;
#else	pdp11
	int	argp, wtmpfd, tsock, selmax, selvalue;
	long	readsel, writesel;
#endif	pdp11
	int	rescount, median, addrsiz;
#ifndef	pdp11
	unsigned long reslist[RESMAX], resvalue;
#else	pdp11
	long reslist[RESMAX], resvalue;
#endif	pdp11
	struct ifreq ifreq;

	for (argp = 1; argp < argc; argp++) {
		if (*argv[argp] == '-') {
			for (swtp = &argv[argp][1]; *swtp != '\0'; swtp++) {
				switch (*swtp) {

				case 's':	/* set time */
					set = 1;
					break;

				case 'v':	/* verbose report */
					verbose = 1;
					break;

				default:
					fprintf(stderr,
						"%s: Unknown switch - %c\n",
						argv[0], *swtp);
					exit(1);
				}
			}
		} else {
			net = argv[argp];
		}
	}

	/* set up the socket and define the base time */

	if ((tsock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		fprintf(stderr, "%s: socket create failure\n", argv[0]);
		exit(1);
	}
	if (setsockopt(tsock, SOL_SOCKET, SO_BROADCAST, &on, sizeof (on)) < 0) {
		fprintf(stderr, "%s: set broadcast failure\n", argv[0]);
		exit(1);
	}

	/* research network and service information */

	if ((tserv = getservbyname("time", "udp")) == NULL) {
		fprintf(stderr, "%s: Time service unknown\n", argv[0]);
		exit(1);
	}
	netaddr.sin_family = AF_INET;
	netaddr.sin_port = tserv->s_port;
	if (net == NULL) {
		(void) gethostname(hostnam, sizeof(hostnam));
		if ((thost = gethostbyname(hostnam)) == NULL) {
			fprintf(stderr, "%s: Host name not known\n", argv[0]);
			exit(1);
		}
		/* next line is for subnets */
		netaddr.sin_addr = inet_makeaddr(inet_netof(
				*(struct in_addr *)thost->h_addr), INADDR_ANY);

	} else {
		if ((tnet = getnetbyname(net)) == NULL) {
			fprintf(stderr, "%s: Unknown network - %s\n",
					argv[0], net);
		}
		(void) strncpy(&netaddr.sin_addr, &tnet->n_net, sizeof(struct in_addr));
	}

#ifndef	pdp11
	(void) gettimeofday(&baset, &basez);
#else	pdp11
	time(&baset);
#endif	pdp11

	/* set up for select, then yell for someone to tell us the time */

#ifndef	pdp11
	timeout.tv_sec = 2;
	timeout.tv_usec = 0;
	readsel = 1 << tsock;
#else	pdp11
	timeout = 2;
	readsel = 1L << tsock;
#endif	pdp11
	writesel = 0;
	selmax = tsock + 1;
	rescount = 0;

	if (sendto(tsock, resbuf, sizeof(resbuf), 0, &netaddr,
				sizeof(netaddr)) < 0) {
		fprintf(stderr, "%s: socket send failure\n", argv[0]);
		exit(1);
	}

	/* loop for incoming packets.  We will break out on error
	   or timeout period expiration */

#ifndef	pdp11
	while ((selvalue = select(selmax, &readsel, &writesel, &writesel,
				&timeout)) > 0) {
#else	pdp11
	while ((selvalue = select(selmax, &readsel, 0, 0, &timeout)) > 0) {
#endif	pdp11

		/* reset for next select */

#ifndef	pdp11
		timeout.tv_sec = 2;
		timeout.tv_usec = 0;
		readsel = 1 << tsock;
#else	pdp11
		timeout = 2;
		readsel = 1L << tsock;
#endif	pdp11
		writesel = 0;
		selmax = tsock + 1;
		
		/* try to pick up packet */

		addrsiz = sizeof(netaddr);
		if (recvfrom(tsock, resbuf, sizeof(resbuf), 0, &netaddr,
				&addrsiz) != sizeof(resvalue)) {
			continue;
		}
		
		/* this little piece of code is to insure that all
		   incoming times are stamped from same base time */

#ifndef	pdp11
		(void) gettimeofday(&nowt, &nowz);
		resvalue = ntohl(*(unsigned long *)resbuf) - 2208988800l;
		reslist[rescount++] = resvalue - (nowt.tv_sec - baset.tv_sec);
#else	pdp11
		time(&nowt);
		resvalue = ntohl(*(long *)resbuf) - 2208988800L;
		reslist[rescount++] = resvalue - (nowt - baset);
#endif	pdp11

		/* if we are verbose, explain what we just got */

		if (verbose != 0) {
			thost = gethostbyaddr(&netaddr.sin_addr,
					sizeof(netaddr.sin_addr), AF_INET);
			printf("%s: %s", (thost == NULL)? "*Unknown*":
					thost->h_name, ctime(&resvalue));
		}

		/* if list is full, we are done */

		if (rescount >= RESMAX) {
			selvalue = 0;
			break;
		}
	}

	/* make sure we did not end abnormally */

	if (selvalue != 0) {
		fprintf(stderr, "%s: select failure\n", argv[0]);
		exit(1);
	}

	/* cheap exit if time list is empty */

	if (rescount == 0) {
		printf("Network time indeterminate\n");
		exit(0);
	}

	/* sort the time list and pick median */

	qsort(reslist, rescount, sizeof(resvalue), tcomp);
	median = (rescount - 1) / 2;

	/* adjust selected value from base time to present */

#ifndef	pdp11
	(void) gettimeofday(&nowt, &nowz);
	resvalue = reslist[median] + (nowt.tv_sec - baset.tv_sec);
#else	pdp11
	time(&nowt);
	resvalue = reslist[median] + (nowt - baset);
#endif	pdp11

	/* if setting, do it, otherwise just print conclusions */

	if (set == 0) {
		fprintf(stderr, "Network time is %s", ctime(&resvalue));
	} else {
#ifndef	pdp11
		wtmp[0].ut_time = nowt.tv_sec;
		wtmp[1].ut_time = resvalue;
		nowt.tv_sec = resvalue;
		if (settimeofday(&nowt, &nowz) != 0) {
			fprintf(stderr, "%s: Time set failed\n", argv[0]);
			exit(1);
		}
#else	pdp11
		wtmp[0].ut_time = nowt;
		wtmp[1].ut_time = resvalue;
		nowt = resvalue;
		if (stime(&nowt) != 0) {
			fprintf(stderr, "%s: Time set failed\n", argv[0]);
			exit(1);
		}
#endif	pdp11
		if ((wtmpfd = open(WTMP, O_WRONLY|O_APPEND)) >= 0) {
			(void) write(wtmpfd, wtmp, sizeof(wtmp));
			(void) close(wtmpfd);
		}
		printf("Time set to %s", ctime(&resvalue));
	}
}

/*
 *	This function aids the sort in the main routine.
 *	It compares two unsigned longs and returns accordingly.
 */

#ifndef	pdp11
tcomp(first, second)
unsigned long *first, *second;
{
	if (*first < *second) {
		return(-1);
	} else if (*first > *second) {
		return(1);
	} else {
		return(0);
	}
}
#else	pdp11
tcomp(first, second)
long *first, *second;
{
	if (*first < 0 && *second >= 0) {
		return(1);
	} else if (*first >= 0 && *second < 0) {
		return(-1);
	} else if (*first < *second) {
		return(-1);
	} else if (*first > *second) {
		return(1);
	} else {
		return(0);
	}
}
#endif	pdp11
