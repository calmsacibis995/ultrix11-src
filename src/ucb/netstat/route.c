
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
 * Based on static char sccsid[] = "@(#)route.c	5.2 85/06/15";
 */
static char *Sccsid = "@(#)route.c	3.0	(ULTRIX-11)	4/22/86";
#endif

#include <sys/types.h>
#include <sys/socket.h>
#ifndef	pdp11
#include <sys/mbuf.h>
#endif	pdp11

#include <net/if.h>
#define	KERNEL		/* to get routehash and RTHASHSIZ */
#include <net/route.h>
#include <netinet/in.h>

#ifdef NS
#include <netns/ns.h>

#endif NS
#include <netdb.h>

extern	int kmem;
extern	int nflag;
extern	char *routename(), *netname(), *ns_print();
#ifdef	pdp11
long	ntohl();
#endif	pdp11

/*
 * Definitions for showing gateway flags.
 */
struct bits {
	short	b_mask;
	char	b_val;
} bits[] = {
	{ RTF_UP,	'U' },
	{ RTF_GATEWAY,	'G' },
	{ RTF_HOST,	'H' },
	{ 0 }
};

#ifdef	pdp11
#define	hashsizeaddr	hsszaddr
#endif	pdp11
/*
 * Print routing tables.
 */
routepr(hostaddr, netaddr, hashsizeaddr)
#ifndef	pdp11
	off_t hostaddr, netaddr, hashsizeaddr;
#else	pdp11
	caddr_t hostaddr, netaddr, hashsizeaddr;
#endif	pdp11
{
#ifndef	pdp11
	struct mbuf mb;
#else	pdp11
	struct rtentry mb;
#endif	pdp11
	register struct rtentry *rt;
#ifndef	pdp11
	register struct mbuf *m;
#else	pdp11
	register struct rtentry *m;
#endif	pdp11
	register struct bits *p;
	char name[16], *flags;
#ifndef	pdp11
	struct mbuf **routehash;
#else	pdp11
	struct rtentry **routehash;
#endif	pdp11
	struct ifnet ifnet;
	int hashsize;
	int i, doinghost = 1;

	if (hostaddr == 0) {
		printf("rthost: symbol not in namelist\n");
		return;
	}
	if (netaddr == 0) {
		printf("rtnet: symbol not in namelist\n");
		return;
	}
	if (hashsizeaddr == 0) {
		printf("rthashsize: symbol not in namelist\n");
		return;
	}
	klseek(kmem, hashsizeaddr, 0);
	read(kmem, &hashsize, sizeof (hashsize));
#ifndef	pdp11
	routehash = (struct mbuf **)malloc( hashsize*sizeof (struct mbuf *) );
#else	pdp11
	routehash = (struct rtentry **)malloc( hashsize*sizeof (struct rtentry *) );
#endif	pdp11
	klseek(kmem, hostaddr, 0);
#ifndef	pdp11
	read(kmem, routehash, hashsize*sizeof (struct mbuf *));
#else	pdp11
	read(kmem, routehash, hashsize*sizeof (struct rtentry *));
#endif	pdp11
	printf("Routing tables\n");
	printf("%-15.15s %-15.15s %-8.8s %-6.6s %-10.10s %s\n",
		"Destination", "Gateway",
		"Flags", "Refcnt", "Use", "Interface");
again:
	for (i = 0; i < hashsize; i++) {
		if (routehash[i] == 0)
			continue;
		m = routehash[i];
		while (m) {
			struct sockaddr_in *sin;
			struct sockaddr_ns *sns;
			long *l = (long *)&rt->rt_dst;

			klseek(kmem, m, 0);
			read(kmem, &mb, sizeof (mb));
#ifndef	pdp11
			rt = mtod(&mb, struct rtentry *);
#else	pdp11
			rt = &mb;
#endif	pdp11
			switch(rt->rt_dst.sa_family) {
		case AF_INET:
			sin = (struct sockaddr_in *)&rt->rt_dst;
			printf("%-15.15s ",
			    (sin->sin_addr.s_addr == 0) ? "default" :
			    (rt->rt_flags & RTF_HOST) ?
			    routename(sin->sin_addr) : netname(sin->sin_addr, 0));
			sin = (struct sockaddr_in *)&rt->rt_gateway;
			printf("%-15.15s ", routename(sin->sin_addr));
			break;
#ifdef	NS
		case AF_NS:
			printf("%-15s ",
			    ns_print((struct sockaddr_ns *)&rt->rt_dst));
			printf("%-15s ",
			    ns_print((struct sockaddr_ns *)&rt->rt_gateway));
			break;
#endif	NS
		default:
			printf("%8.8x %8.8x %8.8x %8.8x",*l, l[1], l[2], l[3]);
			l = (long *)&rt->rt_gateway;
			printf("%8.8x %8.8x %8.8x %8.8x",*l, l[1], l[2], l[3]);
			}
			for (flags = name, p = bits; p->b_mask; p++)
				if (p->b_mask & rt->rt_flags)
					*flags++ = p->b_val;
			*flags = '\0';
			printf("%-8.8s %-6d %-10d ", name,
				rt->rt_refcnt, rt->rt_use);
			if (rt->rt_ifp == 0) {
				putchar('\n');
#ifndef	pdp11
				m = mb.m_next;
#else	pdp11
				m = mb.rt_next;
#endif	pdp11
				continue;
			}
			klseek(kmem, rt->rt_ifp, 0);
			read(kmem, &ifnet, sizeof (ifnet));
			klseek(kmem, (int)ifnet.if_name, 0);
			read(kmem, name, 16);
			printf("%s%d\n", name, ifnet.if_unit);
#ifndef	pdp11
			m = mb.m_next;
#else	pdp11
			m = mb.rt_next;
#endif	pdp11
		}
	}
	if (doinghost) {
		klseek(kmem, netaddr, 0);
#ifndef	pdp11
		read(kmem, routehash, hashsize*sizeof (struct mbuf *));
#else	pdp11
		read(kmem, routehash, hashsize*sizeof (struct rtentry *));
#endif	pdp11
		doinghost = 0;
		goto again;
	}
	free(routehash);
}

