
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * SCCSID: @(#)route.c	3.0	4/21/86
 *	Based on: @(#)route.c	6.10 (Berkeley) 6/8/85
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/mbuf.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/ioctl.h>
#include <errno.h>

#include <net/if.h>
#include <netinet/in_systm.h>
#include <net/af.h>
#include <net/route.h>

int	rttrash;		/* routes not in table but not freed */
struct	sockaddr wildcard;	/* zero valued cookie for wildcard searches */
int	rthashsize = RTHASHSIZ;	/* for netstat, etc. */

/*
 * Packet routing routines.
 */
rtalloc(ro)
	register struct route *ro;
{
	register struct rtentry *rt;
#ifndef	pdp11
	register struct mbuf *m;
	register u_long hash;
#else	pdp11
	register unsigned int hash;
#endif	pdp11
	struct sockaddr *dst = &ro->ro_dst;
	int (*match)(), doinghost, s;
	struct afhash h;
	u_int af = dst->sa_family;
#ifndef	pdp11
	struct mbuf **table;
#else	pdp11
	struct rtentry **table;
#endif	pdp11

	if (ro->ro_rt && ro->ro_rt->rt_ifp && (ro->ro_rt->rt_flags & RTF_UP))
		return;				 /* XXX */
	if (af >= AF_MAX)
		return;
	(*afswitch[af].af_hash)(dst, &h);
	match = afswitch[af].af_netmatch;
	hash = h.afh_hosthash, table = rthost, doinghost = 1;
	s = splnet();
again:
#ifndef	pdp11
	for (m = table[RTHASHMOD(hash)]; m; m = m->m_next)
#else	pdp11
	for (rt = table[RTHASHMOD(hash)]; rt; rt = rt->rt_next)
#endif	pdp11
	{
#ifndef	pdp11
		rt = mtod(m, struct rtentry *);
#endif	pdp11
		if (rt->rt_hash != hash)
			continue;
		if ((rt->rt_flags & RTF_UP) == 0 ||
		    (rt->rt_ifp->if_flags & IFF_UP) == 0)
			continue;
		if (doinghost) {
			if (bcmp((caddr_t)&rt->rt_dst, (caddr_t)dst,
			    sizeof (*dst)))
				continue;
		} else {
			if (rt->rt_dst.sa_family != af ||
			    !(*match)(&rt->rt_dst, dst))
				continue;
		}
		rt->rt_refcnt++;
		splx(s);
		if (dst == &wildcard)
			rtstat.rts_wildcard++;
		ro->ro_rt = rt;
		return;
	}
	if (doinghost) {
		doinghost = 0;
		hash = h.afh_nethash, table = rtnet;
		goto again;
	}
	/*
	 * Check for wildcard gateway, by convention network 0.
	 */
	if (dst != &wildcard) {
		dst = &wildcard, hash = 0;
		goto again;
	}
	splx(s);
	rtstat.rts_unreach++;
}

rtfree(rt)
	register struct rtentry *rt;
{

	if (rt == 0)
		panic("rtfree");
	rt->rt_refcnt--;
	if (rt->rt_refcnt == 0 && (rt->rt_flags&RTF_UP) == 0) {
		rttrash--;
#ifndef	pdp11
		(void) m_free(dtom(rt));
#else	pdp11
		MSFREE(rt);
#endif	pdp11
	}
}

/*
 * Force a routing table entry to the specified
 * destination to go through the given gateway.
 * Normally called as a result of a routing redirect
 * message from the network layer.
 *
 * N.B.: must be called at splnet or higher
 *
 * Should notify all parties with a reference to
 * the route that it's changed (so, for instance,
 * current round trip time estimates could be flushed),
 * but we have no back pointers at the moment.
 */
rtredirect(dst, gateway, flags)
	struct sockaddr *dst, *gateway;
	int flags;
{
	struct route ro;
	register struct rtentry *rt;

	/* verify the gateway is directly reachable */
	if (ifa_ifwithnet(gateway) == 0) {
		rtstat.rts_badredirect++;
		return;
	}
	ro.ro_dst = *dst;
	ro.ro_rt = 0;
	rtalloc(&ro);
	rt = ro.ro_rt;
	/*
	 * Create a new entry if we just got back a wildcard entry
	 * or the the lookup failed.  This is necessary for hosts
	 * which use routing redirects generated by smart gateways
	 * to dynamically build the routing tables.
	 */
	if (rt &&
	    (*afswitch[dst->sa_family].af_netmatch)(&wildcard, &rt->rt_dst)) {
		rtfree(rt);
		rt = 0;
	}
	if (rt == 0) {
		rtinit(dst, gateway, (flags & RTF_HOST) | RTF_GATEWAY);
		rtstat.rts_dynamic++;
		return;
	}
	/*
	 * Don't listen to the redirect if it's
	 * for a route to an interface. 
	 */
	if (rt->rt_flags & RTF_GATEWAY) {
		if (((rt->rt_flags & RTF_HOST) == 0) && (flags & RTF_HOST)) {
			/*
			 * Changing from route to net => route to host.
			 * Create new route, rather than smashing route to net.
			 */
			rtinit(dst, gateway, flags);
		} else {
			/*
			 * Smash the current notion of the gateway to
			 * this destination.  This is probably not right,
			 * as it's conceivable a flurry of redirects could
			 * cause the gateway value to fluctuate wildly during
			 * dynamic routing reconfiguration.
			 */
			rt->rt_gateway = *gateway;
		}
		rtstat.rts_newgateway++;
	}
	rtfree(rt);
}

/*
 * Routing table ioctl interface.
 */
rtioctl(cmd, data)
	int cmd;
	caddr_t data;
{

#ifdef	pdp11
	struct rtentry foo;

	if (copyin(data, (caddr_t)&foo, sizeof(struct rtentry)) == -1)
		return(EFAULT);
#endif	pdp11
	if (cmd != SIOCADDRT && cmd != SIOCDELRT)
		return (EINVAL);
	if (!suser())
		return (u.u_error);
#ifndef	pdp11
	return (rtrequest(cmd, (struct rtentry *)data));
#else	pdp11
	return (rtrequest(cmd, &foo));
#endif	pdp11
}

/*
 * Carry out a request to change the routing table.  Called by
 * interfaces at boot time to make their ``local routes'' known,
 * for ioctl's, and as the result of routing redirects.
 */
static
rtrequest(req, entry)
	int req;
	register struct rtentry *entry;
{
#ifndef	pdp11
	register struct mbuf *m, **mprev;
#else	pdp11
	register struct rtentry *m, **mprev;
#endif	pdp11
	register struct rtentry *rt;
	struct afhash h;
	int s, error = 0, (*match)();
	u_int af;
	u_long hash;
	struct ifaddr *ifa;

	af = entry->rt_dst.sa_family;
	if (af >= AF_MAX)
		return (EAFNOSUPPORT);
	(*afswitch[af].af_hash)(&entry->rt_dst, &h);
	if (entry->rt_flags & RTF_HOST) {
		hash = h.afh_hosthash;
		mprev = &rthost[RTHASHMOD(hash)];
	} else {
		hash = h.afh_nethash;
		mprev = &rtnet[RTHASHMOD(hash)];
	}
	match = afswitch[af].af_netmatch;
	s = splimp();
#ifndef	pdp11
	for (; m = *mprev; mprev = &m->m_next)
#else	pdp11
	for (; m = *mprev; mprev = &m->rt_next)
#endif	pdp11
	{
#ifndef	pdp11
		rt = mtod(m, struct rtentry *);
#else	pdp11
		rt = m;
#endif	pdp11
		if (rt->rt_hash != hash)
			continue;
		if (entry->rt_flags & RTF_HOST) {
#define	equal(a1, a2) \
	(bcmp((caddr_t)(a1), (caddr_t)(a2), sizeof (struct sockaddr)) == 0)
			if (!equal(&rt->rt_dst, &entry->rt_dst))
				continue;
		} else {
			if (rt->rt_dst.sa_family != entry->rt_dst.sa_family ||
			    (*match)(&rt->rt_dst, &entry->rt_dst) == 0)
				continue;
		}
		if (equal(&rt->rt_gateway, &entry->rt_gateway))
			break;
	}
	switch (req) {

	case SIOCDELRT:
		if (m == 0) {
			error = ESRCH;
			goto bad;
		}
#ifndef	pdp11
		*mprev = m->m_next;
#else	pdp11
		*mprev = m->rt_next;
#endif	pdp11
		if (rt->rt_refcnt > 0) {
			rt->rt_flags &= ~RTF_UP;
			rttrash++;
#ifndef	pdp11
			m->m_next = 0;
#else	pdp11
			m->rt_next = 0;
#endif	pdp11
		} else
#ifndef	pdp11
			(void) m_free(m);
#else	pdp11
			MSFREE(m);
#endif	pdp11
		break;

	case SIOCADDRT:
		if (m) {
			error = EEXIST;
			goto bad;
		}
		ifa = ifa_ifwithaddr(&entry->rt_gateway);
		if (ifa == 0) {
			ifa = ifa_ifwithnet(&entry->rt_gateway);
			if (ifa == 0) {
				error = ENETUNREACH;
				goto bad;
			}
		}
#ifndef	pdp11
		m = m_get(M_DONTWAIT, MT_RTABLE);
		if (m == 0) {
			error = ENOBUFS;
			goto bad;
		}
		*mprev = m;
		m->m_off = MMINOFF;
		m->m_len = sizeof (struct rtentry);
		rt = mtod(m, struct rtentry *);
#else	pdp11
		MSGET(rt, struct rtentry, MT_RTABLE, 0, M_DONTWAIT);
		if (rt == NULL) {
			error = ENOBUFS;
			goto bad;
		}
		*mprev = rt;
		rt->rt_next = NULL;
#endif	pdp11
		rt->rt_hash = hash;
		rt->rt_dst = entry->rt_dst;
		rt->rt_gateway = entry->rt_gateway;
		rt->rt_flags =
		    RTF_UP | (entry->rt_flags & (RTF_HOST|RTF_GATEWAY));
		rt->rt_refcnt = 0;
		rt->rt_use = 0;
		rt->rt_ifp = ifa->ifa_ifp;
		break;
	}
bad:
	splx(s);
	return (error);
}

/*
 * Set up a routing table entry, normally
 * for an interface.
 */
rtinit(dst, gateway, flags)
	struct sockaddr *dst, *gateway;
	int flags;
{
	struct rtentry route;
	int cmd;

	if (flags == -1) {
		cmd = (int)SIOCDELRT;
		flags = 0;
	} else {
		cmd = (int)SIOCADDRT;
	}
	bzero((caddr_t)&route, sizeof (route));
	route.rt_dst = *dst;
	route.rt_gateway = *gateway;
	route.rt_flags = flags;
	(void) rtrequest(cmd, &route);
}
