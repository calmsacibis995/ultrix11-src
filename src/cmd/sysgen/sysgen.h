
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)sysgen.h	3.2	8/6/87
 */
/*
 * Definintions and structures for sysgen
 *
 * Fred Canter
 */

#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/types.h>
#include <sys/time.h>
#include <a.out.h>

#define	YES	1
#define	NO	0
#define HELP	1
#define	NOHELP	0

/*
 * Processor type information.
 */
#define	SID	1	/* Split I & D space */
#define	NSID	0
#define	QBUS	1	/* Q bus processor */
#define	UBUS	2	/* unibus processor */

/*
 * Processor and device controller flags,
 * must be unique (could be used in several structures).
 */
#define	A18BIT	1	/* CPU address space */
#define	A22BIT	2
#define	LATENT	4	/* supported but not yet announced */
#define	MSCP	010	/* uses general MSCP disk driver (ra.c) */
#define	MASSBUS	020	/* uses general MASSBUS disk driver (hp.c) */
#define	FIXED	040	/* MASSBUS, but default CSR/VECTOR are fixed */
#define	Q22WARN	0100	/* WARN - if device used with Q22 bus NO RAW I/O */
#define	UBMAP	0200	/* Processor has a unibus map, 11/24 may or may not */
			/* but we assume it does for nbuf calculations */

#define	MAXNBUF	72	/* Maximum number of I/O buffers, if CPU has UB map */
			/* Also defined in /usr/sys/sas/boot.c */

/*
 * Number of contollers and user devices allowed.
 */
#define	MAXDC	11	/* MAX number of disk controllers allowed */
#define	MAXDOC	8	/* MAX number of drive types on disk controller */
#define	MAXUD	4	/* MAX numver of user writted device drivers */

struct	cputyp {
	char	*p_nam;	/* Processor type name string */
	int	p_typ;	/* Processor number */
	int	p_sid;	/* Split I & D space */
	int	p_bus;	/* Type of bus */
	int	p_flag;	/* CPU info, like maximum amount of memory */
};

/*
 * Map disk drive name to drive type,
 * its unix name, and its default CSR and vector.
 * The drive type (dt) is really the index into
 * the ddtype array, entry 0 is a dummy.
 * The drive types are defined so that drives may
 * be accessed expicitly by name, they must be in
 * the same order as the ddtype array.
 */
#define	RM02	1
#define	RM03	2
#define	RM05	3
#define	RP04	4
#define	RP05	5
#define	RP06	6
#define	RP02	7
#define	RP03	8
#define	RK06	9
#define	RK07	10
#define	RL01	11
#define	RL02	12
#define	RX02	13
#define	RK05	14
#define	RA60	15
#define	RA80	16
#define	RA81	17
#define	ML11	18
#define	RX33	19
#define	RX50	20
#define	RD31	21
#define	RD32	22
#define	RD51	23
#define	RD52	24
#define	RD53	25
#define	RD54	26
#define	RC25	27

/*
 * For MASSBUS disks, name is changed according to
 * controller number. CSR/VECTOR are also changed
 * unless FIXED flag is set.
 */
struct	ddtype {
	char	*ddn;	/* disk drive name */
	char	*ddun;	/* disk drive unix name */
	int	ddcsr;	/* default CSR address */
	int	ddvec;	/* default vector address */
	int	ddsdl;	/* index into standard disk layout array */
			/* -1 says this can't be the system disk */
	int	ddnp;	/* number of disk partitions */
	int	ddflag;	/* flags for special cases */
};

/*
 * Standard file system layouts for system disk
 * rootdev, pipedev, swapdev, and eldev only used
 * if user specifies file system layout.
 */

struct	sdfsl {
	char	*rootdev;	/* root device name "hp", "hk", etc. */
	int	rootmd;		/* root minor device */
	char	*pipedev;	/* pipe device name */
	int	pipemd;		/* pipe minor device */
	char	*swapdev;	/* swap device name */
	char	*eldev;		/* error log device name */
	int	swapmd;		/* swap minor device */
	daddr_t	swaplo;		/* swap area start block */
	int	nswap;		/* length of swap area in blocks */
	int	elmd;		/* error log minor device */
	daddr_t	elsb;		/* error log start block */
	int	elnb		/* error log length in blocks */
};

