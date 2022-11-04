
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)prim.c	3.0	4/21/86
 */
#include <sys/param.h>
#include <sys/tty.h>
#include <sys/systm.h>
#include <sys/conf.h>
#include <sys/buf.h>
/*#include <sys/clist.h>	included in systm.h */
#include <sys/seg.h>

int	nodev();	/* needed by cinit() below */


#ifdef	UCB_CLIST
/*
 * Modification to move clists out of kernel data space.
 * Clist space is allocated by startup.
 */
memaddr clststrt;		/* Physical click address of clist */
extern unsigned clstdesc;	/* PDR for clist segment when mapped */

#else	UCB_CLIST
extern struct	cblock	cfree[];
#endif	UCB_CLIST

extern int nclist;
struct	cblock	*cfreelist;
int	cbad;

/*
 * Character list get/put
 */
getc(p)
register struct clist *p;
{
	register struct cblock *bp;
	register int c, s;
#ifdef	UCB_CLIST
	segm sav5;
#endif	UCB_CLIST

	s = spl6();
#ifdef	UCB_CLIST
	saveseg5(sav5);
	mapseg5(clststrt, clstdesc);
#endif	UCB_CLIST
	if (p->c_cc <= 0) {
		c = -1;
		p->c_cc = 0;
		p->c_cf = p->c_cl = NULL;
	} else {
		c = *p->c_cf++ & 0377;
		if (--p->c_cc<=0) {
			bp = (struct cblock *)(p->c_cf-1);
			bp = (struct cblock *) ((int)bp & ~CROUND);
			p->c_cf = NULL;
			p->c_cl = NULL;
			bp->c_next = cfreelist;
			cfreelist = bp;
		} else if (((int)p->c_cf & CROUND) == 0){
			bp = (struct cblock *)(p->c_cf);
			bp--;
			p->c_cf = bp->c_next->c_info;
			bp->c_next = cfreelist;
			cfreelist = bp;
		}
	}
#ifdef	UCB_CLIST
	restorseg5(sav5);
#endif	UCB_CLIST
	splx(s);
	return(c);
}

/*
 ************************************************
 *						*
 *	q_to_b() moved to /sys/dev/mpxpk.c	*
 *						*
 ************************************************
 */

#ifdef	DH
/*
 * Return count of contiguous characters
 * in clist starting at q->c_cf.
 * Stop counting if flag&character is non-null.
 */
ndqb(q, flag)
register struct clist *q;
{
	register cc;
	int s;
#ifdef	UCB_CLIST
	segm sav5;
#endif	UCB_CLIST

	s = spl6();
	if (q->c_cc <= 0) {
		cc = -q->c_cc;
		goto out;
	}
	cc = ((int)q->c_cf + CBSIZE) & ~CROUND;
	cc -= (int)q->c_cf;
	if (q->c_cc < cc)
		cc = q->c_cc;
	if (flag) {
		register char *p, *end;

#ifdef	UCB_CLIST
		saveseg5(sav5);
		mapseg5(clststrt, clstdesc);
#endif	UCB_CLIST
		p = q->c_cf;
		end = p;
		end += cc;
		while (p < end) {
			if (*p & flag) {
				cc = (int)p;
				cc -= (int)q->c_cf;
				break;
			}
			p++;
		}
#ifdef	UCB_CLIST
		restorseg5(sav5);
#endif	UCB_CLIST
	}
out:
	splx(s);
	return(cc);
}



/*
 * Update clist to show that cc characters
 * were removed.  It is assumed that cc < CBSIZE.
 */
