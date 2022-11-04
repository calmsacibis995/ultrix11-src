
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)ip_if.c	3.0	4/21/86
 *	Based on "@(#)ip_if.c	1.2	(ULTRIX-32)	11/15/84";
 */

/*
 * routines to handle protocol specific needs of interface drivers.
 */

#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <net/if.h>
#include <net/route.h>

ip_ifoutput(ifp, m, sockdst, type, linkdst)

	struct ifnet *ifp;  	/* output device */
	struct mbuf *m;        /* output msg without link level encapsulation */
	struct sockaddr *sockdst;   /* destination socket address */
	int *type;		/* place link level data type here */
	char *linkdst;		/* place link level destination address here */
{
	return(0);
}



ip_ifinput(m, ifp, inq)
	struct mbuf *m;
	struct ifnet *ifp;
	struct ifqueue *inq;
{
	return(0);
}



ip_ifioctl(ifp, cmd, data)
	struct ifnet *ifp;
	int cmd;
	caddr_t data;
{
	return(0);
}
