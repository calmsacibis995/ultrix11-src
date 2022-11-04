
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)v7.local.h	3.0	4/22/86
 */
/*
 * Declarations and constants specific to an installation.
 *
 * Vax/Unix version 7.
 *
 *	29-May-84	ma.  Remove sigchild definition.
 *
 */
 
/*
 * Based on
 *	Sccs Id = "@(#)v7.local.h	2.5 1/29/83";
 */

#define	GETHOST				/* System has gethostname syscall */
#ifdef	GETHOST
#define	LOCAL		EMPTYID		/* Dynamically determined local host */
#else
#define	LOCAL		'V'		/* Local host id */
#endif	GETHOST

#define	MAIL		"/bin/mail"	/* Name of mail sender */
#define SENDMAIL	"/usr/lib/sendmail"
					/* Name of classy mail deliverer */
#define	EDITOR		"/bin/ex"	/* Name of text editor */
#define	VISUAL		"/bin/vi"	/* Name of display editor */
#define	SHELL		"/bin/csh"	/* Standard shell */
#define	MORE		"/usr/bin/more"	/* Standard output pager */
#define	HELPFILE	"/usr/lib/Mail.help"
					/* Name of casual help file */
#define	THELPFILE	"/usr/lib/Mail.help.~"
#define	POSTAGE		"/usr/adm/maillog"
					/* Where to audit mail sending */
					/* Name of casual tilde help */
#define	UIDMASK		0177777		/* Significant uid bits */
#define	MASTER		"/usr/lib/Mail.rc"
#define	APPEND				/* New mail goes to end of mailbox */
#define CANLOCK				/* Locking protocol actually works */
#define	UTIME				/* System implements utime(2) */

#ifndef VMUNIX
/*#include "sigretro.h"		*/	/* Retrofit signal defs */
#endif VMUNIX
