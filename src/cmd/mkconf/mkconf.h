
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * Sccsid: @(#)mkconf.h	3.0	4/21/86
 * Definitions and structures for mkconf
 */

#include <stdio.h>

#define	HT	1
#define	TM	2
#define	TS	3
#define	RL	4
#define	RA	5
#define	RC	6
#define	RD	7
#define	RX	8
#define	HK	9
#define	RP	10
#define	HP	11
#define	TK	12
#define CHAR	01
#define BLOCK	02
#define INTR	04
#define EVEN	010	/* vector must start on mod 10 boundry */
#define KL	020
#define ROOT	040
#define	SWAP	0100
#define	PIPE	0200
#define	ERRLOG	0400
#define	NSAV	01000
#define	DLV	02000	/* vector 4 word instead of 2 */
#define	NUNIT	04000	/* count field is number of drives */
#define	IOS	010000	/* needs iostat structure definiton */
#define	UTAB	020000	/* needs ??utab definition - hp, hk */
#define	TAPE	040000	/* needs tape arrays & number of units */
#define	BADS	0100000	/* needs bad structures */

/*
 * The ovtab structure describes each of the modules
 * used to make the overlay text segments of the
 * overlay kernel.
 * The order of appearance of the modules in this
 * structure is critical and must not be changed.
 */

struct	ovtab
{
	char	*mn;		/* object module name */
	int	mc;		/* non-zero if module is configured */
				/* some modules are always configured, */
				/* others are keyed from COUNT */
				/* field of structure "table" above */
	int	ovno;		/* initial overlay number, */
				/* may be changed later */
	int	mts;		/* module text size in bytes */
	char	*mpn;		/* module full path name */
};
extern struct ovtab ovt[];	/* initialized in mkc_ovt.c */
extern struct ovtab sovt[];	/* initialized in mkc_ovt.c */
extern struct ovtab ssovt[];	/* initialized in mkc_ovt.c */
extern struct ovtab netovt[];	/* initialized in mkc_ovt.c */
extern struct ovtab snetovt[];	/* initialized in mkc_ovt.c */

				/* initialized in mkc_str.c */
extern char *stra40[], *stra70[], *stra1[], *stra2id[], *stra2ov[],
	*stra3[], *strb[], *strc[], *osizcmd, *cmcmd, *strovh[],
	*strovh1[], *strovl1[], *ssizcmd,
	*strovl[], *strovz[], *strd1[], *strd2[], *strd3[], *strd4[],
	*stre[], *stre1[], *strf[], *strf1[], strg[], strg1[],
	*strg1a[], strg2[], strg3[], strg4[], *strh[],
	*strj[], *strl1[], *strl2[], *strm[], *strm1[], *strm2[],
	strn[], *stro[], *strfpe[], *strnofpe[], *strq[], *strgk[],
	*strutsn[], *strmausno[], *strsemno[], *strmsgno[], *strflckno[],
	*strshuff[];

struct tab
{
	char	*name;
	int	count;
	int	csr;
	int	vec;
	int	key;
	char	*codea;
	char	*codeb;
	char	*codec;
	char	*coded;
	char	*codee;
	char	*codef;
	char	*codeg;
};
extern struct tab table[];	/* initialized in mkc_table.c */

/*
 * MSCP disk controller information table.
 * Used to create uda.h.
 */
#define	MSCPDEV	-1
struct	mscptab
{
	char	*ms_dcn;	/* disk cntlr name (ra, rc, rq, ru) */
	int	ms_cn;		/* MSCP cntlr number (-1 = not configured) */
	int	ms_nra;		/* number of units for this cntlr */
	int	ms_csr;		/* CSR address for this cntlr */
	int	ms_vec;		/* interrupt vector address of this cntlr */
};
extern	struct	mscptab	mstab[];	/* initialized in mkc_table.c */

/*
 * borrow the MSCP structure for ts
 */
extern	struct	mscptab	tstab[];	/* initialized in mkc_table.c */

/*
 * borrow the MSCP structure for tk
 */
extern	struct	mscptab	tktab[];	/* initialized in mkc_table.c */

/*
 * The ovdtab is an array of structures which
 * describe the actual overlays as they will
 * appear in the "ovload" overlay load file.
 * The first structure (overlay 0) is never used.
 */

struct	ovdes
{
	int	nentry;		/* number of modules in this overlay */
	int	size;		/* total size of this overlay in bytes */
	char	*omns[12];	/* pointers to module pathname strings */
};
extern struct ovdes ovdtab[];
extern char *btab[], *ctab[];	/* initialized in mkc_btab.c */

/* the rest of this stuff is declared and/or initialized in globl.c */

extern int	sid, ov, nfp, dump, ts_csr, tm_csr, ht_csr, rl_csr;
extern int	rc_csr, ubmap, generic;
extern int	rd_csr, rootmaj, rootmin, swapmaj, swapmin, pipemaj, pipemin;
extern long	swplo, dumplo, dumphi, elsb;
extern int	nswap, dumpdn, kfpsim, nldisp, elmaj, elmin, elnb;
extern int	nbuf, ninode, nfile, nmount, nproc, ntext, nclist;
extern int	maxuprc, mapsize, elbsiz, timezone, dstflag;
extern int	ncargs, hz, msgbufs, ncall, first;
extern	int	ra_cn;
extern unsigned int maxseg;
extern long ulimit;

extern char	omn[], mtsize[];

/* SYSTEM V compatability */
extern int	ipc;
extern int	sema, semmap, semmni, semmns, semmnu, semume, semmsl, semopm;
extern int	semvmx, semaem;
extern int	mesg, msgmax, msgmnb, msgtql, msgssz, msgseg, msgmap, msgmni;
extern int	flock, flckrec, flckfil;
extern int	maus, shuffle;
extern int	nmaus, mausize[];
extern int	network, allocs, mbufs, miosize, arptab, npty;

struct netattach {
	char name[2];
	int csr;
	int vec;
	int unit;
	int nvec;
}; 
extern struct	netattach netattach[];
extern int	nnetattach;
