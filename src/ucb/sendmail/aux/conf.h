
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
**  CONF.H -- definitions of compilation flags needed everywhere.
**
**	This, together with conf.c, should be all the configuration
**	information needed.  This stuff could be in a makefile, but
**	we prefer to keep this file very small because it is different
**	on a number of machines.
**
**	@(#)conf.h	4.1	7/25/83
*/


# define	DEBUG		/* turn on debugging information */

/* # define	NEWFTP		/* use new ftp reply codes */

/* # define	NOTUNIX		/* don't use Unix-style mail headers */

# ifdef INGRES
# define	LOG		/* turn on logging */
				/* this flag requires -lsys in makefile */
# endif INGRES

# ifdef VAX
# define	VFORK		/* use the vfork syscall */
# endif VAX

# ifndef V6
# define	DBM		/* use the dbm library */
				/* this flag requires -ldbm in makefile */
# endif V6
