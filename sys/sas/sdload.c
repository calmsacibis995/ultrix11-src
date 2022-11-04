/*
 * SCCSID: @(#)sdload.c	3.3	7/10/87
 */
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * TODO: search for TODO to find them.
 */
/*
 *
 *  File name:
 *
 *	sdload.c
 *
 *  Source file description:
 *
 *	This file is the ULTRIX-11 stand-alone system disk load program.
 *	It is used during the installation to load the ROOT and /USR file
 *	systems onto the system disk from the distribution kit. There are
 *	two versions of sdload, one for the magtape kits and a second
 *	version for the floppy disk kits. Version specific code can be
 *	identified by #ifdef MTLOAD for the magtape version and #ifdef RXLOAD
 *	for the floppy version, RLLOAD for RL02 version, RCLOAD for RC25 version.
 *
 *	The sdload program is loaded by boot when the user invokes the
 *	install command. Sdload loads into low memory, but, unlike most
 *	other stand-alone programs, it relocates to hi memory and runs in
 * 	user mode, in the same manner as boot. This allows sdload to load
 *	and run other stand-alone programs (rabads, mkfs, restor, icheck)
 *	to do most of the installation work. Sdload passes the answers to
 *	questions asked by the stand-alone programs to them via the argflag
 *	and argbuf at address 01010 in each program (see srt0.s).
 *
 *  Functions:
 *
 *	main()		The main, what else can I say?
 *
 *	loadprog()	Loads and executes a stand-alone program in the
 *			same manner as boot (almost exact same code).
 *
 *	copyargs()	Used to pass arguments from sdload to the stand-
 *			alone program, via argflag & argbuf at address
 *			01010 in each program (see srt0.s).
 *
 *	match()		Used to match two srings of up to DIRSIZ (14)
 *			characters in length. If string A is N characters
 *			and string B is N+3 characters, and the first N
 *			characters match string A, then the strings match.
 *
 *	yes()		Return a value that indicates the user typed: yes,
 *			no, ?, or help. Otherwise, ask user to enter y or n.
 *
 *	retry()		Function failed, ask user if it should be retried.
 *
 *	psmg()		Prints a message, consisting of an array of character
 *			pointers.
 *
 *	iflop()		Instruct user to insert the named diskette into the
 *			specified drive.
 *
 *	rflop()		Instruct the user to remove a diskette.
 *
 *	rxpos()		Tells the user the physical mounting position of the
 *			floppy drive, according to the unit number.
 *
 *  Usage:
 *
 *	sdload		Called by boot install command, user does not call
 *			sdload directly.
 *
 *  Compile:
 *
 *	cd /usr/sys/sas; make mtload	(magtape version)
 *	cd /usr/sys/sas; make rxload	(floppy disk version)
 *	cd /usr/sys/sas; make rcload	(RC25 disk version)
 *	cd /usr/sys/sas; make rlload	(RL02 disk version)
 *
 *  Modification history:
 *
 *	01 January 1984
 *		File created -- Fred Canter
 *
 *	Time passes
 *		Many changes -- sccs prt sdload.c
 *
 *	11 March 1985
 *		Version 2.2 -- Fred Canter
 *		Sdload got too big, split it into four versions.
 *		#ifdef MTLOAD is the magtape version.
 *		#ifdef RXLOAD is the floppy disk version.
 *		#ifdef RCLOAD is the RC25 disk version.
 *		#ifdef RLLOAD is the RL02 disk version.
 *	TODO:	use "sccs prt sdload.c" to fill in modification history
 *		someday when I get the time (ha ha)!
 *
 */
#include <sys/param.h>
#include <sys/ino.h>
#include <sys/inode.h>
#include <sys/filsys.h>
#include <sys/dir.h>
#include <sys/seg.h>
#include <sys/tty.h>
#include "saio.h"
#include "sa_defs.h"
#include "ra_saio.h"
#include <a.out.h>

#if	defined(MTLOAD) || defined(RLLOAD) || defined(RCLOAD)
#define	COMMON
#endif

/*
 * WHEN BOOT LOADS SDLOAD
 *
 * 	The boot program (Boot:) loads sdl_bdn with the two character
 *	boot device ID (ht, ts, tm, rx, md, tk) and sdl_bdn with the unit
 *	number of the boot device (1-3 for rx, 0 for all others). These
 *	variables tell sdload where to find the stand-alone programs it
 *	needs to install the system. If the system has 512KB or more memory,
 *	this will be the memory disk, otherwise it will be the actual boot
 *	device. See M.s for code that actually loads these variables.
 *
 * WHEN SDLOAD LOADS BOOT
 *
 *	Sdload uses sdl_bdn and sdl_bdu to pass the ID and unit number of
 *	the system disk (root device) back to the boot program (Boot:).
 *	This tells the boot program the name of the preconfigured kernel
 *	and what disk to boot it from. The boot program constructs a file
 *	spec of the form: ??(#,0)??unix, where ?? is the two character device
 *	code (hk, hp, ra, rc, rd, rl, rp) and # is the unit number (1 for rc,
 *	0 for all others).
 *
 * CAUTION:
 *
 *	Passing info via these variables only works because they live near
 *	the start of M.s, and both boot and sdload begin with M.s, i.e.,
 *	they have the same address in both programs.
 */

int	sdl_bdn;	/* two character device name */
int	sdl_bdu;	/* boot device unit number */

/*
 * WHEN BOOT LOADS SDLOAD
 *
 * 	The boot program (Boot:) loads sdl_ldn with the two character
 *	load device ID (ht, ts, tm, rx, md, tk) and sdl_ldn with the unit
 *	number of the load device (1-3 for rx, 0 for all others). These
 *	variables tell sdload where to find the ROOT and /USR file systems.
 *	If the system has at least 512KB of memory, the boot and load
 *	devices will not be the same. The file systems are always loaded
 *	from the distribution device, but the stand-alone programs can be
 *	loaded from the distribution media or the memory disk.
 *
 * WHEN SDLOAD LOADS BOOT
 *
 *	Used to pass the load device ID back to boot. This allows boot to
 *	auto-select the magtape for the generic kernel.
 *
 * CAUTION:
 *
 *	Same as above.
 */

int	sdl_ldn;	/* two character load device name */
int	sdl_ldu;	/* load device unit number */

/*
 * Variables set by the machine language assist (M.s).
 */

int	sepid;		/* 1 if split I/D CPU, 0 if not */
int	cputype;	/* Current processor type ID */
unsigned maxmem;	/* Memory size in 64 byte clicks (set by M.s) */
int	copybuf;	/* Only to make linker happy (symbol in M.s) */

/*
 * Other globl variables and strings.
 */

int	proctyp;	/* Target processor type ID */
char	cpuname[] = "##\0";
char	line[100];
char	nblk[10];
#define	PSNSIZE	40
char	pack_sn[PSNSIZE];

int	magic;
int	ra_openf[];
int	rl_openf;
int	scload;		/* syscall file loaded */
/*
 * Startup and help message text strings.
 */
#define	YES	1
#define	NO	0
#define	HELP	2

char	*startup[] =
{
	"",
	"       ****** ULTRIX-11 System Disk Load Program ******",
	"",
	"This program loads the base ULTRIX-11 system from the distribution",
	"media onto the system disk, then bootstraps the system disk. After",
	"booting, the setup program begins the initial setup dialogue.",
	"",
	"Before loading can begin, you need to answer some questions about",
	"your system's configuration. Enter your answer, using only lowercase",
	"characters, then press <RETURN>. If you need help answering any of",
	"the questions, enter a ? then press <RETURN>.",
	"",
	"To correct typing mistakes press the <DELETE> key to erase a single",
	"character or <CTRL/U> to erase the entire line.",
	0
};

