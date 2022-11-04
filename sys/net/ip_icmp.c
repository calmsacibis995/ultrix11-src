
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)ip_icmp.c	3.0	4/21/86
 *	Based on "@(#)ip_icmp.c	1.3	(ULTRIX-32)	11/13/84"
 */

/* ------------------------------------------------------------------------
 * Modification History: /sys/netinet/ip_icmp.c
 *
 * 24 Oct 84 -- jrs
 *	Added Berkeley changes to rearrange some case handling
 *	Derived from 4.2BSD, labeled:
 *		ip_icmp.c 6.5	84/03/13
 *
 * -----------------------------------------------------------------------
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/mbuf.h>
#include <sys/protosw.h>
#include <sys/socket.h>

#include <net/route.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp_var.h>

#ifdef ICMPPRINTFS
/*
 * ICMP routines: error generation, receive packet processing, and
 * routines to turnaround packets back to the originator, and
 * host table maintenance routines.
 */
int	icmpprintfs = 0;
#endif

/*
 * Generate an error packet of type error
 * in response to bad packet ip.
 */
icmp_error(oip, type, code)
	struct ip *oip;
	int type, code;
{
	register unsigned oiplen = oip->ip_hl << 2;
	register struct icmp *icp;
	struct mbuf *m;
#ifdef	pdp11
	struct mbuf *om = dtom(oip);
#endif	pdp11
	struct ip *nip;

#ifdef pdp11
	MAPSAVE();
#endif	pdp11
#ifdef ICMPPRINTFS
	if (icmpprintfs)
		printf("icmp_error(%x, %d, %d)\n", oip, type, code);
#endif
	icmpstat.icps_error++;
	/*
	 * Make sure that the old IP packet had 8 bytes of data to return;
	 * if not, don't bother.  Also don't EVER error if the old
	 * packet protocol was ICMP.
	 */
	if (oip->ip_len < 8) {
		icmpstat.icps_oldshort++;
		goto free;
	}
	if (oip->ip_p == IPPROTO_ICMP) {
		icmpstat.icps_oldicmp++;
		goto free;
	}

	/*
	 * First, formulate icmp message
	 */
	m = m_get(M_DONTWAIT, MT_HEADER);
	if (m == NULL)
		goto free;
	m->m_len = oiplen + 8 + ICMP_MINLEN;
	m->m_off = MMAXOFF - m->m_len;
	icp = mtod(m, struct icmp *);
	if ((u_int)type > ICMP_IREQREPLY)
		panic("icmp_error");
	icmpstat.icps_outhist[type]++;
	icp->icmp_type = type;
	icp->icmp_void = 0;
	if (type == ICMP_PARAMPROB) {
		icp->icmp_pptr = code;
		code = 0;
	}
	icp->icmp_code = code;
#ifndef	pdp11
	bcopy((caddr_t)oip, (caddr_t)&icp->icmp_ip, oiplen + 8);
#else	pdp11
	MBCOPY(om, 0, m, (&icp->icmp_ip - icp), oiplen + 8);
#endif	pdp11
	nip = &icp->icmp_ip;
	nip->ip_len += oiplen;
	nip->ip_len = htons((u_short)nip->ip_len);

	/*
	 * Now, copy old ip header in front of icmp
	 * message.  This allows us to reuse any source
	 * routing info present.
	 */
	m->m_off -= oiplen;
	nip = mtod(m, struct ip *);
#ifndef	pdp11
	bcopy((caddr_t)oip, (caddr_t)nip, oiplen);
#else	pdp11
	MBCOPY(om, 0, m, 0, oiplen);
#endif	pdp11
	nip->ip_len = m->m_len + oiplen;
	nip->ip_p = IPPROTO_ICMP;
	/* icmp_send adds ip header to m_off and m_len, so we deduct here */
	m->m_off += oiplen;
	icmp_reflect(nip);

free:
#ifdef	vax
	m_freem(dtom(oip));
#else	pdp11
	m_freem(om);
	MAPREST();
#endif
}

static struct sockproto icmproto = { AF_INET, IPPROTO_ICMP };
static struct sockaddr_in icmpsrc = { AF_INET };
static struct sockaddr_in icmpdst = { AF_INET };

