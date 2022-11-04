
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static	char	Sccsid[] = "@(#)msf.c	3.1	3/26/87";
/*
 *
 *  File name:
 *
 *	msf.c
 *
 *  Source file description:
 *
 *	ULTRIX-11 Make Special Files program (msf).
 *
 *	This program makes, or optionally removes, the special files for a
 *	device. Everything is keyed off the device's generic name, i.e.,
 *	rp06, dh11, lp11, etc. This makes it easy for the user to make the
 *	special files needed to access a device.
 *
 *	The special files are created in the /dev directory. There is no way
 *	to specify an alternate directory.
 *
 *	The msf command can be used interactively, which is recommended for
 *	users. It also accepts device names and other information on the
 *	command line, so it can be called by other programs or shell scripts.
 *
 *	CAUTION!, this program depends on /usr/include/sys/devmaj.h. If the
 *	order of major device numbers changes, msf must be recompiled.
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
 *		/etc/msf	(program prompts for device name, etc.)
 *
 *	Non-interactive usage:
 *
 *		/etc/msf [-t] [-r] [-f #] device [-c #] unit [tty##]
 *
 *		-t	print the device name/information table
 *		-r	remove instead of create special files
 *		-f #	remove/create only disk partition #
 *			(# = all for all partitions)
 *		device	device's generic name (rp06, dh11, lp11, etc.)
 *		-c #	# = MSCP/MASSBUS controller number (default = 0)
 *		unit	for maus, ptty, kl, & dl, unit = number of lines
 *			for all others, unit = unit number or all for all units
 *			(no default)
 *		tty##	first /dev/tty## assigned to a communications device
 *			or pseudo TTY (default = 0)
 *
 *  Compile:
 *
 *	cd /usr/src/cmd; make msf
 *
 *  Modification History:
 *
 *	18 July 1985
 *		File created -- Fred Canter
 *
 */

#include <stdio.h>
#include <pwd.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/devmaj.h>

/*
 * Help messages
 */

char	*help[] =
{
	"The /etc/msf (Make Special Files) command creates or removes a",
	"device's special files. Refer to chapter 1 of the ULTRIX-11",
	"System Management Guide for a description of device special files.",
	"",
	"To execute a command, type the first letter of the command then",
	"press <RETURN>. The command will execute, then return to the command",
	"prompt. The create and remove commands prompt you for the device",
	"name and other information. Each of the create or remove command",
	"prompts include on-line help, activated by typing a ?.",
	"",
	"The msf commands are:",
	"",
	"CTRL/C	Abort current command and return to the command prompt.",
	"help	Print this help message (? also prints help message).",
	"exit	Exit from the make special files program.",
	"table	Prints the device name/information table.",
	"create	Create some or all of a device's special files.",
	"remove	Remove some or all of a device's special files.",
	"",
	0
};

char	*h_devnam[] =
{
	"Enter the device's generic name, using only lowercase characters.",
	"The generic name is the real name for the device as opposed to its",
	"ULTRIX-11 mnemonic, for example, rp06 vs hp. Use the table command",
	"to list the names of all supported devices.",
	"",
	0
};

char	*h_cn[] =
{
	"The MASSBUS disk driver supports up to three RH11/RH70 MASSBUS",
	"disk controllers. The MSCP disk driver supports up to four MSCP",
	"(UDA50 KDA50 KLESI RQDX RUX1) disk controllers. The controller",
	"number is encoded in the special files for MASSBUS/MSCP disks.",
	"",
	"The MASSBUS disks are:",
	"	RH11/RH70 - RP04/5/6, RM02/3/5, and ML11",
	"",
	"The MSCP disks are:",
	"	RX50 RX33 RD31 RD32 RD51 RD52 RD53 RD54 RC25 RA60 RA80 RA81",
	"",
	"The controller number is determined by the order in which the",
	"controllers were specified during the system generation, that is,",
	"controller zero was entered first, controller one second and",
	"controller two third, etc.",
	"",
	0
};

char	*h_units[] =
{
	"For SLUs (Single Line Units), such as the DL11, enter the number",
	"of single line units. This determines how may /dev/tty## files are",
	"to be created/removed.",
	"",
	"For pseudo TTYs, enter the number of /dev/pty?? /dev/tty?? pairs",
	"to be created/removed.",
	"",
	"For maus (multiple access user space), enter the number of maus",
	"special files needed. The maus files are /dev/maus# (# is 0 - 7).",
	"A /dev/maus# file is needed for each maus map entry in the kernel.",
	"The number of maus map entries is specified during system generation.",
	"",
	"The word all may be used to specify that all possible /dev/tty files",
	"are to be created. This is not recommend for /dev/tty files, because",
	"a large number of unnecessary files could be created, or more files",
	"than desired could be removed.",
	"",
	0
};

char	*h_unit[] =
{
	"Enter the unit number of the device for which the special files are",
	"to be created or removed. The word all may be used to specify that",
	"the special files for all possible units are to be created/removed.",
	"The \"all\" feature should be used with caution, because large numbers",
	"of files can be created (tm03, 64 units times 6 files/unit). Also,",
	"more than the desired number of files could be removed.",
	"",
	0
};

char	*h_ttyn[] =
{
	"For communications devices and pseudo TTYs, the starting TTY number",
	"specifies the first /dev/tty## or /dev/pty?? /dev/tty?? pair to be",
	"created/removed. The starting TTY number must be specified because",
	"there may be existing /dev/tty## files.",
	"",
	"For example, a DH11 with a starting tty number of 0 would create the",
	"files /dev/tty00 -> /dev/tty15 (DH11 has 16 lines). If the starting",
	"TTY number was 16, the /dev/tty16 -> /dev/tty31 files get created.",
	"",
	0
};