char	*warnmsg[] =
{
	"",
	"                  ****** WARNING ******",
	"",
	"Installing the ULTRIX-11 software will overwrite your system disk.",
	"In addition, the ULTRIX-11 V3.0 file system is not compatible with",
	"the file systems used by previous ULTRIX-11 releases or any other",
	"software systems. Existing user disks must be converted to the new",
	"1K block file system.",
	"",
	"DO NOT PROCEED UNTIL YOU HAVE READ INSTALLATION GUIDE SECTION 1.7",
	"",
	"Proceed with the installation <y or n> ? ",
	0
};

char	*pt_help[] =
{
	"The target processor is the CPU on which the ULTRIX-11 software",
	"will actually be used, this may or may not be the current CPU.",
	"If you are installing from a magtape distribution and your CPU",
	"does not have a magtape unit, you can install the software on a",
	"processor with a magtape drive, then move the software to the",
	"target processor.",
	"",
	0
};

char	*pt_ask[] =
{
	"Please enter one of the following supported processor types:",
	"",
	"    23, 24, 34, 40, 44, 45, 53, 55, 60, 70, 73, 83, 84",
	"",
	"The Micro/PDP-11 will be one of the following processor types:",
	"",
	"    23  KDF11-B",
	"    53  KDJ11-D",
	"    73  KDJ11-A",
	"    83  KDJ11-B",
	"",
	0
};

char	*rpwarn[] =
{
	"",
	"		****** CAUTION ******",
	"",
	"You must surface verify RP04/5/6 disks, type ?<RETURN> for help!",
	"",
	0
};

char	*svd_help[] =
{
	"Disk surface verification involves a write/read/compare test of",
	"each block on the system disk, using a number of test patterns.",
	"Any disk block which fails this test will be entered into the bad",
	"block file and automatically replaced by an alternate disk block.",
	"",
	"DIGITAL recommends that you surface verify your disk, unless you",
	"are certain that the data integrity of your disk media is accept-",
	"able and that it contains a valid bad block file.",
	"",
	"For RP04/5/6 disks, you must surface verify the disk unless you",
	"are absolutely sure the pack has been initialized with DSKINIT.",
	"Even if the disk pack already has a valid bad block file, bad",
	"block replacement will not function properly until the disk has",
	"been initialized by the DSKINIT program.",
	"",
	0
};

char	*np_help[] =
{
	"The disk surface may be verified using up to eight different",
	"test data patterns. The thoroughness of the surface verification",
	"increases with the number of test patterns that you run. However,",
	"using a large number of test patterns can be very time consuming",
	"because, each pattern writes and reads every block on the disk.",
	"The following table gives the approximate run time per pattern:",
	"",
	0
};

char	*np1_help[] =
{
	"Selecting <1> test pattern will cause the `worst case' data",
	"pattern to be used for disk surface verification.",
	"",
	0
};

char	*fmt_help[] =
{
	"Formatting the disk involves organizing the data bits on each",
	"disk track into 512 byte sectors and writing the appropriate",
	"format information in the header words of each sector header.",
	"",
	"DIGITAL recommends that you format the disk, unless you are",
	"certain that it is properly formatted.",
	"",
	0
};

char	*rt_help[] =
{
	"The attempted operation failed to complete successfully. The",
	"procedure is to determine the cause of the failure, then retry",
	"the operation. Some possible causes of such failures are:",
	"",
	"    o  The system disk is write protected.",
	"",
	"    o  The system disk is off-line,",
	"",
	"    o  The system disk media may be defective.",
	"",
	"    o  There may be a fault in the system hardware.",
	"",
	"Answer `yes' if you wish to retry the operation.",
	"",
	"Answer `no' to abort the entire installation.",
	"",
	0
};

char	*rawarn[] =
{
	"",
	"		****** CAUTION ******",
	"",
	"You must scan MSCP disks for bad blocks, type ?<RETURN> for help!",
	"",
	0
};

char	*ssd_help[] =
{
	"The bad block scan program qualifies disk media for use with the",
	"ULTRIX-11 software by reading the disk and printing information",
	"about any bad blocks encountered during the read.",
	"",
	"For MSCP disks (RD31 RD32 RD51 RD52 RD53 RD54 RC25 RA60 RA80 RA81)",
	"the bad block scan program automatically replaces each bad block",
	"by revectoring it to a replacement block. For all other disks,",
	"the bad block scan program only identifies bad blocks.",
	"",
	"When the bad block scan has been completed, the program will warn",
	"you if the disk cannot be used with the ULTRIX-11 software.",
	"",
	"DIGITAL recommends you answer `yes', unless you are certain your",
	"disk meets the UTLRIX-11 media requirements.",
	"",
	0
};

char	*booterr[] =
{
	"",
	"FATAL ERROR: while attempting to return to Boot:",
	"",
	"Execute the hardware bootstrap for your distribution load device,",
	"that is, RX50, magtape, etc., to return to the Boot: prompt.",
	"",
	0
};

char	*ssao_help[] =
{
	"",
	"A typical ULTRIX-11 system disk layout is:",
	"",
	"	+-------------+------------+",
	"	| SYSTEM AREA | USER FILES |",
	"	+-------------+------------+",
	"",
	"The system area consists of: ROOT, SWAP & ERROR LOG, and /USR.",
	"The remainder of the disk may be allocated for user file storage.",
	"",
	"DIGITAL recommends you scan the entire surface of your disks for",
	"bad blocks. However, you may choose to scan only the system area.",
	"This option can be used to expedite the installation if you are",
	"recovering from an error and/or you have already scanned the disks",
	"for bad blocks.",
	"",
	0
};

char	*baddisk[] =
{
	"",
	"****** WARNING - BAD BLOCK(s) DETECTED ******",
	"",
	"This disk CANNOT be used with the ULTRIX-11 system.",
	"",
	"Replace the defective disk and retry the bad block scan.",
	0
};

char	*disk_ng[] =
{
	"",
	"****** WARNING - DO NOT USE THIS DISK ******",
	"",
	"Replace the defective disk and retry the pack initialization.",
	0
};

char	*rabdsk[] =
{
	"",
	"****** WARNING - DISK HAS UNREPLACED BAD BLOCKS ******",
	"",
	"This disk cannot be used with the ULTRIX-11 system.",
	"",
	"Replace or reformat the media then restart the installation.",
	"",
	0
};

char	*psn_help[] =
{
	"The serial number of the disk pack is part of the information",
	"stored in the bad block file.",
	"",
	"If your disk pack does not have a vaild bad block file, one will",
	"be created when the disk pack is initialized. Enter the disk pack",
	"serial number then press <RETURN>. The pack serial number should",
	"be located on the bottom of the disk pack. Remove the bottom cover",
	"from the pack to expose the serial number.",
	"",
	"If you are certain that the disk pack has a valid bad block file",
	"or you cannot obtain the pack serial number, just press <RETURN>.",
	"The program will enter a fictitious pack serial number into the",
	"disk's bad block file.",
	"",
	0
};

char	*no_bfile[] =
{
	"",
	"****** WARNING - DISK NEEDS INITIALIZATION *****",
	"",
	"This disk does not have a valid bad block file and connot be",
	"used with the ULTRIX-11 software in its present state.",
	"",
	"You must initialize the disk pack by answering `yes' to the",
	"`Surface Verification' question. You have the option to format",
	"the disk pack by answering `yes' to the `Format' question.",
	0
};

