static char Sccsid[] = "@(#)setup.c	3.3	10/14/87";

/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * TODO:
 *
 * [n]	Need lp, mem, null, kmem, console, du & other oddballs!
 *
 * [n]	Convert all error messages to use perror()!!!!!!
 *
 * [n]	When to remove the BOOT floppy?
 *
 * [n]	Look at all fatal error exits (see if they could be warnings).
 *
 * [NO]	Chmod 400 /dev/??07 and /dev/r??07 (except RA) -- DOES NO GOOD!
 *
 * [n]	Test RP02/3 by configuring driver and faking drives present!
 *
 * [n]	Test interruptablity of each setup phase.
 *
 * [y]	Mount /usr/once, dismount on exit (need usr mnt flag for error).
 *
 * [y]	Write setup info back to setup.info if user enters it.
 *
 * [NO]	Print warning: do not remove setup.info, setup, etc. files.
 *	Handle in installation guide.
 *
 * [n]	Test hz, timezone, dstflag loading in /dev/mem and /unix.
 *	tested ok on 0431 kernel, not 0430.
 *
 * [n]	Must be in single-user mode!
 *
 * [y]	Need to replace ??unixes with a generic kernel.
 *
 * [y]	Need new disk partitions and standard root/swap/elog layouts.
 *
 * [y]	Handle case where target cpu not current cpu, split I/D space
 *	or not is the big issue.
 *
 * [y]	Save sid/* and nsid/* if target cpu not current cpu. -- TESTED
 *
 * [n]	Need TK50 supported as load device.
 *
 * [n]	Convert rxunit to rxflag (like setup53).
 *
 * [y]	Blast fstab.
 *
 * [n]	Make setup restartable to do phase 2 (setup.info).
 */
/*
 *
 *  File name:
 *
 *	setup.c
 *
 *  Source file description:
 *
 *	ULTRIX-11 initial setup program.
 *
 *	TBS, when I figger it out!
 *	need comment format of setup.info file
 *
 *		multiple versions, RX, MT, others????
 *		do more work for users
 *		even more on Micro/PDP-11s
 *
 *  Functions:
 *
 *	main()		The main is the main!
 *
 *	pmsg()		Prints a message from an array of character pointers.
 *
 *	yes()		Return YES, NO, HELP, depending on how the user
 *			answered a question.
 *
 *	intr()		Handles interrupt signal (<CTRL/C>).
 *
 *	retry()		Ask the user if a failed function should be retried.
 *
 *	iflop()		Instruct the user to insert the named floppy into
 *			one of the floppy disk drives.
 *
 *	rflop()		Instruct the user to remove the named floppy from
 *			one of the floppy disk drives.
 *
 *	prtc()		Handles "Press <RETURN> to continue: "
 *
 *	OTHERS?
 *
 *  Usage:
 *
 *	cd /.setup; setup
 *
 * 	The setup and setup.ifno files must not be removed.
 *	The above commands are executed by the root .profile when the
 *	system is first booted, after loading by sdload. The root .profile
 *	is replaced after setup has completed its work, and the setup
 *	program is removed. This is so that all subsequent boots be normal.
 *
 *  Compile:
 *
 *	cd /usr/sys/distr; make setup
 *
 *  Modification History:
 *
 *	18 January 1984
 *		File created -- Fred Canter
 *
 *	?? ??????? 198?
 *		Time passes -- Fred Canter
 *		Many changes, see sccs prt setup.c setup53.c
 *
 *	13 March 1985
 *		Version 2.1 -- Fred Canter
 *		Combined setup.c and setup53.c to form the new setup.c.
 *		Many changes for new kit layout and installation flow.
 *		Added setup_osl to handle loading optional software.
 *
 *	12 May 1985
 *		Version 2.2 -- Fred Canter
 *		Clean up setup and finalize installation flow.
 *		Take advantage of magtape in generic kernel.
 *		Handle install on other than target processor.
 *		Added #ifdef DEBUG.
 *	TODO: more?
 *
 */

#include <stdio.h>
#include <a.out.h>
#include <signal.h>
#include <setjmp.h>
#include <fstab.h>
#include <sys/param.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <sys/filsys.h>
#include <sys/devmaj.h>
#include <sys/conf.h>
#include <sys/ra_info.h>
#include <sys/hp_info.h>
#include <sys/tk_info.h>
#include <sys/time.h>

/*	#define	DEBUG	1	/* debug flag: turn on debug code */

jmp_buf savej;

struct	nlist	nl[] =
{
	{ "_cputype" },
#define			X_CPUTYPE	0
	{ "_hz" },
#define			X_HZ		1
	{ "_timezon" },
#define			X_TIMEZONE	2
	{ "_dstflag" },
#define			X_DSTFLAG	3
	{ "_nrk" },
#define			X_NRK		4
	{ "_rk_dt" },
#define			X_RK_DT		5
	{ "_nrp" },
#define			X_NRP		6
	{ "_rp_dt" },
#define			X_RP_DT		7
	{ "_nuda" },
#define			X_NUDA		8
	{ "_nra" },
#define			X_NRA		9
	{ "_ra_inde" },
#define			X_RA_INDEX	10
	{ "_ra_ctid" },
#define			X_RA_CTID	11
	{ "_ra_drv" },
#define			X_RA_DRV	12
	{ "_nrl" },
#define			X_NRL		13
	{ "_rl_dt" },
#define			X_RL_DT		14
	{ "_nhp" },
#define			X_NHP		15
	{ "_hp_inde" },
#define			X_HP_INDEX	16
	{ "_hp_dt" },
#define			X_HP_DT		17
	{ "_nhs" },
#define			X_NHS		18
	{ "_hs_dt" },
#define			X_HS_DT		19
	{ "_nhk" },
#define			X_NHK		20
	{ "_hk_dt" },
#define			X_HK_DT		21
	{ "_nkl11" },
#define			X_NKL11		22
	{ "_ndl11" },
#define			X_NDL11		23
	{ "_ndh11" },
#define			X_NDH11		24
	{ "_nuh11" },
#define			X_NUH11		25
	{ "_dz_cnt" },
#define			X_DZ_CNT	26
	{ "_ntty" },
#define			X_NTTY		27
	{ "_ra_mas" },
#define			X_RA_MAS	28
	{ "_ud_size" },
#define			X_UD_SIZES	29
	{ "_rq_size" },
#define			X_RQ_SIZES	30
	{ "_rc_size" },
#define			X_RC_SIZES	31
	{ "_hp_size" },
#define			X_HP_SIZES	32
	{ "_hk_size" },
#define			X_HK_SIZES	33
	{ "_rp_size" },
#define			X_RP_SIZES	34
	{ "_bdevsw" },
#define			X_BDEVSW	35
	{ "_nht" },
#define			X_NHT		36
	{ "_nts" },
#define			X_NTS		37
	{ "_ntm" },
#define			X_NTM		38
	{ "_ntk", },
#define			X_NTK		39
	{ "_tk_ctid" },
#define			X_TK_CTID	40
	{ "_npty" },
#define			X_NPTY		41
	{ "_nmausen" },
#define			X_NMAUSENT	42
	{ "" },
};

/* Structure for Daylight Savings Time information */
struct	dst_table {
	char	*dst_area;	/* Geographical area */
	int	dst_id;		/* Numeric id */
}  dst_table[] = {
	"USA",DST_USA,
	"Australia",DST_AUST,
	"Western Europe",DST_WET,
	"Central Europe",DST_MET,
	"Eastern Europe",DST_EET,
	0
};

#define	YES	1
#define	NO	0
#define	ERR	1
#define	NOERR	0
#define	HELP	2
#define	NOHELP	0
#define	MOUNT	1
#define	UMOUNT	0
#define	FATAL	1
#define	ABORT	2
#define	MSF	0
#define	MKFS	1
#define	BLOCK	0
#define	RAW	1
#define	FST_SCH	0
#define	FST_ENT	1
#define	FST_RMV	2
#define	CONSOLE	1
#define	OTHER	0


#define	LBSIZE	50
char	lbuf[LBSIZE];
#define	SCSIZE	512	/* CAUTION: 512 min size (doubles as disk read buf) */
char	syscmd[SCSIZE];
char	sphase[15];
char	sdname[15];
char	tproc[15];
char	loadev[20];
char	ldunit[10];
char	*info = "/.setup/setup.info";
char	*osload = "/bin/osload";
char	*rclock = "/.setup/rootcmd.lock";
char	*uclock = "/.setup/usrcmd.lock";
char	*homedir = "/.setup";
char	*lpfdir = "lost+found";
char	*confirm = "\nPLEASE CONFIRM: ";
char	*dsk_sfn = "/dev/setup.dsf";
char	*debug = "\nDEBUG: ";

int	firstfs = YES;	/* making very first file system flag */
int	sdtype;		/* Index into sd_info[] entry for system disk */
int	sdcntlr = 0;	/* System disk on controller number */
			/* TODO: 0 for now, how or if to set this? */
int	sdunit;		/* System disk unit number */
			/* TODO: RUX50 - unit could be 0 ---|	*/
int	rxflag;		/* Unit # if load device is floppy, 0 if not */
int	rxunit;
int	rd2 = NO;	/* YES if second RD disk present (changes RX unit #s) */
int	mtflag;		/* Load device is magtape (unit number) */
int	tkflag;		/* set if magtape is really a TK50 */
int	rlflag;		/* load device is RL02 */
int	rcflag;		/* load device is RC25 */

/*
 * System's disk info structure.
 *
 * Disk drive type ID definitions (must be unique):
 */
/* ra_info: RX33 RX50 RD31 RD32 RD51 RD52 RD53 RD54 RC25 RA60 RA80 RA81 */
/* hp_info: RP04 RP05 RP06 RM02 RM03 RM05 ML11 */
#define	RK05	5
#define	RX02	1
#define	RS03	8
#define	RS04	9
#define	RL01	10240
#define	RL02	20480
#define	RK06	0
#define	RK07	02000
#define	RP02	2
#define	RP03	3

#define	SD_SYSDSK	1	/* This is the system disk */
#define	SD_WINIE	2	/* Disk is a winchester (can be system disk) */
#define	SD_USRDSK	4	/* Disk has non-system partition(s) [/user] */
#define	SD_FLOPPY	010	/* Floppy disk (rx33 or rx50) */
#define	SD_SMALL	020	/* SMALL ROOT (rm -rf /sas to make room) */
#define	SD_ML11		040	/* ML11 (no setup user file systems) */

struct sd_info {
	char	*sd_lname;	/* disk type name in lowercase */
	char	*sd_uname;	/* disk type name in uppercase */
	char	*sd_swap;	/* /dev/swap link (0 if can't be system disk) */
	int	sd_type;	/* numeric disk type */
	char	sd_pmask;	/* BIT WISE -- disk partition usage mask */
	char	sd_smask;	/* BIT WISE -- partitions taken by system */
	char	sd_flags;	/* flags is flags */
} sd_info[] = {
	/* DUMMY ENTRY: must be here, see di_info.di_dt[] */
	"",	"",	0,	-1,	0,	0,	0,
	"rx02",	"RX02",	0,	RX02,	0,	0,	SD_FLOPPY,
	"rk05",	"RK05",	0,	RK05,	0,	0,	0,
	"ml11",	"ML11",	0,	ML11,	0,	0,	SD_ML11,
	"rx33",	"RX33",	0,	RX33,	0200,	0,	SD_FLOPPY,
	"rx50",	"RX50",	0,	RX50,	0200,	0,	SD_FLOPPY,
	"rd31",	"RD31",	"rd05",	RD31,	0341,	0141,	SD_WINIE,
	"rd32",	"RD32",	"rd02",	RD32,	0217,	0007,	(SD_WINIE|SD_USRDSK),
	"rd51",	"RD51",	"rd00",	RD51,	0221,	0021,	(SD_WINIE|SD_SMALL),
	"rd52",	"RD52",	"rd02",	RD52,	0217,	0007,	(SD_WINIE|SD_USRDSK),
	"rd53",	"RD53",	"rd02",	RD53,	0217,	0007,	(SD_WINIE|SD_USRDSK),
	"rd54",	"RD54",	"rd02",	RD54,	0217,	0007,	(SD_WINIE|SD_USRDSK),
	"rl01", "RL01", "rl00",	RL01,	0201,	0001,	SD_SMALL,
	"rl02", "RL02", "rl00",	RL02,	0203,	0003,	SD_SMALL,
	"rk06", "RK06", "hk01",	RK06,	0107,	0007,	SD_SMALL,
	"rk07", "RK07", "hk01",	RK07,	0213,	0013,	(SD_USRDSK|SD_SMALL),
	"rp02", "RP02", "rp01",	RP02,	0107,	0007,	SD_USRDSK,
	"rp03", "RP03", "rp01",	RP03,	0213,	0013,	SD_USRDSK,
	"rp04", "RP04", "hp02",	RP04,	0117,	0007,	SD_USRDSK,
	"rp05", "RP05", "hp02",	RP05,	0117,	0007,	SD_USRDSK,
	"rp06", "RP06", "hp02",	RP06,	0377,	0007,	SD_USRDSK,
	"rm02", "RM02", "hp02",	RM02,	0217,	0007,	SD_USRDSK,
	"rm03", "RM03", "hp02",	RM03,	0217,	0007,	SD_USRDSK,
	"rm05", "RM05", "hp02",	RM05,	0377,	0007,	SD_USRDSK,
	"ra60", "RA60", "ra02",	RA60,	0277,	0007,	SD_USRDSK,
	"ra80", "RA80", "ra02",	RA80,	0217,	0007,	SD_USRDSK,
	"ra81", "RA81", "ra02",	RA81,	0377,	0007,	SD_USRDSK,
	"rc25", "RC25", "rc11",	RC25,	0237,	0007,	SD_USRDSK,
	0
};

/*
 * File system table (/etc/fstab) information.
 * Used to blast the initial fstab.
 */

struct fs_info {
	char	*fs_lname;	/* lowercase disk name (same as sd_info) */
	char	*fs_root;	/* disk to mount on root (/) */
	char	*fs_usr;	/* disk to mount on /usr */
} fs_info[] = {
	"rd31",	"rd00",	"rd06",
	"rd32",	"rd00",	"rd01",
	"rd51",	"rd00",	"rd04",
	"rd52",	"rd00",	"rd01",
	"rd53",	"rd00",	"rd01",
	"rd54",	"rd00",	"rd01",
	"rl01",	"rl00",	"rl10",
	"rl02",	"rl00",	"rl01",
	"rk06",	"hk00",	"hk02",
	"rk07",	"hk00",	"hk03",
	"rp02",	"rp00",	"rp02",
	"rp03",	"rp00",	"rp03",
	"rp04",	"hp00",	"hp01",
	"rp05",	"hp00",	"hp01",
	"rp06",	"hp00",	"hp01",
	"rm02",	"hp00",	"hp01",
	"rm03",	"hp00",	"hp01",
	"rm05",	"hp00",	"hp01",
	"ra60",	"ra00",	"ra01",
	"ra80",	"ra00",	"ra01",
	"ra81",	"ra00",	"ra01",
	"rc25",	"rc10",	"rc12",
	0
};

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
#define	DI_RS	020	/* RS03/4 specical case flag */

