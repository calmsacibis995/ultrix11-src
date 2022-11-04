
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)mbuf.c	3.0	4/21/86
 *	based on mbuf.c	1.36	82/06/20
 */

#include <sys/param.h>
#ifdef	UCB_NET
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/mbuf.h>
#include <netinet/in_systm.h>		/* XXX */

#if !pdp11
mbinit()
{

	if (m_clalloc(4, MPG_MBUFS) == 0)
		goto bad;
	if (m_clalloc(32, MPG_CLUSTERS) == 0)
		goto bad;
	return;
bad:
	panic("mbinit");
}

caddr_t
m_clalloc(ncl, how)
	register int ncl;
	int how;
{
	int npg, mbx;
	register struct mbuf *m;
	register int i;
	int s;

	npg = ncl * CLSIZE;
	s = splimp();		/* careful: rmalloc isn't reentrant */
	mbx = rmalloc(mbmap, npg);
	splx(s);
	if (mbx == 0)
		return (0);
	m = cltom(mbx / CLSIZE);
	if (memall(&Mbmap[mbx], npg, proc, CSYS) == 0)
		return (0);
	vmaccess(&Mbmap[mbx], (caddr_t)m, npg);
	switch (how) {

	case MPG_CLUSTERS:
		s = splimp();
		for (i = 0; i < ncl; i++) {
			m->m_off = 0;
			m->m_next = mclfree;
			mclfree = m;
			m += CLBYTES / sizeof (*m);
			mbstat.m_clfree++;
		}
		mbstat.m_clusters += ncl;
		splx(s);
		break;

	case MPG_MBUFS:
		for (i = ncl * CLBYTES / sizeof (*m); i > 0; i--) {
			m->m_off = 0;
			m->m_free = 0;
			mbstat.m_mbufs++;
			(void) m_free(m);
			m++;
		}
		break;
	}
	return ((caddr_t)m);
}

m_pgfree(addr, n)
	caddr_t addr;
	int n;
{

#ifdef lint
	addr = addr; n = n;
#endif
}

m_expand()
{

	if (m_clalloc(1, MPG_MBUFS) == 0)
		goto steal;
	return (1);
steal:
	/* should ask protocols to free code */
	return (0);
}

/* NEED SOME WAY TO RELEASE SPACE */

/*
 * Space allocation routines.
 * These are also available as macros
 * for critical paths.
 */
struct mbuf *
m_get(canwait)
	int canwait;
{
	register struct mbuf *m;

	MGET(m, canwait, MT_DATA);
	return (m);
}

#endif	!pdp11

struct mbuf *
m_getclr(canwait, type)
	int canwait;
{
	register struct mbuf *m;

	m = m_get(canwait, type);
	if (m == 0)
		return (0);
	m->m_off = MMINOFF;
	bzero(mtod(m, caddr_t), MLEN);
	return (m);
}

#if	!pdp11

struct mbuf *
m_free(m)
	struct mbuf *m;
{
	register struct mbuf *n;

	MFREE(m, n);
	return (n);
}

/*ARGSUSED*/
struct mbuf *
m_more(type)
	int type;
{
	register struct mbuf *m;

	if (!m_expand()) {
		mbstat.m_drops++;
		return (NULL);
	}
#define m_more(x) (panic("m_more"), (struct mbuf *)0)
	MGET(m, type, MT_DATA);
#undef m_more
	return (m);
}
#endif !pdp11

m_freem(m)
	register struct mbuf *m;
{
	register struct mbuf *n;
	register int s;

	if (m == NULL)
		return;
	s = splimp();
	do {
		MFREE(m, n);
	} while (m = n);
	splx(s);
}

/*
 * Mbuffer utility routines.
 */
