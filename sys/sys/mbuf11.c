
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)mbuf11.c	3.0	4/21/86
 *	based on mbuf11.c  1.01    82/09/23
 */

#include <sys/param.h>
#ifdef	UCB_NET
#include <sys/seg.h>
#include <sys/mbuf.h>
#include <netinet/in_systm.h>

/*#define debug   1               /* consistency checks */

#ifdef debug
#define MBTRACE(type,arg) mbtrace(type,arg)
#else
#define MBTRACE(t,a)
#endif

#define IOSIZE  8192            /* area for DMA buffers */
#define NMBCACHE 8              /* cache of mbufs */
extern int nwords;		/* init arena size, bytes=(nwords-2)*WORD */

#define MBXGET(c) { \
	if ((c) = mbfree) { \
		mapseg5(mbfree,MBMAPSIZE); if (MBX->m_ref) panic("MBXGET"); \
		MBX->m_ref = 1;  MBTRACE(6,mbfree);  mbfree = (u_int)MBX->m_mbuf; }}

#define MBXFREE(c) { \
	mapseg5(c,MBMAPSIZE); if (MBX->m_ref != 1) panic("MBXFREE"); \
	MBX->m_ref = 0;  MBX->m_type = 0; MBTRACE(7,c);  MBX->m_mbuf = (struct mbuf *)mbfree; \
	mbfree = (c); }

struct mbuf *mbcache[NMBCACHE]; /* mbuf cache for fast allocation */
int     mbicache;               /* # entries in cache and index of next add */

extern memaddr	mbbase;		/* click address of mbuffer base */
extern u_int	mbsize;		/* size of mbuf area */
u_int   mbend;
u_int   miobase;                /* click of DMA area */
extern u_int   miosize;		/* size of area used for DMA */
u_int   mbfree;                 /* free list */
int	m_want, ms_want;

/*
 * Get an mbuf, consisting of an inaddress header (mbuf) and
 * an external data portion (mbufx).  Try to save some time by using
 * the cache.
 */
struct mbuf *
m_bget(canwait, type)
int canwait;
int type;
{
	register struct mbuf *m;
	int s = splimp();
	u_int click;
	segm	map5;

	saveseg5(map5);
	for (;;) {
		if (mbicache) {
			m = mbcache[--mbicache];
			mbstat.m_mbfree--;
			splx(s);
			MBTRACE(0,m);
			m->m_next = m->m_act = m->m_len = 0;
	/*@*/		m->m_off = MMINOFF;
			mapseg5(m->m_click, MBMAPSIZE);
			MBX->m_type = type;
			restorseg5(map5);
			return(m);
		}
	/* otherwise do it the hard way */
		MSGET(m, struct mbuf, MT_DATA, 0, canwait);
		if (m == 0)
			break;
		MBXGET(click);
		if (!click) {
			MSFREE(m);
			if (canwait) {
				restorseg5(map5);
				m_want++;
				sleep(&mbfree, PZERO - 1);
				continue;
			} else {
				mbstat.m_drops++;
				m = 0;
			}
			break;
		}
		m->m_click = click;
		m->m_next = m->m_act = m->m_len = 0;
		m->m_off = MMINOFF;
		MBX->m_mbuf = m;
		MBX->m_type = type;
		mbstat.m_mbfree--;
		mbstat.m_mbufs = MIN(mbstat.m_mbfree,mbstat.m_mbufs);
		break;
	}
	restorseg5(map5);
	splx(s);
	MBTRACE(1,m);
	return (m);
}

/*
 * Return an mbuf.  If not the last reference, just return the header;
 * else return both mbuf/mbufx.  Use the cache if empty slot exists.
 */
struct mbuf *
m_bfree(m)
	struct mbuf *m;
{
	register struct mbuf *n;
	int s = splimp();
	u_int click;

	MAPSAVE();
	n = m->m_next;
#ifdef debug
	MBTRACE(2,m);
	if (m->m_click < mbbase || m->m_click > mbend)
		panic("m_bf");
#endif
	click = m->m_click;
	mapseg5(click,MBMAPSIZE);
	if (MBX->m_ref != 1) {
		if (MBX->m_ref < 1 || MBX->m_ref > 4) panic("m_bf2");
		MBX->m_ref--;
		MSFREE(m);
		goto out;
	}
	mbstat.m_mbfree++;
	if (mbicache < NMBCACHE) {
		int *ip = (int *) m; ip--;
		if ((*ip & 01) == 0) panic("m_bf3");
		mbcache[mbicache++] = m;
		goto out;
	}
	MSFREE(m);
	MBXFREE(click);
out:
	MAPREST();
	if (m_want) {
		m_want = 0;
		wakeup((caddr_t)m_want);
	}
	splx(s);
	return (n);
}

