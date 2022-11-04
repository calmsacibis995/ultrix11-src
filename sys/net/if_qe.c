/*
 * if_qe.c
 *
 * SCCSID: @(#)if_qe.c	3.1	12/31/87
 *	based on "@(#)if_qe.c	1.1	(decvax!rjl)";
 */

/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/include/COPYRIGHT" for applicable restrictions.  *
 **********************************************************************/
/* ---------------------------------------------------------------------
 * Modification History 
 *
 *  04 Oct. 84 -- jf
 *	Added support for new ioctls to add and delete multicast addresses
 *	and set the physical address.
 *	Add support for general protocols.
 *
 *  14 Aug. 84 -- rjl
 *	Integrated Shannon changes. (allow arp above 1024 and ? )
 *
 *  13 Feb. 84 -- rjl
 *
 *	Initial version of driver. derived from IL driver.
 * 
 * ---------------------------------------------------------------------
 */

/*
 * Digital Q-BUS to NI Adapter
 */
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/mbuf.h>
#include <sys/buf.h>
#include <sys/protosw.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <net/if_qe.h>


/*
 * Ethernet software status per interface.
 *
 * Each interface is referenced by a network interface structure,
 * is_if, which the routing code uses to locate the interface.
 * This structure contains the output queue for the interface, its address, ...
 */
/*
 * We allocate enough descriptors to do scatter/gather plus 1 to
 * construct a ring.
 *
 * The MicroVAX-I doesn't have an I/O map, therefore all addresses presented
 * to devices must be physical address. To keep track of mbufs in use the
 * softc for this device has to record the mbufs because  the data addresses
 * in the ring descriptors are physical addresses instead of virtual. With
 * an I/O map this will no longer be necessary and we'll be able to use
 * the mbuf macro's for allocation and deallocation.
 *
 * There must be enough descriptors in the receive ring to map at least 2
 * of the largest size datagrams so that a race condition doesn't occur in
 * the receiver. (~1600/252 bytes)
 *
 */
/*
 * This constant should really be 60 because the qna adds 4 bytes of crc.
 * However when set to 60 our packets are ignored by deuna's , 3coms are
 * okay ??????????????????????????????????????????
 */
#define MINDATA 64



struct qe_softc qe_softc[];

#define	QEADDR	0174440			/* alternate value is 0174460 */
#define	QEVECT	0400			/* arbitrary, for now... */
extern  int	nNQE;			/* initialised in dds.c */


extern struct protosw *iftype_to_proto(), *iffamily_to_proto();
extern long time;
int	qeattach();
struct mbuf *qeget();
#define	QEUNIT(x)	minor(x)
int	qewrun = 0;
int	qeinit(),qeoutput(),qeioctl(), qewatch();
int	qedebug = 0;
extern	hz;
/*
 * Watchdog timeout value, in clicks.  Three times this value
 * is the maximum time for us to decide that the deqna transmit
 * queue is wedged (could be as short as two times this value).
 */
#define	QETIMO	30
 
/*
 * Interface exists: make available by filling in network interface
 * record.  System will initialize the interface when it is ready
 * to accept packets.
 */
static char QEname[] = { 'q', 'e', '\0' };
qeattach(unit, addr, vector)
int unit;
register struct qedevice *addr;
int vector;
{
	register struct qe_softc *sc;
	register struct ifnet *ifp;
	register int i;

	if (badaddr(addr)) {
		printf("qe%d: non-existant address %o\n", unit, addr);
		return;
	}
	sc = &qe_softc[unit];
	ifp = &sc->is_if;

	ifp->if_unit = unit;
	ifp->if_name = QEname;
	ifp->if_mtu = ETHERMTU;
	ifp->if_flags |= IFF_BROADCAST | IFF_DYNPROTO;

	/*
	 * Read the address from the prom and save it.
	 */
	for( i=0 ; i<6 ; i++ ) 
		sc->is_addr[i] = addr->qe_sta_addr[i] & 0xff;  

	/*
	 * Save the vector for initialization at reset time.
	 */
	sc->qe_intvec = vector;
	sc->addr = addr;

	ifp->if_init = qeinit;
	ifp->if_output = qeoutput;
	ifp->if_ioctl = qeioctl;
	ifp->if_reset = 0;
	if_attach(ifp);
}

/*
 * Initialization of interface and allocation of mbufs for receive ring
 * buffers.
 */
