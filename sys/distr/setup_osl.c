static char Sccsid[] = "@(#)setup_osl.c	3.1	3/28/87";

/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * TODO:
 *
 *	LIST command overflows screen on a CRT. Add Press <RETURN>
 *	for more: (if tty is a CRT).
 *
 * NICE:
 *
 *	Show the user a blocks to be loaded summary before starting the
 *	load. Allow the user to delete items from the to be loaded list,
 *	if it all will not fit in the available free space. Do a free
 *	command to show available free space.
 *
 *	All the load functions [f_*()] should return status and doload()
 *	should check status and print a load failed error if necessary.
 *
 */
/*
 *
 *  File name:
 *
 *	setup_osl.c
 *
 *  Source file description:
 *
 *	ULTRIX-11 initial setup program (optional software load).
 *
 *	TBS, when I figger it out!
 *	Created by mundging osload.c.
 *	loads opt sw, called by setup phase 2 or 3.
 *	target vs current cpu stuff.
 *
 *  Functions:
 *
 *	main()		The main is the main!
 *
 *	pmsg()		Prints a message from an array of character pointers.
 *
 *	yes(NOHELP)		Return YES, NO, HELP, depending on how the user
 *			answered a question.
 *
 *	intr()		Handles interrupt signal (<CTRL/C>).
 *
 *	retry()		Ask the user if a failed function should be retried.
 *
 *	prtc()		Handles "Press <RETURN> to continue: "
 *
 *	OTHERS?		Check above also.
 *
 *  Usage:
 *
 *	/.setup/setup_osl cpu loadev rxtype rxunit rd2
 *		cpu    - Target processsor type
 *		loadev - Distribution load device ?? (rx rl rc ht tm ts tk tu)
 *		rxtype - Floppy disk drive type (RX50 or RX33).
 *		rxunit - Floppy diak unit number (if load medis is RX50).
 *		rd2    - Second winchester disk present (changes RX unit #).
 *
 *	Called by the setup program, not intended for direct user access.
 *
 *  Compile:
 *
 *	cd /usr/sys/distr; make setup_osl
 *
 *  Modification history:
 *
 *	06 April 1985
 *		File created -- Fred Canter
 *
 *	04 May 1985
 *		Version 1.0  -- Fred Canter
 *
 *		Initial SCCS delta crated.
 *
 *	04 May 1985
 *		Version 2.0 -- Fred Canter
 *
 *		Changed SCCS delta to 2.0 and added support for loading
 *		optional software from RX50 diskettes.
 *		TODO: other?????
 *
 */

#include <sys/param.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <sys/ra_info.h>
#include <stdio.h>
#include <a.out.h>
#include <signal.h>
#include <setjmp.h>
#include <errno.h>

struct	stat	statb;
struct	stat	lstatb;

jmp_buf savej;

/*
 * General use defines.
 */
#define	SID	1
#define	NSID	0
#define	YES	1
#define	NO	0
#define	NOHELP	0
#define	YHELP	1
#define	ROOT	0
#define	USR	1
#define	SUP1	1
#define	SUP2	2
#define	SUP3	3
#define	TCSTART	0
#define	TCEND	1
#define	SINGLE	0
#define	MULTI	1

/*
 * Processor type info table.
 */
int	cputype;
int	tpi;
struct	cputyp {
	int	p_type;		/* processor type */
	int	p_sid;		/* Separate I & D space ? */
} cputyp[] = {
	23,	NSID,
	24,	NSID,
	34,	NSID,
	40,	NSID,
	44,	SID,
	45,	SID,
	53,	SID,
	55,	SID,
	60,	SID,
	70,	SID,
	73,	SID,
	83,	SID,
	84,	SID,
	0
};
/* TODO: what to do if cpu not found in table ??? */

#define	NARG	25	/* number of items allowed (very large) */

char	*cmd;		/* command to execute */
int	argp;		/* index into following list */
char	*args[NARG+1];	/* list of items for command to act on */
int	cbsetup;	/* setup_osl called by setup */
/* TODO: check size really needed! */
char	syscmd[1250];
#define	LBSIZE	250
char	lbuf[LBSIZE+1];
char	line[LBSIZE+1];
#define	GLBSIZE	20
char	glbuf[GLBSIZE];
char	*loadev;	/* Load device (rmt0 rht0 rgt0 rtk0 rrl0? rrc0?) */
int	ld_tape;	/* Load device is magtape */
int	ld_rl02;	/* Load device is RL02 */
int	ld_rx50;	/* Load device is RX50 floppy disk */
int	rxunit;		/* RX50/RX33 unit number */
int	rxtype;		/* Floppy disk drive type (RX33 or RX50) */
int	rd2;		/* Second RD disk present (needed by rxpos()) */
char	ddfile[25];	/* file name used by ddopen() and doload() */
char	mtscmd[100];	/* magtape position command used by ddopen() */

#define	EXIT	1
#define	FREE	2
#define	HELP	3
#define	RXUNIT	4
#define	RXDIR	5
#define	LIST	6
#define	LOAD	7
#define	UNLOAD	8
#define	BADCMD	-1
struct	cmdtyp {
	char	*cmd_name;	/* name of command */
	int	cmd_id;		/* command ID */
} cmdtyp[] = {
	"exit",		EXIT,
	"e",		EXIT,
	"ex",		EXIT,
	"bye",		EXIT,
	"b",		EXIT,
	"quit",		EXIT,
	"q",		EXIT,
	"free",		FREE,
	"f",		FREE,
	"fr",		FREE,
	"help",		HELP,
	"?",		HELP,
	"h",		HELP,
	"rxunit",	RXUNIT,
	"rxu",		RXUNIT,
	"rxdir",	RXDIR,
	"rxd",		RXDIR,
	"list",		LIST,
	"li",		LIST,
	"load",		LOAD,
	"lo",		LOAD,
	"unload",	UNLOAD,
	"u",		UNLOAD,
	"un",		UNLOAD,
	0,		BADCMD,
};

/*
 * Optional software load/unload control flags.
 */

#define	OS_SEL	01	/* Item selected for loading or unloading */
#define	OS_SYM	02	/* Item to be loaded with a symbolic link */

int	f_dict(), f_docprep(), f_f77(), f_games(), f_learn(), f_libsa();
int	f_plot(), f_saprog(), f_sccs(), f_spell(), f_sysgen();
int	f_usat(), f_usep(), f_uucp(), f_tcpip(), f_orphan(), f_manuals();
int	f_pascal(), f_userdev();

struct	os_info {
	char	*os_name;	/* Item name string */
	int	os_size;	/* Loading cost in Kbytes (1K blocks) */
	char	os_slink[DIRSIZ+2];	/* Symbolic link directory name */
	int	os_flags;	/* Control flags (SELECT, SYMLINK) */
	char	os_filsys;	/* Where loaded (ROOT or /USR) */
	char	os_offset;	/* Magtape file number (skip this many) */
	char	*os_tarfs;	/* TAR image name for loading optsw from RL/RC */
	int	(*os_func)();	/* Call this function to load/unload item */
	char	*os_flop;	/* Name of first diskette (may be a second) */
	char	*os_desc1;	/* First line of description */
	char	*os_desc2;	/* Optional second line of description */
} os_info[] = {
	"dict",		210,	{0}, 0,	USR,	27,  "ucmds",	f_dict,
		"DICTIONARY",
		"Spell dictionary and hash lists",
		"(Not needed unless remaking spell dictionary)",
	"docprep",	980,	{0}, 0,	USR,	24,  "ucmds",	f_docprep,
		"DOCPREP #1",
		"Document prepration software: nroff troff roff",
		"refer (+ dict/papers) tbl eqn fonts macros",
	"f77",		350,	{0}, 0,	USR,	14,  "ucmds",	f_f77,
		"F77",
		"Fortran 77 programs and libraries",
		"(f77, ratfor, structure and beautify)",
	"games",	300,	{0}, 0,	USR,	29,  "ucmds",	f_games,
		"GAMES",
		"Games (Programmer's Manual, Vol. 1, Section 6)",
		0,
	"learn",	860,	{0}, 0,	USR,	25,  "ucmds",	f_learn,
		"LEARN #1",
		"Learn scripts (Computer Aided Instruction)",
		0,
	"libsa",	60,	{0}, 0,	USR,	26,  "ucmds",	f_libsa,
		"PASCAL",
		"Library for building stand-alone programs",
		0,
	"manuals",	2100,	{0}, 0,	USR,	30,  "ucmds",	f_manuals,
		"MANUALS #1",
		"On-line ULTRIX-11 Programmer's Manual, Volume 1",
		"(for use with man(1) and catman(8) commands)",
	"orphans",	360,	{0}, 0,	USR,	28,  "ucmds",	f_orphan,
		"ORPHANS",
		"ORPHAN files: old versions of some software",
		"(refer to /usr/orphan/README for help)",
	"pascal",	250,	{0}, 0,	USR,	15,  "ucmds",	f_pascal,
		"PASCAL",
		"PASCAL interpreter, executer, and profiler",
		"(University of California at Berkeley 2.9 BSD)",
	"plot",		185,	{0}, 0,	USR,	16,  "ucmds",	f_plot,
		"PLOT",
		"Plot libraries (graphics filters and programs)",
		0,
	"saprog",	125,	{0}, 0,	ROOT,	13,  "rcmds",	f_saprog,
		"BOOT",
		"Stand-alone programs in /sas directory: scat",
		"copy icheck mkfs restor bads rabads dskinit",
	"sccs",		300,	{0}, 0,	USR,	17,  "ucmds",	f_sccs,
		"SCCS",
		"Source Code Control System",
		0,
	"spell",	175,	{0}, 0,	USR,	22,  "ucmds",	f_spell,
		"SPELL",
		"Spelling checker and associated programs",
		"(programs to rebuild hlists from dictionary)",
	"sysgen",	890,	{0}, 0,	USR,	31,  "sysgen",	f_sysgen,
		"SYSGEN #1",
		"System generation programs and files",
		0,
	"tcpip",	410,	{0}, 0,	USR,	20,  "ucmds",	f_tcpip,
		"TCP/IP",
		"TCP/IP ethernet networking software",
		"(for local area network over an ethernet)",
	"usat",		205,	{0}, 0,	USR,	18,  "ucmds",	f_usat,
		"USAT",
		"ULTRIX-11 System Acceptance Test",
		"(verifies the system is installed and working)",
	"usep",		400,	{0}, 0,	USR,	19,  "ucmds",	f_usep,
		"USEP",
		"User-mode System Exerciser Package",
		"(verifes system hardware working properly)",
	"userdev",	140,	{0}, 0, USR,	23,  "ucmds",	f_userdev,
		"DICTIONARY",
		"User written device driver sources and documentation",
		0,
	"uucp",		340,	{0}, 0,	USR,	21,  "ucmds",	f_uucp,
		"UUCP",
		"UUCP (unix to unix copy)",
		"(connect to other systems via phone or hardwire)",
	0
};

/*
 * General use strings
 */

/* TODO: remove after debug
char	*gs_more = "\nPress <RETURN> for more:";
*/
char	*gs_prtc = "\nPress <RETURN> to continue:";
char	*sl_make = "Making symbolic link";
char	gs_txerr[50];	/* TAR extract of ? files */
char	*gs_usr = "/usr";

/*
 * All help messages moved to setup_help.
 */

char	*rx_dir[] =
{
	"",
	"Diskette(s)  Contents",
	"-----------  ------------------------------------------------------",
	"BOOT         FILSYS: boot, auto-install, & stand-alone programs.",
	"ROOT 1-9     DUMP:   ULTRIX-11 ROOT file system.",
	"USR  1-9     DUMP:   ULTRIX-11 /USR file system.",
	"SYSGEN 1-4   MIXED:  (1&3 TAR, 2&4 FILSYS) System generation.",
	"USEP         TAR:    User-mode System Exerciser Package.",
	"UUCP         TAR:    UUCP store and forward networking Software.",
	"TCP/IP       TAR:    TCP/IP networking software.",
	"F77          TAR:    Fortran 77 compiler/libraries & ratfor.",
	"PASCAL       TAR:    PASCAL interpreter/executer/profiler, libsa.a.",
	"SCCS         TAR:    Source Code Control System (part of TCP/IP).",
	"PLOT         TAR:    Graphics filters and libraries (part of UUCP).",
	"USAT         TAR:    USAT, Fortran structure and beautify.",
	"DOCPREP 1-3  TAR:    Document preparation programs & libraries.",
	"SPELL        TAR:    Spelling checker & hashed dictionary files.",
	"DICTIONARY   TAR:    Spell dictionary source & user driver sources.",
	"LEARN 1&2    FILSYS: Learn programs and lesson script archives.",
	"ORPHANS      TAR:    Obsolete software, for backward compatibility.",
	"GAMES        TAR:    Games programs.",
	"MANUALS 1-5  TAR:    On-line manuals, for man(1) and catman(8).",
	"",
	0
};

struct hsub {
	char	*hs_name;
	char	*hs_msg;
} hsub[] = {
	"CTRL/C",	"h_ctrlc",
	"ctrlc",	"h_ctrlc",
	"ctrl/c",	"h_ctrlc",
	"^C",		"h_ctrlc",
	"^c",		"h_ctrlc",
	"abort",	"h_ctrlc",
	"help",		"h_help",
	"?",		"h_help",
	"usage",	"h_usage",
	"commands",	"h_cmds",
	"exit",		"h_exit",
	"bye",		"h_exit",
	"quit",		"h_exit",
	"free",		"h_free",
	"rxunit",	"h_rxunit",
	"rxdir",	"h_rxdir",
	"list",		"h_list",
	"load",		"h_load",
	"unload",	"h_unload",
	0
};

