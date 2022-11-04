
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)sys2.c	3.1	7/21/87
 */
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/reg.h>
#include <sys/file.h>
#include <sys/inode.h>
#include <sys/proc.h>

/*
 * read system call
 */
read()
{
	rdwr(FREAD);
}

/*
 * write system call
 */
write()
{
	rdwr(FWRITE);
}

/*
 * common code for read and write calls:
 * check permissions, set base, count, and offset,
 * and switch out to readi or writei.
 */
static
rdwr(mode)
register mode;
{
	register struct file *fp;
	register struct inode *ip;
	register struct a {
		int	fdes;
		char	*cbuf;
		unsigned count;
	} *uap;
	int type;

	uap = (struct a *)u.u_ap;
	fp = getf(uap->fdes);
	if(fp == NULL)
		return;
	if((fp->f_flag&mode) == 0) {
		u.u_error = EBADF;
		return;
	}
	u.u_base = (caddr_t)uap->cbuf;
	u.u_count = uap->count;
	u.u_segflg = 0;
	u.u_fmode = fp->f_flag;
	if ((u.u_procp->p_flag & SNUSIG) && save(u.u_qsav)) {
		if (u.u_count == uap->count)
			u.u_eosys = RESTARTSYS;
#ifdef	UCB_NET
	} else if (fp->f_flag & FSOCKET) {
		if (mode == FREAD)
			u.u_error = soreceive(fp->f_socket, 0, 0, 0);
		else
			u.u_error = sosend(fp->f_socket, 0, 0, 0);
#endif	UCB_NET
	} else {
		ip = fp->f_inode;
		type = ip->i_mode&IFMT;
		if (type==IFREG || type==IFDIR) {
			if ((u.u_fmode&FAPPEND) && (mode == FWRITE))
				fp->f_un.f_offset = ip->i_size;
		} else if (type == IFIFO)
			fp->f_un.f_offset = 0;
		u.u_offset = fp->f_un.f_offset;
		if((ip->i_mode&(IFCHR&IFBLK)) == 0)
			plock(ip);
		if(mode == FREAD)
			readi(ip);
		else
			writei(ip);
		if((ip->i_mode&(IFCHR&IFBLK)) == 0)
			prele(ip);
		if(type != IFIFO)  
			fp->f_un.f_offset += uap->count-u.u_count;
	}
	u.u_rval1 = uap->count-u.u_count;
}

/*
 * open system call
 * Ohms 2/5/85 - modified for sys3/5 extended open
 */
open()
{
	register struct a {
		char	*fname;
		int	rwmode;
		int	crtmode;
	} *uap;

	uap = (struct a *)u.u_ap;
	open1(uap->rwmode-FOPEN, uap->crtmode); 
}

/*
 * creat system call
 * Ohms 2/5/85 - modified for sys3/5 compatability
 */
creat()
{
	register struct a {
		char	*fname;
		int	fmode;
	} *uap;

	uap = (struct a *)u.u_ap;
	open1(FWRITE|FCREAT|FTRUNC, uap->fmode);
}

/*
 * common code for open and creat.
 * Check permissions, allocate an open file structure,
 * and call the device open routine if any.
 *
 * Ohms 2/5/85 - modified for sys3/5 compatability
 */

extern int fd_undo;	/* see trap.c */

static
open1(mode, arg)
register mode;
{
	register struct inode *ip;
	register struct file *fp;
	int i;

	if ((mode&(FREAD|FWRITE)) == 0) {
		u.u_error = EINVAL;
		return;
	}
	if (mode&FCREAT) {
#ifdef	UCB_SYMLINKS
		ip = namei(uchar, CREATE, 1);
#else	UCB_SYMLINKS
		ip = namei(uchar, CREATE);
#endif	UCB_SYMLINKS
		if (ip == NULL) {
			if (u.u_error)
				return;
			ip = maknode(arg&07777&(~ISVTX));
			if (ip == NULL)
				return;
			mode &= ~FTRUNC;
		} else {
			if (mode&FEXCL) {
				u.u_error = EEXIST;
				iput(ip);
				return;
			}
			mode &= ~FCREAT;
		}
	} else {
#ifdef	UCB_SYMLINKS
		ip = namei(uchar, LOOKUP, 1);
#else	UCB_SYMLINKS
		ip = namei(uchar, LOOKUP);
#endif	UCB_SYMLINKS
		if (ip == NULL)
			return;
	}
	if (!(mode&FCREAT)) {
		if (mode&FREAD)
			access(ip, IREAD);
		if (mode&(FWRITE|FTRUNC)) {
			access(ip, IWRITE);
			if ((ip->i_mode&IFMT) == IFDIR)
				u.u_error = EISDIR;
		}
	}
	if (u.u_error)
		goto out;
	if (mode&FTRUNC)
		itrunc(ip);
	prele(ip);
	if ((fp = falloc()) == NULL)
		goto out;
	fp->f_flag = mode&FMASK;
	if(fd_undo == 0)
		fp->f_flag |= FPENDING;	/* file table slot alloc incomplete */
	fp->f_inode = ip;
	i = u.u_rval1;
	openi(ip, mode);
	fp->f_flag &= ~FPENDING;	/* allocation now complete */
	if(u.u_error == 0)
		return;
	u.u_ofile[i] = NULL;
	fp->f_count--;

out:
	iput(ip);
}

