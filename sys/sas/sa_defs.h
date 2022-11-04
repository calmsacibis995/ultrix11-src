
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)sa_defs.h	3.0	4/21/86
 *
 * Definitions used by standalone programs.
 *
 * Fred Canter 1/14/84
 */

/* EXIT STATUS CODES */
/* NORMAL MUST BE 0, FATAL MUST BE 1 (see mkfs.c, etc.) */

#define	NORMAL	0	/* program completed successfully */
#define	FATAL	1	/* program encountered an unrecoverable error */
#define	HASBADS	2	/* BADS - bad blocks encountered */
#define	NO_BBF	3	/* BADS - pack has NO bad block file */
#define	BADPACK	4	/* DSKINIT - pack cannot be used with V7M-11 */

/*
 * SDLOAD uses an argument buffer to pass information
 * to the standalone programs that it calls.
 * These definitions specify the address and size of
 * the argument buffer, see also srt0.s.
 */

#define	ARGBUF	01012
#define	ARGSIZ	126

/*
 * The exit status of the standalone program is returned
 * to SDLOAD via a the fixed address RTNSTAT, see also srt0.s.
 */

#define	RTNSTAT	01010