main(argc, argv)
int	argc;
char	*argv[];
{
	int	cleanup();
	register int i, j;
	int mem, cc;
	char c;
	char *p;

	if(getuid() != 0) {
		printf("\nsetup_osl: must be super-user!\n");
		xit(1);
	}
	if(argc != 6) {
		printf("\nsetup_osl: bad argument count!\n");
		xit(1);
	}
	cputype = atoi(argv[1]);
	for(tpi=0; cputyp[tpi].p_type; tpi++)
		if(cputype == cputyp[tpi].p_type)
			break;
	if(cputyp[tpi].p_type == 0) {
		printf("\nPDP11/%d processor not supported!\n", cputype);
		xit(1);
	}
/*
 * Determine if setup_osl was called by setup
 * or osload shell script. Changes the way we
 * deal with mounting file systems (symbolic links).
 */
	if(strcmp("/.setup/setup_osl", argv[0]) == 0)
		cbsetup = NO;
	else
		cbsetup = YES;
/*
 * Find out what the load device is and
 * get the density if it is magtape.
 */
	ld_rx50 = NO;
	ld_tape = NO;
	ld_rl02 = NO;
	if(strcmp("rl", argv[2]) == 0) {
		loadev = "rrl17";	/* May be changed later! */
		ld_rl02 = YES;
	} else if(strcmp("rc", argv[2]) == 0) {
		loadev = "rrc04";
	} else if(strcmp("rx", argv[2]) == 0) {
		rxtype = atoi(argv[3]);
		i = atoi(argv[4]);
		if((i < 0) || (i > 7)) {
		    printf("\nsetup_osl: bad floppy disk unit number!\n");
		    xit(1);
		}
		loadev = "rrx0";
		loadev[3] = i + '0';
		rxunit = i;
		ld_rx50 = YES;
		rd2 = atoi(argv[5]);	/*  see setup.c */
	} else if(strcmp("tk", argv[2]) == 0) {
		ld_tape = YES;
		loadev = "rtk0";
	} else if(strcmp("ht", argv[2]) == 0) {
		ld_tape = YES;
		loadev = 0;	/* set later, see getden() */
	} else if(strcmp("tm", argv[2]) == 0) {
		ld_tape = YES;
		loadev = "rmt0";
	} else if(strcmp("ts", argv[2]) == 0) {
		ld_tape = YES;
		loadev = "rht0";
	} else if(strcmp("tu", argv[2]) == 0) {
		ld_tape = YES;
		loadev = "rht0";	/* NO PLANS FOR 6250 DISTRIBUTION */
	} else {
	    printf("\nsetup_osl: bad load device!\n");
	    xit(1);
	}
	printf("\n\nULTRIX-11 SETUP: Optional Software Load Program.\n");
	printf("\nFor instructions type `help', then press <RETURN>.\n");
	/*
	 * Force all file system to be mounted.
	 * Do a mount -a, but ignore errors.
	 * All file systems must be mounted in case
	 * symbolic links used.
	 * If called by setup, no need to tell user about
	 * mounting file systems because setup will unmount them.
	 */
	if(cbsetup == NO) {
	    printf("\n\7\7\7OSLOAD: ");
	    printf("forcing mounting of all file systems!\n");
	}
	system("/etc/mount -a >/dev/null 2>&1");
	while(1) {		/* Command decode loop */
	    setjmp(savej);
	    signal(SIGINT, cleanup);
	    chdir("/");	/* make sure not in directory we are going to work on */
	    printf("\nCommand <help free rxunit rxdir list load unload exit>: ");
	    cc = getline(lbuf);
	    if(cc == 1)
		continue;
	    for(i=0; os_info[i].os_name; i++)
		os_info[i].os_flags = 0;
	    for(i=0; i<NARG+1; i++)
		args[i] = 0;
	    argp = 0;
	    cmd = lbuf;
	    for(p=lbuf; ; p++) {
		c = *p;
		if((c == ' ') || (c == '\n')) {
		    *p = '\0';
		    if(c == '\n')
			break;
		    if(c == ' ') {
			while(*++p == ' ') ;
			if(*p == '\n')
			    break;
			if(argp >= NARG) {
			    printf("\nItem limit reached - truncating line!\n");
			    break;
			}
			args[argp++] = p;
		    }
		}
	    }
	    for(i=0; cmdtyp[i].cmd_name; i++)
		if(strcmp(cmdtyp[i].cmd_name, cmd) == 0)
		    break;
	    switch(cmdtyp[i].cmd_id) {
	    case EXIT:
		xit(0);
	    case HELP:
		dohelp();
		break;
	    case FREE:
		dofree();
		break;
/* TODO: (2nd rd, top left right lower) ???? */
	    case RXUNIT:
		if(args[0] == 0)
		    rxunit = 0;
		else
		    rxunit = atoi(args[0]);
		while(1) {
		    if(rxunit == 0) {
			do
			    printf("\nFloppy disk unit number < 1 2 3 > ? ");
			while(gline("h_rx_un") <= 0);
			rxunit = atoi(glbuf);
		    }
		    if((rxunit < 0) || (rxunit > 3)) {
			rxunit = 0;
			printf("\n%d - bad unit number!\n", rxunit);
			continue;
		    }
		    break;
		}
		loadev[3] = rxunit + '0';
		break;
	    case RXDIR:
		pmsg(rx_dir);
		break;
	    case LIST:
		chklist(LIST);
		dolist();
		break;
	    case LOAD:
		getlist(LOAD);
		if(chklist(LOAD))
		    break;	/* bad or empty item list */
		sblink();
		ddmnt();
		getden();
		doload(LOAD);
		break;
	    case UNLOAD:
		getlist(UNLOAD);
		if(chklist(UNLOAD))
		    break;	/* bad or empty item list */
		doload(UNLOAD);
		break;
	    default:
		printf("\n%s - invalid command!\n", cmd);
		break;
	    }
	}
}

/*
 * Print help message.
 * Look up message name in hsub[] table,
 * then call setup_help to print message.
 */

dohelp()
{
	register int i, j;

	if(args[0] == 0)
		args[0] = "help";
	for(i=0; hsub[i].hs_name; i++)
		if(strcmp(args[0], hsub[i].hs_name) == 0)
			break;
	if(hsub[i].hs_name == 0) {
		printf("\nNo help available for `%s' subject.\n", args[0]);
		return(1);
	}
	phelp(hsub[i].hs_msg);
/*
 * TODO: remove after debug
	for(j=0; hsub[i].hs_msg[j]; j++) {
		if(hsub[i].hs_msg[j] == -1) {
			printf("%s", gs_more);
			while(getchar() != '\n') ;
			continue;
		}
		printf("\n%s", hsub[i].hs_msg[j]);
	}
 * TODO: end
 */
}

dofree()
{
	printf("\nFREE DISK SPACE (/ = ROOT, /usr = /USR):\n\n");
	system("df");
}

doload(com)
{
	register struct os_info *osp;

	for(osp=os_info; osp->os_name; osp++) {
	    if((osp->os_flags&OS_SEL) == 0)
		continue;
	    /*
	     * If item on disk, force unload for two reasons:
	     * Forces user to preserve files he/she doesn't want
	     * overwritten, and to make things clean in case item
	     * is being loaded via symbolic links.
	     */
	    if((com == LOAD) && (*osp->os_func)(LIST, osp) == YES) {
		printf("\n****** UNLOADING (%s) ******\n", osp->os_name);
		(*osp->os_func)(UNLOAD, osp);
		sync();
		chngdir("/.setup");
	    }
	    printf("\n****** %s (%s) ******\n",
		(com==LOAD) ? "LOADING" : "UNLOADING", osp->os_name);
	    sprintf(gs_txerr, "TAR extract of %s files", osp->os_name);
	    (*osp->os_func)(com, osp);	/* call f_????? to do actual work */
	    chngdir("/.setup");	/* always want to end up in /.setup directory */
	    sync();		/* can't hert */
	    /*
	     * If command was LOAD, and loading from RL02/RC25
	     * must dismount the tar image file system from /mnt.
	     * Device name loaded into `ddfile' in ddopen().
	     */
	    if((com == LOAD) && (ld_tape == NO) && (ld_rx50 == NO)) {
		if(umount(ddfile) != 0) {
		    printf("\nWARNING: dismount of distribution disk ");
		    printf("from /mnt directory failed!\n");
		}
	    }
	}
}

dolist()
{
	register struct os_info *osp;

	printf("\nItem    # K- On-  Load  Item");
	printf("\nName    Byte Disk Dir.  Description");
	printf("\n------- ---- ---- ----  -----------");
	printf("-----------------------------------");
	for(osp=os_info; osp->os_name; osp++) {
	    if((osp->os_flags&OS_SEL) == 0)
		continue;
	    printf("\n%-7s %4d ", osp->os_name, osp->os_size);
	    printf("%4s ", ((*osp->os_func)(LIST, osp) == YES) ? "yes" : "no");
	    printf("%4s  ", (osp->os_filsys == ROOT) ? "ROOT" : "/USR");
	    printf("%s", osp->os_desc1);
	    if(osp->os_desc2)
		printf("\n\t\t\t%s", osp->os_desc2);
	}
	printf("\n");
}

pmsg(str)
char	**str;
{
	register int i;

	for(i=0; str[i]; i++)
		printf("\n%s", str[i]);
}

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

char	*yn_err = "\nPlease answer yes or no ? ";
#define	YNLSIZE	10
char	yn_line[YNLSIZE];

yes(hlp)
{
	register int i;

	while(1) {
	    fflush(stdout);
	    for(i=0; i<YNLSIZE; i++) {
		yn_line[i] = getchar();
		if(yn_line[i] == '\n') {
			yn_line[i] = '\0';
			break;
		}
	    }
	    if(i > 4) {
		printf("%s", yn_err);
		continue;
	    }
	    if((strcmp(yn_line, "yes") == 0) || (strcmp(yn_line, "y") == 0))
		return(YES);
	    else if((strcmp(yn_line, "no") == 0) || (strcmp(yn_line, "n") == 0))
		return(NO);
	    else if((hlp == YHELP) &&
		    ((yn_line[0] == '?') ||
		     (strcmp(yn_line, "help") == 0) ||
		     (strcmp(yn_line, "h") == 0)))
			return(HELP);
	    else
		printf("%s", yn_err);
	}
}

/*
 * Get a line of text from the terminal,
 * replace the newline with a NULL.
 * Return the string length (not counting the NULL).
 * Use glbuf[] as the buffer and GLBSIZE as limit.
 * If ? or help is typed, print the help message, if
 * one is available, appologize if not. Return -1 after help.
 */

char	*badline = "\n\7\7\7Bad input line, please try again!\n";

gline(hlp)
char	*hlp;
{
	register int cc, ovflow;

	ovflow = 0;
	fflush(stdout);
	while(1) {
	    glbuf[0] = '\0';
	    if(fgets(&glbuf, LBSIZE, stdin) == NULL) {
		printf("%s", badline);
		return(-1);	/* reprint message */
	    }
	    for(cc=0; glbuf[cc]; cc++) ;
	    if((cc == 0) || (glbuf[0] == '\n'))
		return(0);	/* cause question to be reprinted */
	    if(glbuf[cc-1] != '\n') {	/* line too long */
		ovflow++;
		continue;
	    }
	    if(ovflow) {
		printf("\nLine too long, please try again!\n");
		return(-1);
	    }
	    cc--;
	    glbuf[cc] = '\0';
	    if((glbuf[0] == '?') || (strcmp("help", glbuf) == 0)) {
		if(hlp)
			phelp(hlp);
		else
			printf("\nSorry no additional help available!\n");
		return(-1);
	    }
	    return(cc);
	}
}

getline(buf)
char	*buf;
{
	register int i;

	for(i=0; i<LBSIZE; i++) {
		buf[i] = getchar();
		if((buf[i] >= 'A') && (buf[i] <= 'Z'))
			buf[i] |= 040;	/* force lower case */
		if(buf[i] == '\n')
			break;
	}
	if(i >= LBSIZE) {
		while((buf[i] = getchar()) != '\n') ;
		printf("\nLine length exceeded - truncating line!\n");
	}
	return(i+1);
}

getlist(com)
char	*com;
{
	register int i;
	char	*p;
	char	c;

	if(args[0])
		return;	/* items were specified with command */
	printf("\nPlease enter a list of items to ");
	switch(com) {
	case LOAD:
		printf("LOAD");
		break;
	case UNLOAD:
		printf("UNLOAD");
		break;
	}
	printf(" (? for help).\n");
	/* args[] already zeroed by command parser */
	while(1) {
	    argp = 0;
	    printf("\nList: ");
	    if(getline(line) == 1)
		continue;
	    if(line[0] == '?') {
		phelp("h_glist");
		continue;
	    }
	    i = 0;
	    while(line[i] == ' ')
		i++;
	    if(line[i] == '\n')
		continue;
	    p = &line[i];
	    for(; i<LBSIZE; i++) {
		if((line[i] != ' ') && (line[i] != '\n'))
		    continue;
		c = line[i];
		line[i] = 0;
		if(argp >= NARG) {
		    printf("\nItem buffer full, truncating line\n");
		    break;
		}
		args[argp++] = p;
		while(line[++i] == ' ') ;
		if((c == '\n') || (line[i] == '\n'))
		    break;
		p = &line[i];
	    }
	    break;
	}
}

/*
 * Check list of optional software items.
 *	Validates the name of each OS item.
 *	Sets the OS_SEL flag for each valid item.
 *	All means load/unload all items.
 */

char	cl_buf[100];

