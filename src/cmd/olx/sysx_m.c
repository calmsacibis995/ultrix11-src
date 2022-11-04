
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)sysx_m.c	3.1	3/26/87";
/*
 * ULTRIX-11 system exerciser control program (sysx).
 *
 * Part I - master control program
 *
 *	runs sysx_c to create scripts
 *	runs sysx_r to run scripts
 *
 * Fred Canter 12/18/83
 * Bill Burns 4/84
 *	added check for rx50 for rax
 * 	added eventflag usage
 * Chung-Wu Lee 2/22/85
 *	added TMSCP (TK50/TU81)
 *
 * This program is used to control the execution of the
 * ULTRIX-11 on-line exerciser programs. The major functions
 * of sysx are:
 *
 * 1.	Execute unix commands
 *
 * 2.	List names of exercisers and the devices they exercise.
 *
 * 3.	Create exerciser run scripts, using sysx_c.
 *
 * 4.	Start/stop execution of exercisers, using sysx_r.
 *
 * 5.	Print exerciser log files.
 *
 * 6.	Provide on-line help facility for the user.
 *	****** Requires sysx_?.help files ******
 *
 */


char	*gs_more = "\nPress <RETURN> for more:";
char	*gs_prtc = "\nPress <RETURN> to continue:";

char *h_z[] = 
{
	"",
	"The `!' is used to escape from the sysx program back to ",
	"the ULTRIX-11 command interpreter (shell), execute any of ",
	"the ULTRIX-11 commands, and then return to the sysx program. ",
	"The `!' feature may only be used when sysx is ready to ",
	"accept a command, i.e., in response to the `>' prompt. ",
	"The `!' may also be used, while the exercisers are running,",
	"to execute ULTRIX-11 commands like ps, pstat, and iostat ",
	"to monitor system operation. The usage of the `!' is as ",
	"follows:",
	"",
	"	! command",
	"",
	"where command is any of the ULTRIX-11 commands. For a ",
	"description of the ULTRIX-11 commands refer to section ",
	"one of the `ULTRIX-11 Programmers Manual'. For example:",
	"",
	"	! ls -l",
	"",
	"would produce a listing of all the files in the current ",
	"directory.",
	0
};
char *h_b[] = 
{
	"",
	"The `b' command uses the ULTRIX-11 copy command to save ",
	"existing log files if desired. If an exerciser is started ",
	"with the log file option specified, the existing log file ",
	"for that exerciser will be overwritten. In some cases it ",
	"may be desirable to save the error information in the log ",
	"file prior to starting the exerciser, however saving large ",
	"numbers of log files is not advised because it wastes disk ",
	"space.",
	"",
	"The copy command takes the form:",
	"",
	"	!cp oldfile newfile",
	"",
	"where oldfile is the existing log file name and newfile is ",
	"the new log file name, only one log file at a time may be ",
	"copied.",
	0
};
char *h_c[] = 
{
	"",
	"The `c' command is used to create exerciser run scripts. ",
	"This may NOT be done while exercisers are running. The `c' ",
	"command asks for the script name, which may be a combination ",
	"of letters and numbers up to 11 characters in length. If a ",
	"script by that name already exists, the `c' command will ask ",
	"whether or not to overwrite the existing script.  A script ",
	"may consist of one or more exerciser names including, in ",
	"some cases, multiple copies of the same exerciser.",
	"",
	"Next, `c' prompts for an exerciser name. Respond with the ",
	"name of the exerciser to be entered into the script followed ",
	"by a <RETURN>. After the name is entered, `c' will ask several ",
	"questions about the options for that exerciser. After each ",
	"question, the default answer <default> is printed, to use ",
	"the default answer type a <RETURN>. Answering the question ",
	"with a `?' will produce an expanded explanation of the ",
	"question. Typing <CTRL/D> will cause SYSX to cancel the ",
	"current entry and return to the exerciser name question.",
	"",
	-1,
	"",
	"After all of the option questions have been answered, `c' ",
	"again prompts for an exerciser name. To enter another ",
	"exerciser into the script, type the name followed by a ",
	"<RETURN>. To end the script, respond to the exerciser name ",
	"question with just a <RETURN>. Refer to the `p', `r', and `s' ",
	"commands for information about printing, running, and ",
	"stopping the exerciser run script. Exercisers should be ",
	"entered into the script in the same order as they are ",
	"listed by the `n' command.",
	0
};
char *h_d[] = 
{
	"",
	"The `d' command is used to delete old or unwanted exerciser ",
	"run scripts.  The `x' command can be used to obtain a list ",
	"of the existing scripts.",
	"",
	"To execute this command, respond to the `>' prompt with a ",
	"`d' followed by a <RETURN>. The `d' command will ask for the ",
	"name of the script to be deleted. If the script exists, it ",
	"will be removed, if not an error message will be printed.",
	0
};
char *h_h[] = 
{
	"",
	"The SYSX program has two modes of operation: \"command mode\"",
	"and \"run mode\". SYSX is in run mode whenever exercisers ",
	"are running and in command mode when no exercisers are running.",
	"",
	"The `>' prompt indicates that SYSX is in command mode while the ",
	"`run>' prompt indicates that SYSX is in run mode. ",
	"The following list of SYSX commands is broken down into two ",
	"groups; commands available in \"command mode\", and commands ",
	"available in \"run mode\". ",
	"",
	"Except for <CTRL/D> and <CTRL/C>, commands are ",
	"executed by typing the command letter followed by a <RETURN>. ",
	"The commands will ask for additional information, such as ",
	"script name, if required.  For more help type `h' followed ",
	"by the command, `h r' for help run.",
	"",
	-1,
	"",
	"Commands available in command mode:",
	"",
	"<CTRL/D>	Exit from the sysx program.",
	"<CTRL/C>	Cancel current command and return to the prompt.",
	"! command	Execute an ULTRIX-11 command.",
	"b		Backup, save an existing log file.",
	"c		Create an exerciser run script.",
	"d		Delete an exerciser run script.",
	"l		Print log files on the terminal or line ",
	"			printer.",
	"n		Name the exerciser to run on each device.",
	"p		Print the contents of a script. ",
	"r		Run an exerciser script.",
	"s		Stop all exerciser(s).",
	"x		Print a list of the existing exerciser run ",
	"			scripts.",
	"",
	-1,
	"",
	"Commands available in run mode:",
	"",
	"<CTRL/D>	Exit from the sysx program.",
	"<CTRL/C>	Cancel current command and return to the prompt.",
	"! command	Execute an ULTRIX-11 command.",
	"l		Print log files on the terminal or line ",
	"			printer.",
	"p		Print the status of the currently running ",
	"			script.",
	"r		Restart system exerciser(s).",
	"s		Stop system exerciser(s).",
	0
};
char *h_l[] = 
{
	"",
	"The `l' command is used to print the contents of exerciser",
	"log files on the terminal or the line printer.",
	"",
	"When asked for `which log files', respond with:",
	"1.	A <RETURN> to print `all' existing log files.",
	"	There could be very many log files in existence !",
	"",
	"2.	An exerciser name, to print all log files for that ",
	"	exerciser.  There can be many log files associated ",
	"	with an exerciser, for example there could be 20 ",
	"	copies of CPX running, so there would be 20 log files.",
	"",
	"3.	A specific log file name, obtained from the exerciser ",
	"	script via the `p' command. For example, `cpx_2.log' ",
	"	would print only the log file for the second copy of ",
	"	CPX.",
	"",
	-1,
	"",
	"When asked `Output on ', answer:",
	"1.	Yes for output on the line printer.",
	"	Answering yes when no line printer exists will cause",
	"	line printer spool files to be created and waste ",
	"	disk space.",
	"",
	"2.	No or <RETURN> for output on the terminal.",
	0
};
char *h_n[] = 
{
	"",
	"The `n' command prints a list of the exerciser names. The ",
	"list consists of; the name of the exerciser, the names of ",
	"the devices associated with the exerciser, and a brief ",
	"comment about each exerciser. The exerciser name list is ",
	"used to aid in the generation of exerciser run scripts, ",
	"refer to the `c' command for more on scripts. To execute ",
	"the `n' command respond to the `>' prompt with `n' followed ",
	"by a <RETURN>.",
	0
};
char *h_p[] = 
{
	"",
	"The `p' command prints the contents of an exerciser run ",
	"script. If exercisers are not running `p' asks for the name ",
	"of the script to be printed. To execute the `p' command, ",
	"type `p' followed by a <RETURN> after the `>' prompt. ",
	"The printout consists of; the exerciser number, the name ",
	"of the exerciser, the state of the exerciser (run/stop), ",
	"and the options specified for the exerciser. If the ",
	"exerciser does not exist, an error message will be printed.",
	0
};
char *h_r[] = 
{
	"",
	"The `r' command is used to start an exerciser script running. ",
	"Answer the `Script name' question with the name of the ",
	"exerciser script to be started, the default name is ",
	"`sysxr'. After reading the script into its buffer SYSX; ",
	"prints any warning messages (see below), starts the ",
	"exercisers, waits for the exercisers to start, and then ",
	"prints the `run>' prompt. ",
	"",
	"If an exerciser fails to start, use the `l' command ",
	"to print the log file for that exerciser to determine ",
	"the fault.  SYSX monitors the exerciser log files and ",
	"periodically reports changes in their size, in order ",
	"to indicate the possible occurrence of errors.",
	"",
	-1,
	"",
	"1.	The exercisers normally write all output to a log ",
	"	file, which will be overwritten each time the ",
	"	exerciser is started. A Warning message is printed ",
	"	which allows the user to abort the exerciser startup ",
	"	and save the log files, with the `b' command if ",
	"	desired.",
	"",
	"2.	If the script contains the DH11, DHU11, DHV11, DZ11, ",
	"	DZV11 and DZQ11 communications multiplexer exerciser ",
	"	(cmx), a the user is warned to disconnect any ",
	"	customer equipment from the output lines prior to ",
	"	starting cmx.",
	0
};
char *h_s[] = 
{
	"",
	"The `s' command is used to stop the execution of one, all, ",
	"or a group of exercisers, when SYSX is in the run mode (the ",
	"prompt is `run>'). When not in run mode the SYSX `s' command ",
	"will send the `quit' signal to the exerciser process group, ",
	"which will terminate any exercisers. ",
	0
};
char *h_x[] = 
{
	"",
	"The `x' command prints a list of the existing exerciser run ",
	"scripts.  Respond to the `>' prompt with `x' followed by a ",
	"<RETURN> to produce the list. Each name in the list will have ",
	"`.xs' appended to it, i.e., the script `sysxr' would be ",
	"listed as `sysxr.xs'. The `.xs' extension is added to the ",
	"script name to make it a unique filename for the ULTRIX-11 ",
	"system.  When using the sysx program, all references to ",
	"script names use only the name, in other words the `.xs' is ",
	"ignored and should not be typed.",
	0
};
char	*h_N[] =
{
	"",
	"\tEXERCISER\tDEVICES\t\tCOMMENTS",
	"",
	"\tcpx\t\tCPU\t\tAll PDP11 processors",
	"\tfpx\t\tFP11-A/B/C/E/F\tPDP11/40 FIS not supported",
	"\t\t\tFPF11\t\tPDP11/23 & PDP11/24 Floating",
	"\t\t\t\t\tPoint",
	"\tmemx\t\tMEMORY\t\tAll types of memory",
	"\tlpx\t\tLP11\t\tAll LP11 type line printers",
	"\tcmx\t\tDH11,\t\tCommunications devices",
	"\t\t\tDHU11,DHV11,",
	"\t\t\tDZ11,DZV11,DZQ11",
	"\t\t\tDL11",
	"\tmtx\t\tTM02/3,\t\tTU16/TE16/TU77 magtapes",
	"\t\t\tTM11,\t\tTU10/TE10/TS03 magtapes",
	"\t\t\tTS11\t\tTS11/TSV05/TU80/TK25 magtapes",
	"\t\t\tTK50\t\tTK50/TU81 magtapes",
	"\thpx\t\tRM02/3/5,\tAll disks on RH11/RH70",
	"\t\t\tRP04/5/6,\tcontrollers",
	"\t\t\tML11",
	"",
	-1,
	"\thkx\t\tRK06/7\t\tDisks on RK611/RK711 controller",
	"\trpx\t\tRP02/3\t\tDisks on RP11 controller",
	"\trlx\t\tRL01/2\t\tDisks on RL11 controller",
	"\trkx\t\tRK03/5\t\tDisks on RK11 controller",
	"\trax\t\tRA60/80/81\tDisks on UDA50 controller",
	"\trax\t\tRD31-32,RD51-54\tDisks on RQDX1/2/3 controller",
	"\trax\t\tRX50/RX33\tDisks on RQDX1/2/3 controller",
	"\trax\t\tRC25\t\tDisks on KLESI controller",
	"\thxx\t\tRX02\t\tDisks on RX211 controller",
	0
};

