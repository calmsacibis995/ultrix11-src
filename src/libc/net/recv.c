
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)recv.c	3.0	4/22/86
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <errno.h>

recv(s, buf, len, flags)
int s;
char *buf;
int len, flags;
{
	return(recvit(s, buf, len, flags, 0));
}

recvfrom(s, buf, len, flags, from, fromlen)
int		s;
char		*buf;
int		len, flags;
struct sockaddr	*from;
int		*fromlen;
{
	struct msginfo		foo;
	register struct msginfo	*foop = &foo;
	int	error;

	foop->mi_to = from;
	foop->mi_tlen = *fromlen;
	foop->mi_rights = 0;
	foop->mi_rlen = 0;
	error = recvit(s, buf, len, flags, foop);
	*fromlen = foop->mi_tlen;
	return(error);
}

recvmsg(s, msg, flags)
int			s;
register struct msghdr	*msg;
int			flags;
{
	struct msginfo		foo;
	register struct msginfo	*foop = &foo;
	extern int		errno;
	int			error;

	if (msg->msg_iovlen != 1) {
		errno = EMSGSIZE;
		return(-1);
	}
	foop->mi_to = msg->msg_name;
	foop->mi_tlen = msg->msg_namelen;
	foop->mi_rights = msg->msg_accrights;
	foop->mi_rlen = msg->msg_accrightslen;
	error = recvit(s, msg->msg_iov->iov_base,
			 msg->msg_iov->iov_len, flags, foop);
	msg->msg_namelen = foop->mi_tlen;
	msg->msg_accrightslen = foop->mi_rlen;
	return(error);
}
