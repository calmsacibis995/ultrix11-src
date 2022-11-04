
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
 * SCCSID: @(#)uipc_syscall.c	3.0	4/21/86
 *	@(#)uipc_syscalls.c	6.9 (Berkeley) 6/8/85
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/file.h>
#include <sys/inode.h>
#include <sys/buf.h>
#include <sys/mbuf.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <netinet/in_systm.h>

/*
 * System call interface to the socket abstraction.
 */

struct	file *getsock();
#ifdef vax
extern	struct fileops socketops;
#endif vax

socket()
{
	register struct a {
		int	domain;
		int	type;
		int	protocol;
	} *uap = (struct a *)u.u_ap;
	struct socket *so;
	register struct file *fp;

	if ((fp = falloc()) == NULL)
		return;
#ifndef pdp11
	fp->f_flag = FREAD|FWRITE;
	fp->f_type = DTYPE_SOCKET;
	fp->f_ops = &socketops;
#else pdp11
	fp->f_flag = FREAD|FWRITE|FSOCKET;
#endif pdp11
	u.u_error = socreate(uap->domain, &so, uap->type, uap->protocol);
	if (u.u_error)
		goto bad;
#ifndef pdp11
	fp->f_data = (caddr_t)so;
#else pdp11
	fp->f_socket = so;
#endif pdp11
	return;
bad:
	u.u_ofile[u.u_r.r_val1] = 0;
	fp->f_count = 0;
}

bind()
{
	register struct a {
		int	s;
		caddr_t	name;
		int	namelen;
	} *uap = (struct a *)u.u_ap;
	register struct file *fp;
	struct mbuf *nam;

	fp = getsock(uap->s);
	if (fp == 0)
		return;
	u.u_error = sockargs(&nam, uap->name, uap->namelen, MT_SONAME);
	if (u.u_error)
		return;
#ifndef pdp11
	u.u_error = sobind((struct socket *)fp->f_data, nam);
#else pdp11
	u.u_error = sobind(fp->f_socket, nam);
#endif pdp11
	m_freem(nam);
}

listen()
{
	register struct a {
		int	s;
		int	backlog;
	} *uap = (struct a *)u.u_ap;
	register struct file *fp;

	fp = getsock(uap->s);
	if (fp == 0)
		return;
#ifndef pdp11
	u.u_error = solisten((struct socket *)fp->f_data, uap->backlog);
#else pdp11
	u.u_error = solisten(fp->f_socket, uap->backlog);
#endif pdp11
}

accept()
{
	register struct a {
		int	s;
		caddr_t	name;
		int	*anamelen;
	} *uap = (struct a *)u.u_ap;
	register struct file *fp;
	struct mbuf *nam;
	int namelen;
	int s;
	register struct socket *so;

	if (uap->name == 0)
		goto noname;
#ifndef pdp11
	u.u_error = copyin((caddr_t)uap->anamelen, (caddr_t)&namelen,
		sizeof (namelen));
	if (u.u_error)
		return;
#else pdp11
	if (copyin((caddr_t)uap->anamelen, (caddr_t)&namelen,
							sizeof (namelen))) {
		u.u_error = EFAULT;
		return;
	}
#endif pdp11
#ifndef pdp11
	if (useracc((caddr_t)uap->name, (u_int)namelen, B_WRITE) == 0) {
		u.u_error = EFAULT;
		return;
	}
#endif pdp11
noname:
	fp = getsock(uap->s);
	if (fp == 0)
		return;
	s = splnet();
#ifndef pdp11
	so = (struct socket *)fp->f_data;
#else pdp11
	so = fp->f_socket;
#endif pdp11
	if ((so->so_options & SO_ACCEPTCONN) == 0) {
		u.u_error = EINVAL;
		splx(s);
		return;
	}
	if ((so->so_state & SS_NBIO) && so->so_qlen == 0) {
		u.u_error = EWOULDBLOCK;
		splx(s);
		return;
	}
	while (so->so_qlen == 0 && so->so_error == 0) {
		if (so->so_state & SS_CANTRCVMORE) {
			so->so_error = ECONNABORTED;
			break;
		}
		sleep((caddr_t)&so->so_timeo, PZERO+1);
	}
	if (so->so_error) {
		u.u_error = so->so_error;
		splx(s);
		return;
	}
	if (ufalloc(0) < 0) {
		splx(s);
		return;
	}
	fp = falloc();
	if (fp == 0) {
		u.u_ofile[u.u_r.r_val1] = 0;
		splx(s);
		return;
	}
	{ struct socket *aso = so->so_q;
	  if (soqremque(aso, 1) == 0)
		panic("accept");
	  so = aso;
	}
#ifndef pdp11
	fp->f_type = DTYPE_SOCKET;
	fp->f_flag = FREAD|FWRITE;
	fp->f_ops = &socketops;
	fp->f_data = (caddr_t)so;
#else pdp11
	fp->f_flag = FREAD|FWRITE|FSOCKET;
	fp->f_socket = so;
#endif pdp11
	nam = m_get(M_WAIT, MT_SONAME);
	(void) soaccept(so, nam);
	if (uap->name) {
		if (namelen > nam->m_len)
			namelen = nam->m_len;
		/* SHOULD COPY OUT A CHAIN HERE */
		(void) copyout(mtod(nam, caddr_t), (caddr_t)uap->name,
		    (u_int)namelen);
#ifdef	pdp11
		normalseg5(); /* mtod() remapped us */
#endif	pdp11
		(void) copyout((caddr_t)&namelen, (caddr_t)uap->anamelen,
		    sizeof (*uap->anamelen));
	}
	m_freem(nam);
	splx(s);
}

connect()
{
	register struct a {
		int	s;
		caddr_t	name;
		int	namelen;
	} *uap = (struct a *)u.u_ap;
	register struct file *fp;
	register struct socket *so;
	struct mbuf *nam;
	int s;

	fp = getsock(uap->s);
	if (fp == 0)
		return;
#ifndef	pdp11
	so = (struct socket *)fp->f_data;
#else	pdp11
	so = fp->f_socket;
#endif	pdp11
	u.u_error = sockargs(&nam, uap->name, uap->namelen, MT_SONAME);
	if (u.u_error)
		return;
	u.u_error = soconnect(so, nam);
	if (u.u_error)
		goto bad;
	s = splnet();
	if ((so->so_state & SS_NBIO) &&
	    (so->so_state & SS_ISCONNECTING)) {
		u.u_error = EINPROGRESS;
		goto bad2;
	}
#ifndef pdp11
	if (setjmp(&u.u_qsave)) {
#else pdp11
	if (save(&u.u_qsav)) {
#endif pdp11
		if (u.u_error == 0)
			u.u_error = EINTR;
		goto bad2;
	}
	while ((so->so_state & SS_ISCONNECTING) && so->so_error == 0)
		sleep((caddr_t)&so->so_timeo, PZERO+1);
	u.u_error = so->so_error;
	so->so_error = 0;
bad2:
	splx(s);
bad:
	m_freem(nam);
}

socketpair()
{
	register struct a {
		int	domain;
		int	type;
		int	protocol;
		int	*rsv;
	} *uap = (struct a *)u.u_ap;
	register struct file *fp1, *fp2;
	struct socket *so1, *so2;
	int sv[2];

#ifdef	vax
	if (useracc((caddr_t)uap->rsv, 2 * sizeof (int), B_WRITE) == 0) {
		u.u_error = EFAULT;
		return;
	}
#endif	vax
	u.u_error = socreate(uap->domain, &so1, uap->type, uap->protocol);
	if (u.u_error)
		return;
	u.u_error = socreate(uap->domain, &so2, uap->type, uap->protocol);
	if (u.u_error)
		goto free;
	fp1 = falloc();
	if (fp1 == NULL)
		goto free2;
	sv[0] = u.u_r.r_val1;
#ifndef pdp11
	fp1->f_flag = FREAD|FWRITE;
	fp1->f_type = DTYPE_SOCKET;
	fp1->f_ops = &socketops;
	fp1->f_data = (caddr_t)so1;
#else pdp11
	fp1->f_flag = FREAD|FWRITE|FSOCKET;
	fp1->f_socket = so1;
#endif pdp11
	fp2 = falloc();
	if (fp2 == NULL)
		goto free3;
#ifndef	pdp11
	fp2->f_flag = FREAD|FWRITE;
	fp2->f_type = DTYPE_SOCKET;
	fp2->f_ops = &socketops;
	fp2->f_data = (caddr_t)so2;
#else	pdp11
	fp2->f_flag = FREAD|FWRITE|FSOCKET;
	fp2->f_socket = so2;
#endif	pdp11
	sv[1] = u.u_r.r_val1;
	u.u_error = soconnect2(so1, so2);
	if (u.u_error)
		goto free4;
	if (uap->type == SOCK_DGRAM) {
		/*
		 * Datagram socket connection is asymmetric.
		 */
		 u.u_error = soconnect2(so2, so1);
		 if (u.u_error)
			goto free4;
	}
	u.u_r.r_val1 = 0;
	(void) copyout((caddr_t)sv, (caddr_t)uap->rsv, 2 * sizeof (int));
	return;
free4:
	fp2->f_count = 0;
	u.u_ofile[sv[1]] = 0;
free3:
	fp1->f_count = 0;
	u.u_ofile[sv[0]] = 0;
free2:
	soclose(so2);
free:
	soclose(so1);
}

#ifdef pdp11
sendit()
{
	register struct file *fp;
	struct mbuf *to, *rights;
	struct b {
		caddr_t	to;
		int	tolen;
		caddr_t	rights;
		caddr_t rightslen;
	} minfo;
	register struct a {
		int	s;
		caddr_t	buf;
		int	len;
		int	flags;
		struct b *m;
	} *uap = (struct a *)u.u_ap;

	if (uap->m) {
		if (copyin(uap->m, (caddr_t)&minfo, sizeof (minfo))) {
			u.u_error = EFAULT;
			return;
		}
	} else
		minfo.to = minfo.rights = 0;

	fp = getsock(uap->s);
	if (fp == 0)
		return;
#ifdef	vax
	if (useracc(uap->buf, uap->len, B_READ) == 0) {
		u.u_error = EFAULT;
		return;
	}
#endif	vax
	if (minfo.to) {
		u.u_error = sockargs(&to, minfo.to, minfo.tolen, MT_SONAME);
		if (u.u_error)
			return;
	} else
		to = 0;
	if (minfo.rights) {
		u.u_error = sockargs(&rights, minfo.rights, minfo.rightslen, MT_RIGHTS);
		if (u.u_error)
			goto bad;
	} else
		rights = 0;
	u.u_base = uap->buf;
	u.u_count = uap->len;
#ifdef	pdp11
	u.u_segflg = 0;
#endif	pdp11
	u.u_error = sosend(fp->f_socket, to, uap->flags, rights);
	u.u_r.r_val1 = uap->len - u.u_count;
	if (rights)
		m_freem(rights);
bad:
	if (to)
		m_freem(to);
}

#else pdp11
sendto()
{
	register struct a {
		int	s;
		caddr_t	buf;
		int	len;
		int	flags;
		caddr_t	to;
		int	tolen;
	} *uap = (struct a *)u.u_ap;
	struct msghdr msg;
	struct iovec aiov;

	msg.msg_name = uap->to;
	msg.msg_namelen = uap->tolen;
	msg.msg_iov = &aiov;
	msg.msg_iovlen = 1;
	aiov.iov_base = uap->buf;
	aiov.iov_len = uap->len;
	msg.msg_accrights = 0;
	msg.msg_accrightslen = 0;
	sendit(uap->s, &msg, uap->flags);
}

send()
{
	register struct a {
		int	s;
		caddr_t	buf;
		int	len;
		int	flags;
	} *uap = (struct a *)u.u_ap;
	struct msghdr msg;
	struct iovec aiov;

	msg.msg_name = 0;
	msg.msg_namelen = 0;
	msg.msg_iov = &aiov;
	msg.msg_iovlen = 1;
	aiov.iov_base = uap->buf;
	aiov.iov_len = uap->len;
	msg.msg_accrights = 0;
	msg.msg_accrightslen = 0;
	sendit(uap->s, &msg, uap->flags);
}

sendmsg()
{
	register struct a {
		int	s;
		caddr_t	msg;
		int	flags;
	} *uap = (struct a *)u.u_ap;
	struct msghdr msg;
	struct iovec aiov[MSG_MAXIOVLEN];

	u.u_error = copyin(uap->msg, (caddr_t)&msg, sizeof (msg));
	if (u.u_error)
		return;
	if ((u_int)msg.msg_iovlen >= sizeof (aiov) / sizeof (aiov[0])) {
		u.u_error = EMSGSIZE;
		return;
	}
	u.u_error =
	    copyin((caddr_t)msg.msg_iov, (caddr_t)aiov,
		(unsigned)(msg.msg_iovlen * sizeof (aiov[0])));
	if (u.u_error)
		return;
	msg.msg_iov = aiov;
#ifdef notdef
printf("sendmsg name %x namelen %d iov %x iovlen %d accrights %x &len %d\n",
msg.msg_name, msg.msg_namelen, msg.msg_iov, msg.msg_iovlen,
msg.msg_accrights, msg.msg_accrightslen);
#endif
	sendit(uap->s, &msg, uap->flags);
}

sendit(s, mp, flags)
	int s;
	register struct msghdr *mp;
	int flags;
{
	register struct file *fp;
	struct uio auio;
	register struct iovec *iov;
	register int i;
	struct mbuf *to, *rights;
	int len;
	
	fp = getsock(s);
	if (fp == 0)
		return;
	auio.uio_iov = mp->msg_iov;
	auio.uio_iovcnt = mp->msg_iovlen;
	auio.uio_segflg = UIO_USERSPACE;
	auio.uio_offset = 0;			/* XXX */
	auio.uio_resid = 0;
	iov = mp->msg_iov;
	for (i = 0; i < mp->msg_iovlen; i++) {
		if (iov->iov_len < 0) {
			u.u_error = EINVAL;
			return;
		}
		if (iov->iov_len == 0)
			continue;
		if (useracc(iov->iov_base, (u_int)iov->iov_len, B_READ) == 0) {
			u.u_error = EFAULT;
			return;
		}
		auio.uio_resid += iov->iov_len;
		iov++;
	}
	if (mp->msg_name) {
		u.u_error =
		    sockargs(&to, mp->msg_name, mp->msg_namelen, MT_SONAME);
		if (u.u_error)
			return;
	} else
		to = 0;
	if (mp->msg_accrights) {
		u.u_error =
		    sockargs(&rights, mp->msg_accrights, mp->msg_accrightslen,
		    MT_RIGHTS);
		if (u.u_error)
			goto bad;
	} else
		rights = 0;
	len = auio.uio_resid;
	u.u_error =
	    sosend((struct socket *)fp->f_data, to, &auio, flags, rights);
	u.u_r.r_val1 = len - auio.uio_resid;
	if (rights)
		m_freem(rights);
bad:
	if (to)
		m_freem(to);
}

#endif pdp11
#ifdef pdp11
recvit()
{
	register struct file *fp;
	struct mbuf *from, *rights;
	int len;
	struct b {
		caddr_t	name;
		int	namelen;
		caddr_t	rights;
		caddr_t rightslen;
	} minfo;
	register struct a {
		int	s;
		caddr_t	buf;
		int	len;
		int	flags;
		struct b *m;
	} *uap = (struct a *)u.u_ap;

	fp = getsock(uap->s);
	if (fp == 0)
		return;

	if (uap->m) {
		if (copyin(uap->m, &minfo, sizeof(minfo))) {
			u.u_error = EFAULT;
			return;
		}
	} else {
		minfo.name = 0;
		minfo.rights = 0;
	}

#ifdef	vax
	if (minfo.rights)
		if (useracc((caddr_t)minfo.rights,
		    (unsigned)minfo.rightslen, B_WRITE) == 0) {
			u.u_error = EFAULT;
			return;
		}

	if (useracc(uap->buf, uap->len, B_WRITE) == 0) {
		u.u_error = EFAULT;
		return;
	}
#endif	vax
	u.u_base = uap->buf;
	u.u_count = uap->len;
#ifdef	pdp11
	u.u_segflg = 0;
#endif	pdp11

	u.u_error = soreceive(fp->f_socket, &from, uap->flags, &rights);
	u.u_r.r_val1 = uap->len - u.u_count;
	if (minfo.name) {
		len = minfo.namelen;
		if (len <= 0 || from == 0)
			len = 0;
		else {
			if (len > from->m_len)
				len = from->m_len;
			(void) copyout((caddr_t)mtod(from, caddr_t),
				(caddr_t)minfo.name, (unsigned)len);
#ifdef	pdp11
			normalseg5(); /* mtod() remapped us */
#endif	pdp11
		}
		(void)copyout((caddr_t)&len, &uap->m->namelen, sizeof (int));
	}
	if (minfo.rights) {
		len = minfo.rightslen;
		if (len <= 0 || rights == 0)
			len = 0;
		else {
			if (len > rights->m_len)
				len = rights->m_len;
			(void)copyout((caddr_t)mtod(rights, caddr_t),
			    (caddr_t)minfo.rights, (unsigned)len);
#ifdef	pdp11
			normalseg5(); /* mtod() remapped us */
#endif	pdp11
		}
		(void)copyout((caddr_t)&len, &uap->m->rightslen, sizeof (int));
	}
	if (rights)
		m_freem(rights);
	if (from)
		m_freem(from);
}
#else pdp11
recvfrom()
{
	register struct a {
		int	s;
		caddr_t	buf;
		int	len;
		int	flags;
		caddr_t	from;
		int	*fromlenaddr;
	} *uap = (struct a *)u.u_ap;
	struct msghdr msg;
	struct iovec aiov;
	int len;

	u.u_error = copyin((caddr_t)uap->fromlenaddr, (caddr_t)&len,
	   sizeof (len));
	if (u.u_error)
		return;
	msg.msg_name = uap->from;
	msg.msg_namelen = len;
	msg.msg_iov = &aiov;
	msg.msg_iovlen = 1;
	aiov.iov_base = uap->buf;
	aiov.iov_len = uap->len;
	msg.msg_accrights = 0;
	msg.msg_accrightslen = 0;
	recvit(uap->s, &msg, uap->flags, (caddr_t)uap->fromlenaddr, (caddr_t)0);
}

recv()
{
	register struct a {
		int	s;
		caddr_t	buf;
		int	len;
		int	flags;
	} *uap = (struct a *)u.u_ap;
	struct msghdr msg;
	struct iovec aiov;

	msg.msg_name = 0;
	msg.msg_namelen = 0;
	msg.msg_iov = &aiov;
	msg.msg_iovlen = 1;
	aiov.iov_base = uap->buf;
	aiov.iov_len = uap->len;
	msg.msg_accrights = 0;
	msg.msg_accrightslen = 0;
	recvit(uap->s, &msg, uap->flags, (caddr_t)0, (caddr_t)0);
}

recvmsg()
{
	register struct a {
		int	s;
		struct	msghdr *msg;
		int	flags;
	} *uap = (struct a *)u.u_ap;
	struct msghdr msg;
	struct iovec aiov[MSG_MAXIOVLEN];

	u.u_error = copyin((caddr_t)uap->msg, (caddr_t)&msg, sizeof (msg));
	if (u.u_error)
		return;
	if ((u_int)msg.msg_iovlen >= sizeof (aiov) / sizeof (aiov[0])) {
		u.u_error = EMSGSIZE;
		return;
	}
	u.u_error =
	    copyin((caddr_t)msg.msg_iov, (caddr_t)aiov,
		(unsigned)(msg.msg_iovlen * sizeof (aiov[0])));
	if (u.u_error)
		return;
	msg.msg_iov = aiov;
	if (msg.msg_accrights)
		if (useracc((caddr_t)msg.msg_accrights,
		    (unsigned)msg.msg_accrightslen, B_WRITE) == 0) {
			u.u_error = EFAULT;
			return;
		}
	recvit(uap->s, &msg, uap->flags,
	    (caddr_t)&uap->msg->msg_namelen,
	    (caddr_t)&uap->msg->msg_accrightslen);
}

recvit(s, mp, flags, namelenp, rightslenp)
	int s;
	register struct msghdr *mp;
	int flags;
	caddr_t namelenp, rightslenp;
{
	register struct file *fp;
	struct uio auio;
	register struct iovec *iov;
	register int i;
	struct mbuf *from, *rights;
	int len;
	
	fp = getsock(s);
	if (fp == 0)
		return;
	auio.uio_iov = mp->msg_iov;
	auio.uio_iovcnt = mp->msg_iovlen;
	auio.uio_segflg = UIO_USERSPACE;
	auio.uio_offset = 0;			/* XXX */
	auio.uio_resid = 0;
	iov = mp->msg_iov;
	for (i = 0; i < mp->msg_iovlen; i++) {
		if (iov->iov_len < 0) {
			u.u_error = EINVAL;
			return;
		}
		if (iov->iov_len == 0)
			continue;
		if (useracc(iov->iov_base, (u_int)iov->iov_len, B_WRITE) == 0) {
			u.u_error = EFAULT;
			return;
		}
		auio.uio_resid += iov->iov_len;
		iov++;
	}
	len = auio.uio_resid;
	u.u_error =
	    soreceive((struct socket *)fp->f_data, &from, &auio,
		flags, &rights);
	u.u_r.r_val1 = len - auio.uio_resid;
	if (mp->msg_name) {
		len = mp->msg_namelen;
		if (len <= 0 || from == 0)
			len = 0;
		else {
			if (len > from->m_len)
				len = from->m_len;
			(void) copyout((caddr_t)mtod(from, caddr_t),
			    (caddr_t)mp->msg_name, (unsigned)len);
		}
		(void) copyout((caddr_t)&len, namelenp, sizeof (int));
	}
	if (mp->msg_accrights) {
		len = mp->msg_accrightslen;
		if (len <= 0 || rights == 0)
			len = 0;
		else {
			if (len > rights->m_len)
				len = rights->m_len;
			(void) copyout((caddr_t)mtod(rights, caddr_t),
			    (caddr_t)mp->msg_accrights, (unsigned)len);
		}
		(void) copyout((caddr_t)&len, rightslenp, sizeof (int));
	}
	if (rights)
		m_freem(rights);
	if (from)
		m_freem(from);
}
#endif pdp11

shutdown()
{
	struct a {
		int	s;
		int	how;
	} *uap = (struct a *)u.u_ap;
	struct file *fp;

	fp = getsock(uap->s);
	if (fp == 0)
		return;
#ifndef pdp11
	u.u_error = soshutdown((struct socket *)fp->f_data, uap->how);
#else pdp11
	u.u_error = soshutdown(fp->f_socket, uap->how);
#endif pdp11
}

setsockopt()
{
	struct a {
		int	s;
		int	level;
		int	name;
		caddr_t	val;
		int	valsize;
	} *uap = (struct a *)u.u_ap;
	struct file *fp;
	struct mbuf *m = NULL;

	fp = getsock(uap->s);
	if (fp == 0)
		return;
	if (uap->valsize > MLEN) {
		u.u_error = EINVAL;
		return;
	}
	if (uap->val) {
#ifndef	pdp11
		m = m_get(M_WAIT, MT_SOOPTS);
#else	pdp11
		m = m_get(M_DONTWAIT, MT_SOOPTS);
#endif	pdp11
		if (m == NULL) {
			u.u_error = ENOBUFS;
			return;
		}
#ifndef	pdp11
		u.u_error =
		    copyin(uap->val, mtod(m, caddr_t), (u_int)uap->valsize);
#else	pdp11
		if (copyin(uap->val, mtod(m, caddr_t), (u_int)uap->valsize))
			u.u_error = EFAULT;
		else
			u.u_error = 0;
		normalseg5(); /* mtod() remapped us */
#endif	pdp11
		if (u.u_error) {
			(void) m_free(m);
			return;
		}
		m->m_len = uap->valsize;
	}
	u.u_error =
#ifndef pdp11
	    sosetopt((struct socket *)fp->f_data, uap->level, uap->name, m);
#else pdp11
	    sosetopt(fp->f_socket, uap->level, uap->name, m);
#endif pdp11
}

getsockopt()
{
	struct a {
		int	s;
		int	level;
		int	name;
		caddr_t	val;
		int	*avalsize;
	} *uap = (struct a *)u.u_ap;
	struct file *fp;
	struct mbuf *m = NULL;
	int valsize;

	fp = getsock(uap->s);
	if (fp == 0)
		return;
	if (uap->val) {
#ifndef pdp11
		u.u_error = copyin((caddr_t)uap->avalsize, (caddr_t)&valsize,
			sizeof (valsize));
		if (u.u_error)
			return;
#else pdp11
		if (copyin((caddr_t)uap->avalsize, (caddr_t)&valsize,
							   sizeof (valsize))) {
			u.u_error = EFAULT;
			return;
		}
#endif pdp11
	} else
		valsize = 0;
	u.u_error =
#ifndef pdp11
	    sogetopt((struct socket *)fp->f_data, uap->level, uap->name, &m);
#else pdp11
	    sogetopt(fp->f_socket, uap->level, uap->name, &m);
#endif pdp11
	if (u.u_error)
		goto bad;
	if (uap->val && valsize && m != NULL) {
		if (valsize > m->m_len)
			valsize = m->m_len;
#ifndef	pdp11
		u.u_error = copyout(mtod(m, caddr_t), uap->val, (u_int)valsize);
#else pdp11
		if (copyout(mtod(m, caddr_t), uap->val, (u_int)valsize) < 0)
			u.u_error = EFAULT;
		normalseg5(); /* mtod() remapped us */
#endif pdp11
		if (u.u_error)
			goto bad;
#ifndef	pdp11
		u.u_error = copyout((caddr_t)&valsize, (caddr_t)uap->avalsize,
		    sizeof (valsize));
#else pdp11
		if (copyout((caddr_t)&valsize, (caddr_t)uap->avalsize,
		    sizeof (valsize)) < 0)
			u.u_error = EFAULT;
#endif pdp11
	}
bad:
	if (m != NULL)
		(void) m_free(m);
}

#ifdef	vax
pipe()
{
	register struct file *rf, *wf;
	struct socket *rso, *wso;
	int r;

	u.u_error = socreate(AF_UNIX, &rso, SOCK_STREAM, 0);
	if (u.u_error)
		return;
	u.u_error = socreate(AF_UNIX, &wso, SOCK_STREAM, 0);
	if (u.u_error)
		goto free;
	rf = falloc();
	if (rf == NULL)
		goto free2;
	r = u.u_r.r_val1;
#ifndef pdp11
	rf->f_flag = FREAD;
	rf->f_type = DTYPE_SOCKET;
	rf->f_ops = &socketops;
	rf->f_data = (caddr_t)rso;
#else pdp11
	rf->f_flag = FREAD|FSOCKET;
	rf->f_socket = rso;
#endif pdp11
	wf = falloc();
	if (wf == NULL)
		goto free3;
#ifdef	pdp11
	wf->f_flag = FWRITE;
	wf->f_type = DTYPE_SOCKET;
	wf->f_ops = &socketops;
	wf->f_data = (caddr_t)wso;
#else	pdp11
	wf->f_flag = FWRITE|FSOCKET;
	wf->f_socket = wso;
#endif	pdp11
	u.u_r.r_val2 = u.u_r.r_val1;
	u.u_r.r_val1 = r;
	if (u.u_error = unp_connect2(wso, (struct mbuf *)0, rso))
		goto free4;
	wso->so_state |= SS_CANTRCVMORE;
	rso->so_state |= SS_CANTSENDMORE;
	return;
free4:
	wf->f_count = 0;
	u.u_ofile[u.u_r.r_val2] = 0;
free3:
	rf->f_count = 0;
	u.u_ofile[r] = 0;
free2:
	wso->so_state |= SS_NOFDREF;
	sofree(wso);
free:
	rso->so_state |= SS_NOFDREF;
	sofree(rso);
}
#endif

/*
 * Get socket name.
 */
getsockname()
{
	register struct a {
		int	fdes;
		caddr_t	asa;
		int	*alen;
	} *uap = (struct a *)u.u_ap;
	register struct file *fp;
	register struct socket *so;
	struct mbuf *m;
	int len;

	fp = getsock(uap->fdes);
	if (fp == 0)
		return;
#ifndef pdp11
	u.u_error = copyin((caddr_t)uap->alen, (caddr_t)&len, sizeof (len));
	if (u.u_error)
		return;
	so = (struct socket *)fp->f_data;
	m = m_getclr(M_WAIT, MT_SONAME);
#else pdp11
	if (copyin((caddr_t)uap->alen, (caddr_t)&len, sizeof (len))) {
		u.u_error = EFAULT;
		return;
	}
	so = fp->f_socket;
	m = m_getclr(M_DONTWAIT, MT_SONAME);
#endif pdp11
	if (m == NULL) {
		u.u_error = ENOBUFS;
		return;
	}
	u.u_error = (*so->so_proto->pr_usrreq)(so, PRU_SOCKADDR, 0, m, 0);
	if (u.u_error)
		goto bad;
	if (len > m->m_len)
		len = m->m_len;
#ifndef	pdp11
	u.u_error = copyout(mtod(m, caddr_t), (caddr_t)uap->asa, (u_int)len);
#else pdp11
	if (copyout(mtod(m, caddr_t), (caddr_t)uap->asa, (u_int)len) < 0)
		u.u_error = EFAULT;
	normalseg5(); /* mtod() remapped us */
#endif	pdp11
	if (u.u_error)
		goto bad;
#ifndef	pdp11
	u.u_error = copyout((caddr_t)&len, (caddr_t)uap->alen, sizeof (len));
#else pdp11
	if (copyout((caddr_t)&len, (caddr_t)uap->alen, sizeof (len)) < 0)
		u.u_error = EFAULT;
#endif pdp11
bad:
	m_freem(m);
}

/*
 * Get name of peer for connected socket.
 */
getpeername()
{
	register struct a {
		int	fdes;
		caddr_t	asa;
		int	*alen;
	} *uap = (struct a *)u.u_ap;
	register struct file *fp;
	register struct socket *so;
	struct mbuf *m;
	int len;

	fp = getsock(uap->fdes);
	if (fp == 0)
		return;
#ifndef pdp11
	so = (struct socket *)fp->f_data;
#else pdp11
	so = fp->f_socket;
#endif pdp11
	if ((so->so_state & SS_ISCONNECTED) == 0) {
		u.u_error = ENOTCONN;
		return;
	}
#ifndef	pdp11
	m = m_getclr(M_WAIT, MT_SONAME);
#else	pdp11
	m = m_getclr(M_DONTWAIT, MT_SONAME);
#endif	pdp11
	if (m == NULL) {
		u.u_error = ENOBUFS;
		return;
	}
#ifndef pdp11
	u.u_error = copyin((caddr_t)uap->alen, (caddr_t)&len, sizeof (len));
	if (u.u_error)
		return;
#else pdp11
	if (copyin((caddr_t)uap->alen, (caddr_t)&len, sizeof (len))) {
		u.u_error = EFAULT;
		return;
	}
#endif pdp11
	u.u_error = (*so->so_proto->pr_usrreq)(so, PRU_PEERADDR, 0, m, 0);
	if (u.u_error)
		goto bad;
	if (len > m->m_len)
		len = m->m_len;
#ifndef	pdp11
	u.u_error = copyout(mtod(m, caddr_t), (caddr_t)uap->asa, (u_int)len);
#else pdp11
	if (copyout(mtod(m, caddr_t), (caddr_t)uap->asa, (u_int)len) < 0)
		u.u_error = EFAULT;
	normalseg5(); /* mtod() remapped us */
#endif pdp11
	if (u.u_error)
		goto bad;
#ifndef pdp11
	u.u_error = copyout((caddr_t)&len, (caddr_t)uap->alen, sizeof (len));
#else pdp11
	if (copyout((caddr_t)&len, (caddr_t)uap->alen, sizeof (len)) < 0)
		u.u_error = EFAULT;
#endif pdp11
bad:
	m_freem(m);
}

static
sockargs(aname, name, namelen, type)
	struct mbuf **aname;
	caddr_t name;
	int namelen, type;
{
	register struct mbuf *m;
	int error;

	if (namelen > MLEN)
		return (EINVAL);
#ifndef	pdp11
	m = m_get(M_WAIT, type);
#else	pdp11
	m = m_get(M_DONTWAIT, type);
#endif	pdp11
	if (m == NULL)
		return (ENOBUFS);
	m->m_len = namelen;
#ifndef	pdp11
	error = copyin(name, mtod(m, caddr_t), (u_int)namelen);
#else pdp11
	if (copyin(name, mtod(m, caddr_t), (u_int)namelen))
		error = EFAULT;
	else
		error = 0;
	normalseg5();	/* mtod() remapped us */
#endif	pdp11
	if (error)
		(void) m_free(m);
	else
		*aname = m;
	return (error);
}

static struct file *
getsock(fdes)
	int fdes;
{
	register struct file *fp;

	fp = getf(fdes);
	if (fp == NULL)
		return (0);
#ifndef pdp11
	if (fp->f_type != DTYPE_SOCKET) {
#else pdp11
	if ((fp->f_flag & FSOCKET) == 0) {
#endif pdp11
		u.u_error = ENOTSOCK;
		return (0);
	}
	return (fp);
}
