
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	Based on @(#)defs.h	5.1 (Berkeley) 6/4/85
 *	@(#)defs.h	3.0	(ULTRIX-11)	4/22/86
 */

/*
 * Internal data structure definitions for
 * user routing process.  Based on Xerox NS
 * protocol specs with mods relevant to more
 * general addressing scheme.
 */
#include <sys/types.h>
#include <sys/socket.h>

#include <net/route.h>
#include <netinet/in.h>

#include <stdio.h>
#include <netdb.h>

#include "protocol.h"
#include "trace.h"
#include "interface.h"
#include "table.h"
#include "af.h"

/*
 * When we find any interfaces marked down we rescan the
 * kernel every CHECK_INTERVAL seconds to see if they've
 * come up.
 */
#define	CHECK_INTERVAL	(1*60)

#ifndef	pdp11
#define	LOOPBACKNET	0x7f000000	/* 127.0.0.0 */
#else	pdp11
#define	LOOPBACKNET	0x7f000000L	/* 127.0.0.0 */
#endif	pdp11
#define equal(a1, a2) \
	(bcmp((caddr_t)(a1), (caddr_t)(a2), sizeof (struct sockaddr)) == 0)
#define	min(a,b)	((a)>(b)?(b):(a))

struct	sockaddr_in addr;	/* address of daemon's socket */

int	s;			/* source and sink of all data */
int	kmem;
int	supplier;		/* process should supply updates */
int	install;		/* if 1 call kernel */
int	lookforinterfaces;	/* if 1 probe kernel for new up interfaces */
int	performnlist;		/* if 1 check if /vmunix has changed */
int	externalinterfaces;	/* # of remote and local interfaces */
int	timeval;		/* local idea of time */

char	packet[MAXPACKETSIZE+1];
struct	rip *msg;

char	**argv0;
struct	servent *sp;

extern	char *sys_errlist[];
extern	int errno;

struct	in_addr inet_mkaddr();
#ifndef	pdp11
int	inet_addr();
#endif	pdp11
char	*malloc();
int	exit();
int	sendmsg();
int	supply();
int	timer();
int	cleanup();
#ifdef	pdp11
#define	if_ifwithaddr		IFwaddr
#define	if_ifwithdstaddr	IFwdstaddr
#define	if_ifwithnet		IFwnet
#endif	pdp11
