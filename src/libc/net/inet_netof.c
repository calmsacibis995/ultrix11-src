
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)inet_netof.c	3.0	4/22/86
 * Based on	inet_netof.c	4.3	82/11/14
 */

#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>

/*
 * Return the network number from an internet
 * address; handles class a/b/c network #'s.
 */
u_long
inet_netof(in)
	struct in_addr in;
{
	u_long i = ntohl(in.s_addr);

	if (IN_CLASSA(i))
		return ((i >> IN_CLASSA_NSHIFT)&0xffL);
	else if (IN_CLASSB(i))
		return ((i >> IN_CLASSB_NSHIFT)&0xffffL);
	else
		return ((i >> IN_CLASSC_NSHIFT)&0xffffffL);
}
