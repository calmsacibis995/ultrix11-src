
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/include/COPYRIGHT" for applicable restrictions.  *
 **********************************************************************/

/*
 * SCCSID: @(#)if_n1.c	3.0	4/21/86
 */

#define	NN1	0 	/* Number of units */

#if	NN1 < 1
n1attach(unit, addr, vector)
int unit;
int *addr;
int vector;
{
	printf("n1: non-existant device\n");
}

n1int(unit)
int unit;
{
}

n1rint()
{
}

n1xint()
{
}
#endif	NN1
#if	NN1 > 0

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/mbuf.h>
#include <sys/buf.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/map.h>

#include <net/if.h>
#include <net/netisr.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/if_ether.h>

#define	NXMT	4	/* number of transmit buffers */
#define	NRCV	4	/* number of receive buffers (must be > 1) */
#define	NTOT	(NXMT + NRCV)


/*
 * The n1uba structures generalizes the ifuba structure
 * to an arbitrary number of receive and transmit buffers.
 */
struct	n1uba {
	short	ifu_hlen;		/* local net header length */
	struct	uba_regs *ifu_uba;	/* uba regs, in vm */
	struct	ifrw ifu_r[NRCV];	/* receive information */
	struct	ifrw ifu_w[NXMT];	/* transmit information */
};

/*
 * buba - Bastardized UniBus Address
 * These macros are used so that we can store unibus address
 * in an int.  They also allow us to get at the low and high
 * parts of a UNIBUS address much easier.  These are intended
 * only for click boundries, since we store the 16th & 17th bits
 * in bits 0 and 1.
 */
/* convert between a click address and a buba */
#define	ctobuba(x)	((((x)<<6)&~077)|(((x)>>10)&03))
#define	bubatoc(x)	((((x)&03)<<10)|(((x)&~077)>>6))
/* get the low and high parts from a buba */
#define	lobuba(x)	((x)&~077)
#define	hibuba(x)	((x)&03)


#define	MAPIT(x,y)	{mapseg5((x), ((btoc(ETHERMTU)-1)<<8)|RW);y=SEG5;}
/*
 * Ethernet software status per interface.
 *
 * Each interface is referenced by a network interface structure,
 * ds_if, which the routing code uses to locate the interface.
 * This structure contains the output queue for the interface, its address, ...
 * We also have, for each interface, a UBA interface structure, which
 * contains information about the UNIBUS resources held by the interface:
 * map registers, buffered data paths, etc.  Information is cached in this
 * structure for use by the if_uba.c routines in running the interface
 * efficiently.
 */
struct	n1_softc {
	struct	arpcom ds_ac;		/* Ethernet common part */
#define	ds_if	ds_ac.ac_if		/* network-visible interface */
#define	ds_addr	ds_ac.ac_enaddr		/* hardware Ethernet address */
	char	ds_flags;		/* Has the board be initialized? */
#define	DSF_RUNNING	2
	struct n1device *ds_hwaddr;	/* hardware address of the registers */
	int	ds_ubaddr;		/* map info for incore structs */
	struct	n1uba ds_n1uba;		/* unibus resource structure */
	struct	n1_ring ds_xrent[NXMT];	/* transmit ring entrys */
	struct	n1_ring ds_rrent[NRCV];	/* receive ring entrys */
	struct	n1_udbbuf ds_udbbuf;	/* UNIBUS data buffer */
	struct	n1_counters ds_counters;/* counter block */
	int	ds_xindex;		/* UNA index into transmit chain */
	int	ds_rindex;		/* UNA index into receive chain */
	int	ds_xfree;		/* index for next transmit buffer */
	int	ds_nxmit;		/* # of transmits in progress */
	long	ds_ztime;		/* time counters were last zeroed */
	u_short	ds_unrecog;		/* unrecognized frame destination */
} n1_softc[NN1];


extern struct protosw *iftype_to_proto(), *iffamily_to_proto();
int	n1attach(), n1int();
struct	uba_device ;
int	n1init(), n1output(), n1ioctl();
struct	mbuf *n1get();

static char N1name[] = { 'n', '1', '\0' };

