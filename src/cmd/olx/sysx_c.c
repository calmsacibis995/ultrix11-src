
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)sysx_c.c	3.1	3/26/87";
/*
 * ULTRIX-11 system exerciser control program (sysx).
 *
 * Part II - script creation (sysx_c)
 *
 *
 * Fred Canter 12/18/83
 * Bill Burns 4/84
 *	added check for rx50 for rax
 * 	added eventflag usage
 * Chung_wu Lee 2/22/85
 *	added TMSCP (TK50/TU81)
 *
 *
 *	Funtions of this program:
 *
 * 1.	Create exerciser run scripts.
 *
 * 2.	Provide on-line help facility for the user.
 *	****** Requires sysx_?.help files ******
 *
 */

char	*gs_more = "\nPress <RETURN> for more:";
char	*gs_prtc = "\nPress <RETURN> to continue:";

char *h_brs[] = 
{
	"",
	"The communications multiplexer exerciser (CMX) varies the ",
	"bit rate between lines in a random fashion. It also varies ",
	"the bit rate on each line periodically. Answering yes to ",
	"this question will cause all lines to be exercised at the ",
	"same constant bit rate. The sysx program will ask for the ",
	"bit rate.",
	"",
	0,
};
char *h_caw[] = 
{
	"",
	"The UDA50/RQDX1,2,3/KLESI disk exerciser (RAX) must be able to ",
	"exercise fixed media disks without destroying data in the ",
	"customer area. The fixed media disks are:",
	"",
	"	UDA50-RA60/RA80/RA81, RQDX1/2/3-(RD31-32, RD51-54)",
	"	and KLESI-RC25",
	"",
	"Customer data is protected by allocating the last 32 blocks ",
	"for RQDX1/2/3, the last 102 blocks for KLESI or the last 1000  ",
	"blocks for UDA50 disks as a maintenance area. ",
	"",
	"Normally, the RAX program will read the entire disk ",
	"but only write on the maintenance area. Answering yes to ",
	"this question will cause the RAX program to enable writes ",
	"to the customer area as well as the maintenance area of the ",
	"disk.",
	"",
	-1,
	"CAUTION!, do not answer yes unless you are absolutely ",
	"certain that it is safe to write on the customer area of ",
	"the disk. Verify that the customer either has no data on ",
	"the disk, or has preserved the data on another disk or on ",
	"magtape.",
	"",
	"Answering yes does not override the normal read only file ",
	"system rules, that is, RAX will not write on the ULTRIX-11 ",
	"ROOT, SWAP, ERROR LOG file systems or on any logically ",
	"mounted USER file systems.",
	"",
	0,
};
char *h_dd[] = 
{
	"",
	"If a device becomes completely inoperative or has a very ",
	"high error rate, an inordinately large log file could ",
	"result. In order to limit the size of the log file and to ",
	"prevent choking the system with errors from a broken device, ",
	"this parameter limits the number of errors on any given ",
	"device. If this limit is exceeded, the exerciser for that ",
	"device will be terminated. The default error limit is 100 ",
	"and the maximum is 1000.",
	"",
	0,
};
char *h_dfs[] = 
{
	"",
	"Most disks are partitioned into 8 pseudo disks called ",
	"file systems, refer to the \"ULTRIX-11 System Management ",
	"Guide\" for an explanation of disk partitions. The default ",
	"mode of operation is to exercise all file systems on the disk. ",
	"To invoke the default mode type `a', `all', or <RETURN>. To ",
	"exercise only one file system type the number of that file ",
	"system followed by a <RETURN>, valid numbers are `0' through ",
	"`7'.",
	"",
	0,
};
char *h_dln[] = 
{
	"",
	"Type the number of the line to be deselected followed by a",
	"<RETURN>. If another line is to be deselected, type that line",
	"number followed by a <RETURN>. To terminate the line deselect",
	"process type just a <RETURN>. The number of lines per unit ",
	"are:",
	"",
	"			DH11	16",
	"			DHU11	16",
	"			DHV11	8",
	"			DZ11	8",
	"			DZV11	4",
	"			DZQ11	4",
	"",
	0,
};
char *h_dmm[] = 
{
	"",
	"For some devices, such as disks, a data mismatch error could ",
	"result in several hundred lines of error printout. This ",
	"parameter limits the number of data mismatch errors printed ",
	"for each occurrence of a data error.  For example: if an ",
	"entire disk sector failed, 256 data mismatch error printouts, ",
	"consisting of three lines each, would be generated. The ",
	"default number of data mismatch errors to print is five and ",
	"the maximum is 256.",
	"",
	0,
};
char *h_dt[] = 
{
	"",
	"Each copy of the communications multiplexer exerciser (CMX) ",
	"exercises one DH11, DHU11, DHV11, DZ11, DZV11, DZQ11 unit or ",
	"from one to 16 DL11 units. Type the device type and the unit ",
	"number in the following format; dh#, dhu#, dhv#, dz#, dzv#, ",
	"dzq# or dl#, where # is the unit number except for the DL11. ",
	"For the DL11 # is 0 for the first 16 DL11 units and 1 for ",
	"the second 16 units. The actual DL11 unit or units to be ",
	"exercised will be selected via the line select question ",
	"asked later. DL11 unit 0 line 0, i.e., the first DL11 is ",
	"always the system console and cannot be exercised !",
	"",
	"For example:",
	"",
	"	dh0	exercise the first DH11 unit",
	"	dhu1	exercise the second DHU11 unit",
	"	dzv0	exercise the first DZV11 unit",
	"	dl0	exercise some or all of the first 16 DL11 ",
	"		units",
	"",
	0,
};
char *h_dun[] = 
{
	"",
	"Type the unit number of the disk to be exercised followed by ",
	"a <RETURN>.  There is no default unit number and only one unit ",
	"number should be entered. Valid unit numbers are `0' through ",
	"`7'.",
	"",
	0,
};
char *h_ifs[] = 
{
	"",
	"The disk exercisers will not write on certain areas of the ",
	"disk if those file systems are being used by the system ",
	"software. A file system will be treated as read only if; it ",
	"is the ROOT file system, i.e., where the ULTRIX-11 kernel ",
	"resides, the swap area, the error log area, or if the file ",
	"system is mounted. If there are read only file systems, the ",
	"disk exerciser will print a list of these file systems and ",
	"the reason that each file system was declared read only. ",
	"Answering yes to this question will inhibit the printing of ",
	"this list. In most cases it is wise NOT to inhibit the read ",
	"only file system printout.",
	"",
	0,
};
char *h_ios[] = 
{
	"",
	"All of the disk exercisers and the communications exerciser ",
	"generate periodic printouts containing the number of read ",
	"and write operations, and the number of errors. This ",
	"parameter may be used to specify the time in minutes between ",
	"these printouts. The default time is 30 minutes. To use the ",
	"default time type a <RETURN>, otherwise type the time interval ",
	"in minutes followed by a <RETURN>. The maximum time is 720 ",
	"minutes or 12 hours.",
	"",
	0,
};
char *h_ism[] = 
{
	"",
	"The magtape exerciser (MTX) prints the type and status of ",
	"each tape unit on the specified controller. Answering yes to ",
	"this question will inhibit the unit status printout.",
	"",
	0,
};
char *h_lb[] = 
{
	"",
	"The DL11, DH11, DZ11, DZV11 and DZQ11 multiplexers have a ",
	"maintenance loopback mode of operation, which loops the ",
	"transmit leads of each port back to the input leads. When ",
	"maintenance loopback mode is invoked, all lines on that unit ",
	"are looped back. This is the normal method of exercising",
	"communications multiplexer lines. An alternate method is to ",
	"connect a turnaround connector to each port to be exercised. ",
	"This method is normally used if only a single line is to be ",
	"exercised.",
	"",
	"The DHU11 and DHV11 multiplexers also have maintenance",
	"loopback mode. However maintenance loopback mode is chosen ",
	"for individual ports, allowing single lines to be tested",
	"using maintenance loopback mode without disturbing the other",
	"ports on the multiplexer.",
	"",
	"NOTE: 	The second serial line unit, on the PDP11/24 ",
	"	processor, does not support maintenance loopback ",
	"	mode. The second SLU may be exercised as a DL11, ",
	"	however a turnaround connector must be used to loop ",
	"	the outputs back to the inputs.",
	"",
	0,
};
char *h_lcp[] = 
{
	"",
	"In order to save paper, the line printer exerciser (LPX) ",
	"prints about 12 pages of actual printout then goes into a ",
	"pause state. In the pause state lpx sends characters to the ",
	"line printer but cancels the printout before it begins. This ",
	"exercises the line printer controller and the system without ",
	"wasting large amounts of paper. Answering yes to this ",
	"question will cause the line printer to print continuously.",
	"",
	0,
};
char *h_ld[] = 
{
	"",
	"The normal mode of operation for the communications ",
	"multiplexer exerciser (CMX) is to exercise all lines on the ",
	"selected DL11, DH11, DHU11, DHV11, DZ11, DZV11 or DZQ11 unit. ",
	"However, one or more individual lines may be disabled. This ",
	"is done by typing the number of the line to be disabled ",
	"followed by a <RETURN>. The sysx program will continue to ask ",
	"for line numbers until only a <RETURN> is typed.",
	"",
	0,
};
char *h_lf[] = 
{
	"",
	"The error messages and other output from the exercisers may ",
	"be printed on the terminal or written out to a log file. ",
	"Either method is acceptable, however, if multiple exercisers ",
	"or multiple copies of the same exerciser are running, error ",
	"message output to the terminal could become scrambled. ",
	"Output should be to log files when multiple exercisers are ",
	"running ! The log file name is automatically generated by ",
	"the sysx program, use the `p' command to obtain the log file ",
	"names. The `l' command is used to print the contents of log ",
	"files.",
	"",
	0,
};
char *h_lnp[] = 
{
	"",
	"In order to save paper, the line printer exerciser (LPX) ",
	"prints about 12 pages of actual printout then goes into a ",
	"pause state. In the pause state lpx sends characters to the ",
	"line printer but cancels the printout before it begins. This ",
	"exercises the line printer controller and the system without ",
	"wasting large amounts of paper. Answering yes to this ",
	"question will cause lpx to enter a constant pause state and ",
	"never generate any actual printouts. This mode exercises the ",
	"line printer controller and the system, but not the line ",
	"printer itself.",
	"",
	0,
};
char *h_ls[] = 
{
	"",
	"The normal mode of operation for the communications ",
	"multiplexer exerciser (CMX) is to exercise all lines on the ",
	"selected DH11, DHU11, DHV11, DZ11, DZV11 or DZQ11 unit. ",
	"However, one or more individual lines may be selected. This ",
	"is done by typing the number of the line to be selected ",
	"followed by a <RETURN>. The sysx program will continue to ask ",
	"for line numbers until only a <RETURN> is typed.",
	"",
	"The DL11 is a special case for line selection. The first 16 ",
	"DL11 units are exercised as lines 0 through 15 of DL unit ",
	"zero, and the second 16 DL11 units are exercised as lines 0 ",
	"through 15 on DL unit one. Line zero of the first DL11, i.e., ",
	"the first physical DL11, is the system console and is never ",
	"exercised. CMX will not allowed line zero of DL unit zero to ",
	"be selected. For example, to exercise the second and third ",
	"physical DL11 units select lines one and two of DL unit zero. ",
	"To exercise physical DL11 unit 16, select line 0 of DL unit ",
	"one.",
	"",
	0,
};
char *h_msm[] = 
{
	"",
	"At the end of each pass, the magtape exerciser (MTX) prints ",
	"the number of read and write operations, and the number of ",
	"hard errors for each drive. Answering yes to this question ",
	"will inhibit the end of pass printout.",
	"",
	0,
};
char *h_mtc[] = 
{
	"",
	"Unlike the other exercisers, which run one copy of the ",
	"exerciser for each unit to be exercised, the magtape ",
	"exerciser (MTX) runs one copy for the controller and that ",
	"copy exercises all drives on the controller. This option ",
	"specifies the type of magtape controller to be exercised. ",
	"There is no default controller type. Type one of the ",
	"following controller types followed by a <RETURN>:",
	"",
	"	ht	for TM02/3 with TU16/TE16/TU77",
	"	tm	for TM11 with TU10/TE10/TS03",
	"	ts	for TS11/TSV05/TU80/TK25 ",
	"	tk	for TK50/TU81 ",
	"",
	0,
};
char *h_mtb[] = 
{
	"",
	"This option specifies the number of blocks that will be ",
	"used by the magtape exerciser (MTX). Typing just a <RETURN> ",
	"invokes the default length of 500 records ( 512 bytes for",
	"block mode or 10240 bytes for raw mode per record), otherwise",
	"type the number of records to be used followed by a <RETURN>.",
	"The maximum is 10000 records and the minimum is 1 records.",
	"",
	0,
};
char *h_mtf[] = 
{
	"",
	"This option specifies the length in feet of tape that will ",
	"be used by the magtape exerciser (MTX). Typing just a <RETURN> ",
	"invokes the default length of 500 feet, otherwise type the ",
	"number of feet to be used followed by a <RETURN>. The maximum ",
	"length is 2400 feet and the minimum is 10 feet.",
	"",
	0,
};
char *h_mun[] = 
{
	"",
	"This option is used to select the magtape unit or units to ",
	"be exercised.  Type <RETURN> to exercise all available drives ",
	"on the specified magtape controller. Otherwise, type the ",
	"number of the unit to be exercised.  The sysx program will ",
	"continue to ask for magtape unit numbers until just a <RETURN> ",
	"is typed. The maximum number of drives per controller are:",
	"",
	"",
	"	ht	TM02/3				64",
	"	tm	TM11		 		 8",
	"	ts	TS11/TSV05/TSU05/TU80/TK25	 4",
	"	tk	TK50/TU81			 4",
	"",
	0,
};
char *h_ncr[] = 
{
	"",
	"Some of the exercisers, such as CPX and FPX, allow multiple ",
	"copies of the same exerciser to be running concurrently. ",
	"This options allows the number of copies to be specified. ",
	"The number enclosed in < > is the recommended number of ",
	"copies, type a <RETURN> to use this default number. Otherwise,",
	"type the number of copies to be run followed by a <RETURN>. ",
	"The maximum number of copies is 50. If the entire system is ",
	"to be exercised, i.e., all devices, memory, the CPU, and ",
	"floating point are running concurrently, it is strongly ",
	"recommended that only the default number of copies of CPX ",
	"and FPX be run. If CPX or FPX is the only exerciser running, ",
	"then up to 50 copies may be running concurrently.",
	"",
	0,
};
char *h_ppt[] = 
{
	"",
	"In order to save paper, the line printer exerciser (LPX) ",
	"prints about 12 pages of actual printout then goes into a ",
	"pause state. In the pause state lpx sends characters to the ",
	"line printer but cancels the printout before it begins. This ",
	"exercises the line printer controller and the system without ",
	"wasting large amounts of paper. This parameter is used to ",
	"specify the length of the pause time. To use the default ",
	"time of 15 minutes type a <RETURN>, otherwise type the desired ",
	"pause time followed by a <RETURN>. The maximum pause time is ",
	"480 minutes or 8 hours.",
	"",
	0,
};
char *h_rhn[] = 
{
	"",
	"The (HPX) disk exerciser supports various combinations of ",
	"RM02/3/5, RP04/5/6, and ML11 disks connected to up to three ",
	"RH11 or RH70 controllers. The ULTRIX-11 RH controller number ",
	"is not specified by the physical or electrical position of ",
	"the RH controller on the bus. Respond to the question with ",
	"one of the following controller numbers:",
	"",
	"0 - The first RH controller with RM02/3/5, RP04/5/6 and/or",
	"	ML11 disks connected to it. Disks refered to as \"hp\". ",
	"",
	"1 - The second RH controller with RM02/3/5, RP04/5/6 and/or",
	"	ML11 disks connected to it. Disks refered to as \"hm\". ",
	"",
	"2 - The third RH controller with RM02/3/5, RP04/5/6 and/or",
	"	ML11 disks connected to it. Disks refered to as \"hj\". ",
	"",
	0,
};
char *h_udn[] = 
{
	"",
	"The (RAX) disk exerciser supports multiple MSCP controllers ",
	"on the same system. The MSCP controllers are: ",
	"",
	"	UDA50/UDA50A - for RA60/80/81 disks ",
	"	RQDX1/2/3 - for RD31-32, RD51-54, RX50, RX33 disks ",
	"	KLESI - for RC25 disks ",
	"	RUX1 - for RX50 disks ",
	"",
	"Controllers are assigned numbers based upon the order that ",
	"they are specified during the system generation. Below is  ",
	"a listing of the MSCP controllers in the current configuration. ",
	"",
	"",
	0,
};
char *h_rxm[] = 
{
	"",
	"The RX02 floppy disk drive can operate at one of two ",
	"densities; double density (RX02 mode) or single density ",
	"(RX01 compatibility mode). This option selects the density ",
	"at which the diskettes will be written. The density is ",
	"specified by typing the appropriate number from the list ",
	"below followed by a <RETURN>:",
	"",
	"	1 - Single density (RX01 mode)",
	"	2 - Double density (RX02 mode)",
	"	0 - Both single and double density modes",
	"",
	"By default HXX alternates modes, exercising the RX02 at ",
	"single density for a period of time, then at double density ",
	"for an equal period of time. To select the default option, ",
	"type zero followed by a <RETURN> or just a <RETURN>.",
	"",
	0,
};
char *h_sbr[] = 
{
	"",
	"This parameter is used to set the transmit and receive speed ",
	"on a line or group of lines. Type the desired speed followed ",
	"by a <RETURN>. Available speeds are:",
	"",
	"		110	BPS",
	"		300	BPS",
	"		1200	BPS",
	"		2400	BPS",
	"		4800	BPS",
	"		9600	BPS",
	"",
	"The bit rate need not be specified, to omit the bit rate ",
	"specification type only a <RETURN>. In this case a random bit ",
	"rate pattern will be used, unless a fixed bit rate has been ",
	"specified for ALL lines.",
	"",
	"NOTE:	The DL11 does not support programmable bit rates. ",
	"	A fixed bit rate must be specified for each DL11 line.",
	"",
	0,
};
char *h_sln[] = 
{
	"",
	"Type the number of the line to be selected followed by a",
	"<RETURN>. If another line is to be selected, type that line",
	"number followed by a <RETURN>. To terminate the line select",
	"process type just a <RETURN>. The number of lines per unit are:",
	"",
	"		DH11	16",
	"		DHU11	16",
	"		DHV11	8",
	"		DZ11	8",
	"		DZV11	4",
	"		DZQ11	4",
	"		DL11	16",
	"",
	"The first 16 physical DL11 units are treated as lines 0 ",
	"through 15 of logical DL unit zero, and the second 16 ",
	"physical DL11 units are treated as logical DL unit one. Do ",
	"not select line zero of logical DL unit zero, this is the ",
	"system console terminal.",
	"",
	0,
};
char *h_xn[] = 
{
	"",
	"Type the name of the exerciser for the device to be ",
	"exercised followed by a <RETURN>. An exerciser name may ",
	"be used more than once in the same script.  Use the `n' ",
	"command to obtain the exerciser name for each device.",
	"",
	0,
};

