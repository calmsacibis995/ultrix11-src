
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#ifndef lint
/*
 * Based on "@(#)tftpd.c	1.3	(ULTRIX)	4/11/85";
 */
static char sccsid[] = "@(#)tftpd.c	3.1	(ULTRIX-11)	4/22/87";
#endif

/*-----------------------------------------------------------------------
 *	Modification History
 *
 *	4/5/85 -- jrs
 *		Revise to allow inetd to perform front end functions,
 *		following the Berkeley model.
 *
 *	Based on 4.2BSD labeled:
 *		tftpd.c	4.11	83/07/02
 *
 *-----------------------------------------------------------------------
 */

/*
 * Trivial file transfer protocol server.
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#ifndef	pdp11
#include <sys/wait.h>
#else	pdp11
#include <wait.h>
#endif	pdp11

#include <sys/stat.h>

#include <netinet/in.h>

#include <arpa/tftp.h>

#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <netdb.h>
#include <setjmp.h>

#define	TIMEOUT		5

extern	int errno;
struct	sockaddr_in sin = { AF_INET };
int	f;
int	rexmtval = TIMEOUT;
int	maxtimeout = 5*TIMEOUT;
char	buf[BUFSIZ*2];
int	reapchild();

main(argc, argv)
	char *argv[];
{
	struct sockaddr_in from;
	int fromlen;
	register struct tftphdr *tp;
	register int n;

	f = 0;
	fromlen = sizeof (from);

	/* the alarm is in case inetd came around too quick and fired
	   us off to handle a request now picked up by a previous
	   incarnation */

	alarm(maxtimeout);
	n = recvfrom(f, buf, sizeof (buf), 0, (caddr_t)&from, &fromlen);
	alarm(0);
	if (n <= 0) {
		if (n < 0) {
			perror("tftpd: recvfrom");
		}
	} else {
		tp = (struct tftphdr *)buf;
		tp->th_opcode = ntohs(tp->th_opcode);
		if (tp->th_opcode == RRQ || tp->th_opcode == WRQ) {
			tftp(&from, fromlen, tp, n);
		}
	}
}

int	validate_access();
int	sendfile(), recvfile(), nasendfile(), narecvfile();

struct formats {
	char	*f_mode;
	int	(*f_validate)();
	int	(*f_send)();
	int	(*f_recv)();
} formats[] = {
	{ "image",	validate_access,	sendfile,	recvfile },
	{ "netascii",	validate_access,	nasendfile,	narecvfile },
	{ "octet",	validate_access,	sendfile,	recvfile },
#ifdef notdef
	{ "mail",	validate_user,		sendmail,	recvmail },
#endif
	{ 0 }
};

int	fd;			/* file being transferred */

/*
 * Handle initial connection protocol.
 */
tftp(client, clientsize, tp, size)
	struct sockaddr_in *client;
	int clientsize;
	struct tftphdr *tp;
	int size;
{
	register char *cp;
	int first = 1, ecode;
	register struct formats *pf;
	char *filename, *mode;

	(void) close(f);
	f = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (f < 0) {
		perror("tftpd: socket");
		exit(1);
	}
	if (bind(f, &sin, sizeof(sin)) < 0) {
		perror("tftpd: bind");
		exit(1);
	}
	if (connect(f, client, clientsize) < 0) {
		perror("tftpd: connect");
		exit(1);
	}
	filename = cp = tp->th_stuff;
again:
	while (cp < buf + size) {
		if (*cp == '\0')
			break;
		cp++;
	}
	if (*cp != '\0') {
		nak(EBADOP);
		exit(1);
	}
	if (first) {
		mode = ++cp;
		first = 0;
		goto again;
	}
	for (cp = mode; *cp; cp++)
		if (isupper(*cp))
			*cp = tolower(*cp);
	for (pf = formats; pf->f_mode; pf++)
		if (strcmp(pf->f_mode, mode) == 0)
			break;
	if (pf->f_mode == 0) {
		nak(EBADOP);
		exit(1);
	}
	ecode = (*pf->f_validate)(filename, client, tp->th_opcode);
	if (ecode) {
		nak(ecode);
		exit(1);
	}
	if (tp->th_opcode == WRQ)
		(*pf->f_recv)(pf);
	else
		(*pf->f_send)(pf);
	exit(0);
}

/*
 * Validate file access.  Since we
 * have no uid or gid, for now require
 * file to exist and be publicly
 * readable/writable.
 * Note also, full path name must be
 * given as we have no login directory.
 */
