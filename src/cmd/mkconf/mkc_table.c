
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)mkc_table.c	3.0	4/21/86";
#include "mkconf.h"

/*
 * MSCP disk controller information table,
 * used to create the dds.h header file for dds.c.
 */
struct	mscptab	mstab[] =
{
	"ra",	-1,	8,	0172150,	0154,
	"rc",	-1,	8,	0172150,	0154,
	"rq",	-1,	8,	0172150,	0154,
	"ru",	-1,	8,	0172150,	0154,
	0
};

/*
 * borrow from MSCP for ts
 */
struct	mscptab	tstab[] =
{
	"ts",	-1,	1,	0172520,	0224,
	"ts",	-1,	1,	0172520,	0224,
	"ts",	-1,	1,	0172520,	0224,
	"ts",	-1,	1,	0172520,	0224,
	0
};

/*
 * borrow from MSCP for tmscp
 */
struct	mscptab	tktab[] =
{
	"tk",	-1,	1,	0174500,	0260,
	"tk",	-1,	1,	0174500,	0260,
	"tk",	-1,	1,	0174500,	0260,
	"tk",	-1,	1,	0174500,	0260,
	0
};

struct	tab table[] =
{
	"console",
	-1, 0177560, 060, CHAR+INTR+KL+DLV,
	"	klin; br4\n	klou; br4\n",
	".globl	_klrint\nklin:	jsr	r0,call; jmp _klrint\n",
	".globl	_klxint\nklou:	jsr	r0,call; jmp _klxint\n",
	"",
	"	klopen,   klclose,  klread,   klwrite,\n\tklioctl,  nulldev,  kl11,     ttselect,",
	0,
	"int	klopen(), klclose(), klread(), klwrite(), klioctl();",

	"mem",
	-1, 0, 0, CHAR,
	0,
	0,
	0,
	0,
	"	nulldev,  nulldev,  mmread,   mmwrite,\n\tnodev,    nulldev,  0,        seltrue,",
	0,
	"int	mmread(), mmwrite();",

	"clock",
	-2, 0, 0100, INTR,
	"	kwlp; br6\n",
	".globl	_trap\n\
.globl	_inEMT\n\
ovint:	jsr	r0,call; inc *_inEMT; jmp _trap\n\n\
.globl	_clock\n",	/*@*/
	"kwlp:	jsr	r0,call; jmp _clock\n",
	0,
	0,
	0,
	0,

	"parity",
	-1, 0, 0114, INTR,
	"	trap; br7+10.		/ 11/70 parity\n",
	"",
	"",
	0,
	0,
	0,
	0,

/*
 * 110 unused
 * 114 memory parity
 */
/*
 * 120 was the XY plotter, is now the first DEUNA
 */
	"if_de",
	0, 0174510, 0120, INTR,
	"", /* "	dein; br5+%d.\n", */
	".globl	_deint\ndein:	jsr	r0,call; jmp _deint\n",
	"",
	0,
	0,
	0,
	0,
/*
 * 124 DR11-B
 * 130 AD01
 * 134 AFC11
 * 140 AA11
 * 144 AA11
 */

	"hm",
	0, 0176300, 0150, BLOCK+CHAR+INTR+NUNIT,
	"	hpio; br5+1.\n",
	"",
	"",
	"	hpopen, hpclose, hpstrategy, &hptab[1],",
	"	hpopen,   nulldev,  hpread,   hpwrite,\n\tnodev,    nulldev,  0,        seltrue,",
	"",
	"",

/*
 * CSR and VECTOR = 0, to indicate that we 
 * must search the mstab[] to find them.
 * Special case for multiple MSCP cntlr support.
 */
	"ra",
	0, MSCPDEV, 0, BLOCK+CHAR+INTR+NUNIT+IOS+UTAB,
	"	raio; br5+%d.\n",
	".globl	_raintr\n",
	"raio:	jsr	r0,call; jmp _raintr\n",
	0,
	0,
	0,
	0,

	"rl",
	0, 0174400, 0160, BLOCK+CHAR+INTR+NUNIT+IOS+UTAB,
	"	rlio; br5\n",
	".globl	_rlintr\n",
	"rlio:	jsr	r0,call; jmp _rlintr\n",
	0,
	0,
	0,
	0,

/*
 * 164-174 unused
 */

