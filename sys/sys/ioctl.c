
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)ioctl.c	3.1	7/21/87
 */

/*
 * Ioctl
 */
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/tty.h>
#include <sys/proc.h>
#include <sys/inode.h>
#include <sys/file.h>
#include <sys/reg.h>
#include <sys/conf.h>
#include <sys/mtio.h>


/*
 * stty/gtty writearound
 */
stty()
{
	u.u_arg[2] = u.u_arg[1];
	u.u_arg[1] = TIOCSETP;
	ioctl();
}

gtty()
{
	u.u_arg[2] = u.u_arg[1];
	u.u_arg[1] = TIOCGETP;
	ioctl();
}

/*
 * ioctl system call
 * Check legality, execute common code, and switch out to individual
 * device routine.
 */
ioctl()
{
	register struct file *fp;
	register struct inode *ip;
	register struct a {
		int	fdes;
		int	cmd;
		caddr_t	cmarg;
	} *uap;
	union b {
		struct mtop mtop;
		struct mtget mtget;
	} mtinfo;
	register dev_t dev;
	register fmt;
	caddr_t data;

	uap = (struct a *)u.u_ap;
	if ((fp = getf(uap->fdes)) == NULL)
		return;
	if ((fp->f_flag & (FREAD|FWRITE)) == 0) {
		u.u_error = EBADF;
		return;
	}
	if (uap->cmd==FIOCLEX) {
		u.u_pofile[uap->fdes] |= EXCLOSE;
		return;
	}
	if (uap->cmd==FIONCLEX) {
		u.u_pofile[uap->fdes] &= ~EXCLOSE;
		return;
	}
#ifdef	UCB_NET
	if (fp->f_flag & FSOCKET) {
		u.u_error = soo_ioctl(fp, uap->cmd, uap->cmarg);
		return;
	}
#endif	UCB_NET
	ip = fp->f_inode;
	fmt = ip->i_mode & IFMT;
	if (fmt != IFCHR) {
		if ((uap->cmd == FIONREAD) &&
		    (fmt == IFIFO || fmt == IFREG || fmt == IFDIR)) {
			off_t nread = ip->i_size - fp->f_un.f_offset;
			if (copyout((caddr_t)&nread, uap->cmarg, sizeof(off_t)))
				u.u_error = EFAULT;
		} else
#ifdef	SELECT
			if (uap->cmd == FIONBIO)
				return;
		else
#endif
			u.u_error = ENOTTY;
		return;
	}
	if (uap->cmd == MTIOCTOP) {
		if (copyin(uap->cmarg, (caddr_t)&mtinfo, sizeof(struct mtop))) {
			u.u_error = EFAULT;
			return;
		}
		data = (caddr_t)&mtinfo;
	} else if (uap->cmd == MTIOCGET) {
		bzero((caddr_t)&mtinfo, sizeof(struct mtget));
		data = (caddr_t)&mtinfo;
	} else
		data = uap->cmarg;
	dev = (dev_t)ip->i_rdev;
	u.u_rval1 = 0;
	if ((u.u_procp->p_flag&SNUSIG) && save(u.u_qsav)) {
		u.u_eosys = RESTARTSYS;
		return;
	}
	(*cdevsw[major(dev)].d_ioctl)(dev, uap->cmd, data, fp->f_flag);
	if (uap->cmd == MTIOCGET) {
		if (copyout((caddr_t)&mtinfo, uap->cmarg, sizeof(struct mtget)))
			u.u_error = EFAULT;
	}
}

/*
 * Do nothing specific version of line
 * discipline specific ioctl command.
 */
/*ARGSUSED*/
nullioctl(tp, cmd, addr, flag)
struct tty *tp;
caddr_t addr;
{
	return (cmd);
}
