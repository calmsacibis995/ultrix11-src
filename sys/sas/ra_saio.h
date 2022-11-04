/* SCCSID: @(#)ra_saio.h	3.1	3/26/87	*/

/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/
/*
 *
 * Common definitions for all standalone programs
 * that must deal with MSCP type disks.
 *
 */

/*
 * UQSSP IDs
 */
#define	UDA50	0
#define	KLESI	1
#define	RUX1	2
#define	UDA50A	6
#define	RQDX1	7
#define	KDA50	13
#define	RQDX3	14	/* FAKE RQDX3 ID */
#define	R_RQDX3	19	/* REAL RQDX3 ID */

/*
 * Drive type IDs
 */
#define	RC25	25
#define	RD31	31
#define	RD32	32
#define	RX33	33
#define	RX50	50
#define	RD51	51
#define	RD52	52
#define	RD53	53
#define	RD54	54
#define	RA60	60
#define	RA80	80
#define	RA81	81

/*
 * CAUTION: also defined in /usr/include/sys/ra_info.h
 */
#define	RA_MAS	1000	/* RA60/RA80/RA81 maintenance area size */
#define	RC_MAS	102	/* RC25 maintenance area size */
#define	RD_MAS	32	/* RD31/RD32/RD51/RD52/RD53/RD54 maintenance area size */

/*
 * CAUTION: not the same as ra_drv in the kernel's ra.c
 */
struct	ra_drv {		/* RA drive information */
	char	ra_dt;		/* Drive type: 25,31,33,50,51,52,53,54,60,80,81 */
	char	ra_online;	/* Drive on-line flag */
	char	ra_rbns;	/* Number of replacement blocks per track */
	char	ra_ncopy;	/* Number of RCT copies */
	int	ra_rctsz;	/* Number of blocks in each RCT copy */
	int	ra_trksz;	/* Number of LBNs per track */
	union {
		daddr_t	ra_dsize;	/* Unit size in LBNs */
		struct {
			int	ra_dslo;
			int	ra_dshi;
		} d_str;
	} d_un;
};

/*
 * Address where the RA driver leaves the RQDX
 * controller type ID for use by mkfs.
 */

#define	RQ_CTID	((physadr)0140006)