	"lp",
	0, 0177514, 0200, CHAR+INTR,
	"	lpou; br4\n",
	"",
	".globl _lpintr\nlpou:	jsr	r0,call; jmp _lpintr\n",
	0,
	"	lpopen,   lpclose,  nodev,    lpwrite,\n\tlpioctl,  nulldev,  0,        seltrue,",
	0,
	"int	lpopen(), lpclose(), lpwrite(), lpioctl();",

	"hj",
	0, 0176400, 0204, BLOCK+CHAR+INTR+NUNIT,
	"	hpio; br5+2.\n",
	"",
	"",
	"	hpopen, hpclose, hpstrategy, &hptab[2],",
	"	hpopen,   nulldev,  hpread,   hpwrite,\n\tnodev,    nulldev,  0,        seltrue,",
	"",
	"",

/*
 * 210 RC
 */

	"hk",
	0, 0177440, 0210, BLOCK+CHAR+INTR+NUNIT+IOS+UTAB+BADS,
	"	hkio; br5\n",
	".globl	_hkintr\n",
	"hkio:	jsr	r0,call; jmp _hkintr\n",
	0,
	0,
	0,
	0,

	"rk",
	0, 0177400, 0220, BLOCK+CHAR+INTR+NUNIT+IOS,
	"	rkio; br5\n",
	".globl	_rkintr\n",
	"rkio:	jsr	r0,call; jmp _rkintr\n",
	0,
	0,
	0,
	0,

	"tm",
	0, 0172520, 0224, BLOCK+CHAR+INTR+TAPE,
	"	tmio; br5\n",
	".globl	_tmintr\n",
	"tmio:	jsr	r0,call; jmp _tmintr\n",
	0,
	0,
	0,
	0,

	"ht",
	0, 0172440, 0224, BLOCK+CHAR+INTR+TAPE,
	"	htio; br5\n",
	".globl	_htintr\n",
	"htio:	jsr	r0,call; jmp _htintr\n",
	0,
	0,
	0,
	0,

	"ts",
	0, 0172520, 0224, BLOCK+CHAR+INTR+TAPE,
	"	tsio; br5+%d\n",
	".globl	_tsintr\n",
	"tsio:	jsr	r0,call; jmp _tsintr\n",
	0,
	0,
	0,
	0,

	"cr",
	0, 0177160, 0230, CHAR+INTR,
	"	crin; br6\n",
	"",
	".globl	_crint\ncrin:	jsr	r0,call; jmp _crint\n",
	0,
	"	cropen,   crclose,  crread,   nodev,    nodev,    nulldev,  0,        seltrue,",
	0,
	"int	cropen(), crclose(), crread();",

/*
 * 234 UDC11
 */

	"rp",
	0, 0176710, 0254, BLOCK+CHAR+INTR+NUNIT+IOS,
	"	rpio; br5\n",
	".globl	_rpintr\n",
	"rpio:	jsr	r0,call; jmp _rpintr\n",
	"	nulldev, rpclose, rpstrategy, &rptab,",
	"	nulldev,  nulldev,  rpread,   rpwrite,\n\tnodev,    nulldev,  0,        seltrue,",
	"int	rpstrategy(),rpclose();\nstruct	buf	rptab;",
	"int	rpread(), rpwrite();",

	"hp",
	0, 0176700, 0254, BLOCK+CHAR+INTR+NUNIT,
	"	hpio; br5+0.\n",
	".globl	_hpintr\n",
	"hpio:	jsr	r0,call; jmp _hpintr\n",
	"	hpopen, hpclose, hpstrategy, &hptab[0],",
	0,
	"int	hpopen(),hpclose(), hpstrategy();\nstruct	buf	hptab[];",
	0,

/*
 * 260 TA11 (alt TS11)
 */

	"tk",
	0, 0174500, 0260, BLOCK+CHAR+INTR+TAPE,
	"	tkio; br5+%d\n",
	".globl	_tkintr\n",
	"tkio:	jsr	r0,call; jmp _tkintr\n",
	0,
	0,
	0,
	0,