/*
 * Interface exists: make available by filling in network interface
 * record.  System will initialize the interface when it is ready
 * to accept packets.  We get the ethernet address here.
 */
n1attach(unit, addr, vector)
int unit;
register struct n1device *addr;
int vector;
{
	register struct n1_softc *ds;
	register struct ifnet *ifp;
	struct sockaddr_in *sin;
	int csr0,i;
	int n1_buba2b;

	if (badaddr(addr)) {
		n1rr("non-existant address %o", unit, addr);
		return;
	}

	ds = &n1_softc[unit];
	ifp = &ds->ds_if;
	ds->ds_hwaddr = addr;
	ifp->if_unit = unit;
	ifp->if_name = N1name;
	ifp->if_mtu = ETHERMTU;
	ifp->if_flags |= IFF_BROADCAST|IFF_DYNPROTO;

	/* Reset the board */

	/* map any needed data onto the Unibus. */
	n1_buba2b = ub_alloc((long)INCORE_BASE(ds), INCORE_SIZE);
	ds->ds_ubaddr = ctobuba(n1_buba2b);
	/* get the ethernet address form the hardware, and save it */
	bcopy((caddr_t)&ds->ds_pcbb.pcbb2,(caddr_t)ds->ds_addr,
	    sizeof (ds->ds_addr));
	ifp->if_init = n1init;
	ifp->if_output = n1output;
	ifp->if_ioctl = n1ioctl;
	ifp->if_reset = 0;
	if_attach(ifp);
	/* allocate memory for doing DMA */
	ds->ds_n1uba.ifu_r[0].ifrw_click = malloc(coremap, NTOT*btoc(ETHERMTU));
	n1rr("allocated %d bytes for DMA", unit, ctob(NTOT*btoc(ETHERMTU)));
}

/*
 * Initialization of interface; clear recorded pending
 * operations, and reinitialize UNIBUS usage.
 */
n1init(unit)
	int unit;
{
	register struct n1_softc *ds = &n1_softc[unit];
	register struct n1device *addr;
	register struct ifrw *ifrw;
	register struct ifrw *ifxp;
	struct ifnet *ifp = &ds->ds_if;
	int s;
	struct n1_ring *rp;
	int csr0;

	/* not yet, if address still unknown */
	if (ifp->if_addrlist == (struct ifaddr *)0)
		return;

	if (ifp->if_flags & IFF_RUNNING)
		return;

	if (n1_ubainit(&ds->ds_n1uba, sizeof (struct ether_header)) == 0) { 
		n1rr("can't initialize", unit);
		ds->ds_if.if_flags &= ~IFF_UP;
		return;
	}
	addr = ds->ds_hwaddr;

	/* give the transmit and receive ring header addresses */
	/* to the device */
	/* make sure you give the device unibus addresses if it */
	/* is unibus device */

	/* initialize the mode - enable hardware padding */

	/* let hardware do padding - set MTCH bit on broadcast */

	/* set up the receive and transmit ring entries */
	ifxp = &ds->ds_n1uba.ifu_w[0];
	for (rp = &ds->ds_xrent[0]; rp < &ds->ds_xrent[NXMT]; rp++) {
		rp->r_segbl = lobuba(ifxp->ifrw_buba);
		rp->r_segbh = hibuba(ifxp->ifrw_buba);
		rp->r_flags = 0;
		ifxp++;
	}
	ifrw = &ds->ds_n1uba.ifu_r[0];
	for (rp = &ds->ds_rrent[0]; rp < &ds->ds_rrent[NRCV]; rp++) {
		rp->r_slen = sizeof (struct n1_buf);
		rp->r_segbl = lobuba(ifrw->ifrw_buba);
		rp->r_segbh = hibuba(ifrw->ifrw_buba);
		rp->r_flags = RFLG_OWN;		/* hang receive */
		ifrw++;
	}

	/* start up the board (rah rah) */
	s = splimp();
	ds->ds_rindex = ds->ds_xindex = ds->ds_xfree = 0;
	ds->ds_if.if_flags |= IFF_UP|IFF_RUNNING;
	n1start(unit);				/* queue output packets */
	/* Turn on interrupts */
	ds->ds_flags |= DSF_RUNNING;
	ds->ds_ztime = time;
	splx(s);
}