struct hsub {
	char	*hs_name;
	char	**hs_msg;
} hsub[] = {
	"brs",		&h_brs,
	"caw",		&h_caw,
	"dd",		&h_dd,
	"dfs",		&h_dfs,
	"dln",		&h_dln,
	"dmm",		&h_dmm,
	"dt",		&h_dt,
	"dun",		&h_dun,
	"ifs",		&h_ifs,
	"ios",		&h_ios,
	"ism",		&h_ism,
	"lb",		&h_lb,
	"lcp",		&h_lcp,
	"ld",		&h_ld,
	"lf",		&h_lf,
	"lnp",		&h_lnp,
	"ls",		&h_ls,
	"msm",		&h_msm,
	"mtc",		&h_mtc,
	"mtb",		&h_mtb,
	"mtf",		&h_mtf,
	"mun",		&h_mun,
	"ncr",		&h_ncr,
	"ppt",		&h_ppt,
	"rhn",		&h_rhn,
	"rxm",		&h_rxm,
	"sbr",		&h_sbr,
	"sln",		&h_sln,
	"xn",		&h_xn,
	"udn",		&h_udn,
	0
};


#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <a.out.h>
#include <sys/ra_info.h>
#include <sys/tk_info.h>

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
#define DECN_RH	8	/* HPX -c#, RH11/RH70 controller number */
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
#define DECN_UD	21	/* RAX -c#, MSCP disk controller number */