validate_access(file, client, mode)
	char *file;
	struct sockaddr_in *client;
	int mode;
{
	struct stat stbuf;
	char *hold;
	char *ptr;
	char c;

	if (*file != '/')
		return (EACCESS);
	if (mode == RRQ) {
		if (stat(file, &stbuf) < 0)
			return (errno == ENOENT ? ENOTFOUND : EACCESS);
		if ((stbuf.st_mode&(S_IREAD >> 6)) == 0)
			return (EACCESS);
	} else {
		if (access(file, 0) == 0)
			return (EEXISTS);
		if (errno != ENOENT)
			return (EACCESS);
		hold = file;
		for (ptr = file; *ptr; ptr++) {
			if (*ptr == '/')
				hold = ptr;
		}
		hold++;
		c = *hold;
		*hold = '\0';
		if (stat(file, &stbuf) < 0)
			return (EACCESS);
		if ((stbuf.st_mode&(S_IWRITE >> 6)) == 0)
			return (EACCESS);
		*hold = c;
	}
	fd = open(file, mode == RRQ ? O_RDONLY : O_WRONLY|O_CREAT|O_TRUNC, 0666);
	if (fd < 0)
		return (errno + 100);
	return (0);
}

int	timeout;

timer()
{

	timeout += rexmtval;
	if (timeout >= maxtimeout)
		exit(1);
	alarm(rexmtval);
}

/*
 * Send the requested file.
 */
sendfile(pf)
	struct format *pf;
{
	register struct tftphdr *tp;
	register int block = 1, size, n;

	signal(SIGALRM, timer);
	tp = (struct tftphdr *)buf;
	do {
		size = read(fd, tp->th_data, SEGSIZE);
		if (size < 0) {
			nak(errno + 100);
			break;
		}
		tp->th_opcode = htons((u_short)DATA);
		tp->th_block = htons((u_short)block);
		timeout = 0;
		alarm(rexmtval);
rexmt:
		if (write(f, buf, size + 4) != size + 4) {
			perror("tftpd: write");
			break;
		}
again:
		n = read(f, buf, sizeof (buf));
		if (n <= 0) {
			if (n == 0)
				goto again;
			if (errno == EINTR)
				goto rexmt;
			alarm(0);
			perror("tftpd: read");
			break;
		}
		alarm(0);
		tp->th_opcode = ntohs((u_short)tp->th_opcode);
		tp->th_block = ntohs((u_short)tp->th_block);
		if (tp->th_opcode == ERROR)
			break;
		if (tp->th_opcode != ACK || tp->th_block != block)
			goto again;
		block++;
	} while (size == SEGSIZE);
	(void) close(fd);
}

/*
 * Receive a file.
 */
recvfile(pf)
	struct format *pf;
{
	register struct tftphdr *tp;
	register int block = 0, n, size;

	signal(SIGALRM, timer);
	tp = (struct tftphdr *)buf;
	do {
		timeout = 0;
		alarm(rexmtval);
		tp->th_opcode = htons((u_short)ACK);
		tp->th_block = htons((u_short)block);
		block++;
rexmt:
		if (write(f, buf, 4) != 4) {
			perror("tftpd: write");
			break;
		}
again:
		n = read(f, buf, sizeof (buf));
		if (n <= 0) {
			if (n == 0)
				goto again;
			if (errno == EINTR)
				goto rexmt;
			alarm(0);
			perror("tftpd: read");
			break;
		}
		alarm(0);
		tp->th_opcode = ntohs((u_short)tp->th_opcode);
		tp->th_block = ntohs((u_short)tp->th_block);
		if (tp->th_opcode == ERROR)
			break;
		if (tp->th_opcode != DATA || block != tp->th_block)
			goto again;
		size = write(fd, tp->th_data, n - 4);
		if (size < 0) {
			nak(errno + 100);
			break;
		}
	} while (size == SEGSIZE);
	tp->th_opcode = htons((u_short)ACK);
	tp->th_block = htons((u_short)(block));
	(void) write(f, buf, 4);
	(void) close(fd);
}

struct errmsg {
	int	e_code;
	char	*e_msg;
} errmsgs[] = {
	{ EUNDEF,	"Undefined error code" },
	{ ENOTFOUND,	"File not found" },
	{ EACCESS,	"Access violation" },
	{ ENOSPACE,	"Disk full or allocation exceeded" },
	{ EBADOP,	"Illegal TFTP operation" },
	{ EBADID,	"Unknown transfer ID" },
	{ EEXISTS,	"File already exists" },
	{ ENOUSER,	"No such user" },
	{ -1,		0 }
};