/*
 * Setup output on interface.
 * Get another datagram to send off of the interface queue,
 * and map it to the interface before starting the output.
 */
static
n1start(unit)
	int unit;
{
        int len;
	register struct n1_softc *ds = &n1_softc[unit];
	struct n1device *addr = ds->ds_hwaddr;
	register struct n1_ring *rp;
	struct mbuf *m;
	register int nxmit;

	for (nxmit = ds->ds_nxmit; nxmit < NXMT; nxmit++) {
		IF_DEQUEUE(&ds->ds_if.if_snd, m);
		if (m == 0)
			break;
		rp = &ds->ds_xrent[ds->ds_xfree];
		if (rp->r_flags & XFLG_OWN)
			panic("n1una xmit in progress");
		len = n1put(&ds->ds_n1uba, ds->ds_xfree, m);
		rp->r_slen = len;
		rp->r_tdrerr = 0;
		rp->r_flags = XFLG_STP|XFLG_ENP|XFLG_OWN;

		ds->ds_xfree++;
		if (ds->ds_xfree == NXMT)
			ds->ds_xfree = 0;
	}
	if (ds->ds_nxmit != nxmit) {
		ds->ds_nxmit = nxmit;
		if (ds->ds_flags & DSF_RUNNING)
			/* poke the device */
	}
	else if (ds->ds_nxmit == NXMT) {
		/*
		 * poke device if we have something to send and 
		 * transmit ring is full. 
		 */
		if (ds->ds_flags & DSF_RUNNING)
			/* poke the device */
	}
		
}

/*
 * Command done interrupt.
 */
n1int(unit)
	int unit;
{
	register struct n1_softc *ds = &n1_softc[unit];
	register struct n1_ring *rp;
	register struct ifrw *ifxp;
	short csr0;

	/* save flags right away - clear out interrupt bits */
	csr0 = ds->ds_hwaddr->pcsr0;
	ds->ds_hwaddr->pchigh = csr0 >> 8;

	/*
	 * if receive, put receive buffer on mbuf
	 * and hang the request again
	 */
	rp = &ds->ds_rrent[ds->ds_rindex];
	if ((rp->r_flags & RFLG_OWN) == 0)
		n1recv(ds, rp, unit);

	/*
	 * Poll transmit ring and check status.
	 * Be careful about loopback requests.
	 * Then free buffer space and check for
	 * more transmit requests.
	 */
	for ( ; ds->ds_nxmit > 0; ds->ds_nxmit--) {
		rp = &ds->ds_xrent[ds->ds_xindex];
		if (/* device hasn't sent packet yet */)
			break;
		ds->ds_if.if_opackets++;
		ifxp = &ds->ds_n1uba.ifu_w[ds->ds_xindex];
		/* check for unusual conditions */
		if ( /*Any errors*/ ) {
			/* do error statistics */
		}
		if ( /* got our own packet */ &&
			!(ds->ds_if.if_flags & IFF_LOOPBACK)) {
			/* received our own packet */
			ds->ds_if.if_ipackets++;
			n1read(ds, ifxp, rp->r_slen -
				sizeof (struct ether_header));
		}
		/* check if next transmit buffer also finished */
		ds->ds_xindex++;
		if (ds->ds_xindex == NXMT)
			ds->ds_xindex = 0;
	}
	n1start(unit);
}

/*
 * Ethernet interface receiver interface.
 * If input error just drop packet.
 * Otherwise purge input buffered data path and examine 
 * packet to determine type.  If can't determine length
 * from type, then have to drop packet.  Othewise decapsulate
 * packet based on type and pass to type specific higher-level
 * input routine.
 */
