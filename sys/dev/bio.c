
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)bio.c	3.0	4/21/86
 */
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/devmaj.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/buf.h>
#include <sys/conf.h>
#include <sys/proc.h>
#include <sys/seg.h>
#include <sys/mount.h>

struct {
	int	nbufr;
	long	nread;
	long	nreada;
	long	ncache;
	long	nwrite;
	long	bufcount[];
} io_info;

/*
 * swap IO headers.
 * they are filled in to point
 * at the desired IO operation.
 */
struct	buf	swbuf1;
struct	buf	swbuf2;

/*
 * The following several routines allocate and free
 * buffers with various side effects.  In general the
 * arguments to an allocate routine are a device and
 * a block number, and the value is a pointer to
 * to the buffer header; the buffer is marked "busy"
 * so that no one else can touch it.  If the block was
 * already in core, no I/O need be done; if it is
 * already busy, the process waits until it becomes free.
 * The following routines allocate a buffer:
 *	getblk
 *	bread
 *	breada
 * Eventually the buffer must be released, possibly with the
 * side effect of writing it out, by using one of
 *	bwrite
 *	bdwrite
 *	bawrite
 *	brelse
 */

#define	BUFHSZ 64	/* must be a power of 2 */
#define	BUFHASH(blkno)	((blkno) & (BUFHSZ-1))

struct	buf *bhash[BUFHSZ];

int	closeflg = 0;		/* flag to turn on/off the flushing of buffers
				 * on last close */
/*
 * Read in (if necessary) the block and return a buffer pointer.
 */
struct buf *
bread(dev, blkno)
dev_t dev;
daddr_t blkno;
{
	register struct buf *bp;

	bp = getblk(dev, blkno);
	if (bp->b_flags&B_DONE) {
		io_info.ncache++;
		return(bp);
	}
	bp->b_flags |= B_READ;
	bp->b_bcount = BSIZE;
	(*bdevsw[major(dev)].d_strategy)(bp);
	io_info.nread++;
	iowait(bp);
	return(bp);
}

/*
 * Read in the block, like bread, but also start I/O on the
 * read-ahead block (which is not allocated to the caller)
 */
struct buf *
breada(dev, blkno, rablkno)
dev_t dev;
daddr_t blkno, rablkno;
{
	register struct buf *bp, *rabp;

	bp = NULL;
	if (!incore(dev, blkno)) {
		bp = getblk(dev, blkno);
		if ((bp->b_flags&B_DONE) == 0) {
			bp->b_flags |= B_READ;
			bp->b_bcount = BSIZE;
			(*bdevsw[major(dev)].d_strategy)(bp);
			io_info.nread++;
		}
	}
	rabp = bdevsw[major(dev)].d_tab;
	rabp += dp_adj(dev);
	if (rablkno && ((rabp->b_flags&B_TAPE) == 0) && !incore(dev, rablkno)) {
		rabp = getblk(dev, rablkno);
		if (rabp->b_flags & B_DONE)
			brelse(rabp);
		else {
			rabp->b_flags |= B_READ|B_ASYNC;
			rabp->b_bcount = BSIZE;
			(*bdevsw[major(dev)].d_strategy)(rabp);
			io_info.nreada++;
		}
	}
	if(bp == NULL)
		return(bread(dev, blkno));
	iowait(bp);
	return(bp);
}

/*
 * Write the buffer, waiting for completion.
 * Then release the buffer.
 */
bwrite(bp)
register struct buf *bp;
{
	register flag;

	flag = bp->b_flags;
	bp->b_flags &= ~(B_READ | B_DONE | B_ERROR | B_DELWRI);
	bp->b_bcount = BSIZE;
	io_info.nwrite++;
	(*bdevsw[major(bp->b_dev)].d_strategy)(bp);
	if ((flag&B_ASYNC) == 0) {
		iowait(bp);
		brelse(bp);
	} else if (flag & B_DELWRI)
		bp->b_flags |= B_AGE;
	else
		geterror(bp);
}

/*
 * Release the buffer, marking it so that if it is grabbed
 * for another purpose it will be written out before being
 * given up (e.g. when writing a partial block where it is
 * assumed that another write for the same block will soon follow).
 * This can't be done for magtape, since writes must be done
 * in the same order as requested.
 */