/*
 * Send a nak packet (error message).
 * Error code passed in is one of the
 * standard TFTP codes, or a UNIX errno
 * offset by 100.
 */
nak(error)
	int error;
{
	register struct tftphdr *tp;
	int length;
	register struct errmsg *pe;
	extern char *sys_errlist[];

	tp = (struct tftphdr *)buf;
	tp->th_opcode = htons((u_short)ERROR);
	tp->th_code = htons((u_short)error);
	for (pe = errmsgs; pe->e_code >= 0; pe++)
		if (pe->e_code == error)
			break;
	if (pe->e_code < 0)
		pe->e_msg = sys_errlist[error - 100];
	strcpy(tp->th_msg, pe->e_msg);
	length = strlen(pe->e_msg);
	tp->th_msg[length] = '\0';
	length += 5;
	if (write(f, buf, length) != length)
		perror("nak");
}

/*
 * Send the requested file in netascii.
 */
nasendfile(pf)
	struct format *pf;
{
	register struct tftphdr *tp;
	register int block = 1, size, n;
	int needlf, neednul;
	register char c, *p;
	FILE *ffd = fdopen(fd, "r");

	signal(SIGALRM, timer);
	tp = (struct tftphdr *)buf;
	needlf = neednul = 0;
	do {
		for (size = 0, p = tp->th_data; size < SEGSIZE; size++) {
			if (needlf) {
				needlf = 0;
				*p++ = '\n';
			} else if (neednul) {
				neednul = 0;
				*p++ = '\0';
			} else if ((c = getc(ffd)) == EOF) {
				break;
			} else if (c == '\n') {
				*p++ = '\r';
				needlf = 1;
			} else if (c == '\r') {
				*p++ = c;
				neednul = 1;
			} else {
				*p++ = c;
			}
		}
		tp->th_opcode = htons((u_short)DATA);
		tp->th_block = htons((u_short)block);
		timeout = 0;
		alarm(rexmtval);
rexmt:
		if (write(f, buf, size + 4) != size + 4) {
			perror("tftpd: write");
			break;
		}
again:
		n = read(f, buf, sizeof (buf));
		if (n <= 0) {
			if (n == 0)
				goto again;
			if (errno == EINTR)
				goto rexmt;
			alarm(0);
			perror("tftpd: read");
			break;
		}
		alarm(0);
		tp->th_opcode = ntohs((u_short)tp->th_opcode);
		tp->th_block = ntohs((u_short)tp->th_block);
		if (tp->th_opcode == ERROR)
			break;
		if (tp->th_opcode != ACK || tp->th_block != block)
			goto again;
		block++;
	} while (size == SEGSIZE);
	(void) fclose(ffd);
}

/*
 * Receive a file in netascii.
 */
narecvfile(pf)
	struct format *pf;
{
	register struct tftphdr *tp;
	register int block = 0, n, size;
	int crseen;
	register char *p;
	FILE *ffd = fdopen(fd, "w");

	signal(SIGALRM, timer);
	tp = (struct tftphdr *)buf;
	crseen = 0;
	do {
		timeout = 0;
		alarm(rexmtval);
		tp->th_opcode = htons((u_short)ACK);
		tp->th_block = htons((u_short)block);
		block++;
rexmt:
		if (write(f, buf, 4) != 4) {
			perror("tftpd: write");
			break;
		}
again:
		n = read(f, buf, sizeof (buf));
		if (n <= 0) {
			if (n == 0)
				goto again;
			if (errno == EINTR)
				goto rexmt;
			alarm(0);
			perror("tftpd: read");
			break;
		}
		alarm(0);
		tp->th_opcode = ntohs((u_short)tp->th_opcode);
		tp->th_block = ntohs((u_short)tp->th_block);
		if (tp->th_opcode == ERROR)
			break;
		if (tp->th_opcode != DATA || block != tp->th_block)
			goto again;
		size = n - 4;
		for (p = tp->th_data; p < &(tp->th_data[size]); p++) {
			if (crseen) {
				crseen = 0;
				if (*p == '\n') {
					putc(*p, ffd);
				} else {
					putc('\r', ffd);
					if (*p != '\0')
						putc(*p, ffd);
				}
			} else if (*p == '\r') {
				crseen = 1;
			} else {
				putc(*p, ffd);
			}
		}
	} while (size == SEGSIZE);
	tp->th_opcode = htons((u_short)ACK);
	tp->th_block = htons((u_short)(block));
	(void) write(f, buf, 4);
	(void) fclose(ffd);
}