/*
 * close system call
 */
close()
{
	register struct file *fp;
	register struct a {
		int	fdes;
	} *uap;

	uap = (struct a *)u.u_ap;
	fp = getf(uap->fdes);
	if(fp == NULL)
		return;
	u.u_ofile[uap->fdes] = NULL;
#ifndef	UCB_NET
	closef(fp);
#else	UCB_NET
	closef(fp,0);
#endif	UCB_NET
}

/*
 * seek system call
 */
seek()
{
	register struct file *fp;
	register struct inode *ip;
	register struct a {
		int	fdes;
		off_t	off;
		int	sbase;
	} *uap;

	uap = (struct a *)u.u_ap;
	fp = getf(uap->fdes);
	if(fp == NULL)
		return;
#ifdef	UCB_NET
	if (fp->f_flag&FSOCKET) {
		u.u_error = ESPIPE;
		return;
	}
#endif	UCB_NET
	ip = fp->f_inode;
	if((ip->i_mode&IFMT) == IFIFO) {
		u.u_error = ESPIPE;
		return;
	}
	if(uap->sbase == 1)
		uap->off += fp->f_un.f_offset;
	else if(uap->sbase == 2)
		uap->off += fp->f_inode->i_size;
	else if(uap->sbase != 0) {
		u.u_error = EINVAL;
		psignal(u.u_procp, SIGSYS);
		return;
	}
	if(uap->off < 0) {
		u.u_error = EINVAL;
		uap->off = 0;
	}
	fp->f_un.f_offset = uap->off;
	u.u_r.r_off = uap->off;
}

/*
 * link system call
 */
link()
{
	register struct inode *ip, *xp;
	register struct a {
		char	*target;
		char	*linkname;
	} *uap;

	uap = (struct a *)u.u_ap;
#ifdef	UCB_SYMLINKS
	ip = namei(uchar, LOOKUP, 1);
#else	UCB_SYMLINKS
	ip = namei(uchar, LOOKUP);
#endif	UCB_SYMLINKS
	if(ip == NULL)
		return;
	if((ip->i_mode&IFMT)==IFDIR && !suser())
		goto out;
	ip->i_nlink++;
	ip->i_flag |= ICHG;
	iupdat(ip, &time, &time, 1);
	/*
	 * Unlock to avoid possibly hanging the namei.
	 * Sadly, this means races. (Suppose someone
	 * deletes the file in the meantime?)
	 * Nor can it be locked again later
	 * because then there will be deadly
	 * embraces.
	 */
	prele(ip);
	u.u_dirp = (caddr_t)uap->linkname;
#ifdef	UCB_SYMLINKS
	xp = namei(uchar, CREATE, 0);
#else	UCB_SYMLINKS
	xp = namei(uchar, CREATE);
#endif	UCB_SYMLINKS
	if(xp != NULL) {
		u.u_error = EEXIST;
		iput(xp);
	} else {
		if (u.u_error)
			goto err;
		if(u.u_pdir->i_dev != ip->i_dev) {
			iput(u.u_pdir);
			u.u_error = EXDEV;
		} else
			wdir(ip);
	}
	if (u.u_error) {
err:
		ip->i_nlink--;
		ip->i_flag |= ICHG;
	}

out:
	iput(ip);
}

/*
 * mknod system call
 */
mknod()
{
	register struct inode *ip;
	register struct a {
		char	*fname;
		int	fmode;
		dev_t	dev;
	} *uap;

	uap = (struct a *)u.u_ap;
	if(((uap->fmode&IFMT) != IFIFO) && !suser())
		return;
#ifdef	UCB_SYMLINKS
	ip = namei(uchar, CREATE, 0);
#else	UCB_SYMLINKS
	ip = namei(uchar, CREATE);
#endif	UCB_SYMLINKS
	if(ip != NULL) {
		u.u_error = EEXIST;
		goto out;
	}
	if(u.u_error)
		return;
	ip = maknode(uap->fmode);
	if (ip == NULL)
		return;
	switch(ip->i_mode&IFMT) {
	case IFCHR:
	case IFBLK:
		ip->i_rdev = (dev_t)uap->dev;
		ip->i_flag |= IACC|IUPD|ICHG;
	}
out:
	iput(ip);
}

/*
 * access system call
 */
saccess()
{
	register svuid, svgid;
	register struct inode *ip;
	register struct a {
		char	*fname;
		int	fmode;
	} *uap;

	uap = (struct a *)u.u_ap;
	svuid = u.u_uid;
	svgid = u.u_gid;
	u.u_uid = u.u_ruid;
	u.u_gid = u.u_rgid;
#ifdef	UCB_SYMLINKS
	ip = namei(uchar, LOOKUP, 1);
#else	UCB_SYMLINKS
	ip = namei(uchar, LOOKUP);
#endif	UCB_SYMLINKS
	if (ip != NULL) {
		if (uap->fmode&(IREAD>>6))
			access(ip, IREAD);
		if (uap->fmode&(IWRITE>>6))
			access(ip, IWRITE);
		if (uap->fmode&(IEXEC>>6))
			access(ip, IEXEC);
		iput(ip);
	}
	u.u_uid = svuid;
	u.u_gid = svgid;
}