/*
 * Process a received ICMP message.
 */
icmp_input(m)
	struct mbuf *m;
{
	register struct icmp *icp;
#ifndef	pdp11
	register struct ip *ip = mtod(m, struct ip *);
	int icmplen = ip->ip_len, hlen = ip->ip_hl << 2;
#else	pdp11
	register struct ip *ip;
	int icmplen, hlen;
#endif	pdp11
	register int i;
	int (*ctlfunc)(), code;
	extern u_char ip_protox[];

#ifdef	pdp11
#define	return	goto finishup
	MAPSAVE();
	ip = mtod(m, struct ip *);
	icmplen = ip->ip_len;
	hlen = ip->ip_hl << 2;
#endif	pdp11
	/*
	 * Locate icmp structure in mbuf, and check
	 * that not corrupted and of at least minimum length.
	 */
#ifdef ICMPPRINTFS
	if (icmpprintfs)
		printf("icmp_input from %x, len %d\n", ip->ip_src, icmplen);
#endif
	if (icmplen < ICMP_MINLEN) {
		icmpstat.icps_tooshort++;
		goto free;
	}
	/* THIS LENGTH CHECK STILL MISSES ANY IP OPTIONS IN ICMP_IP */
	i = MIN(icmplen, ICMP_ADVLENMIN + hlen);
 	if ((m->m_off > MMAXOFF || m->m_len < i) &&
 		(m = m_pullup(m, i)) == 0)  {
		icmpstat.icps_tooshort++;
		return;
	}
 	ip = mtod(m, struct ip *);
	m->m_len -= hlen;
	m->m_off += hlen;
	icp = mtod(m, struct icmp *);
	if (in_cksum(m, icmplen)) {
		icmpstat.icps_checksum++;
		goto free;
	}

#ifdef ICMPPRINTFS
	/*
	 * Message type specific processing.
	 */
	if (icmpprintfs)
		printf("icmp_input, type %d code %d\n", icp->icmp_type,
		    icp->icmp_code);
#endif
	if (icp->icmp_type > ICMP_IREQREPLY)
		goto free;
	icmpstat.icps_inhist[icp->icmp_type]++;
	code = icp->icmp_code;
	switch (icp->icmp_type) {

	case ICMP_UNREACH:
		if (code > 5)
			goto badcode;
		code += PRC_UNREACH_NET;
		goto deliver;

	case ICMP_TIMXCEED:
		if (code > 1)
			goto badcode;
		code += PRC_TIMXCEED_INTRANS;
		goto deliver;

	case ICMP_PARAMPROB:
		if (code)
			goto badcode;
		code = PRC_PARAMPROB;
		goto deliver;

	case ICMP_SOURCEQUENCH:
		if (code)
			goto badcode;
		code = PRC_QUENCH;
	deliver:
		/*
		 * Problem with datagram; advise higher level routines.
		 */
		icp->icmp_ip.ip_len = ntohs((u_short)icp->icmp_ip.ip_len);
		if (icmplen < ICMP_ADVLENMIN || icmplen < ICMP_ADVLEN(icp)) {
			icmpstat.icps_badlen++;
			goto free;
		}
#ifdef ICMPPRINTFS
		if (icmpprintfs)
			printf("deliver to protocol %d\n", icp->icmp_ip.ip_p);
#endif
#ifndef	pdp11
		if (ctlfunc = inetsw[ip_protox[icp->icmp_ip.ip_p]].pr_ctlinput){
			(*ctlfunc)(code, (caddr_t)icp);
#else	pdp11
		if ((i = icp->icmp_ip.ip_p) >= IPPROTO_MAX)
			i = IPPROTO_psudoRAW;
		if (ctlfunc = inetsw[ip_protox[i]].pr_ctlinput){
			struct icmp *icpcopy;
			icpcopy = (struct icmp *)m_sget(icmplen);
			if (icpcopy == 0)
				goto free;
			bcopy((caddr_t)icp, (caddr_t)icpcopy, icmplen);
			(*ctlfunc)(code, (caddr_t)icpcopy);
			m_sfree((caddr_t)icpcopy);
#endif	pdp11
		}
		goto free;

	badcode:
		icmpstat.icps_badcode++;
		goto free;

	case ICMP_ECHO:
		icp->icmp_type = ICMP_ECHOREPLY;
		goto reflect;

	case ICMP_TSTAMP:
		if (icmplen < ICMP_TSLEN) {
			icmpstat.icps_badlen++;
			goto free;
		}
		icp->icmp_type = ICMP_TSTAMPREPLY;
		icp->icmp_rtime = iptime();
		icp->icmp_ttime = icp->icmp_rtime;	/* bogus, do later! */
		goto reflect;
		
	case ICMP_IREQ:
#ifdef notdef
		/* fill in source address zero fields! */
		goto reflect;
#else
		goto free;		/* not yet implemented: ignore */
#endif

	case ICMP_REDIRECT:
		if (icmplen < ICMP_ADVLENMIN || icmplen < ICMP_ADVLEN(icp)) {
			icmpstat.icps_badlen++;
			goto free;
		}
		/*
		 * Short circuit routing redirects to force
		 * immediate change in the kernel's routing
		 * tables.  The message is also handed to anyone
		 * listening on a raw socket (e.g. the routing
		 * daemon for use in updating it's tables).
		 */
		icmpsrc.sin_addr = icp->icmp_ip.ip_dst;
		icmpdst.sin_addr = icp->icmp_gwaddr;
		rtredirect((struct sockaddr *)&icmpsrc,
		  (struct sockaddr *)&icmpdst,
		  (code == ICMP_REDIRECT_NET || code == ICMP_REDIRECT_TOSNET) ?
		   RTF_GATEWAY : (RTF_GATEWAY | RTF_HOST));
		/* FALL THROUGH */

	case ICMP_ECHOREPLY:
	case ICMP_TSTAMPREPLY:
	case ICMP_IREQREPLY:
		icmpsrc.sin_addr = ip->ip_src;
		icmpdst.sin_addr = ip->ip_dst;
		raw_input(dtom(icp), &icmproto, (struct sockaddr *)&icmpsrc,
		  (struct sockaddr *)&icmpdst);
		return;

	default:
		goto free;
	}
reflect:
	ip->ip_len += hlen;		/* since ip_input deducts this */
	icmpstat.icps_reflect++;
	icmp_reflect(ip);
	return;
free:
#ifdef	vax
	m_freem(dtom(ip));
#else	pdp11
	m_freem(m);
#undef	return
finishup:
	MAPREST();
#endif	pdp11
}

/*
 * Reflect the ip packet back to the source
 * TODO: rearrange ip source routing options.
 */
static
icmp_reflect(ip)
	struct ip *ip;
{
	struct in_addr t;

