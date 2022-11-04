/*
 * SCCSID: @(#)ht.c	3.0	4/21/86
 */
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * TM02/3 - TU16/TE16/TU77 standalone tape driver
 * Fred Canter
 */

#include <sys/param.h>
#include <sys/inode.h>
#include "saio.h"

struct	device
{
	int	htcs1;
	int	htwc;
	caddr_t	htba;
	int	htfc;
	int	htcs2;
	int	htds;
	int	hter;
	int	htas;
	int	htck;
	int	htdb;
	int	htmr;
	int	htdt;
	int	htsn;
	int	httc;
	int	htbae;	/* 11/70 bus extension */
	int	htcs3;
};




#define	GO	01
#define	WCOM	060
#define	RCOM	070
#define	NOP	0
#define	WEOF	026
#define	SFORW	030
#define	SREV	032
#define	ERASE	024
#define	REW	06
#define	DCLR	010
#define CLR	040
#define P800	01300		/* 800 + pdp11 mode */
#define	P1600	02300		/* 1600 + pdp11 mode */
#define	IENABLE	0100
#define	RDY	0200
#define	TM	04
#define	DRY	0200
#define EOT	02000
#define CS	02000
#define COR	0100000
#define PES	040
#define WRL	04000
#define MOL	010000
#define PIP	020000
#define ERR	040000
#define FCE	01000
#define	TRE	040000
#define HARD	064023	/* UNS|OPI|NEF|FMT|RMR|ILR|ILF */

#define	SIO	1
#define	SSFOR	2
#define	SSREV	3
#define SRETRY	4
#define SCOM	5
#define SOK	6

htopen(io)
register struct iob *io;
{
	register skip;
	int i;

	htstrategy(io, REW);
	skip = io->i_boff;
	while (skip--) {
		io->i_cc = -1;
		while (htstrategy(io, SFORW))
			;
		i = 0;
		while (--i)
			;
		htstrategy(io, NOP);
	}
	return(0);
}

htclose(io)
register struct iob *io;
{
	htstrategy(io, REW);
}

htstrategy(io, func)
register struct iob *io;
{
	register struct device *htaddr;
	register unit;
	int den, errcnt;

	htaddr = devsw[io->i_ino.i_dev].dv_csr;
	unit = io->i_unit;
	errcnt = 0;
retry:
	htaddr->htcs2 = 0;
	if(unit > 3)
		den = P1600;
	else
		den = P800;
	htquiet(htaddr);
	htaddr->httc = (unit&03) | den;
	htaddr->htba = io->i_ma;
	htaddr->htfc = -io->i_cc;
	htaddr->htwc = -(io->i_cc>>1);
	den = ((segflag) << 8) | GO;
	if (func == READ)
		den =| RCOM;
	else if (func == WRITE)
		den =| WCOM;
	else if (func == SREV) {
		htaddr->htfc = -1;
		htaddr->htcs1 = den | SREV;
		return(0);
	} else
		den |= func;
	htaddr->htcs1 = den;
	while ((htaddr->htcs1&RDY) == 0)
		;
	if (htaddr->htds&TM) {
		htinit(htaddr);
		return(0);
	}
	if (htaddr->htcs1&TRE) {
		if (errcnt == 0)
			printf("\nHT unit %d tape error: cs2=%o, er=%o",
			    (unit&3), htaddr->htcs2, htaddr->hter);
		htinit(htaddr);
		if (errcnt == 10) {
			printf("\n(FATAL ERROR)\n");
			return(-1);
		}
		errcnt++;
		htstrategy(io, SREV);
		goto retry;
	}
	if (errcnt)
		printf("\n(RECOVERED by retry)\n");
	return(io->i_cc+htaddr->htfc);
}

htinit(htaddr)
register struct device *htaddr;
{
	int omt, ocs2;

	omt = htaddr->httc & 03777;
	ocs2 = htaddr->htcs2 & 07;

	htaddr->htcs2 = CLR;
	htaddr->htcs2 = ocs2;
	htaddr->httc = omt;
	htaddr->htcs1 = DCLR|GO;
}

htquiet(htaddr)
register struct device *htaddr;
{
	while ((htaddr->htcs1&RDY) == 0)
		;
	while (htaddr->htds&PIP)
		;
}
