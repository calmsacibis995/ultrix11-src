
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)getnetbyaddr.c	3.0	4/22/86
 * Based on getnetbyaddr.c	4.3	82/10/06
 */

#include <netdb.h>

struct netent *
getnetbyaddr(net, type)
	long net;
	register int type;
{
	register struct netent *p;

	setnetent(0);
	while (p = getnetent())
		if (p->n_addrtype == type && p->n_net == net)
			break;
	endnetent();
	return (p);
}