	"hx",
	0, 0177170, 0264, BLOCK+CHAR+INTR+NUNIT,
	"	hxio; br5\n",
	".globl	_hxintr\n",
	"hxio:	jsr	r0,call; jmp _hxintr\n",
	0,
	"	hxopen,   nulldev,  hxread,   hxwrite,\n\thxioctl,  nulldev,  0,        seltrue,",
	0,
	"int	hxread(), hxwrite(), hxioctl();",

	"dc",
	0, 0174000, 0310, CHAR+INTR+DLV,
	"	dcin; br5+%d.\n	dcou; br5+%d.\n",
	".globl	_dcrint\ndcin:	jsr	r0,call; jmp _dcrint\n",
	".globl	_dcxint\ndcou:	jsr	r0,call; jmp _dcxint\n",
	0,
	"	dcopen,   dcclose,  dcread,   dcwrite,\n\tdcioctl,  nulldev,  dc11,     seltrue,",
	0,
	"int	dcopen(), dcclose(), dcread(), dcwrite(), dcioctl();\nstruct	tty	dc11[];",

	"kl",
	0, 0176500, 0310, INTR+KL+DLV,
	"	klin; br4+%d.\n	klou; br4+%d.\n",
	"",
	"",
	0,
	0,
	0,
	0,

	"dp",
	0, 0174770, 0310, CHAR+INTR+DLV,
	"	dpin; br6+%d.\n	dpou; br6+%d.\n",
	".globl	_dprint\ndpin:	jsr	r0,call; jmp _dprint\n",
	".globl	_dpxint\ndpou:	jsr	r0,call; jmp _dpxint\n",
	0,
	"	dpopen,   dpclose,  dpread,   dpwrite,\n\tnodev,    nulldev,  0,        seltrue,",
	0,
	"int	dpopen(), dpclose(), dpread(), dpwrite();",

/*
 * DM11-A
 */

	"dn",
	0, 0175200, 0304, CHAR+INTR,
	"	dnou; br5+%d.\n",
	"",
	".globl	_dnint\ndnou:	jsr	r0,call; jmp _dnint\n",
	0,
	"	dnopen,   dnclose,  nodev,    dnwrite,\n\tnodev,    nulldev,  0,        seltrue,",
	0,
	"int	dnopen(), dnclose(), dnwrite();",

	"dhdm",
	0, 0170500, 0304, INTR,
	"	dmin; br4+%d.\n",
	"",
	".globl	_dmint\ndmin:	jsr	r0,call; jmp _dmint\n",
	0,
	0,
	0,
	0,

/*
 * DR11-A+
 * DR11-C+	(also CAT)
 */

	"ct",
	0, 0167770, 0304, CHAR+INTR,
	"	ctou; br4\n",
	"",
	".globl	_ctintr\nctou:	jsr	r0,call; jmp _ctintr\n",
	0,
	"	ctopen,   ctclose,  nodev,    ctwrite,\n\tnodev,    nulldev,  0,        seltrue,",
	0,
	"int	ctopen(), ctclose(), ctwrite();",

/*
 * PA611+
 * PA611+
 * DT11+
 * DX11+
 */

	"dl",
	0, 0175610, 0310, INTR+KL+DLV,
	"	klin; br4+%d.\n	klou; br4+%d.\n",
	"",
	"",
	0,
	0,
	0,
	0,

/*
 * DJ11
 */

