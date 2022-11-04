
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/


/*
 * SCCSID: @(#)if_to_proto.c	3.0	4/21/86
 *	Based on "@(#)if_to_proto.c	1.4	(ULTRIX-32)	1/4/85"
 */

#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <net/af.h>
#include <net/if_to_proto.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>

#ifdef	pdp11
#define	domain	if_domain
#endif	pdp11

#define	IFNULL \
	{ 0,	0,	0,	0 }

#define IFEND \
	{ -1,	-1,	-1,	0}

#ifndef BINARY
#ifdef INET
/*
 * need an entry for each device that has a data type field. 
 * ethernets and hyperchannels are examples
 */
#define	ETHER_IP  \
	{ ETHERPUP_IPTYPE,	AF_INET,	IPPROTO_IP,	0 }
#else
#define	ETHER_IP	IFNULL
#endif



#ifdef DECNET
#define	ETHER_DECNET  \
	{ ETHERPUP_DNTYPE,	AF_DECnet,	DNPROTO_NSP,	0 }
#else
#define	ETHER_DECNET	IFNULL
#endif

#ifdef LAT
#define ETHER_LAT \
	{ ETHERPUP_LATTYPE,	AF_LAT,		0,		0 }
#else
#define ETHER_LAT	IFNULL
#endif

/*
 * The DLI entry should be the last in the table since it provides a destination
 * for all messages which do not match earlier entries.
 */
#ifdef DLI
#define ETHER_DLI \
	{ -1,			AF_DLI,		0,		0 }
#else
#define ETHER_DLI	IFNULL
#endif

/* INET specific stuff is kept in drivers for now */
struct if_family if_family[] = {
	ETHER_DECNET,	ETHER_LAT,	ETHER_DLI,	IFEND
};

#else

extern struct if_family if_family[];

#endif

/*
 * -given the link level protocol type, find the corresponding protocol family. 
 * -return the switch table entry corresponding to the protocol family. 
 * -return 0 if protocol not found.
 * 
 */
struct protosw *
iftype_to_proto(type)
register int type;
{
	register struct if_family *i;
	for (i=if_family; i->domain != -1; i++)
		if (i->if_type == -1 || i->if_type == type)
			return(i->prswitch);
	return(0);
}


/*
 * -given the address family (domain), find the corresponding protocol family. 
 * -return the switch table entry corresponding to the protocol family. 
 * -return 0 if protocol not found.
 * 
 */
struct protosw *
iffamily_to_proto(family)
register int family;
{
	register struct if_family *i;
	for (i=if_family; i->domain != -1; i++)
		if (i->domain == family)
			return(i->prswitch);
	return(0);
}



if_to_proto_init()
{
	register struct if_family *i;
	for (i=if_family; i->domain != -1; i++)
		i->prswitch = pffindproto(i->domain, i->proto);
}
