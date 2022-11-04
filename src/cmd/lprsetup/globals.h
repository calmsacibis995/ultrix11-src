
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/* 	SCCSID: @(#)globals.h	3.0	4/21/86	*/

/***************************************
*  global variables for lprsetup program
***************************************/

char    progname[] = "lprsetup";/* name of program 			*/
char    pnum[LEN];		/* printer number we are working on	*/
char    pname[LEN];		/* printer name		 		*/
char	longname[LEN];		/* printer name and synonyms		*/
char    ptype[LEN];		/* printer type		 		*/
char    symbolname[LEN];	/* symbol just read in; this needs to be*/
				/* large since is also used to hold the	*/
				/* value of the new symbol. 		*/
char    bp[1024];		/* return from tgetent	 		*/
char	oldfilter[LEN];		/* print filter before modify		*/

/*
 * This structure holds symbols from the PRINTCAP file.
 * The used flag indicates whether the symbol is in
 * use or not, if so, it changes from NO to YES.
 */

struct table    tab[] =
{
    "af", "/usr/adm/lpacct", STR, NO, 0,
    "br", "1200", INT, NO, 0,
    "dn", "/usr/lib/lpd", STR, NO, 0,
    "du", "0", INT, NO, 0,
    "fc", "none", INT, NO, 0,
    "ff", "\\f", STR, NO, 0,
    "fo", "off", BOOL, NO, 0,
    "fs", "none", INT, NO, 0,
    "lf", "/dev/console", STR, NO, 0,
    "lo", "lock", STR, NO, 0,
    "lp", "/dev/lp", STR, NO, 0,
    "mx", "1000", INT, NO, 0,
    "nc", "off", BOOL, NO, 0,
    "of", "/usr/lib/ulf", STR, NO, 0,
    "pl", "66", INT, NO, 0,
    "pw", "132", INT, NO, 0,
    "rw", "off", BOOL, NO, 0,
    "sd", "/usr/spool/lpd", STR, NO, 0,
    "sf", "off", BOOL, NO, 0,
    "sh", "off", BOOL, NO, 0,
    "tr", "none", STR, NO, 0,
    "xc", "none", INT, NO, 0,
    "xs", "none", INT, NO, 0,
    0, 0, 0, 0, 0
};

char h_af[] =
{"\n\
The 'af' parameter is the name of the accounting file used to		\n\
keep track of the number of pages printed by each user for each		\n\
printer.  The name of the accounting file should be unique for		\n\
each printer on your system.						\n\
"};

char h_br[] =
{"\n\
The 'br' parameter specifies the baud rate for the printer.		\n\
The baud rate is dependent upon the printer hardware.			\n\
Consult your printer hardware manual for the correct baud rate.		\n\
"};

char h_dn[] =
{"\n\
The 'dn' parameter specifies the name of the daemon program to		\n\
invoke each time a print request is made to the printer.  The		\n\
default daemon name is \"/usr/lib/lpd\", and should not be		\n\
changed.  The 'dn' parameter is available here so that the		\n\
system may support multiple line printer daemons.			\n\
"};

char h_du[] =
{"\n\
The 'du' parameter specifies the daemon UID used by the printer 	\n\
spooler programs.  The default value, (0) should not be changed.	\n\
"};

char h_fc[] =
{"\n\
The 'fc' parameter specifies which terminal flag bits to clear		\n\
when initializing the printer line.  Normally, all of the bits		\n\
should be cleared (fc=077777 octal) before calling 'fs'.  Refer		\n\
to the discussion of 'sg_flags' on the tty(4) manual page of the	\n\
ULTRIX-11 Programmer's Manual, Vol. 1.					\n\
"};

char h_ff[] =
{"\n\
The 'ff' parameter is the string to send as a form feed to the		\n\
printer.  The default value for this parameter is '\\f'.		\n\
"};

char h_fo[] =
{"\n\
The boolean parameter 'fo' specifies whether a form feed will		\n\
be printed when the device is first opened.  This is in addition	\n\
to the normal form feed which is printed by the driver when the		\n\
device is opened.  To suppress ALL printer induced form feeds,		\n\
use the 'sf' flag, in addition to the 'fo' flag.			\n\
"};

char h_fs[] =
{"\n\
The 'fs' parameter specifies which terminal flag bits to set		\n\
when initializing the printer line.  Normally, all of the bits		\n\
should be cleared (using fc=077777 octal) and then 'fs' should be	\n\
used to set the specified bits.  Refer to the discussion of		\n\
'sg_flags' on the tty(4) manual page of the ULTRIX-11 Programmer's	\n\
Manual, Volume 1.							\n\
"};