char	*h_stdpar[] =
{
	"Some disks do not use all eight possible disk partitions. The msf",
	"program's device information table specifies which partitions are",
	"used for each type of disk. If a non standard disk partition layout",
	"is being used, you may need to override the partition usage data",
	"in the device information table.",
	"",
	"Answer yes if you are using the standard disk partition layout.",
	"This will be the case for most systems. Msf will only create or",
	"remove special files for partitions actually used by the disk.",
	"",
	"Answer no if you are using a nonstandard disk partition layout and",
	"you need special files for additional disk partitions. If you",
	"answer no, the program will prompt for which partitions to use.",
	"",
	0
};

char	*h_usepar[] =
{
	"Enter the partition number for which special files are to be created",
	"or removed. If you enter a number from 0 -> 7, only the special",
	"files for that partition will be created/removed. You can enter the",
	"word all to create/remove the special files for all 8 partitions.",
	"",
	"To use a variation of the standard partitions, first create the",
	"special files for the standard partitions, then create/remove other",
	"special files one at a time.",
	"",
	0
};

char	*usage[] =
{
	"",
	"Usage:\t/etc/msf [-t] [-r] [-f #] device [-c #] unit [tty##]",
	"",
	"	-t	print the device name/information table",
	"	-r	remove special files instead of create them",
	"	-f #	remove/create only disk partition # (# can = all)",
	"	device	generic device name (rp06, dh11, lp11, etc.)",
	"	-c #	# = MSCP/MASSBUS disk controller number (default = 0)",
	"	unit	for maus, ptty & kl/dl, unit = number of lines",
	"		for all others, unit = unit number (unit can = all)",
	"	tty##	start assigning comm. device lines or pseudo ttys",
	"		at /dev/tty## (default = 0)",
	"",
	0
};

char	*bcnerr = "Invalid MSCP/MASSBUS cnltr number with -c option!\n";
char	*bfserr = "Invalid disk partition with -f option!\n";
char	*btnerr = "Invalid starting TTY number with tty## option!\n";
char	*bunerr = "Invalid unit number or number of units!\n";

/*
 * Device type info table.
 * Describes each possible device and provides
 * all info necessary to make/remove its special files.
 */

#define	NODEV	-1	/* No block major device nuber */
#define	MSCP	01	/* MSCP disk (cntlr # in minor device bits 6 & 7) */
#define	RX	02	/* RX50/RX33 (DISK_P with DISK_NP naming) */
			/* 04 was HS (rs03/4) */
#define	LPU4	010	/* MUX - # lines per unit, 4, 8, 16 */
#define	LPU8	020
#define	LPU16	040
#define	D800	0100	/* MAGTAPE - density, 800, 1600, 6250, tk50 */
#define	D1600	0200
#define	D6250	0400
#define	DTK50	01000	/* TK50 is black sheep! */
#define	CN	02000	/* MSCP/MASSBUS, multiple cntlr deivces (ask for cn) */
#define	NOSUP	04000	/* Device not supported (historical dreg) */
#define	ML11	010000	/* ML11 - special case of DISK_P */
#define	MASSBUS	020000	/* MASSBUS cpontrollers (HP HM HJ) */
#define	DISK_NP	0	/* Non partitioned disk */
#define	DISK_P	1	/* Partitioned disk */
#define	DISK_RX	2	/* RX02, requires special handling */
#define	MAGTAPE	3	/* Magnetic tape device */
#define	MUX	4	/* Communications multiplexer */
#define	SLU	5	/* Single line communications interface */
#define	LP	6	/* Line printer */
#define	CT	7	/* C/A/T (V7 phototypesetter interface) */
#define	DN	8	/* DN11 */
#define	DUDP	9	/* DU11 */
#define	PTY	10	/* Pseudo TTYs */
#define	MAUS	11	/* Multiple Access User Space files */