#ifdef	RXLOAD
char	*rxsetup[] =
{
	"",
	"INSTALLING FROM RX50 DISKETTES REQUIRES MANUAL INTERVENTION",
	"",
	"During the installation, you will be asked to insert a diskette",
	"into a floppy disk drive or remove a diskette from a floppy drive.",
	"The diskette is identified by the name on the label. The floppy",
	"disk drive is referenced by its unit number. On systems with more",
	"than one floppy disk drive, the physical mounting position of the",
	"floppy drive is also given, that is:",
	"",
	"VERTICALLY MOUNTED SYSTEMS:",
	"",
	"    Left  drive: UNIT 1 (UNIT 2 if second RD DISK present)",
	"    Right drive: UNIT 2 (UNIT 3 if second RD DISK present)",
	"",
	"HORIZONTALLY MOUNTED SYSTEMS:",
	"",
	"    Top   drive: UNIT 1 (UNIT 2 if second RD DISK present)",
	"    Lower drive: UNIT 2 (UNIT 3 if second RD DISK present)",
	"",
	"Please leave the BOOT diskette loaded until you are instructed to",
	"remove it, or the operating system has been successfully booted.",
	0
};

char	*mnt_root[] =
{
	"The following operation loads the ULTRIX-11 ROOT file system from",
	"RX50 diskettes, labeled ROOT #1 thru ROOT #9, onto the system disk.",
	"You will be requested to insert and remove the ROOT diskettes in",
	"sequence, using the following procedure:",
	0
};

char	*mnt_comn[] =
{
	"",
	"    1. Insert the first diskette.",
	"",
	"    2. Press <RETURN> and wait for the following message:",
	"",
	"           Mount volume # <type return when ready>",
	"",
	"    3. Remove the current diskette and insert the next one.",
	"       (# will be the number of the diskette to insert)",
	"",
	"    4. Repeat steps 2 and 3 until all the diskettes have been",
	"       loaded, then remove the last diskette and press <RETURN>.",
	"       (Each diskette takes about two minutes to load)",
	"",
	0
};

char	*mnt_usr[] =
{
	"",
	"The following operation loads the ULTRIX-11 USR file system from",
	"RX50 diskettes, labeled USR #1 thru USR #9, onto the system disk.",
	"You will be requested to insert and remove the USR diskettes in",
	"sequence, using the following procedure:",
	0
};
#endif	RXLOAD

/*
 * Stand-alone program name file specifications,
 * used to load and run other stand-alone programs.
 * The ?? is the two character device name and # is
 * the unit number. Programs can be loaded from the
 * boot device or the memory disk.
 * ?? (ht tm ts tk rx md)
 * # (1-3 for rx, 0 for all others)
 */

char	mkfs[] = "??(#,0)mkfs\0";
char	restor[] = "??(#,0)restor\0";
char	bads[] = "??(#,0)bads\0";
char	rabads[] = "??(#,0)rabads\0";
char	dskinit[] = "??(#,0)dskinit\0";
char	icheck[] = "??(#,0)icheck\0";
char	syscall[] = "??(#,0)syscall\0";
char	ldboot[] = "??(#,0)boot\0";
char	root[] = "??(#,0)root\0";
char	usr[] = "??(#,0)usr\0";

char	npat[] = "?\0";
char	dkunit[] = "#\0";	/* Disk unit # for BADS, DSKINIT, RABADS */

int	rlflag;		/* loading ONTO an RL01 or RL02 disk */
int	rxflag;		/* loading from RX50 diskettes */
int	rxunit;		/* RX unit # for restor */
int	nrxunit;	/* Number of RX50/RX33 floppy drives available */
int	rdrx_cn;	/* RD/RX MSCP cntlr number (to access ra_drv[][]) */
int	rd2;		/* second RD disk present, RX unit #'s shift up by 1 */
			/* 0 = NO, 2nd didit of drive type if YES */
int	bootout;	/* boot floppy has been removed from drive */

struct	ra_drv	ra_drv[3][4];	/* see ra_saio.h (actually in RA driver) */
				/* size MUST be 3 X 8 */
int	format;

char	volname[15];
char	bootfn[30];
char	textfn[35];
char	rdbuf[512];

/*
 * Argument buffer to be passed to program.
 * See srt0.s and sa_defs.h
 */

int	args[ARGSIZ/2];

/*
 * Program phases,
 * the step variable determines the next step to be
 * executed after the current step is completed.
 */

#define	STEP0	0	/* Initial dialog */
#define	RET0	20	/* Return from STEP 0 */
#define	STEP1	1	/* Format/verify disk media (dskinit optional) */
#define	RET1	21	/* Return from STEP 1 */
#define	STEP2	2	/* Scan disk media for bad blocks (bads optional) */
#define	RET2	22	/* Return from STEP 2 */
#define	STEP3	3	/* ROOT - make file system */
#define	RET3	23	/* Return from STEP 3 */
#define	STEP4	4	/* ROOT - restore root from tape */
#define	RET4	24	/* Return from STEP 4 */
#define	STEP5	5	/* ROOT - icheck file system */
#define	RET5	25	/* Return from STEP 5 */
#define	STEP6	6	/* USR  - make file system */
#define	RET6	26	/* Return from STEP 6 */
#define	STEP7	7	/* USR  - restore /usr from tape */
#define	RET7	27	/* Return from STEP 7 */
#define	STEP8	8	/* USR  - icheck file system */
#define	RET8	28	/* Return from STEP 8 */
#define	STEP9	9	/* MKFS on second RD disk (optional) */
#define	RET9	29	/* Return fron STEP 9 */
#define	STEP10	10	/* Check file system on second RD disk */
#define	RET10	30	/* Return from STEP 10 */
#define	STEP11	11	/* Copy block zero boot to system disk */
#define	RET11	31	/* Return from STEP 11 */
#define	STEP12	12	/* Auto-boot system disk to single-user */
#define	RET12	32	/* Return from STEP 12 */

int	step = STEP0;	/* Start with initial dialog */
int	xitstat;	/* standalone program exit status */

/*
 * System disk information,
 * size and location of each file system,
 * disk name and if bad block replacement supported.
 */

#define	RABADS	2
#define	BADS	1
#define	NOBADS	0

int	sdtype;		/* system disk type, points into sd_info */