ndflush(q, cc)
register struct clist *q;
register cc;
{
	register s;
#ifdef	UCB_CLIST
	segm sav5;
#endif	UCB_CLIST

	s = spl6();
	if (q->c_cc < 0) {
		if (q->c_cf != NULL) {
			q->c_cc += cc;
			q->c_cf += cc;
			goto out;
		}
		q->c_cc = 0;
		goto out;
	}
	if (q->c_cc == 0) {
		goto out;
	}
	if (cc > CBSIZE || cc <= 0) {
		cbad++;
		goto out;
	}
	q->c_cc -= cc;
	q->c_cf += cc;
	if (((int)q->c_cf & CROUND) == 0) {
		register struct cblock *bp;

#ifdef	UCB_CLIST
		saveseg5(sav5);
		mapseg5(clststrt, clstdesc);
#endif	UCB_CLIST
		bp = (struct cblock *)(q->c_cf) -1;
		if (bp->c_next) {
			q->c_cf = bp->c_next->c_info;
		} else {
			q->c_cf = q->c_cl = NULL;
		}
		bp->c_next = cfreelist;
		cfreelist = bp;
#ifdef	UCB_CLIST
		restorseg5(sav5);
#endif	UCB_CLIST
	} else
	if (q->c_cc == 0) {
		register struct cblock *bp;
#ifdef	UCB_CLIST
		saveseg5(sav5);
		mapseg5(clststrt, clstdesc);
#endif	UCB_CLIST
		q->c_cf = (char *)((int)q->c_cf & ~CROUND);
		bp = (struct cblock *)(q->c_cf);
		bp->c_next = cfreelist;
		cfreelist = bp;
		q->c_cf = q->c_cl = NULL;
#ifdef	UCB_CLIST
		restorseg5(sav5);
#endif	UCB_CLIST
	}
out:
	splx(s);
}
#endif	DH


putc(c, p)
register struct clist *p;
{
	register struct cblock *bp;
	register char *cp;
	register s;
#ifdef	UCB_CLIST
	segm sav5;
#endif	UCB_CLIST

	s = spl6();
#ifdef	UCB_CLIST
	saveseg5(sav5);
	mapseg5(clststrt, clstdesc);
#endif	UCB_CLIST
	if ((cp = p->c_cl) == NULL || p->c_cc < 0 ) {
		if ((bp = cfreelist) == NULL) {
#ifdef	UCB_CLIST
			restorseg5(sav5);
#endif	UCB_CLIST
			splx(s);
			return(-1);
		}
		cfreelist = bp->c_next;
		bp->c_next = NULL;
		p->c_cf = cp = bp->c_info;
	} else if (((int)cp & CROUND) == 0) {
		bp = (struct cblock *)cp - 1;
		if ((bp->c_next = cfreelist) == NULL) {
#ifdef	UCB_CLIST
			restorseg5(sav5);
#endif	UCB_CLIST
			splx(s);
			return(-1);
		}
		bp = bp->c_next;
		cfreelist = bp->c_next;
		bp->c_next = NULL;
		cp = bp->c_info;
	}
	*cp++ = c;
	p->c_cc++;
	p->c_cl = cp;
#ifdef	UCB_CLIST
	restorseg5(sav5);
#endif	UCB_CLIST
	splx(s);
	return(0);
}



/*
 * copy buffer to clist.
 * return number of bytes not transfered.
 */
b_to_q(cp, cc, q)
register char *cp;
struct clist *q;
register int cc;
{
	register char *cq;
	register struct cblock *bp;
	register s, acc;
#ifdef	UCB_CLIST
	segm sav5;
#endif	UCB_CLIST

	if (cc <= 0)
		return(0);
	acc = cc;


	s = spl6();
#ifdef	UCB_CLIST
	saveseg5(sav5);
	mapseg5(clststrt, clstdesc);
#endif	UCB_CLIST
	if ((cq = q->c_cl) == NULL || q->c_cc < 0) {
		if ((bp = cfreelist) == NULL)
			goto out;
		cfreelist = bp->c_next;
		bp->c_next = NULL;
		q->c_cf = cq = bp->c_info;
	}

	while (cc) {
		if (((int)cq & CROUND) == 0) {
			bp = (struct cblock *) cq - 1;
			if ((bp->c_next = cfreelist) == NULL)
				goto out;
			bp = bp->c_next;
			cfreelist = bp->c_next;
			bp->c_next = NULL;
			cq = bp->c_info;
		}
		*cq++ = *cp++;
		cc--;
	}
out:
	q->c_cl = cq;
	q->c_cc += acc-cc;
#ifdef	UCB_CLIST
	restorseg5(sav5);
#endif	UCB_CLIST
	splx(s);
	return(cc);
}

/*
 * Initialize clist by freeing all character blocks, then count
 * number of character devices. (Once-only routine)
 *
 * Also set a bit in the character device configuration
 * word (el_cdcw) for each device present. Only the first 16
 * devices are checked because only those before `tty'
 * are of interest.
 */

