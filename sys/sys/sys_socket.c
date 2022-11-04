
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * Copyright (c) 1982 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * SCCSID: @(#)sys_socket.c	3.0	4/21/86
 *	Based on: @(#)sys_socket.c	6.5 (Berkeley) 6/8/85
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/file.h>
#include <sys/mbuf.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include <net/if.h>
#include <netinet/in_systm.h>
#include <net/route.h>

#ifdef	vax
int	soo_rw(), soo_ioctl(), soo_select(), soo_close();
#else	pdp11
int	soo_ioctl(), soo_select(), soo_close();
#endif

#ifdef	vax
struct	fileops socketops =
    { soo_rw, soo_ioctl, soo_select, soo_close };
#endif	vax

#ifdef	vax
soo_rw(fp, rw, uio)
	struct file *fp;
	enum uio_rw rw;
	struct uio *uio;
{
	int soreceive(), sosend();

	return (
	    (*(rw==UIO_READ?soreceive:sosend))
	      ((struct socket *)fp->f_data, 0, uio, 0, 0));
}
#endif	vax

soo_ioctl(fp, cmd, data)
	struct file *fp;
	int cmd;
	register caddr_t data;
{
#ifdef	vax
	register struct socket *so = (struct socket *)fp->f_data;
#endif
#ifdef	pdp11
	register struct socket *so = fp->f_socket;
	int	t;

	switch (cmd) {
	case FIONBIO:
	case FIOASYNC:
	case SIOCSPGRP:
		if (copyin(data, &t, sizeof(int)) == -1)
			return(EFAULT);
	}
#endif

	switch (cmd) {

	case FIONBIO:
#ifndef	pdp11
		if (*(int *)data)
#else	pdp11
		if (t)
#endif
			so->so_state |= SS_NBIO;
		else
			so->so_state &= ~SS_NBIO;
		return (0);

	case FIOASYNC:
#ifndef	pdp11
		if (*(int *)data)
#else	pdp11
		if (t)
#endif	pdp11
			so->so_state |= SS_ASYNC;
		else
			so->so_state &= ~SS_ASYNC;
		return (0);

	case FIONREAD:
#ifndef	pdp11
		*(int *)data = so->so_rcv.sb_cc;
		return (0);
#else	pdp11
		t = so->so_rcv.sb_cc;
		goto cpout;
#endif	pdp11

	case SIOCSPGRP:
#ifndef	pdp11
		so->so_pgrp = *(int *)data;
#else	pdp11
		so->so_pgrp = t;
#endif	pdp11
		return (0);

	case SIOCGPGRP:
#ifndef	pdp11
		*(int *)data = so->so_pgrp;
		return (0);
#else	pdp11
		t = so->so_pgrp;
		goto cpout;
#endif	pdp11

	case SIOCATMARK:
#ifndef	pdp11
		*(int *)data = (so->so_state&SS_RCVATMARK) != 0;
#else	pdp11
		t = (so->so_state&SS_RCVATMARK) != 0;
	cpout:
		if (copyout(&t, data, sizeof(int)) == -1)
			return(EFAULT);
#endif	pdp11
		return (0);
	}
	/*
	 * Interface/routing/protocol specific ioctls:
	 * interface and routing ioctls should have a
	 * different entry since a socket's unnecessary
	 */
#define	cmdbyte(x)	(((x) >> 8) & 0xff)
	if (cmdbyte(cmd) == 'i')
		return (ifioctl(so, cmd, data));
	if (cmdbyte(cmd) == 'r')
		return (rtioctl(cmd, data));
	return ((*so->so_proto->pr_usrreq)(so, PRU_CONTROL, 
	    (struct mbuf *)cmd, (struct mbuf *)data, (struct mbuf *)0));
}

#ifndef	pdp11
soo_select(fp, which)
	struct file *fp;
#else	pdp11
soo_select(so, which)
	register struct socket *so;
#endif	pdp11
	int which;
{
#ifndef	pdp11
	register struct socket *so = (struct socket *)fp->f_data;
#endif
	register int s = splnet();

	switch (which) {

	case FREAD:
		if (soreadable(so)) {
			splx(s);
			return (1);
		}
		sbselqueue(&so->so_rcv);
		break;

	case FWRITE:
		if (sowriteable(so)) {
			splx(s);
			return (1);
		}
		sbselqueue(&so->so_snd);
		break;
	}
	splx(s);
	return (0);
}

/*ARGSUSED*/
soo_stat(so, ub)
	register struct socket *so;
	register struct stat *ub;
{
#ifdef	pdp11
	struct stat sock;
	register t;
#endif	pdp11

#ifdef	pdp11
	bzero((caddr_t)&sock, sizeof (sock));
	t = (*so->so_proto->pr_usrreq)(so, PRU_SENSE,
	    (struct mbuf *)&sock, (struct mbuf *)0, 
	    (struct mbuf *)0);
	if (!t && (copyout((caddr_t)&sock, (caddr_t)ub, sizeof sock) < 0))
		return(EFAULT);
	return(t);
#else
	bzero((caddr_t)ub, sizeof (*ub));
	return ((*so->so_proto->pr_usrreq)(so, PRU_SENSE,
	    (struct mbuf *)ub, (struct mbuf *)0, 
	    (struct mbuf *)0));
#endif
}

soo_close(fp)
	struct file *fp;
{
	int error = 0;
	
#ifndef	pdp11
	if (fp->f_data)
		error = soclose((struct socket *)fp->f_data);
	fp->f_data = 0;
#else	pdp11
	if (fp->f_socket)
		error = soclose(fp->f_socket);
	fp->f_socket = 0;
#endif	pdp11
	return (error);
}