chklist(com)
{
	register struct os_info *osp;
	register int ap;
	char	*p;
	int	cnt;

	switch(com) {
	case LIST:
		p = "LIST";
		break;
	case LOAD:
		p = "LOAD";
		break;
	case UNLOAD:
		p = "UNLOAD";
		break;
	default:
		p = "????";
		break;
	}
	if(args[0] == 0) {
	    if(com == LIST) {
		for(osp=os_info; osp->os_name; osp++)
		    osp->os_flags |= OS_SEL;
		return(0);
	    }
	    printf("\n%s: item list empty!\n", p);
	    return(1);
	}
	ap = -1;
	while(1) {
	    if(args[++ap] == 0)
		return(0);
	    if(strcmp("all", args[ap]) == 0) {	/* list = all */
		cnt = 0;
		for(osp=os_info; osp->os_name; osp++) {
		    if((com == UNLOAD) || (com == LIST)) {
			osp->os_flags |= OS_SEL;
			cnt++;
			continue;
		    }
		    if((*osp->os_func)(LIST, osp) == NO) {
			osp->os_flags |= OS_SEL;
			cnt++;
		    } else {
			printf("\n(%s) already loaded: ", osp->os_name);
			printf("must unload first!\n");
		    }
		}
		if(cnt == 0)
		    return(1);	/* nothing to load */
		continue;
	    }
	    sprintf(&cl_buf, "%s", args[ap]);
	    while(1) {
		for(osp=os_info; osp->os_name; osp++)
		    if(strcmp(&cl_buf, osp->os_name) == 0)
			break;
		if(osp->os_name == 0) {
		    printf("\n\7\7\7%s: %s - not a valid item!\n", p, &cl_buf);
		    printf("\nPlease enter a valid item or press <RETURN> ");
		    printf("to skip this item.\n");
		    printf("\nItem: ");
		    if(gline(NOHELP) == 0) {
			break;
		    } else {
			sprintf(&cl_buf, "%s", &glbuf);
			continue;
		    }
		} else {
		    osp->os_flags |= OS_SEL;
		    break;
		}
	    }
	}
}

/*
 * Ask the user if any symbolic links are needed.
 * If so, get home directory name and save it in the
 * os_info.os_slink[] array.
 *
 * Default answers enclosed in [ ], user presses <RETURN> to take default.
 * Default base directory name is last used name (if there is one).
 */

sblink()
{
	register struct os_info *osp;
	register struct os_info *lastosp;
	register int i;
	char	*p;
	int	cnt, cc;
	int	linkit;

	/*
	 * Count number of items selected for loading.
	 * If none of them can be loaded via symbolic links, return.
	 */
	for(cnt=0, osp=os_info; osp->os_name; osp++)
	    if((osp->os_flags&OS_SEL) && (osp->os_filsys != ROOT))
		cnt++;
	if(cnt == 0)
	    return;
	while(1) {
	    if(cnt == 1)
		break;
	    printf("\nLoad any items with symbolic links (? for help)");
	    printf(" <y or n> ? ");
	    i = yes(YHELP);
	    if(i == HELP) {
		phelp("h_rsl");
		continue;
	    }
	    if(i == NO)
		return;
	    else
		break;
	}
	lastosp = 0;
	for(osp=os_info; osp->os_name; osp++) {
	    if((osp->os_flags&OS_SEL) == 0)
		continue;
	    if(osp->os_filsys == ROOT)
		continue;
	    while(1) {
		linkit = NO;
		do {
		    printf("\nLoad (%s) with symbolic links ", osp->os_name);
		    printf("(? for help) <y or n> ? ");
		} while((cc = gline("h_rsl")) <= 0);
		if((strcmp("n", &glbuf) == 0) || (strcmp("no", &glbuf) == 0))
		    break;
		if((strcmp("y", &glbuf) == 0) || (strcmp("yes", &glbuf) == 0)) {
		    linkit = YES;
		    break;
		}
		/* INVALID RESPONSE, continue (ask question again) */
	    }
	    if(linkit != YES)
		continue;
	    while(1) {
		do {
		    printf("\nSymbolic link base directory ");
		    if(lastosp)
			printf("(Press <RETURN> if %s): ",
			    &lastosp->os_slink);
		    else
			printf("(? for help): ");
		} while((cc = gline("h_rsld")) < 0);
		if(cc == 0) {	/* user typed <RETURN> for default directory */
		    if(lastosp)
			sprintf(&glbuf, "%s", &lastosp->os_slink);
		    else
			continue;
		}
		if(glbuf[0] == '/')
		    p = &glbuf[1];
		else
		    p = &glbuf[0];
		if(strlen(p) > DIRSIZ) {
		    printf("\nMaximum directory name length is %d characters!\n",
			DIRSIZ);
		    continue;
		}
		sprintf(&osp->os_slink, "/%s", p);
		p = &osp->os_slink;
		if(stat(p, &statb) < 0) {
		    printf("\nCan't stat %s - directory does not exist!\n", p);
		    continue;
		}
		if((statb.st_mode&S_IFMT) != S_IFDIR) {
		    printf("\n%s - is not a directory!\n", p);
		    continue;
		}
		osp->os_flags |= OS_SYM;
		lastosp = osp;	/* save last used base directory name */
		break;
	    }
	}
}

retry(str)
char	*str;
{
	printf("\n%s FAILED: ", str);
	printf("Try again <y or n> ? ");
	if(yes(NOHELP) == YES)
	    return(1);
	 else
	    return(0);
}

/*
 * Try to clean up the mess if the user
 * type <CTRL/C>.
 */

cleanup()
{
	signal(SIGINT, cleanup);
	chdir("/.setup");
	printf("\n");
	longjmp(savej, 1);
}

dosystem(cmd)
char	*cmd;
{
	register int s;

	s = system(cmd);
/*	if((s == SIGINT) || (s == 01400))	*/
	if((s & 0377) == SIGINT)
		cleanup();
	else
		return(s);
}


xit(s)
{
	if(cbsetup == NO) {
	    printf("\n\n\7\7\7OSLOAD: following file systems remain mounted!\n\n");
	    system("/etc/mount");
	    printf("\nIf the system is in single-user mode, you should");
	    printf("\nexecute \"/etc/umount -a\" before going multiuser.");
	}
	printf("\n");
	exit(s);
}

/*
 * Open the distribution device, the close it.
 * Print an error message and ask the user if we
 * should retry, if the open fails.
 * If the load device is magtape,
 * this has a side effect of rewinging the tape.
 * If the distribution device is magtape, use the dd
 * command to position the tape at the proper tar image.
 *
 * files - number of tape files to skip.
 * loadev - magtape file name (rmt0 for 800 BPI, rht0 for 1600 BPI).
 * ld_tape - load device is (is not) magtape.
 */

ddopen(osp, flop)
register struct os_info *osp;
char	*flop;
{
	register int i;

	sprintf(ddfile, "/dev/%s", loadev);
	while(1) {
	    if(ld_rx50 == YES)
		iflop(flop);
/* TODO: need to verify correct diskette inserted! */
	    if((i = open(ddfile, 0)) >= 0)	/* rewind the tape */
		close(i);
	    else {
		if(retry("Open of distribution device"))
		    continue;
		else
		    longjmp(savej, 1);	/* go back to command prompt */
	    }
	    break;
	}
	if(ld_rx50 == YES)
	    return;
	if(ld_tape == NO) {	/* must be loading from RL02/RC25 */
				/* mount rc04 or rl07/rl17 read only */
	    sprintf(ddfile, "/dev/%s", &loadev[1]);
	    /*
	     * The umount is in case this is an error retry,
	     * the disk may already be mounted!
	     * Don't care if the umount fails!
	     */
	    umount(ddfile);
	    while(1) {
		if(mount(ddfile, "/mnt", 1) != 0) {
		    if(retry("Mount of distribution disk on /mnt directory"))
			continue;
		    else
			longjmp(savej, 1);	/* go back to command prompt */
		}
		break;
	    }
	    return;
	}
	sprintf(mtscmd,
	    "dd if=/dev/n%s of=/dev/null bs=20b files=%d > /dev/null 2>&1",
	    loadev, osp->os_offset);
	system(mtscmd);
}

/*
 * Set up the tar command used to load optional software.
 *
 * The tar command is created in three steps:
 *
 * 1.	This routine writes the basic tar command string into syscmd[].
 *	The files are extracted from the raw device when loading from
 *	magtape or RX50 diskettes. If the load device is RC25/RL02, the
 *	files are extracted from a file system on the load media thru a pipe.
 *
 * 2.	The f_??() load function appends the file/directory names, to be
 *	extracted from the load media, to the tar command string in syscmd[].
 *
 * 3.	If the load media is RC25/RL02, this routine completes the tar command
 *	string in syscmd[], other wise it just returns.
 *
 * tarcs(op, osp)
 *
 *	op  - TCSTART = load the basic tar command string.
 *	op  - TCEND   = end the tar command string (if load dev is RL/RC).
 *
 *	osp - Optional software load info structure pointer.
 */

tarcs(op, osp)
int	op;
register struct os_info *osp;
{
	char	p[50];

	if(op == TCSTART) {
	    if((ld_tape == YES) || (ld_rx50 == YES))
		sprintf(syscmd, "tar xpbf 20 /dev/%s", loadev);
	    else
		sprintf(syscmd, "cd /mnt/%s; tar cf -", osp->os_tarfs);
	} else {
	    if((ld_tape == NO) && (ld_rx50 == NO)) {
		if(osp->os_filsys == ROOT) {
		    strcat(syscmd, " | (cd /; tar xpf -)");
		} else {
		    sprintf(&p, " | (cd %s; tar xpf -)",
			osp->os_flags&OS_SYM ? (char *)&osp->os_slink : "/usr");
		    strcat(syscmd, &p);
		}
	    }
/*	    printf("\nDEBUG: %s\n", syscmd);	*/
	}
}

/*
 * The following functions (f_?????()) do the
 * actual loading/unloading of optional software.
 * They are called from doload() via their address
 * being in the os_info structure.
 *
 * osp - pointer into os_info[] structure.
 * com - command being executed.
 *	 LOAD = load optional software item.
 *	 UNLOAD = unload item (just what we loaded).
 *	 LIST = list items (best guess if item loaded or not).
 */

char	*l_dict[] =
{
	"/usr/dict/words",
	"/usr/dict/american",
	"/usr/dict/british",
	"/usr/dict/stop",
	0
};

f_dict(com, osp)
register struct os_info *osp;
{
	register int i;

	if(com == LIST) {
	    for(i=0; l_dict[i]; i++)
		if(lstat(l_dict[i], &lstatb) >= 0)
		    return(YES);
	    return(NO);
	} else if(com == UNLOAD) {
	    for(i=0; l_dict[i]; i++)
		rmfile(l_dict[i]);
	} else {
	    if(osp->os_flags&OS_SYM) {
		if(msdir(osp, "dict"))
		    return;
		sync();
		if(chngdir(&osp->os_slink))
		    return;
	    } else {
		if(chngdir(gs_usr))
		    return;
	    }
	    tarcs(TCSTART, osp);	/* Create base tar command string */
	    strcat(syscmd,
" ./dict/words ./dict/american ./dict/british ./dict/stop ");
	    tarcs(TCEND, osp);		/* End tar command string */
	    while(1) {
		ddopen(osp, osp->os_flop);
		if(dosystem(syscmd) != 0) {
		    if(retry(gs_txerr))
			continue;
		    else
			break;
		}
		/* NO SYMBOLIC LINK FOR local */
		dosystem("touch /usr/dict/local");
		chown("/usr/dict/local", 3, 3);
		chmod("/usr/dict/local", 0666);
		if(ld_rx50 == YES)
		    rflop(osp->os_flop);
		break;
	    }
	    if(osp->os_flags&OS_SYM) {
		printf("\n%ss for DICTIONARY files.\n", sl_make);
		for(i=0; l_dict[i]; i++)
		    mslink(osp, l_dict[i]);
	    }
	}
}

char	*l_docprep[] =
{
	"/usr/bin/checkmm",
	"/usr/bin/mm",
	"/usr/bin/mmt",
	"/usr/bin/mvt",
	"/usr/bin/osdd",
	"/usr/bin/refer",
	"/usr/bin/tbl",
	"/usr/bin/eqn",
	"/usr/bin/neqn",
	"/usr/bin/checkeq",
	"/usr/bin/roff",
	"/usr/bin/troff",
	"/usr/bin/nroff",
	"/usr/lib/help/term",
	"/usr/lib/help/text",
	"/usr/lib/suftab",
	"/usr/lib/refer/inv",
	"/usr/lib/refer/mkey",
	"/usr/lib/refer/hunt",
	"/usr/dict/papers/Ind.ia",
	"/usr/dict/papers/Ind.ib",
	"/usr/dict/papers/Ind.ic",
	"/usr/dict/papers/Rv7man",
	"/usr/dict/papers/runinv",
	0
};

char	*d_docprep[] =
{
	"/usr/lib/tmac",
	"/usr/lib/macros",
	"/usr/lib/me",
	"/usr/lib/ms",
	"/usr/lib/font",
	"/usr/lib/term",
	0
};