static
n1recv(ds, rp, unit)
register struct n1_softc *ds;
register struct n1_ring *rp;
	int unit;
{
	register int len;
	struct ether_header *eh;

	do {
		ds->ds_if.if_ipackets++;
		len = (rp->r_lenerr&RERR_MLEN) - sizeof (struct ether_header)
			- 4;	/* don't forget checksum! */
                if( ! (ds->ds_if.if_flags & IFF_LOOPBACK) ) {
		    /* Not loopback. Check for errors, else receive it */
		    if ( /*any errors */)
		      ds->ds_if.if_ierrors++;
		    else
			n1read(ds, &ds->ds_n1uba.ifu_r[ds->ds_rindex], len);
                } else {
			int	ret;
			segm	map5;

			saveseg5(map5);
			MAPIT(ds->ds_n1uba.ifu_r[ds->ds_rindex].ifrw_click, eh);
                        ret = bcmp(eh->ether_dhost, ds->ds_addr, 6);
			restorseg5(map5);
			if (ret == NULL)
                                n1read(ds, &ds->ds_n1uba.ifu_r[ds->ds_rindex], len);
                }

		/* hang the receive buffer again */
		rp->r_lenerr = 0;
		rp->r_flags = RFLG_OWN;

		/* check next receive buffer */
		if (++ds->ds_rindex == NRCV) {
			ds->ds_rindex = 0;
			rp = &ds->ds_rrent[0];
		} else
			rp = &ds->ds_rrent[ds->ds_rindex];
	} while ((rp->r_flags & RFLG_OWN) == 0);
}

/*
 * Pass a packet to the higher levels.
 * We deal with the trailer protocol here.
 */
static
n1read(ds, ifrw, len)
	register struct n1_softc *ds;
	struct ifrw *ifrw;
	int len;
{
	struct ether_header *eh;
    	struct mbuf *m;
	struct protosw *pr;
	int off, resid;
	struct ifqueue *inq;
	segm	map5;
	int	type;

	/*
	 * Deal with trailer protocol: if type is trailer
	 * get true type from first 16-bit word past data.
	 * Remember that type was trailer by setting off.
	 */
	saveseg5(map5);
	MAPIT(ifrw->ifrw_click, eh);
	type = eh->ether_type = ntohs((u_short)eh->ether_type);
	restorseg5(map5);
	if (type >= ETHERTYPE_TRAIL &&
		type < ETHERTYPE_TRAIL+ETHERTYPE_NTRAILER) {
		register u_short *p;
		off = (type - ETHERTYPE_TRAIL) * 512;
		if (off >= ETHERMTU)
			return;		/* sanity */
		mapseg5(ifrw->ifrw_click, ((btoc(ETHERMTU)-1)<<8)|RW)
		p = (u_short *)(SEG5 + sizeof(struct ether_header) + off);
		type = ntohs(*p++);
		resid = ntohs(*p);
		restorseg5(map5);
		if (off + resid > len)
			return;		/* sanity */
		len = off + resid;
	} else
		off = 0;
	if (len == 0)
		return;

	/*
	 * Pull packet off interface.  Off is nonzero if packet
	 * has trailing header; n1get will then force this header
	 * information to be at the front, but we still have to drop
	 * the type and length which are at the front of any trailer data.
	 */
	m = n1get(&ds->ds_n1uba, ifrw, len, off);
	if (m == 0)
		return;
	if (off) {
		m->m_off += 2 * sizeof (u_short);
		m->m_len -= 2 * sizeof (u_short);
	}
	switch (type) {

#ifdef INET
	case ETHERTYPE_IP:
		schednetisr(NETISR_IP);
		inq = &ipintrq;
		break;

	case ETHERTYPE_ARP:
		arpinput(&ds->ds_ac, m);
		return;
#endif
	default:
		/*
		 * see if other protocol families defined
		 * and call protocol specific routines.
		 * If no other protocols defined then dump message.
		 */
		if (pr=iftype_to_proto(type))  {
			if ((m = (struct mbuf *)(*pr->pr_ifinput)(m, &ds->ds_if, &inq)) == 0)
				return;
		} else {
			if (ds->ds_unrecog != 0xffff)
				ds->ds_unrecog++;
			m_freem(m);
			return;
		}
	}

	if (IF_QFULL(inq)) {
		IF_DROP(inq);
		m_freem(m);
		return;
	}
	IF_ENQUEUE(inq, m);
}