struct	sd_info {
	char	*sd_name;	/* system disk name */
	char	*sd_rsize;	/* root file system size */
	char	*sd_rdev;	/* root device file spec */
	char	*sd_usize;	/* /usr file system size */
	char	*sd_udev;	/* /usr device file spec */
	int	sd_bads;	/* NOBADS, BADS, RABADS */
	int	sd_mpp;		/* # of minutes per test data pattern */
	unsigned sd_sas;	/* system area size, 0=entire pack */
	char	*sd_boot;	/* name of block zero boot */
} sd_info[] = {
    "rd31","4850","rd(0,0)","14364","rd(0,12800)",RABADS,0,00000,"rauboot",
    "rd32","4850","rd(0,0)","8650", "rd(0,9700)", RABADS,0,00000,"rauboot",
    "rd51","3730","rd(0,0)","5934", "rd(0,9700)", RABADS,0,00000,"rauboot",
    "rd52","4850","rd(0,0)","8650", "rd(0,9700)", RABADS,0,00000,"rauboot",
    "rd53","4850","rd(0,0)","8650", "rd(0,9700)", RABADS,0,00000,"rauboot",
    "rd54","4850","rd(0,0)","8650", "rd(0,9700)", RABADS,0,00000,"rauboot",
#ifdef	RLLOAD
    "rl01","4000","rl(1,0)","5120", "rl(2,0)",    NOBADS,0,00000,"rluboot",
    "rl02","4000","rl(1,0)","5120", "rl(1,10240)",NOBADS,0,00000,"rluboot",
#else
    "rl01","4000","rl(0,0)","5120", "rl(1,0)",    NOBADS,0,00000,"rluboot",
    "rl02","4000","rl(0,0)","5120", "rl(0,10240)",NOBADS,0,00000,"rluboot",
#endif	RLLOAD
    "rk06","3960","hk(0,0)","8063", "hk(0,10956)",BADS, 10,00000,"hkuboot",
    "rk07","3960","hk(0,0)","21395","hk(0,10956)",BADS, 20,00000,"hkuboot",
    "rp02","4200","rp(0,0)","14200","rp(0,11600)",NOBADS,0,00000,"rpuboot",
    "rp03","4200","rp(0,0)","34200","rp(0,11600)",NOBADS,0,00000,"rpuboot",
    "rp04","4800","hp(0,0)","10032", "hp(0,9614)",BADS, 30,35948,"hpuboot",
    "rp05","4800","hp(0,0)","10032", "hp(0,9614)",BADS, 30,35948,"hpuboot",
    "rp06","4800","hp(0,0)","10032", "hp(0,9614)",BADS, 60,35948,"hpuboot",
    "rm02","4560","hp(0,0)","10000","hp(0,9120)", BADS, 20,34720,"hpuboot",
    "rm03","4560","hp(0,0)","10000","hp(0,9120)", BADS, 15,34720,"hpuboot",
    "rm05","5168","hp(0,0)","10640","hp(0,10336)",BADS, 90,38304,"hpuboot",
    "ra60","4800","ra(0,0)","10000","ra(0,9600)", RABADS,0,35800,"rauboot",
    "ra80","4800","ra(0,0)","10000","ra(0,9600)", RABADS,0,35800,"rauboot",
    "ra81","4800","ra(0,0)","10000","ra(0,9600)", RABADS,0,35800,"rauboot",
    "rc25","4500","rc(1,0)","18800","rc(1,13200)",RABADS,0,00000,"rauboot",
    0
};

