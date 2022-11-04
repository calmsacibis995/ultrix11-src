 
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985.	      *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/include/COPYRIGHT" for applicable restrictions.  *
 **********************************************************************/

/*
 * SCCSID: @(#)dds.c	3.0	4/21/86
 */

/*
 * Data structures used by the MSCP disk controller driver (ra.c)
 */

#include "dds.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/buf.h>
#include <sys/errlog.h>
#if	NUDA > 0
#include <sys/ra_info.h>

#define	RAUNITS	(C0_NRA+C1_NRA+C2_NRA+C3_NRA)
/*
 * Index into non-semetrical arrays of things like:
 * ra_ios[], rautab[], rrabuf[], ra_drv[].
 * takes less data space than [][] type arrays.
 * The index into ra_index is the controller number.
 */

char	ra_index[MAXUDA] = {0, C0_NRA, (C0_NRA+C1_NRA), (C0_NRA+C1_NRA+C2_NRA)};

struct	ios ra_ios[RAUNITS];	/* Disk I/O statistics information (iostat) */
char	*ra_dct[NUDA];		/* Controller name for error messages */
struct	ra_drv ra_drv[RAUNITS];	/* Drive types, status, and unit size */
struct	rasize *ra_sizes[NUDA];	/* Pointer to sizes table for each controller */

char	nra[] = {C0_NRA, C1_NRA, C2_NRA, C3_NRA};
struct	uda_softc uda_softc[NUDA];	/* Software status of each controller */
struct	uda uda[NUDA];		/* UQPORT communications area */
char	ra_rs[] = {C0_RS, C1_RS, C2_RS, C3_RS};		   /* cmd/rsp ring size */
char	ra_rsl2[] = {C0_RSL2, C1_RSL2, C2_RSL2, C3_RSL2};  /* same (log2) */
struct	mscp	ra_rp[C0_RS+C1_RS+C2_RS+C3_RS];		/* rsp packets */
struct	mscp	ra_cp[C0_RS+C1_RS+C2_RS+C3_RS];		/* cmd packets */
daddr_t	ra_mas[NUDA];		/* maint. area size for each controler */
struct	ra_ebuf ra_ebuf[NUDA];	/* error log buffer for each controller */
int	ra_elref[NUDA];		/* error log seguence number used along with */
				/* command reference number to link datagrams */
				/* with end messages. */

struct	buf ratab[NUDA];	/* Head of I/O queue for each controller */
struct	buf rawtab[1];		/* I/O wait queue for all controllers */
struct	buf rautab[RAUNITS];	/* Drive queue, one per unit */
struct	buf rrabuf[RAUNITS];	/* RAW I/O buffer header, one per unit */

#endif
/*
 * Must always be present, even if
 * no MSCP controllers are configured.
 *
 * Cntlr type ID is -1 for cntlr not present,
 * 0 is a valid ID (UDA50), see errlog.c.
 */
int	nuda = NUDA;
char	ra_ctid[] = {0377, 0377, 0377, 0377};	/* cntlr ID + micro-code rev */

/*
 * Data structures used by the massbus disk driver (hp.c)
 */
#if	NRH > 0
#include <sys/hp_info.h>
#include <sys/hpbad.h>
#include <sys/bads.h>

#define	HPUNITS	(C0_NHP+C1_NHP+C2_NHP+C3_NHP)
/*
 * Index into non-semetrical arrays of things like:
 * hp_ios[], hputab[], rhpbuf[], hp_dt[].
 * Takes less space than [][] type arrays.
 * The index into hp_index is the controller number.
 */

char	hp_index[MAXRH] = {0, C0_NHP, (C0_NHP+C1_NHP), (C0_NHP+C1_NHP+C2_NHP)};

struct	ios hp_ios[HPUNITS];	/* Disk I/O statistics information (iostat) */
char	hp_dt[HPUNITS];		/* Drive type ID of each configured unit */
char	nhp[] = {C0_NHP, C1_NHP, C2_NHP, C3_NHP}; /* # units per cntlr */
char	hp_openf[NRH];		/* HP open flag, one per controller */
struct	hp_ebuf hp_ebuf[NRH];	/* Error log buffer for each controller */

struct	buf hptab[NRH];		/* Head of I/O queue for each controller */
struct	buf hputab[HPUNITS];	/* Drive queue (overlap seek) oen per unit */
struct	buf rhpbuf[NRH];	/* RAW I/O buffer header for each controller */

struct	hpbad hp_bads[HPUNITS];	/* Buffer for bad block file, one per unit */
struct	dkres hp_res[NRH];	/* Bads revector and continue structure */
struct	hp_badr hp_r[HPUNITS];	/* Flags & base block, per unit (hp_info.h) */
#endif
