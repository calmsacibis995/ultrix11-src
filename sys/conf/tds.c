 
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985.	      *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/include/COPYRIGHT" for applicable restrictions.  *
 **********************************************************************/

/*
 * SCCSID: @(#)tds.c	3.0	4/21/86
 */

#include "dds.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/buf.h>
#include <sys/errlog.h>

/*
 * Data structures used by ts magtape controller driver (ts.c)
 */

#if	NTS > 0
#include <sys/ts_info.h>
int	cmdpkt[5*NTS];			/* command packet */
struct	chrdat	chrbuf[NTS];		/* characteristics buffer */
struct	mespkt	mesbuf[NTS];		/* message buffer */
struct	buf	tstab[NTS];
struct	buf	ctsbuf[NTS];
struct	buf	rtsbuf[NTS];

struct compkt *ts_cbp[NTS];	/* command packet buffer pointer */
				/* set in tsopen() */
char	*ts_ubcba[NTS];	/* unibus virtual address of command packet buffer */
			/* set in tsopen() to ts_cbp - ts_ubmo */

char	ts_openf[NTS];
u_short	ts_flags[NTS];
daddr_t ts_blkno[NTS];
daddr_t ts_nxrec[NTS];
u_short	ts_erreg[NTS];
u_short	ts_dsreg[NTS];
short	ts_resid[NTS];

struct tsebuf ts_ebuf[NTS];
#endif

int	nts = NTS;

/*
 * Data structures used by tmscp magtape controller driver (tk.c)
 */

#if	NTK > 0
#include <sys/tk_info.h>
struct tk_info tk_info[NTK];
char	tk_on[NTK];
char	tk_rew[NTK];
char	tk_wait[NTK];
char	tk_eot[NTK];
char	tk_nrwt[NTK];
char	tk_clex[NTK];
char	tk_fmt[NTK];
char	tk_cse[NTK];
char	tk_cache[NTK];
char	*tk_dct[NTK];

struct	tk_drv	tk_drv[NTK];

char	tk_ctid[MAXTK];

struct tk_softc tk_softc[NTK];

struct	tk	tk[NTK];

struct	tk_ebuf	tk_ebuf[NTK];
int	tk_elref[NTK];

struct	buf tktab[NTK];
struct	buf tkwtab;
struct	buf rtkbuf[NTK];
struct	buf ctkbuf[NTK];
#endif

int	ntk = NTK;
