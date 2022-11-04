
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)iget.c	3.0	5/5/86
 */
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/inode.h>
#include <sys/ino.h>
#include <sys/filsys.h>
#include <sys/mount.h>
#include <sys/conf.h>
#include <sys/buf.h>
#include <sys/seg.h>
#include <sys/inline.h>

#ifdef	UCB_IHASH
#define	INOHSZ	64		/* must be a power of two */
#define	INOHASH(dev,ino)	(((dev) + (ino)) & (INOHSZ -1))
struct	inode	*ihash[INOHSZ];
struct	inode	*ifreelist;

/*
 * Initialize hash links for inodes
 * and build inode free list.
 */
ihinit()
{
	register struct inode *ip;

	ifreelist = &inode[0];
	for(ip = &inode[0]; ip < &inode[ninode-1]; ip++)
		ip->i_link = ip + 1;
}

/*
 * Find an inode if it is incore.
 * This is the equivalent, for inodes,
 * of ``incore'' in bio.c or ``pfind'' in subr.c.
 */
struct inode *
ifind(dev, ino)
dev_t dev;
ino_t ino;
{
	register struct inode *ip;

	for (ip = ihash[INOHASH(dev,ino)]; ip != NULL;
	    ip = ip->i_link)
		if (ino==ip->i_number && dev ==ip->i_dev)
			return(ip);
	return ((struct inode *)NULL);
}

#endif	UCB_IHASH

/*
 * Look up an inode by device,inumber.
 * If it is in core (in the inode structure),
 * honor the locking protocol.
 * If it is not in core, read it in from the
 * specified device.
 * If the inode is mounted on, perform
 * the indicated indirection.
 * In all cases, a pointer to a locked
 * inode structure is returned.
 *
 * printf warning: no inodes -- if the inode
 *	structure is full
 * panic: no imt -- if the mounted file
 *	system is not in the mount table.
 *	"cannot happen"
 */
struct inode *
iget(dev, ino)
dev_t dev;
ino_t ino;
{
	register struct inode *ip;
	register struct mount *mp;
#ifdef	UCB_IHASH
	register int slot;
#else
	register struct inode *oip;
#endif
	register struct buf *bp;
	register struct dinode *dp;

loop:
#ifdef	UCB_IHASH
	slot = INOHASH(dev, ino);
	ip = ihash[slot];
	while (ip != NULL)
#else
	oip = NULL;
	for(ip = &inode[0]; ip < &inode[ninode]; ip++)
#endif
	{
		if(ino == ip->i_number && dev == ip->i_dev) {
	again:
			if((ip->i_flag&ILOCK) != 0) {
				ip->i_flag |= IWANT;
				sleep((caddr_t)ip, PINOD);
				if(ino == ip->i_number && dev == ip->i_dev)
					goto again;
				goto loop;
			}
			if((ip->i_flag&IMOUNT) != 0) {
				for(mp = &mount[0]; mp < &mount[nmount]; mp++)
				if(mp->m_inodp == ip) {
					dev = mp->m_dev;
					ino = ROOTINO;
					goto loop;
				}
				panic("no imt");
			}
			ip->i_count++;
			ip->i_flag |= ILOCK;
			return(ip);
		}
#ifdef	UCB_IHASH
		ip = ip->i_link;
#else
		if(oip==NULL && ip->i_count==0)
			oip = ip;
#endif
	}
#ifdef	UCB_IHASH
	if (ifreelist == NULL)
#else
	ip = oip;
	if(ip == NULL)
#endif
	{
		printf("Inode table overflow\n");
		u.u_error = ENFILE;
		return(NULL);
	}
#ifdef	UCB_IHASH
	ip = ifreelist;
	ifreelist = ip->i_link;
	ip->i_link = ihash[slot];
	ihash[slot] = ip;
#endif
	ip->i_dev = dev;
	ip->i_number = ino;
	ip->i_flag = ILOCK;
	ip->i_count++;
	ip->i_lastr = 0;
	bp = bread(dev, itod(ino));
	/*
	 * Check I/O errors
	 */
	if((bp->b_flags&B_ERROR) != 0) {
		brelse(bp);
		/*
		 * This was an iput(ip), but if i_nlink happens to be 0,
		 * the itrunk() in iput() might be successful.
		 */
		ip->i_count = 0;
		ip->i_number = 0;
		ip->i_flag = 0;
#ifdef	UCB_IHASH
		ihash[slot] = ip->i_link;
		ip->i_link = ifreelist;
		ifreelist = ip;
#endif
		return(NULL);
	}
	dp = (struct dinode *) mapin(bp);
	dp += itoo(ino);
	iexpand(ip, dp);
	mapout(bp);
	brelse(bp);
	return(ip);
}