struct	dt_info {
	char	*dt_gname;	/* Generic device name */
	char	*dt_uname;	/* ULTRIX-11 device mnemonic */
	int	dt_units;	/* Max # of units (for SLU, max # ttys+1) */
	int	dt_type;	/* Device type (disk, tape, MUX, SLU, etc.) */
	int	dt_bmaj;	/* Block mode major device number or NODEV */
	int	dt_rmaj;	/* Raw/Character mode major device number */
	int	dt_pmask;	/* Disk partition mask (bit=0, unused partition */
	int	dt_flags;	/* Special case flags (ain't it awful!) */
} dt_info[] = {
	"rx02", "hx",  2, DISK_RX, HX_BMAJ, HX_RMAJ,    0, 0,
	"rk03", "rk",  8, DISK_NP, RK_BMAJ, RK_RMAJ,    0, 0,
	"rk05", "rk",  8, DISK_NP, RK_BMAJ, RK_RMAJ,    0, 0,
	"rl01", "rl",  4,  DISK_P, RL_BMAJ, RL_RMAJ, 0201, 0,
	"rl02", "rl",  4,  DISK_P, RL_BMAJ, RL_RMAJ, 0203, 0,
	"rk06", "hk",  8,  DISK_P, HK_BMAJ, HK_RMAJ, 0107, 0,
	"rk07", "hk",  8,  DISK_P, HK_BMAJ, HK_RMAJ, 0213, 0,
	"rp02", "rp",  8,  DISK_P, RP_BMAJ, RP_RMAJ, 0107, 0,
	"rp03", "rp",  8,  DISK_P, RP_BMAJ, RP_RMAJ, 0213, 0,
	"rp04", "hp",  8,  DISK_P, HP_BMAJ, HP_RMAJ, 0117, MASSBUS|CN,
	"rp04", "hm",  8,  DISK_P, HM_BMAJ, HM_RMAJ, 0117, MASSBUS|CN,
	"rp04", "hj",  8,  DISK_P, HJ_BMAJ, HJ_RMAJ, 0117, MASSBUS|CN,
	"rp05", "hp",  8,  DISK_P, HP_BMAJ, HP_RMAJ, 0117, MASSBUS|CN,
	"rp05", "hm",  8,  DISK_P, HM_BMAJ, HM_RMAJ, 0117, MASSBUS|CN,
	"rp05", "hj",  8,  DISK_P, HJ_BMAJ, HJ_RMAJ, 0117, MASSBUS|CN,
	"rp06", "hp",  8,  DISK_P, HP_BMAJ, HP_RMAJ, 0377, MASSBUS|CN,
	"rp06", "hm",  8,  DISK_P, HM_BMAJ, HM_RMAJ, 0377, MASSBUS|CN,
	"rp06", "hj",  8,  DISK_P, HJ_BMAJ, HJ_RMAJ, 0377, MASSBUS|CN,
	"rm02", "hp",  8,  DISK_P, HP_BMAJ, HP_RMAJ, 0217, MASSBUS|CN,
	"rm02", "hm",  8,  DISK_P, HM_BMAJ, HM_RMAJ, 0217, MASSBUS|CN,
	"rm02", "hj",  8,  DISK_P, HJ_BMAJ, HJ_RMAJ, 0217, MASSBUS|CN,
	"rm03", "hp",  8,  DISK_P, HP_BMAJ, HP_RMAJ, 0217, MASSBUS|CN,
	"rm03", "hm",  8,  DISK_P, HM_BMAJ, HM_RMAJ, 0217, MASSBUS|CN,
	"rm03", "hj",  8,  DISK_P, HJ_BMAJ, HJ_RMAJ, 0217, MASSBUS|CN,
	"rm05", "hp",  8,  DISK_P, HP_BMAJ, HP_RMAJ, 0377, MASSBUS|CN,
	"rm05", "hm",  8,  DISK_P, HM_BMAJ, HM_RMAJ, 0377, MASSBUS|CN,
	"rm05", "hj",  8,  DISK_P, HJ_BMAJ, HJ_RMAJ, 0377, MASSBUS|CN,
	"ml11", "hp",  8,  DISK_P, HP_BMAJ, HP_RMAJ, 0001, MASSBUS|CN|ML11,
	"ml11", "hm",  8,  DISK_P, HM_BMAJ, HM_RMAJ, 0001, MASSBUS|CN|ML11,
	"ml11", "hj",  8,  DISK_P, HJ_BMAJ, HJ_RMAJ, 0001, MASSBUS|CN|ML11,
	"ra60", "ra",  8,  DISK_P, RA_BMAJ, RA_RMAJ, 0277, MSCP|CN,
	"ra80", "ra",  8,  DISK_P, RA_BMAJ, RA_RMAJ, 0217, MSCP|CN,
	"ra81", "ra",  8,  DISK_P, RA_BMAJ, RA_RMAJ, 0377, MSCP|CN,
	"rc25", "rc",  8,  DISK_P, RA_BMAJ, RA_RMAJ, 0237, MSCP|CN,
	"rx33", "rx",  8,  DISK_P, RA_BMAJ, RA_RMAJ, 0200, MSCP|CN|RX,
	"rx50", "rx",  8,  DISK_P, RA_BMAJ, RA_RMAJ, 0200, MSCP|CN|RX,
	"rd31", "rd",  8,  DISK_P, RA_BMAJ, RA_RMAJ, 0341, MSCP|CN,
	"rd32", "rd",  8,  DISK_P, RA_BMAJ, RA_RMAJ, 0217, MSCP|CN,
	"rd51", "rd",  8,  DISK_P, RA_BMAJ, RA_RMAJ, 0221, MSCP|CN,
	"rd52", "rd",  8,  DISK_P, RA_BMAJ, RA_RMAJ, 0217, MSCP|CN,
	"rd53", "rd",  8,  DISK_P, RA_BMAJ, RA_RMAJ, 0217, MSCP|CN,
	"rd54", "rd",  8,  DISK_P, RA_BMAJ, RA_RMAJ, 0217, MSCP|CN,
	"tm11", "tm",  8, MAGTAPE, TM_BMAJ, TM_RMAJ,    0, D800,
	"tu10", "tm",  8, MAGTAPE, TM_BMAJ, TM_RMAJ,    0, D800,
	"te10", "tm",  8, MAGTAPE, TM_BMAJ, TM_RMAJ,    0, D800,
	"tk50", "tk",  4, MAGTAPE, TK_BMAJ, TK_RMAJ,    0, DTK50,
	"tu81", "tk",  4, MAGTAPE, TK_BMAJ, TK_RMAJ,    0, D1600|D6250,
	"ts11", "ts",  4, MAGTAPE, TS_BMAJ, TS_RMAJ,    0, D1600,
	"ts04", "ts",  4, MAGTAPE, TS_BMAJ, TS_RMAJ,    0, D1600,
	"ts05", "ts",  4, MAGTAPE, TS_BMAJ, TS_RMAJ,    0, D1600,
	"tu80", "ts",  4, MAGTAPE, TS_BMAJ, TS_RMAJ,    0, D1600,
	"tsv05","ts",  4, MAGTAPE, TS_BMAJ, TS_RMAJ,    0, D1600,
	"tsu05","ts",  4, MAGTAPE, TS_BMAJ, TS_RMAJ,    0, D1600,
	"tk25", "ts",  4, MAGTAPE, TS_BMAJ, TS_RMAJ,    0, D1600,
	"tm02", "ht", 64, MAGTAPE, HT_BMAJ, HT_RMAJ,    0, D800|D1600,
	"tm03", "ht", 64, MAGTAPE, HT_BMAJ, HT_RMAJ,    0, D800|D1600,
	"te16", "ht", 64, MAGTAPE, HT_BMAJ, HT_RMAJ,    0, D800|D1600,
	"tu16", "ht", 64, MAGTAPE, HT_BMAJ, HT_RMAJ,    0, D800|D1600,
	"tu77", "ht", 64, MAGTAPE, HT_BMAJ, HT_RMAJ,    0, D800|D1600,
	"kl11", "kl", 17,     SLU,   NODEV, CO_RMAJ,    0, 0, /* XXX */
	"dl11", "dl", 32,     SLU,   NODEV, CO_RMAJ,    0, 0, /* XXX */
	"klv11","kl", 17,     SLU,   NODEV, CO_RMAJ,    0, 0, /* XXX */
	"dlv11","dl", 32,     SLU,   NODEV, CO_RMAJ,    0, 0, /* XXX */
	"cat",  "ct",  1,      CT,   NODEV, CT_RMAJ,    0, 0,
	"ct",   "ct",  1,      CT,   NODEV, CT_RMAJ,    0, 0,
	"lp11", "lp",  1,      LP,   NODEV, LP_RMAJ,    0, 0,
	"dh11", "dh",  8,     MUX,   NODEV, DH_RMAJ,    0, LPU16,
	"dp11", "dp",  1,    DUDP,   NODEV, DU_RMAJ,    0, 0, /* yes, DU_RMAJ */
	"dhu11","uh",  8,     MUX,   NODEV, UH_RMAJ,    0, LPU16,
	"dhv11","uh",  8,     MUX,   NODEV, UH_RMAJ,    0, LPU8,
	"dn11", "dn",  4,      DN,   NODEV, DN_RMAJ,    0, 0,
	"dz11", "dz", 16,     MUX,   NODEV, DZ_RMAJ,    0, LPU8,
	"dzv11","dz", 16,     MUX,   NODEV, DZ_RMAJ,    0, LPU4,
	"dzq11","dz", 16,     MUX,   NODEV, DZ_RMAJ,    0, LPU4,
	"du11", "du",  4,    DUDP,   NODEV, DU_RMAJ,    0, NOSUP,
	"ptty", "pt",176,     PTY,   NODEV, PTC_RMAJ,   0, 0,
	"maus", "none",8,    MAUS,   NODEV, ME_RMAJ,    0,  0,
	0,
};

