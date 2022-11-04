 
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985.	      *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/include/COPYRIGHT" for applicable restrictions.  *
 **********************************************************************/

/*
 * SCCSID: @(#)nds.c	3.0	4/21/86
 */

#include "dds.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/buf.h>
#include <sys/errlog.h>

#if	NDE > 0

#include <net/if_de.h>
struct	de_softc	de_softc[NDE];

#endif

#if	NQE > 0

#include <net/if_qe.h>
struct	qe_softc	qe_softc[NQE];
int	nNQE = NQE;

#endif
