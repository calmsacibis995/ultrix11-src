
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)rdwri.c	3.0	4/21/86
 */
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/inode.h>
#include <sys/file.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/buf.h>
#include <sys/conf.h>

/*
 * Read the file corresponding to
 * the inode pointed at by the argument.
 * The actual read arguments are found
 * in the variables:
 *	u_base		core address for destination
 *	u_offset	byte offset in file
 *	u_count		number of bytes to read
 *	u_segflg	read to kernel/user/user I
 */
readi(ip)
register struct inode *ip;
{
	struct buf *bp;
	dev_t dev;
	daddr_t lbn, bn;
	off_t diff;
	register on, n;
	register type;

	if(u.u_count == 0)
		return;
	if(u.u_offset < 0) {
		u.u_error = EINVAL;
		return;
	}
	dev = (dev_t)ip->i_rdev;
	type = ip->i_mode&IFMT;

	switch(type) {
	case IFCHR:
		ip->i_flag |= IACC;
		(*cdevsw[major(dev)].d_read)(dev);
		break;
	case IFIFO:
		while (ip->i_size == 0) {
			if (ip->i_fwcnt == 0)
				return;
			if (u.u_fmode&FNDELAY)
				return;
			ip->i_fflag |= IFIR;
			prele(ip);
			sleep((caddr_t)&ip->i_frcnt, PPIPE);
			plock(ip);
		}
		u.u_offset = ip->i_frptr;

	case IFBLK:
	case IFREG:
	case IFDIR:
	case IFLNK:
	do {
		lbn = bn = u.u_offset >> BSHIFT;
		on = u.u_offset & BMASK;
		n = MIN((unsigned)(BSIZE-on), u.u_count);
		if (type != IFBLK) {
			if (type == IFIFO)
				diff = ip->i_size;
			else
				diff = ip->i_size - u.u_offset;
			if (diff <= 0)
				break;
			if (diff < n)
				n = diff;
			bn = bmap(ip, bn, B_READ);
			if (u.u_error)
				break;
			dev = ip->i_dev;
		} else
			rablock = bn+1;
#ifndef	SELECT
		if ((long)bn<0) {
			if (type != IFREG)
				break;
			bp = geteblk();
			clrbuf(bp);
			bp->b_resid = 0;
		} else if (ip->i_lastr+1 == lbn &&
		    (type != IFIFO) && (on+n) == BSIZE)
			bp = breada(dev, bn, rablock);
		else
			bp = bread(dev, bn);
		if ((on+n) == BSIZE)
			ip->i_lastr = lbn;
#else	SELECT
		/*
		 * On fifo's we use the i_lastr data area for keeping
		 * track of people doing selects, so we can't touch it.
		 */
		if ((long)bn<0) {
			if (type != IFREG)
				break;
			bp = geteblk();
			clrbuf(bp);
			bp->b_resid = 0;
			if ((on+n) == BSIZE)
				ip->i_lastr = lbn;
		} else if (type != IFIFO && (on+n) == BSIZE) {
			if (ip->i_lastr+1 == lbn)
				bp = breada(dev, bn, rablock);
			else
				bp = bread(dev, bn);
			ip->i_lastr = lbn;
		} else
			bp = bread(dev, bn);
#endif	SELECT
		if (bp->b_resid) {
			n = 0;
		}
		if (n!=0) {
			pimove(PADDR(bp)+on, n, B_READ);
		}
		if (type == IFIFO) {
			ip->i_size -= n;
			if (u.u_offset >= PIPSIZ)
				u.u_offset = 0;
			if ((on+n) == BSIZE && ip->i_size < (PIPSIZ-BSIZE))
				bp->b_flags &= ~B_DELWRI;
		}
		brelse(bp);
		ip->i_flag |= IACC;
	} while (u.u_error==0 && u.u_count!=0 && n!=0);
		if (type == IFIFO) {
			if (ip->i_size)
				ip->i_frptr = u.u_offset;
			else
				ip->i_frptr = ip->i_fwptr = 0;
			if (ip->i_fflag&IFIW) {
				ip->i_fflag &= ~IFIW;
				curpri = PPIPE;
				wakeup((caddr_t)&ip->i_fwcnt);
			}
#ifdef	SELECT
			if (ip->i_frsel) {
				selwakeup(ip->i_frsel, ip->i_fflag&IFI_RCOLL);
				ip->i_fflag &= ~IFI_RCOLL;
				ip->i_frsel = 0;
			}
#endif	SELECT
		}
		break;

	default:
		u.u_error = ENODEV;
	}
}

/*
 * Write the file corresponding to
 * the inode pointed at by the argument.
 * The actual write arguments are found
 * in the variables:
 *	u_base		core address for source
 *	u_offset	byte offset in file
 *	u_count		number of bytes to write
 *	u_segflg	write to kernel/user/user I
 */