#define	HELP	2
#define	NOHELP	0
#define	YES	1
#define	NO	0
#define	LBSIZE	50
char	lbuf[LBSIZE];

char	dn[6];
char	devn[12];
char	ml_devn[12];
char	*daemon = "daemon";

int	imode;		/* Interactive mode */
int	rflag;		/* Remove special files instead of create them */
int	cntlr;		/* MSCP/MASSBUS controller number */
int	unit;		/* Unit number (-1 for all) or number of units */
int	ttyn;		/* Starting TTY number (tty##) */
int	fflag;		/* Use non standard partitions */
int	fpart;		/* Partitions to use (-1 for all, # >= 0 for just one) */

jmp_buf	savej;

int	sumask;

main(argc, argv)
int	argc;
char	*argv[];
{
	int	intr();
	register struct	dt_info	*dip;	/* Pointer into the dt_info table */
	register int i;
	int	cc;
	char	*p;

	sumask = umask(0);
	imode = 0;
	if(argc == 1) {		/* Interactive mode */
	    imode++;
	    printf("\nULTRIX-11 Make Special Files Program\n");
	    printf("\nFor help, type ? or help, then press <RETURN>.");
	    setjmp(savej);
	    signal(SIGINT, intr);
	    printf("\n");
	    while(1) {
		fflag = NO;
		printf("\nCommand < create exit help remove table >: ");
		if((cc = getline(NOHELP)) <= 0)
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
		case 'c':		/* create special files */
		    if(sucheck("create"))
			continue;
		    rflag = NO;
		    dip = getinfo();
		    if(unit >= 0)
			domsf(dip);
		    else
			for(unit=0; unit<dip->dt_units; unit++)
			    domsf(dip);
		    continue;
		case 'r':		/* remove special files */
		    if(sucheck("remove"))
			continue;
		    rflag = YES;
		    dip = getinfo();
		    if(unit >= 0)
			domsf(dip);
		    else
			for(unit=0; unit<dip->dt_units; unit++)
			    domsf(dip);
		    continue;
		default:
		    printf("\n%s - not a valid command!\n", lbuf);
		    continue;
		}
	    }
	}
/*
 * Non interactive mode.
 */
	signal(SIGINT, intr);
	rflag = NO;
	fflag = NO;
	unit = 0;
	cntlr = 0;
	ttyn = 0;
	for(i=1; argv[i]; i++) {
	    if(equal("-h", argv[i])) {
		pmsg(usage);
		if(argc == 2)
		    xit();
		else {
		    argc--;
		    continue;
		}
	    }
	    if(equal("-t", argv[i])) {
		dotable();
		if(argc == 2)
		    xit();
		else {
		    argc--;
		    continue;
		}
	    }
	    if(equal("-r", argv[i])) {
		rflag = YES;
		continue;
	    }
	    if(equal("-f", argv[i])) {
		fflag = YES;
		i++;
		if(equal("all", argv[i])) {
		    fpart = -1;	/* all partitions */
		    continue;
		}
		if((strlen(argv[i]) != 1) ||
		   (argv[i][0] < '0') || (argv[i][0] > '7')) {
			printf("\nmsf: %s", bfserr);
			pmsg(usage);
			errxit();
		}
		fpart = atoi(argv[i]);
		continue;
	    }
	    if(equal("-c", argv[i])) {
		/*
		 * For now, cntlr limit is 3.
		 * Check again later, when we know device type.
		 */
		i++;
		if((strlen(argv[i]) != 1) ||
		   (argv[i][0] < '0') || (argv[i][0] > '3')) {
			printf("\nmsf: %s", bcnerr);
			pmsg(usage);
			errxit();
		}
		cntlr = atoi(argv[i]);
		continue;
	    }
	    if(strncmp("tty", argv[i], 3) == 0) {
		if((strlen(argv[i]) < 4) ||
		   (strlen(argv[i]) > 6) ||
		   (argv[i][3] < '0') ||
		   (argv[i][3] > '9')) {
			printf("\nmsf: %s", btnerr);
			pmsg(usage);
			errxit();
		}
		ttyn = atoi(&argv[i][3]);
		continue;
	    }
	    if(equal("all", argv[i])) {		/* unit # = all */
		unit = -1;
		continue;
	    }
	    if((argv[i][0] >= '0') && (argv[i][0] <= '9')) {	/* unit # */
		unit = atoi(argv[i]);
		/*
		 * Max unit # is 176 (# of pttys).
		 */
		if((unit < 0) || (unit > 176)) {
		    printf("\nmsf: %s", bunerr);
		    pmsg(usage);
		    errxit();
		}
		continue;
	    } else {		/* assume device name */
		for(dip=dt_info; dip->dt_gname; dip++)
		    if(equal(argv[i], dip->dt_gname))
			break;
		if(dip->dt_gname == 0) {
		    printf("\nmsf: %s - invalid device name!\n", argv[i]);
		    pmsg(usage);
		    errxit();
		}
		continue;
	    }
	}
	if((dip->dt_type==SLU) || (dip->dt_type==PTY) || (dip->dt_type==MAUS)) {
	    if(unit == 0) {
		printf("\nmsf: SLU/PTTY/MAUS number of units missing!\n");
		pmsg(usage);
		errxit();
	    }
	}
	if(dip->dt_flags&CN) {
	    if(dip->dt_flags&MSCP)
		i = 3;
	    else
		i = 2;
	    if(cntlr > i) {
		printf("\nmsf: %d - invalid %s controller number!\n",
		    i, (dip->dt_flags&MSCP) ? "MSCP" : "MASSBUS");
		pmsg(usage);
		errxit();
	    }
	}
	if(dip->dt_flags&MASSBUS)
	    dip += cntlr;	/* point to wright cntlr (hp hm hj) */
	if(argc == 1)	/* only args were -h and/or -t */
	    xit();
	if(rflag)
	    p = "remove";
	else
	    p = "create";
	if(sucheck(p))
	    errxit();
	if(unit >= 0)
	    domsf(dip);
	else
	    for(unit=0; unit<dip->dt_units; unit++)
		domsf(dip);
	xit();
}