struct mbuf *
m_copy(m, off, len)
	register struct mbuf *m;
	int off;
	register int len;
{
	register struct mbuf *n, **np;
	struct mbuf *top, *p;

	if (len == 0)
		return (0);
	if (off < 0 || len < 0)
		panic("m_copy");
	while (off > 0) {
		if (m == 0)
			panic("m_copy");
		if (off < m->m_len)
			break;
		off -= m->m_len;
		m = m->m_next;
	}
	MAPSAVE();
	np = &top;
	top = 0;
	while (len > 0) {
		if (m == 0) {
			if (len != M_COPYALL)
				panic("m_copy");
			break;
		}
/* #if !pdp11  */
		MGET(n, M_DONTWAIT, MT_DATA);	/* Was M_WAIT - dab 8/20/85 */
		*np = n;
		if (n == 0)
			goto nospace;
		n->m_len = MIN(len, m->m_len - off);
#if !pdp11
		if (m->m_off > MMAXOFF) {
			p = mtod(m, struct mbuf *);
			n->m_off = ((int)p - (int)n) + off;
			mclrefcnt[mtocl(p)]++;
		} else
#endif
		{
			n->m_off = MMINOFF;
			MBCOPY(m,off,n,0,(unsigned)n->m_len);
		}
#ifdef never
		MSGET(n, struct mbuf, 0);
		*np = n;
		if (n == 0)
			goto nospace;
		n->m_len = MIN(len, m->m_len - off);
		n->m_off = m->m_off + off;
		n->m_act = n->m_next = 0;
		n->m_click = m->m_click;
		mapseg5(n->m_click,MBMAPSIZE);
		MBX->m_ref++;  
#endif
		if (len != M_COPYALL)
			len -= n->m_len;
		off = 0;
		m = m->m_next;
		np = &n->m_next;
	}
	goto out;
nospace:
	m_freem(top);
	top = 0;
out:
	MAPREST();
	return (top);
}

m_cat(m, n)
	register struct mbuf *m, *n;
{
	while (m->m_next)
		m = m->m_next;
	while (n) {
		if (m->m_off >= MMAXOFF ||
		    m->m_off + m->m_len + n->m_len > MMAXOFF) {
			/* just join the two chains */
			m->m_next = n;
			return;
		}
		/* splat the data from one into the other */
		MBCOPY(n, 0, m, m->m_len, (u_int)n->m_len);
		m->m_len += n->m_len;
		n = m_free(n);
	}
}

m_adj(mp, len)
	struct mbuf *mp;
	register int len;
{
	register struct mbuf *m, *n;

	if ((m = mp) == NULL)
		return;
	if (len >= 0) {
		while (m != NULL && len > 0) {
			if (m->m_len <= len) {
				len -= m->m_len;
				m->m_len = 0;
				m = m->m_next;
			} else {
				m->m_len -= len;
				m->m_off += len;
				break;
			}
		}
	} else {
		/* a 2 pass algorithm might be better */
		len = -len;
		while (len > 0 && m->m_len != 0) {
			while (m != NULL && m->m_len != 0) {
				n = m;
				m = m->m_next;
			}
			if (n->m_len <= len) {
				len -= n->m_len;
				n->m_len = 0;
				m = mp;
			} else {
				n->m_len -= len;
				break;
			}
		}
	}
}

struct mbuf *
m_pullup(m0, len)
	struct mbuf *m0;
	int len;
{
	register struct mbuf *m, *n;
	int count;

	n = m0;
	if (len > MLEN)
		goto bad;
	MGET(m, 0, MT_DATA);
	if (m == 0)
		goto bad;
	m->m_off = MMINOFF;
	m->m_len = 0;
	do {
		count = MIN(MLEN - m->m_len, len);
		if (count > n->m_len)
			count = n->m_len;
		MBCOPY(n, 0, m, m->m_len, (u_int)count);
		len -= count;
		m->m_len += count;
		n->m_off += count;
		n->m_len -= count;
		if (n->m_len)
			break;
		n = m_free(n);
	} while (n);
	if (len) {
		(void) m_free(m);
		goto bad;
	}
	m->m_next = n;
	return (m);
bad:
	m_freem(n);
	return (0);
}
#endif	UCB_NET