/*
 * Allocate a contiguous buffer for DMA IO.  Called from if_ubainit().
 * TODO: fix net device drivers to handle scatter/gather to mbufs
 * on their own; thus avoiding the copy to/from this area.
 */
u_int
m_ioget(size)
{
	u_int base;

	size = ((size + 077) & ~077);   /* round up byte size */
	if (size > miosize) return(0);
	miosize -= size;
	base = miobase;
	miobase += (size>>6);
	MBTRACE(3,base);
	return(base);
}

/*	C storage allocator
 *	circular first-fit strategy
 *	works with noncontiguous, but monotonically linked, arena
 *	each block is preceded by a ptr to the (pointer of) 
 *	the next following block
 *	blocks are exact number of words long 
 *	aligned to the data type requirements of ALIGN
 *	pointers to blocks must have BUSY bit 0
 *	bit in ptr is 1 for busy, 0 for idle
 *	gaps in arena are merely noted as busy blocks
 *	last block of arena (pointed to by alloct) is empty and
 *	has a pointer to first
 *	idle blocks are coalesced during space search
 *
 *	a different implementation may need to redefine
 *	ALIGN, NALIGN, BLOCK, BUSY, INT
 *	where INT is integer type to which a pointer can be cast
*/
#define INT int
#define ALIGN int
#define NALIGN 1
#define WORD sizeof(union store)
#define BLOCK 1024	/* a multiple of WORD*/
#define BUSY 1
#define NULL 0

#define testbusy(p) ((INT)(p)&BUSY)
#define setbusy(p) (union store *)((INT)(p)|BUSY)
#define clearbusy(p) (union store *)((INT)(p)&~BUSY)

union store { union store *ptr;
	      ALIGN dummy[NALIGN];
	      int calloc;	/*calloc clears an array of integers*/
};

extern union store allocs[];  /* initial arena */
union store *allocp;    /*search ptr*/
union store *alloct;    /*arena top*/

#ifdef debug
#define ASSERT(p) if(!(p))botch("p");else
botch(s)
char *s;
{
	printf("botch %s\n",s);
	panic("mbuf11");
}
#ifdef longdebug
allock()
{
	register union store *p;
	int x;
	x = 0;
	for(p= &allocs[0]; clearbusy(p->ptr) > p; p=clearbusy(p->ptr)) {
		if(p==allocp)
			x++;
	}
	ASSERT(p==alloct);
	return(x==1||p==allocp);
}
#endif longdebug
#else
#define ASSERT(p)
#endif debug

char *
m_sget(nbytes,clr,canwait)
unsigned nbytes,clr;
{
	register union store *p, *q;
	register nw;
	int temp,val;
	int s = splimp();

	nw = (nbytes+WORD+WORD-1)/WORD;
top:
	ASSERT(allocp>=allocs && allocp<=alloct);
#ifdef	longdebug
	ASSERT(allock());
#endif	logndebug
	for(p=allocp; ; ) {
		for(temp=0; ; ) {
			if(!testbusy(p->ptr)) {
				while(!testbusy((q=p->ptr)->ptr)) {
					ASSERT(q>p&&q<alloct);
					p->ptr = q->ptr;
				}
				if(q>=p+nw && p+nw>=p)
					goto found;
			}
			q = p;
			p = clearbusy(p->ptr);
			if(p>q)
				ASSERT(p<=alloct);
			else if(q!=alloct || p!=allocs) {
				ASSERT(q==alloct&&p==allocs);
				goto bad;
			} else if(++temp>1)
				break;
		}
		/* malloc would call sbrk here and try again */
		mbstat.m_drops++;
		goto bad;
	}
found:
	allocp = p + nw;
	ASSERT(allocp<=alloct);
	if(q>allocp) {
		allocp->ptr = p->ptr;
	}
	p->ptr = setbusy(allocp);
	mbstat.m_clfree -= /* (clearbusy(p->ptr) - p) */ nw * WORD;
	mbstat.m_clusters = MIN(mbstat.m_clusters, mbstat.m_clfree);
	val = (char *)(p+1);
	splx(s);
	if (clr)
		bzero(val,((nbytes+1)& ~1));
	MBTRACE(4,val);
	return(val);
bad:
	if (canwait == M_WAIT) {
		ms_want++;
		sleep((caddr_t)&allocp, PZERO - 1);
		goto top;
	}
	splx(s);
	return(NULL);
}

