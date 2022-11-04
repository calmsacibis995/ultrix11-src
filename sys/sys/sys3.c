
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)sys3.c	3.0	4/21/86
 */
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/ino.h>
#include <sys/reg.h>
#include <sys/buf.h>
#include <sys/filsys.h>
#include <sys/mount.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/inode.h>
#include <sys/file.h>
#include <sys/conf.h>
#include <sys/stat.h>
#include <sys/inline.h>
#include <sys/flock.h>
#include <sys/ioctl.h>

/*
 * the fstat system call.
 */
fstat()
{
	register struct file *fp;
	register struct a {
		int	fdes;
		struct stat *sb;
	} *uap;

	uap = (struct a *)u.u_ap;
	fp = getf(uap->fdes);
	if(fp == NULL)
		return;
#ifdef	UCB_NET
	if (fp->f_flag & FSOCKET)
		u.u_error = soo_stat(fp->f_socket, uap->sb);
	else
#endif	UCB_NET
	stat1(fp->f_inode, uap->sb, 
		((fp->f_inode->i_flag&IFMT) == IFIFO)? fp->f_un.f_offset: 0);
}

/*
 * the stat system call.
 */
stat()
{
	register struct inode *ip;
	register struct a {
		char	*fname;
		struct stat *sb;
	} *uap;

	uap = (struct a *)u.u_ap;
#ifdef	UCB_SYMLINKS
	ip = namei(uchar, LOOKUP, 1);
#else	UCB_SYMLINKS
	ip = namei(uchar, LOOKUP);
#endif	UCB_SYMLINKS
	if(ip == NULL)
		return;
	stat1(ip, uap->sb, (off_t)0);
	iput(ip);
}

#ifdef	UCB_SYMLINKS
/*
 * Lstat system call; like stat but doesn't follow links.
 */
lstat()
{
	register struct inode *ip;
	register struct a {
		char	*fname;
		struct stat *sb;
	} *uap;

	uap = (struct a *)u.u_ap;
	ip = namei(uchar, 0, 0);
	if (ip == NULL)
		return;
	stat1(ip, uap->sb, (off_t)0);
	iput(ip);
}
#endif

/*
 * The basic routine for fstat and stat:
 * get the inode and pass appropriate parts back.
 */
static
stat1(ip, ub, pipeadj)
register struct inode *ip;
struct stat *ub;
off_t pipeadj;
{
	register struct dinode *dp;
	register struct buf *bp;
	struct stat ds;

	IUPDAT(ip, &time, &time, 0);
	/*
	 * first copy from inode table
	 */
	ds.st_dev = ip->i_dev;
	ds.st_ino = ip->i_number;
	ds.st_mode = ip->i_mode;
	ds.st_nlink = ip->i_nlink;
	ds.st_uid = ip->i_uid;
	ds.st_gid = ip->i_gid;
	ds.st_rdev = (dev_t)ip->i_rdev;
	ds.st_size = ip->i_size - pipeadj;
	/*
	 * next the dates in the disk
	 */
	bp = bread(ip->i_dev, itod(ip->i_number));
	dp = (struct dinode *) mapin(bp);
	dp += itoo(ip->i_number);
	ds.st_atime = dp->di_atime;
	ds.st_mtime = dp->di_mtime;
	ds.st_ctime = dp->di_ctime;
	mapout(bp);
	brelse(bp);
	if (copyout((caddr_t)&ds, (caddr_t)ub, sizeof(ds)) < 0)
		u.u_error = EFAULT;
}

/*
 * the dup system call.
 */
dup()
{
	register struct file *fp;
	register struct a {
		int	fdes;
		int	fdes2;
	} *uap;
	register i, m;

	uap = (struct a *)u.u_ap;
	m = uap->fdes & ~077;
	uap->fdes &= 077;
	fp = getf(uap->fdes);
	if(fp == NULL)
		return;
	if ((m&0100) == 0) {
		if ((i = ufalloc(0)) < 0)
			return;
	} else {
		i = uap->fdes2;
		if (i<0 || i>=NOFILE) {
			u.u_error = EBADF;
			return;
		}
		u.u_rval1 = i;
	}
	if (i!=uap->fdes) {
		if (u.u_ofile[i]!=NULL)
#ifndef	UCB_NET
			closef(u.u_ofile[i]);
#else	UCB_NET
			closef(u.u_ofile[i], 0);
#endif	UCB_NET
		u.u_ofile[i] = fp;
		fp->f_count++;
	}
}

/*
 * the file control system call.
 */