char h_lf[] =
{"\n\
The 'lf' parameter is the logfile where errors are reported.		\n\
The default logfile, if one is not specified, is \"/dev/console\".	\n\
If you have more than one printer on your system, you should give 	\n\
each logfile a unique name.		\n\
"};

char h_lo[] =
{"\n\
The 'lo' parameter is the name of the lock file used by the		\n\
printer daemon to control printing the jobs in each spooling		\n\
directory.  The default value, \"lock\", should not be changed.		\n\
"};

char h_lp[] =
{"\n\
The 'lp' parameter is the name of the special file to open for	\n\
output.  This is normally \"/dev/lp\" for LP11 parallel type	\n\
printers.  For serial printers, a terminal line (eg. /dev/tty00,\n\
/dev/tty01,...) is specified.			\n\
"};

char h_mx[] =
{"\n\
The 'mx' parameter specifies the maximum allowable filesize	\n\
(in BUFSIZ blocks) printable by each user.  Specifying mx=0	\n\
removes the filesize restriction entirely, and is equivalent	\n\
to not specifying 'mx' in /etc/printcap.			\n\
"};

char h_nc[] =
{"\n\
The boolean parameter 'nc' specifies that no control characters		\n\
are allowed in the output file.  Normally, control characters are	\n\
passed directly to the printer.						\n\
"};

char h_of[] =
{"\n\
The 'of' parameter specifies the output filter to be used with		\n\
the printer.  Some of the filters currently available include:		\n\
\n\
	Filter name:	  Description:					\n\
	------------	  ------------					\n\
	/usr/lib/ulf	  'universal' filter (LA50, LA100, others)	\n\
	/usr/lib/lqf	  letter quality filter (LA210, LQP02/03)	\n\
	/usr/lib/ln01of   LN01 filter (LN01 Laser Printer)		\n\
	/usr/lib/ln03of   LN03 filter (LN03 Laser Printer)		\n\
"};

char h_pl[] =
{"\n\
The 'pl' parameter specifies the page length in lines.  The 		\n\
default page length is 66 lines.					\n\
"};

char h_pw[] =
{"\n\
The 'pw' parameter specifies the page width in characters.  The		\n\
default page width is 132 characters, although a page width of		\n\
80 characters is more useful for letter quality printers, whose		\n\
standard paper size is 8 1/2\" x 11\".					\n\
"};

char h_rw[] =
{"\n\
The boolean parameter 'rw' specifies that the printer is to be		\n\
opened for both reading and writing.  Normally, the printer is		\n\
opened for writing only.						\n\
"};

char h_sd[] =
{"\n\
The 'sd' parameter specifies the spooling directory where files		\n\
are queued before they are printed.  Each spooling directory		\n\
should be unique.							\n\
"};

char h_sf[] =
{"\n\
The boolean parameter 'sf' suppresses all printer induced form	\n\
feeds, except those which are actually in the file.  The 'sf'	\n\
flag, in conjunction with 'sh', is useful when printing a letter\n\
on a single sheet of stationary.				\n\
"};

char h_sh[] =
{"\n\
The boolean parameter 'sh' suppresses printing of the normal	\n\
burst page header.  This often saves paper, in addition to being\n\
useful when printing a letter on a single sheet of stationary.	\n\
"};

char h_tr[] =
{"\n\
The 'tr' parameter specifies a trailing string to print when	\n\
the spooling queue empties.  It is generally a series of form	\n\
feeds, or sometimes an escape sequence, to reset the printer	\n\
to a known state.						\n\
"};

char h_xc[] =
{"\n\
The 'xc' parameter specifies the local mode bits to clear	\n\
when the terminal line is first opened.  Refer to the		\n\
discussion of the local mode word in the tty(4) manual		\n\
page in the ULTRIX-11 Programmer's Manual, Volume 1.		\n\
"};

char h_xs[] =
{"\n\
The 'xs' parameter specifies the local mode bits to set		\n\
when the terminal line is first opened.  Refer to the		\n\
discussion of the local mode word in the tty(4) manual		\n\
page in the ULTRIX-11 Programmer's Manual, Volume 1.		\n\
"};


