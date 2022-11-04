
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)dsort.c	3.0	4/21/86
 */
/*
 * disksort()
 * generalized seek sort for disk
 *
 *
 * isbad()
 * fixbad()
 * V7M-11	Unix Runtime Common Bad Sector Code
 *
 * Jerry Brenner	12/20/82
 *
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/buf.h>
#include <sys/seg.h>
#include <sys/bads.h>

#define	b_cylin	b_resid

disksort(dp, bp)
register struct buf *dp, *bp;
{
	register struct buf *ap;
	struct buf *tp;

	ap = dp->b_actf;
	if(ap == NULL) {
		dp->b_actf = bp;
		dp->b_actl = bp;
		bp->av_forw = NULL;
		return;
	}
	tp = NULL;
	for(; ap != NULL; ap = ap->av_forw) {
		if ((bp->b_flags&B_READ) && (ap->b_flags&B_READ) == 0) {
			if (tp == NULL)
				tp = ap;
			break;
		}
		if ((bp->b_flags&B_READ) == 0 && (ap->b_flags&B_READ))
			continue;
		if(ap->b_cylin <= bp->b_cylin)
			if(tp == NULL || ap->b_cylin >= tp->b_cylin)
				tp = ap;
	}
	if(tp == NULL)
		tp = dp->b_actl;
	bp->av_forw = tp->av_forw;
	tp->av_forw = bp;
	if(tp == dp->b_actl)
		dp->b_actl = bp;
}

/*
 * Search the bad sector table looking for
 * the specified sector.  Return index if found.
 * Return -1 if not found.
 *
 * isbad(bads pointer, cylinder, track-sector, sectors per track)
 */

isbad(badp, cn, tsn, nsect)
struct dkbad *badp;
int cn, tsn, nsect;
{
	register struct bt_bad *bt;
	int i;
	long blk, bblk;

	/* if pointer is null, or the must be zero field is not zero
	 * or the serial number is zero
	 * do not revector.
	 */
	if(badp == 0 || badp->bt_mbz
	  || (badp->bt_csnl == 0 && badp->bt_csnh == 0))
		return(-1);	/* invalid pointer */


	blk = (((long)cn)<<16) + tsn;	/* build an asked for number */
	bt = badp->bt_badb;		/* point this to the bads info */

	/*
	 * scan through the bads info
	 *  until:
	 *	we exceed the number of sectors per track for the drive
	 *	find a negative number in bads
	 *	number from bads exceeds requested
	 *	we get a compare
	 *  if we get a compare then return the index into the
	 *  bads field.
	 */
	for (i = 0; i < nsect; i++, bt++) {
		bblk=((long)bt->bt_cyl<<16)+bt->bt_trksec;
		if (blk == bblk)
			return (i);
		if (blk < bblk || bblk < 0)
			break;
	}
	return (-1);
}


/*
 * Build a resume structure for bad blocking and ECC
 *
 * fixbad(dkr, bp, wcnt, flag)
 *
 * dkr	- is a pointer to a resume structure
 * bp	- is a buffer header pointer for current buffer
 * wcnt	- is a copy of the devices updated word count register
 * flag	- tells us if it is bad sector or ecc request
 */

fixbad(dkr, bp, wcnt, flag)
struct dkres *dkr;
struct buf *bp;
unsigned wcnt;
int flag;
{
	int scnt, rcnt, tcnt;
	union {
		long lad;
		int iad[2];
	}tad;

	/*
	 * Calculate successful byte count by taking the original
	 * byte count from the buffer header, converting it to
	 * a negative word count and subtracting from updated word count
	 * and converting back to bytes.
	 * Then round it off to successful blocks transfered in bytes.
	 * The remaining count is derived by subtracting successful count
	 * from the original count.
	 */
	tcnt = -(bp->b_bcount>>1);
	scnt = (wcnt - tcnt)<<1;
	scnt = (scnt/512)*512;
	rcnt = bp->b_bcount - scnt;

	/*
	 * Get the transfer beginning address into a long.
	 * Done funny because 'C' compiler stores longs backwards.
	 * Then calculate updated address by adding successful
	 * bytes transfered
	 */
	tad.iad[0] = bp->b_xmem;
	tad.iad[1] = bp->b_un.b_addr;
	tad.lad += scnt;

	if(flag){	/* This is for Bad Sector */
		dkr->r_vma = tad.iad[1];	/* load address for revector */
		dkr->r_vxm = tad.iad[0];	/* extended memory too */

		/*
		 * If remaining count > 512 bytes. (1 sector) 
		 * Continuation count (r_cc) gets remaining count minus
		 * revector count. Revector count (r_vcc) gets 512.
		 * Push memory address past revector area and load
		 * continue address (r_ma & r_xm).
		 * Now calculate continue block number by taking
		 * the starting block number and adding successful
		 * block transfered + revector block transfered.
		 */
		if(rcnt > 512){
			dkr->r_cc = rcnt - 512;
			dkr->r_vcc = 512;
			tad.lad += 512;
			dkr->r_ma = tad.iad[1];
			dkr->r_xm = tad.iad[0];
			dkr->r_bn = bp->b_blkno+(scnt/512)+1;
		} else {
			/* Make sure that vector count is never <= 0 */
			dkr->r_vcc = rcnt<=0?512:rcnt;
			dkr->r_cc = 0;	/* no continue */
			dkr->r_ma = 0;
		}
	}else{
		/*
		 * This is for ECC continue.
		 * load continue address (r_ma & r_xm)
		 * put the remaining byte count into r_cc, and just
		 * for grins the successful byte count into r_vcc.
		 * Now calculate continue block number by taking
		 * the starting block number and adding successful
		 * block transfered.
		 */
		dkr->r_ma = tad.iad[1];
		dkr->r_xm = tad.iad[0];
		dkr->r_cc = rcnt;
		dkr->r_vcc = scnt;
		dkr->r_bn = bp->b_blkno + (scnt/512);
	}
}