qeinit(unit)
	int unit;
{
	register struct qe_softc *sc = &qe_softc[unit];
	register struct qedevice *addr = sc->addr;
	register struct ifnet *ifp = &sc->is_if;
/*	char (*setup_pkt)[8]; */
	char setup_pkt[16][8];
	int i, s;

	/* address not known */
	/* DECnet must set this somewhere to make device happy */
	if (ifp->if_addrlist == (struct ifaddr *)0)
		return;

	/*
	 * Initialize the transmit and receive rings.
	 */
	initring(sc->rring, sc->rmbuf, NRECV, QEALLOC);
	initring(sc->tring, sc->tmbuf, NXMIT, QENOALLOC);

	sc->nxmit = 0;
	sc->timeout = 0;
	sc->otindex = 0;
	sc->tindex = 0;
	sc->rindex = 0;

	/*
	 * Take the interface out of reset, program the vector, 
	 * enable interrupts, and tell the world we are up.
	 */
	s = splimp();
	addr->qe_vector = sc->qe_intvec;
	if (ifp->if_flags & IFF_LOOPBACK)
		addr->qe_csr = QE_RCV_ENABLE | QE_INT_ENABLE | QE_XMIT_INT | QE_RCV_INT | QE_ELOOP;
	else
		addr->qe_csr = QE_RCV_ENABLE | QE_INT_ENABLE | QE_XMIT_INT | QE_RCV_INT | QE_ILOOP;
	addr->qe_rcvlist_lo = &sc->rring[sc->rindex];
	addr->qe_rcvlist_hi = 0;
	ifp->if_flags |= IFF_UP | IFF_RUNNING;
	/*
	 * Get the addr off of the interface and place it into the setup
	 * packet. This code looks strange due to the fact that the address
	 * is placed in the setup packet in col. major order. 
	 */
/*
	setup_pkt = (char (*)[8])m_sget(16*8, 0);
	if (setup_pkt == NULL)
		panic("qe_setup");
 */
	for( i = 0 ; i < 6 ; i++ )
		setup_pkt[i][1] = addr->qe_sta_addr[i];
	qesetup(sc, setup_pkt);
	qeissuesetup(sc, "qeinit", setup_pkt);
/*
	MSFREE(setup_pkt);
*/
	qestart( unit );
	sc->ztime = time;
	splx( s );

}

/*
 * Initialize a bdl ring, possibly allocating mbufs
 * on the way.
 */
static
initring(rp, mbp, cnt, option)
register struct qe_ring *rp;
register struct mbuf **mbp;
register int cnt, option;
{
	struct qe_ring *orp = rp;

	do {
		qeinitdesc(rp++, mbp++, option, NULL);
	} while (--cnt > 0);
	qeinitdesc(rp, mbp, QENOALLOC, NULL);
	rp->qe_addr_lo = orp;
	rp->qe_addr_hi = 0;
	rp->qe_chain = 1;
	rp->qe_valid = 1;
}
 
/*
 * Start output on interface.
 *
 */
static
qestart(dev)
	dev_t dev;
{
        int unit = QEUNIT(dev);
	register struct qe_softc *sc = &qe_softc[unit];
	register struct qedevice *addr;
	register struct qe_ring *rp;
	struct mbuf *m;
	int desc_needed, i, buf_addr, len, j, tlen, s, diff;

	/*
	 * Check for enough transmit descriptors to map
	 * this datagram onto the interface. If there will never be enough
	 * throw the packet away and complain. If there will be enough but
	 * there aren't right now just return and if there are enough now,
	 * setup one or more descriptors to map the packet onto the interface 
	 * and start it.
	 *
	 * The deqna doesn't look at anything but the valid bit to determine
	 * if it should transmit this packet.  If you fill the ring, the
	 * device will loop indefinately and flood the network with packets
	 * until the ring is broken.  So, we always make sure there is at
	 * least one empty descriptor.
	 */
	s = splimp();
	if( (m = sc->is_if.if_snd.ifq_head) == 0 ){
		splx( s );
		return;				/* Nothing on the queue	*/
	}

	for( desc_needed = 0 ; m ; m = m->m_next )
		desc_needed++;

	if( desc_needed > NXMIT-1 )
		panic("qe: xmit packet too big");

	/*
	 * See if the required descriptors are available.
	 */
	addr = sc->addr;
	i = 0;
	j = sc->tindex;
	while( sc->tring[j].qe_valid == 0 && i <= desc_needed ) {
		i++;
		j = ++j % NXMIT;
	}

	/*
	 * Fred Canter -- 2/20/86
	 * Following allowed qestart() to overrun the xmit ring.
	 * This caused the orphaned mbuf problem.
	 */
/*	if( desc_needed > i+1 ) {	BOGUS!  */
	if((desc_needed+1) > i) {
		splx( s );
		return;
	}

	/*
	 * Record the chain head, attach each mbuf data area to a 
	 * descriptor and start the QNA if the transmit list is invalid.
	 */
	IF_DEQUEUE(&sc->is_if.if_snd, m);
	/*
	 * Save the chain head so that we can deallocate it after the
	 * i/o is done. This will not be necessary when we have an i/o map
	 * because we can use virtual addresses ?
	 */
	for(i=sc->tindex, sc->tmbuf[i]=m, tlen=0 ; m ; m=m->m_next, i = ++i % NXMIT){
		rp = &sc->tring[i];
		len = m->m_len;

		/*
		 *  Does buffer end on odd byte ? 
		 */
		if( len & 1 ) {
			len++;
			rp->qe_odd_end = 1;
		}
		tlen += len;
		rp->qe_buf_len = -(len/2);
		rp->qe_addr_lo = (ctob(m->m_click)&~077) + m->m_off;
		rp->qe_addr_hi = (m->m_click >> 10) & 077;
		if( m->m_next == NULL ) {
			/*
			 * Make sure we don't send a runt.
			 */
			if( tlen < MINDATA ) {
				diff = MINDATA - tlen;
				if( (len + diff + m->m_off) <= MMAXOFF ) {
					rp->qe_buf_len = -(len + diff)/2;
					tlen += diff;
				} else {
					/*
					 * This packet is too short.  Grab
					 * another descriptor, and point it
					 * to arbitrary data, just so that
					 * we can fill up to the minimum
					 * length.  This should probabaly be
					 * an empty buffer, not low core, but
					 * no one should look at it anyway...
					 */
					i = ++i % NXMIT;
					rp = &sc->tring[i];
					if( diff & 1 )
						diff++;
					rp->qe_buf_len = -(diff/2);
					rp->qe_addr_lo = 0;
					rp->qe_addr_hi = 0;
				}
			}
			rp->qe_eomsg = 1;
			rp->qe_valid = 1;
			/*
			 * Last descriptor !! If the QNA is running it could
			 * beat us down the list if we set the valid address 
			 * bits in the forward order, so we do it backwards.
			 */
			for( j = i ; j != sc->tindex ; ) {
				j = --j >= 0 ? j : NXMIT;
				sc->tring[j].qe_valid = 1;
			}
			sc->nxmit++;
			/* a few statistics */
			if ((sc->ctrblk.est_bytesent += len) < len)
				sc->ctrblk.est_bytesent -= len;
			if (++sc->ctrblk.est_bloksent == 0)
				--sc->ctrblk.est_bloksent;
		}
	}
	/*
	 * If the watchdog time isn't running kick it.
	 */
	sc->timeout = 1;
	if (!qewrun++)
		timeout(qewatch, 0, QETIMO);
	/*
	 * See if the xmit list is invalid.
	 */
	if( addr->qe_csr & QE_XL_INVALID ) {
		addr->qe_xmtlist_lo = &sc->tring[sc->tindex];
		addr->qe_xmtlist_hi = 0;
	}
	sc->tindex = i;
	splx( s );
}
 