fcntl()
{
	register struct file *fp;
	register struct a {
		int	fdes;
		int	cmd;
		int	arg;
	} *uap;
	register i;
 	struct flock bf;

	uap = (struct a *)u.u_ap;
	fp = getf(uap->fdes);
	if (fp == NULL)
		return;
	switch(uap->cmd) {
	case 0:			/* F_DUPFD */
		i = uap->arg;
		if (i < 0 || i > NOFILE) {
			u.u_error = EINVAL;
			return;
		}
		if ((i = ufalloc(i)) < 0)
			return;
		u.u_ofile[i] = fp;
		fp->f_count++;
		break;

	case 1:			/* F_GETFD */
		u.u_rval1 = u.u_pofile[uap->fdes];
		break;

	case 2:			/* F_SETFD */
		u.u_pofile[uap->fdes] = uap->arg;
		break;

	case 3:			/* F_GETFL */
		u.u_rval1 = fp->f_flag+FOPEN;
		break;

	case 4:			/* F_SETFL */
		fp->f_flag &= (FREAD|FWRITE);
		fp->f_flag |= (uap->arg-FOPEN) & ~(FREAD|FWRITE);
		if ((uap->arg-FOPEN) & FNDELAY) {
			uap->arg = 1;
		}
		else
			uap->arg = 0;
		uap->cmd = FKRNBIO;
		ioctl();
		u.u_error = 0;   /* ioctl may fail for some valid reasons
				  * like FNDELAY for a pipe */
		break;

 	case 5:			/* F_GETLK */
 		/* get record lock */
 		if (copyin(uap->arg, &bf, sizeof bf))
 			u.u_error = EFAULT;
 		else if ((i=getflck(fp, &bf)) != 0)
 			u.u_error = i;
 		else if (copyout(&bf, uap->arg, sizeof bf))
 			u.u_error = EFAULT;
 		break;
 
 	case 6:			/* F_SETLK */
 		/* set record lock and return if blocked */
 		if (copyin(uap->arg, &bf, sizeof bf))
 			u.u_error = EFAULT;
 		else if ((i=setflck(fp, &bf, 0)) != 0)
 			u.u_error = i;
 		break;
 
 	case 7:			/* F_SETLKW */
 		/* set record lock and wait if blocked */
 		if (copyin(uap->arg, &bf, sizeof bf))
 			u.u_error = EFAULT;
 		else if ((i=setflck(fp, &bf, 1)) != 0)
 			u.u_error = i;
 		break;
 
	default:
		u.u_error = EINVAL;
	}
}

/*
 * the mount system call.
 */
smount()
{
	dev_t dev;
	register struct inode *ip;
	register struct mount *mp;
	struct mount *smp;
	register struct filsys *fp;
	struct buf *bp;
	register struct a {
		char	*fspec;
		char	*freg;
		int	ronly;
	} *uap;

	uap = (struct a *)u.u_ap;
	dev = getmdev();
	if(u.u_error || !suser())
		return;
	u.u_dirp = (caddr_t)uap->freg;
#ifdef	UCB_SYMLINKS
	ip = namei(uchar, LOOKUP, 1);
#else	UCB_SYMLINKS
	ip = namei(uchar, LOOKUP);
#endif	UCB_SYMLINKS
	if(ip == NULL)
		return;
	if(ip->i_count!=1 || (ip->i_mode&(IFBLK&IFCHR))!=0)
		goto out;
	smp = NULL;
	for(mp = &mount[0]; mp < &mount[nmount]; mp++) {
		if(mp->m_inodp != NULL)
		{
			if(dev == mp->m_dev)
				goto out;
		} else
		if(smp == NULL)
			smp = mp;
	}
	mp = smp;
	if(mp == NULL)
		goto out;
	(*bdevsw[major(dev)].d_open)(dev, !uap->ronly);
	if(u.u_error)
		goto out;
	bp = bread(dev, SUPERB);
	if(u.u_error) {
		brelse(bp);
		goto out1;
	}
	if (!uap->ronly) {
		/*
		 * Write it back out.  We don't call bwrite() because
		 * it will release the buffer, and we don't want to
		 * do that. We've just taken the code that we need.
		 * If there are any errors, we assume that it is
		 * because the disk is write-locked, so bomb out.
		 */
		bp->b_flags &= ~(B_READ|B_DONE|B_ERROR|B_DELWRI);
		(*bdevsw[major(dev)].d_strategy)(bp);
		iowait(bp);
		if (bp->b_flags & B_ERROR) {
			brelse(bp);
			u.u_error = EROFS;
			goto out1;
		}
	}
	/*
	 * Sanity check some superblock values,
	 * so we don't mount corrupted or non UNIX file systems.
	 * The checks could be more stringent, but we don't
	 * want to risk limiting file system sizes.
	 * Max file system size assumed to be 2^24 blocks.
	 */
	fp = mapin(bp);
	if((fp->s_isize <= (SUPERB+1)) ||
	   (fp->s_fsize <= 0L) ||
	   (fp->s_fsize > 16777216L) ||
	   (fp->s_isize >= fp->s_fsize)) {
		mapout(bp);
		brelse(bp);
		u.u_error = EBADFS;
		goto out1;
	}
	mp->m_inodp = ip;
	mp->m_dev = dev;
	bp->b_flags |= B_MOUNT;
	mp->m_bufp = bp;
	fp->s_ilock = 0;
	fp->s_flock = 0;
	fp->s_ronly = uap->ronly & 1;
	fp->s_nbehind = 0;
	fp->s_lasti = 1;
	mapout(bp);
	brelse(bp);
	ip->i_flag |= IMOUNT;
	prele(ip);
	return;

out:
	u.u_error = EBUSY;
out1:
	iput(ip);
}