int	rk_gdti(), rp_gdti(), ra_gdti(), rl_gdti();
int	hx_gdti(), hp_gdti(), hs_gdti(), hk_gdti();

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
	int	di_dt[8];	/* Each unit's type (index into sd_info[]) */
				/* Zero indicates non existent drive */
} di_info[] = {
  { "RK11",	"rk", 0, 0, RK_BMAJ, RK_RMAJ, rk_gdti, DI_NPD },
  { "RP11",	"rp", 0, 0, RP_BMAJ, RP_RMAJ, rp_gdti, 0 },
  { "MSCP",	"ra", 0, 0, RA_BMAJ, RA_RMAJ, ra_gdti, DI_MSCP },
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
 * Number of 512 byte blocks per cylinder
 * for each type of disk.
 * Not affected by 1K block file system.
 */

#define	RP23_BPC	200	/* RP02/3 */
#define	RP456_BPC	418	/* RP04/5/6 */
#define	RM23_BPC	160	/* RM02/3 */
#define	RM5_BPC		608	/* RM05 */
#define	RK67_BPC	66	/* RK06/7 */

/*
 * Disk partition info table.
 * Used to guide user thru disk partition selection.
 */

#define	PT_USER	1	/* partition has file system on it already */
#define	PT_SYS	2	/* partition used by the system */
#define	PT_NUP	4	/* non usable (disk doesn't use this partition) */
#define	PT_OP	010	/* not usable (overlapped by a used partition) */

struct	pt_info {
	daddr_t	pt_sb;		/* starting block number (1KB logical block) */
	daddr_t	pt_nb;		/* length in 1KB logical blocks */
	char	pt_op;		/* BIT WISE -- overlapping partitions */
	int	pt_flags;	/* special info flags */
} pt_info[8];

/*
 * Misc. stuff for setup of user file systems.
 */

int	ufsnum;		/* running count of # user file systems */
char	fsname[20];	/* superblock fsname (also mounted on directory name) */
char	volname[20];	/* superblock volname */
char	mntdir[20];	/* mounted on directory (/user#) */
char	fmntdir[20];	/* possible alternate mounted on directory */
char	bdisk[20];	/* block special file name (/dev/??##) */
char	rdisk[20];	/* raw special file name (/dev/r??##) */

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
 * Note: MSCP drive type info arrays are non-semetrical
 *	 ra_index must be used to access drive type info.
 */

int	nuda;		/* Number of configured MSCP controllers */
char	nra[MAXUDA];	/* Number of units configured on each controller */
char	ra_ctid[MAXUDA];/* Controller type ID and micro-code rev level */
char	ra_index[MAXUDA];/* Index into drive type info ra_drv[] */
struct	ra_drv ra_drv[MAXUDA*8];	/* RA drive info (see ra_info.h) */
char	rq_dt[8];	/* RD/RX disk drive types for iflop() & rflop() */
			/* Set up by ra_gdti() */

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

/*
 * Kernel data symbols used to determine number
 * and type of magtape controllers configured.
 */

int	nht;		/* HT - TM02/3 */
int	nts;		/* TS - TS11/TU80/TSV05/TK25 (TK25 no longer supported) */
int	ntm;		/* TM - TM11 */
int	ntk;		/* TK - TK50/TU81 */
char	tk_ctid[MAXTK];	/* TK - TU81/TK50 */

/*
 * Kernel data symbols used to find comm. devices.
 */

int	ntty;		/* Total number of TTY structures */
int	nttys;		/* Number of TTY structures used so far */
int	nkl11;		/* Number of KL lines (including console) */
int	ndl11;		/* Number of DL lines */
int	ndh11;		/* Number of DH lines */
int	nuh11;		/* Number of DHU/DHV lines */
int	dz_cnt;		/* Number of DZ/DZV/DZQ lines */
char	*dl_name;	/* Set to DL11 or DLV11 */
char	*uh_name;	/* Set to DHU11 or DHV11 */
char	*dz_name;	/* Set to DZ11 or DZV11 (DZQ11 is out of luck) */
int	dz_lpu;		/* DZ/DZV lines per unit */
int	uh_lpu;		/* DHU/DHV lines per unit */
int	dh_lpu = 16;	/* DH lines per unit */

/*
 * Number of pseudo ttys and
 * maus segments.
 */

int	npty;
int	nmausent;

/*
 * Supported processor table.
 */
#define	NSID	0
#define	SID	1
#define	QBUS	0
#define	UBUS	1
int	cpi;			/* current processor index into cputyp[] */
int	tpi;			/* target processor index into cputyp[] */
int	ontarget;		/* on the target processor ? */
struct cputyp {
	int	p_type;		/* processor type */
	int	p_sid;		/* separate I & D space ? */
	int	p_bus;		/* Qbus or Unibus */
} cputyp[] = {
	23,	NSID,	QBUS,
	24,	NSID,	UBUS,
	34,	NSID,	UBUS,
	40,	NSID,	UBUS,
	44,	SID,	UBUS,
	45,	SID,	UBUS,
	53,	SID,	QBUS,
	55,	SID,	UBUS,
	60,	NSID,	UBUS,
	70,	SID,	UBUS,
	73,	SID,	QBUS,
	83,	SID,	QBUS,
	84,	SID,	UBUS,
	0
};
int	cputype;	/* current processor type */
int	hz;		/* line frequency */
int	tzone;		/* timezone (minutes behind GMT) */
int	dstflag;	/* daylight savings time flag */

/*
 * Variables used to overlay the new values of
 * hz, timezone, and dstflag onto the kernel.
 * Both memory (/dev/mem) and a.out (/??unix)
 * values are replaced.
 */

int	magic;		/* kernel magic number (430, 431, 450, 451) */
int	novseg;		/* number of overlay segments (8 or 32) */
unsigned int txtsiz;	/* size of root text segment */
unsigned int ovsz[16];	/* size of each overlay */
long	ovsize;		/* total size of all overlays */
long	tsoff;		/* offset to text (32 or 48) */
long	dsoff;		/* offset to data space in a.out file */
long	hzoff;		/* offset to hz */
long	tzoff;		/* offset to timezone */
long	dstoff;		/* offset to dstflag */

int	ab_flag;	/* abort flag, set by intr() */

/*
 * Some stuff for symbolic link of /usr/spool.
 * Remainder local vairables in main().
 */

#define	SL_MAXPL 50		/* MAX pathname length */

char	*sl_usd = "/usr/spool";

/*
 * Setup phase definitions:
 *
 * Phase 0 - cold start, first time setup runs after sdload.
 * Phase 1 - doing initial setup, read of setup.info successful.
 * Phase 2 - initial setup complete, setup restarted after sysgen.
 * Phase 3 - setup completed, customer wants to change one or more items.
 *
 * The setup phase is stored in the first byte of the setup.info
 * file. The sdload program initializes the phase to zero.
 */
#define	SUP0	0
#define	SUP1	1
#define	SUP2	2
#define	SUP3	3

int	phase;

main()
{
	int	intr();
	char	*asktt();
	register struct sd_info *sdp;
	struct	di_info *dip;
	register int i, j;
	int	k;
	int	mem, fd;
	FILE	*fi, *fo;
	int	fsp;
	daddr_t	fsz;
	int	crt;
	int	skipit;
	int	sifdirty;
	long	soff;
	int	*bip;
	int	nunits;
	/* /usr/spool symbolic link stuff */
	int	sl_islink;
	char	sl_src[SL_MAXPL+1];
	char	sl_dst[SL_MAXPL+1];
	char	sl_bdn[SL_MAXPL+1];
	char	sl_sbdn[DIRSIZ+1];
	char	sl_rlb[BUFSIZ];
	char	*p;
	struct	stat	statb;

	mem = -1;	/* supdone() needs to if mem open */
	crt = -1;	/* say we don't know console terminal type */
	ab_flag = 0;
	setjmp(savej);
	if(ab_flag) {
	    printf("\n\nSETUP PROGRAM interrupted via <CTRL/C>!\n");
	    printf("\nEXIT SETUP (y to exit, n to restart setup) <y or n> ? ");
	    if(yes(NOHELP) == YES)
		su_err(0);
	    ab_flag = 0;
	}
	signal(SIGINT,	intr);
/*
 * The /.profile (calls setup) does stty prterase, so
 * the terminal will behave like a printing terminal until
 * we ask the user if it is a CRT.
 *
 * Sneek a peek at the setup.info file, just to get the
 * phase number. If we can't get the phase number, set it
 * to zero (real read of setup.info will fail later).
 * Knowing the phase number in advance helps taylor setup's
 * operation and makes phase 3 less of a pain for the user,
 * and after all the user is paying the bills!
 */
	phase = SUP0;
	if((fi = fopen(info, "r")) != NULL) {
	    if(fgets(sphase, 10, fi) != NULL) {
		if(sphase[1] == '\n')
		    phase = sphase[0] - '0';
	    }
	    fclose(fi);
	}
	if((phase < 0) || (phase > 3))
	    phase = SUP0;
	if(phase < SUP3) {
	    phelp("sum1");
	    phelp("sum2");
	    prtc();
	} else {
	    phelp("sum1");
	    if(access("/usr/bin", 0) == 0) {
		phelp("mu_warn");
		printf("\n\nIs the system in multiuser mode <y or n> ? ");
		if(yes(NOHELP) == YES)
		    exit(1);
	    }
	    printf("\n\n\nPrint instructions <y or n> ? ");
	    if(yes(NOHELP) == YES) {
		phelp("sum2");
		prtc();
	    }
	}
/*
 * Unless in phase 3,
 * ask if the console is a CRT and do
 * stty accordingly.
 */
	if(phase < SUP3)
	    crt = gcrt();
/*
 * Get the setup phase, system disk, target CPU type,
 * and load device type
 * from /.setup/setup.info and validate info.
 * If they are not valid, ask the user for them.
 */
	printf("\n****** READING SETUP DATA FROM setup.info FILE ******\n");
	sifdirty = NO;
	while(1) {
	    if((fi = fopen(info, "r")) == NULL) {
		printf("\nCan't open %s file!\n", info);
		sifdirty = YES;
		break;
	    }
	    if(fgets(sphase, 10, fi) == NULL) {
		printf("\nCan't get setup phase from %s!\n", info);
		sifdirty = YES;
		break;
	    }
	    zapnl(&sphase);		/* change newline to null */
	    if(fgets(sdname, 10, fi) == NULL) {
		printf("\nCan't get system disk type from %s!\n", info);
		sifdirty = YES;
		break;
	    }
	    zapnl(&sdname);		/* change newline to null */
	    if(fgets(tproc, 10, fi) == NULL) {
		printf("\nCan't get target CPU type from %s!\n", info);
		sifdirty = YES;
		break;
	    }
	    zapnl(&tproc);		/* change newline to null */
	    if(fgets(loadev, 15, fi) == NULL) {
		printf("\nCan't get load device name from %s!\n", info);
		sifdirty = YES;
		break;
	    }
	    zapnl(&loadev);		/* change newline to null */
	    rcflag = -1;
	    rlflag = -1;
	    mtflag = -1;
	    tkflag = 0;
	    rxflag = 0;
	    if((strncmp(loadev, "rx", 2) == 0) &&
	       (loadev[3] > '0') && (loadev[3] <= '3')) {
			rxflag = 1;
			rxunit = loadev[3] - '0';
	    } else if((strncmp(loadev, "ht", 2) == 0) ||
		      (strncmp(loadev, "ts", 2) == 0) ||
		      (strncmp(loadev, "tm", 2) == 0))
			mtflag = (loadev[3] - '0') & 3;
	    else if(strncmp(loadev, "tk", 2) == 0) {
			mtflag = (loadev[3] - '0') & 3;
			tkflag = 1;
	    } else if(strncmp(loadev, "rl", 2) == 0)
			rlflag = loadev[3] - '0';
	    else if(strncmp(loadev, "rc", 2) == 0)
			rcflag = loadev[3] - '0';
	    else {
			printf("\n(%s) - bad load device!\n", loadev);
			sifdirty = YES;
			break;
	    }
	    fclose(fi);
	    for(i=0; sd_info[i].sd_lname; i++)
		if(strcmp(sdname, sd_info[i].sd_lname) == 0)
		    break;
	    if(sd_info[i].sd_lname == 0) {
		printf("\n(%s) - bad system disk type!\n", sdname);
		sifdirty = YES;
		break;
	    }
	    sd_info[i].sd_flags |= SD_SYSDSK;
	    sdunit = sd_info[i].sd_swap[2] - '0';
	    sdtype = i;
	    phase = sphase[0] - '0';
	    if((phase < 0) || (phase > 3)) {
		phase = 0;
		printf("\n(%s) - bad setup phase number!\n", sphase);
		sifdirty = YES;
		break;
	    }
	    if(rlflag >= 0)
		break;
	    if(rcflag >= 0)
		break;
	    for(i=0, j=atoi(tproc); cputyp[i].p_type; i++)
		if(cputyp[i].p_type == j)
		    break;
	    if(cputyp[i].p_type == 0) {
		printf("\n(%s) - bad target CPU type!\n", tproc);
		sifdirty = YES;
		break;
	    }
	    tpi = i;	/* save index to target processor type */
	    break;
	}
usr_info:
	if(sifdirty == YES) {
	    if(fi != NULL)
		fclose(fi);
	    phelp("usrinfo");
u_info:
	    printf("\nContinue the installation <y or n> ? ");
	    if(yes(NOHELP) != YES)
		su_err(ABORT);
	    sprintf(&loadev, "rx(0,0)boot");	/* for write to setup.info */
/*
 * Ask the user what phase to enter.
 * This is risky, but it should not be a
 * problem unless setup.info gets blown away!
 */
	    while(1) {
		phase = 0;
		do
		    printf("\nSetup phase number (? for help) < 1 2 3 > ? ");
		while(getline("h_pn") <= 0);
		phase = atoi(lbuf);
		if((phase <= 0) || (phase > 3)) {
		    printf("\n(%s) - bad setup phase number!\n", lbuf);
		    continue;
		}
		break;
	    }
	    while(1) {
		rxflag = 0;
		mtflag = -1;
		tkflag = 0;
		rlflag = -1;
		rcflag = -1;
		printf("\nWas system loaded from RX50 diskettes <y or n> ? ");
		if(yes(NOHELP) == YES) {
		    while(1) {
			do
			    printf("\nFloppy disk unit number < 1 2 3 > ? ");
			while(getline("h_rxldun") <= 0);
			if((strlen(lbuf) != 1) ||
			   (lbuf[0] < '1') ||
			   (lbuf[0] > '3')) {
			    printf("\n(%s) - Bad unit number!\n", lbuf);
			    continue;
			}
			rxflag = 1;
			rxunit = lbuf[0] - '0';
			loadev[3] = lbuf[0];
			break;
		    }
		break;
		}
		printf("\nWas system loaded from TK50 <y or n> ? ");
		if(yes(NOHELP) == YES) {
		    mtflag = 0;		/* assume unit zero */
		    tkflag = 1;
		    loadev[0] = 't';
		    loadev[1] = 'k';
		    break;
		}
		printf("\nWas system loaded from MAGTAPE <y or n> ? ");
		if(yes(NOHELP) == YES) {
		    mtflag = 0;
		    while(1) {
			do
			    printf("\nMagtape controller type < tm ht ts > ? ");
			while(getline("h_mtldct") <= 0);
			if((strlen(lbuf) != 2) ||
			   ((strcmp("ht", lbuf) != 0) &&
			    (strcmp("ts", lbuf) != 0) &&
			    (strcmp("tm", lbuf) != 0))) {
			    printf("\n(%s) - Invalid cntlr type!\n", lbuf);
			    continue;
			}
			loadev[0] = lbuf[0];
			loadev[1] = lbuf[1];
			break;
		    }
		    break;
		}
		printf("\nWas system loaded from RL02 disk pack <y or n> ? ");
		if(yes(NOHELP) == YES) {
		    rlflag = 0;
		    loadev[1] = 'l';
		    break;
		}
		printf("\nWas system loaded from RC25 disk pack <y or n> ? ");
		if(yes(NOHELP) == YES) {
		    rcflag = 0;
		    loadev[1] = 'c';
		    break;
		}
	    }
usr_sdt:
	    while(1) {
		printf("\nPlease enter the generic system disk name.\n");
		printf("\nThese disks are supported:\n");
		for(j=0, i=0; sd_info[i].sd_lname; i++) {
		    if(sd_info[i].sd_swap == 0)
			continue;	/* can't be system disk */
		    if((j & 7) == 0)
			printf("\n        ");
		    printf("%s ", sd_info[i].sd_lname);
		    j++;
		}
		printf("\n\nSystem Disk Type: ");
		    for(i=0; i<10; i++) {
			sdname[i] = getchar();
			if(sdname[i] == '\n') {
			    sdname[i] = '\0';
			    break;
			}
		    }
		for(i=0; sd_info[i].sd_lname; i++)
		    if(strcmp(sdname, sd_info[i].sd_lname) == 0)
			break;
		if(sd_info[i].sd_lname == 0) {
		    printf("\n(%s) - bad system disk!\n", sdname);
		    continue;
		}
		sd_info[i].sd_flags |= SD_SYSDSK;
		sdunit = sd_info[i].sd_swap[2] - '0';
		sdtype = i;
		break;
	    }
usr_cpt:
	    if((rlflag < 0) && (rcflag < 0)) {
		while(1) {
		    phelp("tcpu");
		    printf("\n\nTarget Processor Type: ");
		    for(i=0; i<10; i++) {
			tproc[i] = getchar();
			if(tproc[i] == '\n') {
			    tproc[i] = '\0';
			    break;
			}
		    }
		    for(i=0, j=atoi(tproc); cputyp[i].p_type; i++)
			if(cputyp[i].p_type == j)
			    break;
		    if(cputyp[i].p_type == 0) {
			printf("\n(%s) - bad target CPU type!\n", tproc);
			continue;
		    }
		    tpi = i;
		    break;
		}
	    }
	}
info_ok:
/*
 * Get current CPU type and lots of other
 * needed info from the kernel.
 */
	if(nlist("/unix", nl) < 0) {
		printf("\nCan't access namelist in /unix!\n");
		su_err(FATAL);
	}
	if((nl[X_CPUTYPE].n_value == 0) ||
	   (nl[X_HZ].n_value == 0) ||
	   (nl[X_TIMEZONE].n_value == 0) ||
	   (nl[X_DSTFLAG].n_value == 0)) {
		printf("\nCan't get needed symbol values from ");
		printf("/unix namelist!\n");
	        su_err(FATAL);
	}
	if((mem = open("/dev/mem", 2)) < 0) {
		printf("\nCan't open memory (/dev/mem)!\n");
		su_err(FATAL);
	}
	lseek(mem, (long)nl[X_CPUTYPE].n_value, 0);
	read(mem, (char *)&cputype, sizeof(cputype));
	for(cpi=0; cputyp[cpi].p_type; cpi++)
		if(cputyp[cpi].p_type == cputype)
			break;
	if(rxflag || (rlflag >= 0) || (rcflag >= 0)) {
		sprintf(tproc, "%d", cputype);
		tpi = cpi;
	}
/*
 * Phase 0 is not visable to the user, it is
 * only internal to setup.
 */
	printf("\nSETUP PHASE      = %d", (phase == 0) ? 1 : phase);
	switch(phase) {
	case SUP0:
	case SUP1:
		printf(" (Initial Setup)");
		break;
	case SUP2:
		printf(" (Final Setup)");
		break;
	case SUP3:
		printf(" (Change Setup)");
		break;
	/* NO DEFAULT: phase checked before we get here */
	}
	printf("\nLOAD DEVICE TYPE = ");
	if(rxflag)
		printf("FLOPPY DISK UNIT %d", rxunit);
	else if(rlflag >= 0)
		printf("RL02 UNIT %d", rlflag);
	else if(rcflag >= 0)
		printf("RC25 UNIT %d", rcflag);
	else if(tkflag)
		printf("TK50 UNIT %d", mtflag);
	else
		printf("MAGTAPE UNIT %d", mtflag);
	printf("\nSYSTEM DISK TYPE = %s", sd_info[sdtype].sd_uname);
	printf("\nCURRENT CPU TYPE = 11/%d", cputype);
	printf("\nTARGET  CPU TYPE = 11/%s", tproc);
	printf("\n\nIs the above information correct <y or n> ? ");
	if(yes(NOHELP) != YES) {
		phelp("info_err");
		goto u_info;
	}
/*
 * If we had to ask the user for the info that should
 * have come from setup.info, write the user entered
 * info back to the setup.info file. So, it will be
 * correct next time (most likely in setup phase 3).
 */
	if(sifdirty == YES) {
	    while(1) {
		if((fo = fopen(info, "w")) == NULL) {
		    printf("\nWARNING: can't create %s file!\n", info);
		    break;
		}
		fprintf(fo, "%d\n%s\n%d\n%s\n", phase,
		    sd_info[sdtype].sd_lname, cputyp[tpi].p_type, loadev);
		fclose(fo);
		break;
	    }
	}
/*
 * Finally know what phase we are really in!
 * Modify setup.info to current phase.
 */
	if((phase == SUP0) || ((phase == SUP1) && sifdirty)) {
/*
 * Arrange for a copy of the generic kernel to be
 * preserved in case of disaster.
 * Do this on first call of setup only!
 */
		if(access("/gunix", 0) != 0)
		    link("/unix", "/gunix");
		sync();
		phase = SUP1;
		wrtpn(phase);
	}
	printf("\n****** ENTERING SETUP PHASE %d ******\n", phase);
/*
 * If the system disk has a small root (SD_SMALL),
 * RD51, RL01, RL02, RK06, RK07,
 * remove the /sas directory (unload saprog).
 * The /sas files are always loaded from magtape or TK50,
 * this frees up some space in a tight root. User can re-load
 * saprog with osload if he/she really wants it.
 */
	if((phase == SUP1) && (sd_info[sdtype].sd_flags&SD_SMALL))
	    system("rm -rf /sas");
/*
 * If target CPU type equals the current CPU type,
 * ask the user if this is the target CPU.
 * If they are not equal, we know it is not the target CPU.
 * Hate to ask the user, but it is a critical peice of info
 * and there is no other way to get it!
 * NOTE: If not loading from a magtape kit, must be on target CPU!
 */
	if(phase == SUP1) {
	    if(tpi != cpi) {
		if(mtflag < 0) {
		    printf("\nNOT LOADING FROM MAGTAPE (or TK50): ");
		    printf("must install on target processor!\n");
		    su_err(FATAL);
		} else
		    ontarget = NO;
	    } else {
		while(1) {
		    if(mtflag < 0) {
			ontarget = YES;
			break;
		    }
		    printf("\nIs this the target processor ");
		    printf("(? for help) <y or n> ? ");
		    j = yes(HELP);
		    if(j == HELP) {	
			phelp("h_otp");
			continue;
		    }
		    ontarget = j;
		    printf("%s%sthe target processor <y or n> ? ",
			confirm, (j == YES) ? "on " : "not on ");
		    if(yes(NOHELP) != YES)
			continue;
		    break;
		}
	    }
	} else
	    ontarget = YES;	/* all other phase must be done on target CPU */
/*
 * Blast temporary /etc/ttys and /etc/ttytype files
 * containing entries for the console terminal only.
 * The real files will be blasted after the new kernel
 * is installed and we know what devices are configured.
 */
	if(phase < SUP2)
	    if(bttys(crt, 0, 0) < 0)
		su_err(FATAL);
/*
 * If phase 3, ask user if console terminal type
 * needs to be changed (CRT/PRT).
 */
	if(phase == SUP3) {
	    printf("\nChange console terminal type (CRT vs HARDCOPY) <y or n> ? ");
	    if(yes(NOHELP) == YES)
		crt = gcrt();
	}

/*
 * Ask for info on hz, timezone, & dstflag.
 */
	switch(phase) {
	case SUP0:
	case SUP1:
		skipit = NO;
		break;
	case SUP2:
		skipit = YES;
		break;
	case SUP3:
		printf("\nChange line frequency, timezone, or daylight ");
		printf("savings time <y or n> ? ");
		if(yes(NOHELP) == YES)
		    skipit = NO;
		else
		    skipit = YES;
		break;
	}
g_hz:
	while(1) {
	    if(skipit == YES)
		break;
	    do {
	    	printf("\nWhat is your AC power line frequency in hertz");
	    	printf(" < 50 or 60 > ? ");
	    } while(getline("h_hz") <= 0);
	    hz = atoi(lbuf);
	    if((hz != 50) && (hz != 60))
		printf("\nLine frequency should be 50 or 60 hertz!\n");
	    printf("%sAC line frequency is %d hertz <y or n> ? ", confirm, hz);
	    if(yes(NOHELP) != YES)
		continue;
	    break;
	}
g_tz:
	while(1) {
	    if(skipit == YES)
		break;
	    do {
	    	printf("\nWhat is your local time zone ");
	    	printf("< hours west/behind GMT > ? ");
	    } while(getline("h_tz") <= 0);
	    tzone = atoi(lbuf);
	    if((tzone < 0) || (tzone > 23)) {
	    	printf("\n%d - invalid time zone!\n", tzone);
		continue;
	    }
	    printf("%stime zone is %d hours west/behind GMT <y or n> ? ",
		confirm, tzone);
	    if(yes(NOHELP) != YES)
		continue;
	    tzone *= 60;
	    break;
	}
g_dst:
	while(1) {
	    if(skipit == YES)
		break;
	    printf("\nDoes your local area use daylight savings time <y or n> ? ");
	    j = yes(HELP);
	    if(j == HELP) {
	    	phelp("h_dst");
		continue;
	    }
	    if(j == YES) {
	    	dstflag = 1; 
	    }
	    else
	    	dstflag = 0;
	    printf("%sdaylight savings time%sin use <y or n> ? ",
	    	confirm, dstflag ? " " : " not ");
	    if(yes(NOHELP) != YES)
		continue;
	    break;
	}
	if(dstflag) 
	    while(1) {
		dstflag = get_dst();  /* Get geographic area */
		printf("%s Geographic area is %d <y or n> ?",confirm,dstflag);
		if(yes(NOHELP) != YES)
		    continue;
		break;
	    }
/*
 * Overlay new HZ, TIMEZONE, and DSTFLAG
 * onto kernel a.out file (/unix).
 */
	if(skipit == NO) {
	    if((fd = open("/unix", 2)) < 0) {
	    	printf("\nCan't open /unix for read/write!\n");
	    	su_err(FATAL);
	    }
	    read(fd, (char *)&magic, sizeof(magic));
	    switch(magic) {
	    case 0430:
	    case 0431:
		novseg = 8;
		tsoff = 32L;
		break;
	    case 0450:
	    case 0451:
		novseg = 16;
		tsoff = 48L;
		break;
	    default:
	    	printf("\nType %o kernels not supported!\n", magic);
	    	su_err(FATAL);
	    }
	    read(fd, (char *)&txtsiz, sizeof(txtsiz));
	    lseek(fd, 16L, 0);
	    read(fd, (char *)&ovsz, sizeof(ovsz));
	    ovsize = 0;
	    for(i=1; i<novseg; i++)
	    	ovsize += ovsz[i];
	    dsoff = (long)tsoff + (long)txtsiz + (long)ovsize;	/* data space offset */
	    hzoff = dsoff + (long)nl[X_HZ].n_value;
	    tzoff = dsoff + (long)nl[X_TIMEZONE].n_value;
	    dstoff = dsoff + (long)nl[X_DSTFLAG].n_value;
	    if((magic == 0430) || (magic == 0450)) {
		hzoff -= 060000L;
		tzoff -= 060000L;
		dstoff -= 060000L;
	    }
#ifdef	DEBUG
	    printf("%shzoff=%O tzoff=%O dstoff=%O\n", debug, hzoff, tzoff, dstoff);
#endif	DEBUG
	    lseek(fd, (long)hzoff, 0);
	    write(fd, (char *)&hz, sizeof(hz));
	    lseek(fd, (long)tzoff, 0);
	    write(fd, (char *)&tzone, sizeof(tzone));
	    lseek(fd, (long)dstoff, 0);
	    write(fd, (char *)&dstflag, sizeof(dstflag));
	    close(fd);
	    sync();
/*
 * Overlay new HZ, TIMEZONE, and DSTFLAG
 * onto the running kernel (/dev/mem).
 */
	    lseek(mem, (long)nl[X_HZ].n_value, 0);
	    write(mem, (char *)&hz, sizeof(hz));
	    lseek(mem, (long)nl[X_TIMEZONE].n_value, 0);
	    write(mem, (char *)&tzone, sizeof(tzone));
	    lseek(mem, (long)nl[X_DSTFLAG].n_value, 0);
	    write(mem, (char *)&dstflag, sizeof(dstflag));
	}
/*
 * Set the date and time.
 */
g_date:
	while(1) {
	    if((phase != SUP3) || ((phase == SUP3) && (skipit == NO))) {
		do {
		    printf("\nPlease enter the current date/time ");
		    printf("< yymmddhhmm.ss > ? ");
		} while(getline("h_date") <= 0);
		printf("\n");
		sprintf(syscmd, "date %s", lbuf);
		if(system(syscmd) != 0) {
		    printf("\nAttempt to set date/time failed!\n");
		    continue;
		}
	    } else {
		printf("\n");
		system("date");
	    }
	    printf("%sdate/time correct <y or n> ? ", confirm);
	    if(yes(NOHELP) != YES) {
		skipit = NO;
		continue;
	    }
	    sync();
	    break;
	}
/*
 * Select split or non-split I/D space commands in /bin.
 * The selection will be made twice if the current CPU has
 * split I/D space and the target CPU does not, or visa versa.
 */
sel_bin:
	while(1) {
	    if(phase != SUP1)
		break;
	    if(access(rclock, 0) == 0)
		break;		/* /bin commands already selected */
	    printf("\n****** SELECTING%sSPLIT I/D COMMANDS (/bin) ******\n",
		(cputyp[cpi].p_sid == SID) ? " " : " NON-");
	    if(chngdir("/bin"))
	    	su_err(FATAL);
#ifdef	DEBUG
	    printf("%s", debug);
	    fflush(stdout);
	    system("ls sid");	/* sid/nsid contain same files */
#endif	DEBUG
	    if(cputyp[cpi].p_sid == SID)
	    	sprintf(syscmd, "cp sid/* .");
	    else
	    	sprintf(syscmd, "cp nsid/* .");
	    if(system(syscmd) != 0) {
	    	printf("\nCan't `%s' in /bin!\n", syscmd);
	    	su_err(FATAL);
	    }
	    sync();
#ifdef	DEBUG
	    printf("%s/lib/c2\n", debug);
#endif	DEBUG
	    system("rm -f /lib/c2");
	    if(cputyp[cpi].p_sid == SID)
	    	sprintf(syscmd, "ln /lib/c2_id /lib/c2");
	    else
	    	sprintf(syscmd, "ln /lib/c2_ov /lib/c2");
	    if(system(syscmd) != 0) {
	    	printf("\nCan't `%s'!\n", syscmd);
	    	su_err(FATAL);
	    }
	    sync();
	    if(chngdir(homedir))
		su_err(FATAL);
	    if(cputyp[cpi].p_sid == cputyp[tpi].p_sid) { /* selection final */
		creat(rclock, 0444);			 /* create lock file */
		sync();
	    }
	    break;
	}
/*
 * Remove the alternate commands directories (/bin/sid & /bin/nsid).
 * ONLY if the lock file exists (final selection made).
 */
	while(1) {
	    if(phase != SUP1)
		break;
	    if(access(rclock, 0) != 0)
		break;		/* final selection not made yet */
	    if((access("/bin/sid", 0) != 0) && (access("/bin/nsid", 0) != 0))
		break;		/* /bin commands already removed */
#ifdef	DEBUG
	    printf("%sremove /bin/sid and /bin/nsid <y or n> ? ", debug);
	    if(yes(NOHELP) != YES)
		break;
#endif	DEBUG
	    system("rm -r /bin/sid /bin/nsid");
	    sync();
	    break;
	}
/*
 * Zero the error log file.
 */
	if(phase == SUP1) {
	    printf("\n****** ZEROING THE ERROR LOG FILE ******\n");
	    if(system("/etc/eli -f > /dev/null 2>&1") != 0) {
	    	printf("\nCan't zero the error log file\n");
	    	su_err(FATAL);
	    }
	}
/*
 * Make device special files.
 * If in phase 1, only make special files
 * for drives on the system disk controller and
 * magtapes (if any).
 * Also, take care of the work that used to be
 * done by /dev/makefile, i.e., system disk special files,
 * chmod 400 partition 7 (sometimes 6), /dev/swap file,
 * basic /etc/fstab, and boot block (redundant).
 *
 * If phase 2, make all special files.
 * If phase 3, ask user if we need to make special files.
 * NOTE: Even if the users says no, we go thru the motions of
 *	 making the special files, because the system config info
 *	 gathered is needed by some of the following steps.
 */
	switch(phase) {
	case SUP0:
	case SUP1:
	case SUP2:
		skipit = NO;
		break;
	case SUP3:
		while(1) {
		    printf("\nChange device special files <y or n> ? ");
		    j = yes(HELP);
		    if(j == YES)
			skipit = NO;
		    else if(j == NO)
			skipit = YES;
		    else if(j == HELP) {
			phelp("h_csf");
			continue;
		    }
		    break;
		}
		break;
	/* DEFAULT CASE NOT NEEDED */
	}
	if(chngdir("/dev"))
	    su_err(FATAL);
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
	while(1) {
	    if((phase > SUP1) && (skipit == NO)) {
		if(phase == SUP2) {
		    phelp("msf_rec");
		    prtc();
		}
		printf("\n****** DETERMINING SYSTEM'S DISK CONFIGURATION ******\n");
	    }
	    for(dip=di_info; dip->di_typ; dip++) {
/* 1 */		soff = (long)nl[X_BDEVSW].n_value +
		   (long)(sizeof(struct bdevsw) * dip->di_bmaj);
		lseek(mem, (long)soff, 0);
		read(mem, (char *)&bdevswb, sizeof(struct bdevsw));
/* 2 */		dip->di_nunit = 0;
		if(bdevswb.d_tab == 0)
		    continue;
/* 3 */		unlink(dsk_sfn);
		k = (dip->di_rmaj << 8);
		if(dip->di_flags&DI_MSCP)
		    k |= (dip->di_cn << 6);
		if(mknod(dsk_sfn, 020400, k) < 0) {
		    printf("\nmknod for %s controller %d failed!\n",
			dip->di_typ, dip->di_cn);
		    su_err(FATAL);
		}
/* 4 */		if((i = open(dsk_sfn, 0)) >= 0)
		    close(i);
		unlink(dsk_sfn);
/* 5 */		for(i=0; i<8; i++)	/* set all drive types to NED */
		    dip->di_dt[i] = 0;
		(*dip->di_func)(MSF, dip, mem);	/* get drive type info */
	    }
	    break;
	}
/*
 * If phase 1, don't print disk configuration and only
 * make special files for drives on the system disk controller.
 *
 * If phase 2, print the system's disk configuration
 * and make the disk special files.
 *
 * If phase 3, print the disk configuration then ask
 * if the user wants to remake the special files.
 *
 * DEPENDENCY:	TODO: fixed by sdunit -- check it!
 *
 *	This code assumes that the system disk is always unit 0.
 *	This is true execpt for the RC25 which is unit 1, but the
 *	code still works because, even though the system disk is
 *	unit 1, unit 0 will also be an RC25.
 */

	while(1) {
	    if(skipit == YES)
		break;
	    if(phase > SUP1)
		pdconf(MSF);
	    if(phase == SUP2) {
		prtc();
	    }
	    if(phase == SUP3) {
		phelp("cmp_info");
		while(1) {
		    printf("\nHas the disk configuration changed ");
		    printf("(? for help) <y or n> ? ");
		    j = yes(HELP);
		    if(j == HELP) {
			phelp("h_rmsf");
			continue;
		    } else
			break;
		}
		if(j == NO)
		    break;
	    }
	    printf("\n****** MAKING ");
	    if(phase == SUP1)
		printf("SYSTEM DISK(s) ");
	    else
		printf("DISK ");
	    printf("SPECIAL FILES ******\n");
	    for(dip=di_info; dip->di_typ; dip++) {
		if(dip->di_nunit == 0)
		    continue;
		/*
		 * Following due to HACK in KLESI bus adaptor design!
		 * Get unit status returns valid unit type (rc25) even if
		 * the drive is not present. We only need first two units
		 * during phase one. Things work ok in phase two, assuming
		 * user only configures units that really exist.
		 */
		if((phase == SUP1) && (strcmp("KLESI", dip->di_typ) == 0))
		    nunits = 2;
		else
		    nunits = dip->di_nunit;
		for(i=0; i<nunits; i++) {
		    if(dip->di_dt[i] == 0)
			continue;
		    if(dip->di_flags&(DI_MSCP|DI_MASS))
			sprintf(syscmd, "/etc/msf %s -c %d %d",
			    sd_info[dip->di_dt[i]].sd_lname, dip->di_cn, i);
		    else
			sprintf(syscmd, "/etc/msf %s %d",
			    sd_info[dip->di_dt[i]].sd_lname, i);
#ifdef	DEBUG
		    printf("%s%s\n", debug, syscmd);
#endif	DEBUG
		    if(system(syscmd) != 0) {
			printf("\n%s failed!\n", syscmd);
			su_err(FATAL);
		    }
/* TODO: following done in P1 & P2, I think that's ok! */
		    if(((sd_info[dip->di_dt[i]].sd_flags&SD_SYSDSK) == 0) ||
			(i != sdunit) || (dip->di_cn != sdcntlr))
			    continue; /* following only done for system disk */
		    /*
		     * Create /dev/swap so PS and other programs
		     * can access the swap area.
		     */
		    unlink("/dev/swap");
		    if(link(sd_info[sdtype].sd_swap, "/dev/swap") < 0) {
			printf("\nCan't link %s to /dev/swap!\n",
			    sd_info[sdtype].sd_swap);
			su_err(FATAL);
		    }
		    sync();
		    /*
		     * Blast the initial /etc/fstab (root & /usr entries only).
		     */
		    for(j=0; fs_info[j].fs_lname; j++)
			if(strcmp(sd_info[sdtype].sd_lname, fs_info[j].fs_lname) == 0)
			    break;
		    /* THIS CAN'T HAPPEN! */
		    if(fs_info[j].fs_lname == 0) {
			printf("\nPANIC: sd_info & fs_info names don't match!\n");
			su_err(FATAL);
		    }
		    unlink("/etc/fstab");
		    if((fo = fopen("/etc/fstab", "w")) == NULL) {
			printf("\nCreate of /etc/fstab file failed!\n");
			su_err(FATAL);
		    }
		    fprintf(fo, "/dev/%s:/:rw:0:1\n", fs_info[j].fs_root);
		    fprintf(fo, "/dev/%s:/usr:rw:0:1\n", fs_info[j].fs_usr);
		    fclose(fo);
		    sync();
		    /*
		     * DD the boot block from /mdec to block zero of the
		     * system disk, sdload does this, but we do it to be sure!
		     */
		    sprintf(syscmd,
			"dd if=/mdec/%suboot of=%s count=1 > /dev/null 2>&1",
			(dip->di_flags&DI_MSCP) ? "ra" : dip->di_name,
			dsfn(RAW, sdunit, 0, dip));
#ifdef	DEBUG
		    printf("%s%s\n", debug, syscmd);
#endif	DEBUG
		    system(syscmd);
		    sync();
		}
	    }
	    break;
	}
/*
 * Find out what magtape controllers are present
 * and how many drives are configured.
 *
 * If phase 2, print magtape config and make special files.
 *
 * If phase 3, print magtape config, ask user if special files need remaking.
 *
 * TODO: what about multiple tape cntlr type clashes?
 *	   what about crash dump device?
 */
	while(1) {
	    if(phase < SUP1)
		break;
	    if((skipit == NO) && (phase > SUP1))
		printf("\n****** DETERMINING SYSTEM'S TAPE CONFIGURATION ******\n");
	    nht = nts = ntm = ntk = 0;
	    for(i=0; i<MAXTK; i++)
		tk_ctid[i] = 0;
	    if(nl[X_NHT].n_value) {
		lseek(mem, (long)nl[X_NHT].n_value, 0);
		read(mem, (char *)&nht, sizeof(nht));
	    }
	    if(nl[X_NTS].n_value) {
		lseek(mem, (long)nl[X_NTS].n_value, 0);
		read(mem, (char *)&nts, sizeof(nts));
	    }
	    if(nl[X_NTM].n_value) {
		lseek(mem, (long)nl[X_NTM].n_value, 0);
		read(mem, (char *)&ntm, sizeof(ntm));
	    }
	    if(nl[X_NTK].n_value) {
		lseek(mem, (long)nl[X_NTK].n_value, 0);
		read(mem, (char *)&ntk, sizeof(ntk));
	    }
	    if(nl[X_TK_CTID].n_value) {
		lseek(mem, (long)nl[X_TK_CTID].n_value, 0);
		read(mem, (char *)&tk_ctid, sizeof(tk_ctid));
		for(i=0; i<MAXTK; i++)
		    tk_ctid[i] = (tk_ctid[i] >> 4) & 017;
	    }
	    if(skipit == YES)
		break;
	    if(phase > SUP1) {
		printf("\nMagtape Controller       # Units");
		printf("\n------------------       -------");
	    }
	    j = 0;
	    if(nht) {
		if(phase > SUP1)
		    printf("\nTM02/3 - TU16/TE16/TU77  %d", nht);
		j++;
	    }
	    if(nts) {
		if(phase > SUP1)
		    /* TK25 no longer supported */
		    printf("\nTS11/TU80/TS05           %d", nts);
		j++;
	    }
	    if(ntm) {
		if(phase > SUP1)
		    printf("\nTM11 - TU10/TE10/TS03    %d", ntm);
		j++;
	    }
	    if(ntk) {
		if(phase > SUP1)
		    printf("\nTK50/TU81                %d", ntk);
		j++;
	    }
/*
 * TODO: remove after debug
	    if(tk_ctid) {
		i = 0;
		switch(tk_ctid) {
		case TK50:
		    if(phase > SUP1)
			printf("\nTK50");
		    break;
		case TU81:
		    if(phase > SUP1)
			printf("\nTU81");
		    break;
		default:
		    if(phase > SUP1)
			printf("\nTK50/TU81 configured, but not present!\n");
		    i++;
		    break;
		}
		if(i == 0) {
		    j++;
		    if(phase > SUP1)
			printf("                     1");
		}
	    }
*/
	    if(j == 0) {
		if(phase > SUP1)
		    printf("\nNO MAGTAPES FOUND!\n");
		break;
	    } else
		if(phase > SUP1)
		    printf("\n");
	    if(phase == SUP2) {
		prtc();
	    }
	    if(phase == SUP3) {
		phelp("cmp_info");
		while(1) {
		    printf("\nHas the magtape configuration changed ");
		    printf("(? for help) <y or n> ? ");
		    j = yes(HELP);
		    if(j == HELP) {
			phelp("h_rmsf");
			continue;
		    } else
			break;
		}
		if(j == NO)
		    break;
	    }
	    printf("\n****** MAKING MAGTAPE SPECIAL FILES ******\n");
	    if(nht) {
		for(i=0; i<nht; i++) {
		    sprintf(syscmd, "/etc/msf tm03 %d", i);
#ifdef	DEBUG
		    printf("%s%s\n", debug, syscmd);
#endif	DEBUG
		    if(system(syscmd) != 0) {
			printf("\n%s failed!\n", syscmd);
			su_err(FATAL);
		    }
		}
	    }
	    if(nts) {
		for(i=0; i<nts; i++) {
		    sprintf(syscmd, "/etc/msf ts11 %d", i);
#ifdef	DEBUG
		    printf("%s%s\n", debug, syscmd);
#endif	DEBUG
		    if(system(syscmd) != 0) {
			printf("\n%s failed!\n", syscmd);
			su_err(FATAL);
		    }
		}
	    }
	    if(ntm) {
		for(i=0; i<ntm; i++) {
		    sprintf(syscmd, "/etc/msf tm11 %d", i);
#ifdef	DEBUG
		    printf("%s%s\n", debug, syscmd);
#endif	DEBUG
		    if(system(syscmd) != 0) {
			printf("\n%s failed!\n", syscmd);
			su_err(FATAL);
		    }
		}
	    }
	    if(ntk) {
		for(i=0; i<ntk; i++) {
		    if(tk_ctid[i] == TK50)
			sprintf(syscmd, "/etc/msf tk50 %d", i);
		    else if(tk_ctid[i] == TU81)
			sprintf(syscmd, "/etc/msf tu81 %d", i);
		    else {
			printf("\n\7\7\7TK50/TU81 unit %d: can't get ", i);
			printf("drive type (no special files created)!");
			printf("\nRerun setup or use msf(8) to create ");
			printf("TK50 or TU81 special files.\n");
		    }
#ifdef	DEBUG
		    printf("%s%s\n", debug, syscmd);
#endif	DEBUG
		    if(system(syscmd) != 0) {
			printf("\n%s failed!\n", syscmd);
			su_err(FATAL);
		    }
		}
	    }
	    sync();
	    break;
	}
/*
 * Find out what comm. devices area configured and
 * how many units/lines of each type are configured.
 *
 * If phase 2, print comm. config and make special files.
 *
 * If phase 3, print comm. config, ask user if special files need remaking.
 *
 * TODO: what about misc. devices (du ?) and console, mem, lp?
 */

	while(1) {
	    if(phase < SUP2)
		break;
	    if(skipit == NO) {
		printf("\n****** DETERMINING SYSTEM'S COMMUNICATIONS DEVICE ");
		printf("CONFIGURATION ******\n");
	    }
	    if(cputyp[cpi].p_bus == QBUS) {
		dz_lpu = 4;
		uh_lpu = 8;
		dl_name = "DLV11";
		dz_name = "DZV11";
		uh_name = "DHV11";
	    } else {
		dz_lpu = 8;
		uh_lpu = 16;
		dl_name = "DL11";
		dz_name = "DZ11";
		uh_name = "DHU11";
	    }
	    lseek(mem, (long)nl[X_NTTY].n_value, 0);
	    read(mem, (char *)&ntty, sizeof(ntty));
	    ntty--;	/* don't count console */
	    if(nl[X_NKL11].n_value) {
		lseek(mem, (long)nl[X_NKL11].n_value, 0);
		read(mem, (char *)&nkl11, sizeof(nkl11));
		nkl11--;	/* don't count console */
	    }
	    if(nl[X_NDL11].n_value) {
		lseek(mem, (long)nl[X_NDL11].n_value, 0);
		read(mem, (char *)&ndl11, sizeof(ndl11));
	    }
	    if(nl[X_NDH11].n_value) {
		lseek(mem, (long)nl[X_NDH11].n_value, 0);
		read(mem, (char *)&ndh11, sizeof(ndh11));
	    }
	    if(nl[X_NUH11].n_value) {
		lseek(mem, (long)nl[X_NUH11].n_value, 0);
		read(mem, (char *)&nuh11, sizeof(nuh11));
	    }
	    if(nl[X_DZ_CNT].n_value) {
		lseek(mem, (long)nl[X_DZ_CNT].n_value, 0);
		read(mem, (char *)&dz_cnt, sizeof(dz_cnt));
	    }
	    if(skipit == YES)
		break;
	    printf("\nDevice  # Units  Lines/Unit");
	    printf("\n------  -------  ----------");
	    if((i = nkl11 + ndl11) > 0)
		printf("\n%-6s  %-7d  1", dl_name, i);
	    if(ndh11)
		printf("\n%-6s  %-7d  %d", "DH11", (ndh11/dh_lpu), dh_lpu);
	    if(nuh11)
		printf("\n%-6s  %-7d  %d", uh_name, (nuh11/uh_lpu), uh_lpu);
	    if(dz_cnt)
		printf("\n%-6s  %-7d  %d", dz_name, (dz_cnt/dz_lpu), dz_lpu);
	    if((nkl11 + ndl11 + ndh11 + nuh11 + dz_cnt) == 0) {
		printf("\nNO COMMUNICATIONS DEVICES FOUND!\n");
		break;
	    } else
		printf("\n");
	    if(phase == SUP2) {
		prtc();
	    }
	    if(phase == SUP3) {
		phelp("cmp_info");
		while(1) {
		    printf("\nHas the communications configuration ");
		    printf("changed (? for help) <y or n> ? ");
		    j = yes(HELP);
		    if(j == HELP) {
			phelp("h_rmsf");
			continue;
		    } else
			break;
		}
		if(j == NO)
		    break;
	    }
	    printf("\n****** MAKING COMMUNICATIONS DEVICE SPECIAL FILES ******\n");
	    printf("\nCommunications port assignments:\n");
	    k = 0;
	    nttys = 0;
	    if((i = nkl11 + ndl11) > 0) {
		sprintf(syscmd, "/etc/msf dlv11 %d tty%02d", i, nttys);
#ifdef	DEBUG
		printf("%s%s\n", debug, syscmd);
#endif	DEBUG
		if(system(syscmd) != 0) {
		    printf("\n%s failed!\n", syscmd);
		    su_err(FATAL);
		}
		sync();
		for(j=0; j<i; j++) {
		    k = 1;
		    printf("\n%s\tUNIT %d\t\t/dev/tty%02d", dl_name, j, j);
		    if((++nttys % 16) == 0) {
			k = 0;
			printf("\n");
			prtc();
		    }
		}
	    }
	    if(ndh11) {
		for(i=0; i<ndh11; i++) {
		    if((i % dh_lpu) == 0) {
			sprintf(syscmd, "/etc/msf dh11 %d tty%02d",
				(i/dh_lpu), nttys);
#ifdef	DEBUG
			printf("%s%s\n", debug, syscmd);
#endif	DEBUG
			if(system(syscmd) != 0) {
			    printf("\n%s failed!\n", syscmd);
			    su_err(FATAL);
			}
		    }
		    sync();
		    k = 1;
		    printf("\nDH11\tUNIT %d\tLINE %d\t/dev/tty%02d",
			(i/dh_lpu), (i%dh_lpu), nttys);
		    if((++nttys % 16) == 0) {
			k = 0;
			printf("\n");
			prtc();
		    }
		}
	    }
	    if(nuh11) {
		for(i=0; i<nuh11; i++) {
		    if((i % uh_lpu) == 0) {
			if(cputyp[cpi].p_bus == QBUS)
			    sprintf(syscmd, "/etc/msf dhv11 %d tty%02d",
				(i/uh_lpu), nttys);
			else
			    sprintf(syscmd, "/etc/msf dhu11 %d tty%02d",
				(i/uh_lpu), nttys);
#ifdef	DEBUG
			printf("%s%s\n", debug, syscmd);
#endif	DEBUG
			if(system(syscmd) != 0) {
			    printf("\n%s failed!\n", syscmd);
			    su_err(FATAL);
			}
		    }
		    sync();
		    k = 1;
		    printf("\n%s\tUNIT %d\tLINE %d\t/dev/tty%02d",
			uh_name, (i/uh_lpu), (i%uh_lpu), nttys);
		    if((++nttys % 16) == 0) {
			k = 0;
			printf("\n");
			prtc();
		    }
		}
	    }
	    if(dz_cnt) {
		for(i=0; i<dz_cnt; i++) {
		    if((i % dz_lpu) == 0) {
			if(cputyp[cpi].p_bus == QBUS)
			    sprintf(syscmd, "/etc/msf dzv11 %d tty%02d",
				(i/dz_lpu), nttys);
			else
			    sprintf(syscmd, "/etc/msf dz11 %d tty%02d",
				(i/dz_lpu), nttys);
#ifdef	DEBUG
			printf("%s%s\n", debug, syscmd);
#endif	DEBUG
			if(system(syscmd) != 0) {
			    printf("\n%s failed!\n", syscmd);
			    su_err(FATAL);
			}
		    }
		    sync();
		    k = 1;
		    printf("\n%s\tUNIT %d\tLINE %d\t/dev/tty%02d",
			dz_name, (i/dz_lpu), (i%dz_lpu), nttys);
		    if((++nttys % 16) == 0) {
			k = 0;
			printf("\n");
			prtc();
		    }
		}
	    }
	    if(k)
		printf("\n");
	    break;
	}
/*
 * Make the pseudo TTYs.
 *
 * In phase 2, create them.
 *
 * In phase 3, ask if they changed, re-create if necessary.
 */
	while(1) {
	    if(phase < SUP2)
		break;
	    if(skipit == NO)
		printf("\n****** DETERMINING NUMBER OF PSEUDO TTYS ******\n");
	    lseek(mem, (long)nl[X_NPTY].n_value, 0);
	    read(mem, (char *)&npty, sizeof(npty));
	    if(skipit == YES)
		break;
	    if(npty)
		printf("\nNumber of PTTYs = %d\n", npty);
	    else {
		printf("\nNO PSEUDO TTYS FOUND!\n");
		break;
	    }
	    if(phase == SUP2) {
		prtc();
	    }
	    if(phase == SUP3) {
		phelp("cmp_info");
		printf("\nHas number of PTTYs changed <y or n> ? ");
		j = yes(NOHELP);
		if(j == NO)
		    break;
	    }
	    sprintf(syscmd, "/etc/msf ptty %d", npty);
#ifdef	DEBUG
	    printf("%s%s\n", debug, syscmd);
#endif	DEBUG
	    if(system(syscmd) != 0) {
		printf("\n%s failed!\n", syscmd);
		su_err(FATAL);
	    }
	    sync();
	    break;
	}
/*
 * Make maus special files.
 *
 * In phase 2, create them.
 *
 * In phase 3, ask if they changed, re-create if necessary.
 */
	while(1) {
	    if(phase < SUP2)
		break;
	    if(skipit == NO)
		printf("\n****** DETERMINING NUMBER OF MAUS SEGMENTS ******\n");
	    if(nl[X_NMAUSENT].n_value) {
		lseek(mem, (long)nl[X_NMAUSENT].n_value, 0);
		read(mem, (char *)&nmausent, sizeof(nmausent));
	    } else
		nmausent = 0;
	    if(skipit == YES)
		break;
	    if(nmausent)
		printf("\nNumber of MAUS segments = %d\n", nmausent);
	    else {
		printf("\nMAUS NOT CONFIGURED!\n");
		break;
	    }
	    printf("\n****** MAKING MAUS SPECIAL FILES ******\n");
	    sprintf(syscmd, "/etc/msf maus %d", nmausent);
#ifdef	DEBUG
	    printf("%s%s\n", debug, syscmd);
#endif	DEBUG
	    if(system(syscmd) != 0) {
		printf("\n%s failed!\n", syscmd);
		su_err(FATAL);
	    }
	    sync();
	    break;
	}
/*
 * Create the real /etc/ttys and /etc/ttytype files.
 *
 * If phase 2, just create them.
 *
 * If phase 3, ask user they should be created, save old ones if yes.
 */
	while(1) {
	    if(phase < SUP2)
		break;
	    if(phase != SUP3) {
		printf("\n****** CREATING /etc/ttys and /etc/ttytype ");
		printf("FILES ******\n");
	    }
	    if(phase == SUP3) {
		while(1) {
		    printf("\nCreate new ttys and ttytype files <y or n> ? ");
		    j = yes(HELP);
		    if(j == HELP) {
			phelp("h_ttys");
			continue;
		    } else
			break;
		}
		if(j == NO)
		    break;
		printf("\nSaving current files: /etc/ttys.old ");
		printf("and /etc/ttytype.old!\n");
		sprintf(syscmd, "cp /etc/ttys /etc/ttys.old");
		if(system(syscmd) != 0) {
		    printf("\n%s failed!\n", syscmd);
		    su_err(FATAL);
		}
		sprintf(syscmd, "cp /etc/ttytype /etc/ttytype.old");
		if(system(syscmd) != 0) {
		    printf("\n%s failed!\n", syscmd);
		    su_err(FATAL);
		}
		sync();
	    }
	    if(crt == -1)
		crt = gcrt();
	    if(bttys(crt, ntty, npty) < 0)
		su_err(FATAL);
	    break;
	}
	if(chngdir(homedir))
	    su_err(FATAL);
/*
 * OLDCODE
 * Mount the /usr file system so we can load
 * or otherwise mundge the sysgen files.
 * Leave /usr mounted for later operations.
 */
/*	if(usrmnt(MOUNT))	/* mount /usr file system */
/*	    su_err(FATAL);	*/
	system("/etc/mount /usr >/dev/null 2>&1");
	if((phase == SUP1) && rxflag) {
	    printf("\n****** LOADING SYSGEN FILES ******\n");
	    iflop("SYSGEN #1");
	    if(chngdir("/usr"))
	    	su_err(FATAL);
	    sprintf(syscmd, "tar xpbf 10 /dev/rrx%d ./sys/conf",
		rxunit);
	    if(cputyp[tpi].p_sid == SID)
		strcat(syscmd, " ./sys/sys");
#ifdef	DEBUG
	    printf("%s%s\n", debug, syscmd);
#endif	DEBUG
	    while(system(syscmd) != 0) {
	    	if(retry("SYSGEN FILES LOAD"))
	    		continue;
	    	else
	    		su_err(FATAL);
	    }
	    sync();
	    rflop("SYSGEN #1");
	    if(cputyp[tpi].p_sid == SID)
		printf("\nSYSGEN #2 not used with split I/D processors!\n");
	    if(cputyp[tpi].p_sid == NSID) {
		fmnt("SYSGEN #2");
		chngdir("/usr/sys/ovsys");
		system("ar x /mnt/LIB1_ov");
		system("chog sys *");
		system("chmod 444 *");
		chngdir("../ovnet");
		system("ar x /mnt/LIB3_ov");
		system("chog sys *");
		system("chmod 444 *");
		fmnt(0);
		sync();
	    	rflop("SYSGEN #2");
		printf("\nSYSGEN #3 not used with non split I/D processors!\n");
	    }
	    if(cputyp[tpi].p_sid == SID) {
		if(chngdir("/usr"))
		    su_err(FATAL);
		sprintf(syscmd, "tar xpbf 10 /dev/rrx%d ./sys", rxunit);
	    	iflop("SYSGEN #3");
#ifdef	DEBUG
		printf("%s%s\n", debug, syscmd);
#endif	DEBUG
		while(system(syscmd) != 0) {
		    if(retry("SYSGEN FILES LOAD"))
			continue;
		    else
			su_err(FATAL);
		}
		sync();
	    	rflop("SYSGEN #3");
	    }
	    fmnt("SYSGEN #4");
	    if(cputyp[tpi].p_sid == NSID) {
		chngdir("/usr/sys/ovdev");
		system("cp /mnt/asmfix? .");
		system("ar x /mnt/LIB2_ov");
	    } else {
		chngdir("/usr/sys/net");
		system("ar x /mnt/LIB3_id");
	    }
	    system("chog sys *");
	    system("chmod 444 *");
	    fmnt(0);
	    sync();
	    rflop("SYSGEN #4");
	    chngdir("/usr/sys");
	    if(cputyp[tpi].p_sid == SID)
		system("chmod 755 sys dev net");
	    else
		system("chmod 755 ovsys ovdev ovnet");
	    system("chog sys *");
	    if(chngdir(homedir))
	    	su_err(FATAL);
	    printf("\n");
	}
/*
 * Remove unused sysgen libraries depending on
 * whether or not CPU has split I/D space.
 */
	if((phase == SUP1) && (rxflag == 0)) {
	    if(cputyp[tpi].p_sid == SID) {
		system("rm -f /usr/sys/conf/mch_ov.o");
		system("rm -rf /usr/sys/ovsys /usr/sys/ovdev /usr/sys/ovnet");
	    } else {
		system("rm -f /usr/sys/conf/mch_id.o");
		system("rm -f /usr/sys/sys/LIB1_id /usr/sys/dev/LIB2_id");
		system("rm -f /usr/sys/sys/*.o /usr/sys/net/*.o");
	    }
	    sync();
	}
/*
 * Select the appropriate .profile for /usr/sys.
 * Save a copy in /usr/skel/sys_profile, so osload will get
 * the correct one if it has to reload sysgen (GROSS!).
 * If crt is -1, assume console TTY type didn't change.
 */
	if(crt >= 0) {
	    if(crt == YES)
		system("cp /usr/skel/sys_crt.profil /usr/sys/.profile");
	    else
		system("cp /usr/skel/sys_prt.profil /usr/sys/.profile");
	    system("cp /usr/sys/.profile /usr/skel/sys_profile");
	}
/*
 * Ask the user for the hostname.
 */

g_hostn:
	switch(phase) {
	case SUP0:
		skipit = YES;
		break;
	case SUP1:
		skipit = YES;
		break;
	case SUP2:
		skipit = NO;
		break;
	case SUP3:
		printf("\nChange the system's hostname <y or n> ? ");
		if(yes(NOHELP) == YES)
		    skipit = NO;
		else
		    skipit = YES;
		break;
	}
	if(skipit == NO)
	    printf("\n****** NAMING YOUR ULTRIX-11 SYSTEM ******\n");
	while(1) {
	    if(skipit == YES)
		break;
	    do
		printf("\nPlease enter your system's hostname <? for help> ? ");
	    while(getline("h_hostn") <= 0);
	    if(strlen(lbuf) > 16) {
	    	printf("\nHostname too long (16 char max)!\n");
		continue;
	    }
	    j = 0;
	    for(i=0; lbuf[i]; i++) {
	    	if((lbuf[i] >= 'a') && (lbuf[i] <= 'z'))
	    		continue;
	    	if((lbuf[i] >= '0') && (lbuf[i] <= '9'))
	    		continue;
	    	if(lbuf[i] == '-')
	    		continue;
		j++;
	    }
	    if(j) {
	    	printf("\nBad Hostname (lowercase alphanumeric only)!\n");
		continue;
	    }
	    printf("%shostname is `%s' <y or n> ? ", confirm, lbuf);
	    if(yes(NOHELP) != YES)
		continue;
	    if((fi = fopen("/etc/rc", "r")) == NULL) {
	    	printf("\nCan't open /etc/rc file for reading!\n");
	    	su_err(FATAL);
	    }
	    if((fo = fopen("/tmp/rc.setup", "w")) == NULL) {
	    	printf("\nCan't create /tmp/rc.setup file!\n");
	    	su_err(FATAL);
	    }
	    while(1) {
	        if(fgets(syscmd, SCSIZE, fi) == NULL) {
		    fclose(fi);
		    fclose(fo);
		    break;
	        }
	        if(strlen(syscmd) == (SCSIZE - 1)) {
		    printf("\n/etc/rc format error: line too long!\n");
		    su_err(FATAL);
	        }
	        if(strncmp(syscmd, "hostname", 8) == 0)
		    fprintf(fo, "hostname %s\n", lbuf);
	        else
		    fprintf(fo, "%s", syscmd);
	    }
	    sprintf(syscmd, "cp /tmp/rc.setup /etc/rc");
	    if(system(syscmd) != 0) {
	    	printf("`%s' failed!\n", syscmd);
	    	su_err(FATAL);
	    }
	    unlink("/tmp/rc.setup");
	    sync();
	    shostname(&lbuf, strlen(lbuf));
	    break;
	}
/*
 * Select alternate version of more commands by 
 * mounting the /usr file system and executing
 * makefiles in /usr/bin & /usr/lib & /usr/ucb.
 * Also, unpack help data base (if loaded from floppy kit).
 * ONLY select commands once, use lock file to make sure.
 */
	while(1) {
	    if(phase != SUP1)
		break;
	    if(access(uclock, 0) == 0)
		break;		/* command select already done */
	    if(chngdir("/usr/bin"))
		su_err(FATAL);
	    printf("\n****** SELECTING%sSPLIT I/D COMMANDS ",
		(cputyp[tpi].p_sid == SID) ? " " : " NON-");
	    printf("(/usr/bin, lib, ucb) ******\n");
	    system("rm -f e ex vi view edit");
#ifdef	DEBUG
	    printf("%sex lex pcc yacc awk s5make", debug);
#endif	DEBUG
	    if(cputyp[tpi].p_sid == SID) {
		system("cp ex70 ex");
		system("cp lex70 lex");
		system ("cp pcc70 pcc");
		system("cp yacc70 yacc");
		system("cp awk70 awk");
		system("cp s5make70 s5make");
	    } else {
		system("cp ex40 ex");
		system("cp lex40 lex");
		system("cp pcc40 pcc");
		system("cp yacc40 yacc");
		system("cp awk40 awk");
		system("cp s5make40 s5make");
	    }
	    sync();
	    link("ex", "edit");
	    link("ex", "e");
	    link("ex", "vi");
	    link("ex", "view");
	    system("chog bin ex");
	    chmod("ex", 01755);
	    sync();
	    if(chngdir("/usr/lib"))
		su_err(FATAL);
#ifdef	DEBUG
	    printf("%sccom lint\n", debug);
#endif	DEBUG
	    if(cputyp[tpi].p_sid == SID) {
		system("ln /usr/c/oc2_id /usr/c/oc2");
		system("cp sendmail70 sendmail");
		system("cp ccom70 ccom");
		system("chog bin ccom");
		chmod("ccom", 0755);
		system("cp lint170 lint1");
	    } else {
		system("ln /usr/c/oc2_ov /usr/c/oc2");
		system("cp sendmail40 sendmail");
		system("cp ccom140 ccom1");
		system("cp ccom240 ccom2");
		system("chog bin ccom1 ccom2");
		chmod("ccom1", 0755);
		chmod("ccom2", 0755);
		system("cp lint140 lint1");
	    }
	    sync();
	    if(rxflag) {
		if(chngdir("help"))
		    su_err(FATAL);
		system("unpack U11_help >/dev/null 2>&1");
		sync();
	    }
	    if(chngdir("/usr/ucb"))
		su_err(FATAL);
	    if(cputyp[tpi].p_sid == SID)
		system("cp Mail70 mail");
	    else
		system("cp Mail40 mail");
	    sync();
	    if(chngdir(homedir))
		su_err(FATAL);
	    creat(uclock, 0444);	/* say command select done */
	    sync();
	    break;
	}
/*
 * Remove alternate command files (/usr/bin & /usr/lib).
 * ONLY if lock file exists (selection done).
 * It is possible this could happen more than once, but
 * that is ok.
 */
	while(1) {
	    if(phase != SUP1)
		break;
	    if(access(uclock, 0) != 0)
		break;		/* select not done yet, CAN'T HAPPEN */
#ifdef	DEBUG
	    printf("%sremove /usr/bin & /usr/lib commands <y or n> ? ", debug);
	    if(yes(NOHELP) != YES)
		break;
#endif	DEBUG
	    if(chngdir("/usr/bin"))
		su_err(FATAL);
	    system("rm -f ex40 lex40 pcc40 yacc40 awk40 s5make40");
	    system("rm -f ex70 lex70 pcc70 yacc70 awk70 s5make70");
	    sync();
	    if(chngdir("/usr/lib"))
		su_err(FATAL);
	    system("rm -f ccom70 ccom140 ccom240 lint140 lint170");
	    system("rm -f sendmail40 sendmail70");
	    if(cputyp[tpi].p_sid == SID) {
		unlink("/usr/c/oc2_ov");
		unlink("ccom1");
		unlink("ccom2");
	    } else {
		unlink("/usr/c/oc2_id");
		unlink("ccom");
	    }
	    sync();
	    if(chngdir("/usr/ucb"))
		su_err(FATAL);
	    system("rm -f Mail40 Mail70");
	    sync();
	    if(chngdir(homedir))
		su_err(FATAL);
	    break;
	}
/*
 * Set up LPR printer ports, if any.
 */
	while(1) {
	    if(phase < SUP2)
		break;
	    while(1) {
		printf("\nSet up line printer spooler and printer ports <y or n> ? ");
		j = yes(HELP);
		if(j == HELP) {
		    phelp("h_lpr");
		    continue;
		} else
		    break;
	    }
	    if(j == NO)
		break;
	    system("/usr/etc/lprsetup");
	    break;
	}

/*
 * Find out if user wants to set up user file
 * systems at this time, do setup if so.
 */

	while(1) {
	    if((phase < SUP2) && (ontarget == YES))
		break;
	    while(1) {
		if(phase <= SUP2)
		    printf("\nSet up user file systems");
		else
		    printf("\nChange (or set up new) user file systems");
		printf(" <y or n> ? ");
		j = yes(HELP);
		if(j == HELP) {
		    phelp("h_cufs");
		    continue;
		} else
		    break;
	    }
	    if(j == NO)
		break;
	/*
	 * Following depends on drive type info obtained by
	 * make special files code. This is ok because that
	 * portion of the MSF code is always executed.
	 */
	    if(pdconf(MKFS) == 0) {
		phelp("no_disks");	/* Sorry, no disks available! */
		break;
	    }
	    prtc();
	    phelp("dsk_mntd");	/* tell user to make sure disks mounted */
	    prtc();
	/*
	 * Get drive type info again.
	 * This ensures MSCP disk sizes correct and
	 * on-line status is correct.
	 */
	    for(dip=di_info; dip->di_typ; dip++) {
		if(dip->di_nunit == 0)
		    continue;
		(*dip->di_func)(MKFS, dip, mem);
	    }
	    for(dip=di_info; dip->di_typ; dip++) {
		if(dip->di_nunit == 0)
		    continue;		/* controller not configured */
		for(i=0; i<dip->di_nunit; i++) {
		    if(dip->di_dt[i] == 0)
			continue;	/* non-existent drive */
		    if(sd_info[dip->di_dt[i]].sd_flags&SD_FLOPPY)
			continue;	/* we don't do floppy disks */
		    if(sd_info[dip->di_dt[i]].sd_flags&SD_ML11)
			continue;	/* we don't do ML11 disks */
/* TODO: system disk cntlr number HP HM HJ, RA, RC, RQ ????? */
		    if((sd_info[dip->di_dt[i]].sd_flags&SD_SYSDSK) &&
			(i == sdunit) && (dip->di_cn == sdcntlr) &&
			((sd_info[dip->di_dt[i]].sd_flags&SD_USRDSK) == 0))
			    continue;	/* system disk, no free file systems */
		    if(usedisk(i, dip) == NO)
			continue;	/* user says don't use this disk drive */
		    setdisk(i, dip);	/* setup user file system(s) on disk */
		}
	    }
	    break;
	}
/*
 * Load and/or unload optional software.
 * Call setup.osl to do the actual loading/unloading.
 * Retry if setup.osl fails.
 * If not on the target CPU, assume it has no load device
 * and load optional software during phase 1.
 */
	while(1) {
	    if((phase < SUP2) && (ontarget == YES))
		break;
	    if((phase == SUP1) && (ontarget == NO))
		phelp("ntp_warn");
	    while(1) {
		printf("\nLoad/unload optional software <y or n> ? ");
		j = yes(HELP);
		if(j == HELP) {
		    phelp("h_osl");
		    continue;
		} else
			break;
	    }
	    sprintf(syscmd, "setup_osl %d %.2s %d %c %d", cputyp[tpi].p_type,
		loadev, rxflag ? rq_dt[rxunit] : 0, loadev[3], rd2);
	    if((fo = fopen(osload, "w")) != NULL) {
		fprintf(fo, "/.setup/%s\n", syscmd);
		fclose(fo);
		chmod(osload, 0755);
		system("chog bin /bin/osload");
	    } else
		printf("\nCAUTION: can't create %s!\n", osload);
	    if(j == NO)
		break;
	    system(syscmd);
/*
 * TODO: setup_osl sometimes returns an error status,
 *	   has to do with using <CTRL/C> during setup_osl.
	    if(system(syscmd) != 0) {
		printf("\nOptional software load/unload failed!\n");
		printf("\nTry again <y or n> ? ");
		if(yes(NOHELP) == YES)
		    continue;
		else
		    break;
	    }
 * TODO: end
 */
	    sync();
	    break;
	}
/*
 * Ask the user if the /usr/spool directory
 * should be coverted to a symbolic link.
 *
 *	/usr/spool can already be a symbolic link.
 *	Target file system must have enough free space.
 *	Current and target /usr/spool can't be same directories.
 * TODO:	what if user types <CTRL/C>?
 */
	while(1) {
	    if(phase < SUP2)
		break;
	    while(1) {
		if(phase == SUP2)
		    printf("\nMake ");
		else
		    printf("\nMake (or change) ");
		printf("symbolic link for /usr/spool <y or n> ? ");
		j = yes(HELP);
		if(j == HELP) {
		    phelp("h_ussl");
		    continue;
		} else
		    break;
	    }
	    if(j == NO)
		break;
	    while(1) {
		do {
		    printf("\nSymbolic link base directory name ");
		    printf("(? for help): ");
		} while(getline("h_slbdn") <= 0);
		if(lbuf[0] == '/')
		    p = &lbuf[1];
		else
		    p = &lbuf[0];
		if(strlen(p) > DIRSIZ) {
		    printf("\nName too long (%d char max)!\n", DIRSIZ);
		    continue;
		}
		sprintf(&sl_bdn, "/%s", p);
		sprintf(&sl_dst, "%s/spool", sl_bdn);
		break;
	    }
	    /*
	     * Make sure destination base directory exists.
	     */
	    if(lstat(&sl_bdn, &statb) < 0) {
		printf("\n%s does not exist!\n", &sl_bdn);
		continue;
	    }
	    if((statb.st_mode&S_IFMT) != S_IFDIR) {
		printf("\n%s is not a directory!\n", &sl_bdn);
		continue;
	    }
	    /*
	     * Make sure /usr/spool exists and see if
	     * it is a symbolic link.
	     */
	    if(lstat(sl_usd, &statb) < 0) {
		printf("\n%s does not exist!\n", sl_usd);
		su_err(FATAL);
	    }
	    if((statb.st_mode&S_IFMT) != S_IFDIR) {
		if((statb.st_mode&S_IFMT) == S_IFLNK) {
		    sl_islink = YES;
		    i = readlink(sl_usd, &sl_rlb, BUFSIZ);
		    if(i <= 0) {
			printf("\nreadlink on %s failed!\n", sl_usd);
			continue;
		    } else
			sl_rlb[i] = '\0';
		    if(i > SL_MAXPL) {
			printf("\n%s symbolic link pathname too long!\n",
			    sl_usd);
			continue;
		    } else {
			sprintf(&sl_src, "%s", sl_rlb);
			/* save src directory base name for later use */
			for(p = &sl_rlb[1]; (*p != '/'); p++);
			*p = '\0';
			sprintf(&sl_sbdn, "%s", &sl_rlb);
		    }
		} else {
		    printf("\n%s is not a directory!\n", sl_usd);
		    su_err(FATAL);
		}
	    } else
		sprintf(&sl_src, "%s", sl_usd);	/* src dir is /usr/spool */
	    if(strcmp(&sl_src, &sl_dst) == 0) {
		printf("\nCan't move spool from %s to %s!\n", &sl_src, &sl_dst);
		continue;
	    }
	    /*
	     * Mount the dst file system.
	     * Mount the src file system, only if
	     * /usr/spool was already a symbolic link.
	     * Dismount first, incase already mounted.
	     */
	    sl_mnt(UMOUNT, NOERR, &sl_bdn);
	    if(sl_mnt(MOUNT, ERR, &sl_bdn))
		continue;
	    if(sl_islink) {
		sl_mnt(UMOUNT, NOERR, &sl_sbdn);
		if(sl_mnt(MOUNT, ERR, &sl_sbdn))
		    continue;
	    }
	    printf("\nSize of %s in Kbytes:\n\n", &sl_src);
	    sprintf(&syscmd, "du -s %s", &sl_src);
	    system(&syscmd);
	    printf("\nFile system free space (see %s):\n\n", &sl_bdn);
	    system("df");
	    printf("\nIs there room for %s in %s <y or n> ? ", sl_usd, &sl_bdn);
	    if(yes(NOHELP) != YES)
		continue;
	    /*
	     * Make sure the dst directory exists.
	     */
	    if(lstat(&sl_dst, &statb) < 0) {	/* dir does not exist */
		if(mkdir(&sl_dst, 0775) < 0) {	/* create it */
		    printf("\nCan't make %s directory!\n", &sl_dst);
		    continue;
		}
	    } else {				/* dir exists, check it */
		if((statb.st_mode&S_IFMT) != S_IFDIR) {
		    printf("\n%s exists, but is not a directory!\n", &sl_dst);
		    continue;
		}
	    }
	    /*
	     * Set protection modes and ownership just to be sure.
	     * Values are bogus, but used because of history.
	     */
	    sprintf(&syscmd, "chmod 775 %s", &sl_dst);
	    system(&syscmd);
	    sprintf(&syscmd, "chown root %s", &sl_dst);
	    system(&syscmd);
	    sprintf(&syscmd, "chgrp other %s", &sl_dst);
	    system(&syscmd);
	    printf("\nMoving files from %s to %s...\n", &sl_src, &sl_dst);
	    sprintf(&syscmd, "cd %s; tar cf - . | (cd %s; tar xpf -)",
		&sl_src, &sl_dst);
	    if(system(&syscmd) != 0) {
		printf("\nTAR command failed: cannot move files!\n");
		continue;
	    }
	    /*
	     * Remove the old /usr/spool files (src directory).
	     * If /usr/spool was already a symbolic link,
	     * unlink /usr/spool and rm -r src directory.
	     * Otherwise, just rm -r /usr/spool.
	     */
	    if(sl_islink) {	 	/* If /usr/spool already a symlink */
		unlink(sl_usd);		/* unlink it & blow away src dir */
		sprintf(&syscmd, "rm -rf %s", sl_src);
		system(&syscmd);
	    } else			/* else, blow away /usr/spool */
		system("rm -rf /usr/spool");
	    /*
	     * Make a symbolic link from dst dir to /usr/spool.
	     */
	    printf("\nMaking Symbolic link: %s -> %s\n", sl_usd, &sl_dst);
	    if(symlink(&sl_dst, sl_usd) < 0) {
		printf("\nCan't create symbolic link: %s -> %s\n",
		    sl_usd, &sl_dst);
		continue;
	    }
	    /*
	     * Dismount dst file system.
	     * Dismount src file system, only if
	     * /usr/spool was already a symbolic link.
	     */
	    sl_mnt(UMOUNT, NOERR, &sl_bdn);
	    if(sl_islink)
		sl_mnt(UMOUNT, NOERR, &sl_sbdn);
	    break;
	}
	supdone(mem);
}

/*
 * Mount/umount file system for symbolic links.
 *
 * opr	= mount or umount
 * err	= allow or ignore errors
 * dir	= directory name to mount
 *
 * Mount/umount by directory name requires an entry for
 * the file system in the /etc/fstab.
 */

sl_mnt(opr, err, dir)
int	opr;
int	err;
char	*dir;
{
	register int i;
	char	cmd[100];

	sprintf(&cmd, "/etc/%s %s%s", (opr==MOUNT) ? "mount" : "umount",
	    dir, (err==ERR) ? " " : " >/dev/null 2>&1");
	i = system(&cmd);
	if((err == ERR) && (i != 0)) {
	    printf("\n%s of %s failed!\n",
		(opr==MOUNT) ? "Mount" : "Dismount", dir);
	    printf("%s must exist and be in the /etc/fstab!\n", dir);
	}
	return(i);
}

/* pmsg(str)
char	**str;
{
	register int i;

	for(i=0; str[i]; i++)
		printf("\n%s", str[i]);
	fflush(stdout);
} */

/*
 * Print a help message.
 * Call setup_help and pass along the message name.
 */

phelp(str)
char	*str;
{
	register int i;

	i = fork();
	if(i == -1) {
	    printf("\nCan't call setup_help (fork failed)!\n");
	    return;
	}
	if(i == 0) {
	    execl("/.setup/setup_help", "setup_help", str, (char *)0);
	    exit();
	}
	while(wait(0) != -1) ;
}

char	line[10];

yes(hlp)
{
	register int i;

yorn:
	fflush(stdout);
	for(i=0; i<10; i++) {
		line[i] = getchar();
		if(line[i] == '\n') {
			line[i] = '\0';
			break;
		}
	}
	if(i > 4) {
ynerr:
		printf("\nPlease answer yes or no ? ");
		goto yorn;
	}
	if((strcmp(line, "yes") == 0) || (strcmp(line, "y") == 0))
		return(YES);
	else if((strcmp(line, "no") == 0) || (strcmp(line, "n") == 0))
		return(NO);
	else if((hlp==HELP) && ((line[0]=='?') || (strcmp(line, "help")==0)))
		return(HELP);
	else
		goto  ynerr;
}

intr()
{
	signal(SIGINT, intr);
	ab_flag = 1;
	longjmp(savej, 1);
}

retry(str)
char *str;
{

	printf("\n%s FAILED: try again <y or n> ? ", str);
	if(yes(NOHELP) == YES)
		return(1);
	else
		return(0);
}

iflop(fn)
char	*fn;
{
	printf("\n\7\7\7Insert (%s) diskette into RX%d unit %d",
		fn, rq_dt[rxunit], rxunit);
	if(rq_dt[rxunit] == RX50)
	    	printf(" %s", rxpos(rxunit));
	printf("\n");
	prtc();
}

rflop(fn)
char	*fn;
{
	printf("\n\7\7\7Remove (%s) diskette from RX%d unit %d",
		fn, rq_dt[rxunit], rxunit);
	if(rq_dt[rxunit] == RX50)
	    	printf(" %s", rxpos(rxunit));
	printf("\n");
	prtc();
}

char	*rxp_tl = "(top/left)";
char	*rxp_lr = "(lower/right)";

rxpos(unit)
{
	switch(unit) {
	case 1:
		return(rxp_tl);
	case 2:
		return((rd2 == YES) ? rxp_tl : rxp_lr);
	case 3:
		return(rxp_lr);
	default:
		return("(?)");
	}
}

/*
 * Get a line of text from the terminal,
 * replace the newline with a NULL.
 * Return the string length (not counting the NULL).
 * Return 0 if the user typed only <RETURN>.
 * Use lbuf[] as the buffer and LBSIZE as limit.
 * If ? or help is typed, print the help message, if
 * one is available, appologize if not. Return -1 after help.
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
		return(0);	/* user typed only <RETURN> */
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
	    if((lbuf[0] == '?') || (strcmp("help", lbuf) == 0)) {
		if(hlp)
			phelp(hlp);
		else
			printf("\nSorry no additional help available!\n");
		return(-1);
	    }
	    return(cc);
	}
}