/*
 * Disk controller types
 */

#define	RH11	1
#define	RH70	2
#define	RP11	3
#define	RK611	4
#define	RK711	5
#define	RL11	6
#define	RX211	7
#define	RK11	8
#define	UDA50	9
#define	RQDX1	10
#define	KLESI	11
#define	RUX1	12

/*
 * Map disk controller name to controller type
 */

struct	dctype {
	char	*dcn;	/* disk controller name */
	int	dct;	/* disk controller type */
	int	dcflag;	/* MSCP cntlr or not, unibus/Qbus */
};

/*
 * Drive types allowed on each controller
 */

#define	MAXDOC	8	/* maximum # of drive types on a controller */

struct	doc {
	int	dtct;		/* controller type */
	int	dtdn[MAXDOC];	/* array of allowed drive types */
};

/*
 * Descriptor table for each
 * configured disk controller.
 */
struct	dcd {
	int	dctyp;		/* controller type */
	int	dccn;		/* MSCP cntlr number */
	char	*dcname;	/* unix mnemonic ,i.e., hp, hk, etc. */
	int	dcaddr;		/* CSR address */
	int	dcvect;		/* interrupt vector address */
	int	dcnd;		/* number of drives on controller */
	int	dcsys;		/* If system disk index into (sdfsl) file */
				/* system layout, otherwise set to -1 */
	char	dcunit[8];	/* drive type of each unit, 0 = no drive */
};

/*
 * Magtape controller types
 */

struct	mttype {
	char	*mtct;		/* magtape controller name */
	char	*mtun;		/* magtape unix mnemonic */
	int	mtnunit;	/* number of magtape units on controller */
				/* unit(controller) number for ts magtape */
	int	mtcd;		/* magtape is crashdump device */
				/* -1 means can't be crash dump device */
	int	mtcsr;		/* CSR address */
	int	mtvec;		/* vector address */
};

/*
 * Crash dump device table
 *
 * COMMENTS:
 *
 *	Magtapes only dump to unit 0.
 *	Disks (except RX50) dump to system disk unit number.
 *	RX50 dumps to lowest numbered floppy unit (auto sizes it).
 *
 *	Can only dump to magtape/rx50 if non-standard placements used.
 *	Cannot dump to RX50 on RUX1 controller.
 *	Boot: (auto-unit select) changes dumpdn to system disk unit number.
 *	Boot: (auto-CSR select) changes dump device CSR (disks only).
 *
 *	System disk must be on first RH or MSCP controller!
 */

#define	CD_SG	01	/* device configured into kernel */
#define	CD_TAPE	02	/* device is a magtape */
#define	CD_DISK	04	/* device is a disk */
#define	CD_RX50	010	/* device is RX50 diskette */

struct	cdtab {
	char	*cd_name;	/* device mnemonic */
	char	*cd_gtyp;	/* generic controller name */
	int	cd_dtyp;	/* disk drive type */
	int	cd_unit;	/* unit # (set on the fly unless RX50 or TAPE) */
	daddr_t	cd_dmplo;	/* disk - start block number */
	daddr_t	cd_dmphi;	/* disk - dump max block number */
	int	cd_flags;	/* flags (config, tape, disk, etc.) */
};

/*
 * Line printer
 */

struct	lptype {
	char	*lpn;		/* line printer unix name */
	int	lpcsr;		/* CSR address */
	int	lpvec;		/* vector address */
	int	lpused;		/* printer configured flag */
};

/*
 * C/A/T phototypesetter interface
 */

struct	cttype {
	char	*ctn;		/* CAT unix name */
	int	ctcsr;		/* CSR address */
	int	ctvec;		/* vector address */
	int	ctused;		/* printer configured flag */
};

