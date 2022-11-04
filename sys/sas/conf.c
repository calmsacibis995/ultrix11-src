/*
 * SCCSID: @(#)conf.c	3.0	4/21/86
 */
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#include <sys/param.h>
#include <sys/inode.h>
#include <sys/bads.h>
#include <sys/devmaj.h>
#include "saio.h"

devread(io)
register struct iob *io;
{

	return( (*devsw[io->i_ino.i_dev].dv_strategy)(io,READ) );
}

devwrite(io)
register struct iob *io;
{
	return( (*devsw[io->i_ino.i_dev].dv_strategy)(io, WRITE) );
}

devopen(io)
register struct iob *io;
{
	(*devsw[io->i_ino.i_dev].dv_open)(io);
}

devclose(io)
register struct iob *io;
{
	(*devsw[io->i_ino.i_dev].dv_close)(io);
}

nullsys()
{ ; }

int	nullsys();
int	rpstrategy();
int	rlstrategy();
int	rkstrategy();
int	tmstrategy(), tmrew(), tmopen();
int	htstrategy(), htopen(),htclose();
int	tkstrategy(), tkopen(), tkclose();
int	hpstrategy(), hpopen(), hpclose();
int	hkstrategy(), hkopen(), hkclose();
int	tsstrategy(), tsopen(), tsclose();
int	rastrategy(), raopen();
int	mdstrategy();
struct devsw devsw[] {
   "hk", hkstrategy, hkopen,  hkclose, 0177440, HK_BMAJ, HK_RMAJ, 0, 0,
   "hp", hpstrategy, hpopen,  hpclose, 0176700, HP_BMAJ, HP_RMAJ, 0, 0,
   "hm", hpstrategy, hpopen,  hpclose, 0176300, HM_BMAJ, HM_RMAJ, 1, 0,
   "hj", hpstrategy, hpopen,  hpclose, 0176400, HJ_BMAJ, HJ_RMAJ, 2, 0,
   "ra", rastrategy, raopen,  nullsys, 0172150, RA_BMAJ, RA_RMAJ, 0, DV_RA,
   "rc", rastrategy, raopen,  nullsys, 0172150, RA_BMAJ, RA_RMAJ, 1, DV_RC,
   "rd", rastrategy, raopen,  nullsys, 0172150, RA_BMAJ, RA_RMAJ, 2, DV_RD,
   "rx", rastrategy, raopen,  nullsys, 0172150, RA_BMAJ, RA_RMAJ, 2, DV_RX,
   "rk", rkstrategy, nullsys, nullsys, 0177400, RK_BMAJ, RK_RMAJ, 0, DV_NPDSK,
   "rl", rlstrategy, nullsys, nullsys, 0174400, RL_BMAJ, RL_RMAJ, 0, DV_RL,
   "rp", rpstrategy, nullsys, nullsys, 0176710, RP_BMAJ, RP_RMAJ, 0, 0,
   "ht", htstrategy, htopen,  htclose, 0172440, HT_BMAJ, HT_RMAJ, 0, DV_TAPE,
   "tk", tkstrategy, tkopen,  tkclose, 0174500, TK_BMAJ, TK_RMAJ, 0, DV_TAPE,
   "tm", tmstrategy, tmopen,  tmrew,   0172520, TM_BMAJ, TM_RMAJ, 0, DV_TAPE,
   "ts", tsstrategy, tsopen,  tsclose, 0172520, TS_BMAJ, TS_RMAJ, 0, DV_TAPE,
/* Boot: loads maxmem into dv_csr------------|				*/
   "md", mdstrategy, nullsys, nullsys,       0,       0,       0, 0, DV_MD,
	0
};
/*
 * dk_badf is a flag that tells who is using each
 * of the two bads structures. Zero if not used,
 * file descriptor (io) if used.
 * ASSUMPTION - only time more than one bads structure needed
 * is during disk to disk copy, when both disks are supported
 * by the bad block replacement software.
 */
#ifndef	RABADS
int	dk_badf[2] = {0, 0};
struct dkbad dk_bad[2];
#else
rpstrategy()
{ ; }
rlstrategy()
{ ; }
rkstrategy()
{ ; }
tmstrategy()
{ ; }
tmopen()
{ ; }
tmrew()
{ ; }
htstrategy()
{ ; }
htopen()
{ ; }
htclose()
{ ; }
tkstrategy()
{ ; }
tkopen()
{ ; }
tkclose()
{ ; }
hpstrategy()
{ ; }
hpopen()
{ ; }
hpclose()
{ ; }
hkstrategy()
{ ; }
hkopen()
{ ; }
hkclose()
{ ; }
tsstrategy()
{ ; }
tsopen()
{ ; }
tsclose()
{ ; }
mdstrategy()
{ ; }
#endif
