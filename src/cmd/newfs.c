
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static	char	Sccsid[] = "@(#)newfs.c	3.1	3/26/87";

/* #define CMDLINE	/* for non-interactive usage - obsolete! */
/* #define DEBUG	/* for partition table debugging printout */
/*
 *
 *  File name:
 *
 *	newfs.c
 *
 *  Source file description:
 *
 *	Friendly front end to /etc/mkfs (newfs).
 *
 *	This program allows a user to make a file system by specifying the
 *	generic disk name and partition number. Newfs determines the size
 *	of the file system by reading the disk sizes table from the kernel.
 *	Newfs forks a copy of mkfs to actually make the file system.
 *
 *	This programs helps eliminate some of the confusion associated with
 *	making file systems, i.e., is the size in 512 byte or 1K byte blocks?
 *	Also, user does not need to look up the partition size. Assures
 *	correct file system size even if non standard sizes used (dksizes.c).
 *
 *	CAUTION!, this program depends on /usr/include/sys/devmaj.h. If the
 *	order of major device numbers changes, newfs must be recompiled.
 *
 *  Functions:
 *
 *	main()		The planes in spain fall MAINLY in the rain!
 *
 *	OTHERS?
 *
 *  Usage:
 *
 *	Interactive usage:
 *
 *		/etc/newfs	(program prompts for disk name, etc.)
 *
 *	Non-interactive usage: (obsolete)
 *
 *		/etc/newfs [-t] disk [-c #] unit [dp] [fsname volname]
 *
 *		-t	print list of configured disk controllers/drives
 *		disk	generic disk name (rp06, rm03, rd52, rx50, etc.)
 *		-c #	# = RH11/RH70 MASSBUS controller number (default = 0)
 *		unit	physical disk unit number
 *		dp	disk partition number
 *		fsname	superblock file system name, fsname[6]
 *		volname	superblock volume label, volname[6]
 *
 *  Compile:
 *
 *	cd /usr/src/cmd; make newfs
 *
 *  Modification History:
 *
 *	31 August 1985
 *		File created -- Fred Canter
 *
 */

#include <stdio.h>
#include <a.out.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/param.h>
#include <sys/devmaj.h>
#include <sys/conf.h>
#include <sys/ra_info.h>
#include <sys/hp_info.h>
#include <sys/types.h>
#include <sys/stat.h>

/*
 * Help messages
 */

char	*help[] =
{
	"The /etc/newfs (new file system) program allows you to make a new",
	"file system on a disk without knowing its size. A disk can be a",
	"physical disk unit or a logical sub-unit (partition), depending",
	"on the type of disk. Refer to Chapter 1 and Appendix D in the",
	"ULTRIX-11 System Management Guide for more information about disk",
	"partitioning.",
	"",
	"To execute a command, type the first letter of the command, then",
	"press <RETURN>. The command will execute, then return to the command",
	"prompt. The newfs command prompts you for the disk name and other",
	"information. If you need help answering the prompts, type a '?'.",
	"",
	"The newfs commands are:",
	"",
	"CTRL/C	Abort current command and return to the command prompt.",
	"help	Print this help message (? also prints help message).",
	"exit	Exit from the newfs program.",
	"table	Prints list of configured controllers and drives.",
	"newfs	Make a new file system on a disk (or partition).",
	"",
	0
};

char	*h_dsknam[] =
{
	"Enter the disk's generic name, using only lowercase characters.",
	"The generic name is the real name for the disk as opposed to its",
	"ULTRIX-11 mnemonic, for example, rp06 vs hp. Use the table command",
	"at the newfs prompt to list the names of all configured disks.",
	"",
	0
};

char	*h_cn[] =
{
	"If the disk is a MASSBUS disk, enter the number of the RH11/RH70",
	"disk controller it is connected to. You can use the table command",
	"at the newfs prompt to list the MASSBUS controller numbers.",
	"",
	"The MASSBUS disks are:",
	"	RH11/RH70 - RP04/5/6, RM02/3/5, and ML11",
	"",
	"The controller number is determined by the order in which the",
	"controllers were specified during the system generation, that is,",
	"controller zero was entered first, controller one second and",
	"controller two third, etc.",
	"",
	0
};

char	*h_unit[] =
{
	"Enter the unit number of the disk on which the new file system is",
	"to be created. You can use the table command at the newfs prompt",
	"to list all available disk units.",
	"",
	0
};

char	*h_density[] =
{
	"Enter the density of the RX02 floppy disk. Normally, this is",
	"double density. Single density is used for compatibility with",
	"RX01 floppys. The density you specify must match the density you",
	"specified when you initially formatted the disk. If the disk has",
	"not yet been formatted, you must go back and format the disk using",
	"the /etc/rx2fmt command before you can create a filesystem on it.",
	"",
	0
};

char	*h_fsn[] =
{
	"The file system name is used to fill in the fsname[6] field in the",
	"superblock (see filsys(5)). The fsname has a maximum length of six",
	"characters. The fsname is usually the directory on which the file",
	"system will be mounted, for example, user1, or /tmp.",
	"",
	"The fsname is not required. If you respond with only a <RETURN>,",
	"the fsname field will remain uninitialized. The labelit(8) command",
	"can also be used to modify the superblock fsname[6] field.",
	"",
	0
};
char	*h_voln[] =
{
	"The volume label is used to fill in the volname[6] field in the",
	"superblock (see filsys(5)). The volname has a maximum length of",
	"six characters. The volname should match the disk pack label. For",
	"example: ud_hp0, pack1, system, etc.",
	"",
	"The volname is not required. If you respond with only a <RETURN>,",
	"the volname field will remain uninitialized. The labelit(8) command",
	"can also be used to modify the superblock volname[6] field.",
	"",
	0
};

char	*h_dpart[] =
{
	"Enter the number of the disk partition where the file system is to",
	"be created. There are eight possible partitions, numbered 0 thru 7.",
	"Not all disks use all of the eight possible disk partitions. Refer",
	"to Appendix D in the ULTRIX-11 System Management Guide for a disk",
	"partition layout of all supported disks.",
	"",
	0
};

#ifdef CMDLINE	/* non-interactive mode - obsolete! */
char	*oldusage[] =
{
	"",
	"Usage:\t/etc/newfs [-t] disk [-c #] unit [dp] [fsname volname]",
	"Interactive usage:\t/etc/newfs\t(prompts for disk name, etc.)",
	"",
	"	-t	 print list of configured disk controllers/drives",
	"	disk	 generic disk name (rp06, rm03, rd52, rx50, etc.)",
	"	-c #	 # = RH11/RH70 disk controller number (default = 0)",
	"	unit	 physical disk unit number",
	"	dp	 disk partition number (0 -> 7)",
	"	fsname	 superblock file system name, fsname[6]",
	"	volname	 superblock volume label, volname[6]",
	"",
	0
};
#endif CMDLINE

char	*usage[] =
{
	"usage: /etc/newfs",
	"",
};

char	*bcnerr = "Invalid MASSBUS cntlr number with -c option!\n";
char	*bunerr = "Invalid unit number!\n";

/*
 * Namelist values read from the kernel.
 */