bdwrite(bp)
register struct buf *bp;
{
	register struct buf *dp;

	dp = bdevsw[major(bp->b_dev)].d_tab;
	dp += dp_adj(bp->b_dev);
	if(dp->b_flags & B_TAPE)
/*		bawrite(bp);	*/
		bwrite(bp);
/*
 * The above changes prevents the magtape from
 * hanging the system by sucking up all the I/O
 * buffereds when doing a buffer write from the
 * magtape exerciser. The cost is some loss of
 * I/O throughput for buffered I/O operations to
 * magtape, RAW mode is the way to deal with tapes anyway !
 */
	else {
		bp->b_flags |= B_DELWRI | B_DONE;
		brelse(bp);
	}
}

/*
 * Release the buffer, start I/O on it, but don't wait for completion.
 */
bawrite(bp)
register struct buf *bp;
{

	bp->b_flags |= B_ASYNC;
	bwrite(bp);
}

/*
 * release the buffer, with no I/O implied.
 */
brelse(bp)
register struct buf *bp;
{
	register struct buf **backp;
	register s;

	if (bp->b_flags&B_WANTED)
		wakeup((caddr_t)bp);
	if (bfreelist.b_flags&B_WANTED) {
		bfreelist.b_flags &= ~B_WANTED;
		wakeup((caddr_t)&bfreelist);
	}
	if(bp->b_flags&B_ERROR) {
		/*
		 * Zapping a superblock would cause it to "move"
		 * to a different buffer on the next access, and
		 * the next update() would probably panic.
		 */
		if (!(bp->b_flags & B_MOUNT)) {
			bunhash(bp);
			bp->b_dev = NODEV;  /* no assoc. on error */
		}
		bp->b_resid = 0;    /* fix for 0 byte count returned bug! */
	}
	s = spl6();
    if (!(bp->b_flags & B_MOUNT)) {
	if(bp->b_flags & B_AGE) {
		backp = &bfreelist.av_forw;
		(*backp)->av_back = bp;
		bp->av_forw = *backp;
		*backp = bp;
		bp->av_back = &bfreelist;
	} else {
		backp = &bfreelist.av_back;
		(*backp)->av_forw = bp;
		bp->av_back = *backp;
		*backp = bp;
		bp->av_forw = &bfreelist;
	}
    }
	bp->b_flags &= ~(B_WANTED|B_BUSY|B_ASYNC|B_AGE);
	splx(s);
}

/*
 * See if the block is associated with some buffer
 * (mainly to avoid getting hung up on a wait in breada)
 *
 * 1/23/86 -- Fred Canter (NO cache hits on tape buffers)
 */
incore(dev, blkno)
dev_t dev;
daddr_t blkno;
{
	register struct buf *bp;
	register struct buf *dp;

	bp = bhash[BUFHASH(blkno)];
#ifdef	UCB_NKB
	blkno = fsbtodb(blkno);
#endif	UCB_NKB
	dp = bdevsw[major(dev)].d_tab;
	dp += dp_adj(dev);
	if(dp->b_flags&B_TAPE)
		return(0);
	for(; bp != NULL; bp = bp->b_link)
		if (bp->b_blkno==blkno && bp->b_dev==dev)
			return(1);
	return(0);
}

/*
 * Assign a buffer for the given block.  If the appropriate
 * block is already associated, return it; otherwise search
 * for the oldest non-busy buffer and reassign it.
 *
 * 1/23/86 -- Fred Canter (NO cache hits on tape buffers)
 */
struct buf *
getblk(dev, blkno)
dev_t dev;
daddr_t blkno;
{
	register struct buf *bp;
	register struct buf *dp;
	register int j;
	register i;
#ifdef	UCB_NKB
	daddr_t dblkno;
#endif	UCB_NKB

	if(major(dev) >= nblkdev)
		panic("blkdev");
	dp = bdevsw[major(dev)].d_tab;
	if(dp == NULL)
		panic("devtab");
	dp += dp_adj(dev);
    loop:
	spl0();
	j = BUFHASH(blkno);
	bp = bhash[j];
#ifdef	UCB_NKB
	dblkno = fsbtodb(blkno);
#endif	UCB_NKB
	for (; bp != NULL; bp = bp->b_link)
	{
		if(dp->b_flags&B_TAPE)
			break;
#ifdef	UCB_NKB
		if (bp->b_blkno != dblkno || bp->b_dev != dev)
#else
		if (bp->b_blkno!=blkno || bp->b_dev!=dev)
#endif	UCB_NKB
			continue;
		spl6();
		if (bp->b_flags&B_BUSY) {
			bp->b_flags |= B_WANTED;
			sleep((caddr_t)bp, PRIBIO+1);
			goto loop;
		}
		spl0();
		notavail(bp);
		return(bp);
	}
	spl6();
	if (bfreelist.av_forw == &bfreelist) {
		bfreelist.b_flags |= B_WANTED;
		sleep((caddr_t)&bfreelist, PRIBIO+1);
		goto loop;
	}
	spl0();
	notavail(bp = bfreelist.av_forw);
	i = ((unsigned)((unsigned)bp - (unsigned)&buf[0])/sizeof(struct buf));
	if(i < nbuf)
		io_info.bufcount[i]++;
	if (bp->b_flags & B_DELWRI) {
		bawrite(bp);
		goto loop;
	}
	bunhash(bp);
	bp->b_flags = B_BUSY;
	bp->b_back->b_forw = bp->b_forw;
	bp->b_forw->b_back = bp->b_back;
	bp->b_forw = dp->b_forw;
	bp->b_back = dp;
	dp->b_forw->b_back = bp;
	dp->b_forw = bp;
	bp->b_dev = dev;
#ifdef	UCB_NKB
	bp->b_blkno = dblkno;
#else
	bp->b_blkno = blkno;
#endif	UCB_NKB
	bp->b_error = 0;
	bp->b_link = bhash[j];
	bhash[j] = bp;
	return(bp);
}

