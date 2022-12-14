
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * Copyright (c) 1982 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * SCCSID: @(#)tcp_input.c	3.0	(ULTRIX-11)	4/21/86
 *	Based on: @(#)tcp_input.c	6.11 (Berkeley) 6/8/85
 */


#include <sys/param.h>
#include <sys/systm.h>
#include <sys/mbuf.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <errno.h>

#include <net/if.h>
#include <net/route.h>

#include <netinet/in.h>
#include <netinet/in_pcb.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/tcp.h>
#include <netinet/tcp_fsm.h>
#include <netinet/tcp_seq.h>
#include <netinet/tcp_timer.h>
#include <netinet/tcp_var.h>
#include <netinet/tcpip.h>
#include <netinet/tcp_debug.h>

int	tcpprintfs = 0;
int	tcpcksum = 1;
#ifdef	TCP_DEBUG
struct	tcpiphdr tcp_saveti;
#endif	TCP_DEBUG
extern	tcpnodelack;

struct	tcpcb *tcp_newtcpcb();
/*
 * TCP input routine, follows pages 65-76 of the
 * protocol specification dated September, 1981 very closely.
 */
tcp_input(m0)
	struct mbuf *m0;
{
	register struct tcpiphdr *ti;
	struct inpcb *inp;
	register struct mbuf *m;
	struct mbuf *om = 0;
	int len, tlen, off;
	register struct tcpcb *tp = 0;
	register int tiflags;
	struct socket *so;
	int todrop, acked;
#ifdef	TCP_DEBUG
	short ostate;
#endif	TCP_DEBUG
	struct in_addr laddr;
	int dropsocket = 0;
#ifdef	pdp11
	segm	map;

#define	return	goto finishup
	saveseg5(map);
#endif	pdp11

	/*
	 * Get IP and TCP header together in first mbuf.
	 * Note: IP leaves IP header in first mbuf.
	 */
	m = m0;
	ti = mtod(m, struct tcpiphdr *);
	if (((struct ip *)ti)->ip_hl > (sizeof (struct ip) >> 2))
		ip_stripoptions((struct ip *)ti, (struct mbuf *)0);
	if (m->m_off > MMAXOFF || m->m_len < sizeof (struct tcpiphdr)) {
		if ((m = m_pullup(m, sizeof (struct tcpiphdr))) == 0) {
			tcpstat.tcps_hdrops++;
			return;
		}
		ti = mtod(m, struct tcpiphdr *);
	}

	/*
	 * Checksum extended TCP header and data.
	 */
	tlen = ((struct ip *)ti)->ip_len;
	len = sizeof (struct ip) + tlen;
#ifdef	TCPLOOPBACK
	if (tcpcksum && (ti->ti_src.s_addr != ti->ti_dst.s_addr)) {
#else	TCPLOOPBACK
	if (tcpcksum) {
#endif	TCPLOOPBACK
		ti->ti_next = ti->ti_prev = 0;
#ifdef	pdp11
		ti->ti_pad = 0;
#endif	pdp11
		ti->ti_x1 = 0;
		ti->ti_len = (u_short)tlen;
		ti->ti_len = htons((u_short)ti->ti_len);
		if (ti->ti_sum = in_cksum(m, len)) {
			if (tcpprintfs)
				printf("tcp sum: src %x\n", ti->ti_src);
			tcpstat.tcps_badsum++;
			goto drop;
		}
	}

	/*
	 * Check that TCP offset makes sense,
	 * pull out TCP options and adjust length.
	 */
	off = ti->ti_off << 2;
	if (off < sizeof (struct tcphdr) || off > tlen) {
		if (tcpprintfs)
			printf("tcp off: src %x off %d\n", ti->ti_src, off);
		tcpstat.tcps_badoff++;
		goto drop;
	}
	tlen -= off;
	ti->ti_len = tlen;
	if (off > sizeof (struct tcphdr)) {
		if ((m = m_pullup(m, sizeof (struct ip) + off)) == 0) {
			tcpstat.tcps_hdrops++;
			return;
		}
		ti = mtod(m, struct tcpiphdr *);
		om = m_get(M_DONTWAIT, MT_DATA);
		if (om == 0)
			goto drop;
		om->m_len = off - sizeof (struct tcphdr);
		{ caddr_t op = mtod(m, caddr_t) + sizeof (struct tcpiphdr);
#ifndef	pdp11
		  bcopy(op, mtod(om, caddr_t), (unsigned)om->m_len);
#else	pdp11
		  MBCOPY(m, sizeof(struct tcpiphdr), om, 0, (unsigned)om->m_len);
#endif	pdp11
		  m->m_len -= om->m_len;
		  bcopy(op+om->m_len, op,
		   (unsigned)(m->m_len-sizeof (struct tcpiphdr)));
		}
	}
	tiflags = ti->ti_flags;

	/*
	 * Drop TCP and IP headers.
	 */
	off += sizeof (struct ip);
	m->m_off += off;
	m->m_len -= off;

	/*
	 * Convert TCP protocol specific fields to host format.
	 */
	ti->ti_seq = ntohl(ti->ti_seq);
	ti->ti_ack = ntohl(ti->ti_ack);
	ti->ti_win = ntohs(ti->ti_win);
	ti->ti_urp = ntohs(ti->ti_urp);

	/*
	 * Locate pcb for segment.
	 */
	inp = in_pcblookup
		(&tcb, ti->ti_src, ti->ti_sport, ti->ti_dst, ti->ti_dport,
		INPLOOKUP_WILDCARD);

	/*
	 * If the state is CLOSED (i.e., TCB does not exist) then
	 * all data in the incoming segment is discarded.
	 */
	if (inp == 0)
		goto dropwithreset;
	tp = intotcpcb(inp);
	if (tp == 0)
		goto dropwithreset;
	so = inp->inp_socket;
#ifdef	TCP_DEBUG
	if (so->so_options & SO_DEBUG) {
		ostate = tp->t_state;
		tcp_saveti = *ti;
	}
#endif	TCP_DEBUG
	if (so->so_options & SO_ACCEPTCONN) {
		so = sonewconn(so);
		if (so == 0)
			goto drop;
		/*
		 * This is ugly, but ....
		 *
		 * Mark socket as temporary until we're
		 * committed to keeping it.  The code at
		 * ``drop'' and ``dropwithreset'' check the
		 * flag dropsocket to see if the temporary
		 * socket created here should be discarded.
		 * We mark the socket as discardable until
		 * we're committed to it below in TCPS_LISTEN.
		 */
		dropsocket++;
		inp = (struct inpcb *)so->so_pcb;
		inp->inp_laddr = ti->ti_dst;
		inp->inp_lport = ti->ti_dport;
		tp = intotcpcb(inp);
		tp->t_state = TCPS_LISTEN;
	}

	/*
	 * Segment received on connection.
	 * Reset idle time and keep-alive timer.
	 */
	tp->t_idle = 0;
	tp->t_timer[TCPT_KEEP] = TCPTV_KEEP;

	/*
	 * Process options if not in LISTEN state,
	 * else do it below (after getting remote address).
	 */
	if (om && tp->t_state != TCPS_LISTEN) {
		tcp_dooptions(tp, om, ti);
		om = 0;
	}

	/*
	 * Calculate amount of space in receive window,
	 * and then do TCP input processing.
	 */
	tp->rcv_wnd = sbspace(&so->so_rcv);
	if (tp->rcv_wnd < 0)
		tp->rcv_wnd = 0;

	switch (tp->t_state) {

	/*
	 * If the state is LISTEN then ignore segment if it contains an RST.
	 * If the segment contains an ACK then it is bad and send a RST.
	 * If it does not contain a SYN then it is not interesting; drop it.
	 * Otherwise initialize tp->rcv_nxt, and tp->irs, select an initial
	 * tp->iss, and send a segment:
	 *     <SEQ=ISS><ACK=RCV_NXT><CTL=SYN,ACK>
	 * Also initialize tp->snd_nxt to tp->iss+1 and tp->snd_una to tp->iss.
	 * Fill in remote peer address fields if not previously specified.
	 * Enter SYN_RECEIVED state, and process any other fields of this
	 * segment in this state.
	 */
	case TCPS_LISTEN: {
#ifndef	pdp11
		struct mbuf *am;
#endif	pdp11
		register struct sockaddr_in *sin;

		if (tiflags & TH_RST)
			goto drop;
		if (tiflags & TH_ACK)
			goto dropwithreset;
		if ((tiflags & TH_SYN) == 0)
			goto drop;
#ifndef	pdp11
		am = m_get(M_DONTWAIT, MT_SONAME);
		if (am == NULL)
			goto drop;
		am->m_len = sizeof (struct sockaddr_in);
		sin = mtod(am, struct sockaddr_in *);
#else	pdp11
		MSGET(sin, struct sockaddr_in, MT_SONAME, 0, M_DONTWAIT);
		if (sin == NULL)
			goto drop;
#endif	pdp11
		sin->sin_family = AF_INET;
		sin->sin_addr = ti->ti_src;
		sin->sin_port = ti->ti_sport;
		laddr = inp->inp_laddr;
		if (inp->inp_laddr.s_addr == INADDR_ANY)
			inp->inp_laddr = ti->ti_dst;
#ifndef	pdp11
		if (in_pcbconnect(inp, am))
#else	pdp11
		if (in_pcbconnect(inp, sin))
#endif	pdp11
		{
			inp->inp_laddr = laddr;
#ifndef	pdp11
			(void) m_free(am);
#else	pdp11
			MSFREE(sin);
#endif	pdp11
			goto drop;
		}
#ifndef	pdp11
		(void) m_free(am);
#else	pdp11
		MSFREE(sin);
#endif	pdp11
		tp->t_template = tcp_template(tp);
		if (tp->t_template == 0) {
			in_pcbdisconnect(inp);
			dropsocket = 0;		/* socket is already gone */
			inp->inp_laddr = laddr;
			tp = 0;
			goto drop;
		}
		if (om) {
			tcp_dooptions(tp, om, ti);
			om = 0;
		}
		tp->iss = tcp_iss; tcp_iss += TCP_ISSINCR/2;
		tp->irs = ti->ti_seq;
		tcp_sendseqinit(tp);
		tcp_rcvseqinit(tp);
		tp->t_state = TCPS_SYN_RECEIVED;
		tp->t_timer[TCPT_KEEP] = TCPTV_KEEP;
		dropsocket = 0;		/* committed to socket */
		goto trimthenstep6;
		}

	/*
	 * If the state is SYN_SENT:
	 *	if seg contains an ACK, but not for our SYN, drop the input.
	 *	if seg contains a RST, then drop the connection.
	 *	if seg does not contain SYN, then drop it.
	 * Otherwise this is an acceptable SYN segment
	 *	initialize tp->rcv_nxt and tp->irs
	 *	if seg contains ack then advance tp->snd_una
	 *	if SYN has been acked change to ESTABLISHED else SYN_RCVD state
	 *	arrange for segment to be acked (eventually)
	 *	continue processing rest of data/controls, beginning with URG
	 */
	case TCPS_SYN_SENT:
		if ((tiflags & TH_ACK) &&
/* this should be SEQ_LT; is SEQ_LEQ for BBN vax TCP only */
		    (SEQ_LT(ti->ti_ack, tp->iss) ||
		     SEQ_GT(ti->ti_ack, tp->snd_max)))
			goto dropwithreset;
		if (tiflags & TH_RST) {
			if (tiflags & TH_ACK)
				tp = tcp_drop(tp, ECONNREFUSED);
			goto drop;
		}
		if ((tiflags & TH_SYN) == 0)
			goto drop;
		tp->snd_una = ti->ti_ack;
		if (SEQ_LT(tp->snd_nxt, tp->snd_una))
			tp->snd_nxt = tp->snd_una;
		tp->t_timer[TCPT_REXMT] = 0;
		tp->irs = ti->ti_seq;
		tcp_rcvseqinit(tp);
		tp->t_flags |= TF_ACKNOW;
		if (SEQ_GT(tp->snd_una, tp->iss)) {
			soisconnected(so);
			tp->t_state = TCPS_ESTABLISHED;
			tp->t_maxseg = MIN(tp->t_maxseg, tcp_mss(tp));
			(void) tcp_reass(tp, (struct tcpiphdr *)0);
		} else
			tp->t_state = TCPS_SYN_RECEIVED;
		goto trimthenstep6;

trimthenstep6:
		/*
		 * Advance ti->ti_seq to correspond to first data byte.
		 * If data, trim to stay within window,
		 * dropping FIN if necessary.
		 */
		ti->ti_seq++;
		if (ti->ti_len > tp->rcv_wnd) {
			todrop = ti->ti_len - tp->rcv_wnd;
			m_adj(m, -todrop);
			ti->ti_len = tp->rcv_wnd;
			ti->ti_flags &= ~TH_FIN;
		}
		tp->snd_wl1 = ti->ti_seq - 1;
		goto step6;
	}

	/*
	 * If data is received on a connection after the
	 * user processes are gone, then RST the other end.
	 */
	if ((so->so_state & SS_NOFDREF) && tp->t_state > TCPS_CLOSE_WAIT &&
	    ti->ti_len) {
		tp = tcp_close(tp);
		goto dropwithreset;
	}

	/*
	 * States other than LISTEN or SYN_SENT.
	 * First check that at least some bytes of segment are within 
	 * receive window.
	 */
	if (tp->rcv_wnd == 0) {
		/*
		 * If window is closed can only take segments at
		 * window edge, and have to drop data and PUSH from
		 * incoming segments.
		 */
		if (tp->rcv_nxt != ti->ti_seq)
			goto dropafterack;
		if (ti->ti_len > 0) {
			m_adj(m, ti->ti_len);
			ti->ti_len = 0;
			ti->ti_flags &= ~(TH_PUSH|TH_FIN);
		}
	} else {
		/*
		 * If segment begins before rcv_nxt, drop leading
		 * data (and SYN); if nothing left, just ack.
		 */
		todrop = tp->rcv_nxt - ti->ti_seq;
		if (todrop > 0) {
			if (tiflags & TH_SYN) {
				tiflags &= ~TH_SYN;
				ti->ti_flags &= ~TH_SYN;
				ti->ti_seq++;
				if (ti->ti_urp > 1) 
					ti->ti_urp--;
				else
					tiflags &= ~TH_URG;
				todrop--;
			}
			if (todrop > ti->ti_len ||
			    todrop == ti->ti_len && (tiflags&TH_FIN) == 0)
				goto dropafterack;
			m_adj(m, todrop);
			ti->ti_seq += todrop;
			ti->ti_len -= todrop;
			if (ti->ti_urp > todrop)
				ti->ti_urp -= todrop;
			else {
				tiflags &= ~TH_URG;
				ti->ti_flags &= ~TH_URG;
				ti->ti_urp = 0;
			}
		}
		/*
		 * If segment ends after window, drop trailing data
		 * (and PUSH and FIN); if nothing left, just ACK.
		 */
		todrop = (ti->ti_seq+ti->ti_len) - (tp->rcv_nxt+tp->rcv_wnd);
		if (todrop > 0) {
			if (todrop >= ti->ti_len)
				goto dropafterack;
			m_adj(m, -todrop);
			ti->ti_len -= todrop;
			ti->ti_flags &= ~(TH_PUSH|TH_FIN);
		}
	}

	/*
	 * If the RST bit is set examine the state:
	 *    SYN_RECEIVED STATE:
	 *	If passive open, return to LISTEN state.
	 *	If active open, inform user that connection was refused.
	 *    ESTABLISHED, FIN_WAIT_1, FIN_WAIT2, CLOSE_WAIT STATES:
	 *	Inform user that connection was reset, and close tcb.
	 *    CLOSING, LAST_ACK, TIME_WAIT STATES
	 *	Close the tcb.
	 */
	if (tiflags&TH_RST) switch (tp->t_state) {

	case TCPS_SYN_RECEIVED:
		tp = tcp_drop(tp, ECONNREFUSED);
		goto drop;

	case TCPS_ESTABLISHED:
	case TCPS_FIN_WAIT_1:
	case TCPS_FIN_WAIT_2:
	case TCPS_CLOSE_WAIT:
		tp = tcp_drop(tp, ECONNRESET);
		goto drop;

	case TCPS_CLOSING:
	case TCPS_LAST_ACK:
	case TCPS_TIME_WAIT:
		tp = tcp_close(tp);
#ifdef	TCPLOOPBACK
	case TCPS_CLOSED:
#endif	TCPLOOPBACK
		goto drop;
	}

	/*
	 * If a SYN is in the window, then this is an
	 * error and we send an RST and drop the connection.
	 */
	if (tiflags & TH_SYN) {
		tp = tcp_drop(tp, ECONNRESET);
		goto dropwithreset;
	}

	/*
	 * If the ACK bit is off we drop the segment and return.
	 */
	if ((tiflags & TH_ACK) == 0)
		goto drop;
	
	/*
	 * Ack processing.
	 */
	switch (tp->t_state) {

	/*
	 * In SYN_RECEIVED state if the ack ACKs our SYN then enter
	 * ESTABLISHED state and continue processing, othewise
	 * send an RST.
	 */
	case TCPS_SYN_RECEIVED:
		if (SEQ_GT(tp->snd_una, ti->ti_ack) ||
		    SEQ_GT(ti->ti_ack, tp->snd_max))
			goto dropwithreset;
		tp->snd_una++;			/* SYN acked */
		if (SEQ_LT(tp->snd_nxt, tp->snd_una))
			tp->snd_nxt = tp->snd_una;
		tp->t_timer[TCPT_REXMT] = 0;
		soisconnected(so);
		tp->t_state = TCPS_ESTABLISHED;
		tp->t_maxseg = MIN(tp->t_maxseg, tcp_mss(tp));
		(void) tcp_reass(tp, (struct tcpiphdr *)0);
		tp->snd_wl1 = ti->ti_seq - 1;
		/* fall into ... */

	/*
	 * In ESTABLISHED state: drop duplicate ACKs; ACK out of range
	 * ACKs.  If the ack is in the range
	 *	tp->snd_una < ti->ti_ack <= tp->snd_max
	 * then advance tp->snd_una to ti->ti_ack and drop
	 * data from the retransmission queue.  If this ACK reflects
	 * more up to date window information we update our window information.
	 */
	case TCPS_ESTABLISHED:
	case TCPS_FIN_WAIT_1:
	case TCPS_FIN_WAIT_2:
	case TCPS_CLOSE_WAIT:
	case TCPS_CLOSING:
	case TCPS_LAST_ACK:
	case TCPS_TIME_WAIT:
#define	ourfinisacked	(acked > 0)

		if (SEQ_LEQ(ti->ti_ack, tp->snd_una))
			break;
		if (SEQ_GT(ti->ti_ack, tp->snd_max))
			goto dropafterack;
		acked = ti->ti_ack - tp->snd_una;

		/*
		 * If transmit timer is running and timed sequence
		 * number was acked, update smoothed round trip time.
		 */
		if (tp->t_rtt && SEQ_GT(ti->ti_ack, tp->t_rtseq)) {
			if (tp->t_srtt == 0)
#ifdef pdp11
				tp->t_srtt = tp->t_rtt * 10;
#else vax
				tp->t_srtt = tp->t_rtt;
#endif
			else
				tp->t_srtt =
#ifdef pdp11
				    (tcp_alpha * tp->t_srtt) / 10 +
				    (10 - tcp_alpha) * tp->t_rtt;
#else vax
				    tcp_alpha * tp->t_srtt +
				    (1 - tcp_alpha) * tp->t_rtt;
#endif
			tp->t_rtt = 0;
		}

		if (ti->ti_ack == tp->snd_max)
			tp->t_timer[TCPT_REXMT] = 0;
		else {
			TCPT_RANGESET(tp->t_timer[TCPT_REXMT],
#ifdef pdp11
			    (tcp_beta * tp->t_srtt)/100, TCPTV_MIN, TCPTV_MAX);
#else vax
			    tcp_beta * tp->t_srtt, TCPTV_MIN, TCPTV_MAX);
#endif
			tp->t_rxtshift = 0;
		}
		/*
		 * When new data is acked, open the congestion window a bit.
		 */
		if (acked > 0)
#ifdef	vax
			tp->snd_cwnd = MIN(11 * tp->snd_cwnd / 10, 65535);
#else	pdp11
		{
			/*
			 * We need to avoid overflow. 110% of
			 * anything over 59570 will be over 65535,
			 * so just assign, only do the calculation
			 * when we know it won't overflow!
			 */
			if (tp->snd_cwnd >= 59570)
				tp->snd_cwnd = 65535;
			else {
				register unsigned t;
				t = tp->snd_cwnd/10;
				/* make sure t not 0 -- Fred 2/20/86 */
				if(t == 0)
					t++;
				tp->snd_cwnd = t*11;
			}
		}
#endif
		if (acked > so->so_snd.sb_cc) {
			tp->snd_wnd -= so->so_snd.sb_cc;
			sbdrop(&so->so_snd, so->so_snd.sb_cc);
		} else {
			sbdrop(&so->so_snd, acked);
			tp->snd_wnd -= acked;
			acked = 0;
		}
		if ((so->so_snd.sb_flags & SB_WAIT) || so->so_snd.sb_sel)
			sowwakeup(so);
		tp->snd_una = ti->ti_ack;
		if (SEQ_LT(tp->snd_nxt, tp->snd_una))
			tp->snd_nxt = tp->snd_una;

		switch (tp->t_state) {

		/*
		 * In FIN_WAIT_1 STATE in addition to the processing
		 * for the ESTABLISHED state if our FIN is now acknowledged
		 * then enter FIN_WAIT_2.
		 */
		case TCPS_FIN_WAIT_1:
			if (ourfinisacked) {
				/*
				 * If we can't receive any more
				 * data, then closing user can proceed.
				 */
				if (so->so_state & SS_CANTRCVMORE)
					soisdisconnected(so);
				tp->t_state = TCPS_FIN_WAIT_2;
				/*
				 * This is contrary to the specification,
				 * but if we haven't gotten our FIN in 
				 * 5 minutes, it's not forthcoming.
				tp->t_timer[TCPT_2MSL] = 5 * 60 * PR_SLOWHZ;
				 * MUST WORRY ABOUT ONE-WAY CONNECTIONS.
				 */
			}
			break;

	 	/*
		 * In CLOSING STATE in addition to the processing for
		 * the ESTABLISHED state if the ACK acknowledges our FIN
		 * then enter the TIME-WAIT state, otherwise ignore
		 * the segment.
		 */
		case TCPS_CLOSING:
			if (ourfinisacked) {
				tp->t_state = TCPS_TIME_WAIT;
				tcp_canceltimers(tp);
				tp->t_timer[TCPT_2MSL] = 2 * TCPTV_MSL;
				soisdisconnected(so);
			}
			break;

		/*
		 * The only thing that can arrive in  LAST_ACK state
		 * is an acknowledgment of our FIN.  If our FIN is now
		 * acknowledged, delete the TCB, enter the closed state
		 * and return.
		 */
		case TCPS_LAST_ACK:
			if (ourfinisacked)
				tp = tcp_close(tp);
			goto drop;

		/*
		 * In TIME_WAIT state the only thing that should arrive
		 * is a retransmission of the remote FIN.  Acknowledge
		 * it and restart the finack timer.
		 */
		case TCPS_TIME_WAIT:
			tp->t_timer[TCPT_2MSL] = 2 * TCPTV_MSL;
			goto dropafterack;
		}
#undef ourfinisacked
	}

step6:
	/*
	 * Update window information.
	 */
	if (SEQ_LT(tp->snd_wl1, ti->ti_seq) || tp->snd_wl1 == ti->ti_seq &&
	    (SEQ_LT(tp->snd_wl2, ti->ti_ack) ||
	     tp->snd_wl2 == ti->ti_ack && ti->ti_win > tp->snd_wnd)) {
		tp->snd_wnd = ti->ti_win;
		tp->snd_wl1 = ti->ti_seq;
		tp->snd_wl2 = ti->ti_ack;
	}

	/*
	 * Process segments with URG.
	 */
	if ((tiflags & TH_URG) && ti->ti_urp &&
	    TCPS_HAVERCVDFIN(tp->t_state) == 0) {
		/*
		 * This is a kludge, but if we receive accept
		 * random urgent pointers, we'll crash in
		 * soreceive.  It's hard to imagine someone
		 * actually wanting to send this much urgent data.
		 */
		if (ti->ti_urp + (unsigned) so->so_rcv.sb_cc > 32767) {
			ti->ti_urp = 0;			/* XXX */
			tiflags &= ~TH_URG;		/* XXX */
			ti->ti_flags &= ~TH_URG;	/* XXX */
			goto badurp;			/* XXX */
		}
		/*
		 * If this segment advances the known urgent pointer,
		 * then mark the data stream.  This should not happen
		 * in CLOSE_WAIT, CLOSING, LAST_ACK or TIME_WAIT STATES since
		 * a FIN has been received from the remote side. 
		 * In these states we ignore the URG.
		 */
		if (SEQ_GT(ti->ti_seq+ti->ti_urp, tp->rcv_up)) {
			tp->rcv_up = ti->ti_seq + ti->ti_urp;
			so->so_oobmark = so->so_rcv.sb_cc +
			    (tp->rcv_up - tp->rcv_nxt) - 1;
			if (so->so_oobmark == 0)
				so->so_state |= SS_RCVATMARK;
			sohasoutofband(so);
			tp->t_oobflags &= ~TCPOOB_HAVEDATA;
		}
		/*
		 * Remove out of band data so doesn't get presented to user.
		 * This can happen independent of advancing the URG pointer,
		 * but if two URG's are pending at once, some out-of-band
		 * data may creep in... ick.
		 */
		if (ti->ti_urp <= ti->ti_len)
			tcp_pulloutofband(so, ti);
	}
badurp:							/* XXX */

	/*
	 * Process the segment text, merging it into the TCP sequencing queue,
	 * and arranging for acknowledgment of receipt if necessary.
	 * This process logically involves adjusting tp->rcv_wnd as data
	 * is presented to the user (this happens in tcp_usrreq.c,
	 * case PRU_RCVD).  If a FIN has already been received on this
	 * connection then we just ignore the text.
	 */
	if ((ti->ti_len || (tiflags&TH_FIN)) &&
	    TCPS_HAVERCVDFIN(tp->t_state) == 0) {
		tiflags = tcp_reass(tp, ti);
		if (tcpnodelack == 0)
			tp->t_flags |= TF_DELACK;
		else
			tp->t_flags |= TF_ACKNOW;
	} else {
		m_freem(m);
		tiflags &= ~TH_FIN;
	}

	/*
	 * If FIN is received ACK the FIN and let the user know
	 * that the connection is closing.
	 */
	if (tiflags & TH_FIN) {
		if (TCPS_HAVERCVDFIN(tp->t_state) == 0) {
			socantrcvmore(so);
			tp->t_flags |= TF_ACKNOW;
			tp->rcv_nxt++;
		}
		switch (tp->t_state) {

	 	/*
		 * In SYN_RECEIVED and ESTABLISHED STATES
		 * enter the CLOSE_WAIT state.
		 */
		case TCPS_SYN_RECEIVED:
		case TCPS_ESTABLISHED:
			tp->t_state = TCPS_CLOSE_WAIT;
			break;

	 	/*
		 * If still in FIN_WAIT_1 STATE FIN has not been acked so
		 * enter the CLOSING state.
		 */
		case TCPS_FIN_WAIT_1:
			tp->t_state = TCPS_CLOSING;
			break;

	 	/*
		 * In FIN_WAIT_2 state enter the TIME_WAIT state,
		 * starting the time-wait timer, turning off the other 
		 * standard timers.
		 */
		case TCPS_FIN_WAIT_2:
			tp->t_state = TCPS_TIME_WAIT;
			tcp_canceltimers(tp);
			tp->t_timer[TCPT_2MSL] = 2 * TCPTV_MSL;
			soisdisconnected(so);
			break;

		/*
		 * In TIME_WAIT state restart the 2 MSL time_wait timer.
		 */
		case TCPS_TIME_WAIT:
			tp->t_timer[TCPT_2MSL] = 2 * TCPTV_MSL;
			break;
		}
	}
#ifdef	TCP_DEBUG
	if (so->so_options & SO_DEBUG)
		tcp_trace(TA_INPUT, ostate, tp, &tcp_saveti, 0);
#endif	TCP_DEBUG

	/*
	 * Return any desired output.
	 */
	(void) tcp_output(tp);
	return;

dropafterack:
	/*
	 * Generate an ACK dropping incoming segment if it occupies
	 * sequence space, where the ACK reflects our state.
	 */
	if ((tiflags&TH_RST) ||
	    tlen == 0 && (tiflags&(TH_SYN|TH_FIN)) == 0)
		goto drop;
#ifdef	TCP_DEBUG
	if (tp->t_inpcb->inp_socket->so_options & SO_DEBUG)
		tcp_trace(TA_RESPOND, ostate, tp, &tcp_saveti, 0);
#endif	TCP_DEBUG
	tcp_respond(tp, ti, tp->rcv_nxt, tp->snd_nxt, TH_ACK);
	return;

dropwithreset:
	if (om) {
		(void) m_free(om);
		om = 0;
	}
	/*
	 * Generate a RST, dropping incoming segment.
	 * Make ACK acceptable to originator of segment.
	 */
	if (tiflags & TH_RST)
		goto drop;
	if (tiflags & TH_ACK)
		tcp_respond(tp, ti, (tcp_seq)0, ti->ti_ack, TH_RST);
	else {
		if (tiflags & TH_SYN)
			ti->ti_len++;
		tcp_respond(tp, ti, ti->ti_seq+ti->ti_len, (tcp_seq)0,
		    TH_RST|TH_ACK);
	}
	/* destroy temporarily created socket */
	if (dropsocket)
		(void) soabort(so);
	return;

drop:
	if (om)
		(void) m_free(om);
	/*
	 * Drop space held by incoming segment and return.
	 */
#ifdef	TCP_DEBUG
	if (tp && (tp->t_inpcb->inp_socket->so_options & SO_DEBUG))
		tcp_trace(TA_DROP, ostate, tp, &tcp_saveti, 0);
#endif	TCP_DEBUG
	m_freem(m);
	/* destroy temporarily created socket */
	if (dropsocket)
		(void) soabort(so);
	return;
#ifdef	pdp11
#undef	return
finishup:
	restorseg5(map);
	return;
#endif	pdp11
}

static
tcp_dooptions(tp, om, ti)
	struct tcpcb *tp;
	struct mbuf *om;
	struct tcpiphdr *ti;
{
	register u_char *cp;
	int opt, optlen, cnt;

#ifdef pdp11
	MAPSAVE();
#endif
	cp = mtod(om, u_char *);
	cnt = om->m_len;
	for (; cnt > 0; cnt -= optlen, cp += optlen) {
		opt = cp[0];
		if (opt == TCPOPT_EOL)
			break;
		if (opt == TCPOPT_NOP)
			optlen = 1;
		else {
			optlen = cp[1];
			if (optlen <= 0)
				break;
		}
		switch (opt) {

		default:
			break;

		case TCPOPT_MAXSEG:
			if (optlen != 4)
				continue;
			if (!(ti->ti_flags & TH_SYN))
				continue;
			tp->t_maxseg = *(u_short *)(cp + 2);
			tp->t_maxseg = ntohs((u_short)tp->t_maxseg);
			tp->t_maxseg = MIN(tp->t_maxseg, tcp_mss(tp));
			break;
		}
	}
	(void) m_free(om);
#ifdef pdp11
	MAPREST();
#endif
}

/*
 * Pull out of band byte out of a segment so
 * it doesn't appear in the user's data queue.
 * It is still reflected in the segment length for
 * sequencing purposes.
 */
static
tcp_pulloutofband(so, ti)
	struct socket *so;
	struct tcpiphdr *ti;
{
	register struct mbuf *m;
	int cnt = ti->ti_urp - 1;
	
#ifdef pdp11
	MAPSAVE();
#endif
	m = dtom(ti);
	while (cnt >= 0) {
		if (m->m_len > cnt) {
			char *cp = mtod(m, caddr_t) + cnt;
			struct tcpcb *tp = sototcpcb(so);

			tp->t_iobc = *cp;
			tp->t_oobflags |= TCPOOB_HAVEDATA;
			bcopy(cp+1, cp, (unsigned)(m->m_len - cnt - 1));
			m->m_len--;
#ifdef	pdp11
			goto out;
#else	vax
			return;
#endif
		}
		cnt -= m->m_len;
		m = m->m_next;
		if (m == 0)
			break;
	}
	panic("tcp_pulloutofband");
#ifdef	pdp11
out:
	MAPREST();
#endif
}

#ifdef	pdp11
#define	ti_mbuf ti_sum
#define	DTOM(d) ( (struct mbuf *) ((d)->ti_mbuf) )
#define	INSQUE(i,p) { \
	struct tcpiphdr *tii; \
	MSGET(tii, struct tcpiphdr, MT_HEADER, 0, M_DONTWAIT); if (tii == 0) goto drop; \
	insque(tii, p); \
	tii->ti_len = (i)->ti_len; tii->ti_seq = (i)->ti_seq; \
	tii->ti_flags = (i)->ti_flags; tii->ti_mbuf = m0; i = tii; }
#endif

/*
 * Insert segment ti into reassembly queue of tcp with
 * control block tp.  Return TH_FIN if reassembly now includes
 * a segment with FIN.
 */
static
tcp_reass(tp, ti)
	register struct tcpcb *tp;
	register struct tcpiphdr *ti;
{
	register struct tcpiphdr *q;
	struct socket *so = tp->t_inpcb->inp_socket;
	struct mbuf *m;
	int flags;
#ifdef	pdp11
	struct mbuf *m0;
	register struct tcpiphdr *qp;
#endif	pdp11

	/*
	 * Call with ti==0 after become established to
	 * force pre-ESTABLISHED data up to user socket.
	 */
	if (ti == 0)
		goto present;
#ifdef	pdp11
	m0 = dtom(ti);
#endif	pdp11

	/*
	 * Find a segment which begins after this one does.
	 */
	for (q = tp->seg_next; q != (struct tcpiphdr *)tp;
	    q = (struct tcpiphdr *)q->ti_next)
		if (SEQ_GT(q->ti_seq, ti->ti_seq))
			break;

	/*
	 * If there is a preceding segment, it may provide some of
	 * our data already.  If so, drop the data from the incoming
	 * segment.  If it provides all of our data, drop us.
	 */
	if ((struct tcpiphdr *)q->ti_prev != (struct tcpiphdr *)tp) {
		register int i;
		q = (struct tcpiphdr *)q->ti_prev;
		/* conversion to int (in i) handles seq wraparound */
		i = q->ti_seq + q->ti_len - ti->ti_seq;
		if (i > 0) {
			if (i >= ti->ti_len)
				goto drop;
#ifndef	pdp11
			m_adj(dtom(ti), i);
#else	pdp11
			m_adj(m0, i);
#endif	pdp11
			ti->ti_len -= i;
			ti->ti_seq += i;
		}
		q = (struct tcpiphdr *)(q->ti_next);
	}

	/*
	 * While we overlap succeeding segments trim them or,
	 * if they are completely covered, dequeue them.
	 */
	while (q != (struct tcpiphdr *)tp) {
		register int i = (ti->ti_seq + ti->ti_len) - q->ti_seq;
		if (i <= 0)
			break;
		if (i < q->ti_len) {
			q->ti_seq += i;
			q->ti_len -= i;
#ifndef	pdp11
			m_adj(dtom(q), i);
#else	pdp11
			m_adj(DTOM(q), i);
#endif	pdp11
			break;
		}
#ifndef	pdp11
		q = (struct tcpiphdr *)q->ti_next;
		m = dtom(q->ti_prev);
		remque(q->ti_prev);
#else	pdp11
		qp = q;
		q = (struct tcpiphdr *)q->ti_next;
		m = DTOM(qp);
		remque(qp);
		MSFREE(qp);
#endif	pdp11
		m_freem(m);
	}

	/*
	 * Stick new segment in its place.
	 */
#ifndef	pdp11
	insque(ti, q->ti_prev);
#else	pdp11
	INSQUE(ti, q->ti_prev);
#endif	pdp11

present:
	/*
	 * Present data to user, advancing rcv_nxt through
	 * completed sequence space.
	 */
	if (TCPS_HAVERCVDSYN(tp->t_state) == 0)
		return (0);
	ti = tp->seg_next;
	if (ti == (struct tcpiphdr *)tp || ti->ti_seq != tp->rcv_nxt)
		return (0);
	if (tp->t_state == TCPS_SYN_RECEIVED && ti->ti_len)
		return (0);
	do {
		tp->rcv_nxt += ti->ti_len;
		flags = ti->ti_flags & TH_FIN;
		remque(ti);
#ifndef	pdp11
		m = dtom(ti);
#else	pdp11
		m = DTOM(ti);
		qp = ti;
#endif	pdp11
		ti = (struct tcpiphdr *)ti->ti_next;
#ifdef	pdp11
		MSFREE(qp);
#endif	pdp11
		if (so->so_state & SS_CANTRCVMORE)
			m_freem(m);
		else
			sbappend(&so->so_rcv, m);
	} while (ti != (struct tcpiphdr *)tp && ti->ti_seq == tp->rcv_nxt);
	sorwakeup(so);
	return (flags);
drop:
#ifndef	pdp11
	m_freem(dtom(ti));
#else	pdp11
	m_freem(m0);
#endif	pdp11
	return (0);
}

/*
 *  Determine a reasonable value for maxseg size.
 *  If the route is known, use one that can be handled
 *  on the given interface without forcing IP to fragment.
 *  If bigger than a page (CLSIZE), round down to nearest pagesize
 *  to utilize pagesize mbufs.
 *  If interface pointer is unavailable, or the destination isn't local,
 *  use a conservative size (512 or the default IP max size),
 *  as we can't discover anything about intervening gateways or networks.
 *
 *  This is ugly, and doesn't belong at this level, but has to happen somehow.
 */
tcp_mss(tp)
register struct tcpcb *tp;
{
	struct route *ro;
	struct ifnet *ifp;
	int mss;
	struct inpcb *inp;

	inp = tp->t_inpcb;
	ro = &inp->inp_route;
	if ((ro->ro_rt == (struct rtentry *)0) ||
	    (ifp = ro->ro_rt->rt_ifp) == (struct ifnet *)0) {
		/* No route yet, so try to acquire one */
		if (inp->inp_faddr.s_addr != INADDR_ANY) {
			ro->ro_dst.sa_family = AF_INET;
			((struct sockaddr_in *) &ro->ro_dst)->sin_addr =
				inp->inp_faddr;
			rtalloc(ro);
		}
		if ((ro->ro_rt == 0) || (ifp = ro->ro_rt->rt_ifp) == 0)
			return (TCP_MSS);
	}

	mss = ifp->if_mtu - sizeof(struct tcpiphdr);
#if	(CLBYTES & (CLBYTES - 1)) == 0
	if (mss > CLBYTES)
		mss &= ~(CLBYTES-1);
#else
	if (mss > CLBYTES)
		mss = mss / CLBYTES * CLBYTES;
#endif
	if (in_localaddr(tp->t_inpcb->inp_faddr))
		return(mss);
	return (MIN(mss, TCP_MSS));
}