char *
routename(in)
	struct in_addr in;
{
	char *cp = 0;
	static char line[50];
	struct hostent *hp;

	if (!nflag) {
		hp = gethostbyaddr(&in, sizeof (struct in_addr),
			AF_INET);
		if (hp)
			cp = hp->h_name;
	}
	if (cp)
		strcpy(line, cp);
	else {
#define C(x)	((int)((x) & 0xff))
		in.s_addr = ntohl(in.s_addr);
		sprintf(line, "%u.%u.%u.%u", C(in.s_addr >> 24),
			C(in.s_addr >> 16), C(in.s_addr >> 8), C(in.s_addr));
	}
	return (line);
}

/*
 * Return the name of the network whose address is given.
 * The address is assumed to be that of a net or subnet, not a host.
 */
char *
netname(in, mask)
	struct in_addr in;
	u_long mask;
{
	char *cp = 0;
	static char line[50];
	struct netent *np = 0;
	u_long net;
	register i;

	in.s_addr = ntohl(in.s_addr);
	if (!nflag && in.s_addr) {
		if (mask) {
			net = in.s_addr & mask;
			while ((mask & 1) == 0)
				mask >>= 1, net >>= 1;
			np = getnetbyaddr(net, AF_INET);
		}
		if (np == 0) {
			/*
			 * Try for subnet addresses.
			 */
#ifndef	pdp11
			for (i = 0; ((0xf<<i) & in.s_addr) == 0; i += 4)
				;
#else	pdp11
			for (i = 0; ((0xfL<<i) & in.s_addr) == 0; i += 4)
				;
#endif	pdp11
			for ( ; i; i -= 4)
#ifndef	pdp11
			    if (np = getnetbyaddr((unsigned)in.s_addr >> i,
				    AF_INET))
#else	pdp11
			    /*
			     * don't have unsigned longs, so this
			     * keeps it from sign extending.
			     */
			    if (np = getnetbyaddr(
				(in.s_addr >> i) & ~(0xffffffffL << (32-i)),
					AF_INET))
#endif	pdp11
					break;
		}
		if (np)
			cp = np->n_name;
	}
	if (cp)
		strcpy(line, cp);
#ifndef	pdp11
	else if ((in.s_addr & 0xffffff) == 0)
#else	pdp11
	else if ((in.s_addr & 0xffffffL) == 0)
#endif	pdp11
		sprintf(line, "%u", C(in.s_addr >> 24));
	else if ((in.s_addr & 0xffff) == 0)
		sprintf(line, "%u.%u", C(in.s_addr >> 24) , C(in.s_addr >> 16));
	else if ((in.s_addr & 0xff) == 0)
		sprintf(line, "%u.%u.%u", C(in.s_addr >> 24),
			C(in.s_addr >> 16), C(in.s_addr >> 8));
	else
		sprintf(line, "%u.%u.%u.%u", C(in.s_addr >> 24),
			C(in.s_addr >> 16), C(in.s_addr >> 8), C(in.s_addr));
	return (line);
}
/*
 * Print routing statistics
 */
