
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
 * Based on static char sccsid[] = "@(#)inet.c	5.1 (Berkeley) 6/4/85";
 */
static char *Sccsid = "@(#)play.c	3.0	(ULTRIX-11)	4/22/86";
#endif not lint

/*
 * Temporarily, copy these routines from the kernel,
 * as we need to know about subnets.
 */
#include "defs.h"

extern struct interface *ifnet;

/*
 * Formulate an Internet address from network + host.
 */
struct in_addr
#ifndef	pdp11
inet_makeaddr(net, host)
#else	pdp11
inet_mkaddr(net, host)
#endif	pdp11
	u_long net, host;
{
	register struct interface *ifp;
	register u_long mask;
	u_long addr;

	if (IN_CLASSA(net))
		mask = IN_CLASSA_HOST;
	else if (IN_CLASSB(net))
		mask = IN_CLASSB_HOST;
	else
		mask = IN_CLASSC_HOST;
	for (ifp = ifnet; ifp; ifp = ifp->int_next)
		if ((ifp->int_netmask & net) == ifp->int_net) {
			mask = ~ifp->int_subnetmask;
			break;
		}
	addr = net | (host & mask);
	addr = htonl(addr);
	return (*(struct in_addr *)&addr);
}

/*
 * Return the network number from an internet address.
 */
#ifdef	pdp11
long
#endif
inet_netof(in)
	struct in_addr in;
{
	register u_long i = ntohl(in.s_addr);
	register u_long net;
	register struct interface *ifp;

	if (IN_CLASSA(i))
		net = i & IN_CLASSA_NET;
	else if (IN_CLASSB(i))
		net = i & IN_CLASSB_NET;
	else
		net = i & IN_CLASSC_NET;

	/*
	 * Check whether network is a subnet;
	 * if so, return subnet number.
	 */
	for (ifp = ifnet; ifp; ifp = ifp->int_next)
		if ((ifp->int_netmask & net) == ifp->int_net)
			return (i & ifp->int_subnetmask);
	return (net);
}

/*
 * Return the host portion of an internet address.
 */
#ifdef	pdp11
long
#endif
inet_lnaof(in)
	struct in_addr in;
{
	register u_long i = ntohl(in.s_addr);
	register u_long net, host;
	register struct interface *ifp;

	if (IN_CLASSA(i)) {
		net = i & IN_CLASSA_NET;
		host = i & IN_CLASSA_HOST;
	} else if (IN_CLASSB(i)) {
		net = i & IN_CLASSB_NET;
		host = i & IN_CLASSB_HOST;
	} else {
		net = i & IN_CLASSC_NET;
		host = i & IN_CLASSC_HOST;
	}

	/*
	 * Check whether network is a subnet;
	 * if so, use the modified interpretation of `host'.
	 */
	for (ifp = ifnet; ifp; ifp = ifp->int_next)
		if ((ifp->int_netmask & net) == ifp->int_net)
			return (host &~ ifp->int_subnetmask);
	return (host);
}