/*
 * the umount system call.
 */
sumount()
{
	dev_t dev;
	register struct inode *ip;
	register struct mount *mp;
	struct buf *bp;
	struct buf *dp;
	register struct a {
		char	*fspec;
	};

	dev = getmdev();
	if(u.u_error || !suser())
		return;
	xumount(dev);	/* remove unused sticky files from text table */
	update();
	for(mp = &mount[0]; mp < &mount[nmount]; mp++)
		if(mp->m_inodp != NULL && dev == mp->m_dev)
			goto found;
	u.u_error = EINVAL;
	return;

found:
	for(ip = &inode[0]; ip < &inode[ninode]; ip++)
		if(ip->i_number != 0 && dev == ip->i_dev) {
			u.u_error = EBUSY;
			return;
		}
	(*bdevsw[major(dev)].d_close)(dev, 0);
	/*
	 * the call to update might have just returned, so at this
	 * point we don't know if the flushing is done yet.  So we
	 * call bflush ourselves, and we then know that everything
	 * is flushed.
	 */
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
	ip = mp->m_inodp;
	ip->i_flag &= ~IMOUNT;
	plock(ip);
	iput(ip);
	mp->m_inodp = NULL;
	bp = mp->m_bufp;
	mp->m_bufp = NULL;
	bp->b_flags &= ~B_MOUNT;
	brelse(bp);
}

/*
 * Common code for mount and umount.
 * Check that the user's argument is a reasonable
 * thing on which to mount, and return the device number if so.
 */
static dev_t
getmdev()
{
	dev_t dev;
	register struct inode *ip;

#ifdef	UCB_SYMLINKS
	ip = namei(uchar, LOOKUP, 1);
#else	UCB_SYMLINKS
	ip = namei(uchar, LOOKUP);
#endif	UCB_SYMLINKS
	if(ip == NULL)
		return(NODEV);
	if((ip->i_mode&IFMT) != IFBLK)
		u.u_error = ENOTBLK;
	dev = (dev_t)ip->i_rdev;
	if(major(dev) >= nblkdev)
		u.u_error = ENXIO;
	iput(ip);
	return(dev);
}


#include <sys/utsname.h>

utssys()
{

	register i;
	register struct a {
		char	*cbuf;
		int	mv;
		int	type;
	} *uap;
	register struct mount *mp;

	uap = (struct a *)u.u_ap;
	switch(uap->type) {

case 0:		/* uname */
	if (copyout(&utsname, uap->cbuf, sizeof(struct utsname)))
		u.u_error = EFAULT;
	return;

case 2:		/* ustat */
	for(mp = &mount[i]; mp < &mount[nmount]; mp++) {
		if(mp->m_inodp != NULL && mp->m_dev==uap->mv) {
			register struct filsys *fp;

			fp = (struct filsys *)(mapin(mp->m_bufp));

/* Orig system III code. Our super block is different. OHMS		
			if(copyout(&fp->s_tfree, uap->cbuf, 18))
				u.u_error = EFAULT;
*/

			if(copyout(&fp->s_tfree, uap->cbuf, 6))
				u.u_error = EFAULT;
			if(copyout(&fp->s_fname, (uap->cbuf+6), 12))
				u.u_error = EFAULT;
			mapout(mp->m_bufp);
			return;
		}
	}
	u.u_error = EINVAL;
	return;

default:
	u.u_error = EFAULT;
	}
}

#ifdef	UCB_SYMLINKS
/*
 * Return target name of a symbolic link
 */
readlink()
{
	register struct inode *ip;
	register struct a {
		char	*name;
		char	*buf;
		int	count;
	} *uap;

	ip = namei(uchar, 0, 0);
	if (ip == NULL)
		return;
	if ((ip->i_mode&IFMT) != IFLNK) {
		u.u_error = ENXIO;
		goto out;
	}
	uap = (struct a *)u.u_ap;
	u.u_offset = 0;
	u.u_base = uap->buf;
	u.u_count = uap->count;
	u.u_segflg = 0;
	readi(ip);
out:
	iput(ip);
	u.u_r.r_val1 = uap->count - u.u_count;
}

/*
 * symlink -- make a symbolic link
 */
symlink()
{
	register struct a {
		char	*target;
		char	*linkname;
	} *uap;
	register struct inode *ip;
	register char *tp;
	register c, nc;

	uap = (struct a *)u.u_ap;
	tp = uap->target;
	nc = 0;
	while (c = fubyte(tp)) {
		if (c < 0) {
			u.u_error = EFAULT;
			return;
		}
		tp++;
		nc++;
	}
	u.u_dirp = uap->linkname;
	ip = namei(uchar, 1, 0);
	if (ip) {
		iput(ip);
		u.u_error = EEXIST;
		return;
	}
	if (u.u_error)
		return;
	ip = maknode(IFLNK | 0777);
	if (ip == NULL)
		return;
	u.u_base = uap->target;
	u.u_count = nc;
	u.u_offset = 0;
	u.u_segflg = 0;
	writei(ip);
	iput(ip);
}
#endif	UCB_SYMLINKS