	"dh",
	0, 0160020, 0310, CHAR+INTR+EVEN+DLV,
	"	dhin; br5+%d.\n	dhou; br5+%d.\n",
	".globl	_dhrint\ndhin:	jsr	r0,call; jmp _dhrint\n",
	".globl	_dhxint\ndhou:	jsr	r0,call; jmp _dhxint\n",
	0,
	"	dhopen,   dhclose,  dhread,   dhwrite,\n\tdhioctl,  dhstop,   dh11,     ttselect,",
	0,
	"int	dhopen(), dhclose(), dhread(), dhwrite(), dhioctl(), dhstop();\n",

/* Order in table MUST be UH then UHV, nothing in between! */
	"uh",
	0, 0160440, 0310, CHAR+INTR+EVEN+DLV,
	"	uhin; br5+%d.\n	uhou; br5+%d.\n",
	".globl	_uhrint\nuhin:	jsr	r0,call; jmp _uhrint\n",
	".globl	_uhxint\nuhou:	jsr	r0,call; jmp _uhxint\n",
	0,
	"	uhopen,   uhclose,  uhread,   uhwrite,\n\tuhioctl,  uhstop,   uh11,     ttselect,",
	0,
	"int	uhopen(), uhclose(), uhread(), uhwrite(), uhioctl(), uhstop();\n",
	"uhv",
	0, 0160440, 0310, CHAR+INTR+EVEN+DLV,
	"	uhin; br5+%d.\n	uhou; br5+%d.\n",
	".globl	_uhrint\nuhin:	jsr	r0,call; jmp _uhrint\n",
	".globl	_uhxint\nuhou:	jsr	r0,call; jmp _uhxint\n",
	0,
	"	uhopen,   uhclose,  uhread,   uhwrite,\n\tuhioctl,  uhstop,   uh11,     ttselect,",
	0,
	"int	uhopen(), uhclose(), uhread(), uhwrite(), uhioctl(), uhstop();\n",

/*
 * GT40
 * LPS+
 * DQ11
 * KW11-W
 */

	"du",
	0, 0176040, 0310, CHAR+INTR+DLV,
	"	duin; br6+%d.\n	duou; br6+%d.\n",
	".globl	_durint\nduin:	jsr	r0,call; jmp _durint\n",
	".globl	_duxint\nduou:	jsr	r0,call; jmp _duxint\n",
	0,
	"	duopen,   duclose,  duread,   duwrite,\n\tnodev,    nulldev,  0,        seltrue,",
	0,
	"int	duopen(), duclose(), duread(), duwrite();",

/*
 * DUP11
 * DV11
 * LK11-A
 * DMC11
 */

/* Order in table MUST be DZ then DZV, nothing in between! */
	"dz",
	0, 0160100, 0310, CHAR+INTR+EVEN+DLV,
	"	dzin; br5+%d.\n	dzou; br5+%d.\n",
	".globl _dzrint\ndzin:	jsr	r0,call; jmp _dzrint\n",
	".globl _dzxint\ndzou:	jsr	r0,call; jmp _dzxint\n",
	0,
	"	dzopen,   dzclose,  dzread,   dzwrite,\n\tdzioctl,  nulldev,  dz_tty,   ttselect,",
	0,
	0,

	"dzv",
	0, 0160100, 0310, CHAR+INTR+EVEN+DLV,
	"	dzin; br5+%d.\n	dzou; br5+%d.\n",
	".globl _dzrint\ndzin:	jsr	r0,call; jmp _dzrint\n",
	".globl _dzxint\ndzou:	jsr	r0,call; jmp _dzxint\n",
	0,
	"	dzopen,   dzclose,  dzread,   dzwrite,\n\tdzioctl,  nulldev,  dz_tty,   ttselect,",
	0,
	"int	dzopen(), dzclose(), dzread(), dzwrite(), dzioctl();\n",

	"tty",
	1, 0, 0, CHAR,
	0,
	0,
	0,
	0,
	"	syopen,   nulldev,  syread,   sywrite,\n\tsysioctl, nulldev,  0,        syselect,",
	0,
	"int	syopen(), syread(), sywrite(), sysioctl(), syselect();",

	"ptc",
	1, 0, 0, CHAR,
	0,
	0,
	0,
	0,
	"	ptcopen,  ptcclose, ptcread,  ptcwrite,\n\tptyioctl, nulldev,  pt_tty,   ptcselect,",
	0,
	"int	ptcopen(), ptcclose(), ptcread(), ptcwrite(), ptyioctl(), ptcselect();",

	"pts",
	1, 0, 0, CHAR,
	0,
	0,
	0,
	0,
	"	ptsopen,  ptsclose, ptsread,  ptswrite,\n\tptyioctl, ptsstop,  pt_tty,   ttselect,",
	0,
	"int	ptsopen(), ptsclose(), ptsread(), ptswrite(), ptsstop();",