struct hsub {
	char	*hs_name;
	char	**hs_msg;
} hsub[] = {
	"!",		&h_z,
	"b",		&h_b,
	"c",		&h_c,
	"d",		&h_d,
	"h",		&h_h,
	"l",		&h_l,
	"n",		&h_n,
	"p",		&h_p,
	"r",		&h_r,
	"s",		&h_s,
	"x",		&h_x,
	"N",		&h_N,
	0
};


#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <a.out.h>

#define	SBSIZE	8192
#define	YES	1
#define	NO	0
#define HELP	1
#define	NOHELP	0

/*
 * Exerciser option definitions
 */

#define LOGF	0	/* > filename, write error messages to a log file */
#define NEPRNT	1	/* -n #, number of data mismatches to print per error */
#define NEDROP	2	/* -e #, number of errors allowed before device dropped */
#define CMUNIT	3	/* CMX -d?#, device/unit number:
				dh#, dhu#, dhv#, dz#, dzv#, dl# */
#define CMSLB	4	/* CMX -m, suppress loop back */
#define CMLS	5	/* CMX -l # #, line & bit rate select */
#define CMLDS	6	/* CMX -u #, line deselect */
#define CMBR	7	/* CMX -b #, bit rate select for all lines */
#define DECN	8	/* HPX -c#, RH11/RH70 controller number */
#define DEDN	9	/* -d#, disk exerciser drive number */
#define	DEWRT	10	/* -w, disk exer write on customer area */
#define DEFS	11	/* -f#, disk exerciser file system select */
#define DESTAT	12	/* -s#, disk exerciser print stats interval */
#define DEIFS	13	/* -i, disk exerciser inhibit file system status */
#define	DRXMOD	14	/* HXX -m#, mode (1=RX01, 2=RX02, default is both) */
#define LPNP	15	/* LPX -p#, pause (NO PRINT) time */
#define MTTYPE	16	/* MTX -??, magtape controller type, ht, tm, ts */
#define MTDN	17	/* MTX -d#, magtape drive number (can be more than 1) */
#define MTIDS	18	/* MTX -i, magtape inhibit drive status message */
#define MTSTAT	19	/* MTX -s, suppress I/O stats message */
#define MTFEET	20	/* MTX -f#, magtape number of feet of tape to use */