/*
 * Ethernet interface interrupt processor
 */

qeint(unit)
int unit;
{
	register struct qe_softc *sc;
	struct qedevice *addr;
	int s, csr;
	mapinfo map;
/*@*/	extern char *panicstr;

/*@*/	if (panicstr)
/*@*/		return;
	unit = minor(unit);
	sc = &qe_softc[unit];
	addr = sc->addr;
	csr = addr->qe_csr;
	if (sc->is_if.if_flags & IFF_LOOPBACK)
		addr->qe_csr = QE_RCV_ENABLE | QE_INT_ENABLE | QE_XMIT_INT | QE_RCV_INT | QE_ELOOP;
	else
		addr->qe_csr = QE_RCV_ENABLE | QE_INT_ENABLE | QE_XMIT_INT | QE_RCV_INT | QE_ILOOP;
	savemap(map);
	if (csr & QE_RCV_INT)
		qerint(unit);
	if (csr & QE_NEX_MEM_INT)
		printf("qe%d: NXM intr\n", unit);
	else if (csr & QE_XMIT_INT)
		qetint(unit);
	
	if (addr->qe_csr & QE_RL_INVALID && sc->rring[sc->rindex].qe_status1 == QE_NOTYET) {

		addr->qe_rcvlist_lo = (int)&sc->rring[sc->rindex];
		addr->qe_rcvlist_hi = 0;
	}
	restormap(map);
}
 
/*
 * Ethernet interface transmit interrupt.
 */