f_docprep(com, osp)
register struct os_info *osp;
{
	register int fatal, i;

	if(com == LIST) {
	    for(i=0; l_docprep[i]; i++)
		if(lstat(l_docprep[i], &lstatb) >= 0)
		    return(YES);
	    return(NO);
	} else if(com == UNLOAD) {
	    for(i=0; l_docprep[i]; i++)
		rmfile(l_docprep[i]);
	    printf("\n\7\7\7");
	    printf("Following directories (and all files) will be removed:\n\n");
	    for(i=0; d_docprep[i]; i++)
		printf("\t%s\n", d_docprep[i]);
	    printf("\nFILES: user macro packages, local modifications or files.\n");
	    usfiles(MULTI);
	    for(i=0; d_docprep[i]; i++)
		rmdirf(d_docprep[i]);
	} else {
	    if(osp->os_flags&OS_SYM) {
		if(msdir(osp, "bin"))
		    return;
		if(msdir(osp, "lib"))
		    return;
		if(msdir(osp, "lib/help"))
		    return;
		if(msdir(osp, "lib/refer"))
		    return;
		if(msdir(osp, "dict"))
		    return;
		if(msdir(osp, "dict/papers"))
		    return;
		sync();
		for(i=0; d_docprep[i]; i++) {
		    printf("\n%s: %s -> %s%s\n", sl_make, d_docprep[i],
			&osp->os_slink, &d_docprep[i][4]);
		    msdir1(osp, &d_docprep[i][5]);
		    sprintf(&syscmd, "chog bin %s%s",
			&osp->os_slink, &d_docprep[i][4]);
		    dosystem(syscmd);
		    sprintf(&syscmd, "chmod 755 %s%s",
			&osp->os_slink, &d_docprep[i][4]);
		    dosystem(syscmd);
		    if(mslink(osp, d_docprep[i]))
			return;
		}
		sync();
		if(chngdir(&osp->os_slink))
		    return;
	    } else {
		if(chngdir(gs_usr))
		    return;
	    }
	    tarcs(TCSTART, osp);	/* Create base tar command string */
	    strcat(syscmd,
" ./bin/eqn ./bin/neqn ./bin/checkeq ./bin/roff ./bin/nroff ./bin/troff");
	    strcat(syscmd,
" ./bin/checkmm ./bin/mm ./bin/mmt ./bin/mvt ./bin/osdd");
	    if(cputyp[tpi].p_sid == SID)
		strcat(syscmd, " ./bin/tbl70");
	    else
		strcat(syscmd, " ./bin/tbl40");
	    if(cputyp[tpi].p_sid == SID)
		strcat(syscmd, " ./bin/refer70");
	    else
		strcat(syscmd, " ./bin/refer40");
	    if(ld_rx50 == NO) {
		strcat(syscmd, " ./dict/papers");
		strcat(syscmd, " ./lib/help/term ./lib/help/text");
		strcat(syscmd, " ./lib/suftab");
		strcat(syscmd,
		    " ./lib/font ./lib/tmac ./lib/ms ./lib/me ./lib/term");
		strcat(syscmd, " ./lib/macros");
		strcat(syscmd, " ./lib/refer/inv ./lib/refer/mkey");
		if(cputyp[tpi].p_sid == SID)
		    strcat(syscmd, " ./lib/refer/hunt70");
		else
		    strcat(syscmd, " ./lib/refer/hunt40");
	    }
	    fatal = 0;
	    tarcs(TCEND, osp);		/* End tar command string */
	    while(1) {
		ddopen(osp, osp->os_flop);
		if(dosystem(syscmd) != 0) {
		    if(retry(gs_txerr))
			continue;
		    else {
			fatal++;
			break;
		    }
		}
		break;
	    }
	    if(fatal)
		return;
	    if(ld_rx50 == YES) {
		rflop(osp->os_flop);
		tarcs(TCSTART, osp);
		if(cputyp[tpi].p_sid == SID)
		    strcat(syscmd, " ./lib/refer/hunt70");
		else
		    strcat(syscmd, " ./lib/refer/hunt40");
		strcat(syscmd, " ./dict/papers");
		strcat(syscmd, " ./lib/refer/inv ./lib/refer/mkey");
		strcat(syscmd, " ./lib/help/term ./lib/help/text");
		strcat(syscmd, " ./lib/suftab");
		strcat(syscmd,
		    " ./lib/font ./lib/tmac ./lib/ms ./lib/me ./lib/term");
		tarcs(TCEND, osp);
		fatal = 0;
		while(1) {
		    ddopen(osp, "DOCPREP #2");
		    if(dosystem(syscmd) != 0) {
			if(retry(gs_txerr))
			    continue;
			else {
			    fatal++;
			    break;
			}
		    }
		    break;
		}
		if(fatal)
		    return;
		rflop("DOCPREP #2");
		tarcs(TCSTART, osp);
		strcat(syscmd, " ./lib/macros");
		tarcs(TCEND, osp);
		fatal = 0;
		while(1) {
		    ddopen(osp, "DOCPREP #3");
		    if(dosystem(syscmd) != 0) {
			if(retry(gs_txerr))
			    continue;
			else {
			    fatal++;
			    break;
			}
		    }
		    break;
		}
		if(fatal)
		    return;
	    }
	    if(chngdir("./bin"))
		return;
	    if(cputyp[tpi].p_sid == SID) {
		 dosystem("mv tbl70 tbl");
		 dosystem("mv refer70 refer");
		 if(chngdir("../lib/refer"))
		    return;
		 dosystem("mv hunt70 hunt");
	    } else {
		dosystem("mv tbl40 tbl");
		dosystem("mv refer40 refer");
		if(chngdir("../lib/refer"))
		    return;
		dosystem("mv hunt40 hunt");
	    }
	    dosystem("chog bin hunt");
	    chmod("hunt", 0755);
	    if(chngdir("../../bin"))
		return;
	    dosystem("chog bin tbl refer");
	    chmod("tbl", 0755);
	    chmod("refer", 0755);
	    if(ld_rx50 == YES)
		rflop("DOCPREP #3");
	    if(osp->os_flags&OS_SYM) {
		printf("\n%ss for DOCPREP files.\n", sl_make);
		for(i=0; l_docprep[i]; i++)
		    mslink(osp, l_docprep[i]);
	    }
	}
}

char	*l_f77[] =
{
	"/usr/bin/f77",
	"/usr/lib/f77_strings",
	"/usr/lib/f77pass1",
	"/usr/lib/libF77.a",
	"/usr/lib/libI77.a",
	"/usr/lib/libU77.a",
	"/usr/bin/ratfor",
	"/usr/bin/struct",
	"/usr/lib/struct/structure",
	"/usr/lib/struct/beautify",
	0
};

f_f77(com, osp)
register struct os_info *osp;
{
	register int fatal, i;

	if(com == LIST) {
	    for(i=0; l_f77[i]; i++)
		if(lstat(l_f77[i], &lstatb) >= 0)
		    return(YES);
	    return(NO);
	} else if(com == UNLOAD) {
	    for(i=0; l_f77[i]; i++)
		rmfile(l_f77[i]);
	} else {
	    if(osp->os_flags&OS_SYM) {
		if(msdir(osp, "bin"))
		    return;
		if(msdir(osp, "lib"))
		    return;
		if(msdir(osp, "lib/struct"))
		    return;
		sync();
		if(chngdir(&osp->os_slink))
		    return;
	    } else {
		if(chngdir(gs_usr))
		    return;
	    }
	    tarcs(TCSTART, osp);	/* Create base tar command string */
	    strcat(syscmd,
" ./bin/f77 ./bin/ratfor ./lib/f77_strings ./lib/libF77.a ./lib/libI77.a ");
	    strcat(syscmd, "./lib/libU77.a ");
	    if(ld_rx50 == NO)
		strcat(syscmd, "./bin/struct ./lib/struct/beautify ");
	    if(cputyp[tpi].p_sid == SID)
		strcat(syscmd, "./lib/f77pass1id");
	    else
		strcat(syscmd, "./lib/f77pass1ov");
	    if(ld_rx50 == NO) {
		if(cputyp[tpi].p_sid == SID)
		    strcat(syscmd, " ./lib/struct/structure70");
		else
		    strcat(syscmd, " ./lib/struct/structure40");
	    }
	    fatal = 0;
	    tarcs(TCEND, osp);		/* End tar command string */
	    while(1) {
		ddopen(osp, osp->os_flop);
		if(dosystem(syscmd) != 0) {
		    if(retry(gs_txerr))
			continue;
		    else {
			fatal++;
			break;
		    }
		}
		break;
	    }
	    if(fatal)
		return;
	    if(ld_rx50 == YES) {
		rflop(osp->os_flop);
		printf("\nLoading remainder of F77 from USAT diskette.\n");
		tarcs(TCSTART, osp);
		strcat(syscmd, " ./bin/struct ./lib/struct/beautify");
		if(cputyp[tpi].p_sid == SID)
		    strcat(syscmd, " ./lib/struct/structure70");
		else
		    strcat(syscmd, " ./lib/struct/structure40");
		tarcs(TCEND, osp);
		fatal = 0;
		while(1) {
		    ddopen(osp, "USAT");
		    if(dosystem(syscmd) != 0) {
			if(retry(gs_txerr))
			    continue;
			else {
			    fatal++;
			    break;
			}
		    }
		    break;
		}
		if(fatal)
		    return;
	    }
	    if(cputyp[tpi].p_sid == SID) {
		dosystem("mv ./lib/f77pass1id ./lib/f77pass1");
		dosystem("mv ./lib/struct/structure70 ./lib/struct/structure");
	    } else {
		dosystem("mv ./lib/f77pass1ov ./lib/f77pass1");
		dosystem("mv ./lib/struct/structure40 ./lib/struct/structure");
	    }
	    chmod("./lib/f77pass1", 0775);
	    dosystem("chog bin ./lib/f77pass1");
	    chmod("./lib/struct/structure", 0775);
	    dosystem("chog bin ./lib/struct/structure");
	    if(ld_rx50 == YES)
		rflop("USAT");
	    if(osp->os_flags&OS_SYM) {
		printf("\n%ss for F77 files.\n", sl_make);
		for(i=0; l_f77[i]; i++)
		    mslink(osp, l_f77[i]);
	    }
	}
}

f_games(com, osp)
register struct os_info *osp;
{
	if(com == LIST) {
	    if(lstat("/usr/games", &lstatb) >= 0)
		return(YES);
/* TODO: remove after debug
	    if(chdir("/usr/games") < 0)
		return(NO);
	    if(lstat("fortune", &lstatb) >= 0)
		return(YES);
	    if(lstat("chess", &lstatb) >= 0)
		return(YES);
	    if(lstat("ttt", &lstatb) >= 0)
		return(YES);
	    if(lstat("wump", &lstatb) >= 0)
		return(YES);
	    if(lstat("quiz", &lstatb) >= 0)
		return(YES);
*/
	    return(NO);
	} else if(com == UNLOAD) {
	    printf("\n\7\7\7");
	    printf("The /usr/games directory (and all files) will be removed!\n");
	    printf("\nFILES: any of your own games programs.\n");
	    usfiles(MULTI);
	    rmdirf("/usr/games");
	} else {
	    if(osp->os_flags&OS_SYM) {
		printf("\n%s: /usr/games -> %s/games\n",
		    sl_make, &osp->os_slink);
		if(chngdir(&osp->os_slink))
		    return;
		msdir1(osp, "games");
		dosystem("chmod 755 games");
		dosystem("chog bin games");
		if(mslink(osp, "/usr/games"))
		    return;
	    } else {
		if(chngdir(gs_usr))
		    return;
	    }
	    tarcs(TCSTART, osp);	/* Create base tar command string */
	    strcat(syscmd, " ./games");
	    tarcs(TCEND, osp);		/* End tar command string */
	    while(1) {
		ddopen(osp, osp->os_flop);
		if(dosystem(syscmd) != 0) {
		    if(retry(gs_txerr))
			continue;
		    else
			break;
		}
		if(ld_rx50 == YES)
		    rflop(osp->os_flop);
		break;
	    }
	}
}

char	*l_userdev[] =
{
	"/usr/src/userdev",
	0
};

f_userdev(com, osp)
register struct os_info *osp;
{
	if(com == LIST) {
	    if(lstat(l_userdev[0], &lstatb) >= 0)
		return(YES);
/* TODO: remove after debug
	    if(chdir(l_userdev[0]) < 0)
		return(NO);
	    if(lstat("u1.c", &lstatb) >= 0)
		return(YES);
	    if(lstat("if_n1.c", &lstatb) >= 0)
		return(YES);
	    if(lstat("userdev.doc", &lstatb) >= 0)
		return(YES);
	    if(lstat("rs04_driver.c", &lstatb) >= 0)
		return(YES);
*/
	    return(NO);
	} else if(com == UNLOAD) {
	    printf("\n\7\7\7");
	    printf("The %s directory (and all files) ", l_userdev[0]);
	    printf("will be removed!\n");
	    printf("\nFILES: user device driver sources (*.c) and ");
	    printf("documentation (*.doc).\n");
	    usfiles();
	    rmdirf(l_userdev[0]);
	} else {
	    if(osp->os_flags&OS_SYM) {
		if(msdir(osp, "src"))
		    return;
		printf("\n%s: %s -> %s/src/userdev\n",
		    sl_make, l_userdev[0], &osp->os_slink);
		msdir1(osp, "src/userdev");
		sprintf(&syscmd, "chmod 755 %s/src/userdev", &osp->os_slink);
		dosystem(syscmd);
		sprintf(&syscmd, "chog bin %s/src/userdev", &osp->os_slink);
		dosystem(syscmd);
		if(mslink(osp, l_userdev[0]))
		    return;
		if(chngdir(&osp->os_slink))
		    return;
	    } else {
		if(chngdir(gs_usr))
		    return;
	    }
	    tarcs(TCSTART, osp);	/* Create base tar command string */
	    strcat(syscmd, " ./src/userdev");
	    tarcs(TCEND, osp);		/* End tar command string */
	    while(1) {
		ddopen(osp, osp->os_flop);
		if(dosystem(syscmd) != 0) {
		    if(retry(gs_txerr))
			continue;
		    else
			break;
		}
		printf("\nUnpacking files in %s...\n", l_userdev[0]);
		if(chngdir("src/userdev"))
		    break;
		dosystem("unpack * >/dev/null 2>&1");
		if(ld_rx50 == YES)
		    rflop(osp->os_flop);
		break;
	    }
	}
}