/*
 * Ethernet output routine.
 * Encapsulate a packet of type family for the local net.
 * Use trailer local net encapsulation if enough data in first
 * packet leaves a multiple of 512 bytes of data in remainder.
 */
n1output(ifp, m0, dst)
	struct ifnet *ifp;
	struct mbuf *m0;
	struct sockaddr *dst;
{
	int type, s, error;
	u_char edst[6];
	struct in_addr idst;
	struct protosw *pr;
	register struct n1_softc *ds = &n1_softc[ifp->if_unit];
	register struct mbuf *m = m0;
	register struct ether_header *eh;
	register int off;
	segm	map5;

	saveseg5(map5);
	switch (dst->sa_family) {

#ifdef INET
	case AF_INET:
		idst = ((struct sockaddr_in *)dst)->sin_addr;
		if (!arpresolve(&ds->ds_ac, m, &idst, edst))
			return (0);	/* if not yet resolved */
		/* need per host negotiation */
		if ((ifp->if_flags & IFF_NOTRAILERS) == 0) {
			off = ntohs((u_short)mtod(m, struct ip *)->ip_len) - m->m_len;
			if (off > 0 && (off & 0x1ff) == 0 &&
			    m->m_off >= MMINOFF + 2 * sizeof (u_short)) {
				u_short *p;
				type = ETHERTYPE_TRAIL + (off>>9);
				m->m_off -= 2 * sizeof (u_short);
				m->m_len += 2 * sizeof (u_short);
				p = mtod(m, u_short *);
				*p++ = HTONS(ETHERTYPE_IP);
				*p = htons((u_short)m->m_len);
				goto gottrailertype;
			}
		}
		type = ETHERTYPE_IP;
		off = 0;
		goto gottype;
#endif

	case AF_UNSPEC:
		eh = (struct ether_header *)dst->sa_data;
		bcopy((caddr_t)eh->ether_dhost, (caddr_t)edst, sizeof (edst));
		type = eh->ether_type;
		goto gottype;

	default:
		/*
		 * try to find other address families and call protocol
		 * specific output routine.
		 */
		
		if (pr=iffamily_to_proto(dst->sa_family)) {
			(*pr->pr_ifoutput)(ifp, m0, dst, &type, (char *)edst);
			goto gottype;
		}
		else {
			n1rr("can't handle af%d", ifp->if_unit, dst->sa_family);
			error = EAFNOSUPPORT;
			goto bad;
		}
	}

gottrailertype:
	/*
	 * Packet to be sent as trailer: move first packet
	 * (control information) to end of chain.
	 */
	while (m->m_next)
		m = m->m_next;
	m->m_next = m0;
	m = m0->m_next;
	m0->m_next = 0;
	m0 = m;

gottype:
	/*
	 * Add local net header.  If no space in first mbuf,
	 * allocate another.
	 */
	if (m->m_off > MMAXOFF ||
	    MMINOFF + sizeof (struct ether_header) > m->m_off) {
		m = m_get(M_DONTWAIT, MT_HEADER);
		if (m == 0) {
			error = ENOBUFS;
			goto bad;
		}
		m->m_next = m0;
		m->m_off = MMINOFF;
		m->m_len = sizeof (struct ether_header);
	} else {
		m->m_off -= sizeof (struct ether_header);
		m->m_len += sizeof (struct ether_header);
	}
	eh = mtod(m, struct ether_header *);
	eh->ether_type = htons((u_short)type);
	bcopy((caddr_t)edst, (caddr_t)eh->ether_dhost, sizeof (edst));
	bcopy((caddr_t)ds->ds_addr, (caddr_t)eh->ether_shost, sizeof (ds->ds_addr));
	restorseg5(map5);

	/*
	 * Queue message on interface, and start output if interface
	 * not yet active.
	 */
	s = splimp();
	if (IF_QFULL(&ifp->if_snd)) {
		IF_DROP(&ifp->if_snd);
		splx(s);
		m_freem(m);
		return (ENOBUFS);
	}
	IF_ENQUEUE(&ifp->if_snd, m);
	n1start(ifp->if_unit);
	splx(s);
	return (0);

bad:
	restorseg5(map5);
	m_freem(m0);
	return (error);
}

