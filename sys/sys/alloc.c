
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)alloc.c	3.1	7/21/87
 */
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/filsys.h>
#include <sys/mount.h>
#include <sys/fblk.h>
#include <sys/conf.h>
#include <sys/buf.h>
#include <sys/inode.h>
#include <sys/ino.h>
#include <sys/dir.h>
#include <sys/user.h>
typedef	struct fblk *FBLKP;

#include <sys/seg.h>

/*
 * alloc will obtain the next available
 * free disk block from the free list of
 * the specified device.
 * The super block has up to NICFREE remembered
 * free blocks; the last of these is read to
 * obtain NICFREE more . . .
 *
 * no space on dev x/y -- when
 * the free list is exhausted.
 */
struct buf *
alloc(dev)
dev_t dev;
{
	daddr_t bno;
	register struct filsys *fp;
	register struct buf *bp;
	int	i;
	register struct fblk *fbp;
	struct filsys *fps;
	segm	mntmap;

	if ((fp = getfs(dev)) == NULL)
		goto nofs;
	saveseg5(mntmap);
	while(fp->s_flock & S_BUSY) {
		fp->s_flock |= S_WANTED;
		normalseg5();	/* can't sleep with a buffer mapped! */
		sleep((caddr_t)&fp->s_flock, PINOD);
		restorseg5(mntmap);
	}
	do {
		if(fp->s_nfree <= 0)
			goto nospace;
		if (fp->s_nfree > NICFREE) {
			prdev("Bad free count", dev);
			goto nospace;
		}
		bno = fp->s_free[--fp->s_nfree];
		if(bno == 0)
			goto nospace;
	} while (badblock(fp, bno, dev));
	if(fp->s_nfree <= 0) {
		fp->s_flock |= S_BUSY;
		normalseg5();
		bp = bread(dev, bno);
		restorseg5(mntmap);
		if ((bp->b_flags&B_ERROR) == 0)
			copyio(PADDR(bp), &fp->s_nfree, sizeof (struct fblk),
								U_RKD);
		brelse(bp);
		/*
		 * Write the superblock back, synchronously,
		 * so that the free list pointer won't point at garbage.
		 * We can still end up with dups in free if we then
		 * use some of the blocks in this freeblock, then crash
		 * without a sync.
		 */
		fp->s_fmod = 0;
		fp->s_time = time;
		normalseg5();
		bp = getblk(dev, SUPERB);
		if (!(bp->b_flags & B_MOUNT)) {	/* just a precaution... */
			prdev("SUPERB not B_MOUNT!",dev);
			panic("alloc:");
		}
		/*
		 * We've been keeping the superblock in the buffer,
		 * so all we have to do is write it out.
		 */
		bwrite(bp);
		restorseg5(mntmap);
		if (fp->s_flock & S_WANTED)
			wakeup((caddr_t)&fp->s_flock);
		fp->s_flock = 0;
		if (fp->s_nfree <=0)
			goto nospace;
	}
	normalseg5();
	bp = getblk(dev, bno);
	clrbuf(bp);
	restorseg5(mntmap);
	fp->s_fmod = 1;
	fp->s_tfree--;
	normalseg5();
	return(bp);

nospace:
	fp->s_nfree = 0;
	fp->s_tfree = 0;
	normalseg5();
	prdev("no space", dev);
	for (i = 0; i < 5; i++)
		sleep((caddr_t)&lbolt, PRIBIO);
nofs:
	u.u_error = ENOSPC;
	return(NULL);
}

/*
 * place the specified disk block
 * back on the free list of the
 * specified device.
 */