struct	nlist	nl[] =
{
	{ "_cputype" },
#define			X_CPUTYPE	0
	{ "_nrk" },
#define			X_NRK		1
	{ "_rk_dt" },
#define			X_RK_DT		2
	{ "_nrp" },
#define			X_NRP		3
	{ "_rp_dt" },
#define			X_RP_DT		4
	{ "_nuda" },
#define			X_NUDA		5
	{ "_nra" },
#define			X_NRA		6
	{ "_ra_inde" },
#define			X_RA_INDEX	7
	{ "_ra_ctid" },
#define			X_RA_CTID	8
	{ "_ra_drv" },
#define			X_RA_DRV	9
	{ "_nrl" },
#define			X_NRL		10
	{ "_rl_dt" },
#define			X_RL_DT		11
	{ "_nhp" },
#define			X_NHP		12
	{ "_hp_inde" },
#define			X_HP_INDEX	13
	{ "_hp_dt" },
#define			X_HP_DT		14
	{ "_nhk" },
#define			X_NHK		15
	{ "_hk_dt" },
#define			X_HK_DT		16
	{ "_ra_mas" },
#define			X_RA_MAS	17
	{ "_ud_size" },
#define			X_UD_SIZES	18
	{ "_rq_size" },
#define			X_RQ_SIZES	19
	{ "_rc_size" },
#define			X_RC_SIZES	20
	{ "_hp_size" },
#define			X_HP_SIZES	21
	{ "_hk_size" },
#define			X_HK_SIZES	22
	{ "_rp_size" },
#define			X_RP_SIZES	23
	{ "_bdevsw" },
#define			X_BDEVSW	24
	{ "_nhx" },
#define			X_NHX		25
	{ "_rootdev" },
#define			X_ROOTDEV	26
	{ "" },
};

/*
 * Disk partition sizes table.
 */
struct	pt_info {
	daddr_t	pt_sb;		/* starting block number (1KB logical block) */
	daddr_t	pt_nb;		/* length in 1KB logical blocks */
} pt_info[8];

/*
 * Disk controller and drive info table.
 * Loaded with controller type and type of each
 * configured disk. Used for making the fstab,
 * special files, and file systems.
 */

#define	DI_MSCP	01	/* MSCP controller flag */
#define	DI_MASS	02	/* MASSBUS controller flag */
#define	DI_NPD	04	/* Non-partitioned disk flag */
#define	DI_HX	010	/* RX02 special case flag */
#define	DI_RX50	020	/* RX50/RX30 special case flag */
#define	DI_FIRST 040	/* defines first MSCP controller so RX50/KLESI offline
	message once, instead of 4 messages for each unit */

int	rk_gdti(), rp_gdti(), ra_gdti(), rl_gdti();
int	hx_gdti(), hp_gdti(), hk_gdti();

struct	di_info {
	char	*di_typ;	/* Controller type (name string) */
	char	*di_name;	/* Two char ULTRIX-11 mnemonic */
				/* ("ra" changed on the fly to ra|rc|rd|rx) */
	char	di_cn;		/* Controller number (if massbus or mscp) */
	char	di_nunit;	/* Number units (0 if cntlr not configured) */
	char	di_bmaj;	/* Block major device number */
	char	di_rmaj;	/* Raw major device number */
	int	(*di_func)();	/* Call this function to get drive type info */
	int	di_flags;	/* Flags (massbus, mscp, NP disk, etc.) */
	int	di_dt[8];	/* Each unit's type (index into dt_info[]) */
				/* Zero indicates non existent drive */
} di_info[] = {
  { "RK11",	"rk", 0, 0, RK_BMAJ, RK_RMAJ, rk_gdti, DI_NPD },
  { "RP11",	"rp", 0, 0, RP_BMAJ, RP_RMAJ, rp_gdti, 0 },
  { "MSCP",	"ra", 0, 0, RA_BMAJ, RA_RMAJ, ra_gdti, (DI_MSCP|DI_FIRST) },
  { "MSCP",	"ra", 1, 0, RA_BMAJ, RA_RMAJ, ra_gdti, DI_MSCP },
  { "MSCP",	"ra", 2, 0, RA_BMAJ, RA_RMAJ, ra_gdti, DI_MSCP },
  { "MSCP",	"ra", 3, 0, RA_BMAJ, RA_RMAJ, ra_gdti, DI_MSCP },
  { "RL11",	"rl", 0, 0, RL_BMAJ, RL_RMAJ, rl_gdti, 0 },
  { "RX11",	"hx", 0, 0, HX_BMAJ, HX_RMAJ, hx_gdti, DI_HX },
  { "MASSBUS",	"hp", 0, 0, HP_BMAJ, HP_RMAJ, hp_gdti, DI_MASS },
  { "MASSBUS",	"hm", 1, 0, HM_BMAJ, HM_RMAJ, hp_gdti, DI_MASS },
  { "MASSBUS",	"hj", 2, 0, HJ_BMAJ, HJ_RMAJ, hp_gdti, DI_MASS },
  { "RK611",	"hk", 0, 0, HK_BMAJ, HK_RMAJ, hk_gdti, 0 },
  { 0 },
};

/*
 * System's disk type info structure.
 *
 * Disk drive type ID definitions (must be unique):
 */
/* ra_info: RX33 RX50 RD31 RD32 RD51 RD52 RD53 RD54 RC25 RA60 RA80 RA81 */
/* hp_info: RP04 RP05 RP06 RM02 RM03 RM05 ML11 */
#define	RK05	5
#define	RX02	1
#define	RL01	10240
#define	RL02	20480
#define	RK06	0
#define	RK07	02000
#define	RP02	2
#define	RP03	3

struct dt_info {
	char	*dt_lname;	/* disk type name in lowercase */
	char	*dt_uname;	/* disk type name in uppercase */
	int	dt_type;	/* numeric disk type */
	char	dt_pmask;	/* BIT WISE -- disk partition usage mask */
	char	dt_smask;	/* BIT WISE -- system disk unwritable */
				/* partitions. If zero, can't be system disk */
	int	dt_flags;	/* flags is flags */
} dt_info[] = {
	/* DUMMY ENTRY: must be here, see di_info.di_dt[] */
	"",	"",	-1,	0,	0,	0,
	"rx02",	"RX02",	RX02,	0,	0,	DI_NPD,
	"rk05",	"RK05",	RK05,	0,	0,	DI_NPD,
	"ml11",	"ML11",	ML11,	0,	0,	DI_MASS,
	"rx33",	"RX33",	RX33,	0200,	0,	DI_RX50, /* using DI_RX50 for RX33 */
	"rx50",	"RX50",	RX50,	0200,	0,	DI_RX50,
	"rd31",	"RD31",	RD31,	0341,	0341,	0,
	"rd32",	"RD32",	RD32,	0217,	0207,	0,
	"rd51",	"RD51",	RD51,	0221,	0221,	0,
	"rd52",	"RD52",	RD52,	0217,	0207,	0,
	"rd53",	"RD53",	RD53,	0217,	0207,	0,
	"rd54",	"RD54",	RD54,	0217,	0207,	0,
	"rl01",	"RL01",	RL01,	0201,	0201,	0,
	"rl02",	"RL02",	RL02,	0203,	0203,	0,
	"rk06",	"RK06",	RK06,	0107,	0107,	0,
	"rk07",	"RK07",	RK07,	0213,	0213,	0,
	"rp02",	"RP02",	RP02,	0107,	0107,	0,
	"rp03",	"RP03",	RP03,	0213,	0213,	0,
	"rp04",	"RP04",	RP04,	0117,	0107,	DI_MASS,
	"rp05",	"RP05",	RP05,	0117,	0107,	DI_MASS,
	"rp06",	"RP06",	RP06,	0377,	0307,	DI_MASS,
	"rm02",	"RM02",	RM02,	0217,	0207,	DI_MASS,
	"rm03",	"RM03",	RM03,	0217,	0207,	DI_MASS,
	"rm05",	"RM05",	RM05,	0377,	0207,	DI_MASS,
	"ra60",	"RA60",	RA60,	0277,	0207,	0,
	"ra80",	"RA80",	RA80,	0217,	0207,	0,
	"ra81",	"RA81",	RA81,	0377,	0207,	0,
	"rc25",	"RC25",	RC25,	0237,	0207,	0,
	0
};

/*
 * Number of 512 byte blocks per cylinder
 * for each type of disk.
 */

#define	RP23_BPC	200	/* RP02/3 */
#define	RP456_BPC	418	/* RP04/5/6 */
#define	RM23_BPC	160	/* RM02/3 */
#define	RM5_BPC		608	/* RM05 */
#define	RK67_BPC	66	/* RK06/7 */