#define	XRUN	1
#define	XSTOP	0

int	sswait;		/* number of seconds to wait for exer start/stop */
			/* 23 or 24 = 450, micro/pdp-11 = 600, others = 300 */
int	cputype;

/*
 * Below needed for proper handling
 * of rax "maint area" question
 * Bill 4/20/84
 */
#define RX50	50

struct	ra_drv {		/* RA drive information */
	char	ra_dt;		/* RA drive type */
	char	ra_online;	/* RA drive on-line flag */
	union {
		daddr_t	ra_dsize;	/* RA drive size (# of LBN's) */
		struct {
			int	ra_dslo;
			int	ra_dshi;
		};
	} d_un;
} ra_drv[8];			/* size MUST be 8 */

long	evntflg();	/* type returned by event flag syscall */

struct nlist nl[] =
{
	{ "_cputype" },
	{ "_ra_drv" },
	{ "" },
};

/*
 * Exerciser control table.
 * Contains the exerciser run/stop status,
 * its name, and pointers to its kill & log file names.
 */

#ifdef EFLG
#include <sys/eflg.h>
#define	MAXEXR	64	/* maximum number of exercisers */
struct	extab
{
	char	*exn;		/* pointer to exerciser name in sbuf */
	char	*exo;		/* "	options		*/
	char	*exlf;		/* "	log filename	*/
	int	osz;		/* old logfile size - monitoring */
	int	nsz;		/* new logfile size - monitoring */
	int	exs;		/* run/stop status	*/
} exrtab[MAXEXR+1];
#else
#define	MAXEXR	150	/* maximum number of exercisers */
struct	extab
{
	char	*exn;		/* pointer to exerciser name in sbuf */
	char	*exo;		/* "	options		*/
	char	*exkf;		/* "	kill filename	*/
	char	*exlf;		/* "	log filename	*/
	int	osz;		/* old logfile size - monitoring */
	int	nsz;		/* new logfile size - monitoring */
	int	exs;		/* run/stop status	*/
} exrtab[MAXEXR+1];
#endif