static
qetint(unit)
	int unit;
{
	register struct qe_softc *sc = &qe_softc[unit];
	register struct mbuf *mp;
	register first, index;
	int i, len, status1;
/*	int	status2;	removed, was not used */

	while (sc->otindex != sc->tindex &&
	       sc->tring[sc->otindex].qe_status1 != QE_NOTYET) {
		/*
		 * Find the index of the last descriptor in this 
		 * packet. ( LASTNOT will be clear ) If we can't find one
		 * then the QNA is still working on it. This is necessary
		 * for subsequent passes because we can't be sure that the
		 * QNA is through with a descriptor until we find the last
		 * in the chain.
		 */
		first = index = sc->otindex;
		while (sc->tring[index].qe_valid && !sc->tring[index].qe_eomsg)
			index = ++index % NXMIT;
		/*
		 * Is the QNA done with this packet ?
		 */
		if (sc->tring[index].qe_status1 == QE_NOTYET)
			break;
		/*
		 * Save the status words from the descriptor so that it can
		 * be released.
		 */
		status1 = sc->tring[index].qe_status1;
/*		status2 = sc->tring[index].qe_status2;	not used */
		mp = sc->tmbuf[first];

		qeinitdesc(&sc->tring[first], &sc->tmbuf[first], QENOALLOC, NULL);
		if (--sc->nxmit == 0)
			sc->timeout = 0;
		if (first == NXMIT-1)
			sc->tring[NXMIT].qe_flag = QE_NOTYET;
		while (first != index) {
			first = ++first % NXMIT;
			qeinitdesc(&sc->tring[first], &sc->tmbuf[first], QENOALLOC, NULL);
			if (first == NXMIT-1)
				sc->tring[NXMIT].qe_flag = QE_NOTYET;
		}
		sc->otindex = ++index % NXMIT;

		/*
		 * Do some statistics.
		 */
		sc->is_if.if_opackets++;
		if (status1 & QE_CCNT) {
			i = (status1 & QE_CCNT) >> 4;
			sc->is_if.if_collisions += i;
			if (i == 1) {
				if (++sc->ctrblk.est_single == 0)
					--sc->ctrblk.est_single;
			} else {
				if (++sc->ctrblk.est_multiple == 0)
					--sc->ctrblk.est_multiple;
			}
		}
		if (status1 & QE_FAIL)
			if (++sc->ctrblk.est_collis == 0)
				--sc->ctrblk.est_collis;
		if (status1 & QE_ERROR) { 
			sc->is_if.if_oerrors++;
			if (++sc->ctrblk.est_sendfail == 0)
				--sc->ctrblk.est_sendfail;
			else {
				if (status1 & QE_ABORT)
					sc->ctrblk.est_sendfail_bm |= 1;
				if (status1 & QE_NOCAR)
					sc->ctrblk.est_sendfail_bm |= 2;
			}
			m_freem(mp);
		} else if (mp) {
			/*
			 * The QNA doesn't hear it's own packets. Unfortunately
			 * the code above us expects to hear all broadcast 
			 * traffic including our own. Therefore if this is a
			 * broadcast packet we have to loop it back,
			 * otherwise we simply free the packets.
			 */
			{
				register short *p;
				segm	map5;

				saveseg5(map5);
				p = (int *)(mtod(mp, struct ether_header *)
					->ether_dhost);
				i = *p++ == -1 && *p++ == -1 && *p == -1;
				restorseg5(map5);
			}
			if (i) {
				register struct mbuf *mp0;
				for (mp0 = mp, len=0; mp0; mp0 = mp0->m_next)
					len += mp0->m_len;
				qerecv(sc, mp, len);
			} else	
				m_freem(mp);
		}
	}
	qestart(unit);
}
 
struct foob {
	unsigned char lo;
	unsigned char hi;
};
#define	lobyte(x)	(((struct foob *)&(x))->lo)
#define	hibyte(x)	(((struct foob *)&(x))->hi)
/*
 * Ethernet interface receiver interrupt.
 * If can't determine length from type, then have to drop packet.  
 * Othewise decapsulate packet based on type and pass to type specific 
 * higher-level input routine.
 *	Fred Canter -- 2/20/86
 *	Serach the entire receive ring for a completed message
 *	instead of depending on the DEQNA to always put the next
 *	message in the descriptor pointed to by sc->rindex.
 *	Looks like this fixed the "winking out" problem.
 */
