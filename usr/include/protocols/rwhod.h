
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/
/*
 * SCCSID: @(#)rwhod.h	3.0	(ULTRIX-11)	5/19/86
 *
 * Based on:
 *	@(#)rwhod.h	5.1 (Berkeley) 5/28/85
 */

/*
 * rwho protocol packet format.
 */
struct	outmp {
	char	out_line[8];		/* tty name */
	char	out_name[8];		/* user id */
	long	out_time;		/* time on */
};

struct	whod {
	char	wd_vers;		/* protocol version # */
	char	wd_type;		/* packet type, see below */
	char	wd_pad[2];
#ifndef	pdp11
	int	wd_sendtime;		/* time stamp by sender */
	int	wd_recvtime;		/* time stamp applied by receiver */
	char	wd_hostname[32];	/* hosts's name */
	int	wd_loadav[3];		/* load average as in uptime */
	int	wd_boottime;		/* time system booted */
	struct	whoent {
		struct	outmp we_utmp;	/* active tty info */
		int	we_idle;	/* tty idle time */
	} wd_we[1024 / sizeof (struct whoent)];
#else	pdp11
	long	wd_sendtime;		/* time stamp by sender */
	long	wd_recvtime;		/* time stamp applied by receiver */
	char	wd_hostname[32];	/* hosts's name */
	long	wd_loadav[3];		/* load average as in uptime */
	long	wd_boottime;		/* time system booted */
	struct	whoent {
		struct	outmp we_utmp;	/* active tty info */
		long	we_idle;	/* tty idle time */
	} wd_we[1024 / sizeof (struct whoent)];
#endif	pdp11
};

#define	WHODVERSION	1
#define	WHODTYPE_STATUS	1		/* host status */