free(dev, bno)
dev_t dev;
daddr_t bno;
{
	register struct filsys *fp;
	register struct buf *bp;
	register struct fblk *fbp;
	segm	mntmap;

	if ((fp = getfs(dev)) == NULL)
		return;
	if (badblock(fp, bno, dev))
		goto ret;
	saveseg5(mntmap);
	while(fp->s_flock & S_BUSY) {
		fp->s_flock |= S_WANTED;
		normalseg5();
		sleep((caddr_t)&fp->s_flock, PINOD);
		restorseg5(mntmap);
	}
	if(fp->s_nfree <= 0) {
		fp->s_nfree = 1;
		fp->s_free[0] = 0;
	}
	if(fp->s_nfree >= NICFREE) {
		fp->s_flock |= S_BUSY;
		normalseg5();		/* normal mapping, getblk will sleep */
		bp = getblk(dev, bno);
		restorseg5(mntmap);	/* map back to SB */
		copyio(PADDR(bp), &fp->s_nfree, sizeof(struct fblk), U_WKD);
		fp->s_nfree = 0;
		normalseg5();
		bwrite(bp);
		restorseg5(mntmap);
		if (fp->s_flock & S_WANTED)
			wakeup((caddr_t)&fp->s_flock);
		fp->s_flock = 0;
	}
	fp->s_free[fp->s_nfree++] = bno;
	fp->s_tfree++;
	fp->s_fmod = 1;
ret:
	normalseg5();
}

/*
 * Check that a block number is in the
 * range between the I list and the size
 * of the device.
 * This is used mainly to check that a
 * garbage file system has not been mounted.
 *
 * bad block on dev x/y -- not in range
 */
badblock(fp, bn, dev)
register struct filsys *fp;
daddr_t bn;
dev_t dev;
{

	if (bn < fp->s_isize || bn >= fp->s_fsize) {
		prdev("bad block", dev);
		return(1);
	}
	return(0);
}

/*
 * Allocate an unused I node
 * on the specified device.
 * Used with file creation.
 * The algorithm keeps up to
 * NICINOD spare I nodes in the
 * super block. When this runs out,
 * a linear search through the
 * I list is instituted to pick
 * up NICINOD more.
 */
struct inode *
ialloc(dev)
dev_t dev;
{
	register struct filsys *fp;
	register struct buf *bp;
	register struct inode *ip;
	int i;
	struct dinode *dp;
	ino_t ino;
	daddr_t adr, eadr;
	ino_t	inobas;
	int	first;
	segm	mntmap1, mntmap2;

	if ((fp = getfs(dev)) == NULL)
		goto nofs;
	saveseg5(mntmap1);
loop:
	while (fp->s_ilock & S_BUSY) {
		fp->s_ilock |= S_WANTED;
		normalseg5();
		sleep((caddr_t)&fp->s_ilock, PINOD);
		restorseg5(mntmap1);
	}
	if(fp->s_ninode > 0) {
		ino = fp->s_inode[--fp->s_ninode];
		if (ino < ROOTINO)
			goto loop;
		normalseg5();
		ip = iget(dev, ino);
		if(ip == NULL)
			return(NULL);
		if(ip->i_mode == 0) {
			for (i=0; i<NADDR; i++)
				ip->i_addr[i] = 0;
			restorseg5(mntmap1);
			fp->s_fmod = 1;
			fp->s_tinode--;
			normalseg5();
			return(ip);
		}
		/*
		 * Inode was allocated after all.
		 * Look some more.
		 */
		iput(ip);
		restorseg5(mntmap1);
		goto loop;
	}
	fp->s_ilock |= S_BUSY;
	eadr = fp->s_isize;		/* search to the end of the I-list */
	if (fp->s_nbehind < 4 * NICINOD) {
		first = 1;
		ino = fp->s_lasti;
		adr = itod(ino);
	} else {
fromtop:
		first = 0;
		ino = 1;
		adr = SUPERB+1;
		fp->s_nbehind = 0;
	}
	for (; adr < eadr; adr++)
	{
		normalseg5();
		inobas = ino;
		bp = bread(dev, adr);
		if (bp->b_flags & B_ERROR) {
			brelse(bp);
			ino += INOPB;
			restorseg5(mntmap1);
			continue;
		}
		dp = (struct dinode *) mapin(bp);
		saveseg5(mntmap2);
		for (i = 0; i < INOPB; i++) {
			if (dp->di_mode != 0)
				goto cont;
# ifdef	UCB_IHASH
			if (ifind(dev, ino))
				goto cont;
# else
			for (ip = &inode[0]; ip < &inode[ninode]; ip++)
				if(dev==ip->i_dev && ino==ip->i_number)
					goto cont;
# endif
			restorseg5(mntmap1);
			fp->s_inode[fp->s_ninode++] = ino;
			if (fp->s_ninode >= NICINOD)
				break;
			restorseg5(mntmap2);
		cont:
			ino++;
			dp++;
		}
		restorseg5(mntmap1);
		brelse(bp);
		if (fp->s_ninode >= NICINOD)
			break;
	}
	if (fp->s_ninode < NICINOD && first) {
		/*
		 * Go back and start at the beginning, searching
		 * until we get where we started from.
		 */
		eadr = itod(fp->s_lasti);
		goto fromtop;
	}
	fp->s_lasti = inobas;
	if (fp->s_ilock & S_WANTED)
		wakeup((caddr_t)&fp->s_ilock);
	fp->s_ilock = 0;
	if(fp->s_ninode > 0)
		goto loop;
	normalseg5();
	prdev("Out of inodes", dev);
nofs:
	u.u_error = ENOSPC;
	return(NULL);
}

