
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)gethbyaddr.c	3.0	4/22/86
 * Based on gethostbyaddr.c	4.3	82/10/06
 */

#include <netdb.h>

struct hostent *
gethostbyaddr(addr, len, type)
	char *addr;
	register int len, type;
{
	register struct hostent *p;

	sethostent(0);
	while (p = gethostent()) {
		if (p->h_addrtype != type || p->h_length != len)
			continue;
		if (bcmp(p->h_addr, addr, len) == 0)
			break;
	}
	endhostent();
	return (p);
}