	t = ip->ip_dst;
	ip->ip_dst = ip->ip_src;
	ip->ip_src = t;
	icmp_send(ip);
}

/*
 * Send an icmp packet back to the ip level,
 * after supplying a checksum.
#ifdef	pdp11
 * No need to save ka5 here.  We are only called by icmp_reflect,
 * and icmp_reflect is only called by routines that alread save
 * and restore ka5.
#endif
 */
static
icmp_send(ip)
	struct ip *ip;
{
	register int hlen;
	register struct icmp *icp;
	register struct mbuf *m;

	m = dtom(ip);
	hlen = ip->ip_hl << 2;
	icp = mtod(m, struct icmp *);
	icp->icmp_cksum = 0;
	icp->icmp_cksum = in_cksum(m, ip->ip_len - hlen);
	m->m_off -= hlen;
	m->m_len += hlen;
#ifdef ICMPPRINTFS
	if (icmpprintfs)
		printf("icmp_send dst %x src %x\n", ip->ip_dst, ip->ip_src);
#endif
	(void) ip_output(m, (struct mbuf *)0, (struct route *)0, 0);
}

n_time
iptime()
{
	int s = spl6();
	u_long t;
#ifdef	pdp11
	extern	int hz;

	t = (time % (24*60*60)) * 1000 + lbolt * hz;
#else	pdp11
	t = (time.tv_sec % (24*60*60)) * 1000 + time.tv_usec / 1000;
#endif	pdp11
	splx(s);
	return (htonl(t));
}