int	qe_search = 1;
static
qerint(unit)
	int unit;
{
	register struct qe_softc *sc = &qe_softc[unit];
    	struct mbuf *m, *n;
	int len, index, first, status1, status2, resid, drop;
	struct mbuf tmb;
	int	orindex, i;

	/*
	 * If qe_search set, find where the DEANA really put
	 * the next message (sometimes not where we expect).
	 */
	orindex = sc->rindex;
	i = sc->rindex;
	while(1) {
		if(qe_search == 0)
			break;
		if(sc->rring[i].qe_status1 != QE_NOTYET) {
			sc->rindex = i;
			break;
		}
		i = ++i % NRECV;
		if(i == orindex)
			return;
	}
	/*
	 * Traverse the receive ring looking for packets to pass back.
	 * The search is complete when we find a descriptor not in use.
	 */
	while ( sc->rring[sc->rindex].qe_status1 != QE_NOTYET ) {
		/*
		 * Find the index of the last descriptor in this 
		 * packet. ( LASTNOT will be clear ) If we can't find one
		 * then the QNA is still working on it.
		 */
		first = index = sc->rindex;

		while( (sc->rring[index].qe_status1 & QE_MASK) == QE_MASK )
			index = ++index % NRECV;
		/*
		 * If we found an unused descriptor we've beat the QNA 
		 */
		if( sc->rring[index].qe_status1 == QE_NOTYET )
			break;
		/*
		 * Save the status words from the descriptor so that it can
		 * be released. This thing isn't valid unless the low and
		 * high bytes are the same.
		 */
		status2 = sc->rring[index].qe_status2;
		if (lobyte(status2) != hibyte(status2))
			break;
		status1 = sc->rring[index].qe_status1;
		sc->is_if.if_ipackets++;
		/*
		 * Link the mbufs together, reinitialize the descriptors and
		 * pass the mbuf chain off to qerecv if status is okay.
		 */
		n = &tmb;
		drop = 0;
		for (;;) {
			m = sc->rmbuf[first];
			if (qeinitdesc(&sc->rring[first], &sc->rmbuf[first],
							QEALLOC, m)) {
				n->m_next = m;
				n = m;
			} else
				drop++;
/* see qeinitdesc()	sc->rring[first].qe_status2 = 1;	*/
			if (first == NRECV - 1)
				sc->rring[NRECV].qe_flag = QE_NOTYET;
			if (first == index)
				break;
			first = ++first % NRECV;
		}
		if ((sc->rindex = ++first % NRECV) == NRECV - 1)
			sc->rring[NRECV].qe_flag = QE_NOTYET;
		n->m_next = 0;
		m = tmb.m_next;
		/*
		 * If this was a setup packet, discard it.
		 */
		if (status1 & QE_ESETUP) {
			if (qedebug)
				printf("qe%d: setup pkt\n", unit);
			m_freem( m );
			continue;
		}
		if (drop) {
			sc->is_if.if_ierrors++;
			m_freem( m );
			if (qedebug)
				printf("qe%d: pkt drop\n", unit);
			continue;
		}
		/*
		 * If there was an error discard it.
		 */
		if (status1 & QE_ERROR) {
			sc->is_if.if_ierrors++;
			m_freem( m );
			if (status1&QE_DISCARD) {
				sc->ctrblk.est_recvfail++;
				if (status1&QE_CRCERR) {
					sc->ctrblk.est_recvfail_bm |= 1;
					if (status1&QE_FRAME)
						sc->ctrblk.est_recvfail_bm |= 2;
				}
				if (status1&QE_OVF)
					sc->ctrblk.est_recvfail_bm |= 4;
				if (qedebug) {
				    if (status1&QE_SHORT)
					printf("qe%d: short pkt\n", unit);
				    if (status1&QE_RUNT)
					printf("qe%d: runt pkt\n", unit);
				}
			} else if (qedebug) {
				if (status1&QE_RUNT)
					printf("qe%d: runt pkt\n", unit);
				else
					printf("qe%d: long pkt\n", unit);
			}
			continue;
		}
		/*
		 * Get the actual length and compute the size of data in the
		 * last mbuf. The hardware doesn't have time to count the first
		 * 60 bytes because it is doing address filtering so we add 60.
		 */
		len = ((status1 & QE_RBL_HI) | (status2 & QE_RBL_LO)) + 60;
		sc->ctrblk.est_bytercvd += len;
		sc->ctrblk.est_blokrcvd++;

		/*
		 * The last mbuf may not be full so we have to set the correct 
		 * length in it.
		 */
		if( resid = len % MLEN )
			n->m_len = resid;
		qerecv( sc, m, len );
	}
}
 
/*
 * Process a packet.
 */