f_learn(com, osp)
register struct os_info *osp;
{
	if(com == LIST) {
	    if(lstat("/usr/bin/learn", &lstatb) >= 0)
		return(YES);
	    if(lstat("/usr/lib/learn", &lstatb) >= 0)
		return(YES);
	    return(NO);
	} else if(com == UNLOAD) {
	    printf("\n\7\7\7");
	    printf("The /usr/lib/learn directory (and all files) ");
	    printf("will be removed!\n");
	    printf("\nFILES: log files and any local modifications or files.\n");
	    usfiles(MULTI);
	    rmfile("/usr/bin/learn");
	    rmdirf("/usr/lib/learn");
	} else {
	    if(osp->os_flags&OS_SYM) {
		if(msdir(osp, "bin"))
		    return;
		if(msdir(osp, "lib"))
		    return;
		sync();
		printf("\n%s: /usr/bin/learn -> %s/bin/learn\n",
		    sl_make, &osp->os_slink);
		if(chngdir(&osp->os_slink))
		    return;
		if(chngdir("bin"))
		    return;
		dosystem("touch learn");
		dosystem("chog bin learn");
		dosystem("chmod 755 learn");
		if(mslink(osp, "/usr/bin/learn"))
		    return;
		printf("\n%s: /usr/lib/learn -> %s/lib/learn\n",
		    sl_make, &osp->os_slink);
		if(chngdir("../lib"))
		    return;
		msdir1(osp, "lib/learn");
		dosystem("chmod 777 learn");
		dosystem("chog bin learn");
		if(mslink(osp, "/usr/lib/learn"))
		    return;
		if(chngdir(&osp->os_slink))
		    return;
	    } else {
		if(chngdir(gs_usr))
		    return;
	    }
	    if(ld_rx50 == YES) {
		r_learn(osp);
		return;
	    }
	    tarcs(TCSTART, osp);	/* Create base tar command string */
	    strcat(syscmd," ./bin/learn ./lib/learn");
	    tarcs(TCEND, osp);		/* End tar command string */
	    while(1) {
		ddopen(osp);
		if(dosystem(syscmd) != 0) {
		    if(retry(gs_txerr))
			continue;
		    else
			break;
		}
		break;
	    }
	}
}

r_learn(osp)
register struct os_info *osp;
{
	register int fatal;

	fatal = 0;
	ddopen(osp, osp->os_flop);
	sprintf(syscmd, "/dev/rx%d", rxunit);
	while(1) {
	    if(mount(syscmd, "/mnt", 1) != 0) {
		if(retry("Mount of LEARN #1 diskette on /mnt directory"))
		    continue;
		else {
		    fatal++;
		    break;
		}
	    }
	    break;
	}
	while(1) {
	    if(fatal)
		break;
	    else
		fatal = 1;
	    if(chngdir("/usr/bin"))
		break;
	    dosystem("cp /mnt/learn learn");
	    dosystem("chog bin learn");
	    chmod("learn", 0755);
	    if((osp->os_flags&OS_SYM) == 0) {
		if(chngdir("/usr/lib"))
		    break;
		makedir("learn");
		dosystem("chog bin learn");
		chmod("learn", 0777);
	    }
	    if(chngdir("/usr/lib/learn"))
		break;
	    dosystem("cp /mnt/Linfo /mnt/Xinfo .");
	    dosystem("chog bin Linfo Xinfo");
	    chmod("Linfo", 0644);
	    chmod("Xinfo", 0644);
	    makedir("log");
	    makedir("play");
	    dosystem("chog bin log play");
	    dosystem("chmod 777 log play");
	    makedir("C");
	    dosystem("chog bin C");
/* TODO: tape mode may be wrong (775 s/b 755) */
	    chmod("C", 0755);
	    if(chngdir("C"))
		break;
	    dosystem("ar x /mnt/C.a");
	    dosystem("chog bin *");
	    dosystem("chmod 644 *");
	    if(chngdir(".."))
		break;
	    makedir("editor");
	    dosystem("chog bin editor");
/* TODO: tape mode may be wrong (775 s/b 755) */
	    chmod("editor", 0755);
	    if(chngdir("editor"))
		break;
	    dosystem("ar x /mnt/editor.a");
	    dosystem("chog bin *");
	    dosystem("chmod 644 *");
	    if(chngdir(".."))
		break;
	    fatal = 0;
	    break;
	}
	umount(syscmd);
	rflop(osp->os_flop);
	if(fatal)
	    return;
	ddopen(osp, "LEARN #2");
	while(1) {
	    if(mount(syscmd, "/mnt", 1) != 0) {
		if(retry("Mount of LEARN #2 diskette on /mnt directory"))
		    continue;
		else {
		    fatal++;
		    break;
		}
	    }
	    break;
	}
	while(1) {
	    if(fatal)
		break;
	    else
		fatal = 1;
	    if(chngdir("/usr/lib/learn"))
		break;
	    dosystem("cp /mnt/lcount /mnt/tee .");
	    dosystem("chog bin lcount tee");
	    chmod("lcount", 0755);
	    chmod("tee", 0755);
	    makedir("eqn");
	    dosystem("chog bin eqn");
/* TODO: tape mode may be wrong (775 s/b 755) */
	    chmod("eqn", 0755);
	    if(chngdir("eqn"))
		break;
	    dosystem("ar x /mnt/eqn.a");
	    dosystem("chog bin *");
	    dosystem("chmod 644 *");
	    if(chngdir(".."))
		break;
	    makedir("vi");
	    dosystem("chog bin vi");
/* TODO: tape mode may be wrong (775 s/b 755) */
	    chmod("vi", 0755);
	    if(chngdir("vi"))
		break;
	    dosystem("ar x /mnt/vi.a");
	    dosystem("chog bin *");
	    dosystem("chmod 644 *");
	    if(chngdir(".."))
		break;
	    makedir("files");
	    dosystem("chog bin files");
/* TODO: tape mode may be wrong (775 s/b 755) */
	    chmod("files", 0755);
	    if(chngdir("files"))
		break;
	    dosystem("ar x /mnt/files.a");
	    dosystem("chog bin *");
	    dosystem("chmod 644 *");
	    if(chngdir(".."))
		break;
	    makedir("macros");
	    dosystem("chog bin macros");
/* TODO: tape mode may be wrong (775 s/b 755) */
	    chmod("macros", 0755);
	    if(chngdir("macros"))
		break;
	    dosystem("ar x /mnt/macros.a");
	    dosystem("chog bin *");
	    dosystem("chmod 644 *");
	    if(chngdir(".."))
		break;
	    makedir("morefiles");
	    dosystem("chog bin morefiles");
/* TODO: tape mode may be wrong (775 s/b 755) */
	    chmod("morefiles", 0755);
	    if(chngdir("morefiles"))
		break;
	    dosystem("ar x /mnt/morefiles.a");
	    dosystem("chog bin *");
	    dosystem("chmod 644 *");
	    if(chngdir(".."))
		break;
	    fatal = 0;
	    break;
	}
	umount(syscmd);
	rflop("LEARN #2");
}

char	*l_libsa[] =
{
	"/usr/lib/libsa.a",
	0
};

f_libsa(com, osp)
register struct os_info *osp;
{
	if(com == LIST) {
	    if(lstat(l_libsa[0], &lstatb) >= 0)
		return(YES);
	    else
		return(NO);
	} else if(com == UNLOAD) {
	    rmfile(l_libsa[0]);
	} else {
	    if(osp->os_flags&OS_SYM) {
		if(msdir(osp, "lib"))
		    return;
		sync();
		if(chngdir(&osp->os_slink))
		    return;
	    } else {
		if(chngdir(gs_usr))
		    return;
	    }
	    tarcs(TCSTART, osp);	/* Create base tar command string */
	    strcat(syscmd, " ./lib/libsa.a");
	    tarcs(TCEND, osp);		/* End tar command string */
	    while(1) {
		ddopen(osp, osp->os_flop);
		if(dosystem(syscmd) != 0) {
		    if(retry(gs_txerr))
			continue;
		    else
			break;
		}
		if(ld_rx50 == YES)
		    rflop(osp->os_flop);
		break;
	    }
	    if(osp->os_flags&OS_SYM) {
		printf("\n%s: %s -> %s/lib/libsa.a\n",
		    sl_make, l_libsa[0], &osp->os_slink);
		mslink(osp, l_libsa[0]);
	    }
	}
}

char	*l_plot[] =
{
	"/usr/bin/plot",
	"/usr/bin/tk",
	"/usr/bin/tek",
	"/usr/bin/tla50",
	"/usr/bin/tla100",
	"/usr/bin/tregis",
	"/usr/bin/t300",
	"/usr/bin/t300s",
	"/usr/bin/t450",
	"/usr/bin/vplot",
	"/usr/lib/libtla50.a",
	"/usr/lib/libtla100.a",
	"/usr/lib/libtregis.a",
	"/usr/lib/libtgigi.a",
	"/usr/lib/libt300.a",
	"/usr/lib/libt300s.a",
	"/usr/lib/libt4014.a",
	"/usr/lib/libt450.a",
	"/usr/lib/libplot.a",
	0
};

f_plot(com, osp)
register struct os_info *osp;
{
	register int i;

	if(com == LIST) {
	    for(i=0; l_plot[i]; i++)
		if(lstat(l_plot[i], &lstatb) >= 0)
		    return(YES);
	    return(NO);
	} else if(com == UNLOAD) {
	    for(i=0; l_plot[i]; i++)
		rmfile(l_plot[i]);
	} else {
	    if(osp->os_flags&OS_SYM) {
		if(msdir(osp, "bin"))
		    return;
		if(msdir(osp, "lib"))
		    return;
		sync();
		if(chngdir(&osp->os_slink))
		    return;
	    } else {
		if(chngdir(gs_usr))
		    return;
	    }
	    tarcs(TCSTART, osp);	/* Create base tar command string */
	    strcat(syscmd,
" ./bin/plot \
./bin/tk \
./bin/tek \
./bin/tla50 \
./bin/tla100 \
./bin/tregis \
./bin/t300 \
./bin/t300s \
./bin/t450 \
./bin/vplot ");
	    strcat(syscmd,
"./lib/libtla50.a \
./lib/libtla100.a \
./lib/libtgigi.a \
./lib/libtregis.a \
./lib/libt300.a \
./lib/libt300s.a \
./lib/libt4014.a \
./lib/libt450.a \
./lib/libplot.a");
	    tarcs(TCEND, osp);		/* End tar command string */
	    while(1) {
		ddopen(osp, osp->os_flop);
		if(dosystem(syscmd) != 0) {
		    if(retry(gs_txerr))
			continue;
		    else
			break;
		}
		if(ld_rx50 == YES)
		    rflop(osp->os_flop);
		break;
	    }
	    if(osp->os_flags&OS_SYM) {
		printf("\n%ss for PLOT files.\n", sl_make);
		for(i=0; l_plot[i]; i++)
		    mslink(osp, l_plot[i]);
	    }
	}
}

f_saprog(com, osp)
register struct os_info *osp;
{
	if(com == LIST) {
	    if(lstat("/sas/syscall", &lstatb) >= 0)
		return(YES);
	    else
		return(NO);
	} else if(com == UNLOAD) {
	    unlink("/sas/bads");
	    unlink("/sas/rabads");
	    unlink("/sas/copy");
	    unlink("/sas/dskinit");
	    unlink("/sas/icheck");
	    unlink("/sas/mkfs");
	    unlink("/sas/restor");
	    unlink("/sas/scat");
	    unlink("/sas/syscall");
	} else {
	    if(ld_rx50 == YES) {
		r_saprog(osp);
		return;
	    }
	    if(chngdir("/"))
		return;
	    tarcs(TCSTART, osp);	/* Create base tar command string */
	    strcat(syscmd,
" ./sas/bads \
./sas/rabads \
./sas/copy \
./sas/dskinit \
./sas/icheck \
./sas/mkfs \
./sas/restor \
./sas/scat \
./sas/syscall");
	    tarcs(TCEND, osp);		/* End tar command string */
	    while(1) {
		ddopen(osp);
		if(dosystem(syscmd) != 0) {
		    if(retry(gs_txerr))
			continue;
		    else
			break;
		}
		break;
	    }
	}
}

r_saprog(osp)
register struct os_info *osp;
{
	register int fatal;

	fatal = 0;
	ddopen(osp, osp->os_flop);
	sprintf(syscmd, "/dev/rx%d", rxunit);
	while(1) {
	    if(mount(syscmd, "/mnt", 1) != 0) {
		if(retry("Mount of BOOT diskette on /mnt directory"))
		    continue;
		else {
		    fatal++;
		    break;
		}
	    }
	    break;
	}
	while(1) {
	    if(fatal)
		break;
	    if(chngdir("/mnt"))
		break;
	    dosystem("cp bads rabads copy dskinit icheck mkfs /sas");
	    dosystem("cp restor scat syscall /sas");
	    dosystem("chmod 644 /sas/*");
	    chngdir("/");
	    break;
	}
	umount(syscmd);
	rflop(osp->os_flop);
}

char	*l_sccs[] =
{
	"/usr/bin/admin",
	"/usr/bin/bdiff",
	"/usr/bin/cdc",
	"/usr/bin/comb",
	"/usr/bin/delta",
	"/usr/bin/get",
	"/usr/bin/sccshelp",
	"/usr/bin/prs",
	"/usr/bin/prt",
	"/usr/bin/rmchg",
	"/usr/bin/rmdel",
	"/usr/bin/unget",
	"/usr/bin/val",
	"/usr/bin/vc",
/*	"/usr/bin/what",	(what is part of the base system)	*/
	"/usr/bin/sact",
	"/usr/bin/sccs",
	"/usr/bin/sccsdiff",
	"/usr/lib/help/ad",
	"/usr/lib/help/bd",
	"/usr/lib/help/cb",
	"/usr/lib/help/cm",
	"/usr/lib/help/cmds",
	"/usr/lib/help/co",
	"/usr/lib/help/de",
	"/usr/lib/help/default",
	"/usr/lib/help/ge",
	"/usr/lib/help/he",
	"/usr/lib/help/prs",
	"/usr/lib/help/rc",
	"/usr/lib/help/un",
	"/usr/lib/help/ut",
	"/usr/lib/help/vc",
	0
};