int	cputype;

/*
 * Below needed for proper handling
 * of rax "maint area" question
 * Bill 4/20/84
#define RX33	33
#define RX50	50

struct	ra_drv {		
	char	ra_dt;	
	char	ra_online;
	union {
		daddr_t	ra_dsize;
		struct {
			int	ra_dslo;
			int	ra_dshi;
		};
	} d_un;
};
 */


struct nlist nl[] =
{
	{ "_cputype" },
	{ "_ra_drv" },
	{ "_ra_index" },
	{ "_nra" },
	{ "_ra_ctid" },
	{ "_nuda" },
	{ "_tk_ctid" },
	{ "_tk_csr" },
	{ "_ntk" },
	{ "" },
};
char	ra_index[4];	/* offsets to the ra_drv structure */
struct ra_drv *radp;	/* pointer for calloc'd area for MSCP info */
struct ra_drv *bradp;	/* base pointer for calloc'd area for MSCP info */
char nra[4];		/* place for nra kernel array */
char ra_ctid[4];	/* controller id's /*

long	evntflg();	/* type returned by event flag syscall */
char tk_ctid[MAXTK];
int tk_csr[MAXTK];
int ntk;
int mtdrv;
u_char tkdn;
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
	/* DECN_RH MUST proceed DEDN for HPX */
	{ "hpx",LOGF,NEPRNT,NEDROP,DECN_RH,DEDN,DEFS,DESTAT,DEIFS,-1 },
	{ "hkx",LOGF,NEPRNT,NEDROP,DEDN,DEFS,DESTAT,DEIFS,-1 },
	{ "rpx",LOGF,NEPRNT,NEDROP,DEDN,DEFS,DESTAT,DEIFS,-1 },
	{ "rlx",LOGF,NEPRNT,NEDROP,DEDN,DEFS,DESTAT,DEIFS,-1 },
	{ "rkx",LOGF,NEPRNT,NEDROP,DEDN,DESTAT,DEIFS,-1 },
	{ "rax",LOGF,NEPRNT,NEDROP,DECN_UD,DEDN,DEWRT,DEFS,DESTAT,DEIFS,-1 },
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