/*
 * Misc. stuff for setup of user file systems.
 */

int	cputype;	/* current processor type */
int	rootdev;	/* block major/minor of root device */
char	*dsk_sfn =	"/dev/newfs.dsf";
char	*lpfdir =	"lost+found";
char	fsname[20];	/* superblock fsname (also mounted on directory name) */
char	volname[20];	/* superblock volname */
char	rdisk[20];	/* raw special file name (/dev/r??##) */
char	syscmd[100];	/* system command buffer */
char	rbuf[512];	/* general read buffer */

/*
 * Kernel data structures needed to determine
 * disk controller and drive configuration.
 */

struct	dksize {
	daddr_t	nblocks;	/* partition length - 512 byte blocks (sectors) */
	int	cyloff;		/* partition starting cylinder number */
};

struct	bdevsw	bdevswb;

/*
 * RK05
 *	NED = -1
 *	RK05 = 1
 */
int	nrk;
char	rk_dt[8];
int	rk_size = 4872;

/*
 * RP02/3
 *	NED = -1
 *	RP02 = 0
 *	RP03 = 1
 * Note: must read each configured RP02/3 unit to
 *	 force update of rp_dt[].
 */
int	nrp;
char	rp_dt[8];
struct	dksize	rp_sizes[8];

/*
 * MSCP (UDA50 RC25 RQDX RUX50)
 *	NED = 0
 *	See /usr/include/sys/ra_info.h for drive types.
 *
 * Note: MSCP drive type info arrays are non-symetrical
 *	 ra_index must be used to access drive type info.
 */

int	nuda;		/* Number of configured MSCP controllers */
char	nra[MAXUDA];	/* Number of units configured on each controller */
char	ra_ctid[MAXUDA];/* Controller type ID and micro-code rev level */
char	ra_index[MAXUDA];/* Index into drive type info ra_drv[] */
struct	ra_drv ra_drv[MAXUDA*8];	/* RA drive info (see ra_info.h) */

daddr_t	ra_mas[MAXUDA];		/* Size of maintenance area */
struct	rasize	ud_sizes[8];	/* UDA50 partition sizes */
struct	rasize	rc_sizes[8];	/* RC25 partition sizes */
struct	rasize	rq_sizes[8];	/* RQDX partition sizes */

/*
 * RL01/2
 *	NED = -1
 *	RL01 = 10240
 *	RL02 = 20480
 */
int	nrl;
int	rl_dt[4];
/* SIZE: drive type = size (RL01=10240, RL02=20480) */

/*
 * RX02 
 */
int	nhx;

/*
 * MASSBUS (HP HM HJ)
 *	NED = 0
 *	See /usr/include/sys/hp_info.h for drive types.
 *
 * Note: MASSBUS drive type info arrays are non-semetrical
 *	 hp_index must be used to access drive type info.
 */

char	hp_index[MAXRH];	/* Index into drive type info arrays */
char	nhp[MAXRH];		/* Number units per cntlr (0 = not config) */
char	hp_dt[MAXRH*8];		/* Drive type of each drive */
struct	dksize	hp_sizes[32];	/* disk partition sizes */

/*
 * RK06/7
 *	NED = -1
 *	RK06 = 0
 *	RK07 = 02000
 */
int	nhk;
int	hk_dt[8];
struct	dksize	hk_sizes[8];

#define	MKFS	1
#define	BLOCK	0
#define	RAW	1
#define	HELP	2
#define	NOHELP	0
#define	YES	1
#define	NO	0
#define	LBSIZE	50
char	lbuf[LBSIZE];

char	*mntdir="/Newfs_mntdir1";	/* dir to mount for lost+found work */

int	imode;		/* Interactive mode */
int	cntlr;		/* MASSBUS controller number */
int	unit;		/* Disk unit number */
int	dpart;		/* Disk partition number */
int	hx_single;	/* == 1 single density RX02 (RX01 mode) */

jmp_buf	savej;

int	sumask;