main()
{
	int	i, j;
	char	c0, c1, cn;
	int	fi, fo;
	char	*p;
	struct devsw *dp, *dpa;
	daddr_t	tfbn;
	int	tfbn_bad;
	int	scan_sao;

/*
 * Set the segflag for a 64k word boundry.
 */
#ifndef	BIGKERNEL
	segflag = 2;
#else	BIGKERNEL
	segflag = 3;
#endif	BIGKERNEL

	ra_openf[0] = 0;
	ra_openf[1] = 0;
	ra_openf[2] = 0;
	rl_openf = 0;
	xitstat = mfpi(RTNSTAT); /* S/A program exit status (sa_defs.h/srt0.s) */
				 /* first RTNSTAT is 42, don't matter anyway */
	switch(step) {
	case STEP0:
		goto init;
#ifdef	COMMON
	case RET1:
		goto rtn1;
#endif	COMMON
	case RET2:
		goto rtn2;
	case RET3:
		goto rtn3;
	case RET4:
		goto rtn4;
	case RET5:
		goto rtn5;
	case RET6:
		goto rtn6;
	case RET7:
		goto rtn7;
	case RET8:
		goto rtn8;
	default:
		printf("\nUndefined step (%d)\n", step);
		goto sl_exit;
	}
sl_exit:
	if(loadprog(ldboot)) {
		pmsg(booterr);
		for( ;; );
	}
	step = STEP0;	/* should never return */
	return;

init:
/*
 * Construct standalone program file spec's from
 * boot/load device IDs, passed from Boot: program.
 * Possible names are:
 *
 *	md(0,0)	memory disk (copied from floppy or tape)
 *	ht(0,0) magtapes
 *	ts(0,0)
 *	tm(0,0)
 *	tk(0,0)	TK50
 *	rx(#,0) RX50 (# is unit number)
 */
	c0 = sdl_bdn & 0177;
	c1 = (sdl_bdn >> 8) & 0177;
	cn = sdl_bdu;
	mkfs[0] = c0; mkfs[1] = c1; mkfs[3] = cn;
	restor[0] = c0; restor[1] = c1; restor[3] = cn;
	bads[0] = c0; bads[1] = c1; bads[3] = cn;
	rabads[0] = c0; rabads[1] = c1; rabads[3] = cn;
	dskinit[0] = c0; dskinit[1] = c1; dskinit[3] = cn;
	icheck[0] = c0; icheck[1] = c1; icheck[3] = cn;
	syscall[0] = c0; syscall[1] = c1; syscall[3] = cn;
	ldboot[0] = c0; ldboot[1] = c1; ldboot[3] = cn;
/*
 * Construct load device file names, used for
 * restoring the ROOT and /USR file systems.
 * Possible names are:
 *
 *	ht(0,0) magtapes
 *	ts(0,0)
 *	tm(0,0)
 *	tk(0,0) TK50
 *	rx(#,0) RX50 (# is unit number)
 */
	c0 = sdl_ldn & 0177;
	c1 = (sdl_ldn >> 8) & 0177;
	cn = sdl_ldu;
	if((c0 == 'r') && (c1 == 'l')) {
		sprintf(root, "??(#,1000)");
		sprintf(usr, "??(#,9000)");
	}
	if((c0 == 'r') && (c1 == 'c')) {
		sprintf(root, "??(#,1000)");
		sprintf(usr, "??(#,13200)");
	}
	root[0] = c0; root[1] = c1; root[3] = cn;
	usr[0] = c0; usr[1] = c1; usr[3] = cn;
	if((c0 == 'r') && (c1 == 'x')) {
		root[7] = '\0';
		usr[7] = '\0';
		rxflag = 1;
/*
 * See if second RD disk present.
 * See how many floppy drives we have to work with.
 * Force controller init with open().
 * Look at disk drive types.
 */
		if((j = open("rd(0,0)", 0)) >= 0)
			close(j);
		for(i=0; devsw[i].dv_name; i++)
			if(strcmp("rd", devsw[i].dv_name) == 0)
				break;
		rdrx_cn = devsw[i].dv_cn;	/* MSCP cntlr number */
		if((ra_drv[rdrx_cn][1].ra_dt != RX50) &&
		   (ra_drv[rdrx_cn][1].ra_dt != RX33))
			rd2 = YES;
		else
			rd2 = NO;
		nrxunit = 0;
		for(i=0; i<4; i++) {
			if((ra_drv[rdrx_cn][i].ra_dt == RX50) ||
			   (ra_drv[rdrx_cn][i].ra_dt == RX33))
				nrxunit++;
		}
		if((nrxunit == 1) && (maxmem < 8192)) {
		    printf("\nSingle floppy install needs 512KB memory!\n");
		    goto sl_exit;
		}
		if((nrxunit == 1) && (ldboot[0] != 'm')) {
		    printf("\nSingle floppy install must use memory disk!\n");
		    goto sl_exit;
		}
		if(nrxunit > 1) {
			root[3] = cn - 1;
			usr[3] = cn - 1;
			rxunit = cn - '1';
		} else
			rxunit = cn - '0';
	}
/*
 * Print the startup message.
 */
	pmsg(startup);
	prtc();
/*
 * Warn the user about impending overwrite
 * of the system disk.
 */
	while(1) {
	    pmsg(warnmsg);
	    j = yes();
	    if(j == HELP)
		continue;
	    if(j == NO)
		goto sl_exit;
	    break;	/* YES, proceed with installation */
	}
/*
 * Get the target processor type.
 */
	while(1) {
	    proctyp = cputype;
	    printf("\nTarget processor is an 11/%d <y or n> ? ", proctyp);
	    j = yes();
	    if(j == HELP) {
		pmsg(pt_help);
		continue;
	    }
	    if(j == NO) {
		while(1) {
		    pmsg(pt_ask);
		    printf("\nTarget processor type ? ");
		    gets(line);
		    proctyp = atoi(line);
		    switch(proctyp) {
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
			printf("\n\nSorry - 11/%d not supported!\n", proctyp);
			continue;
		    }
		break;
		}
	    }
	    break;	/* YES, target CPU is an 11/?? */
	}
	cpuname[0] = proctyp/10 + '0';
	cpuname[1] = proctyp%10 + '0';
	printf("\nCURRENT CPU = 11/%d, TARGET CPU = 11/%d\n", cputype, proctyp);
/*
 * Get system disk type,
 * vadidate it,
 * set sd_info pointer.
 */
	while(1) {
	    printf("\nSystem disk type <? for help> ? ");
	    gets(line);
	    if(match(line, "?") || match(line, "help")) {
		printf("\nPlease enter the generic name of your system disk.");
		printf("\nSelect from the following list of supported disks:\n");
		for(i=0; sd_info[i].sd_name; i++) {
			if((i & 7) == 0)
				printf("\n");
			printf("%s ", sd_info[i].sd_name);
		}
		printf("\n");
		continue;
	    }
	    sdtype = -1;
	    for(i=0; sd_info[i].sd_name; i++)
		if(match(sd_info[i].sd_name, line)) {
		    sdtype = i;
		    break;
		}
	    if(sdtype < 0) {
		printf("\nSorry - %s disk not supported!\n", line);
		continue;
	    }
	    break;
	}
#ifdef	RLLOAD
	if(strcmp("rluboot", sd_info[sdtype].sd_boot) == 0)
		rlflag = 1;
	else
		rlflag = 0;
#endif	RLLOAD
#ifdef	RXLOAD
	if(rxflag) {	/* RX50 setup instructions */
		pmsg(rxsetup);
		prtc();
	}
#endif	RXLOAD
step1:			/* Format/verify with DSKINIT (optional) */
#ifdef	COMMON
	if(sd_info[sdtype].sd_bads != BADS)
		goto step2;
fsd:
	printf("\nFormat the system disk <y or n> ? ");
	j = yes();
	if(j == HELP) {
		pmsg(fmt_help);
		goto fsd;
	}
	if(j == YES)
		format = 1;
	else
		format = 0;
	if((strcmp("rp04", sd_info[sdtype].sd_name) == 0) ||
	   (strcmp("rp05", sd_info[sdtype].sd_name) == 0) ||
	   (strcmp("rp06", sd_info[sdtype].sd_name) == 0))
		pmsg(rpwarn);
svsdm:
	printf("\nSurface verify the system disk media <y or n> ? ");
	j = yes();
	if(j == HELP) {
		pmsg(svd_help);
		goto svsdm;
	}
	if(j == NO)
		if(format) {
			i = 1;
			goto s1_fmt;
		} else
			goto step2;
s1_npat:
	printf("\nNumber of test data patterns <1 to 8> ? ");
	gets(line);
	if(match(line, "?") || match(line, "help")) {
		pmsg(np_help);
		printf("\n        Disk    Minutes per pattern");
		printf("\n        ----    -------------------");
		for(i=0; sd_info[i].sd_name; i++) {
			if(sd_info[i].sd_bads == NOBADS)
				continue;
			printf("\n        %s            %d", sd_info[i].sd_name,
				sd_info[i].sd_mpp);
		}
		pmsg(np1_help);
		goto s1_npat;
	}
	i = atoi(line);
	if((i <= 0) || (i > 8))
		goto s1_npat;
s1_fmt:
	npat[0] = i + '0';
sdi_psn:
	printf("\nPlease enter the serial number of your disk pack <? for help>: ");
	gets(pack_sn);
	if(match(pack_sn, "?") || match(pack_sn, "help")) {
		pmsg(psn_help);
		goto sdi_psn;
	}
	if(pack_sn[0] == '\0')
		sprintf(pack_sn, "123456");
sdi_rt:
	printf("\n****** INITIALIZING SYSTEM DISK MEDIA ******\n");
	if(format)
		printf("(Formatting disk - %d minutes)\n", sd_info[sdtype].sd_mpp);
	printf("(Initializing disk - %s pattern(s) @ %d minutes per pattern)\n",
		npat, sd_info[sdtype].sd_mpp);
	if(loadprog(dskinit))
		goto sl_exit;
	copyargs(0, sd_info[sdtype].sd_name);	/* disk type */
	dkunit[0] = sd_info[sdtype].sd_rdev[3];
	copyargs(1, &dkunit);	/* unit number */
/*	copyargs(1, "y");	DSKINIT always prints bad block file */
	copyargs(1, "n");	/* Add to bad block file ? */
	copyargs(1, "y");	/* Initialize pack ? */
	if(format)		/* Format pack ? */
		copyargs(1, "y");
	else
		copyargs(1, "n");
	copyargs(1, "y");	/* Use current bad block file ? */
	copyargs(1, &pack_sn);	/* DSKINIT discards SN if not needed */
	copyargs(1, npat);	/* Number of patterns ? */
	if(format)		/* Begin formatting ? */
		copyargs(1, "y");
	copyargs(2, "");
	step = RET1;
	return;
rtn1:
	switch(xitstat) {
	case NORMAL:
		printf("\n****** SYSTEM DISK INITIALIZATION COMPLETE ******\n");
		goto step3;
	case FATAL:
		if(retry("INITIALIZATION"))
			goto sdi_rt;
		else
			goto sl_exit;
	case BADPACK:
		pmsg(disk_ng);
		prtc();
		goto sdi_rt;
	}

#endif	COMMON
step2:			/* Scan for bads with BADS/RABADS program (optional) */
	if(sd_info[sdtype].sd_bads == RABADS)
		pmsg(rawarn);
s2_rt:
	scan_sao = 0;
	printf("\nScan system disk(s) for bad blocks <y or n> ? ");
	j = yes();
	if(j == HELP) {
		pmsg(ssd_help);
		goto s2_rt;
	}
	if(j == NO)
		goto step3;
	if((sd_info[sdtype].sd_sas == 0)||(sd_info[sdtype].sd_bads == RABADS))
		goto ssd_rt;	/* must scan entire pack(s) */
ssd_all:
	printf("\nScan the entire disk surface <y or n> ? ");
	j = yes();
	if(j == HELP) {
		pmsg(ssao_help);
		goto ssd_all;
	}
	if(j == NO) {
		scan_sao = 1;
		sprintf(nblk, "%u\n", sd_info[sdtype].sd_sas);
	}
ssd_rt:
	printf("\n****** SCANNING SYSTEM DISK(s) FOR BAD BLOCKS ******\n");
	if(sd_info[sdtype].sd_bads == RABADS) {
		if(loadprog(rabads))
			goto sl_exit;
	} else {
		if(loadprog(bads))
			goto sl_exit;
	}
	copyargs(0, sd_info[sdtype].sd_name);	/* Disk type */
	dkunit[0] = sd_info[sdtype].sd_rdev[3];
	copyargs(1, &dkunit);	/* Unit number ? */
	if(sd_info[sdtype].sd_bads == BADS)
		copyargs(1, "y");	/* Print bad block file */
	if(sd_info[sdtype].sd_bads != RABADS)
		copyargs(1, "y");	/* Scan pack for bad blocks */
	copyargs(1, "0");	/* Block offset: */
	if(scan_sao)		/* Number of blocks: */
		copyargs(1, &nblk);
	else
		copyargs(1, "");
#ifdef	COMMON
	/* RL01 is a special case, must scan drive 0 and 1 */
	if(strcmp("rl01", sd_info[sdtype].sd_name) == 0) {
		copyargs(1, sd_info[sdtype].sd_name);
		dkunit[0] = sd_info[sdtype].sd_udev[3];
		copyargs(1, &dkunit);
		if(sd_info[sdtype].sd_bads == BADS)
			copyargs(1, "y");
		copyargs(1, "y");
		copyargs(1, "0");
		copyargs(1, "");
	}
#endif	COMMON
	copyargs(2, "");
	step = RET2;
	return;
rtn2:
	switch(xitstat) {
	case NORMAL:
		printf("\n****** BAD BLOCK SCAN COMPLETE ******\n");
		goto step3;
	case FATAL:
		if(retry("BAD BLOCK SCAN"))
			goto ssd_rt;
		else
			goto sl_exit;
	case NO_BBF:	/* no bad block file */
		pmsg(no_bfile);
		prtc();
		goto step1;
	case HASBADS:
		if(sd_info[sdtype].sd_bads == RABADS) {
			pmsg(rabdsk);
			goto sl_exit;
		}
		if(sd_info[sdtype].sd_bads == BADS)
			goto revect;
		if(sd_info[sdtype].sd_bads == NOBADS) {
			pmsg(baddisk);
			prtc();
			goto ssd_rt;
		}
/*
 * TODO: all this removed because we will never get here,
 *	   due to the new disk layouts.
		printf("\n****** CAUTION - BAD BLOCK(s) DETECTED ******\n");
		printf("\nIf there are any bad blocks located in the system");
		printf("\narea, that is, bad blocks numbered less than %u,",
			sd_info[sdtype].sd_sas);
		printf("\nthen this disk CANNOT be used with ");
		printf("the ULTRIX-11 system.\n");
		printf("\nBad blocks numbered %u or higher are in the user",
			sd_info[sdtype].sd_sas);
		printf("\narea. These bad blocks will not interfere with the");
		printf("\nnormal functioning of the ULTRIX-11 operating system.");
		printf("\nHowever, they may adversely affect user file storage");
		printf("\ndepending on their exact location in the user area.\n");
	usedisk:
		printf("\nDo you want to use this disk <y or n> ? ");
		j = yes();
		if(j == HELP)
			goto usedisk;
		if(j == NO) {
			printf("\nReplace the defective disk and repeat the");
			printf(" bad block scan.");
			prtc();
			goto ssd_rt;
		}
*/
		goto step3;
 revect:
		printf("\n****** BAD BLOCK(s) DETECTED ******\n");
		printf("\nAssuming the disk pack has a valid bad block file,");
		printf("\nall detected bad blocks will automatically be");
		printf("\nreplaced by alternate blocks.\n");
		printf("\nBefore using this disk with the ULTRIX-11 system,");
		printf("\nyou should surface verify it with the dskinit program.\n");
	usedisk:
		printf("\nDo you want to use this disk <y or n> ? ");
		j = yes();
		if(j == HELP)
			goto usedisk;
		if(j == NO) {
			printf("\nRestart the installation, answer yes to the");
			printf("\nsurface verify question.\n");
			goto sl_exit;
		}
	}
step3:			/* Make ROOT file system */
	printf("\n\n****** MAKING EMPTY (ROOT) FILE SYSTEM ******\n\n");
	if(loadprog(mkfs)) 
		goto sl_exit;
	copyargs(0, sd_info[sdtype].sd_rsize);
	copyargs(1, sd_info[sdtype].sd_name);
	copyargs(1, cpuname);
	copyargs(1, "root");
	sprintf(volname, "sd_%.2s%c", sd_info[sdtype].sd_rdev,
		(sd_info[sdtype].sd_rdev[3] - rlflag));
	copyargs(1, &volname);
	copyargs(2, sd_info[sdtype].sd_rdev);
	step = RET3;
	return;
rtn3:
	if(xitstat == NORMAL) {
		printf("\n****** EMPTY FILE SYSTEM COMPLETED ******\n");
		goto step4;
	}
	if(retry("MAKE FILE SYSTEM"))
		goto step3;
	else
		goto sl_exit;

step4:			/* Restore ROOT onto system disk */
#ifdef	RXLOAD
	if(rxflag) {
		if((nrxunit == 1) && (bootout == 0)) {
			rflop("BOOT");
			bootout++;
		}
		pmsg(mnt_root);
		pmsg(mnt_comn);
		iflop("ROOT #1");
	}
#endif	RXLOAD
	printf("\n\n****** RESTORING (ROOT) ONTO SYSTEM DISK ******\n\n");
	if(loadprog(restor)) 
		goto sl_exit;
	copyargs(0, root);
	if(rxflag) {
		copyargs(1, "50");
		copyargs(1, "");
	}
	copyargs(1, sd_info[sdtype].sd_rdev);
	copyargs(2, "");
	step = RET4;
	return;
rtn4:
	if(xitstat == NORMAL) {
#ifdef	RXLOAD
		if(rxflag)
			rflop("ROOT #9");
#endif	RXLOAD
		printf("\n****** FILE SYSTEM RESTORE COMPLETE ******\n");
		goto step5;
	}
	if(retry("FILE SYSTEM RESTORE")) {
#ifdef	RXLOAD
		if(rxflag)
			rflop("ROOT #?");
#endif	RXLOAD
		goto step3;
	} else
		goto sl_exit;

step5:			/* Check ROOT file system with ICHECK program */
	printf("\n\n****** CHECKING (ROOT) FILE SYSTEM ******\n\n");
	if(loadprog(icheck)) 
		goto sl_exit;
	copyargs(0, sd_info[sdtype].sd_rdev);
	copyargs(2, "n");
	step = RET5;
	return;
rtn5:
	if(xitstat == NORMAL) {
		printf("\n****** FILE SYSTEM CHECK COMPLETE ******\n");
		goto step6;
	}
	if(retry("FILE SYSTEM CHECK"))
		goto step3;
	else
		goto sl_exit;

step6:			/* Make USR file system */
	printf("\n\n****** MAKING EMPTY (USR) FILE SYSTEM ******\n\n");
	if(loadprog(mkfs)) 
		goto sl_exit;
	copyargs(0, sd_info[sdtype].sd_usize);
	copyargs(1, sd_info[sdtype].sd_name);
	copyargs(1, cpuname);
	copyargs(1, "/usr");
	sprintf(volname, "sd_%.2s%c", sd_info[sdtype].sd_udev,
		(sd_info[sdtype].sd_udev[3] - rlflag));
	copyargs(1, &volname);
	copyargs(2, sd_info[sdtype].sd_udev);
	step = RET6;
	return;
rtn6:
	if(xitstat == NORMAL) {
		printf("\n****** EMPTY FILE SYSTEM COMPLETED ******\n");
		goto step7;
	}
	if(retry("MAKE FILE SYSTEM"))
		goto step6;
	else
		goto sl_exit;

step7:			/* Restore USR onto system disk */
#ifdef	RXLOAD
	if(rxflag) {
		pmsg(mnt_usr);
		pmsg(mnt_comn);
		iflop("USR #1");
	}
#endif	RXLOAD
	printf("\n\n****** RESTORING (USR) ONTO SYSTEM DISK ******\n\n");
	if(loadprog(restor)) 
		goto sl_exit;
	copyargs(0, usr);
	if(rxflag) {
		copyargs(1, "50");
		copyargs(1, "");
	}
	copyargs(1, sd_info[sdtype].sd_udev);
	copyargs(2, "");
	step = RET7;
	return;
rtn7:
	if(xitstat == NORMAL) {
#ifdef	RXLOAD
		if(rxflag)
			rflop("USR #9");
#endif	RXLOAD
		printf("\n****** FILE SYSTEM RESTORE COMPLETE ******\n");
		goto step8;
	}
	if(retry("FILE SYSTEM RESTORE")) {
#ifdef	RXLOAD
		if(rxflag)
			rflop("USR #?");
#endif	RXLOAD
		goto step6;
	} else
		goto sl_exit;

step8:			/* Check USR file system with ICHECK program */
	printf("\n\n****** CHECKING (USR) FILE SYSTEM ******\n\n");
	if(loadprog(icheck)) 
		goto sl_exit;
	copyargs(0, sd_info[sdtype].sd_udev);
	copyargs(2, "n");
	step = RET8;
	return;
rtn8:
	if(xitstat == NORMAL) {
		printf("\n****** FILE SYSTEM CHECK COMPLETE ******\n");
		goto step9;
	}
	if(retry("FILE SYSTEM CHECK"))
		goto step6;
	else
		goto sl_exit;

step9:			/* MKFS on second RD disk (removed) */
step11:			/* Copy boot to block zero of system disk */
	printf("\n\n****** COPYING BOOT TO SYSTEM DISK BLOCK ZERO ******\n");
	/* construct a boot file name, of the form hp(0,0)/mdec/hpuboot */
	strcpy(bootfn, sd_info[sdtype].sd_rdev);
	bootfn[7] = '\0';	/* CAUTION - strcpy bug */
	strcat(bootfn, "/mdec/");
	strcat(bootfn, sd_info[sdtype].sd_boot);
	if((fi = open(bootfn, 0)) < 0) {
		printf("\nCan't open %s file\n", bootfn);
		goto sl_exit;
	}
	if(read(fi, &rdbuf, 512) <= 0) {
		printf("\n%s file read error\n", bootfn);
		goto sl_exit;
	}
	close(fi);
	if((fo = open(sd_info[sdtype].sd_rdev, 1)) < 0) {
		printf("\nCan't open %s\n", sd_info[sdtype].sd_rdev);
		goto sl_exit;
	}
	lseek(fo, 0L, 0);
	if(write(fo, &rdbuf, 512) < 0) {
		printf("\n%s write error\n", sd_info[sdtype].sd_rdev);
		goto sl_exit;
	}
	/* construct text file name, of the form hp(0,0)/.setup/setup.info */
	strcpy(textfn, sd_info[sdtype].sd_rdev);
/*	printf("\n%s",textfn);	*/
	textfn[7] = '\0';	/* CAUTION - strcpy bug */
/*	printf("\n%s", textfn);	*/
	strcat(textfn, "/.setup/setup.info");
	/* open that file for read */
	if((fi = open(textfn, 0)) < 0) {
		printf("\nCan't open %s\n", textfn);
		goto sl_exit;
	}
/*
 * TRICK PLAY,
 * find the logical block number of the /.setup/setup.info
 * file from the copy if its inode in the io buffer.
 * This is a back door way of writing to a file !!!!!
 */
	tfbn_bad = 0;
	tfbn = (long)iob[fi-3].i_ino.i_addr[0];
	/* 100 is best guess at smallest ilist */
	if((tfbn <= 100L) || (tfbn >= (long)atoi(sd_info[sdtype].sd_rsize)))
		tfbn_bad++;	/* blk # out of reasonable root filsys range */
	tfbn *= 2;	/* 1KB file system: convert logical bn to physical bn */
	if(tfbn_bad || (iob[fi-3].i_ino.i_size > 512)) {
	    printf("\nsetup.info file: block number or size out of range!\n");
	    goto sl_exit;
	}
	close(fi);
	/* zero the buffer, just for neatness */
	for(i=0; i<512; i++)
		rdbuf[i] = 0;
	/*
	 * If the memory disk is used, the load device name
	 * will be md(#,#). Change it to the correct name, i.e.,
	 * the name of the device booted from.
	 */
	ldboot[0] = root[0];
	ldboot[1] = root[1];
	ldboot[3] = root[3];
	/*
	 * Load following info into buffer:
	 *	Setup phase (init to zero)
	 *	System disk generic name (rd51, rl02, etc.)
	 *	Target processor type (23, 45, etc.)
	 *	Load device file spec (??(#,0)boot)
	 */
	sprintf(rdbuf, "%d\n%s\n%d\n%s\n", 0, sd_info[sdtype].sd_name, 
		proctyp, ldboot);
	/*
	 * Put the load device name back the way it was.
	 * So boot will be loaded from the memory driver
	 * if MD is being used.
	 */
	ldboot[0] = mkfs[0];
	ldboot[1] = mkfs[1];
	ldboot[3] = mkfs[3];
	/* write buffer to /.setup/setup.info file */
	lseek(fo, (long)(tfbn*512L), 0);
	if(write(fo, &rdbuf, 512) < 0) {
		printf("\n%s write error\n", textfn);
		goto sl_exit;
	}
	close(fo);
	printf("\n\n****** BLOCK ZERO BOOT LOADED ******\n");

step12:			/* Boot ULTRIX-11 to single-user mode */
#ifdef	RLLOAD
	printf("\n\7\7\7");
	printf("Unload the ROOT & /USR distribution disk from unit zero.\n");
	printf("\nLoad the OPTIONAL SOFTWARE disk into RL02 unit zero");
	printf("\nand make sure the drive is on-line and ready.\n");
	printf("\nSet the RL unit number plugs as follows:\n");
	if(strcmp("rl01", sd_info[sdtype].sd_name) == 0) {
	    printf("\n\tRL01 unit 0 -- system disk (ROOT)");
	    printf("\n\tRL01 unit 1 -- system disk (/USR)");
	    printf("\n\tRL02 unit 2 -- OPTIONAL SOFTWARE DISK\n");
	} else if(strcmp("rl02", sd_info[sdtype].sd_name) == 0) {
	    printf("\n\tRL02 unit 0 -- system disk (ROOT and /USR)");
	    printf("\n\tRL02 unit 1 -- OPTIONAL SOFTWARE DISK\n");
	} else {
	    printf("\n\tRL02 unit 0 -- OPTIONAL SOFTWARE DISK\n");
	    /*
	     * Distr disk removed, must load boot from system disk.
	     */
	    ldboot[0] = sd_info[sdtype].sd_rdev[0];
	    ldboot[1] = sd_info[sdtype].sd_rdev[1];
	    ldboot[3] = sd_info[sdtype].sd_rdev[3];
	}
	printf("\nPlease write protect the OPTIONAL SOFTWARE disk,");
	printf("\nall other disks should be write enabled.\n");
	while(1) {
	    printf("\nReady to continue <y or n> ? ");
	    if(yes() == YES)
		break;
	}
	rl_openf = 0;	/* make sure driver learns of possible drive type changes */
#endif	RLLOAD
	printf("\n\n****** BOOTING ULTRIX-11 SYSTEM TO SINGLE-USER MODE ******\n");
	/* Load Boot: from boot device */
	if(loadprog(ldboot))
		goto sl_exit;
	/* Tell Boot: root device ID and kernel name */
	i = sd_info[sdtype].sd_rdev[1] << 8;
	i += sd_info[sdtype].sd_rdev[0];
	/* Works because variables have same address in Boot: & sdload */
	mtpi(i, &sdl_bdn);
	i = sd_info[sdtype].sd_rdev[3] - '0' - rlflag;
	mtpi(i, &sdl_bdu);
	/* Pass load device ID back to Boot: for generic kernel magtape select */
	mtpi(sdl_ldn, &sdl_ldn);
	mtpi(sdl_ldu, &sdl_ldu);
/*
 * Copy the devsw[].dv_csr from this program to 
 * back to the boot program.
 * This updates any CSR address changes, which were passed
 * from boot to sdload in the first place.
 * Confused? I don't blame you!
 */
	dpa = mfpi(0100); /* boot loc 100 has address of devsw */
	for(dp = &devsw; dp->dv_name; dp++, dpa++)
		mtpi(dp->dv_csr, (char *)&dpa->dv_csr);
	return;
}

