
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)if_de.c	3.1	10/27/87
 * based on "@(#)if_de.c	1.13      (ULTRIX-32)	6/20/85";
 */

/************************************************************************
 *			Modification History				*
 *									*
 *	2-Mar-86 - Fred Canter						*
 *		Install George Mathew's change to fix broken unibus	*
 *		address (buba) allocation code. The DEUNA failed on	*
 *		the 11/24 (just dumb luck it worked on split I/D CPUs).	*
 *									*
 *	15-Jul-85 - Dave Borman						*
 *		Frontal lobotomy for the PDP11.  There's a lot		*
 *		of VAX code that just doesn't make sense on a PDP.	*
 *									*
 * 	19-Jun-85 -- jaw						*
 *		VAX8200 name change.					*
 *									*
 *	13 Mar 85 - Jaw							*
 *		add support for VAX8200 and bua				*
 *									*
 *	LSC001 - Larry Cohen, 3/6/85: add internal loopback 		*
 *									*
 *	LSC002 - Larry cohen, 3/685: poke deuna when transmit ring	*
 *		   is full and more data to send. Prevents system from 	*
 *		   dropping off the network and hanging occasionally	*
 ***********************************************************************/

/*
 * DEC DEUNA interface
 *
 *	Lou Salkind
 *	New York University
 *
 * TODO:
 *	timeout routine (get statistics)
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/mbuf.h>
#include <sys/buf.h>
#include <sys/protosw.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/map.h>


#include <net/if_de.h>




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

struct de_softc de_softc[];  

#ifdef	DE_DEBUG
int dedebug = 0;
#endif	DE_DEBUG

#ifdef DOMULTI
u_char unused_multi[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
#endif DOMULTI
extern struct protosw *iftype_to_proto(), *iffamily_to_proto();
int	deattach(), deint();
struct	uba_device ;
u_short destd[] = { 0 };
int	deinit(),deoutput(),deioctl();
struct	mbuf *deget();

#define	DELUA_LOOP_LEN	(32 - sizeof(struct ether_header))

static char DEname[] = { 'd', 'e', '\0' };

unsigned	de_dmaa;
int	de_dmas;

/*
 * Interface exists: make available by filling in network interface
 * record.  System will initialize the interface when it is ready
 * to accept packets.  We get the ethernet address here.
 */
