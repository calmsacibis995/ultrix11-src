
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
 * SCCSID: @(#)uipc_socket.c	3.0	5/5/86
 *	@(#)uipc_socket.c	6.15 (Berkeley) 6/8/85
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/file.h>
#include <sys/inode.h>
#include <sys/buf.h>
#include <sys/mbuf.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <net/if.h>
#ifdef	UNIX_DOMAIN
#include <sys/un.h>
#include <sys/domain.h>
#endif	UNIX_DOMAIN
#ifdef	vax
#include "uio.h"
#endif

/*
 * Socket operation routines.
 * These routines are called by the routines in
 * sys_socket.c or from a system process, and
 * implement the semantics of socket operations by
 * switching out to the protocol specific routines.
 *
 * TODO:
 *	test socketpair
 *	clean up async
 */
/*ARGSUSED*/
socreate(dom, aso, type, proto)
	struct socket **aso;
	register int type;
	int proto;
{
	register struct protosw *prp;
	register struct socket *so;
#ifndef	pdp11
	register struct mbuf *m;
#endif	pdp11
	register int error;

	if (proto)
		prp = pffindproto(dom, proto, type);
	else
		prp = pffindtype(dom, type);
	if (prp == 0)
		return (EPROTONOSUPPORT);
	if (prp->pr_type != type)
		return (EPROTOTYPE);
#ifndef	pdp11
	m = m_getclr(M_WAIT, MT_SOCKET);
	so = mtod(m, struct socket *);
#else	pdp11
	MSGET(so, struct socket, MT_SOCKET, 1, M_DONTWAIT);
	if (so == NULL)
		return(ENOBUFS);
#endif	pdp11
	so->so_options = 0;
	so->so_state = 0;
	so->so_type = type;
	if (u.u_uid == 0)
		so->so_state = SS_PRIV;
	so->so_proto = prp;
	error =
	    (*prp->pr_usrreq)(so, PRU_ATTACH,
		(struct mbuf *)0, (struct mbuf *)proto, (struct mbuf *)0);
	if (error) {
		so->so_state |= SS_NOFDREF;
		sofree(so);
		return (error);
	}
	*aso = so;
	return (0);
}

sobind(so, nam)
	struct socket *so;
	struct mbuf *nam;
{
	int s = splnet();
	int error;

	error =
	    (*so->so_proto->pr_usrreq)(so, PRU_BIND,
		(struct mbuf *)0, nam, (struct mbuf *)0);
	splx(s);
	return (error);
}

solisten(so, backlog)
	register struct socket *so;
	int backlog;
{
	int s = splnet(), error;

	error =
	    (*so->so_proto->pr_usrreq)(so, PRU_LISTEN,
		(struct mbuf *)0, (struct mbuf *)0, (struct mbuf *)0);
	if (error) {
		splx(s);
		return (error);
	}
	if (so->so_q == 0) {
		so->so_q = so;
		so->so_q0 = so;
		so->so_options |= SO_ACCEPTCONN;
	}
	if (backlog < 0)
		backlog = 0;
	so->so_qlimit = MIN(backlog, SOMAXCONN);
	splx(s);
	return (0);
}

sofree(so)
	register struct socket *so;
{

	if (so->so_head) {
		if (!soqremque(so, 0) && !soqremque(so, 1))
			panic("sofree dq");
		so->so_head = 0;
	}
	if (so->so_pcb || (so->so_state & SS_NOFDREF) == 0)
		return;
	sbrelease(&so->so_snd);
	sorflush(so);
#ifndef	pdp11
	(void) m_free(dtom(so));
#else	pdp11
	MSFREE(so);
#endif	pdp11
}

/*
 * Close a socket on last file table reference removal.
 * Initiate disconnect if connected.
 * Free socket when disconnect complete.
 */
soclose(so)
	register struct socket *so;
{
	int s = splnet();		/* conservative */
	int error;

	if (so->so_options & SO_ACCEPTCONN) {
		while (so->so_q0 != so)
			(void) soabort(so->so_q0);
		while (so->so_q != so)
			(void) soabort(so->so_q);
	}
	if (so->so_pcb == 0)
		goto discard;
	if (so->so_state & SS_ISCONNECTED) {
		if ((so->so_state & SS_ISDISCONNECTING) == 0) {
			error = sodisconnect(so, (struct mbuf *)0);
			if (error)
				goto drop;
		}
		if (so->so_options & SO_LINGER) {
			if ((so->so_state & SS_ISDISCONNECTING) &&
			    (so->so_state & SS_NBIO))
				goto drop;
			while (so->so_state & SS_ISCONNECTED)
				sleep((caddr_t)&so->so_timeo, PZERO+1);
		}
	}
drop:
	if (so->so_pcb) {
		int error2 =
		    (*so->so_proto->pr_usrreq)(so, PRU_DETACH,
			(struct mbuf *)0, (struct mbuf *)0, (struct mbuf *)0);
		if (error == 0)
			error = error2;
	}
discard:
	if (so->so_state & SS_NOFDREF)
		panic("soclose: NOFDREF");
	so->so_state |= SS_NOFDREF;
	sofree(so);
	splx(s);
	return (error);
}

/*
 * Must be called at splnet...
 */
soabort(so)
	struct socket *so;
{

	return (
	    (*so->so_proto->pr_usrreq)(so, PRU_ABORT,
		(struct mbuf *)0, (struct mbuf *)0, (struct mbuf *)0));
}

soaccept(so, nam)
	register struct socket *so;
	struct mbuf *nam;
{
	int s = splnet();
	int error;

	if ((so->so_state & SS_NOFDREF) == 0)
		panic("soaccept: !NOFDREF");
	so->so_state &= ~SS_NOFDREF;
	error = (*so->so_proto->pr_usrreq)(so, PRU_ACCEPT,
	    (struct mbuf *)0, nam, (struct mbuf *)0);
	splx(s);
	return (error);
}

soconnect(so, nam)
	register struct socket *so;
	struct mbuf *nam;
{
	int s = splnet();
	int error;

	if (so->so_state & (SS_ISCONNECTED|SS_ISCONNECTING)) {
		error = EISCONN;
		goto bad;
	}
	error = (*so->so_proto->pr_usrreq)(so, PRU_CONNECT,
	    (struct mbuf *)0, nam, (struct mbuf *)0);
bad:
	splx(s);
	return (error);
}

soconnect2(so1, so2)
	register struct socket *so1;
	struct socket *so2;
{
	int s = splnet();
	int error;

	error = (*so1->so_proto->pr_usrreq)(so1, PRU_CONNECT2,
	    (struct mbuf *)0, (struct mbuf *)so2, (struct mbuf *)0);
	splx(s);
	return (error);
}

static
sodisconnect(so, nam)
	register struct socket *so;
	struct mbuf *nam;
{
	int s = splnet();
	int error;

	if ((so->so_state & SS_ISCONNECTED) == 0) {
		error = ENOTCONN;
		goto bad;
	}
	if (so->so_state & SS_ISDISCONNECTING) {
		error = EALREADY;
		goto bad;
	}
	error = (*so->so_proto->pr_usrreq)(so, PRU_DISCONNECT,
	    (struct mbuf *)0, nam, (struct mbuf *)0);
bad:
	splx(s);
	return (error);
}

/*
 * Send on a socket.
 * If send must go all at once and message is larger than
 * send buffering, then hard error.
 * Lock against other senders.
 * If must go all at once and not enough room now, then
 * inform user that this would block and do nothing.
 * Otherwise, if nonblocking, send as much as possible.
 */
#ifndef	pdp11
sosend(so, nam, uio, flags, rights)
#else	pdp11
sosend(so, nam, flags, rights)
#endif	pdp11
	register struct socket *so;
	struct mbuf *nam;
#ifndef	pdp11
	register struct uio *uio;
#endif	pdp11
	int flags;
	struct mbuf *rights;
{
	struct mbuf *top = 0;
	register struct mbuf *m, **mp;
	register int space;
	int len, error = 0, s, dontroute, first = 1;

#ifndef	pdp11
	if (sosendallatonce(so) && uio->uio_resid > so->so_snd.sb_hiwat)
		return (EMSGSIZE);
#else	pdp11
	if (sosendallatonce(so) && u.u_count > so->so_snd.sb_hiwat)
		return (EMSGSIZE);
#endif	pdp11
	dontroute =
	    (flags & MSG_DONTROUTE) && (so->so_options & SO_DONTROUTE) == 0 &&
	    (so->so_proto->pr_flags & PR_ATOMIC);
#ifndef	pdp11
	u.u_ru.ru_msgsnd++;
#endif
#define	snderr(errno)	{ error = errno; splx(s); goto release; }

restart:
	sblock(&so->so_snd);
	do {
		s = splnet();
		if (so->so_state & SS_CANTSENDMORE)
			snderr(EPIPE);
		if (so->so_error) {
			error = so->so_error;
			so->so_error = 0;			/* ??? */
			splx(s);
			goto release;
		}
		if ((so->so_state & SS_ISCONNECTED) == 0) {
			if (so->so_proto->pr_flags & PR_CONNREQUIRED)
				snderr(ENOTCONN);
			if (nam == 0)
				snderr(EDESTADDRREQ);
		}
		if (flags & MSG_OOB)
			space = 1024;
		else {
			space = sbspace(&so->so_snd);
			if (space <= 0 ||
#ifndef	pdp11
			   (sosendallatonce(so) && space < uio->uio_resid) ||
			   (uio->uio_resid >= CLBYTES && space < CLBYTES &&
#else	pdp11
			   (sosendallatonce(so) && space < u.u_count) ||
			   (u.u_count >= CLBYTES && space < CLBYTES &&
#endif	pdp11
			   so->so_snd.sb_cc >= CLBYTES &&
			   (so->so_state & SS_NBIO) == 0)) {
				if (so->so_state & SS_NBIO) {
					if (first)
						error = EWOULDBLOCK;
					splx(s);
					goto release;
				}
				sbunlock(&so->so_snd);
				sbwait(&so->so_snd);
				splx(s);
				goto restart;
			}
		}
		splx(s);
		mp = &top;
		while (space > 0) {
#ifndef	pdp11
			register struct iovec *iov = uio->uio_iov;

			MGET(m, M_WAIT, MT_DATA);
			if (iov->iov_len >= NBPG && space >= CLBYTES) {
				register struct mbuf *p;
				MCLGET(p, 1);
				if (p == 0)
					goto nopages;
				m->m_off = (int)p - (int)m;
				len = min(CLBYTES, iov->iov_len);
				space -= CLBYTES;
			} else {
nopages:
				len = MIN(MLEN, iov->iov_len);
				space -= len;
			}
			error = uiomove(mtod(m, caddr_t), len, UIO_WRITE, uio);
#else	pdp11
			MGET(m, M_DONTWAIT, MT_DATA);
			if (m == NULL) {
				error = ENOBUFS;
				goto release;
			}
			m->m_off = MMINOFF;
			len = MIN(MLEN, u.u_count);
			pimove(ctob((long)m->m_click) + m->m_off, len, B_WRITE);
			normalseg5();	/* mtod() remapped us */
			if (error = u.u_error)
				u.u_error = 0;
#endif	pdp11
			m->m_len = len;
			*mp = m;
			if (error)
				goto release;
			mp = &m->m_next;
#ifdef	vax
			if (uio->uio_resid <= 0)
#else	pdp11
			if (u.u_count <= 0)
#endif
				break;
#ifdef vax
			while (uio->uio_iov->iov_len == 0) {
				uio->uio_iov++;
				uio->uio_iovcnt--;
				if (uio->uio_iovcnt <= 0)
					panic("sosend");
			}
#endif vax
		}
		if (dontroute)
			so->so_options |= SO_DONTROUTE;
		s = splnet();					/* XXX */
		error = (*so->so_proto->pr_usrreq)(so,
		    (flags & MSG_OOB) ? PRU_SENDOOB : PRU_SEND,
		    top, (caddr_t)nam, rights);
		splx(s);
		if (dontroute)
			so->so_options &= ~SO_DONTROUTE;
		rights = 0;
		top = 0;
		first = 0;
		if (error)
			break;
	}
#ifndef	pdp11
		while (uio->uio_resid);
#else	pdp11
		while (u.u_count);
#endif	pdp11

release:
	sbunlock(&so->so_snd);
	if (top)
		m_freem(top);
	if (error == EPIPE)
		psignal(u.u_procp, SIGPIPE);
	return (error);
}

#ifndef	pdp11
soreceive(so, aname, uio, flags, rightsp)
#else	pdp11
soreceive(so, aname, flags, rightsp)
#endif	pdp11
	register struct socket *so;
	struct mbuf **aname;
#ifndef	pdp11
	register struct uio *uio;
#endif	pdp11
	int flags;
	struct mbuf **rightsp;
{
	register struct mbuf *m, *n;
	register int len, error = 0, s, tomark;
	struct protosw *pr = so->so_proto;
	struct mbuf *nextrecord;
	int moff;

	if (rightsp)
		*rightsp = 0;
	if (aname)
		*aname = 0;
	if (flags & MSG_OOB) {
#ifndef	pdp11
		m = m_get(M_WAIT, MT_DATA);
#else	pdp11
		m = m_get(M_DONTWAIT, MT_DATA);
		if (m == NULL)
			return(ENOBUFS);
#endif
		error = (*pr->pr_usrreq)(so, PRU_RCVOOB,
		    m, (struct mbuf *)0, (struct mbuf *)0);
		if (error)
			goto bad;
#ifndef	pdp11
		do {
			len = uio->uio_resid;
			if (len > m->m_len)
				len = m->m_len;
			error =
			    uiomove(mtod(m, caddr_t), (int)len, UIO_READ, uio);
			m = m_free(m);
		} while (uio->uio_resid && error == 0 && m);
#else	pdp11
		do {
			len = u.u_count;
			if (len > m->m_len)
				len = m->m_len;
			pimove(ctob((long)m->m_click) + m->m_off, len, B_READ);
			m = m_free(m);
		} while (u.u_count && u.u_error == 0 && m);
		normalseg5(); /* mtod above remapped us */
		error = u.u_error;
		u.u_error = 0;
#endif	pdp11
bad:
		if (m)
			m_freem(m);
		return (error);
	}

restart:
	sblock(&so->so_rcv);
	s = splnet();

#define	rcverr(errno)	{ error = errno; splx(s); goto release; }
	if (so->so_rcv.sb_cc == 0) {
		if (so->so_error) {
			error = so->so_error;
			so->so_error = 0;
			splx(s);
			goto release;
		}
		if (so->so_state & SS_CANTRCVMORE) {
			splx(s);
			goto release;
		}
		if ((so->so_state & SS_ISCONNECTED) == 0 &&
		    (so->so_proto->pr_flags & PR_CONNREQUIRED))
			rcverr(ENOTCONN);
		if (so->so_state & SS_NBIO)
			rcverr(EWOULDBLOCK);
		sbunlock(&so->so_rcv);
		sbwait(&so->so_rcv);
		splx(s);
		goto restart;
	}
#ifndef	pdp11
	u.u_ru.ru_msgrcv++;
#endif	pdp11
	m = so->so_rcv.sb_mb;
	if (pr->pr_flags & PR_ADDR) {
#ifndef	pdp11
		if (m == 0 || m->m_type != MT_SONAME)
			panic("receive 1a");
#else
		if (m == 0)
			panic("receive 1a");
		else {
			mapseg5(m->m_click, MBMAPSIZE);
			if (MBX->m_type != MT_SONAME)
				panic("receive 1a");
			normalseg5();
		}
#endif
		if (flags & MSG_PEEK) {
			if (aname)
				*aname = m_copy(m, 0, m->m_len);
			else
				m = m->m_act;
		} else {
			if (aname) {
				*aname = m;
				sbfree(&so->so_rcv, m);
if(m->m_next) panic("receive 1b");
				so->so_rcv.sb_mb = m = m->m_act;
			} else
				m = sbdroprecord(&so->so_rcv);
		}
	}
#ifndef	pdp11
	if (m && m->m_type == MT_RIGHTS) {
#else	pdp11
	if (m) {
	    int t;
	    mapseg5(m->m_click, MBMAPSIZE);
	    t = MBX->m_type;
	    normalseg5();
	    if (t == MT_RIGHTS) {
#endif	pdp11
		if ((pr->pr_flags & PR_RIGHTS) == 0)
			panic("receive 2a");
		if (flags & MSG_PEEK) {
			if (rightsp)
				*rightsp = m_copy(m, 0, m->m_len);
			else
				m = m->m_act;
		} else {
			if (rightsp) {
				*rightsp = m;
				sbfree(&so->so_rcv, m);
if(m->m_next) panic("receive 2b");
				so->so_rcv.sb_mb = m = m->m_act;
			} else
				m = sbdroprecord(&so->so_rcv);
		}
#ifdef	pdp11
	    }
#endif	pdp11
	}
#ifndef	pdp11
	if (m == 0 || (m->m_type != MT_DATA && m->m_type != MT_HEADER))
		panic("receive 3");
#else	pdp11
	if (m == 0)
		panic("receive 3");
	else {
		mapseg5(m->m_click, MBMAPSIZE);
		if (MBX->m_type != MT_DATA && MBX->m_type != MT_HEADER)
			panic("receive 3");
		normalseg5();
	}
#endif	pdp11
	moff = 0;
	tomark = so->so_oobmark;
#ifndef	pdp11
	while (m && uio->uio_resid > 0 && error == 0) {
		len = uio->uio_resid;
#else	pdp11
	while (m && u.u_count > 0 && error == 0) {
		len = u.u_count;
#endif
		so->so_state &= ~SS_RCVATMARK;
		if (tomark && len > tomark)
			len = tomark;
		if (len > m->m_len - moff)
			len = m->m_len - moff;
		splx(s);
#ifndef	pdp11
		error =
		    uiomove(mtod(m, caddr_t) + moff, (int)len, UIO_READ, uio);
#else	pdp11
		pimove(ctob((long)m->m_click) + m->m_off + moff, len, B_READ);
		normalseg5();	/* mtod() remapped us */
		error = u.u_error;
		u.u_error = 0;
#endif	pdp11
		s = splnet();
		if (len == m->m_len - moff) {
			if ((flags & MSG_PEEK) == 0) {
				nextrecord = m->m_act;
				sbfree(&so->so_rcv, m);
				MFREE(m, n);
				if (m = n)
					m->m_act = nextrecord;
				so->so_rcv.sb_mb = m;
			} else
				m = m->m_next;
			moff = 0;
		} else {
			if (flags & MSG_PEEK)
				moff += len;
			else {
				m->m_off += len;
				m->m_len -= len;
				so->so_rcv.sb_cc -= len;
			}
		}
		if ((flags & MSG_PEEK) == 0 && so->so_oobmark) {
			so->so_oobmark -= len;
			if (so->so_oobmark == 0) {
				so->so_state |= SS_RCVATMARK;
				break;
			}
		}
		if (tomark) {
			tomark -= len;
			if (tomark == 0)
				break;
		}
	}
	if ((flags & MSG_PEEK) == 0) {
		if (m == 0)
			so->so_rcv.sb_mb = nextrecord;
		else if (pr->pr_flags & PR_ATOMIC)
			(void) sbdroprecord(&so->so_rcv);
		if (pr->pr_flags & PR_WANTRCVD && so->so_pcb)
			(*pr->pr_usrreq)(so, PRU_RCVD, (struct mbuf *)0,
			    (struct mbuf *)0, (struct mbuf *)0);
	}
release:
	sbunlock(&so->so_rcv);
	if (error == 0 && rightsp && *rightsp && pr->pr_domain->dom_externalize)
		error = (*pr->pr_domain->dom_externalize)(*rightsp);
	splx(s);
	return (error);
}

soshutdown(so, how)
	register struct socket *so;
	register int how;
{
	register struct protosw *pr = so->so_proto;

	how++;
	if (how & FREAD)
		sorflush(so);
	if (how & FWRITE)
		return ((*pr->pr_usrreq)(so, PRU_SHUTDOWN,
		    (struct mbuf *)0, (struct mbuf *)0, (struct mbuf *)0));
	return (0);
}

static
sorflush(so)
	register struct socket *so;
{
	register struct sockbuf *sb = &so->so_rcv;
	register struct protosw *pr = so->so_proto;
	register int s;
	struct sockbuf asb;

	sblock(sb);
	s = splimp();
	socantrcvmore(so);
	sbunlock(sb);
	asb = *sb;
	bzero((caddr_t)sb, sizeof (*sb));
	splx(s);
	if (pr->pr_flags & PR_RIGHTS && pr->pr_domain->dom_dispose)
		(*pr->pr_domain->dom_dispose)(asb.sb_mb);
	sbrelease(&asb);
}

sosetopt(so, level, optname, m0)
	register struct socket *so;
	int level, optname;
	struct mbuf *m0;
{
	int error = 0;
	register struct mbuf *m = m0;

	if (level != SOL_SOCKET) {
		if (so->so_proto && so->so_proto->pr_ctloutput)
			return ((*so->so_proto->pr_ctloutput)
				  (PRCO_SETOPT, so, level, optname, &m0));
		error = ENOPROTOOPT;
	} else {
		switch (optname) {

		case SO_LINGER:
			if (m == NULL || m->m_len != sizeof (struct linger)) {
				error = EINVAL;
				goto bad;
			}
			so->so_linger = mtod(m, struct linger *)->l_linger;
			/* fall thru... */

		case SO_DEBUG:
		case SO_KEEPALIVE:
		case SO_DONTROUTE:
		case SO_USELOOPBACK:
		case SO_BROADCAST:
		case SO_REUSEADDR:
			if (m == NULL || m->m_len < sizeof (int)) {
				error = EINVAL;
				goto bad;
			}
			if (*mtod(m, int *))
				so->so_options |= optname;
			else
				so->so_options &= ~optname;
			break;

		case SO_SNDBUF:
		case SO_RCVBUF:
		case SO_SNDLOWAT:
		case SO_RCVLOWAT:
		case SO_SNDTIMEO:
		case SO_RCVTIMEO:
			if (m == NULL || m->m_len < sizeof (int)) {
				error = EINVAL;
				goto bad;
			}
			switch (optname) {

			case SO_SNDBUF:
			case SO_RCVBUF:
				if (sbreserve(optname == SO_SNDBUF ? &so->so_snd :
				    &so->so_rcv, *mtod(m, int *)) == 0) {
					error = ENOBUFS;
					goto bad;
				}
				break;

#ifndef	pdp11
			case SO_SNDLOWAT:
				so->so_snd.sb_lowat = *mtod(m, int *);
				break;
			case SO_RCVLOWAT:
				so->so_rcv.sb_lowat = *mtod(m, int *);
				break;
			case SO_SNDTIMEO:
				so->so_snd.sb_timeo = *mtod(m, int *);
				break;
			case SO_RCVTIMEO:
				so->so_rcv.sb_timeo = *mtod(m, int *);
				break;
#endif
			}
			break;

		default:
			error = ENOPROTOOPT;
			break;
		}
	}
bad:
#ifdef	pdp11
	normalseg5(); /* mtod() remapped us */
#endif	pdp11
	if (m)
		(void) m_free(m);
	return (error);
}

sogetopt(so, level, optname, mp)
	register struct socket *so;
	int level, optname;
	struct mbuf **mp;
{
	register struct mbuf *m;

	if (level != SOL_SOCKET) {
		if (so->so_proto && so->so_proto->pr_ctloutput) {
			return ((*so->so_proto->pr_ctloutput)
				  (PRCO_GETOPT, so, level, optname, mp));
		} else 
			return (ENOPROTOOPT);
	} else {
#ifndef	pdp11
		m = m_get(M_WAIT, MT_SOOPTS);
#else	pdp11
		m = m_get(M_DONTWAIT, MT_SOOPTS);
		if (m == NULL)
			return(ENOBUFS);
#endif	pdp11
		switch (optname) {

		case SO_LINGER:
#ifndef	pdp11
			m->m_len = sizeof (struct linger);
			mtod(m, struct linger *)->l_onoff =
				so->so_options & SO_LINGER;
			mtod(m, struct linger *)->l_linger = so->so_linger;
#else	pdp11
		    {
			register struct linger *lp;
			m->m_len = sizeof (struct linger);
			lp = mtod(m, struct linger *);
			lp->l_onoff = so->so_options & SO_LINGER;
			lp->l_linger = so->so_linger;
		    }
#endif	pdp11
			break;

		case SO_USELOOPBACK:
		case SO_DONTROUTE:
		case SO_DEBUG:
		case SO_KEEPALIVE:
		case SO_REUSEADDR:
		case SO_BROADCAST:
			m->m_len = sizeof (int);
			*mtod(m, int *) = so->so_options & optname;
			break;

		case SO_SNDBUF:
			*mtod(m, int *) = so->so_snd.sb_hiwat;
			break;

		case SO_RCVBUF:
			*mtod(m, int *) = so->so_rcv.sb_hiwat;
			break;

#ifndef	pdp11
		case SO_SNDLOWAT:
			*mtod(m, int *) = so->so_snd.sb_lowat;
			break;

		case SO_RCVLOWAT:
			*mtod(m, int *) = so->so_rcv.sb_lowat;
			break;

		case SO_SNDTIMEO:
			*mtod(m, int *) = so->so_snd.sb_timeo;
			break;

		case SO_RCVTIMEO:
			*mtod(m, int *) = so->so_rcv.sb_timeo;
			break;
#else	pdp11
		case SO_SNDLOWAT:
		case SO_RCVLOWAT:
		case SO_SNDTIMEO:
		case SO_RCVTIMEO:
			*mtod(m, int *) = 0;
			break;
#endif	pdp11

		default:
			m_free(m);
			return (ENOPROTOOPT);
		}
#ifdef	pdp11
		normalseg5(); /* mtod() remapped us */
#endif	pdp11
		*mp = m;
		return (0);
	}
}

sohasoutofband(so)
	register struct socket *so;
{
	register struct proc *p;

	if (so->so_pgrp < 0)
		gsignal(-so->so_pgrp, SIGURG);
#ifdef	vax
	else if (so->so_pgrp > 0 && (p = pfind(so->so_pgrp)) != 0)
		psignal(p, SIGURG);
#else	pdp11
	else if (so->so_pgrp > 0) {
		register int pid = so->so_pgrp;
		segm	map5;
		/*
		 * We can get called with ka5 mapped someplace
		 * else, so we have to put it back so we can
		 * look at the proc table.
		 */
		saveseg5(map5);
		normalseg5();
		for (p = &proc[0]; p <= maxproc; p++)
			if (p->p_pid == pid) {
				psignal(p, SIGURG);
				break;
			}
		restorseg5(map5);
	}
#endif
}