loadprog(prog)
char	*prog;
{
register addr,s;
long phys;
long ovaddr;
unsigned phys_i;
unsigned	txtsiz,datsiz,bsssiz,ovsize;
unsigned	symsiz;
unsigned ovsizes[8];
int i,cnt;
int pdr, psz;
int sc;
int io;
char *p;
struct devsw *dp, *dpa;

	if((io = open(prog, 0)) < 0) {
		printf("\nCan't open %s file\n", prog);
		goto errxit;
	}
	lseek(io, (off_t)0, 0);
/* TODO: better way, use devsw tape flag */
	scload = 0;	/* say need to load syscall segment */
	if(strcmp(&prog[7], "boot") == 0) {	/* loading boot from tape */
		scload++;	/* don't need syscall segment with boot */
		if((strncmp(prog, "ht", 2) == 0) ||
		   (strncmp(prog, "tm", 2) == 0) ||
		   (strncmp(prog, "tk", 2) == 0) ||
		   (strncmp(prog, "ts", 2) == 0))
			for(i=0; i<512; i++)	/* skip 2 MT boot blocks */
				getw(io);
	}
	magic = getw(io);
	txtsiz = getw(io);
	datsiz = getw(io);
	bsssiz = getw(io);
	symsiz = getw(io);


	switch (magic) {
	case 0407:
	case 0401:
		/*
		 * If the file has a symbol table, assume it is unix
		 * which can't be loaded.
		 */

		if(symsiz) {
			printf("\nCan't load ( %o) unix files\n", magic);
		errxit:
			close(io);
			return(1);
		}
		/*
		 * If loading a 407 type file on a separate I & D space
		 * type CPU, disable separate I & D space.
		 */
		if(sepid) {
			setseg(0);
			snsid();	/* disable sep I & D space */
		}
		/*
		 * Space over the remainder of the header.
		 * We do this instead of seeking
		 * because the input might be a tape which doesn't know 
		 * how to seek.
		 */
		getw(io); getw(io); getw(io);
		phys_i = txtsiz+datsiz;
		for (addr = 0; addr != phys_i; addr += 2)
			mtpi(getw(io),addr);
		clrseg(addr, bsssiz);
		close(io);
		if(strcmp("rabads", &prog[7]) == 0) { /* loading rabads */
/*
 * Copy the devsw[].dv_csr from this program to 
 * to the rabads program (no syscall segment).
 * This updates any CSR address changes.
 */
			dpa = mfpi(0100); /* rabads loc 100 has address of devsw */
			for(dp = &devsw; dp->dv_name; dp++, dpa++)
				mtpi(dp->dv_csr, (char *)&dpa->dv_csr);
			scload++;	/* don't need syscall with rabads */
		}
		if(scload)
			return(0);	/* syscall not needed */
/*
 * Load /sas/syscall file at 64KB,
 * system calls for all 407 type standalone programs.
 */
		sc = open(syscall, 0);
		if(sc < 0) {
			printf("\nCan't open %s file\n", p);
		errxit1:
			close(sc);
			return(1);
		}
		setseg(02000);	/* 64 KB */
		lseek(sc, (off_t)0, 0);
		magic = getw(sc);
		txtsiz = getw(sc);
		datsiz = getw(sc);
		bsssiz = getw(sc);
		symsiz = getw(sc);
		getw(sc); getw(sc); getw(sc);
		phys_i = txtsiz+datsiz;
		for (addr = 0; addr != phys_i; addr += 2)
			mtpi(getw(sc),addr);
/*
 * Copy the devsw[].dv_csr from this program to 
 * the syscall segment of the stand-alone program.
 * This updates any CSR address changes.
 */
		dpa = mfpi(0);	/* syscall loc 0 has address of devsw */
		for(dp = &devsw; dp->dv_name; dp++, dpa++)
			mtpi(dp->dv_csr, (char *)&dpa->dv_csr);
/* Clears entire program not just bss segment ????? */
/* Not needed anyway. */
/*		clrseg(addr, bsssiz);	*/
		setseg(0);
		if(sepid)
			snsid();	/* Insure KISA7 set to I/O space */
/*
 * MUST LOAD SYSCALL EVERY TIME (OH HECK DARN!)
 * This is because there is no clean way to clear
 * the RA open flag (ra_openf). Thus, causing the second
 * program (using RA disks) loaded to fail because the RA
 * driver does not init the MSCP controller.
 * The code remains in the hope that it will get fixed someday.
 */
/*		scload++;	/* say syscall file already loaded */
		close(sc);
		return(0);

	default:
		printf("Can't load %o files\n", magic);
		goto errxit;
	}
}