/*
 * Ask user for device name,
 * [cn], unit number, [tty##].
 */

getinfo()
{
	register struct dt_info *dp;
	register int j, cc;

	while(1) {
	    do
		printf("\nDevice name (? for help) < rp06, dz11, lp11, etc. >: ");
	    while((cc = getline(h_devnam)) < 0);
	    if(cc == 0)
		continue;	/* user typed <RETURN> */
	    for(dp=dt_info; dp->dt_gname; dp++)
		if(strcmp(lbuf, dp->dt_gname) == 0)
		    break;
	    if(dp->dt_gname == 0) {
		printf("\n%s - bad device name!\n", lbuf);
		continue;
	    }
	    break;
	}
	while(1) {
	    cntlr = 0;
	    if((dp->dt_flags&CN) == 0)
		break;
	    do {
		if(dp->dt_flags&MSCP)
		    printf("\nMSCP ");
		else
		    printf("\nMASSBUS ");
		printf("controller number < 0 1 2 ");
		if(dp->dt_flags&MSCP)
		    printf("3 ");
		printf("> : ");
	    } while((cc = getline(h_cn)) < 0);
	    if(cc == 0)
		continue;	/* <RETURN> */
	    if(dp->dt_flags&MSCP)
		j = '3';
	    else
		j = '2';
	    if((cc != 1) || (lbuf[0] < '0') || (lbuf[0] > j)) {
		printf("\n%s - bad controller number!\n", lbuf);
		continue;
	    }
	    cntlr = lbuf[0] - '0';
	    if(dp->dt_flags&MASSBUS)
		dp += cntlr;	/* TRICK PLAY, dt_info has entry for each cntlr */
	    break;
	}
	while(1) {
	    unit = 0;
	    if((dp->dt_type==SLU) || (dp->dt_type==PTY) || (dp->dt_type==MAUS)) {
		j = dp->dt_units;
		if(dp->dt_type == SLU)
		    j--;
		do
		    printf("\nNumber of units < %d maximum >: ", j);
		while((cc = getline(h_units)) < 0);
		if(cc == 0)
			continue;
		unit = atoi(lbuf);
		if((unit <= 0) || (unit > j)) {
		    printf("\n%s - bad number of units!\n", lbuf);
		    continue;
		}
		break;
	    } else {
		do {
		    printf("\nUnit number ");
		    if(dp->dt_units == 1) {
			printf("= 0, only one %s allowed!\n", dp->dt_gname);
			break;
		    }
		    printf("< 0 -> %d or all >: ", dp->dt_units - 1);
		} while((cc = getline(h_unit)) < 0);
		if(cc == 0)
		    continue;
		if(equal("all", lbuf)) {
		    unit = -1;
		    break;
		}
		unit = atoi(lbuf);
		if((unit < 0) || (unit > (dp->dt_units - 1))) {
		    printf("\n%s - bad unit number!\n", lbuf);
		    continue;
		}
		break;
	    }
	}
	while(1) {
	    if(dp->dt_type != DISK_P)
		break;
	    if(dp->dt_flags&RX)
		break;	/* RX50/RX33 always use only partition 7 */
	    if(dp->dt_flags&ML11)
		break;	/* No paritions on ML11 */
	    while(1) {
		printf("\nAssume standard disk partitions ");
		printf("(? for help) <y or n> ? ");
		j = yes(HELP);
		if(j == HELP) {
		    pmsg(h_stdpar);
		    continue;
		}
		if(j == YES)
		    fflag = NO;
		else
		    fflag = YES;
		break;
	    }
	    if(fflag != YES)
		break;
	    while(1) {
		do
		    printf("\n%s partitions < 0 -> 7 or all > ? ",
			rflag ? "Remove" : "Create");
		while((getline(h_usepar)) < 0);
		if(equal("all", lbuf)) {
		    fpart = -1;
		    break;
		}
		if((strlen(lbuf) != 1) || (lbuf[0] < '0') || (lbuf[0] > '7')) {
		    printf("\n%s - invalid response!\n", lbuf);
		    continue;
		}
		fpart = lbuf[0] - '0';
		break;
	    }
	    break;
	}
	while(1) {
	    ttyn = 0;
	    if((dp->dt_type != MUX) && (dp->dt_type != SLU))
		break;
	    do
		printf("\nStarting TTY number < ? for help >: ");
	    while((cc = getline(h_ttyn)) < 0);
	    if(cc == 0)
		continue;
	    ttyn = atoi(lbuf);
	    if((ttyn < 0) || (ttyn >= 100)) {
		printf("\n%s - bad TTY number!\n", lbuf);
		continue;
	    }
	    break;
	}
	return(dp);
}