prtc()
{
	printf("\nPress <RETURN> to continue: ");
	fflush(stdout);
	while(getchar() != '\n') ;
}
/*
 * Create /etc/ttys and /etc/ttytype.
 * First line depends on console CRT or hardcopy.
 * Remaining lines depend on _ntty from kernel.
 *
 *		ttys		ttytype
 * CRT		22console	vt100	console
 * Hardcopy	24console	dw3	console
 */

#define	TTMAX	80		/* MAX terminal type name length */
char	ttylast[TTMAX+1];	/* Last tty type name used (for default) */
char	*tt_vt100 = "vt100";
char	*tt_dw3 = "dw3";

bttys(crt, ntty, npty)
{
	register int i;
	FILE	*fi, *fo;
	char	p, n;
	int	ask;
	char	*tp;

	if((fo = fopen("/tmp/ttys.setup", "w")) == NULL) {
		printf("\nCan't create /tmp/ttys.setup file!\n");
		return(-1);
	}
	if(crt == YES)
		fprintf(fo, "22console\n");
	else
		fprintf(fo, "24console\n");
	if(ntty)
	    for(i=0; i<ntty; i++)
		fprintf(fo, "00tty%02d\n", i);
	if(npty > 0) {
	    for(i=0; i<npty; i++) {
		p = 'p' + (i / 16);
		if((i % 16) < 10)
		    n = '0' + (i % 16);
		else
		    n = 'a' + ((i % 16) - 10);
		fprintf(fo, "00tty%c%c\n", p, n);
	    }
	}
	fclose(fo);
	sprintf(syscmd, "cp /tmp/ttys.setup /etc/ttys");
	if(system(syscmd) != 0) {
		printf("\n`%s' failed!\n", syscmd);
		return(-1);
	}
	unlink("/tmp/ttys.setup");
	sync();
	if(phase == SUP3) {
	    printf("\nWas number of pseudo TTYs the only change <y or n> ? ");
	    if(yes(NOHELP) == YES) {
		printf("\nNo need to remake /etc/ttytype file!\n");
		return;
	    }
	}
	ttylast[0] = '\0';	/* init default tty type to no default */
	if((fo = fopen("/tmp/ttytype.setup", "w")) == NULL) {
		printf("\nCan't create /tmp/ttytype.setup file!\n");
		return(-1);
	}
	tp = asktt(CONSOLE, 0, crt);	/* ask for console terminal type */
	fprintf(fo, "%s\tconsole\n", tp);
	if(ntty) {
	    while(1) {
		printf("\nAssume (for now) all remaining terminals ");
		printf("are vt100 <y or n> ? ");
		ask = yes(HELP);
		if(ask == HELP) {
		    phelp("h_gta");
		    continue;
		}
		break;
	    }
	    for(i=0; i<ntty; i++) {
		if(ask == YES)
		    tp = tt_vt100;
		else
		    tp = asktt(OTHER, i, NO);
		fprintf(fo, "%s\ttty%02d\n", tp, i);
	    }
	}
	fclose(fo);
	sprintf(syscmd, "cp /tmp/ttytype.setup /etc/ttytype");
	if(system(syscmd) != 0) {
		printf("\n`%s' failed!\n", syscmd);
		return(-1);
	}
	unlink("/tmp/ttytype.setup");
	sync();
}