main(argc, argv)
int	argc;
char	*argv[];
{
	int	intr();
	register struct dt_info *dtp;
	register struct di_info *dip;
	register int i;
	int	mem;
	long	soff;
#ifdef CMDLINE
	int part, gotunit, gotdisk, gotfsname;	/* for non-interactive use */
#endif CMDLINE

	if(sucheck())
	    exit(1);
/*
 * Get CPU type and other info from kernel.
 */
	if(nlist("/unix", nl) < 0) {
	    printf("\nCan't access namelist in /unix!\n");
	    exit(1);
	}
	if(nl[X_CPUTYPE].n_value == 0) {
	    printf("\nCan't get needed symbol values from /unix namelist!\n");
	    exit(1);
	}
	if((mem = open("/dev/mem", 0)) < 0) {
	    printf("\nCan't open /dev/mem (memory) for reading!\n");
	    exit(1);
	}
	if(nl[X_ROOTDEV].n_value == 0) {
	    printf("\nCan't get root device from kernel!\n");
	    exit(1);
	} else {
	    lseek(mem, (long)nl[X_ROOTDEV].n_value, 0);
	    read(mem, (char *)&rootdev, sizeof(rootdev));
	}
	if (nl[X_CPUTYPE].n_value) {
	    lseek(mem, (long)nl[X_CPUTYPE].n_value, 0);
	    read(mem, (char *)&cputype, sizeof(cputype));
	} else {
	    printf("Cannot get CPU type from kernel!\n");
	    cputype=0;
	}
	while(1) {
	    i = 0;
	    switch(cputype) {
		case 23:
		case 24:
		case 34:
		case 40:
		case 44:
		case 45:
		case 53:
		case 55:
		case 60:
		case 70:
		case 73:
		case 83:
		case 84:
			break;
		default:
			i++;
			break;
	    }
	    if(i == 0)
		break;
	    printf("\n%d is not a supported CPU type!\n", cputype);
	    printf("\nPlease enter the CPU type with features most closely");
	    printf("\nmatching your CPU, choose one of the following CPUs:\n");
	    printf("\n    23 24 34 40 44 45 55 60 70 73 83 84\n");
	    if(getline(NOHELP) < 0)
		continue;
	    cputype = atoi(lbuf);
	    continue;
	}
/*
 * Find out what disk controllers/drives are present
 * and save the necessary info in the di_info[] table.
 *
 * 1. Read bdevsw[] entry for disk controller.
 * 2. If d_tab entry is 0, controller not configured.
 * 3. Make a special file to access cntlr unit 0.
 * 4. Open unit 0 to force driver to load ??_dt[] with drive types.
 *    Note: RP require reading each unit.
 * 5. Call appropriate routine to get drive type info and
 *    load it into di_info[].
 *
 */
	for(dip=di_info; dip->di_typ; dip++) {
/* 1 */		soff = (long)nl[X_BDEVSW].n_value +
		   (long)(sizeof(struct bdevsw) * dip->di_bmaj);
		lseek(mem, (long)soff, 0);
		read(mem, (char *)&bdevswb, sizeof(struct bdevsw));
/* 2 */		dip->di_nunit = 0;
		if(bdevswb.d_tab == 0)
		    continue;
/* 3 */		unlink(dsk_sfn);
		if(mknod(dsk_sfn, 020400, (dip->di_rmaj<<8) | (dip->di_cn<<6)) < 0) {
		    printf("\nERROR: mknod for %s controller %d failed!\n",
			dip->di_typ, dip->di_cn);
			exit(1);
		}
/* 4 */		if((i = open(dsk_sfn, 0)) >= 0)
		    close(i);
		unlink(dsk_sfn);
/* 5 */		for(i=0; i<8; i++)	/* set all drive types to NED */
		    dip->di_dt[i] = 0;
		(*dip->di_func)(MKFS, dip, mem); /* get drive type info */
	}

	sumask = umask(0);
	imode = 0;
	if(argc == 1) {		/* Interactive mode */
	    imode++;
	    printf("\nULTRIX-11 New File System Program\n");
	    printf("\nFor help, type ? or help, then press <RETURN>.");
	    setjmp(savej);
	    signal(SIGINT, intr);
	    printf("\n");
	    while(1) {
		printf("\nCommand < exit help newfs table >: ");
		if(getline(NOHELP) <= 0)
		    continue;	/* user typed help, <RETURN> or <CTRL/D> */
		switch(lbuf[0]) {
		case 'h':
		case '?':		/* print help message */
		    pmsg(help);
		    continue;
		case 'e':		/* exit from program */
		    umask(sumask);
		    exit(0);
		case 't':		/* print device table */
		    dotable();
		    continue;
		case 'n':		/* make a new file system */
		    dtp = getinfo();
		    donewfs(dtp, mem);
		    continue;
		default:
		    printf("\n%s - not a valid command!\n", lbuf);
		    continue;
		}
	    }
	}
#ifdef CMDLINE
/*
 * Non interactive mode.
 */
	signal(SIGINT, intr);
	unit = 0;
	part = gotunit = gotdisk = gotfsname = 0;
	cntlr = 0;
	for(i=1; argv[i]; i++) {
	    if(equal("-t", argv[i])) {	/* TABLE */
		dotable();
		if(argc == 2)
		    xit();
		else {
		    argc--;
		    continue;
		}
	    }
	    if(equal("-c", argv[i])) {	/* CONTROLLER NUMBER */
		/*
		 * For now, cntlr limit is 3.
		 * Check again later, when we know device type.
		 */
		i++;	/* get next arg */
		if((strlen(argv[i]) != 1) ||
		   (argv[i][0] < '0') || (argv[i][0] > '3')) {
			printf("\nnewfs: %s", bcnerr);
			pmsg(oldusage);
			errxit();
		}
		cntlr = atoi(argv[i]);
		continue;
	    }

	    if ((strlen(argv[i]) == 1) &&
	    	((argv[i][0] >= '0') && (argv[i][0] <= '7'))) {	/* unit # */
		if (gotunit == 0) {
		    unit = atoi(argv[i]);
		    gotunit++;
		    if((unit < 0) || (unit > 7)) {
		        printf("\nnewfs: %s", bunerr);
		        pmsg(oldusage);
		        errxit();
		    }
		} else {	/* already have unit, must be dp# */
		    part = atoi(argv[i]);
		    if((unit < 0) || (unit > 7)) {
		        printf("\nnewfs: %s", bunerr);
		        pmsg(oldusage);
		        errxit();
		    }
		}
		continue;
	    } else {		/* must assume it's a device name */
		if (gotdisk == 0) {
		    for(dtp=dt_info; dtp->dt_lname; dtp++)
		        if(equal(argv[i], dtp->dt_lname)) {
			    gotdisk++;
			    break;
			}
		    if(dtp->dt_lname == 0) {
		        printf("\nnewfs: %s - invalid device name!\n", argv[i]);
		        pmsg(oldusage);
		        errxit();
		    }
		} else {	/* look for optional fsname/volname */
		    if (gotfsname == 0) {
		        gotfsname++;
		        strncpy(fsname, argv[i], 6);
		    }
		    else {	/* have fsname, must be volname */
		        strncpy(volname, argv[i], 6);
			i--;	/* should be end of input */
		        if (argv[i]) {
			    printf("newfs: too many args.\n");
			    pmsg(oldusage);
			    errxit();
		        }
		        break;
		    }
		}
	    }

	    if(((strlen(argv[i]) == 1)) &&
		((argv[i][0] >= '0') && (argv[i][0] <= '7'))) {	/* partition# */
		unit = atoi(argv[i]);
		if((unit < 0) || (unit > 7)) {
		    printf("\nnewfs: %s", bunerr);
		    pmsg(oldusage);
		    errxit();
		}
		continue;
	    }
	} /* for i in argv[i] DONE scanning input */

	donewfs(dtp, mem);
	xit();
#endif CMDLINE
}

/*
 * Ask user for device name,
 * [cn], unit number, [tty##].
 */
getinfo()
{
	register struct dt_info *dp;
	register int cc;

	while(1) {
	    do
		printf("\nDisk name (? for help) < rd52, rp06, ra60, etc. >: ");
	    while((cc = getline(h_dsknam)) < 0);
	    if(cc == 0)
		continue;	/* user typed <RETURN> */
	    for(dp=dt_info; dp->dt_lname; dp++)
		if(strcmp(lbuf, dp->dt_lname) == 0)
		    break;
	    if(dp->dt_lname == 0) {
		printf("\n%s - bad disk name!\n", lbuf);
		continue;
	    }
	    break;
	}
	while(1) {
	    cntlr = 0;
	    if(!(dp->dt_flags&DI_MASS))	/* only prompt if MASSBUS */
		break;
	    do
		printf("\nMASSBUS disk controller number < 0 1 2 >: ");
	    while((cc = getline(h_cn)) < 0);
	    if(cc == 0)
		continue;	/* <RETURN> */
	    if((cc != 1) || (lbuf[0] < '0') || (lbuf[0] > '2')) {
		printf("\n%s - bad controller number!\n", lbuf);
		continue;
	    }
	    cntlr = lbuf[0] - '0';
	    break;
	}
	while(1) {
	    unit = 0;
	    do {
		if (dp->dt_type==RX02)
		    printf("\nUnit number < 0  1 >: ");
		else
		    printf("\nUnit number < 0 -> 7 >: ");
	    } while((cc = getline(h_unit)) < 0);
	    if(cc == 0)
		continue;
	    if (dp->dt_type==RX02)	/* RX02 can only be unit 0 or 1 */
	        if((cc == 1) && ((lbuf[0] == '0') || (lbuf[0] == '1'))) {
	    	    unit = lbuf[0] - '0';	/* OK */
		    break;
	        } else
		    cc = 2;/* so catch it next line */

	    if((cc != 1) || (lbuf[0] < '0') || (lbuf[0] > '7')) {
		    printf("\n%s - bad unit number!\n", lbuf);
		    continue;
	    }
	    unit = lbuf[0] - '0';
	    break;
	}

	if ((dp->dt_flags&DI_NPD) && (dp->dt_type==RX02)) {
	    hx_single = 0;	/* set double density default */
	    while(1) {
	        do {
		    printf("\nsingle or double density? < double >: ");
	        } while((cc = getline(h_density)) < 0);
	        if(cc == 0)
		    break;	/* assume default (double density) */
	        if((lbuf[0] == 's') || (lbuf[0] == 'S')) {
		    hx_single = 1; 	/* single density */
	        }
	        break;
	    }
	}

	while(1) {
	    dpart = 0;
	    /* Use parition 0 for RK05 */
	    if((dp->dt_flags&DI_NPD) &&	(dp->dt_type == RK05)) {
		dpart = 0;
		break;
	    }
	    /* Use parition 7 if RX02 (DI_NPD) or if RX50/RX33 (DI_RX50) */
	    if((dp->dt_flags&DI_NPD) ||	(dp->dt_flags&DI_RX50)) {
		dpart = 7;
		break;
	    }
	    if(dp->dt_type == ML11) {	/* ML11 uses unit number instead */
		dpart = unit;
		break;
	    }
	    while(1) {
		do
		    printf("\nDisk partition number < 0 -> 7 >: ");
		while((cc = getline(h_dpart)) < 0);
		if(cc == 0)
		    continue;
		if((cc != 1) || (lbuf[0] < '0') || (lbuf[0] > '7')) {
		    printf("\n%s - bad partition number!\n", lbuf);
		    continue;
		}
		dpart = lbuf[0] - '0';
		/* check if disk can utilize this partition or not */
		if((dp->dt_pmask&(1 << dpart)) == 0) {
		    printf("\nSorry, %s disk does not use partition %d!\n",
		        dp->dt_uname, dpart);
		    continue;
	        }
		break;
	    }
	    break;
	}
	while(1) {
	    do
		printf("\nSuperblock: file system name (fsname) < ? for help >: ");
	    while((cc = getline(h_fsn)) < 0);
	    if(cc == 0) {
		sprintf(&fsname, "      ");
		break;
	    }
	    if(cc > 6) {
		printf("\nfsname - 6 characters maximum!\n");
		continue;
	    }
	    sprintf(&fsname, "%s", lbuf);
	    break;
	}
	while(1) {
	    do
		printf("\nSuperblock: volume name (volname) < ? for help >: ");
	    while((cc = getline(h_voln)) < 0);
	    if(cc == 0) {
		sprintf(&volname, "      ");
		break;
	    }
	    if(cc > 6) {
		printf("\nvolname - 6 characters maximum!\n");
		continue;
	    }
	    sprintf(&volname, "%s", lbuf);
	    break;
	}
	return(dp);
}