/*
 * get an empty block,
 * not assigned to any particular device
 */
struct buf *
geteblk()
{
	register struct buf *bp;
	register struct buf *dp;

loop:
	spl6();
	while (bfreelist.av_forw == &bfreelist) {
		bfreelist.b_flags |= B_WANTED;
		sleep((caddr_t)&bfreelist, PRIBIO+1);
	}
	spl0();
	dp = &bfreelist;
	notavail(bp = bfreelist.av_forw);
	if (bp->b_flags & B_DELWRI) {
		bp->b_flags |= B_ASYNC;
		bwrite(bp);
		goto loop;
	}
	bunhash(bp);
	bp->b_flags = B_BUSY;
	bp->b_back->b_forw = bp->b_forw;
	bp->b_forw->b_back = bp->b_back;
	bp->b_forw = dp->b_forw;
	bp->b_back = dp;
	dp->b_forw->b_back = bp;
	dp->b_forw = bp;
	bp->b_dev = (dev_t)NODEV;
	bp->b_error = 0;
	bp->b_link = NULL;
	return(bp);
}

bunhash(bp)
register struct buf *bp;
{
	register struct buf *ep;
	register int i;

	if (bp->b_dev == NODEV)
		return;
#ifdef	UCB_NKB
	i = BUFHASH(dbtofsb(bp->b_blkno));
#else
	i = BUFHASH(bp->b_blkno);
#endif	UCB_NKB
	ep = bhash[i];
	if (ep == bp) {
		bhash[i] = bp->b_link;
		return;
	}
	for (; ep != NULL; ep = ep->b_link)
		if (ep->b_link == bp) {
			ep->b_link = bp->b_link;
			return;
		}
	panic("bunhash");
}

/*
 * Wait for I/O completion on the buffer; return errors
 * to the user.
 */
iowait(bp)
register struct buf *bp;
{

	spl6();
	while ((bp->b_flags&B_DONE)==0)
		sleep((caddr_t)bp, PRIBIO);
	spl0();
	geterror(bp);
}

/*
 * Unlink a buffer from the available list and mark it busy.
 * (internal interface)
 */
notavail(bp)
register struct buf *bp;
{
	register s;

	s = spl6();
	if (!(bp->b_flags & B_MOUNT)) {
		bp->av_back->av_forw = bp->av_forw;
		bp->av_forw->av_back = bp->av_back;
	}
	bp->b_flags |= B_BUSY;
	splx(s);
}

/*
 * Mark I/O complete on a buffer, release it if I/O is asynchronous,
 * and wake up anyone waiting for it.
 */
iodone(bp)
register struct buf *bp;
{

	if(bp->b_flags&B_MAP)
		mapfree(bp);
	bp->b_flags |= B_DONE;
	if (bp->b_flags&B_ASYNC)
		brelse(bp);
	else {
		bp->b_flags &= ~B_WANTED;
		wakeup((caddr_t)bp);
	}
}

/*
 * Zero the core associated with a buffer.
 * No problem, we now save the mapping before
 * doing the clear, and restore it back the way
 * it was.  This is needed because when alloc()
 * calls clrbuf() it already has a superblock
 * mapped and we don't want to lose that.
 */
clrbuf(bp)
struct buf *bp;
{
	register *p;
	register c;
	segm	tseg;

