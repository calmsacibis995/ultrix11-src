
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)ubmap.c	3.0	4/21/86
 */
/*
 *  File name:
 *
 *	ubmap.c
 *
 *  Source file description:
 *
 *	This module contains the kernel routines needed to: initialize,
 *	allocate, and free the unibus map resource. These routines were
 *	moved from main.c and machdep.c to ubmap.c so that they can be
 *	optionally included in the kernel at sysgen time. This saves
 *	kernel space if the processor does not have a unibus map.
 *
 *  Functions:
 *
 *	mapinit		Initialize the unibus mapping registers.
 *	mapalloc	Allocates the unibus map for an I/O transfer.
 *	mapfree		Frees the map after the I/O transfer completes.
 *
 *  Usage:
 *
 *	Called by the kernel and device drivers.
 *
 *  Compile:
 *
 *	cd /usr/sys/sys; make -k
 *	cd /usr/sys/ovsys; make -k
 *	(cc -c -O ubmap.c)
 *
 *  Modification history:
 *
 *	June 1985
 *		Modified to use malloc/mfree for keeping track
 *		of the mapping registers.  This also has changes
 *		for mapping out the clists.  -Dave Borman
 *	30 December 1984
 *		File Created -- Fred Canter
 *
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/seg.h>
#include <sys/buf.h>
#include <sys/uba.h>
#include <sys/map.h>

memaddr	bpaddr;		/* physical address of buffers (in 64 byte clicks) */

struct map ub_map[12] = { MAPDATA(12) };
/*
 * Initialize the ub_map, and some of the mapping registers.
 * The first register maps the stuff between &ub_start and &ub_end.
 * This includes things like uda, hp_bads, hk_bads, and
 * TS (cmdpkt, chrbuf, mesbuf).  If the clists aren't mapped out,
 * cfree is also included in this.  If the clists are mapped out,
 * then the second unibus register maps them.  We then allocate
 * enough registers to map the buffers, and free up the rest.
 */

mapinit()
{
	register int i, lastbuf_reg;
	long paddr = (long)bpaddr << 6;
	extern int nclist;

	if(!ubmaps)
		return;
	setubregno(0, (long)&cfree);
#ifdef	UCB_CLIST
	setubregno(CLIST_UBADDR/UBPAGE, clstaddr);
#endif	UCB_CLIST

	lastbuf_reg = nubreg(nbuf, BSIZE) + BUF_UBADDR/UBPAGE;
	for (i = BUF_UBADDR/UBPAGE; i < lastbuf_reg; i++) {
		setubregno(i, paddr);
		paddr += (long)UBPAGE;
	}
	mfree(ub_map, 31 - lastbuf_reg, lastbuf_reg);
}

/*
 * Unibus map allocation routine
 *
 * Unibus Map Registers are allocated as follows:
 *
 *  UMBR 31 is not used.
 *
 * Map register 0 maps the fisrt 8K bytes of BSS space, this covers
 * 	NPR access to: UDA comm. area, hp_bads & hk_bads.
 * Map register 1 maps to the clist area.
 * Map registers 2 thru ? map the I/O buffer cache.
 * The rest of the registers are available for mapping raw I/O transfers.
 */

int	maplock;

mapalloc(bp)
register struct buf *bp;
{
	long			paddr, ubaddr;
	int			s, ub_nregs;
	register int		ub_first;
	register struct ubmap	*ubp;

	if(!ubmaps)
		return;

	paddr = (((long) ((unsigned) bp->b_xmem)) << 16 )
		| ((long) ((unsigned) bp->b_un.b_addr));
	if ((bp->b_flags & B_PHYS) == 0) {
		/*
		 * Transfer in the buffer cache.
		 * Change the buffers's physical address
		 * into a UNIBUS address for the driver.
		 */
		ubaddr = paddr - (((ubadr_t) bpaddr) << 6) + BUF_UBADDR;
		bp->b_un.b_addr = loint(ubaddr);
		bp->b_xmem = hiint(ubaddr);
		bp->b_flags |= B_MAP;
	} else {
		ub_nregs = (int)btoub(bp->b_bcount);

		s = spl6();
		while ((ub_first = malloc(ub_map, ub_nregs)) == NULL) {
			maplock = 1;
			sleep((caddr_t)&ub_map, PSWP+1);
		}
		splx(s);

		ubp = &UBMAP[ub_first];
		bp->b_xmem = ub_first >> 3;
		bp->b_un.b_addr = (ub_first & 07) << 13;
		bp->b_flags |= B_MAP;

		while (ub_nregs--) {
			ubp->ub_lo = loint(paddr);
			ubp->ub_hi = hiint(paddr);
			ubp++;
			paddr += (long)UBPAGE;
		}
	}
}

/*
 * ub_alloc(paddr, count)
 * Given a physical address and a byte count, ub_alloc
 * maps the given memory onto the unibus and returns
 * this unibus address in clicks.
 */

ub_alloc(paddr, count)
long	paddr;
int	count;
{
	register int ub_first, ub_nregs;
	register struct ubmap	*ubp;
	int s;

	ub_nregs = (int)btoub(count);

	s = spl6();
	while ((ub_first = malloc(ub_map, ub_nregs)) == NULL) {
		maplock = 1;
		sleep((caddr_t)&ub_map, PSWP+1);
	}
	splx(s);

	for (ubp = &UBMAP[ub_first]; ub_nregs--; ubp++) {
		ubp->ub_lo = loint(paddr);
		ubp->ub_hi = hiint(paddr);
		paddr += (long)UBPAGE;
	}
	return(ub_first<<7);
}

#ifdef	notused
/*
 * ub_free(ubaddr, count)
 * Given a unibus address (in clicks) and a byte count, ub_free()
 * releases that segment of the unibus map and wakes up anyone
 * waiting for the unibus map.
 */

ub_free(ubaddr, count)
unsigned	ubaddr;
unsigned	count;
{
	register int s = spl6();

	mfree(ub_map, (int) btoub(count),
		ubaddr>>7);
	splx(s);
	if (maplock) {
		wakeup((caddr_t)ub_map);
		maplock = 0;
	}
}
#endif	notused

mapfree(bp)
register struct buf *bp;
{
	register int s;

	if (bp->b_flags & B_PHYS) {
		s = spl6();
		mfree(ub_map, (int) btoub(bp->b_bcount),
			(bp->b_xmem << 3) | (((int)bp->b_un.b_addr >> 13) & 07));
		splx(s);
		bp->b_flags &= ~B_MAP;
		if (maplock) {
			wakeup((caddr_t)ub_map);
			maplock = 0;
		}
	} else {
		long ubaddr, paddr;
		/*
		 * Translate the UNIBUS virtual address of this buffer
		 * back to a physical memory address.
		 */
		ubaddr = (((long) ((unsigned) bp->b_xmem)) << 16)
			| ((long) ((unsigned) bp->b_un.b_addr));
		paddr = ubaddr - (long) BUF_UBADDR + (((long) bpaddr) << 6);
		bp->b_un.b_addr = loint(paddr);
		bp->b_xmem = hiint(paddr);
		bp->b_flags &= ~B_MAP;
	}
}