/*
 * Print table of configured disk controllers
 * and drives on each controller.
 */

dotable()
{
	register struct di_info *dip;
	register int j;

	printf("\nULTRIX-11 System's Disk Configuration:\n");
	printf("\nX = disk not configured, ");
	printf("NED = disk configured but not present.\n");
	printf("\nDisk    Cntlr  Unit  Unit  Unit  Unit  ");
	printf("Unit  Unit  Unit  Unit");
	printf("\nCntlr   #      0     1     2     3     ");
	printf("4     5     6     7");
	printf("\n-----   -----  ----  ----  ----  ----  ");
	printf("----  ----  ----  ----");
	for(dip=di_info; dip->di_typ; dip++) {
	    if(dip->di_nunit == 0)
		continue;
	    printf("\n%-6s  %-5d  ", dip->di_typ, dip->di_cn);
	    for(j=0; j<8; j++) {
		if(j >= dip->di_nunit)
		    printf("X   ");
		else if(dip->di_dt[j] == 0)
		    printf("NED ");
		else
		    printf("%-4s", dt_info[dip->di_dt[j]].dt_uname);
		printf("  ");
	    }
	}
	printf("\n");
}

/*
 * need funct header
 */

donewfs(dp, mem)
register struct dt_info *dp;
int	mem;
{
	register struct di_info *dip;
	register int i, j;
	int	found, problem;
	char buf[128];	/* buffer to hold mntdir + lpfdir + "junkxx" */
	struct stat statb;

	found = 0;
	for(dip=di_info; dip->di_typ; dip++) {
	    if(dip->di_nunit == 0)
		continue;	/* cntlr not configured */

	/* If controllers don't match then it's only the wrong one if
	 * DI_MASS flag is set (since user specified which MASSBUS cntlr#).
	 * The cntlr variable is sometimes 0 when it really could be
	 * 1 or 2, so we check them all for the MSCP case.
	 */
	    if(dip->di_cn != cntlr)
		if (dip->di_flags&DI_MASS)
		    continue;		/* wrong MASSBUS controller */

	    for(j=0; j < dip->di_nunit; j++) {
		if(j != unit)
		    continue;		/* wrong unit */
		if(dp->dt_type == dt_info[dip->di_dt[j]].dt_type) {
		    found++;
		    break;
		}
	    }
	    if(found)
		break;
	}
	if(dip->di_typ == 0) {
	    printf("\n%s Cntlr %d Unit %d -- ", dp->dt_uname, cntlr, unit);
	    printf("not configured or not present!\n");
	    return;
	}
	/*
	 * Don't allow the user to make file systems
	 * on partitions used by the system, if this
	 * is the system disk.
	 * Notes: RL01 uses drives 0 & 1 for system.
	 *	  System disk can only be on controller zero.
	 */
	if((dp->dt_type == RL01) && (unit == 1))
	    j = 0;	/* root device will be unit 0 */
	else
	    j = unit;
	i = (dip->di_bmaj << 8) | (j << 3);
	if(i == (rootdev & ~7)) {	/* this is the system disk */
	    if(dp->dt_smask & (1 << dpart)) {
		printf("\n\7\7\7NEWFS ABORTED: ");
		printf("would overwrite part of the system disk!\n");
		return;
	    }
	}
	/*
	 * RX50/RX33 special case,
	 * open drive and get sizes info again.
	 * For RX33, media size can change any time because
	 * RX33 works with both RX33 (1200 blocks) and RX50 (400 blocks).
	 * Also, RA driver can't detect media change or off-line
	 * until the floppy door is closed (with media mounted).
	 */
	while(1) {
	    if((dp->dt_flags&DI_RX50) == 0)
		break;	/* not RX50/RX33 disk */
	    j = open(dsfn(RAW, unit, dpart, dip), 0);
	    if((j >= 0) && (read(j, (char *)&rbuf, 512) == 512)) {
		close(j);
		if (nl[X_RA_DRV].n_value) {
		    j = 0;	/* need to know how many MSCP units */
		    for(i=0; i<MAXUDA; i++)
			j += nra[i];
		    lseek(mem, (long)nl[X_RA_DRV].n_value, 0);
		    read(mem, (char *)&ra_drv, sizeof(struct ra_drv) * j);
		}
	    } else {
		if(j >= 0)
		    close(j);
		printf("\n%s Unit %d appears to be off-line!\n",
		    dp->dt_uname, unit);
		return;
	    }
	    break;
	}
	spt(unit, dip);		/* set partition sizes table */

#ifdef DEBUG
	for (j=0; j<=7; j++)
	    printf("%ld	%ld\n",pt_info[j].pt_nb, pt_info[j].pt_sb);
#endif DEBUG

	sprintf(syscmd, "/etc/mkfs %s %D %s %d %.6s %.6s",
	    dsfn(RAW, unit, dpart, dip),
	    (long)pt_info[dpart].pt_nb/2,
	    dt_info[dip->di_dt[unit]].dt_lname,
	    cputype, fsname, volname);
	printf("\nMake file system command will be:\n");
	printf("\n    %s\n", syscmd);
	printf("\nPress <RETURN> to continue (or <CTRL/C> to abort):");
	while(getchar() != '\n');
	printf("\n%s\n", syscmd);
	if (system(syscmd) != 0) {
	    printf("\nERROR: make file system failed\n");
	    return(1);
	}
	problem=0;	/* only a problem if > 8 */
	while(1) {
	if (stat(mntdir, &statb) < 0) {
	    if (mkdir(mntdir, 755) < 0) {
		printf("\nnewfs: cannot make directory %s!\n", mntdir);
		printf("Warning: no lost+found directory was created on the new filesystem.\n");
		return(1);
	    } else {	/* OK */
		chmod(mntdir, 0755);
		break;
	    }
	} else if ((statb.st_mode&S_IFMT) == S_IFDIR) {
		mntdir[13] += 1;
		if (problem++ >= 8) {	/* try ...dir1 through ...dir9 */
		    printf("\nnewfs: cannot create a unique mount directory!\n", mntdir);
		    printf("Warning: no lost+found directory was created on the new filesystem.\n");
		    return(1);
		}
	} else { /* existing mntdir is not directory, zap it and continue */
		sprintf(syscmd, "rm -f %s", mntdir);
		system(syscmd);		/* continue */
	}
	} /* while(1) */

	/*
	 * Mount the file system and chmod 0755.
	 * Make the lost+found directory.
	 */
	sprintf(syscmd,"/etc/mount %s %s", dsfn(BLOCK, unit, dpart, dip), mntdir);
	if(system(syscmd) != 0) {
	    printf("newfs: cannot mount %s on %s\n",
		dsfn(BLOCK, unit, dpart, dip), mntdir);
	    printf("Warning: no lost+found directory was created on the new filesystem.\n");
	    /* rmdir and return */
	}
	else {	/* the mount succeeded, create some junk files */
	    strcpy(buf, mntdir);	/* start at mount point */
	    strcat(buf, "/");
	    strcat(buf, lpfdir);	/* tack on "/lost+found" */
	    while(1) {
	        if ((mkdir(buf, 0700)) < 0)
		    printf("Warning: cannot mkdir %s\n", buf);
	        for(i=0; i<25; i++) {
		    sprintf(&syscmd, "%s/junk%d", buf, i);
		    j = creat(&syscmd, 0666);
		    if(j < 0)
		        continue;
		    write(j, (char *)&syscmd, 512);
		    close(j);
	        }
	        sync();
	        sync();
	        sprintf(&syscmd, "rm -f %s/junk*", buf);
	        system(syscmd);
	        break;
	    }	/* while */
	    sprintf(syscmd, "/etc/umount %s", dsfn(BLOCK, unit, dpart, dip));
	    if(system(syscmd) != 0) {
	        printf("\nWARNING: (%s) file system dismount failed!\n",
		    dsfn(BLOCK, unit, dpart, dip));
	    }
	}
	rmdir(mntdir);
}