/*
 * Print the device type/name table.
 */

dotable()
{
	register struct dt_info *dp;
	register int i;

	printf("\n\nNon partitioned disks:\n");
	printf("\nGeneric  ULTRIX-11  Units/  Block  Raw");
	printf("\nName     Mnemonic   Cntlr   Major  Major");
	printf("\n-------  ---------  ------  -----  -----");
	for(dp=dt_info; dp->dt_gname; dp++) {
		if((dp->dt_type != DISK_NP) && (dp->dt_type != DISK_RX))
			continue;
		printf("\n%-7s  %-9s  %-6d  %-5d  %-5d", dp->dt_gname,
			dp->dt_uname, dp->dt_units, dp->dt_bmaj, dp->dt_rmaj);
	}
	printf("\n\nPartitioned disks:\n");
	printf("\nGeneric  ULTRIX-11  Units/  Block  Raw    Partitions");
	printf("\nName     Mnemonic   Cntlr   Major  Major  Used");
	printf("\n-------  ---------  ------  -----  -----  ---------------");
	for(dp=dt_info; dp->dt_gname; dp++) {
		if(dp->dt_type != DISK_P)
			continue;
		printf("\n%-7s  %-9s  %-6d  %-5d  %-5d  ", dp->dt_gname,
			dp->dt_uname, dp->dt_units, dp->dt_bmaj, dp->dt_rmaj);
		for(i=0; i<8; i++)
			if(dp->dt_pmask&(1 << i))
				printf("%d ", i);
			else
				printf("x ");
	}
	printf("\n\nMagnetic tapes:\n");
	printf("\nGeneric  ULTRIX-11  Units/  Block  Raw    Density(s)");
	printf("\nName     Mnemonic   Cntlr   Major  Major  Supported");
	printf("\n-------  ---------  ------  -----  -----  ----------");
	for(dp=dt_info; dp->dt_gname; dp++) {
		if(dp->dt_type != MAGTAPE)
			continue;
		printf("\n%-7s  %-9s  %-6d  %-5d  %-5d  ", dp->dt_gname,
			dp->dt_uname, dp->dt_units, dp->dt_bmaj, dp->dt_rmaj);
		i = 0;
		if(dp->dt_flags&DTK50) {
			printf("N/A");
			continue;
		}
		if(dp->dt_flags&D800) {
			printf("800");
			i++;
		}
		if(dp->dt_flags&D1600) {
			if(i)
				printf("/");
			printf("1600");
			i++;
		}
		if(dp->dt_flags&D6250) {
			if(i)
				printf("/");
			printf("6250");
			i++;
		}
	}
	printf("\n\nCommunications single line units:\n");
	printf("\nGeneric  ULTRIX-11  # units  Raw");
	printf("\nName     Mnemonic   Allowed  Major");
	printf("\n-------  ---------  -------  -----");
	for(dp=dt_info; dp->dt_gname; dp++) {
		if(dp->dt_type != SLU)
			continue;
		printf("\n%-7s  %-9s  %-7d  %-5d", dp->dt_gname,
			dp->dt_uname, dp->dt_units, dp->dt_rmaj);
	}
	printf("\n\nCommunications multiplexers:\n");
	printf("\nGeneric  ULTRIX-11  # units  Raw    # lines");
	printf("\nName     Mnemonic   Allowed  Major  / unit");
	printf("\n-------  ---------  -------  -----  -------");
	for(dp=dt_info; dp->dt_gname; dp++) {
		if(dp->dt_type != MUX)
			continue;
		printf("\n%-7s  %-9s  %-7d  %-5d  ", dp->dt_gname,
			dp->dt_uname, dp->dt_units, dp->dt_rmaj);
		if(dp->dt_flags&LPU4)
			printf("4");
		if(dp->dt_flags&LPU8)
			printf("8");
		if(dp->dt_flags&LPU16)
			printf("16");
	}
	printf("\n\nMiscellaneous devices:\n");
	printf("\nGeneric  ULTRIX-11  # units  Raw    Device");
	printf("\nName     Mnemonic   Allowed  Major  Type");
	printf("\n-------  ---------  -------  -----  ------");
	for(dp=dt_info; dp->dt_gname; dp++) {
		if((dp->dt_type != LP) &&
		   (dp->dt_type != CT) &&
		   (dp->dt_type != DUDP) &&
		   (dp->dt_type != DN) &&
		   (dp->dt_type != MAUS) &&
		   (dp->dt_type != PTY))
			continue;
		printf("\n%-7s  %-9s  %-7d  %-5d  ", dp->dt_gname,
			dp->dt_uname, dp->dt_units, dp->dt_rmaj);
		if(dp->dt_type == LP)
		    printf("Line printer");
		if(dp->dt_type == PTY)
		    printf("Pseudo TTY for TCP/IP networking");
		if(dp->dt_type == CT)
		    printf("CAT phototypesetter interface");
		if(dp->dt_type == DN)
		    printf("Obsolete auto-call unit");
		if(dp->dt_type == DUDP)
		    printf("Obsolete synchronous communications");
		if(dp->dt_type == MAUS)
		    printf("Multiple Access User Space (MAUS)");
	}
	printf("\n");
}

/*
 * need funct header
 */