/*
 * Copy arguments into a local argument buffer,
 * then copy that buffer to the program argument buffer.
 *
 * init	0 = init buffer & pointer + copy arg
 *	1 = just copy arg
 *	2 = copy arg, close & copy buffer
 */
char	*ap;


copyargs(init, arg)
int	init;
char	*arg;
{
	char	*p;
	int	i;

	if(init == 0) {
		args[0] = 1;
		for(i=1; i<(ARGSIZ/2); i++)
			args[i] = 0;
		ap = (char *)&args[1];
	}
	p = arg;
	while(*p)
		*ap++ = *p++;
	*ap++ = '\r';
	if(init == 2)
		for(i=0; i<(ARGSIZ/2); i++)
			mtpi(args[i], ARGBUF+(i*2));
}

match(s1,s2)
register char *s1,*s2;
{
	register cc;

	cc = DIRSIZ;
	while (cc--) {
		if (*s1 != *s2)
			return(0);
		if (*s1++ && *s2++)
			continue; else
			return(1);
	}
	return(1);
}

yes()
{
yorn:
	gets(line);
	if(match(line, "yes") || match(line, "y"))
		return(YES);
	else if(match(line, "no") || match(line, "n"))
		return(NO);
	else if(match(line, "?") || match(line, "help"))
		return(HELP);
	else {
		printf("\nPlease answer y or n ? ");
		goto yorn;
	}
}

retry(str)
char	*str;
{
	register int i;

rt:
	printf("\n%s FAILED: try again <y or n> ? ", str);
	i = yes();
	if(i == HELP) {
		pmsg(rt_help);
		goto rt;
	} else if(i == YES)
		return(1);
	else
		return(0);
}

pmsg(str)
char	**str;
{
	int i;

	for(i=0; str[i]; i++)
		printf("\n%s", str[i]);
}

prtc()
{
	printf("\n\nPress <RETURN> to continue:");
	while(getchar() != '\n') ;
}

#ifdef	RXLOAD
iflop(fn)
char	*fn;
{
	printf("\n\7\7\7Insert (%s) diskette into RX%d unit %d",
		fn, ra_drv[rdrx_cn][rxunit].ra_dt, rxunit);
	if(nrxunit > 1)
		printf(" %s", rxpos(rxunit));
	prtc();
}

rflop(fn)
char	*fn;
{
	printf("\n\7\7\7Remove (%s) diskette from RX%d unit %d",
		fn, ra_drv[rdrx_cn][rxunit].ra_dt, rxunit);
	if(nrxunit > 1)
		printf(" %s", rxpos(rxunit));
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
#endif	RXLOAD