/*
 * Get a line of text from the terminal,
 * replace the newline with a NULL.
 * Return the string length (not counting the NULL).
 * Use lbuf[] as the buffer and LBSIZE as limit.
 * If ? or help is typed, print the help message, if
 * one is available, apologize if not. Return -1 after help.
 */

char	*badline = "\n\7\7\7Bad input line, please try again!\n";

getline(hlp)
char	*hlp;
{
	register int cc, ovflow;

	ovflow = 0;
	fflush(stdout);
	while(1) {
	    lbuf[0] = '\0';
	    if(fgets(&lbuf, LBSIZE, stdin) == NULL) {
		printf("%s", badline);
		return(-1);	/* reprint message */
	    }
	    for(cc=0; lbuf[cc]; cc++) ;
	    if((cc == 0) || (lbuf[0] == '\n'))
		return(0);	/* (cc==0) says user typed just a <RETURN> */
	    if(lbuf[cc-1] != '\n') {	/* line too long */
		ovflow++;
		continue;
	    }
	    if(ovflow) {
		printf("\nLine too long, please try again!\n");
		return(-1);
	    }
	    cc--;
	    lbuf[cc] = '\0';
	    if((lbuf[0] == '?') || equal("help", lbuf) || equal("h", lbuf)) {
		if(hlp) {
		    pmsg(hlp);
		    return(-1);
		}
	    }
	    return(cc);
	}
}

equal(a, b)
char	*a, *b;
{
	return(!strcmp(a, b));
}

pmsg(str)
char	**str;
{
	register int i;

	for(i=0; str[i]; i++)
		printf("\n%s", str[i]);
	fflush(stdout);
}

sucheck()
{
	if(getuid() == 0)
		return(0);
	else {
		printf("\nOnly super-user can make file systems!\n");
		return(1);
	}
}

intr()
{
	if(imode == 0) {	/* non interactive mode */
	    umask(sumask);
	    exit(0);
	}
	longjmp(savej, 1);
}

errxit()
{
	umask(sumask);
	exit(1);
}

xit()
{
	umask(sumask);
	exit(0);
}

/*
 * Routines to fish the disk drive type
 * info out of the running kernel (/dev/mem).
 * BAD DEPENDENCY: routines need to know how each
 * disk driver defines NED and drive types.
 *
 * usage - identifies section of code calling ??_gdti(),
 *	   ONLY USED BY ra_gdti().
 *	   MSF, called by make special files code.
 *	   MKFS, called by setup user file systems code.
 * dip   - pointer to entry in di_info[].
 * mem   - open file descriptor for /dev/mem.
 *
 * Note:
 *	 The data structures used by these routines
 *	 are located before main(), so code in main can
 *	 access them.
 */

/*
 * RK05
 *	NED = -1
 *	RK05 = 1
 */

rk_gdti(usage, dip, mem)
register struct di_info *dip;
{
	register int i, j;

	if (nl[X_NRK].n_value) {
	    lseek(mem, (long)nl[X_NRK].n_value, 0);
	    read(mem, (char *)&nrk, sizeof(nrk));
	}
	if (nl[X_RK_DT].n_value) {
	    lseek(mem, (long)nl[X_RK_DT].n_value, 0);
	    read(mem, (char *)&rk_dt, sizeof(rk_dt));
	}
	dip->di_nunit = nrk;
	for(i=0; i<nrk; i++) {
	    if(rk_dt[i] != 1)
		dip->di_dt[i] = 0;	/* NED (configured but not found) */
	    else {
		for(j=0; dt_info[j].dt_lname; j++)
		    if(dt_info[j].dt_type == RK05)
			break;
		dip->di_dt[i] = j;	/* save index into drive info table */
	    }
	}
}

/*
 * RP02/3
 *	NED = -1
 *	RP02 = 0
 *	RP03 = 1
 * Note: must read each configured RP02/3 unit to
 *	 force update of rp_dt[].
 */

rp_gdti(usage, dip, mem)
register struct di_info *dip;
{
	register int i, j;

	for(i=0; i<nrp; i++) {
	    mknod(dsk_sfn, 020400, (dip->di_rmaj<<8) | (i<<3));
	    if((j = open(dsk_sfn, 0)) >= 0) {
		read(j, (char *)&rbuf, 512);
		close(j);
	    }
	    unlink(dsk_sfn);
	}
	if (nl[X_NRP].n_value) {
	    lseek(mem, (long)nl[X_NRP].n_value, 0);
	    read(mem, (char *)&nrp, sizeof(nrp));
	}
	if (nl[X_RP_DT].n_value) {
	    lseek(mem, (long)nl[X_RP_DT].n_value, 0);
	    read(mem, (char *)&rp_dt, sizeof(rp_dt));
	}
	if (nl[X_RP_SIZES].n_value) {
	    lseek(mem, (long)nl[X_RP_SIZES].n_value, 0);
	    read(mem, (char *)&rp_sizes, sizeof(rp_sizes));
	}
	dip->di_nunit = nrp;
	for(i=0; i<nrp; i++) {
	    if(rp_dt[i] < 0)
		dip->di_dt[i] = 0;	/* NED (configured but not found) */
	    else {
		for(j=0; dt_info[j].dt_lname; j++)
		    /* setup uses (RP02=2, RP03=3, so we add 2) */
		    if(dt_info[j].dt_type == (rp_dt[i] + 2))
			break;
		dip->di_dt[i] = j;	/* save index into drive info table */
	    }
	}
}

/*
 * MSCP (UDA50 RC25 RQDX RUX50)
 *	NED = 0
 *	See /usr/include/sys/ra_info.h for drive types.
 *
 * Note: (usage argument)
 *
 *	If usage == MSF (called from make special files code),
 *	just get drive type info. If usage == MKFS (called from
 *	setup user file systems code), get drive type info and
 *	force all possible drives on-line by attempting open.
 *	This ensures the size info is correct.
 *
 * Note: MSCP drive type info arrays are non-semetrical
 *	 ra_index must be used to access drive type info.
 */