/*	freeing strategy tuned for LIFO allocation
*/
m_sfree(ap)
register char *ap;
{
	register union store *p = (union store *)ap;
	int s = splimp();

	MBTRACE(5,ap);
	/* ASSERT(p>clearbusy(allocs[1].ptr)&&p<=alloct); */
	ASSERT(p>allocs&&p<=alloct);
#ifdef	longdebug
	ASSERT(allock());
#endif	longdebug
	/* allocp = --p;   leaves old blocks around longer for debugging */
		--p;
	ASSERT(testbusy(p->ptr));
	p->ptr = clearbusy(p->ptr);
	ASSERT(p->ptr > p && p->ptr <= alloct);
	mbstat.m_clfree += ((p->ptr) - p) * WORD;
	if (ms_want) {
		ms_want = 0;
		wakeup((caddr_t)&allocp);
	}
	splx(s);
}

/*
 * Mbuf address to data address, with bounds checking.  Called from
 * "mtod" macro which does type cast.
 */
mtodf(m)
register struct mbuf *m;
{
#ifdef	debug
	if (m < (struct mbuf *)&allocs[0] || m > (struct mbuf *) alloct)
		panic("allocs-mtodf");
	if (m->m_click < mbbase || m->m_click > mbend)
		panic("m_click-mtodf");
	if (m->m_off < MMINOFF)
		panic("m_off-MIN-mtodf");
	if (m->m_off > MMAXOFF) {
		printf("m %o, m_off %o\n",m, m->m_off);
		panic("m_off-MAX-mtodf");
	}
	if (m->m_len < 0 || m->m_len > MLEN)
		panic("m_len-mtodf");
#else !debug
	if (m < (struct mbuf *)&allocs[0] || m > (struct mbuf *) alloct
#ifdef debug
	    || m->m_click < mbbase || m->m_click > mbend
	    || m->m_off < MMINOFF || m->m_off > MMAXOFF
	    || m->m_len < 0 || m->m_len > MLEN
#endif
		) panic("mtodf");
#endif debug
	mapseg5(m->m_click,MBMAPSIZE);
	MBX->m_mbuf = m;
	return ((int)MBX + m->m_off);
}

/*
 * Initialize the buffer pool.  Called from netinit/main.
 */
mbinit()
{
	register i;
	u_int base;

	MAPSAVE();
	/* setup the arena */
	allocs[0].ptr = &allocs[nwords-1];
	allocs[nwords-1].ptr = setbusy(&allocs[0]);
	alloct = &allocs[nwords-1];
	allocp = &allocs[0];
	mbstat.m_clusters = mbstat.m_clfree = (nwords-2) * WORD;
	/* setup DMA IO area */
	miobase = mbbase;
	mbbase += (miosize>>6);
	mbsize -= miosize;
	/* link the mbufs */
	mbstat.m_mbufs = mbstat.m_mbfree = mbsize/MSIZE;
	base = mbbase;
	mbend = mbbase + (mbsize >> 6);

	for(i=0 ; i<(mbsize/MSIZE); i++) {
		mapseg5(base,MBMAPSIZE);
		MBX->m_ref = 1;
		MBXFREE(base);
		base += (MSIZE>>6);
	}
	MAPREST();
}


#ifdef debug

#define NMBTBUF 40

struct mbtbuf {
	u_int   mt_type;        /* type plus KISA6 */
	u_int   mt_pc;
	u_int   mt_arg;
} mbtbuf[NMBTBUF], *mbitbuf = mbtbuf;

mbtrace(type,arg)
{
	int s = spl7();
	register int *ip;
	register struct mbtbuf *mt = mbitbuf;
	extern int _ovno;

	if (++mbitbuf >= &mbtbuf[NMBTBUF])
		mbitbuf = mbtbuf;
	mt->mt_type = (type << 12) | _ovno;
	ip = &type;  ip--;  ip--;
	ip = *ip;  ip++;
	mt->mt_pc = *ip;
	mt->mt_arg = arg;
	splx(s);
}

#ifdef	notdef
mbprint(m,s)
register struct mbuf *m;
char *s;
{
	register char *ba;
	int col,i,bc;

#ifndef	debug
	return;
#endif
	MAPSAVE();
	printf("MB %s\n",s);
	for (;;) {
		if (m == 0) break;
		ba = mtod(m, char *);
		col = 0;  bc = m->m_len;
		printf("m%o next%o off%o len%o click%o act%o back%o ref%o\n",
			m, m->m_next, m->m_off, m->m_len, m->m_click, m->m_act,
			MBX->m_mbuf, MBX->m_ref);
		for(; bc ; bc--) {
			i = *ba++ & 0377;
			printf("%o ",i);
			if(++col > 31) {
				col = 0;
				printf("\n  ");
			}
		}
		printf("\n");
		m = m->m_next;
	}
	MAPREST();
}

#endif	notdef
#endif debug
#endif	UCB_NET