/*
 * Ask user for terminal type (for ttytype file entry).
 *
 *  tt	= terminal type (console/other).
 *  tn	= tty## (0 for console).
 * crt	= console is CRT or PRT.
 *
 * NOTE: some variables used by this routine are
 *	 defined just in front of bttys(), above.
 */


asktt(tt, tn, crt)
int	tt;
int	tn;
int	crt;
{
	register int cc;
	char	*p;

	while(1) {
	    do {
		printf("\nTerminal type for ");
		if(tt == CONSOLE)
		    printf("CONSOLE terminal ");
		else
		    printf("TTY%02d ", tn);
		if(ttylast[0])
		    p = &ttylast;
		else if(tt == CONSOLE)
		    p = (crt==YES) ? tt_vt100 : tt_dw3;
		else
		    p = tt_vt100;
		printf("< %s > ? ", p);
	    } while((cc = getline("h_gtt")) < 0);
	    if(cc == 0)		/* user typed <RETURN>, use default */
		return(p);
	    if(cc == 1)
		continue;	/* in case user type y or n */
	    if((strcmp("yes", &lbuf) == 0) || (strcmp("no", &lbuf) == 0))
		continue;	/* in case user types yes or no */
	    if(cc > TTMAX) {
		printf("\nName too long (%d char max)!\n", TTMAX);
		continue;
	    }
	    sprintf(&ttylast, "%s", &lbuf);
	    return(&ttylast);
	}
}