/*
 * Free the specified I node
 * on the specified device.
 * The algorithm stores up
 * to NICINOD I nodes in the super
 * block and throws away any more.
 */
ifree(dev, ino)
dev_t dev;
ino_t ino;
{
	register struct filsys *fp;


	if ((fp = getfs(dev)) == NULL)
		return;
	fp->s_tinode++;
	if (fp->s_ilock & S_BUSY)
		goto ret;
	if (fp->s_ninode >= NICINOD) {
		if (fp->s_lasti > ino)
			fp->s_nbehind++;
	} else {
		fp->s_inode[fp->s_ninode++] = ino;
		fp->s_fmod = 1;
	}
ret:
	normalseg5();
}

/*
 * getfs maps a device number into
 * a pointer to the incore super
 * block.
 * The algorithm is a linear
 * search through the mount table.
 * A consistency check of the
 * in core free-block and i-node
 * counts.
 *
 * bad count on dev x/y -- the count
 *	check failed. At this point, all
 *	the counts are zeroed which will
 *	almost certainly lead to "no space"
 *	diagnostic
 * panic: no fs -- the device is not mounted.
 *	this "cannot happen"
 *	Or so we thought, if the pipedev isn't mounted
 *	we'll get the appropriate message, so we just do
 *	a warning and return NULL.
 */
struct filsys *
getfs(dev)
dev_t dev;
{
	register struct mount *mp;
	register struct filsys *fp;

	for(mp = &mount[0]; mp < &mount[nmount]; mp++)
	if (mp->m_inodp != NULL && mp->m_dev == dev) {
		fp = mapin(mp->m_bufp);
		if(fp->s_nfree > NICFREE || fp->s_ninode > NICINOD) {
			prdev("bad count", dev);
			fp->s_nfree = 0;
			fp->s_ninode = 0;
		}
		return(fp);
	}
	prdev("no fs", dev);
	return((struct filsys *) NULL);
}

/*
 * update is the internal name of
 * 'sync'. It goes through the disk
 * queues to initiate sandbagged IO;
 * goes through the I nodes to write
 * modified nodes; and it goes through
 * the mount table to initiate modified
 * super blocks.
 */
update()
{
	register struct inode *ip;
	register struct mount *mp;
	register struct buf *bp;
	struct filsys *fp;
	struct filsys *fpdst;
	segm	mntmap;

	if(updlock)
		return;
	updlock++;
	for(mp = &mount[0]; mp < &mount[nmount]; mp++)
		if(mp->m_inodp != NULL)
		{
			fp = (struct filsys *)mapin(mp->m_bufp);
			if(fp->s_fmod==0 || (fp->s_ilock & S_BUSY) ||
			   fp->s_flock!=0 || fp->s_ronly!=0) {
				normalseg5();
				continue;
			}
			saveseg5(mntmap);
			normalseg5();
			bp = getblk(mp->m_dev, SUPERB);
			if (bp != mp->m_bufp) {
				prdev("sblock pointer changed ",mp->m_dev);
				panic("update");
			}
			if (bp->b_flags & B_ERROR)
				continue;
			restorseg5(mntmap);
			fp->s_fmod = 0;
			fp->s_time = time;
			normalseg5();
			bwrite(bp);
		}
	for(ip = &inode[0]; ip < &inode[ninode]; ip++)
		if((ip->i_flag&ILOCK)==0 && ip->i_count) {
			ip->i_flag |= ILOCK;
			ip->i_count++;
			iupdat(ip, &time, &time, 0);
			iput(ip);
		}
	updlock = 0;
	bflush(BF_FLUSH, NODEV);
}
