
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#
/*	sccsid: @(#)defs.h 3.0 4/21/86	*/
/*
 *
 *	UNIX debugger - common definitions
 *
 *	Modified for overlay text support,
 *	changes flagged by the word OVERLAY
 *
 *	Fred Canter 4/1/82
 *
 */



/*	Layout of a.out file (fsym):
 *
 *	header of 8 words	magic number 405, 407, 410, 411
 *				text size	)
 *				data size	) in bytes but even
 *				bss size	)
 *				symbol table size
 *				entry point
 *				{unused}
 *				flag set if no relocation
 *
 *
 *	header:		0
 *	text:		16
 *	data:		16+textsize
 *	relocation:	16+textsize+datasize
 *	symbol table:	16+2*(textsize+datasize) or 16+textsize+datasize
 *
 */

/*	Layout of a.out file (fsym) for overlay text:
 *
 *	header of 8 words	magic number 430, 431
 *				text size	)
 *				data size	) in bytes but even
 *				bss size	)
 *				symbol table size
 *				entry point
 *				{unused}
 *				flag set if no relocation
 *
 *	overlay text header	size of largest overlay in bytes
 *	8 words
 *				overlay 1	) size in bytes,
 *				overlay 2	) 0 if overlay not used
 *				overlay 3	)
 *				overlay 4	)
 *				overlay 5	)
 *				overlay 6	)
 *				overlay 7	)
 *
 *	header:		0
 *	root text:	32
 *	overlay text:	32+textsize
 *	data:		32+textsize+overlay textsize
 *	relocation:	32+textsize+overlay textsize+datasize
 *	symbol table:	32 + 2*(textsize+overlay textsize+datasize) or
 *			32+textsize+overlay textsize+datasize
 *
 */


#include <sys/param.h>
#include <sys/dir.h>
#include <sys/reg.h>
#include <sys/user.h>
#include <sgtty.h>
#include "mac.h"
#include "mode.h"
#ifdef DEBUG
#include <stdio.h>
extern FILE *tracef;
extern int debugflg;
#endif DEBUG


#define VARB	11
#define VARD	13
#define VARE	14
#define VARM	22
#define VARS	28
#define VART	29

#define COREMAGIC 0140000

#define RD	0
#define WT	1
#define NSP	0
#define	ISP	1
#define	DSP	2
#define STAR	4
#define	OVRLAY	010	/* OVERLAY - flag for overlay mode (&) in use */
#define STARCOM 0200
#define DSYM	7
#define ISYM	2
#define ASYM	1
#define NSYM	0
#define ESYM	(-1)
#define BKPTSET	1
#define BKPTEXEC 2
#define	SYMSIZ	100
#define MAXSIG	20

/* OVERLAY - position of pc & ps on stack frame */
/* NOT USED !, stack moved to below user structure in U block
	       so these values are no longer valid !
#ifdef OVERLAY
#define USERPS	2*(512-2)
#define USERPC	2*(512-3)
#else
#define USERPS	2*(512-1)
#define USERPC	2*(512-2)
#endif
*/

#define BPT	03
#define FD	0200
#define	SETTRC	0
#define	RDUSER	2
#define	RIUSER	1
#define	WDUSER	5
#define WIUSER	4
#define	RUREGS	3
#define	WUREGS	6
#define	CONTIN	7
#define	SINGL	9	/* OVERLAY - was SINGLE, conflict with opsec.c */
#define	EXIT	8

#define	FROFF	(&(((struct user *)0)->u_fps))
#define	FRLEN	(sizeof(((struct user *)0)->u_fps))
#define FRMAX	6
#define	FERROFF	(&(((struct user *)0)->u_fperr))
#define	FERRLEN	(sizeof(((struct user *)0)->u_fperr))
#define	UROFF	(sizeof(((struct user *)0)->u_stack))

/* stack frame register offsets */
#define	dev	R6-4	/* trap type */
#define	ps	RPS-3
#define	pc	PC-3
#define	sp	R6-3
#define	r5	R5-3
#define	r4	R4-3
#define	r3	R3-3
#define	r2	R2-3
#define	r1	R1-3
#define	r0	R0-3

#define MAXOFF	1023
#define MAXPOS	80
#define MAXLIN	128
#ifndef DEBUG
#define EOF	0
#endif DEBUG
#define EOR	'\n'
#define TB	'\t'
#define QUOTE	0200
#define STRIP	0177
#define LOBYTE	0377
#define EVEN	-2


/* long to ints and back (puns) */
union {
	INT	I[2];
	L_INT	L;
} itolws;

#define leng(a)		((long)((unsigned)(a)))
#define shorten(a)	((int)(a))
#define itol(a,b)	(itolws.I[0]=(a), itolws.I[1]=(b), itolws.L)



/* result type declarations */
L_INT		inkdot();
SYMPTR		lookupsym();
SYMPTR		symget();
POS		get();
POS		chkget();
STRING		exform();
L_INT		round();
BKPTR		scanbkpt();
VOID		fault();

typedef struct sgttyb TTY;
TTY	adbtty, usrtty;
#include <setjmp.h>
jmp_buf erradb;
unsigned sizeov;