	saveseg5(tseg);
	p = (int *) mapin(bp);
	c = (BSIZE/sizeof(int)) >> 2;
	do {
		*p++ = 0; *p++ = 0;
		*p++ = 0; *p++ = 0;
	} while (--c);
	bp->b_resid = 0;
	restorseg5(tseg);
}

/*
 * swap I/O
 */
swap(blkno, coreaddr, count, rdflg)
register count;
{
	register struct buf *bp;
	register tcount;

	bp = &swbuf1;
	if(bp->b_flags & B_BUSY)
		if((swbuf2.b_flags&B_WANTED) == 0)
			bp = &swbuf2;
	spl6();
	while (bp->b_flags&B_BUSY) {
		bp->b_flags |= B_WANTED;
		sleep((caddr_t)bp, PSWP+1);
	}
	while (count) {
		bp->b_flags = B_BUSY | B_PHYS | rdflg;
		bp->b_dev = swapdev;
		tcount = count;
		/* this was 1700, we changed it to 1600 so that
		 * swap will never need more than 7 UNIBUS mapping
		 * registers.  8/3/84 -Dave & Fred.
		 */
		if (tcount >= 01600)	/* prevent byte-count wrap */
			tcount = 01600;
		bp->b_bcount = ctob(tcount);
		bp->b_blkno = swplo+blkno;
		bp->b_un.b_addr = (caddr_t)(coreaddr<<6);
		bp->b_xmem = (coreaddr>>10) & 077;
		(*bdevsw[major(swapdev)].d_strategy)(bp);
		spl6();
		while((bp->b_flags&B_DONE)==0)
			sleep((caddr_t)bp, PSWP);
		spl0();
		count -= tcount;
		coreaddr += tcount;
		blkno += ctod(tcount);
	}
	if (bp->b_flags&B_WANTED)
		wakeup((caddr_t)bp);
	bp->b_flags &= ~(B_BUSY|B_WANTED);
	if (bp->b_flags & B_ERROR)
		panic("IO err in swap");
}

/*
 * bdflush system call
 * Use by on-line disk exercisers, must be 
 * super-user, to cancel delayed write on
 * all buffers owned by the device.
 * This is necessary because the ecercisers do 
 * both block and raw I/O, and the delayed writes
 * from the block I/O operations were overwritting
 * the disk blocks used by the raw I/O test.
 */

bdflush()
{
	register struct a {
		dev_t	dev	/* major/minor device number */
		} *uap;

	uap = (struct a *)u.u_ap;
	if(suser())
		bflush(BF_CANCEL, uap->dev);
}

/*
 * make sure all write-behind blocks
 * on dev (or NODEV for all)
 * are flushed out.
 * (from umount and update)
 * If flag is nonzero then cancel delayed write
 * of buffer's owned by dev instead of flushing
 * them. This feature is only to be used by
 * on-line disk exercisers.
 */
bflush(flag, dev)
int flag;
dev_t dev;
{
	register struct buf *bp;

loop:
	spl6();
	for (bp = bfreelist.av_forw; bp != &bfreelist; bp = bp->av_forw) {
		if (bp->b_flags&B_DELWRI && (dev == NODEV||dev==bp->b_dev)) {
			if(flag)
				bp->b_flags &= ~(B_DELWRI);
			else {
				bp->b_flags |= B_ASYNC;
				notavail(bp);
				bwrite(bp);
				}
			goto loop;
		}
	}
	spl0();
}

/*
 * Raw I/O. The arguments are
 *	The strategy routine for the device
 *	A buffer, which will always be a special buffer
 *	  header owned exclusively by the device for this purpose
 *	The device number
 *	Read/write flag
 * Essentially all the work is computing physical addresses and
 * validating them.
 */
