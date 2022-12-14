
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
 * Based on static char sccsid[] = "@(#)if.c	5.2 (Berkeley) 6/15/85";
 */
static char *Sccsid = "@(#)if.c	3.0	(ULTRIX-11)	4/22/86";
#endif not lint

#include <sys/types.h>
#include <sys/socket.h>

#include <net/if.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#ifdef	NS
#include <netns/ns.h>
#endif

#include <stdio.h>

extern	int kmem;
extern	int tflag;
extern	int nflag;
extern	char *interface;
extern	int unit;
extern	char *routename(), *netname();

/*
 * Print a description of the network interfaces.
 */
intpr(interval, ifnetaddr)
	int interval;
#ifndef	pdp11
	off_t ifnetaddr;
#else	pdp11
	caddr_t ifnetaddr;
#endif	pdp11
{
	struct ifnet ifnet;
	union {
		struct ifaddr ifa;
		struct in_ifaddr in;
	} ifaddr;
#ifndef	pdp11
	off_t ifaddraddr;
#else	pdp11
	caddr_t ifaddraddr;
	long htonl();
#endif	pdp11
	char name[16];

	if (ifnetaddr == 0) {
		printf("ifnet: symbol not defined\n");
		return;
	}
	if (interval) {
		sidewaysintpr(interval, ifnetaddr);
		return;
	}
	klseek(kmem, ifnetaddr, 0);
	read(kmem, &ifnetaddr, sizeof ifnetaddr);
	printf("%-5.5s %-5.5s %-10.10s  %-12.12s %-7.7s %-5.5s %-7.7s %-5.5s",
		"Name", "Mtu", "Network", "Address", "Ipkts", "Ierrs",
		"Opkts", "Oerrs");
	printf(" %-6.6s", "Collis");
	if (tflag)
		printf(" %-6.6s", "Timer");
	putchar('\n');
	ifaddraddr = 0;
	while (ifnetaddr || ifaddraddr) {
		struct sockaddr_in *sin;
		register char *cp;
		int n;
		char *index();
#ifndef	pdp11
		struct in_addr in, inet_makeaddr();
#else	pdp11
		struct in_addr in, inet_mkaddr();
#endif	pdp11

		if (ifaddraddr == 0) {
			klseek(kmem, ifnetaddr, 0);
			read(kmem, &ifnet, sizeof ifnet);
#ifndef	pdp11
			klseek(kmem, (off_t)ifnet.if_name, 0);
#else	pdp11
			klseek(kmem, (caddr_t)ifnet.if_name, 0);
#endif	pdp11
			read(kmem, name, 16);
			name[15] = '\0';
#ifndef	pdp11
			ifnetaddr = (off_t) ifnet.if_next;
#else	pdp11
			ifnetaddr = (caddr_t) ifnet.if_next;
#endif	pdp11
			if (interface != 0 &&
			    (strcmp(name, interface) != 0 || unit != ifnet.if_unit))
				continue;
			cp = index(name, '\0');
			*cp++ = ifnet.if_unit + '0';
			if ((ifnet.if_flags&IFF_UP) == 0)
				*cp++ = '*';
			*cp = '\0';
#ifndef	pdp11
			ifaddraddr = (off_t)ifnet.if_addrlist;
#else	pdp11
			ifaddraddr = (caddr_t)ifnet.if_addrlist;
#endif	pdp11
		}
		printf("%-5.5s %-5d ", name, ifnet.if_mtu);
		if (ifaddraddr == 0) {
			printf("%-10.10s  ", "none");
			printf("%-12.12s ", "none");
		} else {
			klseek(kmem, ifaddraddr, 0);
			read(kmem, &ifaddr, sizeof ifaddr);
#ifndef	pdp11
			ifaddraddr = (off_t)ifaddr.ifa.ifa_next;
#else	pdp11
			ifaddraddr = (caddr_t)ifaddr.ifa.ifa_next;
#endif	pdp11
			switch (ifaddr.ifa.ifa_addr.sa_family) {
			case AF_UNSPEC:
				printf("%-10.10s  ", "none");
				printf("%-12.12s ", "none");
				break;
			case AF_INET:
				sin = (struct sockaddr_in *)&ifaddr.in.ia_addr;
#ifdef notdef
				/* can't use inet_makeaddr because kernel
				 * keeps nets unshifted.
				 */
				in = inet_makeaddr(ifaddr.in.ia_subnet,
					INADDR_ANY);
				printf("%-10.10s  ", netname(in));
#else
				printf("%-10.10s  ",
					netname(htonl(ifaddr.in.ia_subnet),
						ifaddr.in.ia_subnetmask));
#endif
				printf("%-12.12s ", routename(sin->sin_addr));
				break;
#ifdef	NS
			case AF_NS:
				{
				struct sockaddr_ns *sns =
				(struct sockaddr_ns *)&ifaddr.in.ia_addr;
				printf("ns:%-8d ",
					ntohl(ns_netof(sns->sns_addr)));
				printf("%-12s ",ns_phost(sns));
				}
				break;
#endif	NS
			default:
				printf("af%2d: ", ifaddr.ifa.ifa_addr.sa_family);
				for (cp = (char *)&ifaddr.ifa.ifa_addr +
				    sizeof(struct sockaddr) - 1;
				    cp >= ifaddr.ifa.ifa_addr.sa_data; --cp)
					if (*cp != 0)
						break;
				n = cp - (char *)ifaddr.ifa.ifa_addr.sa_data + 1;
				cp = (char *)ifaddr.ifa.ifa_addr.sa_data;
				if (n <= 6)
					while (--n)
						printf("%02d.", *cp++ & 0xff);
				else
					while (--n)
						printf("%02d", *cp++ & 0xff);
				printf("%02d ", *cp & 0xff);
				break;
			}
		}
#ifndef	pdp11
		printf("%-7d %-5d %-7d %-5d %-6d",
#else	pdp11
		printf("%-7u %-5u %-7u %-5u %-6u",
#endif	pdp11
		    ifnet.if_ipackets, ifnet.if_ierrors,
		    ifnet.if_opackets, ifnet.if_oerrors,
		    ifnet.if_collisions);
		if (tflag)
			printf(" %-6d", ifnet.if_timer);
		putchar('\n');
	}
}

#define	MAXIF	10
struct	iftot {
	char	ift_name[16];		/* interface name */
	int	ift_ip;			/* input packets */
	int	ift_ie;			/* input errors */
	int	ift_op;			/* output packets */
	int	ift_oe;			/* output errors */
	int	ift_co;			/* collisions */
} iftot[MAXIF];

/*
 * Print a running summary of interface statistics.
 * Repeat display every interval seconds, showing
 * statistics collected over that interval.  First
 * line printed at top of screen is always cumulative.
 */
sidewaysintpr(interval, off)
	int interval;
#ifndef	pdp11
	off_t off;
#else	pdp11
	caddr_t off;
#endif	pdp11
{
	struct ifnet ifnet;
#ifndef	pdp11
	off_t firstifnet;
#else	pdp11
	caddr_t firstifnet;
#endif	pdp11
	register struct iftot *ip, *total;
	register int line;
	struct iftot *lastif, *sum, *interesting;
	int maxtraffic;

	klseek(kmem, off, 0);
#ifndef	pdp11
	read(kmem, &firstifnet, sizeof (off_t));
#else	pdp11
	read(kmem, &firstifnet, sizeof (firstifnet));
#endif	pdp11
	lastif = iftot;
	sum = iftot + MAXIF - 1;
	total = sum - 1;
	interesting = iftot;
	for (off = firstifnet, ip = iftot; off;) {
		char *cp;

		klseek(kmem, off, 0);
		read(kmem, &ifnet, sizeof ifnet);
		klseek(kmem, (int)ifnet.if_name, 0);
		ip->ift_name[0] = '(';
		read(kmem, ip->ift_name + 1, 15);
		if (interface && strcmp(ip->ift_name + 1, interface) == 0 &&
		    unit == ifnet.if_unit)
			interesting = ip;
		ip->ift_name[15] = '\0';
		cp = index(ip->ift_name, '\0');
		sprintf(cp, "%d)", ifnet.if_unit);
		ip++;
		if (ip >= iftot + MAXIF - 2)
			break;
#ifndef	pdp11
		off = (off_t) ifnet.if_next;
#else	pdp11
		off = (caddr_t) ifnet.if_next;
#endif	pdp11
	}
	lastif = ip;
banner:
	printf("    input   %-6.6s    output       ", interesting->ift_name);
	if (lastif - iftot > 0)
		printf("   input  (Total)    output       ");
	for (ip = iftot; ip < iftot + MAXIF; ip++) {
		ip->ift_ip = 0;
		ip->ift_ie = 0;
		ip->ift_op = 0;
		ip->ift_oe = 0;
		ip->ift_co = 0;
	}
	putchar('\n');
	printf("%-7.7s %-5.5s %-7.7s %-5.5s %-5.5s ",
		"packets", "errs", "packets", "errs", "colls");
	if (lastif - iftot > 0)
		printf("%-7.7s %-5.5s %-7.7s %-5.5s %-5.5s ",
			"packets", "errs", "packets", "errs", "colls");
	putchar('\n');
	fflush(stdout);
	line = 0;
loop:
	sum->ift_ip = 0;
	sum->ift_ie = 0;
	sum->ift_op = 0;
	sum->ift_oe = 0;
	sum->ift_co = 0;
	for (off = firstifnet, ip = iftot; off && ip < lastif; ip++) {
		klseek(kmem, off, 0);
		read(kmem, &ifnet, sizeof ifnet);
		if (ip == interesting)
#ifndef pdp11
			printf("%-7d %-5d %-7d %-5d %-5d ",
#else pdp11
			printf("%-7u %-5u %-7u %-5u %-5u ",
#endif pdp11
				ifnet.if_ipackets - ip->ift_ip,
				ifnet.if_ierrors - ip->ift_ie,
				ifnet.if_opackets - ip->ift_op,
				ifnet.if_oerrors - ip->ift_oe,
				ifnet.if_collisions - ip->ift_co);
		ip->ift_ip = ifnet.if_ipackets;
		ip->ift_ie = ifnet.if_ierrors;
		ip->ift_op = ifnet.if_opackets;
		ip->ift_oe = ifnet.if_oerrors;
		ip->ift_co = ifnet.if_collisions;
		sum->ift_ip += ip->ift_ip;
		sum->ift_ie += ip->ift_ie;
		sum->ift_op += ip->ift_op;
		sum->ift_oe += ip->ift_oe;
		sum->ift_co += ip->ift_co;
#ifndef	pdp11
		off = (off_t) ifnet.if_next;
#else	pdp11
		off = (caddr_t) ifnet.if_next;
#endif	pdp11
	}
	if (lastif - iftot > 0)
#ifndef pdp11
		printf("%-7d %-5d %-7d %-5d %-5d\n",
#else pdp11
		printf("%-7u %-5u %-7u %-5u %-5u\n",
#endif pdp11
			sum->ift_ip - total->ift_ip,
			sum->ift_ie - total->ift_ie,
			sum->ift_op - total->ift_op,
			sum->ift_oe - total->ift_oe,
			sum->ift_co - total->ift_co);
	*total = *sum;
	fflush(stdout);
	line++;
	if (interval)
		sleep(interval);
	if (line == 21)
		goto banner;
	goto loop;
	/*NOTREACHED*/
}