f_sccs(com, osp)
register struct os_info *osp;
{
	register int i;

	if(com == LIST) {
	    for(i=0; l_sccs[i]; i++)
		if(lstat(l_sccs[i], &lstatb) >= 0)
		    return(YES);
	    return(NO);
	} else if(com == UNLOAD) {
	    for(i=0; l_sccs[i]; i++)
		rmfile(l_sccs[i]);
	} else {
	    if(osp->os_flags&OS_SYM) {
		if(msdir(osp, "bin"))
		    return;
		if(msdir(osp, "lib"))
		    return;
		if(msdir(osp, "lib/help"))
		    return;
		sync();
		if(chngdir(&osp->os_slink))
		    return;
	    } else {
		if(chngdir(gs_usr))
		    return;
	    }
	    tarcs(TCSTART, osp);	/* Create base tar command string */
	    strcat(syscmd,
" ./bin/admin \
./bin/bdiff \
./bin/cdc \
./bin/comb \
./bin/delta \
./bin/get \
./bin/sccshelp \
./bin/prs \
./bin/prt \
./bin/rmchg \
./bin/rmdel \
./bin/unget \
./bin/val \
./bin/vc \
./bin/sact \
./bin/sccs \
./bin/sccsdiff ");
/* NOTE: /usr/bin/what is now part of the base system! */
	    strcat(syscmd,
" ./lib/help/ad \
./lib/help/bd \
./lib/help/cb \
./lib/help/cm \
./lib/help/cmds \
./lib/help/co \
./lib/help/de \
./lib/help/default \
./lib/help/ge \
./lib/help/he \
./lib/help/prs \
./lib/help/rc \
./lib/help/un \
./lib/help/ut \
./lib/help/vc");
	    tarcs(TCEND, osp);		/* End tar command string */
	    while(1) {
		ddopen(osp, osp->os_flop);
		if(dosystem(syscmd) != 0) {
		    if(retry(gs_txerr))
			continue;
		    else
			break;
		}
		if(ld_rx50 == YES)
		    rflop(osp->os_flop);
		break;
	    }
	    if(osp->os_flags&OS_SYM) {
		printf("\n%ss for SCCS files.\n", sl_make);
		for(i=0; l_sccs[i]; i++)
		    mslink(osp, l_sccs[i]);
	    }
	}
}

char	*l_spell[] =
{
	"/usr/bin/spell",
	"/usr/lib/spell",
	"/usr/lib/spellin",
	"/usr/lib/spellout",
	"/usr/dict/hlista",
	"/usr/dict/hlistb",
	"/usr/dict/hstop",
	0
};

f_spell(com, osp)
register struct os_info *osp;
{
	register int i;

	if(com == LIST) {
	    for(i=0; l_spell[i]; i++)
		if(lstat(l_spell[i], &lstatb) >= 0)
		    return(YES);
	    return(NO);
	} else if(com == UNLOAD) {
	    for(i=0; l_spell[i]; i++)
		rmfile(l_spell[i]);
	} else {
	    if(osp->os_flags&OS_SYM) {
		if(msdir(osp, "bin"))
		    return;
		if(msdir(osp, "lib"))
		    return;
		if(msdir(osp, "dict"))
		    return;
		sync();
		if(chngdir(&osp->os_slink))
		    return;
	    } else {
		if(chngdir(gs_usr))
		    return;
	    }
	    tarcs(TCSTART, osp);	/* Create base tar command string */
	    strcat(syscmd, " ./bin/spell");
	    if(cputyp[tpi].p_sid == SID) {
		strcat(syscmd,
" ./lib/spell70 \
./lib/spellin70 \
./lib/spellout70 \
./dict/hlista70 \
./dict/hlistb70 \
./dict/hstop70");
	    } else {
		strcat(syscmd,
" ./lib/spell40 \
./lib/spellin40 \
./lib/spellout40 \
./dict/hlista40 \
./dict/hlistb40 \
./dict/hstop40");
	    }
	    tarcs(TCEND, osp);		/* End tar command string */
	    while(1) {
		ddopen(osp, osp->os_flop);
		if(dosystem(syscmd) != 0) {
		    if(retry(gs_txerr))
			continue;
		    else
			break;
		}
		if(chngdir("lib"))
		    return;
		if(cputyp[tpi].p_sid == SID) {
		    dosystem("mv spell70 spell");
		    dosystem("mv spellin70 spellin");
		    dosystem("mv spellout70 spellout");
		    if(chngdir("../dict"))
			return;
		    dosystem("mv hlista70 hlista");
		    dosystem("mv hlistb70 hlistb");
		    dosystem("mv hstop70 hstop");
		} else {
		    dosystem("mv spell40 spell");
		    dosystem("mv spellin40 spellin");
		    dosystem("mv spellout40 spellout");
		    if(chngdir("../dict"))
			return;
		    dosystem("mv hlista40 hlista");
		    dosystem("mv hlistb40 hlistb");
		    dosystem("mv hstop40 hstop");
		}
		dosystem("chog bin hlista hlistb hstop");
		dosystem("chmod 0644 hlista hlistb hstop");
		/* NO SYMBOLIC LINK FOR spellhist */
		dosystem("touch /usr/dict/spellhist");
		dosystem("chog bin /usr/dict/spellhist");
		dosystem("chmod 0666 /usr/dict/spellhist");
		if(chngdir("../lib"))
		    return;
		dosystem("chog bin spell spellin spellout");
		dosystem("chmod 0755 spell spellin spellout");
		if(ld_rx50 == YES)
		    rflop(osp->os_flop);
		break;
	    }
	    if(osp->os_flags&OS_SYM) {
		printf("\n%ss for SPELL files.\n", sl_make);
		for(i=0; l_spell[i]; i++)
		    mslink(osp, l_spell[i]);
	    }
	}
}

char	*l_pascal[] =
{
	"/usr/bin/pi",
	"/usr/bin/pix",
	"/usr/bin/px",
	"/usr/bin/pxp",
	"/usr/lib/how_pi",
	"/usr/lib/how_pix",
	"/usr/lib/how_pxp",
	"/usr/lib/npx_header",
	"/usr/lib/pi1.2strings",
	0
};

f_pascal(com, osp)
register struct os_info *osp;
{
	register int i, err;

	if(com == LIST) {
	    for(i=0; l_pascal[i]; i++)
		if(lstat(l_pascal[i], &lstatb) >= 0)
		    return(YES);
	    return(NO);
	} else if(com == UNLOAD) {
	    for(i=0; l_pascal[i]; i++)
		rmfile(l_pascal[i]);
	} else {
	    if(osp->os_flags&OS_SYM) {
		if(msdir(osp, "bin"))
		    return;
		if(msdir(osp, "lib"))
		    return;
		sync();
		if(chngdir(&osp->os_slink))
		    return;
	    } else {
		if(chngdir(gs_usr))
		    return;
	    }
	    tarcs(TCSTART, osp);	/* Create base tar command string */
	    strcat(syscmd, " ./bin/pix ./bin/pxp");
	    if(cputyp[tpi].p_sid == SID) {
		strcat(syscmd, " ./bin/pi70 ./bin/px70");
	    } else {
		strcat(syscmd, " ./bin/pi40 ./bin/px40");
	    }
	    strcat(syscmd,
" ./lib/how_pi ./lib/how_pix ./lib/how_pxp ./lib/npx_header ./lib/pi1.2strings");
	    tarcs(TCEND, osp);		/* End tar command string */
	    while(1) {
		ddopen(osp, osp->os_flop);
		if(dosystem(syscmd) != 0) {
		    if(retry(gs_txerr))
			continue;
		    else
			break;
		}
		if(chngdir("bin"))
		    return;
		if(cputyp[tpi].p_sid == SID) {
		    dosystem("mv pi70 pi");
		    dosystem("mv px70 px");
		} else {
		    dosystem("mv pi40 pi");
		    dosystem("mv px40 px");
		}
		dosystem("chog bin pi px");
		dosystem("chmod 755 pi px");
		if(ld_rx50 == YES)
		    rflop(osp->os_flop);
		break;
	    }
	    if(osp->os_flags&OS_SYM) {
		printf("\n%ss for PASCAL files.\n", sl_make);
		for(i=0; l_pascal[i]; i++)
		    mslink(osp, l_pascal[i]);
	    }
	}
}

f_sysgen(com, osp)
register struct os_info *osp;
{
	register char *p;
	int	fatal;

	if(com == LIST) {
	    if(lstat("/usr/sys", &lstatb) >= 0)
		return(YES);
/* TODO: remove after debug
	    if(lstat("/usr/sys/conf/sysgen", &lstatb) >= 0)
		return(YES);
	    if(lstat("/usr/sys/conf/mkconf", &lstatb) >= 0)
		return(YES);
*/
	    return(NO);
	} else if(com == UNLOAD) {
	    printf("\n\7\7\7");
	    printf("The /usr/sys directory (all subdirectories and files");
	    printf(" will be removed!\n");
	    printf("\nFILES: configuration files (conf/*.cf) and ");
	    printf("user device drivers.\n");
	    usfiles(MULTI);
	    rmdirf("/usr/sys");
	} else {
	    if(osp->os_flags&OS_SYM) {
		printf("\n%s: /usr/sys -> %s/sys\n",
		    sl_make, &osp->os_slink);
		if(chngdir(&osp->os_slink))
		    return;
		msdir1(osp, "sys");
		dosystem("chmod 755 sys");
		dosystem("chog sys sys");
		if(mslink(osp, "/usr/sys"))
		    return;
		sync();
	    } else {
		if(chngdir(gs_usr))
		    return;
	    }
	    tarcs(TCSTART, osp);	/* Create base tar command string */
	    strcat(syscmd, " ./sys/conf");
/*
 * TODO: remove after debug
	    strcat(syscmd, " ./sys/crt.profile ./sys/prt.profile ./sys/conf");
*/
	    if(ld_rx50 == NO) {
		if(cputyp[tpi].p_sid == SID)
		    strcat(syscmd, " ./sys/sys ./sys/dev/LIB2_id ./sys/net");
		else
		    strcat(syscmd, " ./sys/ovsys ./sys/ovdev ./sys/ovnet");
	    } else {
		if(cputyp[tpi].p_sid == SID)
		    strcat(syscmd, " ./sys/sys");
	    }
	    tarcs(TCEND, osp);		/* End tar command string */
	    while(1) {
		ddopen(osp, osp->os_flop);
		if(dosystem(syscmd) != 0) {
		    if(retry(gs_txerr))
			continue;
		    else
			break;
		}
		/* .profile saved in /usr/skel by setup */
		dosystem("cp /usr/skel/sys_profile /usr/sys/.profile");
		if(cputyp[tpi].p_sid == SID)
		    unlink("./sys/conf/mch_ov.o");
		else
		    unlink("./sys/conf/mch_id.o");
		break;
	    }
	    if(ld_rx50 == NO)
		return;
	    rflop("SYSGEN #1");
	    if(cputyp[tpi].p_sid == SID)
		printf("\nSYSGEN #2 not used with split I/D processors!\n");
	    if(cputyp[tpi].p_sid == NSID) {
		if(fmnt("SYSGEN #2"))
		    return;
		mkdir("/usr/sys/ovsys", 0755);
		mkdir("/usr/sys/ovnet", 0755);
		dosystem("chog sys /usr/sys/ovsys /usr/sys/ovnet");
		dosystem("chmod 755 /usr/sys/ovsys /usr/sys/ovnet");
		if(chngdir("/usr/sys/ovsys") == 0) {
		    system("ar x /mnt/LIB1_ov");
		    system("chog sys *");
		    system("chmod 444 *");
		    if(chngdir("../ovnet") == 0) {
			system("ar x /mnt/LIB3_ov");
			system("chog sys *");
			system("chmod 444 *");
		    }
		}
		fmnt(0);
		sync();
	    	rflop("SYSGEN #2");
		printf("\nSYSGEN #3 not used with non split I/D processors!\n");
	    }
	    if(cputyp[tpi].p_sid == SID) {
		if(osp->os_flags&OS_SYM) {
		    if(chngdir(&osp->os_slink))
			return;
		} else {
		    if(chngdir(gs_usr))
			return;
		}
		tarcs(TCSTART, osp);
		p = "SYSGEN #3";
		strcat(syscmd, " ./sys/sys ./sys/dev");
		tarcs(TCEND, osp);
		while(1) {
		    ddopen(osp, p);
		    if(dosystem(syscmd) != 0) {
			if(retry(gs_txerr))
			    continue;
			else
			    break;
		    }
		    rflop(p);
		    break;
		}
		sync();
	    }
	    if(fmnt("SYSGEN #4"))
		return;
	    fatal = 0;
	    if(cputyp[tpi].p_sid == NSID) {
		mkdir("/usr/sys/ovdev", 0755);
		dosystem("chog sys /usr/sys/ovdev");
		dosystem("chmod 755 /usr/sys/ovdev");
		if(chngdir("/usr/sys/ovdev") == 0) {
		    system("cp /mnt/asmfix? .");
		    system("ar x /mnt/LIB2_ov");
		} else
		    fatal++;
	    } else {
		mkdir("/usr/sys/net", 0755);
		dosystem("chog sys /usr/sys/net");
		dosystem("chmod 0755 /usr/sys/net");
		if(chngdir("/usr/sys/net") == 0)
		    system("ar x /mnt/LIB3_id");
		else
		    fatal++;
	    }
	    if(fatal == 0) {
		system("chog sys *");
		system("chmod 444 *");
	    }
	    fmnt(0);
	    sync();
	    rflop("SYSGEN #4");
	}
}

char	*l_usat[] =
{
	"/usr/bin/usat",
	0
};

f_usat(com, osp)
register struct os_info *osp;
{
	if(com == LIST) {
	    if(lstat(l_usat[0], &lstatb) >= 0)
		return(YES);
	    if(lstat("/usr/lib/usat", &lstatb) >= 0)
		return(YES);
	    return(NO);
	} else if(com == UNLOAD) {
	    printf("\n\7\7\7");
	    printf("The /usr/lib/usat directory (and all files) ");
	    printf("will be removed!\n");
	    printf("\nFILES: log files and any local modifications or files.\n");
	    usfiles(MULTI);
	    rmfile(l_usat[0]);
	    rmdirf("/usr/lib/usat");
	} else {
	    if(osp->os_flags&OS_SYM) {
		if(msdir(osp, "bin"))
		    return;
		if(msdir(osp, "lib"))
		    return;
		sync();
		printf("\n%s: /usr/lib/usat -> %s/lib/usat\n",
		    sl_make, &osp->os_slink);
		if(chngdir(&osp->os_slink))
		    return;
		if(chngdir("lib"))
		    return;
		msdir1(osp, "lib/usat");
		dosystem("chmod 755 usat");
		dosystem("chog bin usat");
		if(mslink(osp, "/usr/lib/usat"))
		    return;
		if(chngdir(&osp->os_slink))
		    return;
	    } else {
		if(chngdir(gs_usr))
		    return;
	    }
	    tarcs(TCSTART, osp);	/* Create base tar command string */
	    strcat(syscmd, " ./bin/usat ./lib/usat");
	    tarcs(TCEND, osp);		/* End tar command string */
	    while(1) {
		ddopen(osp, osp->os_flop);
		if(dosystem(syscmd) != 0) {
		    if(retry(gs_txerr))
			continue;
		    else
			break;
		}
		if(ld_rx50 == YES)
		    rflop(osp->os_flop);
		break;
	    }
	    if(osp->os_flags&OS_SYM) {
		printf("\n%s: %s -> %s/bin/usat\n",
		    sl_make, l_usat[0], &osp->os_slink);
		mslink(osp, l_usat[0]);
	    }
	}
}