/*
 * Routines supporting UNIBUS network interfaces.
 */

/*
 * Init UNIBUS for interface.  We map the i/o area
 * onto the UNIBUS. We then split up the i/o area among
 * all the receive and transmit buffers.
 */

static
n1_ubainit(ifu, hlen)
	register struct n1uba *ifu;
	int hlen;
{
	register unsigned cp, ubaddr;
	int ncl;

	/*
	 * If the second read buffer has a non-zero
	 * address, it means we have already allocated core
	 * If the first read buffer has a zero entry,
	 * it means that n1attach couldn't malloc the
	 * needed core space for dma.
	 */
	if (ifu->ifu_r[1].ifrw_click)
		return(1);
	if (ifu->ifu_r[0].ifrw_click == 0)
		return(0);

	ncl = btoc(ETHERMTU);
	cp = ifu->ifu_r[0].ifrw_click;
	ifu->ifu_uba = ub_alloc(ctob((long)cp), ctob(NTOT*ncl));
	ubaddr = ifu->ifu_uba;
	ifu->ifu_hlen = hlen;
	{
		register struct ifrw *ifrw;

		for (ifrw = ifu->ifu_r; ifrw < &ifu->ifu_r[NRCV]; ifrw++) {
			ifrw->ifrw_click = cp;
			ifrw->ifrw_buba = ctobuba(ubaddr);
			cp += ncl;
			ubaddr += ncl;
		}
	}
	{
		register struct ifrw *ifxp;

		for (ifxp = ifu->ifu_w; ifxp < &ifu->ifu_w[NXMT]; ifxp++) {
			ifxp->ifrw_click = cp;
			ifxp->ifrw_buba = ctobuba(ubaddr);
			cp += ncl;
			ubaddr += ncl;
		}
	}
	return (1);
}

/*
 * Pull read data off a interface.
 * Len is length of data, with local net header stripped.
 * Off is non-zero if a trailer protocol was used, and
 * gives the offset of the trailer information.
 * We copy the trailer information and then all the normal
 * data into mbufs.
 */
static struct mbuf *
n1get(ifu, ifrw, totlen, off0)
	register struct n1uba *ifu;
	register struct ifrw *ifrw;
	int totlen, off0;
{
	struct mbuf *top, **mp, *m;
	int off = off0, len;
	register caddr_t cp = ifu->ifu_hlen;

	top = 0;
	mp = &top;
	while (totlen > 0) {
		MGET(m, M_DONTWAIT, MT_DATA);
		if (m == 0)
			goto bad;
		if (off) {
			len = totlen - off;
			cp = ifu->ifu_hlen + off;
		} else
			len = totlen;
		m->m_len = MIN(MLEN, len);
		m->m_off = MMINOFF;
		copyv(ifrw->ifrw_click, cp, m->m_click, m->m_off, m->m_len);
		cp += m->m_len;
		*mp = m;
		mp = &m->m_next;
		if (off) {
			/* sort of an ALGOL-W style for statement... */
			off += m->m_len;
			if (off == totlen) {
				cp = ifu->ifu_hlen;
				off = 0;
				totlen = off0;
			}
		} else
			totlen -= m->m_len;
	}
	return (top);
bad:
	m_freem(top);
	return (0);
}

/*
 * Map a chain of mbufs onto a network interface
 * in preparation for an i/o operation.
 * The argument chain of mbufs includes the local network
 * header which is copied to be in the mapped, aligned
 * i/o space.
 */
static
n1put(ifu, n, m)
	struct n1uba *ifu;
	int n;
	register struct mbuf *m;
{
	register struct mbuf *mp;
	register int cc;
	int click;

	click = ifu->ifu_w[n].ifrw_click;
	cc = 0;
	while (m) {
		copyv(m->m_click, m->m_off, click, cc, m->m_len);
		cc += m->m_len;
		MFREE(m, mp);
		m = mp;
	}

	return (cc);
}

/*
 * Process an ioctl request.
 */
