/*
 * SCCSID: @(#)rk.c	3.0	4/21/86
 */
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * RK05 standalone disk driver
 * Fred Canter
 */

#include <sys/param.h>
#include <sys/inode.h>
#include "saio.h"

#define	NRK	4
#define	NRKBLK	4872

#define	RESET	0
#define	WCOM	2
#define	RCOM	4
#define	GO	01
#define	DRESET	014
#define	IENABLE	0100
#define	DRY	0200
#define	ARDY	0100
#define	WLO	020000
#define	CTLRDY	0200

struct	device
{
	int	rkds;
	int	rker;
	int	rkcs;
	int	rkwc;
	caddr_t	rkba;
	int	rkda;
};

rkstrategy(io, func)
register struct iob *io;
{
	register struct device *rkaddr;
	register com;
	daddr_t bn;
	int dn, cn, sn;
	int errcnt;

	rkaddr = devsw[io->i_ino.i_dev].dv_csr;
	errcnt = 0;
retry:
	bn = io->i_bn;
	dn = io->i_unit;
	cn = bn/12;
	sn = bn%12;
	rkaddr->rkda = (dn<<13) | (cn<<4) | sn;
	rkaddr->rkba = io->i_ma;
	rkaddr->rkwc = -(io->i_cc>>1);
	com = (segflag<<4)|GO;
	if (func == READ)
		com |= RCOM; else
		com |= WCOM;
	rkaddr->rkcs = com;
	while ((rkaddr->rkcs&CTLRDY) == 0)
		;
	if (rkaddr->rkcs<0) {	/* error bit */
		if(errcnt == 0) {
			printf("\nRK unit %d disk error: cyl=%d sect=%d",
				dn, cn, sn);
			printf("\ncs=%o ds=%o er=%o",
			    rkaddr->rkcs, rkaddr->rkds, rkaddr->rker);
		}
		if(++errcnt >= 10) {
			printf("\n(FATAL ERROR)\n");
			return(-1);
		}
		rkaddr->rkcs = RESET|GO;
		while((rkaddr->rkcs&CTLRDY) == 0) ;
		goto retry;
	}
	if(errcnt)
		printf("\n(RECOVERED by retry)\n");
	return(io->i_cc);
}