/************************************************
*  This structure holds the correct printcap
*  values for various printers, other values
*  -> default.  The first member of the structure
*  holds the printer name.
*************************************************/
struct 
{
    char *name;
    struct nameval entry[80];
} printer[] = 
{
    /* LP11 */
    {
    	"lp11",
	"mx", "5000",
	"lp", "/dev/lp",
	0, 0
    },
    /* LA50 */
    {
    	"la50",
	"br", "4800",
	"fc", "077777",
	"fs", "016620",
	"sh", "none",
	"pw", "80",
	0, 0
    },
    /* LA100 */
    {
    	"la100",
	"br", "4800",
	"fc", "077777",
	"fs", "06020",
	0, 0
    },
    /* LA210 */
    {
    	"la210",
	"br", "9600",
	"fc", "077777",
	"fs", "020",
	"mx", "500",
	"of", "/usr/lib/lqf",
	"pw", "80",
	0, 0
    },
    /* LN01 */
    {
    	"ln01",
	"br", "9600",
	"fc", "077777",
	"fs", "023",
	"mx", "1000",
	"of", "/usr/lib/ln01of",
	"pw", "80",
	"lp", "/dev/rlp",
	0, 0
    },
    /* LN03 */
    {
    	"ln03",
	"br", "9600",
	"fc", "077777",
	"fs", "023",
	"mx", "500",
	"of", "/usr/lib/ln03of",
	"pw", "80",
	0, 0
    },
    /* LQP02/03 */
    {
    	"lqp",
	"br", "9600",
	"fc", "077777",
	"fs", "020",
	"of", "/usr/lib/lqf",
	"pw", "80",
	0, 0
    },
    /* NULL */
    {
	0, 0, 0
    }
};

/*************************************************
*  This structure maps a command character string
*  against an integral code
*************************************************/
struct cmdtyp   cmdtyp[] =
{
    "?", HELP,
    "add", ADD,
    "delete", DELETE,
    "exit", QUIT,
    "help", HELP,
    "list", LIST,
    "modify", MODIFY,
    "no", NO,
    "print", PRINT,
    "quit", QUIT,
    "yes", YES,
    0, 0
};

/******************************************
*  the following arrays hold the help text
*  associated with each function and are
*  referred to specifically by the function.
******************************************/
char h_help[] =
{"\n\
This program sets up your printers for you.  You will be	\n\
asked to enter the number of the printer (e.g. 0, 1) and	\n\
then some of the required fields for the printcap file.  	\n\
Later, optional fields or previous answers may be changed.	\n\
								\n\
For all symbols, a default value is given in \[ \].  To use	\n\
the default, just press <return>.  All of the possible symbols	\n\
for /etc/printcap are listed in printcap(5) in the ULTRIX-11	\n\
Programmer's Manual.  You can always enter '?' for help.	\n\
\n\
"};

char h_doadd[] =
{"\n\
Enter the number of the printer.  If you only have one printer		\n\
on your system, it should be number 0.  Printer number 0 is 		\n\
generally the default line printer.  The number you enter 		\n\
should be a single digit (e.g. '1') which is the number used 		\n\
to identify this printer.  For example, if the printer is known		\n\
as #1, then the command \"lpr -P1 files ...\" will print files		\n\
on this printer.							\n\
"};

char h_dodel[] =
{"\n\
Enter the number of the printer.  The number should be a		\n\
single digit (e.g. '0') which is the number used to identify		\n\
this printer.  The number of the printer that you enter must 		\n\
already exist in the /etc/printcap file, for you to delete it.		\n\
"};

char h_domod[] =
{"\n\
This section modifies an existing printcap entry.  You enter 		\n\
the number of the printer (e.g. '0') you wish to modify, and 		\n\
then you specify which field to change.  Enter:				\n\
        'q'     to quit (no more changes)				\n\
        'p'     to print the symbols you have specified so far		\n\
        'l'     to list all of the possible symbols and defaults	\n\
"};

char h_symsel[] =
{"\n\
Enter the name of the printcap field you wish to modify.  Enter		\n\
'p' to print existing values, or 'q' to quit (no more changes).		\n\
The available fields are:	\n\
\n\
"};

char h_synonym[] =
{"\n\
Enter an alternate name for this printer.  Some examples include	\n\
\"draft\",  \"letter\", and \"LA-180 DecWriter III\".  If the name	\n\
contains blanks or tabs it should be specified last.  You may enter	\n\
as many alternate names for a printer as you like, but the total	\n\
length of this line must be less than 80 characters.  If there are	\n\
no more synonyms, press <return> to continue.				\n\
"};

char h_type[] =
{"\n\
Enter one of the following, or press <return> for 'unknown':	\n\
\n\
"};

char h_default[] =
{"\n\
Enter a new value, or press <return> to use the default. \n\
"};

char isnotused[] =
{"feature is not used with 				\n\
the LP11 parallel line printer interface.  You would only\n\
specify this symbol for a printer which is connected via\n\
a serial terminal line. 				\n\
"};

/***********************************
*  end of globals.h
***********************************/