/*
 * Zap the newline at the end of a string
 * read by fgets(), i.e., replace nl with 0.
 */

zapnl(s)
char	*s;
{
	register int i;

	for(i=0; s[i]; i++)
		if(s[i] == '\n') {
			s[i] = '\0';
			break;
		}
}

/*
 * Write the current phase number to 
 * the setup.info file.
 * No error checking, because file may not
 * even exist.
 */

char	pnbuf;

wrtpn(pn)
{
	register int fd;

	if((fd = open(info, 1)) < 0)
		return;
	pnbuf = pn + '0';
	write(fd, (char *)&pnbuf, 1);
	close(fd);
	sync();
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

	lseek(mem, (long)nl[X_NRK].n_value, 0);
	read(mem, (char *)&nrk, sizeof(nrk));
	lseek(mem, (long)nl[X_RK_DT].n_value, 0);
	read(mem, (char *)&rk_dt, sizeof(rk_dt));
	dip->di_nunit = nrk;
	for(i=0; i<nrk; i++) {
	    if(rk_dt[i] != 1)
		dip->di_dt[i] = 0;	/* NED (configured but not found) */
	    else {
		for(j=0; sd_info[j].sd_lname; j++)
		    if(sd_info[j].sd_type == RK05)
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
		read(j, (char *)&syscmd, 512);
		close(j);
	    }
	    unlink(dsk_sfn);
	}
	lseek(mem, (long)nl[X_NRP].n_value, 0);
	read(mem, (char *)&nrp, sizeof(nrp));
	lseek(mem, (long)nl[X_RP_DT].n_value, 0);
	read(mem, (char *)&rp_dt, sizeof(rp_dt));
	lseek(mem, (long)nl[X_RP_SIZES].n_value, 0);
	read(mem, (char *)&rp_sizes, sizeof(rp_sizes));
	dip->di_nunit = nrp;
	for(i=0; i<nrp; i++) {
	    if(rp_dt[i] < 0)
		dip->di_dt[i] = 0;	/* NED (configured but not found) */
	    else {
		for(j=0; sd_info[j].sd_lname; j++)
		    /* setup uses (RP02=2, RP03=3, so we add 2) */
		    if(sd_info[j].sd_type == (rp_dt[i] + 2))
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
 *	force all possilbe drives on-line by attempting open.
 *	This ensures the size info is correct.
 *
 * Note: MSCP drive type info arrays are non-semetrical
 *	 ra_index must be used to access drive type info.
 */

int	raload = NO;		/* RA info already loaded */

ra_gdti(usage, dip, mem)
register struct di_info *dip;
{
	register int i, j;
	int	cn, ind;
	int	raunits;

/*
 * Read in the controller and drive type info.
 */
/*
 * OLDCODE: for MKFS must get drive type info
 *	   each time.
	if(raload == NO) {
*/
	    raload = YES;
	    lseek(mem, (long)nl[X_NUDA].n_value, 0);
	    read(mem, (char *)&nuda, sizeof(nuda));
	    lseek(mem, (long)nl[X_NRA].n_value, 0);
	    read(mem, (char *)&nra, MAXUDA);
	    lseek(mem, (long)nl[X_UD_SIZES].n_value, 0);
	    read(mem, (char *)&ud_sizes, sizeof(ud_sizes));
	    lseek(mem, (long)nl[X_RQ_SIZES].n_value, 0);
	    read(mem, (char *)&rq_sizes, sizeof(rq_sizes));
	    lseek(mem, (long)nl[X_RC_SIZES].n_value, 0);
	    read(mem, (char *)&rc_sizes, sizeof(rc_sizes));
	    lseek(mem, (long)nl[X_RA_MAS].n_value, 0);
	    read(mem, (char *)&ra_mas, sizeof(daddr_t)*nuda);
	    raunits = 0;
	    for(i=0; i<MAXUDA; i++)
	        raunits += nra[i];
	    lseek(mem, (long)nl[X_RA_INDEX].n_value, 0);
	    read(mem, (char *)&ra_index, MAXUDA);
	    lseek(mem, (long)nl[X_RA_CTID].n_value, 0);
	    read(mem, (char *)&ra_ctid, MAXUDA);
	/*
	 * If called from make user file systems code,
	 * make sure size and on-line information in ra_drv[]
	 * is updated by attempting to open each configured drive.
	 */
	    if(usage == MKFS) {
		for(i=0; i<nuda; i++)
		    for(j=0; j<nra[i]; j++) {
			ind = (dip->di_rmaj << 8) | (i << 6) | (j << 3) | 7;
		/* TODO: some user may have /dev/setup.dsf! */
		/*		check for it??? */
			mknod(dsk_sfn, 020400, ind);
			if((ind = open(dsk_sfn, 0)) >= 0)
			    close(ind);
			unlink(dsk_sfn);
		    }
	    }
	    lseek(mem, (long)nl[X_RA_DRV].n_value, 0);
	    read(mem, (char *)&ra_drv, sizeof(struct ra_drv) * raunits);
/*	}	*/
	cn = dip->di_cn;
	ind = ra_index[cn];
	dip->di_nunit = nra[cn];
	for(i=0; i<nra[cn]; i++) {
	    if(ra_drv[ind+i].ra_dt == 0) {
		dip->di_dt[i] = 0;	/* NED (configured but not found) */
	    } else {
		for(j=0; sd_info[j].sd_lname; j++)
		    if(sd_info[j].sd_type == ra_drv[ind+i].ra_dt)
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
 * Also, save RD/RX drive types in rq_dt[] if controller
 * is RQDX1/2/3 (for iflop()).
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
		dip->di_name[1] = 'd';	/* RD31/RD32/RD51/RD52/RD53/RD54 */
		for(i=0; i<nra[cn]; i++)
		    rq_dt[i] = ra_drv[ind+i].ra_dt;
		if(rxflag) {
		    if((rq_dt[1] != RX50) && (rq_dt[1] != RX33))
			rd2 = YES;
		}
		break;
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
/* TODO: some names will change */

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
		return("RQDX1");
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

	dip->di_nunit = 2;
	for(i=0; sd_info[i].sd_lname; i++)
	    if(sd_info[i].sd_type == RX02)
		break;
	dip->di_dt[0] = i;	/* unit 0 = RX02 */
	dip->di_dt[1] = i;	/* unit 1 = RX02 */
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

	lseek(mem, (long)nl[X_NRL].n_value, 0);
	read(mem, (char *)&nrl, sizeof(nrl));
	lseek(mem, (long)nl[X_RL_DT].n_value, 0);
	read(mem, (char *)&rl_dt, sizeof(rl_dt));
	dip->di_nunit = nrl;
	for(i=0; i<nrl; i++) {
	    if(rl_dt[i] < 0)
		dip->di_dt[i] = 0;	/* NED (configured but not found) */
	    else {
		for(j=0; sd_info[j].sd_lname; j++)
		    if(sd_info[j].sd_type == rl_dt[i])
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
	    lseek(mem, (long)nl[X_NHP].n_value, 0);
	    read(mem, (char *)&nhp, MAXRH);
	    lseek(mem, (long)nl[X_HP_SIZES].n_value, 0);
	    read(mem, (char *)&hp_sizes, sizeof(hp_sizes));
	    hpunits = 0;
	    for(i=0; i<MAXRH; i++)
	        hpunits += nhp[i];
	    lseek(mem, (long)nl[X_HP_INDEX].n_value, 0);
	    read(mem, (char *)&hp_index, MAXRH);
	}
	lseek(mem, (long)nl[X_HP_DT].n_value, 0);
	read(mem, (char *)&hp_dt, hpunits);
	cn = dip->di_cn;
	ind = hp_index[cn];
	dip->di_nunit = nhp[cn];
	for(i=0; i<nhp[cn]; i++) {
	    if(hp_dt[ind+i] == 0)
		dip->di_dt[i] = 0;	/* NED (configured but not found) */
	    else {
		for(j=0; sd_info[j].sd_lname; j++)
		    if(sd_info[j].sd_type == hp_dt[ind+i])
			break;
		dip->di_dt[i] = j;	/* save index into drive info table */
	    }
	}
/*
 * Change the controller name from MASBUS to
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

	lseek(mem, (long)nl[X_NHK].n_value, 0);
	read(mem, (char *)&nhk, sizeof(nhk));
	lseek(mem, (long)nl[X_HK_DT].n_value, 0);
	read(mem, (char *)&hk_dt, sizeof(hk_dt));
	lseek(mem, (long)nl[X_HK_SIZES].n_value, 0);
	read(mem, (char *)&hk_sizes, sizeof(hk_sizes));
	dip->di_nunit = nhk;
	for(i=0; i<nhk; i++) {
	    if(hk_dt[i] < 0)
		dip->di_dt[i] = 0;	/* NED (configured but not found) */
	    else {
		for(j=0; sd_info[j].sd_lname; j++)
		    if(sd_info[j].sd_type == hk_dt[i])
			break;
		dip->di_dt[i] = j;	/* save index into drive info table */
	    }
	}
}

/*
 * Mount or unmount the /usr file system.
 *
 *  mnt = 0 for unmount, mnt = 1 for mount
 *
 * Don't mount if /usr already mounted, and
 * don't unmount if not mounted. Also,
 * don't unmount if setup didn't mount /usr.
 */

int	usrmntd = NO;
int	su_didit = NO;
char	*mntcmd = "/etc/mount /usr";
char	*umntcmd = "/etc/umount /usr";

/* TODO: routine not used */
usrmnt(mnt)
{
	switch(mnt) {
	case MOUNT:
		if(usrmntd == YES)
			break;
		if(access("/usr/bin", 0) == 0) {
			usrmntd = YES;
			su_didit = NO;
			break;
		}
		while(system(mntcmd) != 0) {
			if(retry("Mount of /usr file system"))
				continue;
			else
				return(1);
		}
		usrmntd = YES;
		su_didit = YES;
		break;
	case UMOUNT:
		if(usrmntd == NO)
			break;
		if(su_didit == NO)
			break;
		while(system(umntcmd) != 0) {
			if(retry("Dismount of /usr file system"))
				continue;
			else
				return(1);
		}
		usrmntd = NO;
		break;
	default:
		return(1);
	}
	return(0);
}

/*
 * Change directory,
 * print error message if chdir() fails.
 */

chngdir(dir)
char	*dir;
{
	if(chdir(dir) < 0) {
	    printf("\nCannot change directory to %s!\n", dir);
	    return(1);
	} else
	    return(0);
}

/*
 * Fatal error routine.
 * Depending on the error type (et),
 * print error messages, sync, and exit.
 */

su_err(et)
{
	signal(SIGINT, SIG_IGN);
	chdir("/");
	system("/etc/umount -a >/dev/null 2>&1");
	sync();
	if(et) {
	    if(et == FATAL)
		printf("\n****** FATAL ERROR DURING SETUP PHASE %d ******",
		    phase);
	    phelp("abort");
	}
	exit(1);
}

/*
 * Normal exit routine,
 * called at the end of each phase.
 * TODO: need messages about goto IG do sysgen.
 *	   may exec a copy of sysgen?????
 */

char	*cd_warn = "\nWARNING: alternate commands (*40 & *70) not removed from ";

supdone(mem)
{
	int	clean;

	signal(SIGINT, SIG_IGN);
	if(mem > 0)
	    close(mem);		/* I know, but I don't like loose ends! */
	chdir(homedir);
	if(ontarget == YES)
	    printf("\n\7\7\7****** ULTRIX-11 Setup Phase %d Completed ******\n",
		phase);
#ifdef	DEBUG
	if(ontarget == YES) {
	    printf("%sdo end of phase %d cleanup <y or n> ? ", debug, phase);
	    if(yes(NOHELP) == YES)
		clean = YES;
	    else
		clean = NO;
	}
#else	DEBUG
	clean = YES;
#endif	DEBUG
	if((clean == NO) || (phase == SUP3))
	    wrtpn(phase);
	else if((phase == SUP1) && (ontarget == NO))
	    wrtpn(SUP1);
	else
	    wrtpn(phase+1);
	if((phase == SUP1) && (ontarget == YES)) {
	    unlink(rclock);
	    unlink(uclock);
	}
	if((phase == SUP2) && (clean == YES)) {
	    sprintf(syscmd, "mv /profile /.profile");
	    if(system(syscmd) != 0)
		printf("\nWARNING: `%s' failed!\n", syscmd);
	    chmod("/.profile", 0644);
	    unlink("/profile");
	    sync();
	}
	chdir("/");
/*
 * OLDCODE
	if(usrmnt(UMOUNT))
	    printf("\nWARNING: dismount of /usr failed!\n");
*/
	system("/etc/umount -a >/dev/null 2>&1");
	sync();
	if(phase == SUP1) {
	    if(ontarget == YES)
		phelp("eop1_otp");
	    else
		phelp("eop1_ntp");
	} else if(phase == SUP2)
	    phelp("eop2");
	else
	    phelp("eop3");
	exit(0);
}

/*
 * Print disk configuration.
 * Printout altered slightly by usage argument:
 *
 *  usage = MSF (make special files)
 *  usage = MKFS (make user file systems)
 *
 * If usage == MKFS, return number of available disks.
 */

pdconf(usage)
{
	register struct di_info *dip;
	register int j, d;

	printf("\nULTRIX-11 System's Disk Configuration:\n");
	if(usage == MKFS) {
	    printf("\n* = SYSTEM DISK -- no partitions available for user ");
	    printf("file systems.");
	}
	printf("\nX = disk not configured, ");
	printf("NED = disk configured but not present.\n");
	printf("\nDisk    Cntlr  System  Unit  Unit  Unit  Unit  ");
	printf("Unit  Unit  Unit  Unit");
	printf("\nCntlr   #      Disk    0     1     2     3     ");
	printf("4     5     6     7");
	printf("\n-----   -----  ------  ----  ----  ----  ----  ");
	printf("----  ----  ----  ----");
	d = 0;
	for(dip=di_info; dip->di_typ; dip++) {
	    if(dip->di_nunit == 0)
		continue;
	    printf("\n%-6s  %-5d  ", dip->di_typ, dip->di_cn);
	    if((sd_info[dip->di_dt[sdunit]].sd_flags&SD_SYSDSK) == 0)
		printf("        ");
	    else
		printf("UNIT %c  ", sd_info[dip->di_dt[0]].sd_swap[2]);
	    for(j=0; j<8; j++) {
		if(j >= dip->di_nunit)
		    printf("X   ");
		else if(dip->di_dt[j] == 0)
		    printf("NED ");
		else {
		    printf("%-4s", sd_info[dip->di_dt[j]].sd_uname);
		    d++;
		}
		if((j == sdunit) && (usage == MKFS) &&
		   (sd_info[dip->di_dt[j]].sd_flags&SD_SYSDSK) &&
		   ((sd_info[dip->di_dt[j]].sd_flags&SD_USRDSK) == 0)) {
			printf("* ");
			d--;
		} else
			printf("  ");
	    }
	}
	printf("\n");
	return(d);
}

/*
 * Ask the user if this disk drive is to be
 * used for user file storage. Return YES or NO.
 *
 *	dip	drive info structure pointer
 *	unit	unit number
 */

usedisk(unit, dip)
register struct di_info *dip;
int	unit;
{
	register int j;

	while(1) {
	    pdoc(unit, dip);	/* print disk unit on cntlr message */
	    printf("file systems available.\n");
	    while(1) {
		printf("\nSet up user file system(s) on this disk ");
		printf("(? for help) <y or n> ? ");
		j = yes(HELP);
		if(j == HELP)
		    phelp("h_udisk");
		else
		    break;
	    }
	    return(j);
	}
}

/*
 * Set up user file system(s) on the specified disk drive.
 * Make the fstab entry and, possilby, make an empty
 * file system.
 */

setdisk(unit, dip)
register struct di_info *dip;
int	unit;
{
	register int i, j;
	int	fs, fd;
	struct	filsys	*fp;
	int	isfs, isv3fs;
	int	didmkfs, opt;
	int	fseflag;
	struct stat statb;

	spt(unit, dip);		/* Setup partition table for this disk */
	while(1) {
	    do {
		if(ppt(unit, dip)) {
		    printf("\nNo more partitions available on this disk!\n");
		    return;
		}
		if(phase == SUP3)
		    phelp("ufs_warn");
		printf("\nSelect a disk partition (? for help, ");
	        printf("`.' if done with this disk).\n");
		printf("\nDisk partition < ");
		for(i=0; i<8; i++)
		    if(pt_info[i].pt_flags == 0)
			printf("%d ", i);
		printf("> ? ");
	    } while(getline("h_sdp") <= 0);
	    if(strlen(lbuf) != 1) {
		printf("\nInvalid response!\n");
		continue;
	    }
	    if(lbuf[0] == '.') {
		printf("%sfinished with this disk <y or n> ? ", confirm);
		if(yes(NOHELP) == YES)
		    return;
		else
		    continue;
	    }
	    if((lbuf[0] < '0') || (lbuf[0] > '7')) {
		printf("\n%s - bad partition number!\n", lbuf);
		continue;
	    }
	    fs = lbuf[0] - '0';
	    if(pt_info[fs].pt_flags&PT_NUP) {
		printf("\nSorry, %s disk does not use partition %d!\n",
		    sd_info[dip->di_dt[unit]].sd_uname, fs);
		continue;
	    }
	    if(pt_info[fs].pt_flags&PT_SYS) {
		printf("\nSorry, partition %d used by the system!\n", fs);
		continue;
	    }
	    if(pt_info[fs].pt_flags&PT_USER) {
		printf("\nSorry, partition %d is already used!\n", fs);
		continue;
	    }
	    if(pt_info[fs].pt_flags&PT_OP) {
		printf("\nSorry, partition %d ", fs);
		printf("overlaps an existing file system!\n");
		continue;
	    }
	    printf("%sset up user file system on partition %d <y or n> ? ",
		confirm, fs);
	    if(yes(NOHELP) != YES)
		continue;
	    /*
	     * Set up superblock volname and fsname,
	     * mounted on directory name, and
	     * block/raw disk names.
	     */
	    sprintf(bdisk, "%s", dsfn(BLOCK, unit, fs, dip));
	    sprintf(rdisk, "%s", dsfn(RAW, unit, fs, dip));
	    if((sd_info[dip->di_dt[unit]].sd_flags&SD_SYSDSK) &&
	      (unit == sdunit) && (dip->di_cn == sdcntlr))
		sprintf(volname, "sd_%.3s", &bdisk[5]);
	    else
		sprintf(volname, "ud_%.3s", &bdisk[5]);
	    for(ufsnum=1; ufsnum<100; ufsnum++) {
		sprintf(fsname, "user%d", ufsnum);
		sprintf(mntdir, "/user%d", ufsnum);
		if(access(&mntdir, 0) == 0)
		    continue;	/* directory exists, this name already used */
		else
		    break;
/* TODO: may want to warn if bdisk or mntdir already in fstab */
	    }
	/* TODO: may ask user for 6 char name ????? */
	    if(ufsnum >= 100) {		/* PANIC: all 99 names used! */
		printf("\nSorry, all 99 possible file system names used!\n");
		su_err(FATAL);
	    }
	    /*
	     * Warn the user if a file system exists on the selected partition.
	     * If it's not a V3.0 1K file system, say can access via rawfs(8).
	     * Check s_fsize & s_isize to see if file system exists,
	     * check superblock magic numbers to see if its a V3.0 file system.
	     */
	    opt = 0;	/* opt tells how to deal with existing file systems */
	    fseflag = NO;
	    if((fd = open(rdisk, 0)) < 0) {
		printf("\n%s: open failed!\n", rdisk);
		su_err(FATAL);
	    }
	    j = sizeof(struct filsys);
	    lseek(fd, (long)(SUPERB*BSIZE), 0);
	    if(read(fd, (char *)&syscmd, j) != j) {
		printf("\n%s: superblock read failed!\n", rdisk);
		su_err(FATAL);
	    }
	    close(fd);
	    fp = (struct filsys *)&syscmd;
	    isfs = YES;
	    isv3fs = NO;
	    if((fp->s_isize <= (SUPERB+1)) ||
	       (fp->s_fsize <= 0L) ||
	       (fp->s_fsize > 16777216L) ||
	       (fp->s_isize >= fp->s_fsize))
		isfs = NO;
	    if(isfs == YES) {	/* see if it's a V3.0 file system */
		if((fp->s_magic[0] != S_0MAGIC) ||
		   (fp->s_magic[1] != S_1MAGIC) ||
		   (fp->s_magic[2] != S_2MAGIC) ||
		   (fp->s_magic[3] != S_3MAGIC))
			isv3fs = NO;
		else
			isv3fs = YES;
	    }
	    printf("\nSuperblock check indicates: ");
	    if(isfs == NO)
		printf("no ");
	    else
		printf("\n\n\t");
	    if(isfs == YES) {
		if(isv3fs == YES)
		    printf("An ");
		else
		    printf("A non ");
		printf("ULTRIX-11 V3.0 ");
	    }
	    printf("file system exists on partition %d!", fs);
	    if(isfs == YES) {
		printf("\n\tSuperblock fsname  = %.6s", fp->s_fname);
		printf("\n\tSuperblock volname = %.6s", fp->s_fpack);
	    }
	    printf("\n");
	    if((isfs == YES) && (isv3fs == NO)) {
		phelp("ufs_fe1");
		printf("\nOk to overwrite existing file system <y or n> ? ");
		if(yes(NOHELP) != YES)
		    opt = 3;	/* preserve filsys, don't use, mark part. used */
	    }
	    if((isfs == YES) && (isv3fs == YES)) {
		phelp("ufs_fe1");
		while(1) {
		    do {
			phelp("ufs_fe2");
			printf("\nOption < 1 2 3 > ? ");
		    } while(getline(NOHELP) <= 0);
		    if((strlen(lbuf) != 1) ||
		       (lbuf[0] < '1') ||
		       (lbuf[0] > '3')) {
			    printf("\nInvalid option!\n");
			    continue;
		    }
		    opt = lbuf[0] - '0';
		    break;
		}
	    }
	    while(1) {
		didmkfs = NO;
		if((isfs == YES) && (isv3fs == NO)) {
		    if(opt == 3)
			j = NO;	/* preserve filsys, don't use it */
		    else
			j = YES;
		    break;
		}
		if((isfs == YES) && (isv3fs == YES)) {
		    if(opt == 1)
			j = YES;	/* overwrite file system, use it */
		    else
			j = NO;		/* (opt==2), preserve filsys, use it */
					/* (opt==3), preserve filsys, don't use */
		    break;
		}
		printf("\nMake an empty file system on partition ");
		printf("%d (? for help) <y or n> ? ", fs);
		j = yes(HELP);
		if(j == HELP)
		    phelp("h_mkfs");
		else
		    break;
	    }
	    if(j == NO) {
		if(opt != 3) {
		    printf("%salready a file system on partition %d <y or n> ? ",
			confirm, fs);
		    if(yes(NOHELP) != YES)
			continue;
		}
	    } else {		/* answer must have been YES */
		printf("%smake a file system on partition %d <y or n> ? ",
		    confirm, fs);
		if(yes(NOHELP) != YES)
		    continue;
		if(mfsdisk(unit, fs, dip))
		    continue;	/* mkfs failed */
		didmkfs = YES;
	    }
	    /*
	     * Mark partition, and any overlapping ones, used.
	     */
	    pt_info[fs].pt_flags |= PT_USER;
	    for(j=0; j<8; j++) {
		if(j == fs)
		    continue;
		if(pt_info[fs].pt_op&(1 << j))
		    pt_info[j].pt_flags |= PT_OP;
	    }
	    if(opt == 3) {
		if(fstab(FST_RMV, bdisk, mntdir))
		    printf("\nPartition %d (%s): /etc/fstab entry removed!\n",
			fs, bdisk);
		continue;	/* preserve filsys, but don't use it */
				/* mark partition used (done above) */
	    }
	    /*
	     * If the device (bdisk) and the fsname match an entry in the
	     * fstab and the fsname matches an existing mount directory,
	     * then, we assume this is an existing user file system.
	     * Otherwise, we treate it as a new file system.
	     */
	    if((isfs == YES) && (isv3fs == YES) && (opt == 2)) {
		sprintf(fmntdir, "/%.6s", fp->s_fname);
		if(fstab(FST_SCH, &bdisk, &fmntdir) &&
		   (access(&fmntdir, 0) == 0)) {
		    sprintf(fsname, "%.6s", fp->s_fname);
		    sprintf(mntdir, "%s", fmntdir);
		    fseflag = YES;
		}
	    }
	    /*
	     * Print fsname, volname, mounted on directory.
	     * User may need this info later.
	     * First time only, tell user values can be changed.
	     */
	    printf("\nSuperblock file system name (fsname):\t%s", fsname);
	    printf("\nSuperblock volume label (volname):\t%s", volname);
	    printf("\nUser file system will be mounted on:\t%s\n", mntdir);
	    if(firstfs == YES) {
		firstfs = NO;
		printf("\nNote: you can change these values, refer to System ");
		printf("\n      Management Guide chapter 4 for instructions.\n");
	    }
	    if((isfs == YES) && (didmkfs == NO)) {
		printf("\nRe-labeling file system superblock...\n");
		sprintf(syscmd, "/etc/labelit %s %s %s -n",
		    rdisk, fsname, volname);
		while(system(syscmd) != 0) {
		    if(retry(syscmd))
			continue;
		    else
			su_err(FATAL);
		}
	    }
	    /*
	     * Make the mounted on directory.
	     * We know it does not already exist, check above.
	     * Don't make mount directory if this is an existing file system
	     * being reused, mount directory already exists.
	     */
	    if(fseflag == NO) {
		sprintf(syscmd, "mkdir %s", mntdir);
		if(system(syscmd) != 0)
		    printf("\nWARNING: %s failed!\n", syscmd);
		chmod(&mntdir, 0755);
	    }
	    /*
	     * Mount the file system and chmod 0755.
	     * Make the lost+found directory.
	     */
	    sprintf(syscmd, "/etc/mount %s %s", bdisk, mntdir);
	    if(system(syscmd) != 0)
		printf("\nWARNING: (%s) file system mount failed!\n", syscmd);
	    else {
		while(1) {
		    if(chngdir(mntdir))
			break;
		    if(lstat(lpfdir, &statb) >= 0) {
			if((statb.st_mode&S_IFMT) != S_IFDIR) {
			    printf("\n%s: lost+found exists, but ", mntdir);
			    printf("is not a directory (see fsck(8))!\n");
			}
			break;
		    }
		    mkdir(lpfdir, 0700);
		    if(chngdir(lpfdir))
			break;
		    for(i=0; i<25; i++) {
			sprintf(&syscmd, "junk%d", i);
			j = creat(&syscmd, 0666);
			if(j < 0)
			    continue;
			write(j, (char *)&syscmd, SCSIZE);
			close(j);
		    }
		    sync();
		    sync();
		    system("rm -f junk*");
		    break;
		}
		chngdir(homedir);
	    }
	    chmod(&mntdir, 0755);
	    sprintf(syscmd, "/etc/umount %s", bdisk);
	    if(system(syscmd) != 0)
		printf("\nWARNING: (%s) file system dismount failed!\n",syscmd);
	    if(fseflag == NO)
		fstab(FST_ENT, &bdisk, &mntdir);	/* add entry to /etc/fstab */
	    printf("\n%sile system table (/etc/fstab):\n\n",
		(fseflag == NO) ? "New f" : "F");
	    system("cat /etc/fstab");
	    prtc();
	}
}

/*
 * Set up the disk partition table.
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
	int i, j, nbpc;
	long i_sb, i_eb, j_sb, j_eb;

	for(i=0; i<8; i++) {
	    pt_info[i].pt_op = 0;
	    pt_info[i].pt_nb = 0L;
	    pt_info[i].pt_sb = 0L;
	    pt_info[i].pt_flags = 0;
	    if(((dip->di_flags&DI_NPD) == 0) &&
	       ((sd_info[dip->di_dt[unit]].sd_pmask&(1 << i)) == 0))
		    pt_info[i].pt_flags |= PT_NUP;    /* nonusable partition */
	}
	switch(dip->di_bmaj) {
	case RK_BMAJ:
	    pt_info[0].pt_sb = 0L;
	    pt_info[0].pt_nb = (long)rk_size;
	    for(i=1; i<8; i++)
		pt_info[i].pt_flags |= PT_NUP;
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
		rsp = &rq_sizes;
		break;
	    case RUX1:
		/* THIS CANNOT HAPPEN */
		printf("\n(spt) - can't use RUX1!\n");
		return(1);
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
		    pt_info[i].pt_sb = rsp->blkoff;
		    pt_info[i].pt_nb = rsp->nblocks;
		} else
		    pt_info[i].pt_flags |= PT_NUP;
		/* See comment above (-1 & -2 same in this case) */
		if(rsp->nblocks < 0L) {
		    pt_info[i].pt_nb =
			ra_drv[ra_index[dip->di_cn]+unit].d_un.ra_dsize -
			rsp->blkoff;
		    pt_info[i].pt_nb -= ra_mas[dip->di_cn];
		}
		rsp++;
	    }
	    nbpc = 0;
	    break;
	case RL_BMAJ:
	    /* all partitions start at block 0 */
	    pt_info[7].pt_nb = (long)rl_dt[unit]; /* drive type = unit size */
	    pt_info[7].pt_op = 3;	/* operlaps 0 and 1 */
	    pt_info[0].pt_nb = 10240L;	/* RL01 & RL02 both use partition 0 */
	    i = 2;
	    if(rl_dt[unit] == RL02) {
		pt_info[1].pt_nb = 10240L;
	    } else
		i = 1;
	    for(; i<7; i++)
		pt_info[i].pt_flags |= PT_NUP;
/* TODO: should use sd_smask, but that's just too bad! -- Fred Canter */
	    if(sd_info[dip->di_dt[unit]].sd_flags&SD_SYSDSK) {
		if((rl_dt[unit] == RL02) && (unit == sdunit)) {
		    pt_info[0].pt_flags |= PT_SYS;
		    pt_info[1].pt_flags |= PT_SYS;
		    pt_info[7].pt_flags |= PT_OP;
		}
		if((rl_dt[unit] == RL01) && ((unit == 0) || (unit == 1))) {
		    pt_info[0].pt_flags |= PT_SYS;
		    pt_info[7].pt_flags |= PT_OP;
		}
	    }
	    return(0);
	case HX_BMAJ:
	    /* THIS CAN'T HAPPEN */
	    printf("\n(spt) - can't use RX02!\n");
	    return(1);
	case HP_BMAJ:
	case HM_BMAJ:
	case HJ_BMAJ:
	    switch(sd_info[dip->di_dt[unit]].sd_type) {
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
		/* THIS CAN'T HAPPEN */
		printf("\n(spt) - can't use ML11!\n");
		return(1);
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
		} else
		    pt_info[i].pt_flags |= PT_NUP;
		dsp++;
	    }
	for(i=0; i<8; i++) {		/* find overlapping partitions */
	    if(pt_info[i].pt_flags&PT_NUP)
		continue;		/* non usable partition */
	    i_sb = pt_info[i].pt_sb;
	    i_eb = pt_info[i].pt_sb + pt_info[i].pt_nb - 1;
	    for(j=0; j<8; j++) {
		if(pt_info[j].pt_flags&PT_NUP)
		    continue;		/* non usable partition */
		if(j == i)
		    continue;		/* can't overlap itself */
		j_sb = pt_info[j].pt_sb;
		j_eb = pt_info[j].pt_sb + pt_info[j].pt_nb - 1;
		if(((i_sb >= j_sb) && (i_sb <= j_eb)) ||
		   ((i_eb >= j_sb) && (i_eb <= j_eb)) ||
		   ((j_sb >= i_sb) && (j_sb <= i_eb)) ||
		   ((j_eb >= i_sb) && (j_eb <= i_eb)) ||
		   ((j_sb <= i_eb) && (j_eb >= i_sb)))
			pt_info[i].pt_op |= (1 << j);
	    }
	}
	/*
	 * If this is the system disk,
	 * mark any partitions used by the system unusable.
	 * Also, mark any partitions which overlap them unusable.
	 */
	if((sd_info[dip->di_dt[unit]].sd_flags&SD_SYSDSK) &&
	   (unit == sdunit) && (dip->di_cn == sdcntlr)) {
		for(i=0; i<8; i++) {
		    if(sd_info[dip->di_dt[unit]].sd_smask&(1<<i)) {
			pt_info[i].pt_flags |= PT_SYS;
			for(j=0; j<8; j++)
			    if(pt_info[i].pt_op & (1<<j))
				pt_info[j].pt_flags |= PT_OP;
		    }
		}
	}
	return(0);
}

/*
 * Print the disk partition layout table (pt_info[]).
 * Show only the available partitions.
 * Return 1 if no available partitions remain.
 */

ppt(unit, dip)
register struct di_info *dip;
int	unit;
{
	register int i, j;
	int	npa;

	npa = 0;
	for(i=0; i<8; i++)
	    if(pt_info[i].pt_flags == 0)
		npa++;
	pdoc(unit, dip);	/* print disk unit # on cntlr message */
	printf("disk partition layout:");
	printf("\n(See also, System Management Guide Appendix D)\n");
	printf("\nPart-  Start    Size in  Overlapping      Partition");
	printf("\nition  Block #  K bytes  Partitions       Available");
	printf("\n-----  -------  -------  ---------------  ---------");
	for(i=0; i<8; i++) {
	    if(pt_info[i].pt_flags&PT_NUP)
		continue;
	    printf("\n%5d  %7D  %7D  ",i,pt_info[i].pt_sb/2,pt_info[i].pt_nb/2);
	    for(j=0; j<8; j++) {
		if((pt_info[i].pt_op&(1<<j)))
		    printf("%d ", j);
		else
		    printf("  ");
	    }
	    printf(" ");
	    if(pt_info[i].pt_flags&PT_SYS)
		printf("NO: used by the system!");
	    else if(pt_info[i].pt_flags&PT_USER)
		printf("NO: already used!");
	    else if(pt_info[i].pt_flags&PT_OP)
		printf("NO: overlaps used partition!");
	    else
		printf("YES: available for use!");
	}
	printf("\n");
	if(npa == 0)
	    return(1);
	else
	    return(0);
}

/*
 * Make a file system the specified disk.
 * TODO: directory naming scheme will break if there
 *	   are more than 10 mounted user file systems!
 */

char	mfscmd[100];

mfsdisk(unit, fs, dip)
register struct di_info *dip;
int	unit,fs;
{
	register char *p;

	p = dsfn(RAW, unit, fs, dip);	/* node name (/dev/r??##) */
	sprintf(mfscmd, "/etc/mkfs %s %D %s %d %s %s",
	    p, (long)pt_info[fs].pt_nb/2, sd_info[dip->di_dt[unit]].sd_lname,
	    cputyp[tpi].p_type, fsname, volname);
	printf("\n%s\n", mfscmd);
	while(system(mfscmd) != 0) {
	    if(retry(&mfscmd))
		continue;
	    else
		return(1);
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
	if(dip->di_flags&DI_NPD)
	    sprintf(dsfname, "/dev%s%s%d", (mode==RAW) ? "/r" : "/",
		dip->di_name, unit);
	else if(dip->di_flags&DI_MSCP)
	    sprintf(dsfname, "/dev%s%.2s%d%d", (mode==RAW) ? "/r" : "/",
		sd_info[dip->di_dt[unit]].sd_lname, unit, fs);
	else
	    sprintf(dsfname, "/dev%s%s%d%d", (mode==RAW) ? "/r" : "/",
		dip->di_name, unit, fs);
	return(&dsfname);
}

/*
 * Prints:
 *	DISK Unit # on ?????? Controller # --
 */

pdoc(unit, dip)
register struct di_info *dip;
int	unit;
{
	printf("\n%s Unit %d on %s Controller ",
	    sd_info[dip->di_dt[unit]].sd_uname, unit, dip->di_typ);
	if(dip->di_flags&DI_MASS)
	    printf("%d ", dip->di_cn);
	printf("-- ");
}

/*
 * Add an entry to the /etc/fstab for
 * the new user file system.
 *
 * flag	  - FST_SCH = return true if entry is in the fstab
 *          FST_ENT = make new fstab entry (discard any existing entry)
 * bdisk  - block special file (/dev/??##)
 * mntdir - mounted on directory (/user##)
 *
 * If making a new entry,
 * remove any fstab entry that matches bdisk or mntdir.
 */

fstab(flag, bdisk, mntdir)
int	flag;
char *bdisk;
char *mntdir;
{
	register FILE *fo;
	register struct fstab *fstp;
	int	pn, j, match;

	if(flag != FST_SCH) {
	    if((fo = fopen("/tmp/setup.fstab", "w")) == NULL) {
		printf("\nCreate of /tmp/setup.fstab file failed!\n");
		su_err(FATAL);
	    }
	}
	pn = 2;
	match = 0;
	while(1) {
	    if((fstp = getfsent()) == NULL)
		break;				/* end of fstab */
	    if(flag == FST_SCH) {
		if(fsequal(bdisk, fstp->fs_spec) &&
		   fsequal(mntdir, fstp->fs_file)) {
			match++;
			break;
		} else
		    continue;
	    }
	    if(fsequal(bdisk, fstp->fs_spec)) {
		match++;
		continue;			/* bdisk matches, discard */
	    }
	    if(flag == FST_ENT) {
		if(fsequal(mntdir, fstp->fs_file))
		    continue;			/* mntdir matches, discard */
	    }
	    if(fsequal("/", fstp->fs_file))
		j = 1;
	    else
		j = pn++;
	    fprintf(fo, "%s:%s:%s:%d:%d\n", fstp->fs_spec, fstp->fs_file,
		fstp->fs_type, fstp->fs_freq, j);
	}
	endfsent();
	if(flag == FST_SCH)
		return(match);
	if(flag == FST_ENT) {
	    if(strcmp("/", mntdir) == 0)
		j = 1;
	    else
		j = pn++;
	    fprintf(fo, "%s:%s:rw:1:%d\n", bdisk, mntdir, j);  /* add new entry */
	}
	fclose(fo);
	sprintf(syscmd, "cp /tmp/setup.fstab /etc/fstab");
	if(system(syscmd) != 0) {
	    printf("\n%s failed!\n", syscmd);
	    su_err(FATAL);
	}
	unlink("/tmp/setup.fstab");
	sync();
	return(match);
}

/*
 * Ask if console terminal is a CRT,
 * do stty and select correct .profile if so.
 */
/*
 * TODO: need to deal with following:
 *
 *	.cshrc and .login file for root
 *	change 24 -> 22 for console in /etc/ttys
 *	change dw3 to vt100 in /etc/ttytpye
 *	fix profiles & csh files for any possible console login account
 *	need to deal with speeds < 1200 BPS (no backspace/erase on CRT)
 */
gcrt()
{
	register int j, crt;

	while(1) {
	    crt = NO;
	    printf("\nIs the console terminal a CRT (video terminal) <y or n> ? ");
	    j = yes(HELP);
	    if(j == YES) {
	    	crt = YES;
	    	system("cp /crt.profile /profile");
	    	system("stty 9600");
	    	system("stty dec crt");
	    } else if(j == NO) {
		system("stty 300");
		system("stty dec prterase");
	    	system("cp /prt.profile /profile");
	    } else if(j == HELP) {
	    	phelp("h_crt");
		continue;
	    }
	    printf("%sconsole is a %s terminal <y or n> ? ",
		confirm, (crt == YES) ? "VIDEO" : "HARDCOPY");
	    if(yes(NOHELP) != YES)
		continue;
	    break;
	}
	return(crt);
}

fsequal(fs, fst)
char	*fs;	/* bdisk or mntdir */
char	*fst;	/* fstab: fs_spec or fs_file */
{
	register int i;
	register char *p;
	char	str[20];

	p = fst;
	for(i=0; i<20; i++) {
		str[i] = *p++;
		if(str[i] == ':') {
			str[i] = '\0';
			break;
		}
	}
	return(!strcmp(fs, &str));
}

fmnt(flop)
char	*flop;

{
	char	p[100];
	char	sysmnt[100];

	sprintf(sysmnt, "/dev/rx%d", rxunit);
	if(flop == 0) {
	    umount(sysmnt);
	    return(0);
	}
	iflop(flop);
	sprintf(p, "Mount of %s diskette on /mnt", flop);
	while(1) {
	    if(mount(sysmnt, "/mnt", 1) != 0) {
		if(retry(&p))
		     continue;
		else
		    su_err(FATAL);
	    }
	    break;
	}
}

get_dst()
{

	register int cc;
	register struct dst_table *dst_ptr;
again:
	printf("\nChoose the Geographic Area for the daylight savings time from  the table below\n");
	printf("\n\t\tGeographic Area\tSelection\n");
	printf("\t\t---------------\t---------\n");
	for(dst_ptr=dst_table;dst_ptr->dst_area;dst_ptr++) {
		printf("\t\t%s",dst_ptr->dst_area);
		if(strlen(dst_ptr->dst_area) > 7)
			printf("\t");
		else
			printf("\t\t");
		printf("%4d\n",dst_ptr->dst_id);
	}
	do 
		printf("\nEnter the selection number <%d> ",DST_USA);
	while((cc = getline("h_dstarea")) < 0);
	if(cc == 0)
		return(DST_USA);  /* USA (default) */
	cc = atoi(lbuf);
	if((cc < DST_USA) || (cc > DST_EET)) {
		printf("Enter a number between %d and %d\n",DST_USA,DST_EET);
		goto again;
	}
	return(cc);

}