struct	xtab
{
	char	*xn;		/* exerciser name */
	int	opt[12];	/* options, -1 ends list */
} xrtab[] = {
	{ "cpx",LOGF,-1 },
	{ "fpx",LOGF,NEPRNT,NEDROP,-1 },
	{ "memx",LOGF,-1 },
	{ "lpx",LOGF,LPNP,-1 },
	{ "cmx",LOGF,NEPRNT,NEDROP,CMUNIT,CMSLB,CMLS,CMLDS,CMBR,DESTAT,-1 },
	{ "mtx",LOGF,NEPRNT,NEDROP,MTTYPE,MTDN,MTIDS,MTSTAT,MTFEET,-1},
	/* DECN MUST proceed DEDN for HPX */
	{ "hpx",LOGF,NEPRNT,NEDROP,DECN,DEDN,DEFS,DESTAT,DEIFS,-1 },
	{ "hkx",LOGF,NEPRNT,NEDROP,DEDN,DEFS,DESTAT,DEIFS,-1 },
	{ "rpx",LOGF,NEPRNT,NEDROP,DEDN,DEFS,DESTAT,DEIFS,-1 },
	{ "rlx",LOGF,NEPRNT,NEDROP,DEDN,DEFS,DESTAT,DEIFS,-1 },
	{ "rkx",LOGF,NEPRNT,NEDROP,DEDN,DESTAT,DEIFS,-1 },
	{ "rax",LOGF,NEPRNT,NEDROP,DEDN,DEWRT,DEFS,DESTAT,DEIFS,-1 },
	{ "hxx",LOGF,NEPRNT,NEDROP,DEDN,DRXMOD,DESTAT,DEIFS,-1 },
	0
};