int	alrmflag;

jmp_buf	savej;

int	buflag;		/* used to cancel a line */

int	swiflag;	/* flag to interrupt exer startup wail loop */
int	mem;
int	ra;		/* flag set to indicate current system MSCP
			   information has been read in */
int	radcnt;		/* total count of MSCP disk drives */
int	nuda;		/* count of MSCP controllers */
int	tkflag;		/* flag set to indicate TK50 */
int	mtflag;		/* flag set to indicate type of drive */

#define	ISHT	01
#define	ISTK	02
#define	ISTM	04
#define	ISTS	010

main(argc, argv)
int argc;
char *argv[];
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
	int	dn, cn;

	if(argc != 2)
		exit(1);
	
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
	setjmp(savej);
cloop:
cloop1:

/* 
	case 'c':	create script 
 */
	strcpy(script, argv[1]);
	p = &script;
	p = &sbuf[0];	/* clear script buffer */
	for(i=0; i<SBSIZE+256; i++)
		*p++ = 0;
	printf("\nTo cancel a script entry, type <CTRL/D> !");
	printf("\nAnswer any question with a `?' for help !\n");
csloop:
	buflag = 0;
	do
		printf("\nExerciser name ? ");
	while((cc = getline(1)) < 0);
	if(buflag) {
		printf("\n");
		goto csloop;	/* ^D - cancel script line */
	}
	if(cc == 1) {	/* return - script is complete */
		printf("\nConfirm script complete");
		if(yes(NO, NOHELP) != YES)
			goto csloop;
	csdone:
		if((fd = creat(&script, 0744)) < 0) {
		printf("\nsysx: Can,t create %s script file\n", script);
			exit(1);
		}
		for(i=0; i<(SBSIZE+256); i++)
		  if((sbuf[i] == '\n')&&(sbuf[i+1] == 0)&&(sbuf[i+2] == 0))
				break;
		i++;
		if(write(fd, (char *)&sbuf, i) != i) {
			printf("\nsysx: %s file write error\n", script);
			exit(1);
		}
		close(fd);
		exit(0);
	}
	if(cc > 5) {
	badn:
		printf("\nBad name");
		goto csloop;
	}
	for(xp=xrtab; xp->xn; xp++)
		if(strcmp(line, xp->xn) == 0)
			break;
	if(xp->xn == 0) {
		printf("\nNo such exerciser");
		goto csloop;
	}
	for( p = &sbuf[0], i=0; i<SBSIZE; i++, p++) {
		if((*p == 0) && (i == 0))
			break;	/* buffer empty */
		if(i == SBSIZE)
			goto sbfull;
		if((*p == 0) && (*(p+1) == 0)) {	/* end of buf */
			if(p >= &sbuf[SBSIZE]) {
			sbfull:
				printf("\nScript buffer full !");
				goto csdone;
			}
			/* p++;	found end ok */
			/* above line removed to avoid nulls in script files */
			break;
		}
	}
	ncopy = 0;	/* If cpx or fpx, allow multiple copies */
	if(strcmp(xp->xn, "cpx") == 0)
		ncopy = 1;	/* 1 copy recommended */
	if(strcmp(xp->xn, "fpx") == 0)
		ncopy = 2;	/* 2 copies recommended */
	if(strcmp(xp->xn, "rax") == 0) {
		if(!ra)
			rasetup();	 /* get mscp disk info from kernel */
		if(!nuda) {
			printf("\n\007\007\007There are no MSCP controllers ");
			printf("in the current system configuration !\n");
			goto csloop;
		}
	}
	if(strcmp(xp->xn, "mtx") == 0) {
		ntk = 0;
		if(nl[6].n_value > 0) {
			lseek(mem, (long)nl[6].n_value, 0);
			read(mem, &tk_ctid, sizeof(tk_ctid));
		}
		if(nl[7].n_value > 0) {
			lseek(mem, (long)nl[7].n_value, 0);
			read(mem, &tk_csr, sizeof(tk_csr));
		}
		if(nl[8].n_value > 0) {
			lseek(mem, (long)nl[8].n_value, 0);
			read(mem, &ntk, sizeof(ntk));
		}
		tkdn = 0;
	}
	savebp = p;		/* save buffer pointer for ^D cancel */
	p = strcat(p, xp->xn);	/* load exer name into script buffer */
	p = strcat(p, " ");
	if(strcmp("cmx", xp->xn) == 0)	/* force -i option to cmx */
		p = strcat(p, "-i ");
	i = 0;
	while(xp->opt[i] >= 0)	/* load options */
		switch(xp->opt[i++]) {
		case LOGF:
			logfil = 0;
		clf:
			printf("\nOutput errors to log file");
			if((yn = yes(YES, HELP)) == -1) {
				dohelp("lf");
				goto clf;
			}
			if(buflag) {	/* ^D - cancel script line */
			ckill:
				n = savebp;
				while(*n)
					*n++ = 0;
				printf("\n");
				goto csloop;
			}
			if(yn == YES)
				logfil++;
			sprintf(&logfn, "%s_1", xp->xn);
			n = &logfn[0];
			while(*n++ != '_');
			break;
		case NEPRNT:
		nep:
			do
				printf("\nData error printout limit <5> ? ");
			while((cc = getline(2)) < 0);
			if(buflag)
				goto ckill;
			if(cc == 1) {
				p = strcat(p, "-n 5 ");
				break;
			}
			j = atoi(line);
			if((j <= 0) || (j > 256)) {
				printf("\nBad limit, range is 1 -> 256");
				goto nep;
			}
			p = strcat(p, "-n ");
			p = strcat(p, line);
			p = strcat(p, " ");
			break;
		case NEDROP:
		ned:
			do
			 printf("\nDrop device after how many errors <100> ? ");
			while((cc = getline(3)) < 0);
			if(buflag)
				goto ckill;
			if(cc == 1) {
				p = strcat(p, "-e 100 ");
				break;
			}
			j = atoi(line);
			if((j <= 0) || (j > 1000)) {
				printf("\nBad number, range is 1 -> 1000");
				goto ned;
			}
			p = strcat(p, "-e ");
			p = strcat(p, line);
			p = strcat(p, " ");
			break;
		case CMUNIT:
		cmu:
			do {
		printf("\nFor DL11, # is NOT unit number, type ? for help !");
		printf("\nDevice type & unit number "); 
		printf("\n(dh#, dhu#, dhv#, dz#, dzv#, dzq#, dl#) ? ");
			} while((cc = getline(4)) < 0);
			if(buflag)
				goto ckill;
			if((cc > 5) ||( cc < 4) || (line[0] != 'd')) {
			cmubad:
				printf("\nBad device");
				goto cmu;
			}
			q = &line[1];
			if((*q != 'h') && (*q != 'z') && (*q != 'l'))
				goto cmubad;
			if(*++q == 'v')
				q++;
			else if(*q == 'q')
				q++;
			else if(*q == 'u')
				q++;
			else if(cc > 4)
				goto cmubad;
			if((*q < '0') || (*q > '7'))
				goto cmubad;
			p = strcat(p, "-");
			p = strcat(p, line);
			p = strcat(p, " ");
			*n = 0;	/* log file name */
			n = strcat(&logfn, line);
			break;
		case CMSLB:
		clb:
			printf("\nUse maintenance loopback mode");
			if((yn = yes(YES, HELP)) == -1) {
				dohelp("lb");
				goto clb;
			}
			if(buflag)
				goto ckill;
			if(yn == NO)
				p = strcat(p, "-m ");
			break;
		case CMLS:
		cls:
			printf("\nSelect line(s)");
			if((yn = yes(NO, HELP)) == -1) {
				dohelp("ls");
				goto cls;
			}
			if(buflag)
				goto ckill;
			if(yn == NO)
				break;
		cmlsel:
			do
				printf("\nLine ? ");
			while((cc = getline(5)) < 0);
			if(buflag)
				goto ckill;
			if(cc == 1)
				break;
			if((cc > 3) || (atoi(line) > 15)) {
				printf("\nBad line number");
				goto cmlsel;
			}
			p = strcat(p, "-l ");
			p = strcat(p, line);
			p = strcat(p, " ");
		cmlsbr:
			do
				printf("\nBit rate <Random> ? ");
			while((cc = getline(6)) < 0);
			if(buflag)
				goto ckill;
			if(cc == 1)
				goto cmlsel;
			if((cc > 5) || (atoi(line) > 9600)) {
				printf("\n Bad bit rate");
				goto cmlsbr;
			}
			p = strcat(p, line);
			p = strcat(p, " ");
			goto cmlsel;
		case CMLDS:
		cld:
			printf("\nDisable line(s)");
			if((yn = yes(NO, HELP)) == -1) {
				dohelp("ld");
				goto cld;
			}
			if(buflag)
				goto ckill;
			if(yn == NO)
				break;
		cmldes:
			do
				printf("\nLine ? ");
			while((cc = getline(7)) < 0);
			if(buflag)
				goto ckill;
			if(cc == 1)
				break;
			if((cc >3) || (atoi(line) > 15)) {
				printf("\nBad line number");
				goto cmldes;
			}
			p = strcat(p, "-u ");
			p = strcat(p, line);
			p = strcat(p, " ");
			goto cmldes;
		case CMBR:
		cmbrs:
			printf("\nSelect bit rate for all lines");
			if((yn = yes(NO, HELP)) == -1) {
				dohelp("brs");
				goto cmbrs;
			}
			if(buflag)
				goto ckill;
			if(yn == NO)
				break;
			do
				printf("\nBit rate <9600> ? ");
			while((cc = getline(8)) < 0);
			if(buflag)
				goto ckill;
			if((cc > 5) || (atoi(line) > 9600)) {
				printf("\n Bad bit rate");
				goto cmbrs;
			}
			p = strcat(p, "-b ");
			if(cc == 1)
				p = strcat(p, "9600");
			else
				p = strcat(p, line);
			p = strcat(p, " ");
			break;
		case DECN_RH:
		drhn:
			do {
				printf("\nRH11/RH70 controller number");
				printf(" < Type ? for help ! > ? ");
			} while((cc = getline(9)) < 0);
			if(buflag)
				goto ckill;
			if((cc != 2) || (line[0] < '0') || (line[0] > '2')) {
				printf("\nBad RH number");
				goto drhn;
			}
			p = strcat(p, "-c");
			p = strcat(p, line);
			p = strcat(p, " ");
			*n = 0;	/* log file name */
			n = strcat(&logfn, line);
			n = strcat(&logfn, "_0");
			while(*n++ != '_') ;
			while(*n++ != '_') ;
			break;
		case DECN_UD:
		dudn:
			do {
				printf("\nMSCP disk controller number");
				printf(" < Type ? for help ! > ? ");
			} while((cc = getline(22)) < 0);
			if(buflag)
				goto ckill;
			if((cc != 2) || (line[0] < '0') || (line[0] > '3')) {
				printf("\nBad MSCP controller number");
				goto dudn;
			}
			cn = atoi(line);
			p = strcat(p, "-c");
			p = strcat(p, line);
			p = strcat(p, " ");
			*n = 0;	/* log file name */
			n = strcat(&logfn, line);
			n = strcat(&logfn, "_0");
			while(*n++ != '_') ;
			while(*n++ != '_') ;
			break;
		case DEDN:
		dunit:
			do
				printf("\nUnit number ? ");
			while((cc = getline(10)) < 0);
			if(buflag)
				goto ckill;
			if((cc != 2) || (line[0] < '0') || (line[0] > '7')) {
				printf("\nBad unit number");
				goto dunit;
			}
			dn = atoi(line);
			p = strcat(p, "-d");
			p = strcat(p, line);
			p = strcat(p, " ");
			*n = 0;	/* log file name */
			n = strcat(&logfn, line);
			break;
		case DEWRT:
		dwrt:
			radp = bradp;
			radp += ra_index[cn];
			radp += dn;
			if((radp->ra_dt == RX50) || (radp->ra_dt == RX33)) {
				p = strcat(p, "-x ");
				break;
			}
			printf("\nAllow writes on customer data area");
			if((yn = yes(NO, HELP)) == -1) {
				dohelp("caw");
				goto dwrt;
			}
			if(buflag)
				goto ckill;
			if(yn == YES) {
			    printf("\n******\07\07\07 CAUTION !, writes ");
			    printf("to customer area will be allowed ******\n");
			    p = strcat(p, "-w ");
			} else {
			    printf("\n******\07\07\07 Writes will be to ");
			    printf("maintenance area only ******\n");
			}
			break;
		case DRXMOD:
		rxmode:
			do
			    printf("\nMode <1 = RX01, 2 = RX02, 0 = both> ? ");
			while((cc = getline(21)) < 0);
			if(buflag)
				goto ckill;
			if(cc == 1)
				break;
			if((cc != 2) || (line[0] < '0') || (line[0] > '2')) {
				printf("\nBad mode");
				goto rxmode;
			}
			if(line[0] != '0') {
				p = strcat(p, "-m");
				p = strcat(p, line);
				p = strcat(p, " ");
			}
			break;
		case DEFS:
		dfs:
			do
				printf("\nFile system(s) <all> ? ");
			while((cc = getline(11)) < 0);
			if(buflag)
				goto ckill;
			if(tall(cc))
				break;
			if((cc != 2) || (line[0] < '0') || (line[0] > '7')) {
				printf("\nBad file system");
				goto dfs;
			}
			p = strcat(p, "-f");
			p = strcat(p, line);
			p = strcat(p, " ");
			break;
		case DESTAT:
		dstat:
			do
	printf("\nHow many minutes between I/O statistics printouts <30> ? ");
			while((cc = getline(12)) < 0);
			if(buflag)
				goto ckill;
			if(cc == 1)
				break;
			if((cc > 4) || (atoi(line) <= 0) || (atoi(line) > 720)) {
				printf("\nBad interval");
				goto dstat;
			}
			p = strcat(p, "-s");
			p = strcat(p, line);
			p = strcat(p, " ");
			break;
		case DEIFS:
		difs:
			printf("\nInhibit disk file system status printout");
			if((yn = yes(NO, HELP)) == -1) {
				dohelp("ifs");
				goto difs;
			}
			if(buflag)
				goto ckill;
			if(yn == YES)
				p = strcat(p, "-i ");
			break;
		case LPNP:
		lpcp:
			printf("\nLP continuous printing");
			if((yn = yes(NO, HELP)) == -1) {
				dohelp("lcp");
				goto lpcp;
			}
			if(buflag)
				goto ckill;
			if(yn == YES) {
				p = strcat(p, "-p0 ");
				break;
			}
		lpnp:
			printf("\nLP (NO PRINT) exercise controller only");
			if((yn = yes(NO, HELP)) == -1) {
				dohelp("lnp");
				goto lpnp;
			}
			if(buflag)
				goto ckill;
			if(yn == YES) {
				p = strcat(p, "-p ");
				break;
			}
		lppt:
			do
			printf("\nLP pause (NO PRINT) time in minutes <15> ? ");
			while((cc = getline(13)) < 0);
			if(buflag)
				goto ckill;
			if(cc == 1) {
				p = strcat(p, "-p15 ");
				break;
			}
			if((cc > 4) || (atoi(line) < 0) || (atoi(line) > 480)) {
				printf("Bad interval, 8 hours maximum");
				goto lppt;
			}
			p = strcat(p, "-p");
			p = strcat(p, line);
			p = strcat(p, " ");
			break;
		case MTTYPE:
		mtct:
			mtflag = 0;
			mtdrv = 0;
			do {
			printf("\nMagtape controller type < ht, tm, ts or tk ");
				printf("- type ? for help ! > ? ");
			} while((cc = getline(14)) < 0);
			if(buflag)
				goto ckill;
			if(cc != 3) {
				printf("\nBad controller type");
				goto mtct;
			}
			if (strcmp(line, "ht") == 0)
				mtflag = ISHT;
			else if (strcmp(line, "tm") == 0)
				mtflag = ISTM;
			else if (strcmp(line, "ts") == 0)
				mtflag = ISTS;
			else if (strcmp(line, "tk") == 0)
				mtflag = ISTK;
			else {
				printf("\nBad controller type");
				goto mtct;
			}
			if(mtflag&ISTK) {
				if(ntk <= 0) {
					printf("\n\007\007\007There are no TMSCP controllers ");
					printf("in the current system configuration !\n");
					goto mtct;
				}
				mtdrv = ntk;
			}
			else
				mtdrv = 1;
			p = strcat(p, "-");
			p = strcat(p, line);
			p = strcat(p, " ");
			*n = 0;	/* log file name */
			n = strcat(&logfn, line);
			break;
		case MTDN:
			mtfirst = 0;
		mtds:
			do
				printf("\nMagtape unit number <all> ? ");
			while((cc = getline(15)) < 0);
			if(buflag)
				goto ckill;
			if((mtfirst == 0) && (tall(cc))) {
				if(mtflag&ISTK)
					tkdn |= 017;
				break;
			}
			if(cc == 1)
				break;
			j = atoi(line);
			if((cc > 3) || j < 0 || j > 63)
				goto bmtds;
			if((mtflag&ISTM) && j > 7)
				goto bmtds;
			if((mtflag&ISTS) && j > 3)
				goto bmtds;
			if(mtflag&ISTK) {
				if (j > 3)
					goto bmtds;
				else
					tkdn |= (1 << j);
			}
			p = strcat(p, "-d");
			p = strcat(p, line);
			p = strcat(p, " ");
			mtfirst++;
			goto mtds;
		bmtds:
			printf("\nBad unit number");
			goto mtds;
		case MTIDS:
		mids:
			printf("\nInhibit magtape unit status message");
			if((yn = yes(NO, HELP)) == -1) {
				dohelp("ism");
				goto mids;
			}
			if(buflag)
				goto ckill;
			if(yn == YES)
				p = strcat(p, "-i ");
			break;
		case MTSTAT:
		msm:
			printf("\nSuppress end of pass I/O statistics");
			if((yn = yes(NO, HELP)) == -1) {
				dohelp("msm");
				goto msm;
			}
			if(buflag)
				goto ckill;
			if(yn == YES)
				p = strcat(p, "-s ");
			break;
		case MTFEET:
			for (j=0; j<mtdrv; j++) {
				tkflag = 0;
				if (mtflag&ISTK) {
					if (tk_csr[j] == 0 || (tkdn&(1<<j)) == 0) {
						p = strcat(p, "-f0 ");
						continue;
					}
					if (((tk_ctid[j] >> 4)&017) == TK50)
						tkflag = 1;
					else if (((tk_ctid[j] >> 4)&017) == TU81)
						tkflag = 2;
					else {
						p = strcat(p, "-f0 ");
						continue;
					}
				}
		mtlen:
				do
					if (tkflag <= 0)
						printf("\nLength of tape <500 feet> ? ");
					else {
						printf("\nUnit %d - ", j);
						if (tkflag == 1)
							printf("tk50\n\nNumber of records <500 records> ? ");
						else
							printf("tu81\n\nLength of tape <500 feet> ? ");
					}
				while((cc = getline(16)) < 0);
				if(buflag)
					goto ckill;
				if (tkflag != 1) {
					if(cc == 1) {
						p = strcat(p, "-f500 ");
						continue;
					}
					if((cc > 5)||(atoi(line) < 10)||(atoi(line) > 2400)) {
						printf("\nBad length");
						goto mtlen;
					}
				}
				else {
					if(cc == 1) {
						p = strcat(p, "-f500 ");
						continue;
					}
					if((cc > 6)||(atoi(line) < 1)||(atoi(line) > 10000)) {
						printf("\nBad number");
						goto mtlen;
					}
				}
				p = strcat(p, "-f");
				p = strcat(p, line);
				p = strcat(p, " ");
			}
			break;
		default:
			printf("\nBad option, ingored !");
			break;
		}
		if(logfil) {
			p = strcat(p, "> ");
			p = strcat(p, &logfn);
			if(ncopy)
				p = strcat(p, "0.log");
			else
				p = strcat(p, ".log");
		}
		p = strcat(p, " &\n");
		ncpy:
		if(ncopy) {	/* If cpx or fpx, allow multiple copies */
			do
			  printf("\nNumber of copies to run <%d> ? ", ncopy);
			while((cc = getline(17)) < 0);
			if(buflag)
				goto ckill;
			if(cc != 1) {	/* wants > recommended # of copies */
				if(cc > 3) {
				badnc:
					printf("\nBad number of copies");
					goto ncpy;
				}
				ncopy = atoi(line);
				if((ncopy < 0) || (ncopy > 50))
					goto badnc;
			}
			for(i=0; i<ncopy; i++) {
				if(logfil) {
					n = p;
					while(*n++ != '>') ;
					while(*n++ != '_') ;
					*n++ = ((i+1)/10) + '0';
					*n++ = ((i+1)%10) + '0';
				}
				if(i == (ncopy - 1)) {
					while(*p++) ;
					*p++ = 0;
					break;
				}
				n = p;
				while(*n) 
					n++;
				while((*n++ = *p++) != '\n')
					;
			/*	p = strcpy(n, p); */
				if(p > &sbuf[SBSIZE])
					goto sbfull;
			}
		}
		goto csloop;
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
	alrmflag = 0;
	fflush(stdout);
	cc = read(0, (char *)&resp, 10);
	if((cc == -1) && alrmflag)
		goto yn;
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
	alrmflag = 0;
	fflush(stdout);
	while((cc = read(0, (char *)&line, 50)) >= 50)
		printf("\nToo many characters, try again !\n");
	if((cc == -1) && alrmflag)
		goto loop;
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
			dohelp("xn");
			break;
		case 2:
			dohelp("dmm");
			break;
		case 3:
			dohelp("dd");
			break;
		case 4:
			dohelp("dt");
			break;
		case 5:
			dohelp("sln");
			break;
		case 6:
			dohelp("sbr");
			break;
		case 7:
			dohelp("dln");
			break;
		case 8:
			dohelp("sbr");
			break;
		case 9:
			dohelp("rhn");
			break;
		case 10:
			dohelp("dun");
			break;
		case 11:
			dohelp("dfs");
			break;
		case 12:
			dohelp("ios");
			break;
		case 13:
			dohelp("ppt");
			break;
		case 14:
			dohelp("mtc");
			break;
		case 15:
			dohelp("mun");
			break;
		case 16:
			if (tkflag <= 0)
				dohelp("mtf");
			else
				dohelp("mtb");
			break;
		case 17:
			dohelp("ncr");
			break;
		case 18:
			dohelp("s");
			break;
		case 19:
			dohelp("l");
			break;
		case 20:
			dohelp("xsn");
			break;
		case 21:
			dohelp("rxm");
			break;
		case 22:
			dohelp("udn");
			rahlp();
			break;
		default:
			printf("\nsysx: bad help message number\n");
			break;
		}
	}
	return(cc);
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