ra_gdti(usage, dip, mem)
register struct di_info *dip;
{
	register int i, j;
	int	cn, ind;
	int	raunits;

/*
 * Read in the controller and drive type info.
 */
	    if (nl[X_NUDA].n_value) {
		lseek(mem, (long)nl[X_NUDA].n_value, 0);
		read(mem, (char *)&nuda, sizeof(nuda));
	    }
	    if (nl[X_NRA].n_value) {
		lseek(mem, (long)nl[X_NRA].n_value, 0);
		read(mem, (char *)&nra, MAXUDA);
	    }
	    if (nl[X_UD_SIZES].n_value) {
		lseek(mem, (long)nl[X_UD_SIZES].n_value, 0);
		read(mem, (char *)&ud_sizes, sizeof(ud_sizes));
	    }
	    if (nl[X_RQ_SIZES].n_value) {
		lseek(mem, (long)nl[X_RQ_SIZES].n_value, 0);
		read(mem, (char *)&rq_sizes, sizeof(rq_sizes));
	    }
	    if (nl[X_RC_SIZES].n_value) {
		lseek(mem, (long)nl[X_RC_SIZES].n_value, 0);
		read(mem, (char *)&rc_sizes, sizeof(rc_sizes));
	    }
	    if (nl[X_RA_MAS].n_value) {
		lseek(mem, (long)nl[X_RA_MAS].n_value, 0);
		read(mem, (char *)&ra_mas, sizeof(daddr_t)*nuda);
	    }
	    raunits = 0;
	    for(i=0; i<MAXUDA; i++)
	        raunits += nra[i];
	    if (nl[X_RA_INDEX].n_value) {
		lseek(mem, (long)nl[X_RA_INDEX].n_value, 0);
		read(mem, (char *)&ra_index, MAXUDA);
	    }
	    if (nl[X_RA_CTID].n_value) {
		lseek(mem, (long)nl[X_RA_CTID].n_value, 0);
		read(mem, (char *)&ra_ctid, MAXUDA);
	    }
	/*
	 * If called from make user file systems code,
	 * make sure size and on-line information in ra_drv[]
	 * is updated by attempting to open each configured drive.
	 * Only do this once, to avoid multiple UNIT off-lines.
	 */
	    if(dip->di_flags&DI_FIRST) {
		for(i=0; i<nuda; i++)
		    for(j=0; j<nra[i]; j++) {
			ind = (dip->di_rmaj << 8) | (i << 6) | (j << 3) | 7;
			mknod(dsk_sfn, 020400, ind);
			if((ind = open(dsk_sfn, 0)) >= 0)
			    close(ind);
			unlink(dsk_sfn);
		    }
	    }
	    if (nl[X_RA_DRV].n_value) {
		lseek(mem, (long)nl[X_RA_DRV].n_value, 0);
		read(mem, (char *)&ra_drv, sizeof(struct ra_drv) * raunits);
	    }

	cn = dip->di_cn;
	ind = ra_index[cn];
	dip->di_nunit = nra[cn];
	for(i=0; i<nra[cn]; i++) {
	    if(ra_drv[ind+i].ra_dt == 0)
		dip->di_dt[i] = 0;	/* NED (configured but not found) */
	    else {
		for(j=0; dt_info[j].dt_lname; j++)
		    if(dt_info[j].dt_type == ra_drv[ind+i].ra_dt)
			break;
		dip->di_dt[i] = j;	/* save index into drive info table */
	    }
	}
/*
 * Change the controller name from MSCP to
 * the real name (UDA50, RQDX, etc.).
 */
	dip->di_typ = ra_gcti(ra_ctid[cn]);
/*
 * Change the two character ULTRIX-11 mnemonic to
 * ra, rc, rd, rx according to the controller type.
 */
	switch((ra_ctid[cn] >> 4) & 017) {
	case UDA50:
	case UDA50A:
	case KDA50:
		dip->di_name[1] = 'a';		/* RA60/RA80/RA81 */
		break;
	case KLESI:
		dip->di_name[1] = 'c';		/* RC25 */
		break;
	case RQDX1:
	case RQDX3:
		dip->di_name[1] = 'd';		/* RD51/RD52/RD53/RD54 */
		break;				/* RD31/RD32 */
	case RUX1:
		dip->di_name[1] = 'x';		/* RUX50 */
		break;
	default:
		dip->di_name[1] = 'a';		/* UNKNOWN (can't happen) */
		break;
	}
}

/*
 * Return a pointer to the controller name string
 * according to the controller type ID.
 */

ra_gcti(ctid)
{
	switch((ctid >> 4) & 017) {
	case UDA50:
		return("UDA50");
	case KDA50:
		return("KDA50");
	case KLESI:
		return("KLESI");
	case UDA50A:
		return("UDA50A");
	case RQDX1:
		return("RQDX");
	case RQDX3:
		return("RQDX3");
	case RUX1:
		return("RUX1");
	default:
		return("UNKNOWN");
	}
}

/*
 * RX02
 *	RX02 driver does not have drive type info.
 *	Always 2 RX02 if HX configured.
 */
hx_gdti(usage, dip, mem)
register struct di_info *dip;
{
	register int i;

	if (nl[X_NHX].n_value) {
	    lseek(mem, (long)nl[X_NHX].n_value, 0);
	    read(mem, (char *)&nhx, sizeof(nhx));
	}
	if (nhx) {
	    dip->di_nunit = 2;
	    for(i=0; dt_info[i].dt_lname; i++)
	        if(dt_info[i].dt_type == RX02)
		    break;
	    dip->di_dt[0] = i;	/* unit 0 = RX02 */
	    dip->di_dt[1] = i;	/* unit 1 = RX02 */
	}
}

/*
 * RL01/2
 *	NED = -1
 *	RL01 = 10240
 *	RL02 = 20480
 */

rl_gdti(usage, dip, mem)
register struct di_info *dip;
{
	register int i, j;

	if (nl[X_NRL].n_value) {
	    lseek(mem, (long)nl[X_NRL].n_value, 0);
	    read(mem, (char *)&nrl, sizeof(nrl));
	}
	if (nl[X_RL_DT].n_value) {
	    lseek(mem, (long)nl[X_RL_DT].n_value, 0);
	    read(mem, (char *)&rl_dt, sizeof(rl_dt));
	}
	dip->di_nunit = nrl;
	for(i=0; i<nrl; i++) {
	    if(rl_dt[i] < 0)
		dip->di_dt[i] = 0;	/* NED (configured but not found) */
	    else {
		for(j=0; dt_info[j].dt_lname; j++)
		    if(dt_info[j].dt_type == rl_dt[i])
			break;
		dip->di_dt[i] = j;	/* save index into drive info table */
	    }
	}
}

/*
 * MASSBUS (HP HM HJ)
 *	NED = 0
 *	See /usr/include/sys/hp_info.h for drive types.
 *
 * Note: MASSBUS drive type info arrays are non-semetrical
 *	 hp_index must be used to access drive type info.
 */

int	hpload = NO;		/* HP info already loaded */

hp_gdti(usage, dip, mem)
register struct di_info *dip;
{
	register int i, j;
	int	cn, ind;
	int	hpunits;

/*
 * Read in the controller and drive type info.
 */
	if(hpload == NO) {
	    hpload = YES;
	    if (nl[X_NHP].n_value) {
		lseek(mem, (long)nl[X_NHP].n_value, 0);
		read(mem, (char *)&nhp, MAXRH);
	    }
	    if (nl[X_HP_SIZES].n_value) {
		lseek(mem, (long)nl[X_HP_SIZES].n_value, 0);
		read(mem, (char *)&hp_sizes, sizeof(hp_sizes));
	    }
	    hpunits = 0;
	    for(i=0; i<MAXRH; i++)
	        hpunits += nhp[i];
	    if (nl[X_HP_INDEX].n_value) {
		lseek(mem, (long)nl[X_HP_INDEX].n_value, 0);
		read(mem, (char *)&hp_index, MAXRH);
	    }
	    if (nl[X_HP_DT].n_value) {
		lseek(mem, (long)nl[X_HP_DT].n_value, 0);
		read(mem, (char *)&hp_dt, hpunits);
	    }
	}
	cn = dip->di_cn;
	ind = hp_index[cn];
	dip->di_nunit = nhp[cn];
	for(i=0; i<nhp[cn]; i++) {
	    if(hp_dt[ind+i] == 0)
		dip->di_dt[i] = 0;	/* NED (configured but not found) */
	    else {
		for(j=0; dt_info[j].dt_lname; j++)
		    if(dt_info[j].dt_type == hp_dt[ind+i])
			break;
		dip->di_dt[i] = j;	/* save index into drive info table */
	    }
	}
/*
 * Change the controller name from MASSBUS to
 * the real name (RH11 or RH70).
 */
	if(cputype == 70)
	    dip->di_typ = "RH70";
	else
	    dip->di_typ = "RH11";
}