char	helpcmd[] = "cat sysx_?.help\0";
char	xrn[20];	/* exerciser name */
char	logfn[20];	/* log file name */

#ifdef EFLG
int	efid[2];	/* eventflag id array */
long	eflg[2];	/* eventflags */
int	nexer;		/* number of exercisers in script */
#else
char	killfn[20];	/* kill file name */
#endif


char	line[140];
char	script[20];	/* name of current script */
char	sbuf[SBSIZE+256];	/* script buffer */

/*
 * Exerciser run flag,
 * 1 = Some or all exercisers are running
 * 0 = No exercisers are running
 *
 * When set, this flag prevents the execution of
 * certain commands and modifies the operation
 * of other commands.
 */

jmp_buf	savej;

int	buflag;	/* used to cancel a line when creating a script */

struct stat statb;	/* used by logfile montioring */

time_t	timbuf;
time_t	stime1;
time_t	stime2;

int	swiflag;	/* flag to interrupt exer startup wail loop */
int	pgrp = 31111;

main()
{
	int	swintr();
	int	intr();
	register int i;
	int	j, k;
	register char *p, *n;
	char	*q;
	struct xtab *xp;
	int	cc, fd;
	int	mtfirst;
	int	ncopy;
	int	logfil;
	int	sst;
	int	cmxon;
	int	yn;
	int	lstall;
	int	savebp;
	int	ker;
	FILE	*fnx;
	int	ncbx, ncix, nbix;
	int	mem;
	int	dn;
	int	pid;
#ifdef EFLG
	int	pis;
#endif
	int	wtstat;
	int	owtstat;
	int	*ip;

	nlist("/unix", nl);
	if(nl[0].n_type == 0) {
		printf("\nsysx: can't access namelist in /unix\n");
		exit(1);
	}
	if((mem = open("/dev/mem", 0)) < 0) {
		printf("\nsysx: can't open /dev/mem\n");
		exit(1);
	}
	lseek(mem, (long)nl[0].n_value, 0);
	read(mem, (char *)&cputype, sizeof(cputype));
	lseek(mem, (long)nl[1].n_value, 0);
	read(mem, (char *)&ra_drv, sizeof(ra_drv));
	close(mem);
/*
	sswait = 300;
	if((cputype == 23) || (cputype == 24))
		sswait += 150;
	if((cputype == 23) && (nl[i].n_type))
		sswait += 150;	/* if ra_drv, assume Micro/pdp-11 */
	signal(SIGINT, intr);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	printf("\n\nSystem exerciser control program");
	printf("\nType h for help\n");
	setjmp(savej);
cloop:
	printf("\n\n> ");
cloop1:
	fflush(stdout);
	cc = read(0, (char *)&line[0], 132);
	if(cc == 0) {	/* control d */
		printf("\n\n");
		exit(0);
	}
	if(cc == 1)
		goto cloop;
	if(cc > 100) {
	badl:
		printf("\nBad command line !");
		goto cloop;
	}
	p = &line;
	while((*p != '\r') && (*p != '\n')) {
		if((*p >= 'A') && (*p <= 'Z'))
			*p |= 040;	/* force lower case */
		p++;
	}
	*p = 0;
	p = line;
	if((*p >= 'a') && (*p <= 'z') && (strlen(p) != 1) && (*p != 'h'))
		goto badl;
	while(*++p == ' ') ;
/*
 * Decode the command and act accordingly
 * line[0] = command
 * *p	   = rest of command line
 */
	switch(line[0]) {
	case 'n':
		dohelp("N");
		goto cloop;
	case 'h':	/* on-line help facility */
		if(!(*p)) {
			*p = 'h';
			*(p+1) = '\0';
		}
		if(*p == "!")
			*p = "z";
		dohelp(p);
		goto cloop;
	case '!':	/* execute unix command */
		if(*p) {
			fflush(stdout);
			system(p);
			printf("!\n> ");
		} else
			printf("\nCommand missing !\n> ");
		goto cloop1;
	case 'c':	/* create script */
		p = sname();		/* get script name */
		if(access(p, 0) == 0) {	/* see if script exists */
			printf("\nScript exists, overwrite it");
			if(yes(NO, NOHELP) != YES)
				break;
		}
		if((pid = fork()) == 0) {
			execl("sysx_c", "sysx_c", p, 0);
			printf("\n\nCan't exec sysx_c !\n");
			exit(1);
		} else
			while(wait() != -1) ;
		break;
	case 'p':	/* Display script/status */
		p = sname();	/* get script name */
		if((fd = open(&script, 0)) < 0) {
			printf("\nCan't open %s, are you sure it exists ?", script);
			break;
		}
		for(i=0; i<(SBSIZE+256); i++)
			sbuf[i] = 0;
		if(read(fd, (char *)&sbuf, SBSIZE+256) < 0) {
			printf("\n%s script file read error", script);
			break;
		}
		close(fd);
		setext();	/* set up exerciser control table (exrtab) */
		printf("\n\n # EXER  STATE      LOGFILE  OPTIONS");
		for(i=0; exrtab[i].exn; i++) {
			printf("\n%2d ", i+1);
			printf("%4s  ", exrtab[i].exn);
			if(exrtab[i].exs == XRUN)
				printf("run   ");
			else
				printf("stop  ");
			if(exrtab[i].exlf)
				printf("%12s  ", exrtab[i].exlf);
			else
				printf("              ");
			if(exrtab[i].exo)
				printf("%s", exrtab[i].exo);
		}
		break;
	case 'r':	/* run an exerciser script */
		signal(SIGINT, SIG_IGN);
#ifndef EFLG
		system("touch junk.kill");
		system("rm -f *.kill");
#endif
		p = sname();
		if((fd = open(p, 0)) < 0) {
			printf("\nCan't open %s, are you sure it exists ?", p);
			goto r_xit;
		}
		close(fd);
		if((pid = fork()) == 0) {
			execl("sysx_r", "sysx_r", p, 0);
			printf("\n\nCan't exec sysx_r !\n");
			exit(1);
		} else {
			ip = &wtstat;
			while(wait(ip) != -1) 
				owtstat = wtstat;
			if(((owtstat >> 8) & 0377) == 1)
				exit(0);
/*
			ip = &wtstat;
			wait(ip);
			if(((*ip >> 8) & 0377) == 1)
				exit(1);
*/
		}
	r_xit:
		break;
	case 's':	/* stop exercisers */
		printf("\nNo exercisers running, ");
		printf("will kill all exercisers anyway !");
		killpg(pgrp, SIGQUIT);
		break;
	case 'l':	/* print log files */
		do
			printf("\nPrint which log files <all> ? ");
		while((cc = getline(19)) < 0);
		if(tall(cc))
			lstall = 1;
		else
			lstall = 0;
		printf("\nOutput on line printer");
		if(yes(NO, NOHELP) == YES) {
			if(lstall)
				system("pr *.log |lpr");
			else if(cc <= 5) {
				p = strcpy(&xrn, "pr ");
				p = strcat(p, &line);
				p = strcat(p, "*.log |lpr");
				system(p);
			} else {
				if(access(&line, 0) != 0) {
				lferr:
					printf("\nNo such log file !");
					break;
				}
				p = strcpy(&xrn, "pr ");
				p = strcat(p, &line);
				p = strcat(p, " |lpr");
				system(p);
			}
		} else {
/* code to use more commented out
			printf("\nUse more for video terminal");
			if(yes(NO, NOHELP) == YES) {
				if(lstall)
					system("pr *.log | more -22");
				else if(cc <= 5) {
					p = strcpy(&xrn, "pr ");
					p = strcat(p, &line);
					p = strcat(p, "*.log");
					p = strcat(p, " | more -22");
					system(p);
				} else 	{
					if(access(&line, 0) != 0)
						goto lferr;
					p = strcpy(&xrn, "pr ");
					p = strcat(p, &line);
					p = strcat(p, " | more -22");
					system(p);
				}
			} else {
*/
			if(lstall)
				system("pr *.log");
			else if(cc <= 5) {
				p = strcpy(&xrn, "pr ");
				p = strcat(p, &line);
				p = strcat(p, "*.log");
				system(p);
			} else {
				if(access(&line, 0) != 0)
					goto lferr;
				p = strcpy(&xrn, "pr ");
				p = strcat(p, &line);
				system(p);
			}
		/* } commented out: goes with 'more' code */
		}
		break;
	case 'd':	/* delete a script */
		p = sname();
		if(unlink(p) < 0)
			printf("\n%s script nonexistent !", p);
		break;
	case 'x':	/* list of existing scripts */
		printf("\nList of existing scripts, ignore the `.xs' !\n\n");
		system("ls *.xs");
		break;
	case 'b':	/* backup log files */
		printf("\nUse the `p' command to obtain log file names.");
		printf("\nUse the unix copy command to save log files:");
		printf("\n\t`!cp oldfile newfile'");
		break;
	default:
		goto badl;
	}
	goto cloop;
}