cinit()
{
	register int ccp;
	register struct cblock *cp;
	register struct cdevsw *cdp;

#ifdef	UCB_CLIST
	cfree = SEG5;	/* Virt. Addr. of clists (0120000 - 0140000 */
	ccp = (int)cfree;
	mapseg5(clststrt, clstdesc);
#else	UCB_CLIST
	ccp = (int)cfree;
	ccp = (ccp+CROUND) & ~CROUND;
#endif	UCB_CLIST
	for(cp=(struct cblock *)ccp; cp <= &cfree[nclist-1]; cp++) {
		cp->c_next = cfreelist;
		cfreelist = cp;
	}
#ifdef	UCB_CLIST
	normalseg5();
#endif	UCB_CLIST
	ccp = 0;
	for(cdp = cdevsw; cdp->d_open; cdp++) {
		if((ccp < 16) && (cdp->d_write != &nodev))
			el_cdcw |= (1 << ccp);
		ccp++;
	}
	nchrdev = ccp;
}

/*
 ********************************************************
 *							*
 *	getw() and putw() moved to /sys/dev/mpxpk.c	*
 *							*
 ********************************************************
 */

/*
 * Given a non-NULL pointer into the list (like c_cf which
 * always points to a real character if non-NULL) return the pointer
 * to the next character in the list or return NULL if no more chars.
 *
 * Callers must not allow getc's to happen between nextc's so that the
 * pointer becomes invalid.  Note that interrupts are NOT masked.
 */
char *
nextc(p, cp)
register struct clist *p;
register char *cp;
{
	register char *rcp;
#ifdef	UCB_CLIST
	segm sav5;

	saveseg5(sav5);
	mapseg5(clststrt, clstdesc);
#endif	UCB_CLIST
	if (p->c_cc && ++cp != p->c_cl) {
		if (((int)cp & CROUND) == 0)
			rcp = ((struct cblock *)cp)[-1].c_next->c_info;
		else
			rcp = cp;
	}
	else {
		rcp = 0;
	}
#ifdef	UCB_CLIST
	restorseg5(sav5);
#endif	UCB_CLIST
	return rcp;
}

/*
 * lookc returns the character pointed at by cp, which is a nextc-style
 * (e.g. possibly mapped out) char pointer.
 */
lookc(cp)
register char *cp;
{
	register char rc;
#ifdef	UCB_CLIST
	segm sav5;

	saveseg5(sav5);
	mapseg5(clststrt, clstdesc);
#endif	UCB_CLIST
	rc = *cp;
#ifdef	UCB_CLIST
	restorseg5(sav5);
#endif	UCB_CLIST
	return rc;
}

/*
 * Remove the last character in the list and return it.
 */
unputc(p)
register struct clist *p;
{
	register struct cblock *bp;
	register int c, s;
	struct cblock *obp;
#ifdef	UCB_CLIST
	segm sav5;
#endif	UCB_CLIST

	s = spl6();
#ifdef	UCB_CLIST
	saveseg5(sav5);
	mapseg5(clststrt, clstdesc);
#endif	UCB_CLIST
	if (p->c_cc <= 0)
		c = -1;
	else {
		/*
		 * We don't have unsigned chars, so keep it from
		 * sign extending. 4/21/85 -Dave Borman
		 */
		c = (*--p->c_cl)&0377;
		if (--p->c_cc <= 0) {
			bp = (struct cblock *)p->c_cl;
			bp = (struct cblock *)((int)bp & ~CROUND);
			p->c_cl = p->c_cf = NULL;
			bp->c_next = cfreelist;
			cfreelist = bp;
		} else if (((int)p->c_cl & CROUND) == sizeof(bp->c_next)) {
			p->c_cl = (char *)((int)p->c_cl & ~CROUND);
			bp = (struct cblock *)p->c_cf;
			bp = (struct cblock *)((int)bp & ~CROUND);
			while (bp->c_next != (struct cblock *)p->c_cl)
				bp = bp->c_next;
			obp = bp;
			p->c_cl = (char *)(bp + 1);
			bp = bp->c_next;
			bp->c_next = cfreelist;
			cfreelist = bp;
			obp->c_next = NULL;
		}
	}
#ifdef	UCB_CLIST
	restorseg5(sav5);
#endif	UCB_CLIST
	splx(s);
	return (c);
}

/*
 * Put the chars in the ``from'' queue
 * on the end of the ``to'' queue.
 */
catq(from, to)
struct clist *from, *to;
{
	register c;

	while ((c = getc(from)) >= 0)
		 putc(c, to);
}
