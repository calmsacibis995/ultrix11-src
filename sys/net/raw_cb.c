
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
 * SCCSID: @(#)raw_cb.c	3.0	4/21/86
 *	@(#)raw_cb.c	6.6 (Berkeley) 6/8/85
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/domain.h>
#include <sys/protosw.h>
#include <errno.h>

#include <net/if.h>
#include <net/route.h>
#include <net/raw_cb.h>
#include <netinet/in.h>
#ifdef PUP
#include <netpup/pup.h>
#endif

#ifdef	vax
#include "../vax/mtpr.h"
#endif	vax

/*
 * Routines to manage the raw protocol control blocks. 
 *
 * TODO:
 *	hash lookups by protocol family/protocol + address family
 *	take care of unique address problems per AF?
 *	redo address binding to allow wildcards
 */

/*
 * Allocate a control block and a nominal amount
 * of buffer space for the socket.
 */
raw_attach(so, proto)
	register struct socket *so;
	int proto;
{
#ifndef	pdp11
	struct mbuf *m;
#endif	pdp11
	register struct rawcb *rp;

#ifndef	pdp11
	m = m_getclr(M_DONTWAIT, MT_PCB);
	if (m == 0)
		return (ENOBUFS);
#else	pdp11
	MSGET(rp, struct rawcb, MT_PCB, 1, M_DONTWAIT);
	if (rp == NULL)
		return (ENOBUFS);
#endif	pdp11
	if (sbreserve(&so->so_snd, RAWSNDQ) == 0)
		goto bad;
	if (sbreserve(&so->so_rcv, RAWRCVQ) == 0)
		goto bad2;
#ifndef	pdp11
	rp = mtod(m, struct rawcb *);
#endif	pdp11
	rp->rcb_socket = so;
	so->so_pcb = (caddr_t)rp;
	rp->rcb_pcb = 0;
	rp->rcb_proto.sp_family = so->so_proto->pr_domain->dom_family;
	rp->rcb_proto.sp_protocol = proto;
	insque(rp, &rawcb);
	return (0);
bad2:
	sbrelease(&so->so_snd);
bad:
#ifndef	pdp11
	(void) m_free(m);
#else	pdp11
	MSFREE(rp);
#endif	pdp11
	return (ENOBUFS);
}

/*
 * Detach the raw connection block and discard
 * socket resources.
 */
raw_detach(rp)
	register struct rawcb *rp;
{
	struct socket *so = rp->rcb_socket;

	so->so_pcb = 0;
	sofree(so);
	remque(rp);
#ifndef	pdp11
	m_freem(dtom(rp));
#else	pdp11
	MSFREE(rp);
#endif	pdp11
}

/*
 * Disconnect and possibly release resources.
 */
raw_disconnect(rp)
	struct rawcb *rp;
{

	rp->rcb_flags &= ~RAW_FADDR;
	if (rp->rcb_socket->so_state & SS_NOFDREF)
		raw_detach(rp);
}

raw_bind(so, nam)
	register struct socket *so;
	struct mbuf *nam;
{
#ifndef	pdp11
	struct sockaddr *addr = mtod(nam, struct sockaddr *);
#else	pdp11
	struct sockaddr *addr;
	int	error;
#endif	pdp11
	register struct rawcb *rp;

	if (ifnet == 0)
		return (EADDRNOTAVAIL);
#ifdef	pdp11
#define	return(x)	{error=x;goto bad;}
	MAPSAVE();
	addr = mtod(nam, struct sockaddr *);
#endif	pdp11
/* BEGIN DUBIOUS */
	/*
	 * Should we verify address not already in use?
	 * Some say yes, others no.
	 */
	switch (addr->sa_family) {

#ifdef INET
	case AF_IMPLINK:
	case AF_INET: {
		if (((struct sockaddr_in *)addr)->sin_addr.s_addr &&
		    ifa_ifwithaddr(addr) == 0)
			return (EADDRNOTAVAIL);
		break;
	}
#endif

#ifdef PUP
	/*
	 * Curious, we convert PUP address format to internet
	 * to allow us to verify we're asking for an Ethernet
	 * interface.  This is wrong, but things are heavily
	 * oriented towards the internet addressing scheme, and
	 * converting internet to PUP would be very expensive.
	 */
	case AF_PUP: {
		struct sockaddr_pup *spup = (struct sockaddr_pup *)addr;
		struct sockaddr_in inpup;

		bzero((caddr_t)&inpup, (unsigned)sizeof(inpup));
		inpup.sin_family = AF_INET;
		inpup.sin_addr = in_makeaddr(spup->spup_net, spup->spup_host);
		if (inpup.sin_addr.s_addr &&
		    ifa_ifwithaddr((struct sockaddr *)&inpup) == 0)
			return (EADDRNOTAVAIL);
		break;
	}
#endif

	default:
		return (EAFNOSUPPORT);
	}
/* END DUBIOUS */
	rp = sotorawcb(so);
	bcopy((caddr_t)addr, (caddr_t)&rp->rcb_laddr, sizeof (*addr));
	rp->rcb_flags |= RAW_LADDR;
#ifndef	pdp11
	return (0);
#else	pdp11
#undef	return
	error = 0;
bad:
	MAPREST();
	return(error);
#endif	pdp11
}

/*
 * Associate a peer's address with a
 * raw connection block.
 */
raw_connaddr(rp, nam)
	struct rawcb *rp;
	struct mbuf *nam;
{
#ifndef	pdp11
	struct sockaddr *addr = mtod(nam, struct sockaddr *);
#else	pdp11
	struct sockaddr *addr;

	MAPSAVE();
	addr = mtod(nam, struct sockaddr *);
#endif	pdp11

	bcopy((caddr_t)addr, (caddr_t)&rp->rcb_faddr, sizeof(*addr));
	rp->rcb_flags |= RAW_FADDR;
#ifdef	pdp11
	MAPREST();
#endif	pdp11
}