/*
 * Handle yes or no responses.
 * If only return typed take default response.
 * y or yes is the yes response anything else is NO !
 * def = 1, default answer is yes - return 1 on yes
 * def = 0, default answer is no - return 0 on no
 * hlp = 1, help available - return -1 on ?
 * hlp = 0, no help - print "type yes or no" on ?
 */

yes(def, hlp)
{
	char	resp[10];
	int	cc;

	buflag = 0;
	if(def)
		printf(" <yes> ? ");
	else
		printf(" <no> ? ");
yn:
	fflush(stdout);
	cc = read(0, (char *)&resp, 10);
	if(cc == 0) {	/* ^D - cancel script line */
		buflag++;
		return(NO);
	}
	if(cc > 4)
		return(NO);
	if(cc == 1)
		return(def);
	if((cc == 2) && (resp[0] == '?')) {
		if(hlp == HELP)
			return(-1);
		else {
			printf("\nPlease answer yes or no ! ");
			goto yn;
		}
	}
	if(resp[0] == 'y') {
		if(cc == 2)
			return(YES);
		if((cc == 4) && (resp[1] == 'e') && (resp[2] == 's'))
			return(YES);
	} else
		return(NO);
}

/*
 * Get a line of text form the terminal,
 * replace the new line character with 0
 * and return the character count.
 * hlp = 0, print `no help available'
 * hlp > 0, print help message if `?' typed
 */

