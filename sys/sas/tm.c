/*
 * SCCSID: @(#)tm.c	3.0	4/21/86
 */
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * TM11 - TU10/TE10/TS03 standalone tape driver
 * Fred Canter
 */

#include <sys/param.h>
#include <sys/inode.h>
#include "saio.h"

struct device {
	int	tmer;
	int	tmcs;
	int	tmbc;
	char	*tmba;
	int	tmdb;
	int	tmrd;
};


#define	GO	01
#define	RCOM	02
#define	WCOM	04
#define	WEOF	06
#define	SFORW	010
#define	SREV	012
#define	WIRG	014
#define	REW	016
#define	DENS	060000		/* 9-channel */
#define	IENABLE	0100
#define	CRDY	0200
#define GAPSD	010000
#define	TUR	1
#define	SDWN	010
#define	HARD	0102200	/* ILC, EOT, NXM */
#define	EOF	0040000

#define	SSEEK	1
#define	SIO	2


tmrew(io)
register struct iob *io;
{
	tmstrategy(io, REW);
}

tmopen(io)
register struct iob *io;
{
	register skip;

	tmstrategy(io, REW);
	skip = io->i_boff;
	while (skip--) {
		io->i_cc = 0;
		while (tmstrategy(io, SFORW))
			;
	}
	return(0);
}
tmstrategy(io, func)
register struct iob *io;
{
	register struct device *tmaddr;
	register int com;
	int errcnt, unit;

	tmaddr = devsw[io->i_ino.i_dev].dv_csr;
	unit = io->i_unit;
	errcnt = 0;
retry:
	tmquiet(tmaddr);
	com = (unit<<8)|(segflag<<4)|DENS;
	tmaddr->tmbc = -io->i_cc;
	tmaddr->tmba = io->i_ma;
	if (func == READ)
		tmaddr->tmcs = com | RCOM | GO;
	else if (func == WRITE)
		tmaddr->tmcs = com | WCOM | GO;
	else if (func == SREV) {
		tmaddr->tmbc = -1;
		tmaddr->tmcs = com | SREV | GO;
		return(0);
	} else
		tmaddr->tmcs = com | func | GO;
	while ((tmaddr->tmcs&CRDY) == 0)
		;
	if (tmaddr->tmer&EOF)
		return(0);
	if (tmaddr->tmer < 0) {
		if (errcnt == 0)
			printf("\nTM unit %d tape error: er=%o cs=%o",
				unit, tmaddr->tmer, tmaddr->tmcs);
		if (errcnt==10) {
			printf("\n(FATAL ERROR)\n");
			return(-1);
		}
		errcnt++;
		tmstrategy(io, SREV);
		goto retry;
	}
	if (errcnt)
		printf("\n(RECOVERED by retry)\n");
	return( io->i_cc+tmaddr->tmbc );
}

tmquiet(tmaddr)
register struct device *tmaddr;
{
	while ((tmaddr->tmcs&CRDY) == 0)
		;
	while ((tmaddr->tmer&TUR) == 0)
		;
	while ((tmaddr->tmer&SDWN) != 0)
		;
}
