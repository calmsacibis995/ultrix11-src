
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
**  CONF.H -- All user-configurable parameters for sendmail
**
**	Based on @(#)conf.h	4.1		7/25/83
**	@(#)conf.h	3.0	4/22/86
*/



/*
**  Table sizes, etc....
**	There shouldn't be much need to change these....
*/

# define MAXLINE	256		/* max line length */
# define MAXNAME	128		/* max length of a name */
# define MAXFIELD	2500		/* max total length of a hdr field */
# define MAXPV		40		/* max # of parms to mailers */
# define MAXHOP		30		/* max value of HopCount */
# define MAXATOM	100		/* max atoms per address */
#ifndef	SMALL
# define MAXMAILERS	25		/* maximum mailers known to system */
#else	SMALL
# define MAXMAILERS	15		/* maximum mailers known to system */
#endif	SMALL
# define MAXRWSETS	30		/* max # of sets of rewriting rules */
# define MAXPRIORITIES	25		/* max values for Precedence: field */
#ifndef	SMALL
# define MAXTRUST	30		/* maximum number of trusted users */
#else	SMALL
# define MAXTRUST	20		/* maximum number of trusted users */
#endif	SMALL

/*
**  Compilation options.
*/

#define DBM		1	/* use DBM library (requires -ldbm) */
#ifndef	SMALL
#define DEBUG		1	/* enable debugging */
#endif	SMALL
#define LOG		1	/* enable logging */
#define SMTP		1	/* enable user and server SMTP */
#define QUEUE		1	/* enable queueing */
#define UGLYUUCP	1	/* output ugly UUCP From lines */
#define DAEMON		1	/* include the daemon (requires IPC) */
