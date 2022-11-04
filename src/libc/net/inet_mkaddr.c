
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)inet_mkaddr.c	3.0	4/22/86
 * Based on	inet_makeaddr.c	4.3	82/11/14
 */

#include <sys/types.h>
#include <netinet/in.h>

/*
 * Formulate an Internet address from network + host.  Used in
 * building addresses stored in the ifnet structure.
 */
#ifndef	pdp11
struct in_addr
inet_makeaddr(net, host)
	int net, host;
#else	pdp11
struct in_addr
inet_mkaddr(net, host)
	long net, host;
#endif	pdp11
{
	u_long addr, htonl();

#ifndef	pdp11
	if (net < 128)
		addr = (net << IN_CLASSA_NSHIFT) | host;
	else if (net < 65536)
		addr = (net << IN_CLASSB_NSHIFT) | host;
	else
		addr = (net << IN_CLASSC_NSHIFT) | host;
#else	pdp11
	if (net < 128L)
		addr = (net << (long)IN_CLASSA_NSHIFT) | host;
	else if (net < 65536L)
		addr = (net << (long)IN_CLASSB_NSHIFT) | host;
	else
		addr = (net << (long)IN_CLASSC_NSHIFT) | host;
#endif	pdp11
	addr = htonl(addr);
	return (*(struct in_addr *)&addr);
}