deattach(unit, addr, vector)
int unit;
register struct dedevice *addr;
int vector;
{
	register struct de_softc *ds;
	register struct ifnet *ifp;
	struct sockaddr_in *sin;
	int csr0,i;
	int de_buba2b;

	if (badaddr(addr)) {
		derr("non-existant address %o", unit, addr);
		return;
	}

	ds = &de_softc[unit];
	ifp = &ds->ds_if;
	ds->ds_hwaddr = addr;
	/*
	 * Is it a DEUNA or a DELULA? Save the device id.
	 */
	ds->ds_devid = (addr->pcsr1 & PCSR1_DEVID) >> 4;
	ifp->if_unit = unit;
	ifp->if_name = DEname;
	ifp->if_mtu = ETHERMTU;
	ifp->if_flags |= IFF_BROADCAST|IFF_DYNPROTO;

#ifdef DOMULTI
	/*
	 * Fill the multicast address table with unused entries (broadcast
	 * address) so that we can always give the full table to the device
	 * and we don't have to worry about gaps.
	 */
	for (i=0; i < NMULTI; i++)
		bcopy(unused_multi,&ds->ds_multicast[i],MULTISIZE);

#endif DOMULTI
	/*
	 * Reset the board and map the pcbb buffer onto the Unibus.
	 */
	addr->pcsr0 = PCSR0_RSET;
	while ((addr->pcsr0 & PCSR0_INTR) == 0)
		;
	csr0 = addr->pcsr0;
	addr->pchigh = csr0 >> 8;
	if ((csr0 & PCSR0_PCEI) || (csr0 & PCSR0_DNI)==0)
		derr_csr("reset failed", unit, csr0, addr->pcsr1);
	de_buba2b = ub_alloc((long)INCORE_BASE(ds), INCORE_SIZE);
	ds->ds_ubaddr = ctobuba(de_buba2b);
	addr->pcsr2 = lobuba(ds->ds_ubaddr);
	addr->pcsr3 = hibuba(ds->ds_ubaddr);
	addr->pclow = CMD_GETPCBB;
	while ((addr->pcsr0 & PCSR0_INTR) == 0)
		;
	csr0 = addr->pcsr0;
	addr->pchigh = csr0 >> 8;
	if ((csr0 & PCSR0_PCEI) || (csr0 & PCSR0_DNI)==0)
		derr_csr("pcbb failed", unit, csr0, addr->pcsr1);
	ds->ds_pcbb.pcbb0 = FC_RDPHYAD;
	addr->pclow = CMD_GETCMD;
	while ((addr->pcsr0 & PCSR0_INTR) == 0)
		;
	csr0 = addr->pcsr0;
	addr->pchigh = csr0 >> 8;
	if ((csr0 & PCSR0_PCEI) || (csr0 & PCSR0_DNI)==0)
		derr_csr("rdphyad failed", unit, csr0, addr->pcsr1);
#ifdef	DE_DEBUG
	if (dedebug)
		derr("addr=%d:%d:%d:%d:%d:%d", unit,
		    ds->ds_pcbb.pcbb2&0xff, (ds->ds_pcbb.pcbb2>>8)&0xff,
		    ds->ds_pcbb.pcbb4&0xff, (ds->ds_pcbb.pcbb4>>8)&0xff,
		    ds->ds_pcbb.pcbb6&0xff, (ds->ds_pcbb.pcbb6>>8)&0xff);
#endif	DE_DEBUG
	bcopy((caddr_t)&ds->ds_pcbb.pcbb2,(caddr_t)ds->ds_addr,
	    sizeof (ds->ds_addr));
	ifp->if_init = deinit;
	ifp->if_output = deoutput;
	ifp->if_ioctl = deioctl;
	ifp->if_reset = 0;
	if_attach(ifp);
	ds->ds_deuba.ifu_r[0].ifrw_click = malloc(coremap, NTOT*btoc(ETHERMTU));
	de_dmaa = ds->ds_deuba.ifu_r[0].ifrw_click;	/* for memstat */
	de_dmas = (NTOT*btoc(ETHERMTU));		/* for memstat */
	derr("allocated %d bytes for DMA", unit, ctob(NTOT*btoc(ETHERMTU)));
}

/*
 * Initialization of interface; clear recorded pending
 * operations, and reinitialize UNIBUS usage.
 */