getline(hlp)
{
	register int	cc, i;

	buflag = 0;
loop:
	fflush(stdout);
	while((cc = read(0, (char *)&line, 50)) >= 50)
		printf("\nToo many characters, try again !\n");
	for(i=0; i<50; i++) {
		if((line[i] >= 'A') && (line[i] <= 'Z'))
			line[i] |= 040;	/* force lower case */
		if((line[i] == '\r') || (line[i] == '\n')) {
			line[i] = 0;
			break;
		}
	}
	if(cc == 0) {	/* ^D - cancel script line */
		buflag++;
		return(cc);
	}
	if((cc == 2) && (line[0] == '?')) {
		cc = -1;
		switch(hlp) {
		case NOHELP:
			printf("\nSorry no help available !");
			break;
		case 1:
			system("cat sysx_xn.help");
			break;
		case 2:
			system("cat sysx_dmm.help");
			break;
		case 3:
			system("cat sysx_dd.help");
			break;
		case 4:
			system("cat sysx_dt.help");
			break;
		case 5:
			system("cat sysx_sln.help");
			break;
		case 6:
			system("cat sysx_sbr.help");
			break;
		case 7:
			system("cat sysx_dln.help");
			break;
		case 8:
			system("cat sysx_sbr.help");
			break;
		case 9:
			system("cat sysx_rhn.help");
			break;
		case 10:
			system("cat sysx_dun.help");
			break;
		case 11:
			system("cat sysx_dfs.help");
			break;
		case 12:
			system("cat sysx_ios.help");
			break;
		case 13:
			system("cat sysx_ppt.help");
			break;
		case 14:
			system("cat sysx_mtc.help");
			break;
		case 15:
			system("cat sysx_mun.help");
			break;
		case 16:
			system("cat sysx_mtf.help");
			break;
		case 17:
			system("cat sysx_ncr.help");
			break;
		case 18:
			system("cat sysx_s.help");
			break;
		case 19:
			system("cat sysx_l.help");
			break;
		case 20:
			system("cat sysx_xsn.help");
			break;
		case 21:
			system("cat sysx_rxm.help");
			break;
		default:
			printf("\nsysx: bad help message number\n");
			break;
		}
	}
	return(cc);
}