static
iexpand(ip, dp)
register struct inode *ip;
register struct dinode *dp;
{
	register char *p1;
	char *p2;
	int i;

	ip->i_mode = dp->di_mode;
	ip->i_nlink = dp->di_nlink;
	ip->i_uid = dp->di_uid;
	ip->i_gid = dp->di_gid;
	ip->i_size = dp->di_size;
	p1 = (char *)ip->i_addr;
	p2 = (char *)dp->di_addr;
	for(i=0; i<NADDR; i++) {
		*p1++ = *p2++;
		*p1++ = 0;
		*p1++ = *p2++;
		*p1++ = *p2++;
	}
}

/*
 * Decrement reference count of
 * an inode structure.
 * On the last reference,
 * write the inode out and if necessary,
 * truncate and deallocate the file.
 */
iput(ip)
register struct inode *ip;
{
#ifdef	UCB_IHASH
	register struct inode *jp;
	register int i;
#endif

	if(ip->i_count == 1) {
		ip->i_flag |= ILOCK;
		if(ip->i_nlink <= 0) {
			itrunc(ip);
			ip->i_mode = 0;
			ip->i_flag |= IUPD|ICHG;
			ifree(ip->i_dev, ip->i_number);
		}
		IUPDAT(ip, &time, &time, 0);
		prele(ip);
#ifdef	UCB_IHASH
		i = INOHASH(ip->i_dev, ip->i_number);
		if (ihash[i] == ip)
			ihash[i] = ip->i_link;
		else {
			for (jp = ihash[i]; jp != NULL; jp = jp->i_link)
				if (jp->i_link == ip) {
					jp->i_link = ip->i_link;
					goto done;
				}
			panic("iput");
		}
	done:
		ip->i_link = ifreelist;
		ifreelist = ip;
#endif	UCB_IHASH
		ip->i_flag = 0;
		ip->i_number = 0;
	} else
		prele(ip);
	ip->i_count--;
}

/*
 * Check accessed and update flags on
 * an inode structure.
 * If any are on, update the inode
 * with the current time.
 */
/*
 * If waitfor set, then must insure
 * i/o order by waiting for the write
 * to complete.
 */
iupdat(ip, ta, tm, waitfor)
register struct inode *ip;
time_t *ta, *tm;
int waitfor;
{
	register struct buf *bp;
	struct dinode *dp;
	struct filsys *fp;
	register char *p1;
	char *p2;
	int i;

	if((ip->i_flag&(IUPD|IACC|ICHG)) != 0) {
		if ((fp = getfs(ip->i_dev)) == NULL)
			return;
		i = fp->s_ronly;
		normalseg5();
		if (i)
			return;
		bp = bread(ip->i_dev, itod(ip->i_number));
		if (bp->b_flags & B_ERROR) {
			brelse(bp);
			return;
		}
		dp = (struct dinode *) mapin(bp);
		dp += itoo(ip->i_number);
		dp->di_mode = ip->i_mode;
		dp->di_nlink = ip->i_nlink;
		dp->di_uid = ip->i_uid;
		dp->di_gid = ip->i_gid;
		dp->di_size = ip->i_size;
		p1 = (char *)dp->di_addr;
		p2 = (char *)ip->i_addr;
		if ((ip->i_mode&IFMT)==IFIFO) {
			for(i=0; i<NFADDR; i++) {
				*p1++ = *p2++;
				if(*p2++ != 0) {
	printf("iaddress[%d] > 2^24(%D), i_number = %d, i_dev = %o\n",
		i, ip->i_addr[i], ip->i_number, ip->i_dev);
				}
				*p1++ = *p2++;
				*p1++ = *p2++;
			}
			for(;i<NADDR; i++) {
				*p1++ = 0;
				*p1++ = 0;
				*p1++ = 0;
			}
		} else 
		for(i=0; i<NADDR; i++) {
			*p1++ = *p2++;
			if(*p2++ != 0) {
	printf("iaddress[%d] > 2^24(%D), i_number = %d, i_dev = %o\n",
		i, ip->i_addr[i], ip->i_number, ip->i_dev);
			}
			*p1++ = *p2++;
			*p1++ = *p2++;
		}
		if(ip->i_flag&IACC)
			dp->di_atime = *ta;
		if(ip->i_flag&IUPD)
			dp->di_mtime = *tm;
		if(ip->i_flag&ICHG)
			dp->di_ctime = time;
		ip->i_flag &= ~(IUPD|IACC|ICHG);
		mapout(bp);
		if (waitfor)
			bwrite(bp);
		else
			bdwrite(bp);
	}
}