domsf(dp)
register struct dt_info *dp;
{
	register int i;
	struct passwd *np;
	int	err;
	int	j, k, cn, md, fsmask, mode;
	int	tline, nline;
	char	p, n;
	/* not sure of these yet */
	int	dlflag, klflag, numkl, dcflag;

	err = 0;
	switch(dp->dt_type) { /* device type tells what to do */
	case DISK_NP:
		k = unit;		/* k is minor device number */
		sprintf(&devn, "/dev/%s%d", dp->dt_uname, unit);
		unlink(devn);
		if(!rflag)
		    if(mknod(devn, 060600, ((dp->dt_bmaj << 8) | k)) < 0) {
			err++; break;
		    }
		sprintf(&devn, "/dev/r%s%d", dp->dt_uname, unit);
		unlink(devn);
		if(!rflag)
		    if(mknod(devn, 020600, ((dp->dt_rmaj << 8) | k)) < 0) {
			err++; break;
		    }
		break;
	case DISK_P:
		if(dp->dt_flags&MSCP)
		    cn = cntlr;	/* cntlr # in bits 6 & 7 of minor dev */
		else
		    cn = 0;
		if(fflag == YES) {
		    if(fpart == -1)
			fsmask = 0377;	/* make/remove all partitions */
		    else
			fsmask = (1<<fpart);	/* make/remove just one partition */
		} else
		    fsmask = dp->dt_pmask;
		for(k=0; k<8; k++) {
		    md = (unit << 3) | k;	/* minor dev */
		    md |= (cn << 6);	/* MSCP cont # */
		    if(dp->dt_flags&RX) {
			mode = 060666;
			sprintf(&devn, "/dev/%s%d", dp->dt_uname, unit);
		    } else {
			mode = 060600;
			sprintf(&devn, "/dev/%s%d%d", dp->dt_uname, unit, k);
		    }
		    if(fflag == NO)
			unlink(devn);
		    else if(fsmask & (1 << k))
			unlink(devn);
		    if(!rflag  && (fsmask & (1 << k)))
			if(mknod(devn, mode, ((dp->dt_bmaj << 8) | md)) < 0) {
			    err++; break;
			}
		    if(dp->dt_flags&RX) {
			mode = 020666;
			sprintf(&devn, "/dev/r%s%d", dp->dt_uname, unit);
		    } else {
			mode = 020600;
			sprintf(&devn, "/dev/r%s%d%d", dp->dt_uname, unit, k);
		    }
		    if(fflag == NO)
			unlink(devn);
		    else if(fsmask & (1 << k))
			unlink(devn);
		    if(!rflag && (fsmask & (1 << k)))
			if(mknod(devn, mode, ((dp->dt_rmaj << 8) | md)) <0) {
			    err++; break;
			}
		}
		/* For ML11, link /dev/ml0, and /dev/rml0 */
		if(dp->dt_flags&ML11) {
		    /* restore old name (/dev/hp00) */
		    sprintf(&devn, "/dev/%s%d%d", dp->dt_uname, unit, 0);
		    /* set up /dev/ml0 */
		    sprintf(&ml_devn, "/dev/%s%d", "ml", unit);
		    unlink(ml_devn);
		    if (!rflag) {
		        if (link(devn, ml_devn) < 0)
			    printf("Cannot link %s to %s!\n",devn, ml_devn);
		    }

		    /* restore old name (/dev/rhp00) */
		    sprintf(&devn, "/dev/r%s%d%d", dp->dt_uname, unit, 0);
		    /* set up /dev/rml0 */
		    sprintf(&ml_devn, "/dev/r%s%d", "ml", unit);
		    unlink(ml_devn);
		    if (!rflag) {
		        if (link(devn, ml_devn) < 0)
			    printf("Cannot link %s to %s!\n",devn, ml_devn);
		    }
		}
		break;
	case MAGTAPE:
		if(dp->dt_flags&D800) {
		    sprintf(&devn, "/dev/mt%d", unit);
		    unlink(devn);
		    if(dp->dt_bmaj == TM_BMAJ)	/* minor device number */
		    	md = unit;		/* tm */
		    else
		    	md = unit | 0100;	/* ht (bit 6 = 800 BPI) */
		    if(!rflag)
			if(mknod(devn, 060666, ((dp->dt_bmaj << 8) | md)) < 0) {
			    err++; break;
			}
		    sprintf(&devn, "/dev/rmt%d", unit);
		    unlink(devn);
		    if(!rflag)
			if(mknod(devn, 020666, ((dp->dt_rmaj << 8) | md)) < 0) {
			    err++; break;
			}
		    sprintf(&devn, "/dev/nrmt%d", unit);
		    unlink(devn);		/* no rewind on close */
		    md |= 0200;			/* bit 7 is no rewind on close */
		    if(!rflag)
			if(mknod(devn, 020666, ((dp->dt_rmaj << 8) | md)) < 0) {
			    err++; break;
			}
		}
		if(dp->dt_flags&D1600) {
		    md = unit;
		    sprintf(&devn, "/dev/ht%d", unit);
		    unlink(devn);
		    if(!rflag)
			if(mknod(devn, 060666, ((dp->dt_bmaj << 8) | md)) < 0) {
			    err++; break;
			}
		    sprintf(&devn, "/dev/rht%d", unit);
		    unlink(devn);
		    if(!rflag)
			if(mknod(devn, 020666, ((dp->dt_rmaj << 8) | md)) < 0) {
			    err++; break;
			}
		    sprintf(&devn, "/dev/nrht%d", unit);
		    unlink(devn);		/* no rewind on close */
		    md |= 0200;			/* bit7 says no rewind */
		    if(!rflag)
			if(mknod(devn, 020666, ((dp->dt_rmaj << 8) | md)) < 0) {
			    err++; break;
			}
		}
		if(dp->dt_flags&D6250) {
		    md = unit | 0100;
		    sprintf(&devn, "/dev/gt%d", unit);
		    unlink(devn);
		    if(!rflag)
			if(mknod(devn, 060666, ((dp->dt_bmaj << 8) | md)) < 0) {
			    err++; break;
			}
		    sprintf(&devn, "/dev/rgt%d", unit);
		    unlink(devn);
		    if(!rflag)
			if(mknod(devn, 020666, ((dp->dt_rmaj << 8) | md)) < 0) {
			    err++; break;
			}
		    sprintf(&devn, "/dev/nrgt%d", unit);
		    unlink(devn);		/* no rewind on close */
		    md |= 0200;			/* bit7 says no rewind */
		    if(!rflag)
			if(mknod(devn, 020666, ((dp->dt_rmaj << 8) | md)) < 0) {
			    err++; break;
			}
		}
		if(dp->dt_flags&DTK50) {
		    md = unit;
		    sprintf(&devn, "/dev/tk%d", unit);
		    unlink(devn);
		    if(!rflag)
			if(mknod(devn, 060666, ((dp->dt_bmaj << 8) | md)) < 0) {
			    err++; break;
			}
		    sprintf(&devn, "/dev/rtk%d", unit);
		    unlink(devn);
		    if(!rflag)
			if(mknod(devn, 020666, ((dp->dt_rmaj << 8) | md)) < 0) {
			    err++; break;
			}
		    sprintf(&devn, "/dev/nrtk%d", unit);
		    unlink(devn);		/* no rewind on close */
		    md |= 0200;			/* bit7 says no rewind */
		    if(!rflag)
			if(mknod(devn, 020666, ((dp->dt_rmaj << 8) | md)) < 0) {
			    err++; break;
			}
		}
		break;
	case MUX:
		tline = ttyn;
		if(dp->dt_flags&LPU4)
		    nline = 4;
		if(dp->dt_flags&LPU8)
		    nline = 8;
		if(dp->dt_flags&LPU16)
		    nline = 16;
		for(j=0; j<nline; j++) {
		    sprintf(&devn, "/dev/tty%02d", tline+j);
		    unlink(devn);
		    if(!rflag) {
			md = (unit*nline) + j;
			if(mknod(devn, 020666, ((dp->dt_rmaj << 8) | md)) < 0) {
			    err++; break;
		 	}
		    }
		}
		break;
/* NOTE: needs modification to support multiple LP units */
	case LP:
		sprintf(&devn, "/dev/lp");
		unlink(devn);
		unlink("/dev/rlp");
		if(!rflag) {
		    if(mknod(devn, 020200, (dp->dt_rmaj << 8)) < 0) {
			err++; break;
		    }
		    if(mknod("/dev/rlp", 020200, ((dp->dt_rmaj << 8) | 04)) < 0) {
			err++; break;
		    }
		}
		if((np = getpwnam(daemon)) == NULL) {
		    printf("\nmsf: Can't find daemon password entry!\n");
		    exit(1);
		}
		endpwent();
		chown(devn, np->pw_uid, np->pw_gid);
		chown("/dev/rlp", np->pw_uid, np->pw_gid);
		break;
/* NOTE: not tested yet! */
	case DN:
		sprintf(&devn, "/dev/dn%d", unit);
		unlink(devn);
		if(!rflag)
		    if(mknod(devn, 020666, ((dp->dt_rmaj << 8) | j)) < 0) {
			err++; break;
		    }
		break;
	case CT:
		sprintf(&devn, "/dev/cat");
		unlink(devn);
		if(!rflag)
		    if(mknod(devn, 020222, (dp->dt_rmaj << 8)) < 0) {
			err++; break;
		    }
		break;
	case DISK_RX:
/*
 * This code assumes there are only two RX02 units.
 */
		for(j=0; j<10; j++) {
		    if((j&1) != unit)
			continue;
		    sprintf(&devn, "/dev/hx%d", j);
		    unlink(devn);
		    if(!rflag)
			if(mknod(devn, 060600, ((dp->dt_bmaj << 8) | j)) < 0) {
			    err++; break;
			}
		    sprintf(&devn, "/dev/rhx%d", j);
		    unlink(devn);
		    if(!rflag)
			if(mknod(devn, 020600, ((dp->dt_rmaj << 8) | j)) < 0) {
			    err++; break;
			}
		}
		break;
/* NOTE: not tested yet! */
	case DUDP:
		sprintf(&devn, "/dev/%s%d", dp->dt_uname, unit);
		unlink(devn);
		if(!rflag)
		    if(mknod(devn, 020666, ((dp->dt_rmaj << 8) | unit)) < 0) {
			err++; break;
		    }
		break;
	case SLU:
		tline = ttyn;
		if(equal("kl", dp->dt_uname)) {
		    klflag++;
		    numkl = unit;
		    md = 1;
		} else if(equal("dl", dp->dt_uname)) {
		    dlflag++;
		    md = numkl + 1;	/* set when kl nodes made */
		} else {
		    dcflag++;
		    md = 0;
		}
		for(j=0; j<unit; j++) {
		    sprintf(&devn, "/dev/tty%02d", tline+j);
		    unlink(devn);
		    if(!rflag)
			if(mknod(devn, 020666, ((dp->dt_rmaj<<8)|(md+j))) < 0) {
			    err++; break;
			}
		}
		break;
	case PTY:
		for(j=0; j<unit; j++) {
		    p = 'p' + (j / 16);
		    if((j % 16) < 10)
			n = '0' + (j % 16);
		    else
			n = 'a' + ((j % 16) - 10);
		    sprintf(&devn, "/dev/pty%c%c", p, n);
		    unlink(devn);
		    if(!rflag)
			if(mknod(devn, 020666, ((dp->dt_rmaj<<8) | j)) < 0) {
			    err++; break;
			}
		    sprintf(&devn, "/dev/tty%c%c", p, n);
		    unlink(devn);
		    if(!rflag)
			if(mknod(devn, 020666, (((dp->dt_rmaj+1)<<8) | j)) < 0) {
			    err++; break;
			}
		}
		break;
	case MAUS:
		for(j=0; j<unit; j++) {
		    sprintf(&devn, "/dev/maus%d", j);
		    unlink(devn);
		    if(!rflag)
			if(mknod(devn, 020666, ((dp->dt_rmaj<<8) | j+8)) < 0) {
			    err++; break;
			}
		}
		break;
	default:
		printf("\nmsf: device information table format error!\n");
		break;
	}
	if(err) {
		printf("\nmsf: can't create %s special file!\n", devn);
		perror("errno");
		return(-1);
	} else
		return(0);
}

/*
 * Get a line of text from the terminal,
 * replace the newline with a NULL.
 * Return the string length (not counting the NULL).
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

sucheck(str)
char	*str;
{
	if(getuid() == 0)
		return(0);
	else {
		printf("\nOnly super-user can %s special files!\n", str);
		return(1);
	}
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