dohelp(s)
char *s;
{
	register int i, j;

/* if(args[0] == 0)
		args[0] = "help"; */
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

/*
 * Get MSCP disk information
 *
 * required for rax controller help message and
 * to elimiante "maint area question for RX50's
 */
rasetup()
{
	int i;

	ra++;
	lseek(mem, (long)nl[2].n_value, 0);
	read(mem, (char *)&ra_index, sizeof(ra_index));
	lseek(mem, (long)nl[3].n_value, 0);
	read(mem, (char *)&nra, sizeof(nra));
	lseek(mem, (long)nl[4].n_value, 0);
	read(mem, (char *)&ra_ctid, sizeof(ra_ctid));
	lseek(mem, (long)nl[5].n_value, 0);
	read(mem, (char *)&nuda, sizeof(nuda));
	if(!nuda)
		return;
	radcnt = 0;
	for( i = 0; i < MAXUDA; i++)
		radcnt += nra[i];
	bradp = calloc(radcnt, sizeof(struct ra_drv));
	radp = bradp;
	if(radp == NULL) {
		printf("sysx: cannot allocate space for MSCP drive info\n");
		exit(1);
	}
	lseek(mem, (long)nl[1].n_value, 0);
	for(i = 0; i < radcnt; i++, radp++) {
		read(mem, (char *)radp, sizeof(struct ra_drv));
	}
}
/*
 * print MSCP controller information
 */
rahlp()
{
	int i, count;
	
	count = 0;

	if(nuda == 1) {
		printf("\nThere is 1 MSCP controller in the ");
		printf("current system configuration.\n");
	} else {
		printf("\nThere are %d MSCP controllers in the ",nuda);
		printf("current system configuration.\n");
	}
	i = 0;
	while(nra[i]) {
		printf("\tController #%d is a ",i);
		switch((ra_ctid[i] >> 4) & 017) {
		case UDA50:
			printf("UDA50\n");
			break;
		case UDA50A:
			printf("UDA50A\n");
			break;
		case KDA50:
			printf("KDA50\n");
			break;
/* NO KDA25 support -- Fred
		case KDA25:
			printf("KDA25\n");
			break;
*/
		case KLESI:
			printf("KLESI\n");
			break;
		case RQDX1:
			printf("RQDX1\n");
			break;
		case RQDX3:
			printf("RQDX3\n");
			break;
		case RUX1:
			printf("RUX1\n");
			break;
		default:
			printf("unknown\n");
			break;
		}
		i++;
	}
}