rt_stats(off)
	off_t off;
{
	struct rtstat rtstat;

	if (off == 0) {
		printf("rtstat: symbol not in namelist\n");
		return;
	}
	klseek(kmem, off, 0);
	read(kmem, (char *)&rtstat, sizeof (rtstat));
	printf("routing:\n");
	printf("\t%d bad routing redirect%s\n",
		rtstat.rts_badredirect, plural(rtstat.rts_badredirect));
	printf("\t%d dynamically created route%s\n",
		rtstat.rts_dynamic, plural(rtstat.rts_dynamic));
	printf("\t%d new gateway%s due to redirects\n",
		rtstat.rts_newgateway, plural(rtstat.rts_newgateway));
	printf("\t%d destination%s found unreachable\n",
		rtstat.rts_unreach, plural(rtstat.rts_unreach));
	printf("\t%d use%s of a wildcard route\n",
		rtstat.rts_wildcard, plural(rtstat.rts_wildcard));
}
#ifdef NS
short ns_bh[] = {-1,-1,-1};

char *
ns_print(sns)
struct sockaddr_ns *sns;
{
	register struct ns_addr *dna = &sns->sns_addr;
	long net = ntohl(ns_netof(*dna));
	static char mybuf[50];
	register char *p = mybuf;
	short port = dna->x_port;

	sprintf(p,"%ld:", net);

	while(*p)p++; /* find end of string */

	if (strncmp(ns_bh,dna->x_host.c_host,6)==0)
		sprintf(p,"any");
	else
		sprintf(p,"%x.%x.%x.%x.%x.%x",
			    dna->x_host.c_host[0], dna->x_host.c_host[1],
			    dna->x_host.c_host[2], dna->x_host.c_host[3],
			    dna->x_host.c_host[4], dna->x_host.c_host[5]);
	if (port) {
	while(*p)p++; /* find end of string */
		printf(":%d",port);
	}
	return(mybuf);
}
char *
ns_phost(sns)
struct sockaddr_ns *sns;
{
	register struct ns_addr *dna = &sns->sns_addr;
	long net = ntohl(ns_netof(*dna));
	static char mybuf[50];
	register char *p = mybuf;
	if (strncmp(ns_bh,dna->x_host.c_host,6)==0)
		sprintf(p,"any");
	else
		sprintf(p,"%x,%x,%x",
			   dna->x_host.s_host[0], dna->x_host.s_host[1],
			    dna->x_host.s_host[2]);
	return(mybuf);
}
#endif BSD