f_usep(com, osp)
register struct os_info *osp;
{
	if(com == LIST) {
	    if(lstat("/usr/usep", &lstatb) >= 0)
		return(YES);
/* TODO: remove after debug
	    if(lstat("/usr/usep/sysx", &lstatb) >= 0)
		return(YES);
*/
	    return(NO);
	} else if(com == UNLOAD) {
	    printf("\n\7\7\7");
	    printf("The /usr/usep directory (and all files) will be removed!\n");
	    printf("\nFILES: log files (*.log) and exerciser scripts (*.xs).\n");
	    usfiles(MULTI);
	    rmdirf("/usr/usep");
	} else {
	    if(osp->os_flags&OS_SYM) {
		printf("\n%s: /usr/usep -> %s/usep\n", sl_make, &osp->os_slink);
		if(chngdir(&osp->os_slink))
		    return;
		msdir1(osp, "usep");
		dosystem("chog bin usep");
		dosystem("chmod 755 usep");
		if(mslink(osp, "/usr/usep"))
		    return;
	    } else {
		if(chngdir(gs_usr))
		    return;
	    }
	    tarcs(TCSTART, osp);	/* Create base tar command string */
	    strcat(syscmd, " ./usep");
	    tarcs(TCEND, osp);		/* End tar command string */
	    while(1) {
		ddopen(osp, osp->os_flop);
		if(dosystem(syscmd) != 0) {
		    if(retry(gs_txerr))
			continue;
		    else
			break;
		}
		/*
		 * The idea here is to get the correct .profile
		 * from the root (CRT or PRT). It will be in /profile
		 * if we are in setup phase 1 or 2, in /.profile otherwise.
		 */
		if(access("/profile", 0) == 0)
		    dosystem("cp /profile usep/.profile");
		else
		    dosystem("cp /.profile usep/.profile");
		printf("\n\7\7\7");
		printf("You can recover some disk space by removing exercisers");
		printf("\nfor devices not configured on your system.\n");
		if(ld_rx50 == YES)
		    rflop(osp->os_flop);
		break;
	    }
	}
}

char	*l_uucp[] =
{
	"/usr/bin/uucp",
	"/usr/bin/uulog",
	"/usr/bin/uuname",
	"/usr/bin/uupoll",
	"/usr/bin/uustat",
	"/usr/bin/uux",
	0
};

f_uucp(com, osp)
register struct os_info *osp;
{
	register int i, fatal;

	if(com == LIST) {
	    for(i=0; l_uucp[i]; i++)
		if(lstat(l_uucp[i], &lstatb) >= 0)
		    return(YES);
	    if(lstat("/usr/lib/uucp", &lstatb) >= 0)
		return(YES);
	    return(NO);
	} else if(com == UNLOAD) {
	    printf("\n\7\7\7");
	    printf("The /usr/lib/uucp directory (and all files) will be removed!\n");
	    printf("\nFILES: L.sys, L-devices, log files, local modifications.\n");
	    usfiles(MULTI);
	    for(i=0; l_uucp[i]; i++)
		rmfile(l_uucp[i]);
	    rmdirf("/usr/lib/uucp");
	} else {
	    if(osp->os_flags&OS_SYM) {
		if(msdir(osp, "bin"))
		    return;
		if(msdir(osp, "lib"))
		    return;
		sync();
		printf("\n%s: /usr/lib/uucp -> %s/lib/uucp\n",
		    sl_make, &osp->os_slink);
		if(chngdir(&osp->os_slink))
		    return;
		if(chngdir("lib"))
		    return;
		msdir1(osp, "lib/uucp");
		dosystem("chmod 755 uucp");
		dosystem("chown uucp uucp");
		dosystem("chgrp daemon uucp");
		if(mslink(osp, "/usr/lib/uucp"))
		    return;
		if(chngdir(&osp->os_slink))
		    return;
	    } else {
		if(chngdir(gs_usr))
		    return;
	    }
	    tarcs(TCSTART, osp);	/* Create base tar command string */
	    if(ld_rx50 == NO)
		strcat(syscmd,
" ./bin/uucp ./bin/uulog ./bin/uuname ./bin/uupoll ./bin/uustat ./bin/uux");
	    strcat(syscmd, " ./lib/uucp");
	    fatal = 0;
	    tarcs(TCEND, osp);		/* End tar command string */
	    while(1) {
		ddopen(osp, osp->os_flop);
		if(dosystem(syscmd) != 0) {
		    if(retry(gs_txerr))
			continue;
		    else {
			fatal++;
			break;
		    }
		}
		break;
	    }
	    if(fatal)
		return;
	    if(ld_rx50 == YES) {
		rflop(osp->os_flop);
		printf("\nLoading remainder of UUCP from PLOT diskette.\n");
		tarcs(TCSTART, osp);
		strcat(syscmd,
" ./bin/uucp ./bin/uulog ./bin/uuname ./bin/uupoll ./bin/uustat ./bin/uux");
		tarcs(TCEND, osp);
		fatal = 0;
		while(1) {
		    ddopen(osp, "PLOT");
		    if(dosystem(syscmd) != 0) {
			if(retry(gs_txerr))
			    continue;
			else {
			    fatal++;
			    break;
			}
		    }
		    break;
		}
		if(fatal)
		    return;
	    }
	    if(chngdir("./lib/uucp"))
		return;
	    if(cputyp[tpi].p_sid == SID) {
		unlink("uucico40");
		dosystem("mv uucico70 uucico");
	    } else {
		unlink("uucico70");
		dosystem("mv uucico40 uucico");
	    }
	    if(ld_rx50 == YES)
		rflop("PLOT");
	    if(osp->os_flags&OS_SYM) {
		printf("\n%ss for UUCP files.\n", sl_make);
		for(i=0; l_uucp[i]; i++)
		    mslink(osp, l_uucp[i]);
	    }
	}
}

char	*l_tcpip[] =
{
	"/usr/bin/dls",
	"/usr/bin/dcat",
	"/usr/bin/dcp",
	"/usr/bin/dlogin",
	"/usr/bin/drm",
	"/usr/ucb/rlogin",
	"/usr/ucb/rcp",
	"/usr/ucb/rwho",
	"/usr/ucb/ruptime",
	"/usr/ucb/talk",
	"/usr/ucb/telnet",
	"/usr/ucb/rsh",
	"/usr/ucb/netstat",
	"/usr/ucb/ftp",
	"/usr/ucb/tftp",
	"/usr/etc/ftpd",
	"/usr/etc/inetd",
	"/usr/etc/miscd",
	"/usr/etc/rexecd",
	"/usr/etc/rlogind",
	"/usr/etc/routed",
	"/usr/etc/rshd",
	"/usr/etc/syslog",
	"/usr/etc/talkd",
	"/usr/etc/tftpd",
	"/usr/etc/telnetd",
	"/usr/etc/rwhod",
	"/usr/etc/dgated",
	0
};

f_tcpip(com, osp)
register struct os_info *osp;
{
	register int i, fatal;

	if(com == LIST) {
	    for(i=0; l_tcpip[i]; i++)
		if(lstat(l_tcpip[i], &lstatb) >= 0)
		    return(YES);
	    return(NO);
	} else if(com == UNLOAD) {
	    for(i=0; l_tcpip[i]; i++)
		rmfile(l_tcpip[i]);
	} else {
	    if(osp->os_flags&OS_SYM) {
		if(msdir(osp, "bin"))
		    return;
		if(msdir(osp, "ucb"))
		    return;
		if(msdir(osp, "etc"))
		    return;
		sync();
		if(chngdir(&osp->os_slink))
		    return;
	    } else {
		if(chngdir(gs_usr))
		    return;
	    }
	    tarcs(TCSTART, osp);	/* Create base tar command string */
	    if(ld_rx50 == NO)
		strcat(syscmd,
" ./bin/dls ./bin/dcat ./bin/dcp ./bin/dlogin ./bin/drm");
	    strcat(syscmd,
" ./ucb/rlogin \
./ucb/rcp \
./ucb/rwho \
./ucb/ruptime \
./ucb/talk \
./ucb/telnet \
./ucb/netstat \
./ucb/ftp \
./ucb/tftp \
./ucb/rsh");
	    strcat(syscmd,
" ./etc/ftpd \
./etc/inetd \
./etc/miscd \
./etc/rexecd \
./etc/rlogind \
./etc/routed \
./etc/talkd \
./etc/rshd \
./etc/syslog \
./etc/tftpd");
	    if(ld_rx50 == NO)
		strcat(syscmd,
" ./etc/telnetd \
./etc/rwhod \
./etc/dgated");
	    fatal = 0;
	    tarcs(TCEND, osp);		/* End tar command string */
	    while(1) {
		ddopen(osp, osp->os_flop);
		if(dosystem(syscmd) != 0) {
		    if(retry(gs_txerr))
			continue;
		    else {
			fatal++;
			break;
		    }
		}
		break;
	    }
	    if(fatal)
		return;
	    if(ld_rx50 == YES) {
		rflop(osp->os_flop);
		printf("\nLoading remainder of TCP/IP from SCCS diskette.\n");
		tarcs(TCSTART, osp);
		strcat(syscmd,
" ./bin/dls ./bin/dcat ./bin/dcp ./bin/dlogin ./bin/drm");
		strcat(syscmd,
" ./etc/telnetd ./etc/rwhod ./etc/dgated");
		tarcs(TCEND, osp);
		fatal = 0;
		while(1) {
		    ddopen(osp, "SCCS");
		    if(dosystem(syscmd) != 0) {
			if(retry(gs_txerr))
			    continue;
			else {
			    fatal++;
			    break;
			}
		    }
		    break;
		}
		if(fatal)
		    return;
	    }
	    if(osp->os_flags&OS_SYM) {
		printf("\n%ss for TCP/IP files.\n", sl_make);
		for(i=0; l_tcpip[i]; i++)
		    mslink(osp, l_tcpip[i]);
	    }
	    if(ld_rx50 == YES)
		rflop("SCCS");
	}
}

f_orphan(com, osp)
register struct os_info *osp;
{
	if(com == LIST) {
	    if(lstat("/usr/orphan/usr", &lstatb) >= 0)
		return(YES);
	    else
		return(NO);
	} else if(com == UNLOAD) {
	    printf("\n\7\7\7");
	    printf("All files (except README) will be removed from ");
	    printf("/usr/orphan directory!\n");
	    printf("\nFILES: local modifications.\n");
	    usfiles(MULTI);
	    rmdirf("/usr/orphan/bin");
	    rmdirf("/usr/orphan/lib");
	    rmdirf("/usr/orphan/usr");
	} else {
	    if(osp->os_flags&OS_SYM) {
		if(msdir(osp, "orphan"))
		    return;
		if(msdir1(osp, "orphan/bin"))
		    return;
		sprintf(&syscmd, "chog bin %s/orphan/bin", &osp->os_slink);
		dosystem(syscmd);
		sprintf(&syscmd, "chmod 755 %s/orphan/bin", &osp->os_slink);
		dosystem(syscmd);
		if(msdir1(osp, "orphan/lib"))
		    return;
		sprintf(&syscmd, "chog bin %s/orphan/lib", &osp->os_slink);
		dosystem(syscmd);
		sprintf(&syscmd, "chmod 755 %s/orphan/lib", &osp->os_slink);
		dosystem(syscmd);
		if(msdir1(osp, "orphan/usr"))
		    return;
		sprintf(&syscmd, "chog bin %s/orphan/usr", &osp->os_slink);
		dosystem(syscmd);
		sprintf(&syscmd, "chmod 755 %s/orphan/usr", &osp->os_slink);
		dosystem(syscmd);
		sync();
		printf("\n%s: /usr/orphan/bin -> %s/orphan/bin\n",
		    sl_make, &osp->os_slink);
		if(mslink(osp, "/usr/orphan/bin"))
		    return;
		printf("\n%s: /usr/orphan/lib -> %s/orphan/lib\n",
		    sl_make, &osp->os_slink);
		if(mslink(osp, "/usr/orphan/lib"))
		    return;
		printf("\n%s: /usr/orphan/usr -> %s/orphan/usr\n",
		    sl_make, &osp->os_slink);
		if(mslink(osp, "/usr/orphan/usr"))
		    return;
		sync();
		if(chngdir(&osp->os_slink))
		    return;
	    } else {
		if(chngdir(gs_usr))
		    return;
	    }
	    tarcs(TCSTART, osp);	/* Create base tar command string */
	    strcat(syscmd, " ./orphan");
	    tarcs(TCEND, osp);		/* End tar command string */
	    while(1) {
		ddopen(osp, osp->os_flop);
		if(dosystem(syscmd) != 0) {
		    if(retry(gs_txerr))
			continue;
		    else
			break;
		}
		if(ld_rx50 == YES)
		    rflop(osp->os_flop);
		break;
	    }
	}
}