	"if_qe",
	0, 0174440, 0400, INTR,
	"", /* "	qein; br5+%d.\n", */
	".globl	_qeint\nqein:	jsr	r0,call; jmp _qeint\n",
	"",
	0,
	0,
	0,
	0,

	"if_n1",
	0, 0164000, 0310, INTR,
	"", /* "	n1in; br5+%d.\n", */
	".globl	_n1int\nn1in:	jsr	r0,call; jmp _n1int\n",
	".globl	_n1xint\nn1ou:	jsr	r0,call; jmp _n1xint\n",
	0,
	0,
	0,
	0,

	"if_n2",
	0, 0164000, 0310, INTR,
	"", /* "	n2in; br5+%d.\n", */
	".globl	_n2int\nn2in:	jsr	r0,call; jmp _n2int\n",
	".globl	_n2xint\nn2ou:	jsr	r0,call; jmp _n2xint\n",
	0,
	0,
	0,
	0,

	"u1",
	0, 0164000, 0310, BLOCK+CHAR+INTR+EVEN+DLV,
	"	u1in; br5+%d.\n	u1ou; br5+%d.\n",
	".globl	_u1rint\nu1in:	jsr	r0,call; jmp _u1rint\n",
	".globl	_u1xint\nu1ou:	jsr	r0,call; jmp _u1xint\n",
	"	u1open, u1close, u1strategy, &u1tab,",
	"	u1open,   u1close,  u1read,   u1write,\n\tu1ioctl,  u1stop,   u1_tty,   u1select,",
	"int	u1open(), u1close(), u1strategy();\nstruct\tbuf\tu1tab;",
	"int	u1read(), u1write(), u1ioctl(), u1stop(), u1select();\n",

	"u2",
	0, 0164000, 0310, BLOCK+CHAR+INTR+EVEN+DLV,
	"	u2in; br5+%d.\n	u2ou; br5+%d.\n",
	".globl	_u2rint\nu2in:	jsr	r0,call; jmp _u2rint\n",
	".globl	_u2xint\nu2ou:	jsr	r0,call; jmp _u2xint\n",
	"	u2open, u2close, u2strategy, &u2tab,",
	"	u2open,   u2close,  u2read,   u2write,\n\tu2ioctl,  u2stop,   u2_tty,   u2select,",
	"int	u2open(), u2close(), u2strategy();\nstruct\tbuf\tu2tab;",
	"int	u2read(), u2write(), u2ioctl(), u2stop(), u2select();\n",

	"u3",
	0, 0164000, 0310, BLOCK+CHAR+INTR+EVEN+DLV,
	"	u3in; br5+%d.\n	u3ou; br5+%d.\n",
	".globl	_u3rint\nu3in:	jsr	r0,call; jmp _u3rint\n",
	".globl	_u3xint\nu3ou:	jsr	r0,call; jmp _u3xint\n",
	"	u3open, u3close, u3strategy, &u3tab,",
	"	u3open,   u3close,  u3read,   u3write,\n\tu3ioctl,  u3stop,   u3_tty,   u3select,",
	"int	u3open(), u3close(), u3strategy();\nstruct\tbuf\tu3tab;",
	"int	u3read(), u3write(), u3ioctl(), u3stop(), u3select();\n",

	"u4",
	0, 0164000, 0310, BLOCK+CHAR+INTR+EVEN+DLV,
	"	u4in; br5+%d.\n	u4ou; br5+%d.\n",
	".globl	_u4rint\nu4in:	jsr	r0,call; jmp _u4rint\n",
	".globl	_u4xint\nu4ou:	jsr	r0,call; jmp _u4xint\n",
	"	u4open, u4close, u4strategy, &u4tab,",
	"	u4open,   u4close,  u4read,   u4write,\n\tu4ioctl,  u4stop,   u4_tty,   u4select,",
	"int	u4open(), u4close(), u4strategy();\nstruct\tbuf\tu4tab;",
	"int	u4read(), u4write(), u4ioctl(), u4stop(), u4select();\n",

	0
};