n1ioctl(ifp, cmd, data)
	register struct ifnet *ifp;
	int cmd;
	caddr_t data;
{
	register struct n1_softc *ds = &n1_softc[ifp->if_unit];
	register struct n1device *addr = ds->ds_hwaddr;
	struct protosw *pr;
	struct sockaddr *sa;
	struct ifreq *ifr = (struct ifreq *)data;
	struct ifdevea *ifd = (struct ifdevea *)data;
	register struct ifaddr *ifa = (struct ifaddr *)data;
	int s = splimp(), error = 0;
	int csr0;

	switch (cmd) {
        case SIOCENABLBACK:
                if ( (error = n1loopback(ifp, ds, addr, 1)) != NULL )
                        break;
                ifp->if_flags |= IFF_LOOPBACK;
                break;
 
        case SIOCDISABLBACK:
                if ( (error = n1loopback(ifp, ds, addr, 0)) != NULL )
                        break;
                ifp->if_flags &= ~IFF_LOOPBACK;
                n1init(ifp->if_unit);
                break;
 
        case SIOCRPHYSADDR: 
                /*
                 * Get the default hardware address, and copy
		 * it to ifd->default_pa.
                 */
                bcopy(&ds->ds_pcbb.pcbb2, ifd->default_pa, 6);
                /*
                 * copy current physical address to ifd->current_pa
                 * for requestor.
                 */
                bcopy(ds->ds_addr, ifd->current_pa, 6);
                break;
 

	case SIOCSPHYSADDR: 
		/* Set the ethernet address */
		/*
		 * The new address is in
		 *	ifr->ifr_addr.sa_data
		 * and is 6 bytes long.
		 */

		/* If operation worked, save the new ethernet address */
		bcopy (ifr->ifr_addr.sa_data, (caddr_t)ds->ds_addr,
	    		sizeof (ds->ds_addr));
		n1init(ifp->if_unit);
		break;

	case SIOCRDCTRS:
	case SIOCRDZCTRS:
		{
		register struct ctrreq *ctr = (struct ctrreq *)data;

		/* Return statistic information */
		bzero(&ctr->ctr_ctrs, sizeof(struct estat));
		ctr->ctr_type = CTR_ETHER;
			/*
			 * Fill out the ctr structure
			 */
		/* zero time if need to */
		if (cmd == SIOCRDZCTRS) {
			ds->ds_ztime = time;
			ds->ds_unrecog = 0;
		}
		break;
		}

	case SIOCSIFADDR:
		ifp->if_flags |= IFF_UP;
		n1init(ifp->if_unit);
		switch (ifa->ifa_addr.sa_family) {
#ifdef	INET
		case AF_INET:
			((struct arpcom *)ifp)->ac_ipaddr =
				IA_SIN(ifa)->sin_addr;
			arpwhohas((struct arpcom *)ifp, &IA_SIN(ifa)->sin_addr);
			break;
#endif
		default:
			if (pr=iffamily_to_proto(ifa->ifa_addr.sa_family)) {
				error = (*pr->pr_ifioctl)(ifp, cmd, data);
			}
			break;
		}
		break;

	default:
		error = EINVAL;
	}
done:	splx(s);
	return (error);
}

/*
 * enable or disable internal loopback
 */
static
n1loopback(ifp, ds, addr, lb_ctl )
register struct ifnet *ifp;
register struct n1_softc *ds;
register struct n1device *addr;
u_char lb_ctl;
{
        /*
         * set or clear the loopback bit as a function of lb_ctl
         */
        if ( lb_ctl == 1 ) {
		/* put the hardware into loopback mode */
        } else {
		/* take the hardware out of loopback mode */.
        }
        if (/*couldn't set loopback mode*/) {
                return(EIO);
        }
        return(NULL);
}

static
n1rr_csr(msg, unit, csr0, csr1)
char *msg;
{
	n1rr("%s, csr0=%o csr1=%o", unit, msg, csr0, csr1);
}

static
n1rr(fmt, unit, a1, a2, a3, a4, a5, a6)
{
	printf("n1%d: ", unit);
	printf(fmt, a1, a2, a3, a4, a5, a6);
	putchar('\n');
}

#endif	NN1