static
qerecv( sc, m, len )
	struct qe_softc *sc;
	struct mbuf *m;
	int len;
{
    	register struct mbuf *mp;
	int off, resid, index;
	struct ifqueue *inq;
	register int type;
	struct protosw *pr;
	segm	map5;

	/*
	 * Deal with trailer protocol: If type is PUP trailer
	 * get true type from first 16-bit word past data.
	 * Remember that type was trailer by setting off.
	 */
	saveseg5(map5);
	type = ntohs(mtod(m, struct ether_header *)->ether_type);
	restorseg5(map5);
	if (type >= ETHERTYPE_TRAIL &&
	    type < ETHERTYPE_TRAIL+ETHERTYPE_NTRAILER) {
		off = ((type - ETHERTYPE_TRAIL) * 512) + sizeof(struct ether_header);;
		if (off >= ETHERMTU) 
			goto bad;		/* sanity */

		for (index=0, mp=m; mp && (index + mp->m_len) <= off; ) {
			index += mp->m_len;
			mp = mp->m_next;
		}
		if (mp) {
			register u_short *p;
			p = mtod(mp, caddr_t) + (off-index);
			type = ntohs(*p++);
			resid = ntohs(*p);
			restorseg5(map5);
		} else 
			goto bad;
		 
		if (off + resid > len) 
			goto bad;		/* sanity */

		len = off + resid;
	} else
		off = 0;
	if (len == 0)
		goto bad;

	/*
	 * Pull packet off interface.  Off is nonzero if packet
	 * has trailing header; qeget will then force this header
	 * information to be at the front, but we still have to drop
	 * the type and length words which are at the front of any trailer data.
	 */
	if (off) {
		if ((m = qeget(m, mp, off-index)) == 0) 
			return;
		 
		m->m_off += 2 * sizeof(u_short);
		m->m_len -= 2 * sizeof(u_short);
	} else {
		m->m_off += sizeof(struct ether_header);
		m->m_len -= sizeof(struct ether_header);
	}
	switch (type) {
#ifdef INET
	case ETHERTYPE_IP:
		schednetisr(NETISR_IP);
		inq = &ipintrq;
		break;

	case ETHERTYPE_ARP:
		arpinput(&sc->is_ac, m);
		return;
#endif
	default:
		/*
		 * See if other protocol families are defined and
		 * call protocol specific routines. If no other
		 * protocol families are defined then dump the message.
		 */
		if (pr = iftype_to_proto(type))
			(*pr->pr_ifinput)( m, &sc->is_if, &inq);
		else {
			if (++sc->ctrblk.est_unrecog == 0)
				--sc->ctrblk.est_unrecog;
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
	return;

	/*
	 * If the packet format looks like a trailer but is currupt we exit here
	 * after releasing the mbuf.
	 */
bad:
	m_freem(m);
	return;
}
 

/*
 * Ethernet output routine.
 * Encapsulate a packet of type family for the local net.
 * Use trailer local net encapsulation if enough data in first
 * packet leaves a multiple of 512 bytes of data in remainder.
 */
qeoutput(ifp, m0, dst)
	struct ifnet *ifp;
	struct mbuf *m0;
	struct sockaddr *dst;
{
	int type, s, error;
	u_char edst[6];
	struct in_addr idst;
	struct protosw *pr;
	register struct qe_softc *is = &qe_softc[ifp->if_unit];
	register struct mbuf *m = m0;
	register struct ether_header *eh;
	register int off;
	segm	map5;

	saveseg5(map5);
	switch (dst->sa_family) {

#ifdef INET
	case AF_INET:
		idst = ((struct sockaddr_in *)dst)->sin_addr;
		if (!arpresolve(&is->is_ac, m, &idst, edst))
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
			*p = htons(m->m_len);
			goto gottraqeertype;
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
		 * Try to find other address families and call protocol
		 * specific output routine.
		 */
		if (pr = iffamily_to_proto(dst->sa_family)) {
			(*pr->pr_ifoutput)(ifp, m0, dst, &type, (char *)edst);
			goto gottype;
		} else {
			printf("qe%d: can't handle af%d\n", ifp->if_unit,
				dst->sa_family);
			error = EAFNOSUPPORT;
			goto bad;
		}
	}

gottraqeertype:
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
	if (m->m_off > MMAXOFF || MMINOFF + sizeof (struct ether_header) > m->m_off) {
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
	bcopy((caddr_t)is->is_addr, (caddr_t)eh->ether_shost, sizeof (is->is_addr));
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
	qestart(ifp->if_unit);
	splx(s);
	return (0);

bad:
	restorseg5(map5);
	m_freem(m0);
	return (error);
}
 

/*
 * Process an ioctl request.
 */
qeioctl(ifp, cmd, data)
	register struct ifnet *ifp;
	int cmd;
	caddr_t data;
{
	struct qe_softc *sc = &qe_softc[ifp->if_unit];
	struct sockaddr *sa;
	struct sockaddr_in *sin;
	struct ifreq *ifr = (struct ifreq *)data;
	struct protosw *pr;
	int i,s = splimp(), error = 0;
	struct ifaddr *ifa = (struct ifaddr *)data;

	switch (cmd) {

	case SIOCSPHYSADDR:
	    {
/*
		char (*setup_pkt)[8];
*/
		char setup_pkt[16][8];

		/*
		 * Before we do anything, make sure we can get the
		 * data space if we need it.
		 */
/*
		if (ifp->if_flags & IFF_RUNNING) {
			setup_pkt = (char (*)[8])m_sget(16*8, 0);
			if (setup_pkt == NULL) {
				splx(s);
				return(ENOBUFS);
			}
		}
*/
		bcopy(ifr->ifr_addr.sa_data, &sc->is_addr, sizeof(sc->is_addr));
		if (ifp->if_flags & IFF_RUNNING) {
			for (i = 0; i < 6; i++)
			    setup_pkt[i][1] = sc->is_addr[i];
			qesetup(sc, setup_pkt);
			qeissuesetup(sc, "SIOCPHYSADDR", setup_pkt);
#ifdef	notdef
			if_rtinit(ifp, -1);
#endif
/*
			MSFREE(setup_pkt);
*/
		}
		qeinit(ifp->if_unit);
		break;
	    }

	case SIOCSIFADDR:
		ifp->if_flags |= IFF_UP;
		qeinit(ifp->if_unit);
		switch(ifa->ifa_addr.sa_family) {
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
					(qe_softc[ifp->if_unit].qe_addr);
			} else {
				qe_setaddr(ina->x_host.c_host, ifp->if_unit);
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
		error = EINVAL;
	}
done:	splx(s);
	return (error);
}

/*
 * Initialize a ring descriptor with mbuf allocation side effects
 * If we are allocating, and mp2 is non-null, then use it if we
 * can't allocate a new mbuf.  The return value indicates whether
 * or not we used mp2.
 */
static
qeinitdesc(rp, mp, option, mp2)
register struct qe_ring *rp;
register struct mbuf **mp;
int option;
struct mbuf *mp2;
{
	/*
	 * clear the entire descriptor
	 */
	bzero(rp, sizeof(*rp));

	rp->qe_flag = rp->qe_status1 = QE_NOTYET;
	rp->qe_status2 = 0xff00;

	/*
	 * Perform the necessary allocation/deallocation
	 */
	if (option == QENOALLOC)
		*mp = NULL;
	else {
		register struct mbuf *m;

		if ((option == QEALLOC) || ((m = *mp) == NULL))
			MGET(m, M_DONTWAIT, MT_DATA);
		if (m == 0) {
		    if ((m = mp2) == 0)
			panic("qe: no mbufs for desc ring");
		}
		*mp = m;
		rp->qe_buf_len = -(MLEN/2);
		m->m_len = MLEN; 
		m->m_off = MMINOFF;
		rp->qe_addr_lo = (ctob(m->m_click)&~077) + m->m_off;
		rp->qe_addr_hi = (m->m_click >> 10) & 077;
		/*
		 * Must be last. QNA could be listening.
		 */
		rp->qe_valid = 1;
		return(m != mp2);
	}
	return(1);
}
 

/*
 * Pull the trailer info up to the front of the chain.
 */
static struct mbuf *
qeget(m, mtp, off)
register struct mbuf 	*m;	/* Pointer to first mbuf in the chain 	*/
register struct mbuf	*mtp;	/* Pointer to first mbuf in trailer	*/
int			off;	/* Offset to trailer in trailer mbuf	*/
{
	register struct mbuf	*mp;
	struct mbuf		*mp0;

	/*
	 * The trailer consists of 2 words followed by an IP header.
	 * The rest of the code assumes that the trailer is enclosed in
	 * a single mbuf. In our case the trailer spans two mbufs so it
	 * has to be packed back into a single packet so that other routines
	 * can access it as a structure.
	 */
	MGET(mp, M_DONTWAIT, MT_DATA);
	if (mp == NULL) {
		m_freem(m);
		return(0);
	}
	mp->m_off = MMINOFF;
	/*
	 * Copy the portion of the trailer in this mbuf to the new
	 * mbuf and adjust the length. This mbuf now becomes the last
	 * in the packet.
	 */
	MBCOPY(mtp, off, mp, 0, mtp->m_len - off);
	mp->m_len = mtp->m_len - off;
	mtp->m_len = off;
	mp0 = mtp;			/* Save so we can break the chain */
	/*
	 * The remainder of the trailer is in the next mbuf.
	 */
	mtp = mtp->m_next;
	if (mtp) {
		if (mtp->m_len + mp->m_len > MLEN || mtp->m_next) {
			/*
			 * Can't fit trailer in an mbuf. Must be insane.
			 */
			m_freem(m);
			m_freem(mp);
			return(0);
		}
		/*
		 * Pull the data up, break the chain and release the old
		 * trailer.
		 */
		MBCOPY(mtp, 0, mp, mp->m_len, mtp->m_len);
		mp->m_len += mtp->m_len;
		mp0->m_next = NULL;	/* Break the chain		*/
		m_freem(mtp);
	}

	/*
	 * Move the new trailer to the head of the packet and remove the 
	 * ethernet header.
	 */
	mp->m_next = m;
	m->m_off += sizeof (struct ether_header);
	m->m_len -= sizeof (struct ether_header);
	return(mp);
}

/*
 * Build a setup packet - the physical address will already be present
 * in first column.
 */
static
qesetup(sc, setup_pkt)
struct qe_softc *sc;
char setup_pkt[16][8];
{
    register int i,j;

    /*
     * Copy the target address to the rest of the entries in this row.
     */
     for (j = 0; j < 6; j++)
	    for (i = 2; i < 8; i++)
		    setup_pkt[j][i] = setup_pkt[j][1];
    /*
     * Duplicate the first half.
     */
    bcopy(setup_pkt, setup_pkt[8], 64);
    /*
     * Fill in the broadcast address.
     */
    for (i = 0; i < 6; i++)
	    setup_pkt[i][2] = 0xff;
}

/*
 * Issue a setup packet to the QNA and wait for it's completion. Report an
 * error if we can't immediately get the transmit ring entry to send the
 * setup packet or if the transmission fails.
 */
static
qeissuesetup(sc, cmdname, setup_pkt)
struct qe_softc *sc;
char *cmdname;
char setup_pkt[16][8];
{
    register struct qe_ring *rp = (struct qe_ring *)&sc->tring[sc->tindex];
    struct qedevice *addr = sc->addr;

    if (rp->qe_valid == 0) {
	    sc->tmbuf[sc->tindex] = 0;
	    if (setup_pkt >= 0140000) {
		    /* packet is on stack, get 22 bit address */
		    long paddr;
		    paddr = ctob((long)*KDSA6) + (long)setup_pkt - 0140000L;
		    rp->qe_addr_lo = (int)(paddr&0177777);
		    rp->qe_addr_hi = (int)((paddr>>16)&077);
	    } else {
		    rp->qe_addr_lo = setup_pkt;
		    rp->qe_addr_hi = 0;
	    }
	    rp->qe_buf_len = -64;
	    rp->qe_setup = 1;
	    rp->qe_eomsg = 1;
	    rp->qe_valid = 1;

	    if (addr->qe_csr & QE_XL_INVALID) {
		    addr->qe_xmtlist_lo = rp;
		    addr->qe_xmtlist_hi = 0;
	    }
	    sc->nxmit++;
	    sc->tindex = ++sc->tindex % NXMIT;
	    while (rp->qe_status1 == QE_NOTYET)
		    ;
	    /*
	     * Avoid an obscure race condition with the hardware continueing
	     * around the transmit ring and finding this setup packet again.
	     */
	    rp->qe_setup = 0;
    } else {
	    printf("qe%d: %s failed: no ring entry\n", sc->is_if.if_unit, cmdname);
    }
}

qewatch()
{
	register struct	qe_softc *sc;
	register int i, inprogress = 0;

	for (sc = &qe_softc[0], i = nNQE; i ; --i) {
		if (sc->timeout) {
			if (++sc->timeout > 3)
				qerestart(sc);
			else
				inprogress++;
		}
		sc++;
	}
	if (inprogress) {
		timeout(qewatch, 0, QETIMO);
		qewrun++;
	} else
		qewrun = 0;
}

/*
 * Fred Canter -- 2/20/86
 * On a qerestart, if qe_xfree is set free all mbufs
 * on the transmit ring, otherwise re-transmit them.
 * Freeing is the recommended approach.
 */
int	qe_xfree = 1;
int	qe_restarts;	/* number of qerestarts since boot */

static
qerestart(sc)
register struct qe_softc *sc;
{
	register struct ifnet *ifp = &sc->is_if;
	register struct qedevice *addr = sc->addr;
	char setup_pkt[16][8];
	register i;

	/*
	 * 1. Reset the device.
	 * 2. Clean out the transmit ring, and put all the pending
	 *    transmits back on the send queue.  We do this in
	 *    reverse order, and stick them at the front of the queue.
	 * 3. Clean out and re-allocate the receive ring, making
	 *    sure that we free up any mbufs that don't get re-used!
	 * 4. Turn on interrupts, validate the receive ring, and
	 *    issue a setup packet.
	 * 5. Restart the device.
	 */
	addr->qe_csr = QE_RESET;
	/*
	 * Clear of CSR added at the request of hardware
	 * engineering, so the DELQA can be bug for bug
	 * compatible with the DEQNA.
	 * 1/28/86 -- Fred Canter
	 */
	addr->qe_csr = 0;
	sc->timeout = 0;
	qe_restarts++;

	if(qe_xfree == 1) {
	    for(i=0; i<NXMIT; i++) {
		register struct mbuf *m;
		if (m = sc->tmbuf[i])
			m_freem(m);
	    }
	}
	if(qe_xfree == 0) {
	    i = sc->tindex;
	    do {
		register struct mbuf *m;
		if ( --i < 0)
			i = NXMIT - 1;
		if (m = sc->tmbuf[i])
			IF_PREPEND(&ifp->if_snd, m);
	    } while(i != sc->otindex);
	}
	initring(sc->tring, sc->tmbuf, NXMIT, QENOALLOC);
	initring(sc->rring, sc->rmbuf, NRECV, QEREALLOC);

	sc->nxmit = sc->otindex = sc->tindex = sc->rindex = 0;

	if (ifp->if_flags & IFF_LOOPBACK)
		addr->qe_csr = QE_RCV_ENABLE | QE_INT_ENABLE | QE_XMIT_INT |
							QE_RCV_INT | QE_ELOOP;
	else
		addr->qe_csr = QE_RCV_ENABLE | QE_INT_ENABLE | QE_XMIT_INT |
							QE_RCV_INT | QE_ILOOP;
	addr->qe_rcvlist_lo = &sc->rring[0];
	addr->qe_rcvlist_hi = 0;

	for( i = 0 ; i < 6 ; i++ )
		setup_pkt[i][1] = sc->is_addr[i];
	qesetup(sc, setup_pkt);
	qeissuesetup(sc, "qerestart", setup_pkt);
	qestart(ifp->if_unit);
	if (qedebug)
		printf("qerestart: restarted qe%d\n", ifp->if_unit);
}