/*
 * RK06/7
 *	NED = -1
 *	RK06 = 0
 *	RK07 = 02000
 */

hk_gdti(usage, dip, mem)
register struct di_info *dip;
{
	register int i, j;

	if (nl[X_NHK].n_value) {
	    lseek(mem, (long)nl[X_NHK].n_value, 0);
	    read(mem, (char *)&nhk, sizeof(nhk));
	}
	if (nl[X_HK_DT].n_value) {
	    lseek(mem, (long)nl[X_HK_DT].n_value, 0);
	    read(mem, (char *)&hk_dt, sizeof(hk_dt));
	}
	if (nl[X_HK_SIZES].n_value) {
	    lseek(mem, (long)nl[X_HK_SIZES].n_value, 0);
	    read(mem, (char *)&hk_sizes, sizeof(hk_sizes));
	}
	dip->di_nunit = nhk;
	for(i=0; i<nhk; i++) {
	    if(hk_dt[i] < 0)
		dip->di_dt[i] = 0;	/* NED (configured but not found) */
	    else {
		for(j=0; dt_info[j].dt_lname; j++)
		    if(dt_info[j].dt_type == hk_dt[i])
			break;
		dip->di_dt[i] = j;	/* save index into drive info table */
	    }
	}
}

/*
 * Set up the disk partition sizes table.
 *
 *	unit	disk unit number
 *	dip	drive info table pointer
 */

spt(unit, dip)
register struct di_info *dip;
int	unit;
{
	register struct dksize *dsp;
	register struct rasize *rsp;
	int i, nbpc;

	for(i=0; i<8; i++) {
	    pt_info[i].pt_nb = 0L;
	    pt_info[i].pt_sb = 0L;
	}
	switch(dip->di_bmaj) {
	case RK_BMAJ:
	    pt_info[0].pt_sb = 0L;
	    pt_info[0].pt_nb = (long)rk_size;
	    return(0);
	case RP_BMAJ:
	    nbpc = RP23_BPC;
	    dsp = &rp_sizes;
	    break;
	case RA_BMAJ:
	    switch((ra_ctid[dip->di_cn] >> 4) & 017) {
	    case UDA50:
	    case UDA50A:
	    case KDA50:
		rsp = &ud_sizes;
		break;
	    case KLESI:
		rsp = &rc_sizes;
		break;
	    case RQDX1:
	    case RQDX3:
	    case RUX1:
		rsp = &rq_sizes;
		break;
	    default:
		/* THIS CAN'T HAPPEN */
		printf("\n(spt) - unknown controller type!\n");
		return(1);
	    }
	/*
	 * RA sizes tables use following magic numbers:
	 * (ud_sizes, rc_sizes, rq_sizes)
	 * -1, partition ends at end of disk.
	 * -2, partition ends at start of maintenance area.
	 * Note: for the purposes of mkfs -1 and -2 mean the same.
	 *	 This prevents a file system on partition 7 from
	 *	 overlapping the maintenance area.
	 */
	    for(i=0; i<8; i++) {
		if(rsp->nblocks) {
		    pt_info[i].pt_nb = rsp->nblocks;
		    pt_info[i].pt_sb = rsp->blkoff;
		}
		/* -1 and -2 mean the same thing here, see comment above */
		if(rsp->nblocks < 0L) {
		    pt_info[i].pt_nb = (long)ra_drv[ra_index[dip->di_cn]+unit].d_un.ra_dsize - (long)rsp->blkoff;
		    /*
		     * Don't subtract maintenance area if RX33 or RX50 disk
		     */
		    if((dt_info[dip->di_dt[unit]].dt_flags&DI_RX50) == 0)
		        pt_info[i].pt_nb -= ra_mas[dip->di_cn];
		}
		rsp++;
	    }
	    nbpc = 0;
	    break;
	case RL_BMAJ:
	    /* all partitions (except RL02 partition 1) start at block 0 */
	    pt_info[7].pt_nb = (long)rl_dt[unit]; /* drive type = unit size */
	    pt_info[0].pt_nb = 10240L;	/* RL01 & RL02 both use partition 0 */
	    if(rl_dt[unit] == RL02) {
		pt_info[1].pt_sb = 10240L;
		pt_info[1].pt_nb = 10240L;
	    }
	    return(0);
	case HX_BMAJ:
	    /* if((dt_info[dip->di_dt[unit]].dt_flags&DI_SINGLE) == 1) */
	    if (hx_single == 1)
	        pt_info[7].pt_nb = 500L;
	    else	/* normal RX02 double density */
	        pt_info[7].pt_nb = 1000L;
	    return(0);
	case HP_BMAJ:
	case HM_BMAJ:
	case HJ_BMAJ:
	    switch(dt_info[dip->di_dt[unit]].dt_type) {
	    case RP04:
	    case RP05:
	    case RP06:
		nbpc = RP456_BPC;
		dsp = &hp_sizes;
		break;
	    case RM02:
	    case RM03:
		nbpc = RM23_BPC;
		dsp = &hp_sizes[8];
		break;
	    case RM05:
		nbpc = RM5_BPC;
		dsp = &hp_sizes[16];
		break;
	    case ML11:
		nbpc = 1;
		dsp = &hp_sizes[24];
		break;
	    default:
		/* THIS CAN'T HAPPEN */
		printf("\n(spt) - unknown drive type!\n");
		return(1);
	    }
	    break;
	case HK_BMAJ:
	    nbpc = RK67_BPC;
	    dsp = &hk_sizes;
	    break;
	default:
	    /* THIS CAN'T HAPPEN */
	    printf("\n(spt) - unknown controller type!\n");
	    return(1);
	}
	if(nbpc)	/* get partition sizes, unless this is MSCP disk */
	    for(i=0; i<8; i++) {
		if(dsp->nblocks) {
		    pt_info[i].pt_sb = (long)dsp->cyloff * (long)nbpc;
		    pt_info[i].pt_nb = dsp->nblocks;
		}
		dsp++;
	    }
	return(0);
}

/*
 * Return a pointer to a special device name string.
 * RAW /dev/r??## or /dev/r??#
 * BLK /dev/??## or /dev/??#
 */

char	dsfname[15];

dsfn(mode, unit, fs, dip)
register struct di_info *dip;
int	mode, unit, fs;
{
	/* Non-Partitioned (RX02 or RK05) */
	if(dt_info[dip->di_dt[unit]].dt_flags&DI_NPD) {
	    /* if RX02 and not single density, bump unit number by 2 */
	    if((dt_info[dip->di_dt[unit]].dt_type == RX02) && (hx_single==0))
		unit += 2;	/* use double density */
	    sprintf(dsfname, "/dev%s%s%d", (mode==RAW) ? "/r" : "/",
		dip->di_name, unit);
	}

	/* change RX33/RX50 /dev/rd? to /dev/rx? */
	else if(dt_info[dip->di_dt[unit]].dt_flags&DI_RX50)
	    sprintf(dsfname, "/dev%s%s%d", (mode==RAW) ? "/r" : "/",
		"rx", unit);

	/* MSCP */
	else if(dip->di_flags&DI_MSCP)
	    sprintf(dsfname, "/dev%s%.2s%d%d", (mode==RAW) ? "/r" : "/",
		dt_info[dip->di_dt[unit]].dt_lname, unit, fs);

	/* all others */
	else
	    sprintf(dsfname, "/dev%s%s%d%d", (mode==RAW) ? "/r" : "/",
		dip->di_name, unit, fs);
	return(&dsfname);
}