physio(strat, bp, dev, rw)
register struct buf *bp;
int (*strat)();
{
	register unsigned base;
	register int nb;
	int ts;

	base = (unsigned)u.u_base;
	/*
	 * Check odd base, odd count, and address wraparound
	 */
	if (base&01 || u.u_count&01 || base>=base+u.u_count)
		goto bad;
	/*
	 * Find start of data space, i.e., where we think
	 * a process should be allowed to start a raw I/O transfer.
	 *   file type	ts value
	 *	0407	0	(all data space, no text)
	 *	0410	text size rounded to next 8Kb
	 *	0411	0	split I/D
	 *	0430	root text size rounded to next 8Kb +
	 *		# seg regs used for largest overlay
	 *	431	0	split I/D
	 */
	if (u.u_sep)
		ts = 0;				/* 0411 & 0431 */
	else
		ts = (u.u_tsize+127) & ~0177;	/* 0407 & 0410 */
	if(u.u_exdata.ux_mag == 0430)		/* 0430 */
		ts += (u.u_ovdata.uo_nseg * 0200);
	nb = (base>>6) & 01777;
	/*
	 * Check overlap with text. (ts and nb now
	 * in 64-byte clicks)
	 */
	if (nb < ts)
		goto bad;
	/*
	 * Check that transfer is either entirely in the
	 * data or in the stack: that is, either
	 * the end is in the data or the start is in the stack
	 * (remember wraparound was already checked).
	 */
	if ((((base+u.u_count)>>6)&01777) >= ts+u.u_dsize
	    && nb < 1024-u.u_ssize)
		goto bad;
	spl6();
	while (bp->b_flags&B_BUSY) {
		bp->b_flags |= B_WANTED;
		sleep((caddr_t)bp, PRIBIO+1);
	}
	bp->b_flags = B_BUSY | B_PHYS | rw;
	bp->b_dev = dev;
	/*
	 * Compute physical address by simulating
	 * the segmentation hardware.
	 */
	ts = (u.u_sep? UDSA: UISA)->r[nb>>7] + (nb&0177);
	bp->b_un.b_addr = (caddr_t)((ts<<6) + (base&077));
	bp->b_xmem = (ts>>10) & 077;
#ifdef	UCB_NKB
	bp->b_blkno = u.u_offset >> PGSHIFT;
#else
	bp->b_blkno = u.u_offset >> BSHIFT;
#endif	UCB_NKB
	bp->b_bcount = u.u_count;
	bp->b_error = 0;
	u.u_procp->p_flag |= SLOCK;
	(*strat)(bp);
	spl6();
	while ((bp->b_flags&B_DONE) == 0)
		sleep((caddr_t)bp, PRIBIO);
	u.u_procp->p_flag &= ~SLOCK;
	if (bp->b_flags&B_WANTED)
		wakeup((caddr_t)bp);
	spl0();
	bp->b_flags &= ~(B_BUSY|B_WANTED);
	u.u_count = bp->b_resid;
	geterror(bp);
	return;
    bad:
	u.u_error = EFAULT;
}

/*
 * Pick up the device's error number and pass it to the user;
 * if there is an error but the number is 0 set a generalized
 * code.  Actually the latter is always true because devices
 * don't yet return specific errors.
 */
geterror(bp)
register struct buf *bp;
{

	if (bp->b_flags&B_ERROR)
		if ((u.u_error = bp->b_error)==0)
			u.u_error = EIO;
}

/* routine called by block devices on last close of a file to flush all
 * the buffers. George Mathew */

bflclose(dev)
dev_t dev;
{

	register struct buf *bp;
	register struct buf *dp;
	struct mount *mp;

	if(closeflg)
		return;
	for(mp = &mount[0]; mp < &mount[nmount]; mp++)
		if((dev == mp->m_dev) && (mp->m_inodp != NULL))     /* do not flush for mounted file system */
			return;
	bflush(BF_FLUSH, dev);
	dp = bdevsw[major(dev)].d_tab;
	dp += dp_adj(dev);
	spl6();
backthere:
	for (bp = dp->b_forw; bp != dp; bp = bp->b_forw) {
		if (bp->b_dev == dev) {
			if (bp->b_flags&(B_BUSY|B_ASYNC)) {
				/* It's active, wait for it to finish */
				bp->b_flags |= B_WANTED;
				sleep((caddr_t)bp, PRIBIO+1);
				goto backthere;
			} else {
				/* not active, disassociate it from the drive */
					bunhash(bp);
					bp->b_dev = NODEV;
			}
		}
	}
	spl0();
}

/*
 * Adjust a device table pointer, if the driver
 * supports multiple controllers (RA, TK, & TS).
 * Return the controller number, to be added to the
 * device table pointer (struct buf *)dp, if the
 * driver supports multiple controllers. Otherwise
 * return 0.
 *
 * For most devices, bdevsw[].d_tab points to the device
 * table which heads the I/O queue for the driver. For
 * multi contorller drivers, however, bdevsw[].d_tab
 * points to the first table in an array of tables,
 * one for each controller.
 *
 * Added 1/20/86 -- Fred Canter (to fix multi-cntlr support)
 */

dp_adj(dev)
{
	register int maj;

	maj = major(dev);
	if((maj == TK_BMAJ) || (maj == TS_BMAJ))
		return(minor(dev) & 3);
	else if(maj == RA_BMAJ)
		return((minor(dev)>>6) & 3);
	else
		return(0);
}