/*
 * User devices
 */
struct	udtype {
	char	*udname;	/* user device name */
	int	udcsr;		/* user device CSR address */
	int	udvec;		/* user device vector */
	int	udused;		/* user device used flag */
};

/*
 * Communications multiplexers
 */

struct	cmtype {
	char	*cmn;		/* comm mux unix name */
	char	*cmnx;		/* name for "cf" file  added by bpb */
	char	*cmdn;		/* real device name DH11, DZ11, etc */
	int	cmumax;		/* MAX number of units */
	int	cmnunit;	/* comm mux number of units */
	int	cmcsr;		/* CSR address */
	int	cmvec;		/* vector address */
};

/*
 * Network devices
 */
struct	nettype {
	char	*ntn;		/* user device name */
	char	*ntnx;		/* unix name for device */
	int	*ntcsr;		/* CSR address */
	int	*ntvec;		/* vector */
	int	ntumax;		/* MAX number of units */
	int	ntnunit;	/* number of units */
	int	nvec;		/* Number of interrupt vectors (1 or 2) */
};

/*
 * System parameters
 */

struct	syspar {
	char		*spname;	/* parameter name */
	char		*sphelp;	/* name of help message file */
	unsigned	spv_ov;		/* overlay kernel default value */
	unsigned	spv_id;		/* separate I & D default value */
	unsigned	spval;		/* actual value, may be user specified */
	int	spask;			/* -1, don't ask for value with others */
					/*     value obtained separately */
};

/*
 * Structure for finding and printing help text,
 * implemented this way due to conversion from
 * help text files (sg_???.help).
 */
struct	sghelp {
	char	*sgh_name;	/* name of the help message */
	char	**sgh_addr;	/* address of help text */
};

/* Structure for Daylight Savings Time information */
struct	dst_table {
	char	*dst_area;	/* Geographical area */
	int	dst_id;		/* Numeric id */
};

/*
 * Global variables
 */

extern	jmp_buf	savej;
extern	int	buflag;		/* used for <CTRL/D> handling */
extern	int	cpu;		/* CPU type */
extern	int	fpp;		/* Set of CPU has floating point hardware */
extern	int	hz;		/* AC line frequency */
extern	int	tz;		/* timezone (hours west of GMT) */
extern	int 	dst;		/* daylight savings time flag */
extern	int	mscp_cn;	/* MSCP disk controller number */
extern	int	cputype;	/* Current processor type */
extern	unsigned realmem;	/* Current CPU's memory size in 64 byte clicks */
extern	int	rn_ssr3;	/* M/M SSR3 (identify 23+ vs 23 CPU) */
extern	int	gkopt;		/* -g, allow generation of a generic kernel */

extern	char	line[];
extern	char	config[];
extern	char	cpf[];
extern	char	cfline[];
extern	char	*sg_help[];
extern	char	*devlist[];
extern	char	*install[];
extern	char	*wm_q22[];
extern	char	*wm_ml11[];
extern	char	*sg_mtu[];
extern  char	*sg_dstarea[];

extern	struct	cputyp	cputyp[];
extern	struct	ddtype	ddtype[];
extern	struct	sdfsl	sdfsl[];
extern	struct	dctype	dctype[];
extern	struct	doc	doc[];
extern	struct	dcd	dcd[];
extern	struct	mttype	mttype[];
extern	struct	mttype	mtts[];
extern	struct	mttype	mttk[];
extern	struct	lptype	lptype[];
extern	struct	cttype	cttype[];
extern	struct	udtype	udtype[];
extern	struct	cmtype	cmtype[];
extern	struct	syspar	syspar[];
extern	struct	syspar	msgpar[];
extern	struct	syspar	sempar[];
extern	struct	syspar	flckpar[];
extern	struct	sghelp	sghelp[];
extern	struct	cdtab	cdtab[];
extern	struct	nettype	nettype[];
extern	struct	syspar	netpar[];
extern 	struct	syspar	mauspar[];
extern 	struct	dst_table dst_table[];
