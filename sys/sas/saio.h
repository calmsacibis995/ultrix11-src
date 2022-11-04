/*
 * SCCSID: @(#)saio.h	3.1	3/26/87
 */
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * header file for standalone package
 */

/*
 * io block: includes an
 * inode, cells for the use of seek, etc,
 * and a buffer.
 */
struct	iob {
	char	i_flgs;
	struct inode i_ino;
	int i_unit;
	daddr_t	i_boff;
	daddr_t	i_cyloff;
	off_t	i_offset;
	daddr_t	i_bn;
	char	*i_ma;
	int	i_cc;
#ifndef	UCB_NKB
	char	i_buf[512];
#else
	char	i_buf[BSIZE];
#endif	UCB_NKB
};

#define F_READ	01
#define F_WRITE	02
#define F_ALLOC	04
#define F_FILE	010




/*
 * dev switch
 */
/*
 * devsw[] table flags
 */
#define	DV_NPDSK	1	/* non-partitioned disk (auto-unit select) */
#define	DV_TAPE		2	/* Device is a magtape (open() in SYS.c) */
#define	DV_RX		4	/* Drive must be RX33/RX50 */
#define	DV_RD		010	/* Drive must be RD31/32/51/52/53/54 */
#define	DV_RA		020	/* Drive must be RA60/RA80/RA81 */
#define	DV_RC		040	/* Drive must be RC25 */
#define	DV_MD		0100	/* Memory disk driver (CSR = maxmem) */
#define	DV_RL		0200	/* Identify load device as an RL02 */

struct devsw {
	char	*dv_name;
	int	(*dv_strategy)();
	int	(*dv_open)();
	int	(*dv_close)();
	int	dv_csr;
	int	dv_bmaj;
	int	dv_rmaj;
	char	dv_cn;
	char	dv_flags;
};

struct devsw devsw[];

/*
 * request codes. Must be the same a F_XXX above
 */
#define	READ	1
#define	WRITE	2

/*
 * NO_FIO is defined when building programs like rabads
 * that only access physical devices and don't need all
 * the code and buffers used to deal with the file system.
 */
#ifndef	NO_FIO

/*
 * The 1K block file system caused all stand-alone
 * program to grow significantly. Cutting NBUF back to
 * 1 saves 3K bytes in each program. See also, code changes
 * in sbmap() in SYS.c.
 *
 * THIS CHANGES LIMITS THE SIZE FILES WHICH CAN BE ACCESSED
 * BY STAND-ALONE PROGRAMS TO 260K BYTES (1 INDIRECT BLOCK).
 *
 * Fred Canter 6/20/85
 */
#ifdef	UCB_NKB
#define	NBUFS	1
#else
#define	NBUFS	4
#endif	UCB_NKB

#ifndef	UCB_NKB
char	sbmapb[NBUFS][512];
#else
char	sbmapb[NBUFS*BSIZE];
#endif	UCB_NKB
daddr_t	blknos[NBUFS];
#endif	NO_FIO


#define NFILES	4
struct	iob iob[NFILES];

/*
 * Set to which 32Kw segment the code is physically running in.
 * Must be set by the users main (or there abouts).
 */
int	segflag;