writei(ip)
register struct inode *ip;
{
	struct buf *bp;
	dev_t dev;
	daddr_t bn;
	register n, on;
	register type;
	unsigned usave;
	struct	direct *dp;
	struct buf *tdp;

	if(u.u_offset < 0) {
		u.u_error = EINVAL;
		return;
	}
	dev = (dev_t)ip->i_rdev;
	type = ip->i_mode&IFMT;
	switch(type) {
	case IFCHR:
		ip->i_flag |= IUPD|ICHG;
		(*cdevsw[major(dev)].d_write)(dev);
		break;

	case IFIFO:
	floop:
		usave = 0;
		while ((u.u_count+ip->i_size) > PIPSIZ) {
			if (ip->i_frcnt == 0)
				break;
			if ((u.u_count > PIPSIZ) && (ip->i_size < PIPSIZ)) {
				usave = u.u_count;
				u.u_count = PIPSIZ - ip->i_size;
				usave -= u.u_count;
				break;
			}
			if (u.u_fmode&FNDELAY)
				return;
			ip->i_fflag |= IFIW;
			prele(ip);
			sleep((caddr_t)&ip->i_fwcnt, PPIPE);
			plock(ip);
		}
		if (ip->i_frcnt == 0) {
			u.u_error = EPIPE;
			psignal(u.u_procp, SIGPIPE);
			break;
		}
		u.u_offset = ip->i_fwptr;

	case IFBLK:
	case IFREG:
	case IFDIR:
	case IFLNK:
	while (u.u_error==0 && u.u_count!=0) {
		bn = u.u_offset >> BSHIFT;
		on = u.u_offset & BMASK;
		n = MIN((unsigned)(BSIZE-on), u.u_count);
		if (type != IFBLK) {
			bn = bmap(ip, bn, B_WRITE);
			if (u.u_error)
				break;
			dev = ip->i_dev;
		}
		if (n == BSIZE) 
			bp = getblk(dev, bn);
		else if (type==IFIFO && on==0 && ip->i_size < (PIPSIZ-BSIZE))
			bp = getblk(dev, bn);
		else
			bp = bread(dev, bn);
		pimove(PADDR(bp)+on, n, B_WRITE);
		if (u.u_error) {
			brelse(bp);
		} else {
			dp = (struct direct *)((unsigned)mapin(bp)+on);
			if ((ip->i_mode&IFMT) == IFDIR && (dp->d_ino == 0)) {
				mapout(bp);
				/*
				 * Writing to clear a directory entry.
				 * Must insure the write occurs before
				 * the inode is freed, or may end up
				 * pointing at a new (different) file
				 * if inode is quickly allocated again
				 * and system crashes.
				 */
				bwrite(bp);
			} else {
				mapout(bp);
				tdp = bdevsw[major(bp->b_dev)].d_tab;
				tdp += dp_adj(bp->b_dev);
				if(((u.u_offset & BMASK) == 0) &&
				   ((tdp->b_flags&B_TAPE) == 0)) {
					bp->b_flags |= B_AGE;
					bawrite(bp);
				} else
					bdwrite(bp);
			}
		}
#ifdef	UCB_SYMLINKS
		if (type == IFREG || type == IFDIR || type == IFLNK)
#else	UCB_SYMLINKS
		if (type == IFREG || type == IFDIR)
#endif	UCB_SYMLINKS
		{
			if (u.u_offset > ip->i_size)
				ip->i_size = u.u_offset;
		} else if (type == IFIFO) {
			ip->i_size += n;
			if (u.u_offset == PIPSIZ)
				u.u_offset = 0;
		}
		ip->i_flag |= IUPD|ICHG;
	}
		if (type == IFIFO) {
			ip->i_fwptr = u.u_offset;
			if (ip->i_fflag&IFIR) {
				ip->i_fflag &= ~IFIR;
				curpri = PPIPE;
				wakeup((caddr_t)&ip->i_frcnt);
			}
#ifdef	SELECT
			if (ip->i_fwsel) {
				selwakeup(ip->i_fwsel, ip->i_fflag&IFI_WCOLL);
				ip->i_fwsel = 0;
				ip->i_fflag &= ~IFI_WCOLL;
			}
#endif	SELECT
			if (u.u_error==0 && usave!=0) {
				u.u_count = usave;
				goto floop;
			}
		}
		break;

	default:
		u.u_error = ENODEV;
	}
}

#ifndef	MAX
/*
 * Return the logical maximum
 * of the 2 arguments.
 */
max(a, b)
unsigned a, b;
{

	if(a > b)
		return(a);
	return(b);
}

/*
 * Return the logical minimum
 * of the 2 arguments.
 */
min(a, b)
unsigned a, b;
{

	if(a < b)
		return(a);
	return(b);
}
#endif

/*
 * Move n bytes at byte location
 * cp to/from (flag) the
 * user/kernel (u.u_segflg) area starting at u.u_base.
 * Update all the arguments by the number
 * of bytes moved.
 */
pimove(cp, n, flag)
long cp;
register unsigned n;
{
	if (copyio(cp, u.u_base, n, (u.u_segflg<<1)|flag))
		u.u_error = EFAULT;
	else {
		u.u_base += n;
		u.u_offset += n;
		u.u_count -= n;
	}
}