deinit(unit)
	int unit;
{
	register struct de_softc *ds = &de_softc[unit];
	register struct dedevice *addr;
	register struct ifrw *ifrw;
	register struct ifrw *ifxp;
	struct ifnet *ifp = &ds->ds_if;
	int s;
	struct de_ring *rp;
	int csr0;

	/* not yet, if address still unknown */
	/* DECnet must set this somewhere to make device happy */
	if (ifp->if_addrlist == (struct ifaddr *)0)
		return;

	if (ifp->if_flags & IFF_RUNNING)
		return;

	if (de_ubainit(&ds->ds_deuba, sizeof (struct ether_header)) == 0) { 
		derr("can't initialize", unit);
		ds->ds_if.if_flags &= ~IFF_UP;
		return;
	}
	addr = ds->ds_hwaddr;

	/* set the transmit and receive ring header addresses */
	ds->ds_pcbb.pcbb0 = FC_WTRING;
	ds->ds_pcbb.pcbb2 = lobuba(ds->ds_ubaddr) + UDBBUF_OFFSET;
	ds->ds_pcbb.pcbb4 = hibuba(ds->ds_ubaddr);

	ds->ds_udbbuf.b_tdrbl = lobuba(ds->ds_ubaddr) + XRENT_OFFSET;
	ds->ds_udbbuf.b_tdrbh = hibuba(ds->ds_ubaddr);
	ds->ds_udbbuf.b_telen = sizeof (struct de_ring) / sizeof (short);
	ds->ds_udbbuf.b_trlen = NXMT;
	ds->ds_udbbuf.b_rdrbl = lobuba(ds->ds_ubaddr) + RRENT_OFFSET;
	ds->ds_udbbuf.b_rdrbh = hibuba(ds->ds_ubaddr);
	ds->ds_udbbuf.b_relen = sizeof (struct de_ring) / sizeof (short);
	ds->ds_udbbuf.b_rrlen = NRCV;

	addr->pclow = CMD_GETCMD;
	while ((addr->pcsr0 & PCSR0_INTR) == 0)
		;
	csr0 = addr->pcsr0;
	addr->pchigh = csr0 >> 8;
	if (csr0 & PCSR0_PCEI)
		derr_csr("wtring failed", unit, csr0, addr->pcsr1);

	/* initialize the mode - enable hardware padding */
	ds->ds_pcbb.pcbb0 = FC_WTMODE;
	/* let hardware do padding - set MTCH bit on broadcast */
	ds->ds_pcbb.pcbb2 = MOD_TPAD|MOD_HDX;
	addr->pclow = CMD_GETCMD;
	while ((addr->pcsr0 & PCSR0_INTR) == 0)
		;
	csr0 = addr->pcsr0;
	addr->pchigh = csr0 >> 8;
	if (csr0 & PCSR0_PCEI)
		derr_csr("wtmode failed", unit, csr0, addr->pcsr1);

	/* set up the receive and transmit ring entries */
	ifxp = &ds->ds_deuba.ifu_w[0];
	for (rp = &ds->ds_xrent[0]; rp < &ds->ds_xrent[NXMT]; rp++) {
		rp->r_segbl = lobuba(ifxp->ifrw_buba);
		rp->r_segbh = hibuba(ifxp->ifrw_buba);
		rp->r_flags = 0;
		ifxp++;
	}
	ifrw = &ds->ds_deuba.ifu_r[0];
	for (rp = &ds->ds_rrent[0]; rp < &ds->ds_rrent[NRCV]; rp++) {
		rp->r_slen = sizeof (struct de_buf);
		rp->r_segbl = lobuba(ifrw->ifrw_buba);
		rp->r_segbh = hibuba(ifrw->ifrw_buba);
		rp->r_flags = RFLG_OWN;		/* hang receive */
		ifrw++;
	}

	/* start up the board (rah rah) */
	s = splimp();
	ds->ds_rindex = ds->ds_xindex = ds->ds_xfree = 0;
	ds->ds_if.if_flags |= IFF_UP|IFF_RUNNING;
	destart(unit);				/* queue output packets */
	addr->pclow = PCSR0_INTE;		/* avoid interlock */
	addr->pclow = CMD_START | PCSR0_INTE;
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
destart(unit)
	int unit;
{
        int len;
	register struct de_softc *ds = &de_softc[unit];
	struct dedevice *addr = ds->ds_hwaddr;
	register struct de_ring *rp;
	struct mbuf *m;
	register int nxmit;

	for (nxmit = ds->ds_nxmit; nxmit < NXMT; nxmit++) {
		IF_DEQUEUE(&ds->ds_if.if_snd, m);
		if (m == 0)
			break;
		rp = &ds->ds_xrent[ds->ds_xfree];
		if (rp->r_flags & XFLG_OWN)
			panic("deuna xmit in progress");
		len = deput(&ds->ds_deuba, ds->ds_xfree, m);
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
			addr->pclow = PCSR0_INTE|CMD_PDMD;
	}
	else if (ds->ds_nxmit == NXMT) {  /* LSC002 */
		/*
		 * poke device if we have something to send and 
		 * transmit ring is full. 
		 */
#ifdef	DE_DEBUG
		if (dedebug) { 
			rp = &ds->ds_xrent[0];
			derr("did not xmt: %d, %d, %d, flag0=%x, flag1=%x",
				unit, ds->ds_xindex, ds->ds_nxmit, ds->ds_xfree,
				rp++->r_flags, rp->r_flags);
		}
#endif	DE_DEBUG
		if (ds->ds_flags & DSF_RUNNING)
			addr->pclow = PCSR0_INTE|CMD_PDMD;
	}
		
}

/*
 * Command done interrupt.
 */
deint(unit)
	int unit;
{
	register struct de_softc *ds = &de_softc[unit];
	register struct de_ring *rp;
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
		derecv(ds, rp, unit);

	/*
	 * Poll transmit ring and check status.
	 * Be careful about loopback requests.
	 * Then free buffer space and check for
	 * more transmit requests.
	 */
	for ( ; ds->ds_nxmit > 0; ds->ds_nxmit--) {
		rp = &ds->ds_xrent[ds->ds_xindex];
		if (rp->r_flags & XFLG_OWN)
			break;
		ds->ds_if.if_opackets++;
		ifxp = &ds->ds_deuba.ifu_w[ds->ds_xindex];
		/* check for unusual conditions */
		if (rp->r_flags & (XFLG_ERRS|XFLG_MTCH|XFLG_ONE|XFLG_MORE)) {
			if (rp->r_flags & XFLG_ERRS) {
				/* output error */
				ds->ds_if.if_oerrors++;
#ifdef	DE_DEBUG
				if (dedebug)
				    derr("oerror, flags=%o tdrerr=%o (len=%d)",
					unit, rp->r_flags, 
					rp->r_tdrerr, rp->r_slen);
#endif	DE_DEBUG
			} else {
				if (rp->r_flags & XFLG_ONE) {
					/* one collision */
					ds->ds_if.if_collisions++;
				} else if (rp->r_flags & XFLG_MORE) {
					/* more than one collision */
					ds->ds_if.if_collisions += 2; /*guess*/
				}
				if ((rp->r_flags & XFLG_MTCH) &&
					!(ds->ds_if.if_flags & IFF_LOOPBACK)) {
					/* received our own packet */
					ds->ds_if.if_ipackets++;
					deread(ds, ifxp, rp->r_slen -
						sizeof (struct ether_header));
				}
			}
		}
		/* check if next transmit buffer also finished */
		ds->ds_xindex++;
		if (ds->ds_xindex == NXMT)
			ds->ds_xindex = 0;
	}
	destart(unit);

	if (csr0 & PCSR0_RCBI) {
		ds->ds_if.if_ierrors++;
		ds->ds_hwaddr->pclow = PCSR0_INTE|CMD_PDMD;
	}
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
derecv(ds, rp, unit)
register struct de_softc *ds;
register struct de_ring *rp;
	int unit;
{
	register int len;
	struct ether_header *eh;

	do {
		ds->ds_if.if_ipackets++;
		len = (rp->r_lenerr&RERR_MLEN) - sizeof (struct ether_header)
			- 4;	/* don't forget checksum! */
                if( ! (ds->ds_if.if_flags & IFF_LOOPBACK) ) {
		/* check for errors */
		    if ((rp->r_flags & (RFLG_ERRS|RFLG_FRAM|RFLG_OFLO|RFLG_CRC))
		        || (rp->r_flags&(RFLG_STP|RFLG_ENP)) != (RFLG_STP|RFLG_ENP) 
		        || (rp->r_lenerr & (RERR_OVRN|RERR_BUFL|RERR_UBTO|RERR_NCHN)) ||
		        len < ETHERMIN || len > ETHERMTU) {
		  	    ds->ds_if.if_ierrors++;
#ifdef	DE_DEBUG
			    if (dedebug)
			     derr("ierror, flags=%o lenerr=%o (len=%d)",
				unit, rp->r_flags, rp->r_lenerr, len);
#endif	DE_DEBUG
		    } else
			deread(ds, &ds->ds_deuba.ifu_r[ds->ds_rindex], len);
                } else {
			int	ret;
			segm	map5;

			saveseg5(map5);
			MAPIT(ds->ds_deuba.ifu_r[ds->ds_rindex].ifrw_click, eh);
                        ret = bcmp(eh->ether_dhost, ds->ds_addr, 6);
			restorseg5(map5);
			if (ret == NULL)
                                deread(ds, &ds->ds_deuba.ifu_r[ds->ds_rindex], len);
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
deread(ds, ifrw, len)
	register struct de_softc *ds;
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
	 * has trailing header; deget will then force this header
	 * information to be at the front, but we still have to drop
	 * the type and length which are at the front of any trailer data.
	 */
	m = deget(&ds->ds_deuba, ifrw, len, off);
	if (m == 0)
		return;
	if (off) {
		m->m_off += 2 * sizeof (u_short);
		m->m_len -= 2 * sizeof (u_short);
	}
	switch (type) {

#ifdef INET
	case ETHERTYPE_IP:
#ifdef vax
		if (nINET==0) {
			m_freem(m);
			return;
		}
#endif vax
		schednetisr(NETISR_IP);
		inq = &ipintrq;
		break;

	case ETHERTYPE_ARP:
#ifdef vax
		if (nETHER==0) {
			m_freem(m);
			return;
		}
#endif vax
		arpinput(&ds->ds_ac, m);
		return;
#endif
	default:
		/*
		 * see if other protocol families defined
		 * and call protocol specific routines.
		 * If no other protocols defined then dump message.
		 */
#ifdef vax
		if (pr=iftype_to_proto(eh->ether_type))  {
			if ((m = (struct mbuf *)(*pr->pr_ifinput)(m, &ds->ds_if, &inq, eh)) == 0)
#else vax
		if (pr=iftype_to_proto(type))  {
			if ((m = (struct mbuf *)(*pr->pr_ifinput)(m, &ds->ds_if, &inq)) == 0)
#endif vax
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
deoutput(ifp, m0, dst)
	struct ifnet *ifp;
	struct mbuf *m0;
	struct sockaddr *dst;
{
	int type, s, error;
	u_char edst[6];
	struct in_addr idst;
	struct protosw *pr;
	register struct de_softc *ds = &de_softc[ifp->if_unit];
	register struct mbuf *m = m0;
	register struct ether_header *eh;
	register int off;
	segm	map5;

	saveseg5(map5);
	switch (dst->sa_family) {

#ifdef INET
	case AF_INET:
#ifdef vax
		if (nINET == 0) {
			derr("can't handle af%d", ifp->if_unit, dst->sa_family);
			error = EAFNOSUPPORT;
			goto bad;
		}
#endif vax
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
#ifdef	NS
	case AF_NS:
		type = ETHERTYPE_NS;
		bcopy((caddr_t)&(((struct sockaddr_ns *)dst)->sns_addr.x_host),
		(caddr_t)edst, sizeof (edst));
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
		 * If we are in loopback mode check the length and
		 * device type.  DELUA can only loopback frames of 36 bytes
		 * or less including crc.
		 */
		if ((ifp->if_flags & IFF_LOOPBACK) &&
		    (m->m_len > DELUA_LOOP_LEN) && (ds->ds_devid == DELUA))
			return(EINVAL);
		/*
		 * try to find other address families and call protocol
		 * specific output routine.
		 */
		
		if (pr=iffamily_to_proto(dst->sa_family)) {
			(*pr->pr_ifoutput)(ifp, m0, dst, &type, (char *)edst);
			goto gottype;
		}
		else {
			derr("can't handle af%d", ifp->if_unit, dst->sa_family);
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
	/* DEUNA fills in source address */
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
	destart(ifp->if_unit);
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
de_ubainit(ifu, hlen)
	register struct deuba *ifu;
	int hlen;
{
	register unsigned cp, ubaddr;
	int ncl;

	/*
	 * If the second read buffer has a non-zero
	 * address, it means we have already allocated core
	 * If the first read buffer has a zero entry,
	 * it means that deattach couldn't malloc the
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
deget(ifu, ifrw, totlen, off0)
	register struct deuba *ifu;
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
deput(ifu, n, m)
	struct deuba *ifu;
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
deioctl(ifp, cmd, data)
	register struct ifnet *ifp;
	int cmd;
	caddr_t data;
{
	register struct de_softc *ds = &de_softc[ifp->if_unit];
	register struct dedevice *addr = ds->ds_hwaddr;
	struct protosw *pr;
	struct sockaddr *sa;
	struct ifreq *ifr = (struct ifreq *)data;
	struct ifdevea *ifd = (struct ifdevea *)data;
	register struct ifaddr *ifa = (struct ifaddr *)data;
	int s = splimp(), error = 0;
	int csr0;

	switch (cmd) {

/*LSC001*/
        case SIOCENABLBACK:
                derr("internal loopback enable requested", ifp->if_unit);
                if ( (error = deloopback(ifp, ds, addr, 1)) != NULL )
                        break;
                ifp->if_flags |= IFF_LOOPBACK;
                break;
 
        case SIOCDISABLBACK:
                derr("internal loopback disable requested", ifp->if_unit);
                if ( (error = deloopback(ifp, ds, addr, 0)) != NULL )
                        break;
                ifp->if_flags &= ~IFF_LOOPBACK;
                deinit(ifp->if_unit);
                break;
 
        case SIOCRPHYSADDR: 
                /*
                 * read default hardware address.
                 */
                ds->ds_pcbb.pcbb0 = FC_RDDEFAULT;
                addr->pclow = CMD_GETCMD|((ds->ds_flags & DSF_RUNNING) ? PCSR0_INTE : 0);
                while ((addr->pcsr0 & PCSR0_DNI) == 0) 
                        ;
                csr0 = addr->pcsr0;
                addr->pchigh = csr0 >> 8;
                if (csr0 & PCSR0_PCEI) {
			derr_csr("read default hardware address failed",
                        	ifp->if_unit, csr0, addr->pcsr1);
                        error = EIO;
                        break;
                }
                /*
                 * copy current physical address and default hardware address
                 * for requestor.
                 */
                bcopy(&ds->ds_pcbb.pcbb2, ifd->default_pa, 6);
                bcopy(ds->ds_addr, ifd->current_pa, 6);
                break;
 

	case SIOCSPHYSADDR: 
		/* Set the DNA address as the de's physical address */
		ds->ds_pcbb.pcbb0 = FC_WTPHYAD;
		bcopy (ifr->ifr_addr.sa_data, &ds->ds_pcbb.pcbb2, 6);
		addr->pclow = CMD_GETCMD|((ds->ds_flags & DSF_RUNNING) ? PCSR0_INTE : 0);
		while ((addr->pcsr0 & PCSR0_DNI) == 0)
			;
		csr0 = addr->pcsr0;
		addr->pchigh = csr0 >> 8;
		if (csr0 & PCSR0_PCEI)
			derr_csr("wtphyad failed", ifp->if_unit, csr0,
								addr->pcsr1);
		bcopy((caddr_t)&ds->ds_pcbb.pcbb2,(caddr_t)ds->ds_addr,
	    		sizeof (ds->ds_addr));
		deinit(ifp->if_unit);
		break;

#ifdef DOMULTI
	case SIOCDELMULTI: 
	case SIOCADDMULTI: 
		{
		int i,j = -1,incaddr = ds->ds_ubaddr + MULTI_OFFSET;

		if (cmd==SIOCDELMULTI) {
		   for (i = 0; i < NMULTI; i++)
		       if (bcmp(&ds->ds_multicast[i],ifr->ifr_addr.sa_data,MULTISIZE) == 0) {
			    if (--ds->ds_muse[i] == 0)
				bcopy(unused_multi,&ds->ds_multicast[i],MULTISIZE);
		       }
		} else {
		    for (i = 0; i < NMULTI; i++) {
			if (bcmp(&ds->ds_multicast[i],ifr->ifr_addr.sa_data,MULTISIZE) == 0) {
			    ds->ds_muse[i]++;
			    goto done;
			}
		        if (bcmp(&ds->ds_multicast[i],unused_multi,MULTISIZE) == 0)
			    j = i;
		    }
		    if (j == -1) {
			derr("mtmulti failed, multicast list full: %d",
				ui->ui_unit, NMULTI);
			error = ENOBUFS;
			goto done;
		    }
		    bcopy(ifr->ifr_addr.sa_data, &ds->ds_multicast[j], MULTISIZE);
		    ds->ds_muse[j]++;
		}

		ds->ds_pcbb.pcbb0 = FC_WTMULTI;
		ds->ds_pcbb.pcbb2 = incaddr & 0xffff;
		ds->ds_pcbb.pcbb4 = (NMULTI << 8) | ((incaddr >> 16) & 03);
		addr->pclow = CMD_GETCMD|((ds->ds_flags & DSF_RUNNING) ? PCSR0_INTE : 0);
		while ((addr->pcsr0 & PCSR0_DNI) == 0)
		;
		csr0 = addr->pcsr0;
		addr->pchigh = csr0 >> 8;
		if (csr0 & PCSR0_PCEI)
			derr_csr("wtmulti failed", ui->ui_unit, csr0,
								addr->pcsr1);
		break;

		}

#endif DOMULTI
	case SIOCRDCTRS:
	case SIOCRDZCTRS:
		{
		register struct ctrreq *ctr = (struct ctrreq *)data;

		ds->ds_pcbb.pcbb0 = cmd == SIOCRDCTRS ? FC_RDCNTS : FC_RCCNTS;
		ds->ds_pcbb.pcbb2 = lobuba(ds->ds_ubaddr) + COUNTER_OFFSET;
		ds->ds_pcbb.pcbb4 = hibuba(ds->ds_ubaddr);
		ds->ds_pcbb.pcbb6 = sizeof(struct de_counters);
		addr->pclow = CMD_GETCMD|((ds->ds_flags & DSF_RUNNING) ? PCSR0_INTE : 0);
		while ((addr->pcsr0 & PCSR0_DNI) == 0)
			;
		csr0 = addr->pcsr0;
		addr->pchigh = csr0 >> 8;
		if (csr0 & PCSR0_PCEI) {
			derr_csr("rdcnts failed", ifp->if_unit, csr0,
								addr->pcsr1);
			error = ENOBUFS;
			break;
		}
		bzero(&ctr->ctr_ctrs, sizeof(struct estat));
		ctr->ctr_type = CTR_ETHER;
		ctr->ctr_ether.est_seconds = (time - ds->ds_ztime) > 0xfffe ? 0xffff : (time - ds->ds_ztime);
		ctr->ctr_ether.est_bytercvd = *(int *)ds->ds_counters.c_brcvd;
		ctr->ctr_ether.est_bytesent = *(int *)ds->ds_counters.c_bsent;
		ctr->ctr_ether.est_mbytercvd = *(int *)ds->ds_counters.c_mbrcvd;
		ctr->ctr_ether.est_blokrcvd = *(int *)ds->ds_counters.c_prcvd;
		ctr->ctr_ether.est_bloksent = *(int *)ds->ds_counters.c_psent;
		ctr->ctr_ether.est_mblokrcvd = *(int *)ds->ds_counters.c_mprcvd;
		ctr->ctr_ether.est_deferred = *(int *)ds->ds_counters.c_defer;
		ctr->ctr_ether.est_single = *(int *)ds->ds_counters.c_single;
		ctr->ctr_ether.est_multiple = *(int *)ds->ds_counters.c_multiple;
		ctr->ctr_ether.est_sendfail = ds->ds_counters.c_snderr;
		ctr->ctr_ether.est_sendfail_bm = ds->ds_counters.c_sbm & 0xff;
		ctr->ctr_ether.est_collis = ds->ds_counters.c_collis;
		ctr->ctr_ether.est_recvfail = ds->ds_counters.c_rcverr;
		ctr->ctr_ether.est_recvfail_bm = ds->ds_counters.c_rbm & 0xff;
		ctr->ctr_ether.est_unrecog = ds->ds_unrecog;
		ctr->ctr_ether.est_sysbuf = ds->ds_counters.c_ibuferr;
		ctr->ctr_ether.est_userbuf = ds->ds_counters.c_lbuferr;
		if (cmd == SIOCRDZCTRS) {
			ds->ds_ztime = time;
			ds->ds_unrecog = 0;
		}
		break;
		}

	case SIOCSIFADDR:
		ifp->if_flags |= IFF_UP;
		deinit(ifp->if_unit);
		switch (ifa->ifa_addr.sa_family) {
#ifdef	INET
		case AF_INET:
			((struct arpcom *)ifp)->ac_ipaddr =
				IA_SIN(ifa)->sin_addr;
			arpwhohas((struct arpcom *)ifp, &IA_SIN(ifa)->sin_addr);
			break;
#endif
#ifdef	NS
		case AF_NS:
		    {
			register struct ns_addr *ina = &(IA_SNS(ifa)->sns_addr);

			if (ns_nullhost(*ina)) {
				ina->x_host = * (union ns_host *)
					(de_softc[ifp->if_unit].ds_addr);
			} else {
				de_setaddr(ina->x_host.c_host,ifp->if_unit);
			}
			break;
		    }
#endif
		default:
			if (pr=iffamily_to_proto(ifa->ifa_addr.sa_family)) {
				error = (*pr->pr_ifioctl)(ifp, cmd, data);
			}
			break;
		}
		break;

	default:
#ifdef	should_this_be_here
		if (pr=iffamily_to_proto(ifa->ifa_addr.sa_family))
			error = (*pr->pr_ifioctl)(ifp, cmd, data);
		else
#endif
		error = EINVAL;
	}
done:	splx(s);
	return (error);
}

/*
 * enable or disable internal loopback   LSC001
 */
static
deloopback(ifp, ds, addr, lb_ctl )
register struct ifnet *ifp;
register struct de_softc *ds;
register struct dedevice *addr;
u_char lb_ctl;
{
        int csr0;

        /*
         * read current mode register.
         */
        ds->ds_pcbb.pcbb0 = FC_RDMODE;
        addr->pclow = CMD_GETCMD|((ds->ds_flags & DSF_RUNNING) ? PCSR0_INTE : 0);
        while ((addr->pcsr0 & PCSR0_DNI) == 0) 
                ;
        csr0 = addr->pcsr0;
        addr->pchigh = csr0 >> 8;
        if (csr0 & PCSR0_PCEI) {
		derr_csr("read mode register failed",
                        ifp->if_unit, csr0, addr->pcsr1);
                return(EIO);
        }

        /*
         * set or clear the loopback bit as a function of lb_ctl and
         * return mode register to driver.
         */
        if ( lb_ctl == 1 ) {
                ds->ds_pcbb.pcbb2 |= MOD_LOOP;
		if (ds->ds_devid == DELUA)
			ds->ds_pcbb.pcbb2 |= MOD_INTL;
		else
                	ds->ds_pcbb.pcbb2 &= ~MOD_HDX;
        } else {
                ds->ds_pcbb.pcbb2 &= ~MOD_LOOP;
		if (ds->ds_devid == DELUA)
			ds->ds_pcbb.pcbb2 &= ~MOD_INTL;
		else
			ds->ds_pcbb.pcbb2 |= MOD_HDX;
        }
        ds->ds_pcbb.pcbb0 = FC_WTMODE;
        addr->pclow = CMD_GETCMD|((ds->ds_flags & DSF_RUNNING) ? PCSR0_INTE : 0);
        while ((addr->pcsr0 & PCSR0_DNI) == 0) 
                ;
        csr0 = addr->pcsr0;
        addr->pchigh = csr0 >> 8;
        if (csr0 & PCSR0_PCEI) {
		derr_csr("write mode register failed",
                        ifp->if_unit, csr0, addr->pcsr1);
                return(EIO);
        }

        return(NULL);
}

static
derr_csr(msg, unit, csr0, csr1)
char *msg;
{
	derr("%s, csr0=%o csr1=%o", unit, msg, csr0, csr1);
}

static
derr(fmt, unit, a1, a2, a3, a4, a5, a6)
{
	printf("de%d: ", unit);
	printf(fmt, a1, a2, a3, a4, a5, a6);
	putchar('\n');
}
