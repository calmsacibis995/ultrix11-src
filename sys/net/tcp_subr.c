
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
 * SCCSID: @(#)tcp_subr.c	3.0	(ULTRIX-11)	4/21/86
 *	@(#)tcp_subr.c	6.6 (Berkeley) 6/8/85
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/protosw.h>
#include <errno.h>

#include <net/route.h>
#include <net/if.h>

#include <netinet/in.h>
#include <netinet/in_pcb.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>
#include <netinet/tcp_fsm.h>
#include <netinet/tcp_seq.h>
#include <netinet/tcp_timer.h>
#include <netinet/tcp_var.h>
#include <netinet/tcpip.h>

/*
 * Tcp initialization
 */
tcp_init()
{

	tcp_iss = 1;		/* wrong */
	tcb.inp_next = tcb.inp_prev = &tcb;
	tcp_alpha = TCP_ALPHA;
	tcp_beta = TCP_BETA;
}

/*
 * Create template to be used to send tcp packets on a connection.
 * Call after host entry created, allocates an mbuf and fills
 * in a skeletal tcp/ip header, minimizing the amount of work
 * necessary when the connection is used.
 */
struct tcpiphdr *
tcp_template(tp)
	struct tcpcb *tp;
{
	register struct inpcb *inp = tp->t_inpcb;
#ifdef	vax
	register struct mbuf *m;
#endif	vax
	register struct tcpiphdr *n;

#ifdef	vax
	m = m_get(M_WAIT, MT_HEADER);
	if (m == NULL)
		return (0);
	m->m_off = MMAXOFF - sizeof (struct tcpiphdr);
	m->m_len = sizeof (struct tcpiphdr);
	n = mtod(m, struct tcpiphdr *);
#else	pdp11
	MSGET(n, struct tcpiphdr, MT_HEADER, 1, M_DONTWAIT);
	if (n == NULL)
		return (0);
#endif
	n->ti_next = n->ti_prev = 0;
#ifdef	pdp11
	n->ti_pad = 0;
#endif	pdp11
	n->ti_x1 = 0;
	n->ti_pr = IPPROTO_TCP;
	n->ti_len = htons(sizeof (struct tcpiphdr) - sizeof (struct ip));
	n->ti_src = inp->inp_laddr;
	n->ti_dst = inp->inp_faddr;
	n->ti_sport = inp->inp_lport;
	n->ti_dport = inp->inp_fport;
	n->ti_seq = 0;
	n->ti_ack = 0;
	n->ti_x2 = 0;
	n->ti_off = 5;
	n->ti_flags = 0;
	n->ti_win = 0;
	n->ti_sum = 0;
	n->ti_urp = 0;
	return (n);
}

/*
 * Send a single message to the TCP at address specified by
 * the given TCP/IP header.  If flags==0, then we make a copy
 * of the tcpiphdr at ti and send directly to the addressed host.
 * This is used to force keep alive messages out using the TCP
 * template for a connection tp->t_template.  If flags are given
 * then we send a message back to the TCP which originated the
 * segment ti, and discard the mbuf containing it and any other
 * attached mbufs.
 *
 * In any case the ack and sequence number of the transmitted
 * segment are as specified by the parameters.
 */
tcp_respond(tp, ti, ack, seq, flags)
	struct tcpcb *tp;
	register struct tcpiphdr *ti;
	tcp_seq ack, seq;
	int flags;
{
	struct mbuf *m;
	int win = 0, tlen;
	struct route *ro = 0;

	if (tp) {
		win = sbspace(&tp->t_inpcb->inp_socket->so_rcv);
		ro = &tp->t_inpcb->inp_route;
	}
	if (flags == 0) {
		m = m_get(M_DONTWAIT, MT_HEADER);
		if (m == NULL)
			return;
		m->m_len = sizeof (struct tcpiphdr) + 1;
		*mtod(m, struct tcpiphdr *) = *ti;
		ti = mtod(m, struct tcpiphdr *);
		flags = TH_ACK;
		tlen = 1;
	} else {
		m = dtom(ti);
		m_freem(m->m_next);
		m->m_next = 0;
#ifndef	pdp11
		m->m_off = (int)ti - (int)m;
#else	pdp11
		m->m_off = (caddr_t)ti - (caddr_t)MBX;
#endif	pdp11
		m->m_len = sizeof (struct tcpiphdr);
#define xchg(a,b,type) { type t; t=a; a=b; b=t; }
		xchg(ti->ti_dst.s_addr, ti->ti_src.s_addr, u_long);
		xchg(ti->ti_dport, ti->ti_sport, u_short);
#undef xchg
		tlen = 0;
	}
	ti->ti_next = ti->ti_prev = 0;
#ifdef	pdp11
	ti->ti_pad = 0;
#endif	pdp11
	ti->ti_x1 = 0;
	ti->ti_len = htons((u_short)(sizeof (struct tcphdr) + tlen));
	ti->ti_seq = htonl(seq);
	ti->ti_ack = htonl(ack);
	ti->ti_x2 = 0;
	ti->ti_off = sizeof (struct tcphdr) >> 2;
	ti->ti_flags = flags;
	ti->ti_win = htons((u_short)win);
	ti->ti_urp = 0;
#ifdef	TCPLOOPBACK
	if (ti->ti_src.s_addr == ti->ti_dst.s_addr) {
		/* Fake IP transit and get to TCP layer fast */
		((struct ip *)ti)->ip_hl = sizeof (struct ip) >> 2;
		((struct ip *)ti)->ip_len = sizeof (struct tcphdr) + tlen;
		tcp_input(m);
	} else {
#endif	TCPLOOPBACK
		ti->ti_sum = in_cksum(m, sizeof (struct tcpiphdr) + tlen);
		((struct ip *)ti)->ip_len = sizeof (struct tcpiphdr) + tlen;
		((struct ip *)ti)->ip_ttl = TCP_TTL;
		(void) ip_output(m, (struct mbuf *)0, ro, 0);
#ifdef	TCPLOOPBACK
	}
#endif	TCPLOOPBACK
}

/*
 * Create a new TCP control block, making an
 * empty reassembly queue and hooking it to the argument
 * protocol control block.
 */
struct tcpcb *
tcp_newtcpcb(inp)
	struct inpcb *inp;
{
#ifdef	vax
	struct mbuf *m = m_getclr(M_DONTWAIT, MT_PCB);
#endif	vax
	register struct tcpcb *tp;

#ifdef	vax
	if (m == NULL)
		return ((struct tcpcb *)0);
	tp = mtod(m, struct tcpcb *);
#else	pdp11
	MSGET(tp, struct tcpcb, MT_PCB, 1, M_DONTWAIT);
	if (tp == NULL)
		return ((struct tcpcb *)0);
#endif
	tp->seg_next = tp->seg_prev = (struct tcpiphdr *)tp;
	tp->t_maxseg = TCP_MSS;
	tp->t_flags = 0;		/* sends options! */
	tp->t_inpcb = inp;
	tp->t_srtt = TCPTV_SRTTBASE;
	tp->snd_cwnd = sbspace(&inp->inp_socket->so_snd);
	inp->inp_ppcb = (caddr_t)tp;
	return (tp);
}

/*
 * Drop a TCP connection, reporting
 * the specified error.  If connection is synchronized,
 * then send a RST to peer.
 */
struct tcpcb *
tcp_drop(tp, errno)
	register struct tcpcb *tp;
	int errno;
{
	struct socket *so = tp->t_inpcb->inp_socket;

	if (TCPS_HAVERCVDSYN(tp->t_state)) {
		tp->t_state = TCPS_CLOSED;
		(void) tcp_output(tp);
	}
	so->so_error = errno;
	return (tcp_close(tp));
}

tcp_abort(inp)
	struct inpcb *inp;
{

	(void) tcp_close((struct tcpcb *)inp->inp_ppcb);
}

#ifdef	pdp11
#define	ti_mbuf ti_sum
#define	DTOM(d) ((struct mbuf *)((d)->ti_mbuf))
#endif	pdp11
/*
 * Close a TCP control block:
 *	discard all space held by the tcp
 *	discard internet protocol block
 *	wake up any sleepers
 */
struct tcpcb *
tcp_close(tp)
	register struct tcpcb *tp;
{
	register struct tcpiphdr *t;
	struct inpcb *inp = tp->t_inpcb;
	struct socket *so = inp->inp_socket;
#ifndef	pdp11
	register struct mbuf *m;
#else	pdp11
	register struct tcpiphdr *to;
#endif	pdp11

	t = tp->seg_next;
	while (t != (struct tcpiphdr *)tp) {
#ifdef	pdp11
		to = t;
		t = (struct tcpiphdr *)t->ti_next;
		remque(to);
		m_freem(DTOM(to));
		MSFREE(to);
#else
		t = (struct tcpiphdr *)t->ti_next;
		m = dtom(t->ti_prev);
		remque(t->ti_prev);
		m_freem(m);
#endif	pdp11
	}
	if (tp->t_template)
#ifdef	vax
		(void) m_free(dtom(tp->t_template));
#else	pdp11
		MSFREE(tp->t_template);
#endif
	if (tp->t_tcpopt)
		(void) m_free(dtom(tp->t_tcpopt));
	if (tp->t_ipopt)
		(void) m_free(dtom(tp->t_ipopt));
#ifndef	pdp11
	(void) m_free(dtom(tp));
#else	pdp11
	MSFREE(tp);
#endif	pdp11
	inp->inp_ppcb = 0;
	soisdisconnected(so);
	in_pcbdetach(inp);
	return ((struct tcpcb *)0);
}

tcp_drain()
{

}

tcp_ctlinput(cmd, arg)
	int cmd;
	caddr_t arg;
{
	struct in_addr *in;
	extern u_char inetctlerrmap[];
	int tcp_quench(), in_rtchange();

	if (cmd < 0 || cmd > PRC_NCMDS)
		return;
	switch (cmd) {

	case PRC_ROUTEDEAD:
		break;

	case PRC_QUENCH:
		in = &((struct icmp *)arg)->icmp_ip.ip_dst;
		in_pcbnotify(&tcb, in, 0, tcp_quench);
		break;

	case PRC_REDIRECT_NET:
	case PRC_REDIRECT_HOST:
		in = &((struct icmp *)arg)->icmp_ip.ip_dst;
		in_pcbnotify(&tcb, in, 0, in_rtchange);
		break;

	case PRC_IFDOWN:
		in = &((struct sockaddr_in *)arg)->sin_addr;
		goto notify;

	case PRC_HOSTDEAD:
	case PRC_HOSTUNREACH:
		in = (struct in_addr *)arg;
		goto notify;

	default:
		if (inetctlerrmap[cmd] == 0)
			return;		/* XXX */
		in = &((struct icmp *)arg)->icmp_ip.ip_dst;
notify:
		in_pcbnotify(&tcb, in, (int)inetctlerrmap[cmd], tcp_abort);
	}
}

/*
 * When a source quench is received, close congestion window
 * to 80% of the outstanding data (but not less than one segment).
 */
tcp_quench(inp)
	struct inpcb *inp;
{
	struct tcpcb *tp = intotcpcb(inp);

#ifdef	vax
	tp->snd_cwnd = MAX(8 * (tp->snd_nxt - tp->snd_una) / 10, tp->t_maxseg);
#else	pdp11
	register unsigned x1;

	x1 = (tp->snd_nxt - tp->snd_una)/10;
	/* make sure x1 not 0 -- Fred 2/20/86 */
	if(x1 == 0)
		x1++;
	x1 *= 8;
	tp->snd_cwnd = MAX(x1, tp->t_maxseg);
#endif
}
