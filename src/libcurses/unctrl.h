
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/* SCCSID: @(#)unctrl.h	3.0	4/22/86 */
/*
 * unctrl.h
 *
 * 2/17/82 (Berkeley) @(#)unctrl.h	1.2
 */

extern	char	*_unctrl[];	/* defined in /usr/src/lib/curses/unctrl.c */

#define	unctrl(ch)	(_unctrl[ch & 0177])