/*
 * Free all the disk blocks associated
 * with the specified inode structure.
 * The blocks of the file are removed
 * in reverse order. This FILO
 * algorithm will tend to maintain
 * a contiguous free list much longer
 * than FIFO.
 */
itrunc(ip)
register struct inode *ip;
{
	register i;
	dev_t dev;
	daddr_t bn;
	struct inode itmp;

	i = ip->i_mode & IFMT;
#ifdef	UCB_SYMLINKS
	if (i!=IFREG && i!=IFDIR && i!=IFLNK)
#else	UCB_SYMLINKS
	if (i!=IFREG && i!=IFDIR)
#endif	UCB_SYMLINKS
		return;
	/*
	 * Clean inode on disk before freeing blocks
	 * to insure no duplicates if system crashes.
	 */
	itmp = *ip;
	itmp.i_size = 0;
	for (i = 0; i < NADDR; i++)
		itmp.i_addr[i] = 0;
	itmp.i_flag |= ICHG|IUPD;
	iupdat(&itmp, &time, &time, 1);
	ip->i_flag &= ~(IUPD|IACC|ICHG);

	/*
	 * Now return blocks to free list... if machine
	 * crashes, they will be harmless MISSING blocks.
	 */

	dev = ip->i_dev;
	for(i=NADDR-1; i>=0; i--) {
		bn = ip->i_addr[i];
		if(bn == (daddr_t)0)
			continue;
		ip->i_addr[i] = (daddr_t)0;
		switch(i) {

		default:
			free(dev, bn);
			break;

		case NADDR-3:
			tloop(dev, bn, 0, 0);
			break;

		case NADDR-2:
			tloop(dev, bn, 1, 0);
			break;

		case NADDR-1:
			tloop(dev, bn, 1, 1);
		}
	}
	ip->i_size = 0;
	/*
	 * Inode was written and flags updated above.
	 * No need to modify flags here.
	 */
}

static
tloop(dev, bn, f1, f2)
dev_t dev;
daddr_t bn;
{
	register i;
	register struct buf *bp;
	register daddr_t *bap;
	daddr_t nb;

	bp = NULL;
	for(i=NINDIR-1; i>=0; i--) {
		if(bp == NULL) {
			bp = bread(dev, bn);
			if (bp->b_flags & B_ERROR) {
				brelse(bp);
				return;
			}
		}
		bap = (daddr_t *) mapin(bp);
		nb = bap[i];
		mapout(bp);
		if(nb == (daddr_t)0)
			continue;
		if(f1) {
			brelse(bp);
			bp = NULL;
			tloop(dev, nb, f2, 0);
		} else
			free(dev, nb);
	}
	if(bp != NULL)
		brelse(bp);
	free(dev, bn);
}

/*
 * Make a new file.
 */
struct inode *
maknode(mode)
{
	register struct inode *ip;

	ip = ialloc(u.u_pdir->i_dev);
	if(ip == NULL) {
		iput(u.u_pdir);
		return(NULL);
	}
	ip->i_flag |= IACC|IUPD|ICHG;
	if((mode&IFMT) == 0)
		mode |= IFREG;
	ip->i_mode = mode & ~u.u_cmask;
	ip->i_nlink = 1;
	ip->i_uid = u.u_uid;
	ip->i_gid = u.u_gid;
	/*
	 * Make sure inode goes to disk before directory entry.
	 */
	iupdat(ip, &time, &time, 1);
	wdir(ip);
	return(ip);
}

/*
 * Write a directory entry with
 * parameters left as side effects
 * to a call to namei.
 */
wdir(ip)
struct inode *ip;
{

	if (u.u_pdir->i_nlink <= 0) {
		u.u_error = ENOTDIR;
		goto out;
	}
	u.u_dent.d_ino = ip->i_number;
	bcopy((caddr_t)u.u_dbuf, (caddr_t)u.u_dent.d_name, DIRSIZ);
	u.u_count = sizeof(struct direct);
	u.u_segflg = 1;
	u.u_base = (caddr_t)&u.u_dent;
	writei(u.u_pdir);
out:
	iput(u.u_pdir);
}