/*
 * Read the script name from the terminal
 * and load it into the array `script'.
 * The default script name is always `sysxr'.
 * The script name can be up to 11 characters.
 */

sname()
{
	register char *p;
	register int cc;

gsn:
	do
		printf("\nScript name <sysxr> ? ");
	while((cc = getline(NOHELP)) < 0);
	if(cc > 12) {
		printf("\nName too long");
		goto gsn;
	}
	if(cc == 1)
		p = strcpy(&script, "sysxr.xs");
	else {
		p = strcpy(&script, line);
		p = strcat(p, ".xs");
	}
	return(p);
}

intr()
{
	signal(SIGINT, intr);
	longjmp(savej, 1);
}

swintr()
{
	signal(SIGINT, SIG_IGN);
	swiflag++;
}

tall(cc)
{

	if((cc == 1) ||
	  (strcmp("a", line) == 0) ||
	  (strcmp("all", line) == 0))
		return(1);
	else
		return(0);
}

/*
 * Scan the script in sbuf and load the needed
 * pointers to names and the like into the exerciser
 * control table (exrtab).
 */

setext()
{
	register int i;
	register char *n;

	eflg[0] = 0;
	eflg[1] = 0;
	for(i=0, n = &sbuf; *n; i++) {
		exrtab[i].exn = n;	/* exer name */
		while(*++n != ' ') ;
		*n++ = '\0';		/* terminate name string */
		if(*n == '-')
			exrtab[i].exo = n;	/* options */
		else
			exrtab[i].exo = 0;	/* no options, can't happen */
#ifdef EFLG
		eflg[i>>5] |= (1 << (i & 037));
		nexer++;
		while((*n != '>') && (*n != '&') && (*n != '\n'))
			n++;
		if(exrtab[i].exo)
			*(n-1) = '\0';
#else
		exrtab[i].exkf = 0;
		while((*n != '>') && (*n != '&') && (*n != '\n')) {
			if(*n++ == '-')
				if(*n++ == 'r') {	/* kill file */
					exrtab[i].exkf = ++n;
					while(*++n != ' ') ;
					*n++ = '\0';
				}
		}
		if(exrtab[i].exo && (exrtab[i].exkf == 0))
			*(n-1) = '\0';
#endif
		exrtab[i].exlf = 0;
		if(*n == '>') {
			n += 2;
			exrtab[i].exlf = n;	/* log file */
			while(*++n != ' ') ;
			*n++ = '\0';
		}
		while(*n++ != '\n') ;
		if((*n == 0) && (*(n+1) != 0))
			n++;	/* ed strips nulls from end of line */
		exrtab[i].exs = XSTOP;
		exrtab[i+1].exn = 0;	/* terminate table */
		if(i == (MAXEXR - 1)) {
			printf("\nMaximum of %d exercisers reached !", MAXEXR);
			break;
		}
	}
}

nostop(kfn)
char	*kfn;
{
	register char *q, *p;
	char	xn[20];

	q = &xn;
	p = kfn;
	while(*p != '.')
		*q++ = *p++;
	*q = '\0';
	printf("\n`%8s' did not stop !", &xn);
}

stoped(kfn)
char	*kfn;
{
	register char *q, *p;
	char	xn[20];

	q = &xn;
	p = kfn;
	while(*p != '.')
		*q++ = *p++;
	*q = '\0';
	time(&timbuf);
	printf("\n%8s stopped - %s", &xn, ctime(&timbuf));
}

wmsg()
{
	printf("\nWaiting for shutdown confirmation from exercisers.");
	printf("\nTyping <CTRL/C> will cancel wait loop !\n");
	swiflag = 0;
	signal(SIGINT, swintr);
}

dohelp(s)
char *s;
{
	register int i, j;

/*
	if(args[0] == 0)
		args[0] = "help";
*/
	for(i=0; hsub[i].hs_name; i++)
		if(strcmp(s, hsub[i].hs_name) == 0)
			break;
	if(hsub[i].hs_name == 0) {
		printf("\nNo help available for `%s' subject.\n", s);
		return(1);
	}
	for(j=0; hsub[i].hs_msg[j]; j++) {
		if(hsub[i].hs_msg[j] == -1) {
			printf("%s", gs_more);
			while(getchar() != '\n') ;
			continue;
		}
		printf("\n%s", hsub[i].hs_msg[j]);
	}
}