f_manuals(com, osp)
register struct os_info *osp;
{
	register int fatal, i;
	char	fn[20];

	if(com == LIST) {
	    if(lstat("/usr/man", &lstatb) >= 0)
		return(YES);
	    else
		return(NO);
/* TODO: remove after debug
	    for(i=1; i<9; i++) {
		sprintf(&fn, "/usr/man/man%d", i);
		if(lstat(&fn, &lstatb) >= 0)
		    return(YES);
	    }
	    return(NO);
*/
	} else if(com == UNLOAD) {
	    printf("\n\7\7\7");
	    printf("The /usr/man directory (and all files) will be removed!\n");
	    printf("\nFILES: manual page directories [man1-man8, cat1-cat8].\n");
	    usfiles(MULTI);
	    rmdirf("/usr/man");
	} else {
	    if(osp->os_flags&OS_SYM) {
		printf("\n%s: /usr/man -> %s/man\n",
		    sl_make, &osp->os_slink);
		if(chngdir(&osp->os_slink))
		    return;
		msdir1(osp, "man");
		dosystem("chmod 755 man");
		dosystem("chog bin man");
		if(mslink(osp, "/usr/man"))
		    return;
		sync();
	    } else {
		if(chngdir(gs_usr))
		    return;
	    }
	    tarcs(TCSTART, osp);	/* Create base tar command string */
	    if(ld_rx50 == NO)
		strcat(syscmd, " ./man");
	    fatal = 0;
	    tarcs(TCEND, osp);		/* End tar command string */
	    while(1) {
		ddopen(osp, osp->os_flop);
		if(dosystem(syscmd) != 0) {
		    if(retry(gs_txerr))
			continue;
		    else {
			fatal++;
			break;
		    }
		}
		break;
	    }
	    if((fatal) || (ld_rx50 == NO)) {
		if((ld_rl02 == YES) && (fatal == 0))
		    upman();
		return;
	    }
	    rflop(osp->os_flop);
	    for(i=2; i<6; i++) {
		sprintf(fn, "MANUALS #%d", i);
		fatal = 0;
		while(1) {
		    ddopen(osp, &fn);
		    if(dosystem(syscmd) != 0) {
			if(retry(gs_txerr))
			    continue;
			else {
			    fatal++;
			    break;
			}
		    }
		    break;
		}
		if(fatal)
		    break;
		rflop(&fn);
	    }
	    upman();
	}
}

/*
 * Unpack the man page files.
 * For RL02 and RX50 kits only.
 */

upman()
{
	register int i;
	char	fn[20];

	printf("\nUnpacking files for on-line manuals....\n");
	/* assumes we are in /usr (or a symbolic link to it) */
	if(chngdir("./man/man2") == 0) {
	    for(i=1; i<9; i++) {	/* unpack man files */
		sprintf(&fn, "../man%d", i);
		if(chngdir(&fn))
		    break;
		dosystem("unpack * >/dev/null 2>&1");
	    }
	}
}

/*
 * If the load device is TM02/3 (ht) magtape, ask for the
 * tape density, unless it has already been set.
 * Set loadev to rmt0 for 800 or rht0 for 1600.
 * If load device is not magtape, just return.
 */

getden()
{
	register int j;

	if(ld_tape == NO)
	    return;
	if(loadev)
	    return;
	while(1) {
	    do {
		printf("\nDistribution tape density (? for help) ");
		printf("< 800 1600 > ? ");
	    } while(gline("h_dtden") <= 0);
	    j = atoi(glbuf);
	    if(j == 800)
		loadev = "rmt0";
	    else if(j == 1600)
		loadev = "rht0";
	    else {
		printf("\n%s - bad density!\n", glbuf);
		continue;
	    }
	    break;
	}
}

/*
 * Ask the user to make sure the distribution media
 * is mounted, on-line, and ready. This may be RL02,
 * RC25, or magtape. Only ask once!
 */

int	ddmount = 0;

ddmnt()
{
	if(ld_rx50 == YES)
	    return;
	if(ddmount)
	    return;
	ddmount++;
	if(ld_tape == YES) {
	    printf("\nMake sure the distribution tape (or TK50 cartridge) is");
	    printf("\nmounted in unit zero and the unit is on-line and ready.\n");
	} else if(strncmp(loadev, "rrl", 3) == 0) {
	    printf("\nMake sure the OPTIONAL SOFTWARE disk is loaded and the");
	    printf("\ndisk drive is on-line and ready. Enter the unit number");
	    printf("\nof the RL02 where the OPTIONAL SOFTWARE disk is mounted.\n");
	    while(1) {
		do
		    printf("\nOPTIONAL SOFTWARE disk unit number < 0 1 2 3 > ? ");
		while(gline(NOHELP) <= 0);
		if((strlen(glbuf) != 1) ||
		   (glbuf[0] < '0') ||
		   (glbuf[0] > '3'))
			continue;
		loadev[3] = glbuf[0];
		break;
	    }
	} else if(strncmp(loadev, "rrc", 3) == 0) {
	    printf("\nMake sure the distribution disk is loaded in RC25 unit");
	    printf("\nzero and the disk is ready.\n");
	} else
	    printf("\nUnknown distribution media!\n");
	prtc();
}

iflop(fn)
char	*fn;
{
	printf("\n\7\7\7Insert (%s) diskette into RX%d unit %d",
		fn, (rxtype==RX33) ? RX33 : RX50, rxunit);
	if(rxtype != RX33)
		printf(" %s", rxpos(rxunit));
	printf("\n");
	prtc();
}

rflop(fn)
char	*fn;
{
	printf("\n\7\7\7Remove (%s) diskette from RX%d unit %d",
		fn, (rxtype==RX33) ? RX33 : RX50, rxunit);
	if(rxtype != RX33)
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
		return((rd2) ? rxp_tl : rxp_lr);
	case 3:
		return(rxp_lr);
	default:
		return("(?)");
	}
}

prtc()
{
	printf("%s", gs_prtc);
	fflush(stdout);
	while(getchar() != '\n') ;
}

/*
 * Make a directory, only if it
 * does not already exist.
 */

makedir(dir)
char *dir;
{
	char	syscmd[30];

	if(access(dir, 0) != 0) {
	    sprintf(syscmd, "mkdir %s", dir);
	    dosystem(syscmd);
	}
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
		    return(1);
	    }
	    break;
	}
	return(0);
}

chngdir(dir)
char *dir;
{
	if(chdir(dir) < 0) {
	    printf("\nCannot change directory to %s!\n", dir);
	    return(1);
	} else
	    return(0);
}

/*
 * Remove a file.
 * If file is a symbolic link, remove it and the real file.
 *
 *   file	full pathname including file name
 */

rmfile(file)
char	*file;
{
	register int cc;
	struct	stat	statb;
	char	buf[BUFSIZ];

	if(lstat(file, &statb) < 0)
	    return;			/* file does not exist */
	if((statb.st_mode&S_IFMT) == S_IFLNK) {	/* file is a symbolic link */
	    cc = readlink(file, &buf, BUFSIZ);	/* get pathname of real file */
	    if(cc > 0) {
		buf[cc] = 0;
		unlink(&buf);
	    }
/* TODO: may want to warn symlink could not be removed??? */
	}
	unlink(file);
}

/*
 * Remove a directory and its files.
 *
 * If the directory is a symbolic link, remove the link
 * and the real directory and its files.
 *
 *   dir	full pathname of directory
 */

rmdirf(dir)
char	*dir;
{
	register int cc;
	struct	stat	statb;
	char	scmd[100];
	char	buf[BUFSIZ];

	if(lstat(dir, &statb) < 0)
	    return;			/* directory does not exist */
	if((statb.st_mode&S_IFMT) == S_IFLNK) {	/* dir is a symbolic link */
	    cc = readlink(dir, &buf, BUFSIZ);	/* get pathname of real file */
	    if(cc > 0) {
		buf[cc] = 0;
		sprintf(&scmd, "rm -rf %s", &buf);
		dosystem(&scmd);
	    }
	}
	sprintf(&scmd, "rm -rf %s", dir);
	dosystem(&scmd);
}

/*
 * Make subdirectories needed for symbolic links to
 * individual files only (not directories).
 * Makes entries in both /usr and the mounted file system
 * where the real file(s) will exist.
 * Also, "chog bin" and "chmod 755".
 * No error if directory already exists, all others are fatal.
 *
 *   osp	optional software info table pointer
 *   dir	subdirectory name (no / in front of name)
 */

msdir(osp, dir)
register struct os_info *osp;
char	*dir;
{
	char	*p, *q, *v;
	char	pn[50];		/* parent directory name (eg, /user1/.) */
	char	dn[50];		/* directory name (eg, /user1/bin) */
	char	dnp[50];	/* dn + /. (eg, /user1/bin/.) */
	char	dnpp[50];	/* dn + /.. (eg, /user1/bin/..) */
	char	scmd[100];

	/*
	 * Create path name string for parent directory.
	 * Will be the symbolic link base directory (eg, /user1)
	 * if the directory being created (*dir) has no slashes
	 * in its path name. Otherwise, will be base + *dir (eg, /user1/bin/.).
	 */
	sprintf(&pn, "%s/.", &osp->os_slink);
	for(p = &pn; (*p != '.'); p++);
	for(v=0, q=dir; *q; q++)
	    if(*q == '/')
		v = q;
	if(v) {
	    for(q=dir; ((int)q < (int)v); q++)
		*p++ = *q;
	    *p++ = '/';
	    *p++ = '.';
	    *p++ = '\0';
	}
/*	printf("\nDEBUG: %s\n", &pn);	*/
	/*
	 * Make subdirectory in mounted file system (real files).
	 */
	sprintf(&dn, "%s/%s", &osp->os_slink, dir);
	if(mknod(&dn, 040755, 0) < 0) {
	    if(errno != EEXIST) {
		printf("\nCan't make %s directory!\n", &dn);
		return(1);
	    }
	} else {
	    sprintf(&scmd, "chog bin %s/%s", &osp->os_slink, dir);
	    dosystem(scmd);
	    /* We do chmod so se don't have to mess with the users' umask */
	    sprintf(&scmd, "chmod 755 %s/%s", &osp->os_slink, dir);
	    /* Link . and .. */
	    sprintf(&dnp, "%s/.", &dn);
	    if(link(&dn, &dnp) < 0) {
		linkerr(dn, dnp);
		unlink(dn);
		return(1);
	    }
	    sprintf(&dnpp, "%s/..", &dn);
	    if(link(&pn, &dnpp) < 0) {
		linkerr(dn, dnpp);
		unlink(dnp);
		unlink(dn);
		return(1);
	    }
	}
	/*
	 * Make subdirectory in /usr (for symbolic links).
	 */
	sprintf(&pn, "/usr/.");
	sprintf(&dn, "/usr/%s", dir);
	if(mknod(&dn, 040755, 0) < 0) {
	    if(errno != EEXIST) {
		printf("\nCan't make %s directory!\n", &dn);
		return(1);
	    }
	} else {
	    sprintf(&scmd, "chog bin /usr/%s", dir);
	    dosystem(scmd);
	    /* We do chmod so se don't have to mess with the users' umask */
	    sprintf(&scmd, "chmod 755 /usr/%s", dir);
	    /* Link . and .. */
	    sprintf(&dnp, "%s/.", &dn);
	    if(link(&dn, &dnp) < 0) {
		linkerr(dn, dnp);
		unlink(dn);
		return(1);
	    }
	    sprintf(&dnpp, "%s/..", &dn);
	    if(link(&pn, &dnpp) < 0) {
		linkerr(pn, dnpp);
		unlink(dnp);
		unlink(dn);
		return(1);
	    }
	}
	return(0);
}

linkerr(to, from)
char	*to;
char	*from;
{
	printf("\nCannot link %s -> %s\n", from, to);
}

/*
 * Make directory needed for symbolic links.
 * This is the directory in the mounted file system
 * where the files will be actually loaded.
 *
 * If the file exists and is a directory, just return.
 * If the file does not exist, make the directory.
 * If the file exists and is not directory, give the
 * user a chance to preserve the file, then blow it away and
 * make the directory.
 * Return an error if can't make the directory.
 *
 *   osp	optional software info table pointer
 *   dir	directory name, relative to base directory
 *		eg, "lib/uucp", no beginning slash.
 *
 */

msdir1(osp, dir)
register struct os_info *osp;
char	*dir;
{
	struct stat statb;
	char	dn[100];
	sprintf(&dn, "%s/%s", &osp->os_slink, dir);
	if(lstat(&dn, &statb) >= 0) {
	    if((statb.st_mode&S_IFMT) == S_IFDIR)
		return(0);
	    printf("\nCONFLICT: a file named %s exists!\n", &dn);
	    usfiles(SINGLE);
	    unlink(&dn);
	}
	if(mkdir(&dn, 0777) < 0) {
	    printf("\nCannot make %s directory!\n", &dn);
	    return(1);
	}
	return(0);
}

/*
 * Make symbolic link for a single file.
 *
 *   osp	optional software info table pointer
 *   path	full pathname of symbolic link (not real file name)
 *		ASSUMPTION: pathname begins with /usr!
 */

mslink(osp, path)
register struct os_info *osp;
char	*path;
{
	char	rfname[100];

	sprintf(&rfname, "%s%s", &osp->os_slink, &path[4]);
	if(symlink(rfname, path) < 0) {
	    printf("\nCannot create symbolic link: %s -> %s", path, rfname);
	    printf("\n\7\7\7Try unloading (%s), then retry loading (%s).\n",
		osp->os_name, osp->os_name);
	    return(1);
	} else
	    return(0);
}

/*
 * Escape to the shell,
 * to give the user a chance to preserve critical files
 * and/or local modifications when unloading optional software.
 *
 * Assumes caller printed appropriate warning messages.
 *
 *   nf		single file or many files
 */

usfiles(nf)
{
	printf("\nDo you want to preserve %s <y or n> ? ",
	    (nf==SINGLE) ? "this file" : "any of these files");
	if(yes(NOHELP) == NO)
	    return;
	phelp("h_suf");
	fflush(stdout);
	system("sh");
	if(nf==MULTI)
	    printf("\nRemoving files...\n");
	else
	    printf("\nContinuing...\n");
}
