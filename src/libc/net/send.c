
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)send.c	3.0	4/22/86
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <errno.h>

send(s, msg, len, flags)
int s;
char *msg;
int len, flags;
{
	return(sendit(s, msg, len, flags, 0));
}

sendto(s, msg, len, flags, to, tolen)
int		s;
char		*msg;
int		len, flags;
struct sockaddr	*to;
int		tolen;
{
	struct msginfo		foo;
	register struct msginfo	*foop = &foo;

	foop->mi_to = to;
	foop->mi_tlen = tolen;
	foop->mi_rights = 0;
	foop->mi_rlen = 0;
	return(sendit(s, msg, len, flags, foop));
}

sendmsg(s, msg, flags)
int			s;
register struct msghdr	*msg;
int			flags;
{
	struct msginfo		foo;
	register struct msginfo	*foop = &foo;
	extern			errno;

	if (msg->msg_iovlen != 1) {
		errno = EMSGSIZE;
		return(-1);
	}
	foop->mi_to = msg->msg_name;
	foop->mi_tlen = msg->msg_namelen;
	foop->mi_rights = msg->msg_accrights;
	foop->mi_rlen = msg->msg_accrightslen;
	return(sendit(s, msg->msg_iov->iov_base,
			 msg->msg_iov->iov_len, flags, foop));
}
