/*
 * SCCSID: @(#)rp.c	3.0	4/21/86
 */
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * RP02/3 standalone disk driver
 * Fred Canter
 */

#include <sys/param.h>
#include <sys/inode.h>
#include "saio.h"

struct device {
	int	rpds;
	int	rper;
	union {
		int	w;
		char	c;
	} rpcs;
	int	rpwc;
	char	*rpba;
	int	rpca;
	int	rpda;
};


#define	GO	01
#define	DONE	0200
#define	RESET	0
#define	HSEEK	014

#define	IENABLE	0100
#define	READY	0200
#define	RCOM	4
#define	WCOM	2

#define	SUFU	01000
#define	SUSU	02000
#define	SUSI	04000
#define	HNF	010000



rpstrategy(io, func)
register struct iob *io;
{
	register struct device *rpaddr;
	int com,cn,tn,sn;
	int errcnt, ctr;

	rpaddr = devsw[io->i_ino.i_dev].dv_csr;
	errcnt = 0;
retry:
	cn = io->i_bn/(20*10);
	sn = io->i_bn%(20*10);
	tn = sn/10;
	sn = sn%10;
	rpaddr->rpcs.w = (io->i_unit<<8);
	rpaddr->rpda = (tn<<8) | sn;
	rpaddr->rpca = cn;
	rpaddr->rpba = io->i_ma;
	rpaddr->rpwc = -(io->i_cc>>1);
	com = (segflag<<4)|GO;
	if (func == READ)
		com |= RCOM; else
		com |= WCOM;
	
	rpaddr->rpcs.w |= com;
	while ((rpaddr->rpcs.w&DONE)==0)
		;
	if (rpaddr->rpcs.w < 0) {	/* error bit */
		if(errcnt == 0) {
		    printf("\nRP unit %d disk error: cyl=%d track=%d sect=%d",
			io->i_unit, cn, tn, sn);
		    printf("\ncs=%o ds=%o er=%o",
			rpaddr->rpcs.w, rpaddr->rpds, rpaddr->rper);
		}
		if(++errcnt >= 10) {
			printf("\n(FATAL ERROR)\n");
			return(-1);
		}
		if(rpaddr->rpds & (SUFU|SUSI|HNF)) {
			rpaddr->rpcs.c = HSEEK|GO;
			ctr = 0;
			while ((rpaddr->rpds&SUSU) && --ctr)
				;
		}
		rpaddr->rpcs.w = RESET|GO;
		ctr = 0;
		while ((rpaddr->rpcs.w&READY) == 0 && --ctr)
			;
		goto retry;
	}
	if(errcnt)
		printf("\n(RECOVERED by retry)\n");
	return(io->i_cc);
}
