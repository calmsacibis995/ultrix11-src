static char Sccsid[] = "@(#)setup_help.c	3.1	8/6/87";

/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * TODO:
 *
 * [n]	Many help messages need much work, unfortunatley by only one person.
 */
/*
 *  File name:
 *
 *	setup_osl.c
 *
 *  Source file description:
 *
 *	Called, via fork/exec, from setup to print help messages. The help
 *	message name string is passed to setup_help in an argument. Only
 *	exists because setup got too big for a 0407 type file.
 *
 *  Functions:
 *
 *	main()		The planes in spain fall mainly in the rain, and
 *			in this file, they are all alone!
 *
 *  Usage:
 *
 *	cd /sas; setup_help name
 *
 *		name - help message name string.
 *
 *	Called by setup only, not intended for users.
 *
 *
 *  Compile:
 *
 *	cd /usr/sys/distr; make setup_help
 *
 *  Modification history:
 *
 *	13 May 1985
 *		File created (V2.0) -- Fred Canter
 */

#include <stdio.h>

struct	suhelp {
	char	*suh_name;	/* name of the help message */
	char	**suh_addr;	/* address of help text */
};


char	*sum2[] =
{
	"",
	"This program performs operating system setup functions during",
	"installation and normal system operation. Setup operates in one",
	"of three possible modes (phases), depending on the current state",
	"of the system. The three modes are:",
	"",
	" Phase 1: Initial setup -- prepares system for first sysgen.",
	" Phase 2: Final setup -- completes the system setup.",
	" Phase 3: Change setup -- handles system setup changes.",
	"",
	"The program will ask several setup questions. Enter your answer",
	"to each question, using lowercase characters, then press <RETURN>.",
	"",
	"The questions include helpful hints enclosed in angle brackets < >",
	"and/or parenthesis ( ). If you need additional help answering any",
	"question, enter a ? or the word help then press <RETURN>.",
	"",
	"You can correct typing mistakes by pressing the <DELETE> key to",
	"erase a single character or <CTRL/U> to erase the entire line.",
	"You can interrupt the setup program by typing <CTRL/C>. This",
	"allows you to abort the setup process or restart it.",
	0
};

char	*ntp_warn[] =
{
	"",
	"NOT INSTALLING ON THE TARGET PROCESSOR!",
	"",
	"If the target CPU does not have a distribution load device,",
	"you must load any needed items of optional software now.",
	"",
	0
};

char	*dsk_mntd[] =
{
	"Prepare for user file system setup as follows:",
	"",
	"   1.	Make sure all disk drives, on which user file systems will",
	"	be created, are on-line and ready.",
	"",
	"   2.	Disk media must be formatted and checked before setting up",
	"	user file systems. Refer to Appendix C in the ULTRIX-11",
	"	Installation Guide for media qualification instructions.",
	"",
	"   3.	You may want to write protect or take off-line any disks,",
	"	other than the system disk(s), where you don't want user",
	"	file systems created (such as data base disks).",
	"",
	"   4.	Plan your user file system layout before implementing it!",
	"	If you are not prepared, continue with the following steps",
	"	but don't create any user file systems. This will inform",
	"	you about available disk partitions. You can create user",
	"	file systems later by invoking setup phase three.",
	0
};

char	*ufs_fe1[] =
{
	"If you make a new file system on this partition, the contents of",
	"the existing file system will be destroyed. You can use the rawfs(8)",
	"command to list and extract files from the existing file system.",
	0
};

char	*ufs_fe2[] =
{
	"You have the following options:",
	"",
	"   1.	Overwrite the existing file system and create a new user",
	"	file system in its place.",
	"",
	"   2.	Preserve the existing file system and use it as one of your",
	"	new user file systems.",
	"",
	"   3.	Not use the existing file system, but preserve it anyway.",
	"	Perhaps it contains some files you need to save.",
	0
};

char	*ufs_warn[] =
{
	"		     ****** CAUTION ******",
	"",
	"Partitions marked available may already contain a user file system.",
	"The setup program will warn you if a user file system exists on the",
	"partition you select. You must determine how to proceed.",
	0
};

char	*usrinfo[] =
{
	"",
	"The initial setup program cannot obtain the information it needs",
	"to complete the installation from the setup.info file. The sdload",
	"program writes this information into the setup.info file after",
	"loading the software onto the system disk. For some unknown reason",
	"the setup program cannot access the information.",
	"",
	"You can supply the missing information or abort the installation.",
	"If you continue, the program will prompt you for the following:",
	"",
	"    o  Setup phase number",
	"",
	"    o  System disk type",
	"",
	"    o  Target processor type",
	"",
	"    o  Software load device type",
	"",
	0
};

char	*msf_rec[] =
{
	"DIGITAL recommends you add the following information to the",
	"configuration work sheet located in the Installation Guide.",
	"This work sheet will be helpful if you need to remake your",
	"device special files using setup phase 3.",
	"",
	"The program will pause periodically to allow time for you",
	"to record the configuration information.",
	0
};

char	*no_disks[] =
{
	"",
	"Sorry, cannot setup any user file systems. The system disk(s) have",
	"no free partitions and there are no other disk drives available.",
	"",
	"If there are additional (non system) disk drives, check the kernel",
	"configuration file. Maybe you forgot to configure them in the kernel.",
	"Also, make sure the disk drive is on-line.",
	"",
	"User files can be stored in the root and /usr file systems, but this",
	"is not recommended, and there is generally not much free space in",
	"root and /usr.",
	"",
	0
};

char	*h_pn[] =
{
	"",
	"The setup program runs in one of three possible phases. Normally",
	"the setup phase is determined by reading the setup.info file.",
	"For some unknown reason the setup phase cannot be read from the",
	"setup.info file. Select the appropriate setup phase, from the ",
	"following list, enter its number, then press <RETURN>.",
	"",
	"Phase 1:",
	"    This is the initial setup phase. The software has transferred",
	"    from the distribution kit onto the system disk, but the system",
	"    generation and installation of the new operating system kernel",
	"    has not been completed.",
	"",
	"Phase 2:",
	"    This phase is entered after booting the new operating system.",
	"    Phase 2 completes the installation of the software. Phase 2",
	"    should be selected only if phase 1 and the system generation",
	"    have been completed.",
	"",
	"Phase 3:",
	"    Phase 3 is not used during the software installation. Phase 3",
	"    is used when something in the system changes, such as: a new",
	"    hardware device is added, the system is moved another timezone,",
	"    or the hostname needs to be changed, etc.",
	0
};

char	*h_hz[] =
{
	"Enter the AC power line frequency for your local area then press",
	"<RETURN>. The line frequency is expressed in hertz, also known as",
	"cycles per second.",
	"",
	"The AC power line frequency for the United States of America and",
	"Canada is 60 hertz. Australia, England, and Europe use 50 hertz AC",
	"power. Japan uses 50 hertz in some areas and 60 hertz in others.",
	"If you are unsure about your AC power line frequency, you should",
	"constact your local power company.",
	"",
	"Although 50 and 60 hertz are the standard line frequencies, you",
	"can enter any value, suggested range of 45 - 65 hertz. This allows",
	"you to compensate for AC line frequency variations that sometimes",
	"occur in local areas. CAUTION, if you enter an incorrect AC line",
	"frequency value, your computer system will not keep accurate time.",
	"",
	0
};

/* TODO: message needs more work (maybe - hours from GMT) */
char	*h_tz[] =
{
	"Enter the time zone for your local area. The time zone is given",
	"as the number of hours west (or behind) GMT (Greenwich mean time).",
	"Use standard time, do not include daylight savings time in the",
	"time zone specification. Some of the possible time zones are:",
	"",
	"	00  GMT		12",
	"	01		13",
	"	02		14",
	"	03		15",
	"	04		16",
	"	05  EST		17",
	"	06  CST		18",
	"	07  MST		19",
	"	08  PST		20",
	"	09		21",
	"	10		22",
	"	11		23",
	"",
	0
};

char	*h_dst[] =
{
	"If your local area uses daylight savings time, enter yes, if not,",
	"enter no, then press <RETURN>. If you enter yes, your ULTRIX-11",
	"system will automatically change to daylight savings time and back",
	"to standard time on the appropriate dates for your time zone.",
	"",
	0
};

char	*h_dstarea[] =
{
	"",
	"Choose the geographic area of your country from the following list.The",
	"operating system will automatically adjust the time for daylight saving",
	"time for your country.",
	"",
	"USA:		 United States of America",
	"Australia:	 Australia",
	"Western Europe:  England,Ireland,Portugal",
	"Central Europe:  Belgium,Luxembourg,Netherlands, Denmark, Norway,",
	"		 Austria, Poland, Czechoslovakia, Sweden, Switzerland,",
	"		 DDR, DBR, France, Spain, Hungary, Italy, Jugoslavia",
	"Eastern Europe:  Bulgaria, Finland, Greece, Rumania, Turkey, Western Russia",
	0
};
char	*h_rmsf[] =
{
	"",
	"If your system's hardware configuration has changed, you need",
	"to remake some of the device special files. Compare the current",
	"configuration information with the configuration recorded in",
	"appendix F of the installation guide.",
	"",
	"If they differ, answer yes to remake the device special files,",
	"otherwise, answer no to use the current special files.",
	"",
	0
};

char	*h_ttys[] =
{
	"",
	"If your system's communications device configuration has changed",
	"you need to remake the /etc/ttys and /etc/ttytype files. This is",
	"necessary to ensure these files match the system's current terminal",
	"configuration. In addition, the /etc/ttys file must be recreated",
	"if the number of pseudo TTYs configured into the kernel changes.",
	"",
	"CAUTION:",
	"",
	"	The ttys and ttytype files are recreated from scratch.",
	"	Data in the current files will be overwritten, however",
	"	the current files will be saved for your reference.",
	"",
	"Answer yes to remake the ttys and ttytype files. If you answer no",
	"these files will not be recreated. If the ttys and ttytype files",
	"are not recreated, you must edit them to make sure they correctly",
	"specify your system's current terminal configuration. Failure to",
	"update these files will result in improper operation of at least",
	"some of your terminals.",
	"",
	0
};

char	*h_osl[] =
{
	"",
	"Some of the ULTRIX-11 software is optional, that is, it is not",
	"automatically loaded from the distribution kit during installation.",
	"This is necessary due to space limitations on the smaller disks.",
	"Setup allows you to optimize disk usage by loading only the items",
	"of optional software you intend to use. You can also unload items",
	"of optional software that are no longer used.",
	"",
	"How much optional software can be loaded depends of the system disk",
	"size. The smaller disks cannot hold all of the optional software.",
	"The moderate sized disks can hold most or all the optional software,",
	"but user file space may be severly limited if all optional software",
	"is loaded. The larger disks have enough space to hold all optional",
	"software. You need to balance optional software loding against the",
	"available space on your system disk.",
	"",
	"Answer no if you do not want to load/unload optional software. If",
	"you are installing on the targer processor, you can run the setup",
	"program and load/unload optional software at any time. However, if",
	"you are not installing on the target processor you must load any",
	"optional software you need before moving to the target processor.",
	"Answer yes to load/unload optional software or to list the items of",
	"optional software you can load.",
	"",
	0
};

char	*h_cufs[] =
{
	"Before storing user's files on a disk you need to set up a user",
	"file system on that disk, create its entry in the file system table",
	"(/etc/fstab), and create a directory for mounting the file system.",
	"",
	"The setup program can create the user file systems or you can set",
	"them up yourself by following the procedure in chapter 4 of the",
	"ULTRIX-11 System Management Guide.",
	"",
	"Answer yes to set up the user file systems now. Answer no if you",
	"want to postpone setting up user file systems. If you answer no,",
	"you can create or change the user file systems by running the setup",
	"program or using the manual procedure at any time after completing",
	"setup phase two.",
	"",
	0
};

char	*h_udisk[] =
{
	"You have a disk available for user file storage. In this case,",
	"disk means a free partition on the system disk or the entire disk",
	"for any winchester disk other than unit zero (system disk).",
	"",
	"Answer yes if you want this disk to be allocated for storage of",
	"your user's files. You should answer yes, even if the disk already",
	"contains a user file system. This allows you to reconstruct your",
	"system disk, by reloading the software from the distribution kit,",
	"without destroying your user's files.",
	"",
	"Answer no if the disk is not to be used for user file storage.",
	"Possibly, you have an application, such as a data base manager,",
	"that accesses the disk directly instead of through the file system.",
	"",
	"You need not make this decision now. You can allocate a disk for",
	"user file storage at any time by invoking setup phase three or",
	"following the instructions in chapter 4 of the ULTRIX-11 System",
	"Management Guide.",
	"",
	0
};

char	*h_mkfs[] =
{
	"An empty ULTRIX-11 file system must be created on a disk before",
	"it can be used to store user's files. Also, each user file system",
	"requires an entry in the file system table (/etc/fstab) and a",
	"directory on which the file system can be mounted.",
	"",
	"Answering yes to the previous question causes the directory and",
	"the file system table entry to be created automatically. However,",
	"you must decide whether or not an empty file system should be",
	"created. This is necessary because the disk may already contain",
	"a file system and user's files.",
	"",
	"Answer no if the disk already contains a user file system. You",
	"would answer no if you were re-installing the ULTRIX-11 software",
	"but wanted to preserve the files on a user disk.",
	"",
	"Answer yes if the disk needs to be intiialized with an empty file",
	"system. You would answer yes if you are installing the ULTRIX-11",
	"software for the first time.",
	"",
	0
};

char	*h_sdp[] =
{
	"",
	"Please enter the number of the disk partition you want to use for",
	"setting up the user file system. Select from the list of available",
	"partitions enclosed in angle brackets < # # # >. For additional",
	"help with partition selection, refer to appendix D in the System",
	"Management Guide. If you have no more user file systems to set up",
	"on this disk, enter a period to indicate you are finished with",
	"this disk.",
	"",
	0
};

char	*h_hostn[] =
{
	"",
	"Your system needs a hostname. The hostname appears in the login",
	"prompt and, optionally, in the shell command prompt. If your",
	"system is connected to a computer network, the hostname uniquely",
	"identifies your system to the other computers on the network.",
	"",
	"Enter the hostname then press <RETURN>. The name you choose must",
	"be made up of no more than 16 lowercase alphanumeric characters.",
	"The hostname may include a dash, but no other special characters.",
	"For example: mypdp11, system1, my-system.",
	"",
	"If your system is connecting to a uucp network, then the first",
	"six characters of your hostname must be different from any other",
	"site name on the network. To verify if your name is unique, ask",
	"the site through which you are connecting for a list of existing",
	"site names.",
	"",
	"If you do not wish to select a hostname now, enter \"noname\" and",
	"press <RETURN>. You can change the hostname later, by editing the",
	"last line of the /etc/rc file or running the setup program.",
	"",
	0
};

/* TODO: 11/53 will change this message */
char	*h_rxldun[] =
{
	"Please enter the unit number of the RX50 floppy disk drive used",
	"to load the ROOT and USR diskettes. This should always be the",
	"RX50 with the lower unit number.",
	"",
	"For Micro/PDP-11 systems with multiple RX50 units, you would use:",
	"",
	"VERTICALLY MOUNTED SYSTEMS:",
	"",
	"    Left drive: UNIT 1 (UNIT 2 if second RD DISK present)",
	"",
	"HORIZONTALLY MOUNTED SYSTEMS:",
	"",
	"    Top  drive: UNIT 1 (UNIT 2 if second RD DISK present)",
	"",
	0
};

char	*h_mtldct[] =
{
	"",
	"Please enter the ULTRIX-11 mnemonic (2 character device name) for",
	"your type of magtape controller, then press <RETURN>. The magtape",
	"mnemonics are:",
	"",
	"	tm = TM11-TU10/TE10",
	"	ht = TM02/3-TU16/TE16/TU77",
	"	ts = TS11/TU80/TSV05/TSU05",
	"",
	0
};

/* TODO: message not used */
char	*q_sdt[] =
{
	"",
	"Please enter the generic name of your system disk then press the",
	"<RETURN> key. Select from the list of supported disks enclosed in",
	"angle brackets < >.",
	"",
	0
};

char	*h_otp[] =
{
	"",
	"The target processor is the CPU on which the ULTRIX-11 software",
	"will actually be used. You normally install the software on the",
	"target processor, however, the target CPU may not have one of the",
	"supported distribution load devices. In that case, the software is",
	"installed on the current CPU and moved to the target CPU later.",
	"",
	"Answer yes if you are installing on the target CPU or no if the",
	"current CPU is not the target processor.",
	"",
	0
};

/* TODO: message not used */
char	*q_cpt[] =
{
	"",
	"TODO: needs rewrite or remove",
	"This version setup only supports the Micro/PDP-11/53 processor.",
	"If the current processor is not a Micro/PDP-11/53, answer no to",
	"abort the installation.",
	"",
	0
};

char	*h_date[] =
{
	"Please enter the current date/time using the following format:",
	"",
	"        yymmddhhmm.ss",
	"",
	"Where:",
	"        yy equals the last two digits of the year",
	"        mm equals the number of the month of the year",
	"        dd equals the number of the day of the month",
	"        dd equals the hour of the day (based on 24 hours)",
	"        mm equals the minutes of the hour",
	"        ss equals the seconds of the minute (optional)",
	"",
	"To set the date/time to 30 seconds past 6:30 PM on January 27,",
	"1985, for example, you would enter:",
	"",
	"        8501271830.30",
	"",
	0
};

/* TODO: message not used */
char	*stophere[] =
{
	"THIS IS NOT THE TARGET PROCESSOR",
	"",
	"The installation must be completed on the target processor,",
	"that is, the processor on which the ULTRIX-11 software will",
	"actually be used.",
	"",
	"    o  Halt the current processor.",
	"",
	"    o  Move the system disk pack(s) to the target processor.",
	"",
	"    o  Boot the system disk using your hardware bootstrap.",
	"",
	"    o  Continue the installation on the target processor.",
	"",
	0
};

char	*h_crt[] =
{
	"Your computer system may be equipped with either of two console",
	"terminal types:",
	"",
	"    A CRT (video terminal) such as the VT100 or VT200 series.",
	"",
	"    A hardcopy terminal such as the Decwriter series.",
	"",
	"To assure proper erase (<DELETE>) and kill (<CTRL/U>) handling,",
	"you need to identify which type of console terminal you have.",
	"",
	"If you have a CRT answer y for yes, otherwise answer n for no,",
	"then press <RETURN>.",
	"",
	0
};

char	*h_csf[] =
{
	"",
	"Device special files are used by the operating system and many",
	"utility programs to access devices, such as disks and magtapes.",
	"",
	"You need to remake the device special files, when:",
	"",
	" 1. A new device is added to the system or a device is removed.",
	"",
	" 2. Some special files were mistakenly deleted.",
	"",
	" 3. Communication device configuration changes.",
	"",
	" 4. The number of pseudo TTYs configured into the kernel changes.",
	"",
	" 5. The number of MAUS map enteries in the kernel changed.",
	"",
	"The system must be in single-user mode when creating special files.",
	"",
	0
};

char	*h_lpr[] =
{
	"",
	"You need to set up the line printer spooler (LPR) if you have an",
	"LP11 type line printer or printer ports. A printer port is defined",
	"as an asynchronous communications port use to drive a printer.",
	"",
	"The LPR spooler setup script will guide you thru setting up printer",
	"ports. You can set up printers now or wait until a later time.",
	"",
	0
};

char	*h_ussl[] =
{
	"",
	"You can increase the space available for \"spooling\" files by",
	"moving the spooling directory to another file system (possibly",
	"on another disk drive) and creating a symbolic link from the new",
	"directory to /usr/spool.",
	"",
	"You should make a symbolic link for the /usr/spool directory, if:",
	"your system disk is one of the smaller disks (RL01, RL02, RD52, RK06,",
	"or RP02) and you expect any significant amount of spooling activity,",
	"or you have one of the larger disks and you expect high volumes of",
	"spooling files. Spooling activity is generated by: mail, uucp, and",
	"the line printer spooler.",
	"",
	0
};

char	*h_slbdn[] =
{
	"",
	"When making a symbolic link for /usr/spool, you need to supply",
	"a base directory name. This is the directory on which the file",
	"system where the actual spool directory and files will reside.",
	"",
	"The file system and base directory must exist. They are normally",
	"created during the \"setup user file systems\" step in phase 2",
	"of the setup program.",
	"",
	"For example, if a file system exists on RL02 unit 1 and is mounted",
	"on the /user1 directory, then /user1 would be the base directory",
	"for creation of the symbolic link from /usr/spool.",
	"",
	0
};

char	*h_gtt[] =
{
	"",
	"Please enter the type of terminal connected to the indicated port,",
	"that is, console or tty## (for example, vt100, vt52, la120 ...).",
	"A default entry is displayed in angle brackets < >. Press <RETURN>",
	"to use the default terminal type. The name \"dw3\" is sometimes",
	"used in place of DECwriter III or LA120.",
	"",
	"The terminal type will be entered into the /etc/ttytype file,",
	"which tells the system what type of terminal is connected to each",
	"communications port. You should enter only a terminal type that",
	"has an entry in the terminal capabilities data base (/etc/termcap).",
	"",
	"If you do not know the terminal type or there is not a terminal",
	"connected to the communications port, use the default or vt100.",
	"You can change the terminal type by editing the /etc/ttytype file.",
	"Refer to Chapter 4 of the ULTRIX-11 System Management Guide for",
	"instructions.",
	"",
	0
};

char	*h_gta[] =
{
	"",
	"The /etc/ttytype file tells the system what type of terminal is",
	"connected to each communications port. You can specify terminal",
	"types now or at some later time.",
	"",
	"If you answer \"yes\", all terminal type entries will be set to",
	"vt100. You can change a terminal type entry, at any time, by",
	"editing the /etc/ttytype file. Refer to Chapter 4 in the ULTRIX-11",
	"System Management Guide for instructions. You can also invoke",
	"setup phase 3 to change terminal type entries.",
	"",
	"If you answer \"no\", you will be asked to specify the type of each",
	"terminal on your system. A default terminal type will be displayed",
	"in angle brackets < >, press <RETURN> to use the default. If you",
	"do not know the terminal type or there is no terminal connected",
	"to a communications port, take the default or specify vt100.",
	"Remember, you can change the terminal type at any time.",
	"",
	0
};

/*
 * Start of messages from setup_osl.
 */

char	*h_help[] =
{
	"To obtain help, type `help subject' then press <RETURN>.",
	"",
	"Help is available for the following subjects:",
	"",
	"Subject\t\tDescription",
	"-------\t\t-----------",
	"help\t\tPrint this general help message.",
	"usage\t\tGives general program usage information.",
	"commands\tLists the available commands.",
	"abort\t\tDescribes the <CTRL/C> command abort function.",
	"exit\t\tDescribes the EXIT command.",
	"free\t\tDescribes the FREE command.",
	"rxunit\t\tDescribes the RXUNIT command.",
	"rxdir\t\tDescribes the RXDIR command.",
	"list\t\tDescribes the LIST command.",
	"load\t\tDescribes the LOAD command.",
	"unload\t\tDescribes the UNLOAD command.",
	"",
	"To print the general usage instructions you would enter:",
	"",
	"    help usage",
	0
};

char	*h_usage[] =
{
	"This program loads items of optional software from the distribution",
	"media onto the system disk. It can also unload optional software.",
	"DIGITAL recommends using this program in single-user mode, however,",
	"you can use it when the system is up multiuser. In that case, you",
	"must be super-user and prevent users from attempting to access the",
	"items of optional software you are loading/unloading.",
	"",
	"You enter all input using only lowercase characters. A list of items",
	"enclosed in < > indicates you should choose one item from the list.",
	"Command arguments enclosed in [ ] are optional.",
	"",
	"The program prompts for commands as follows:",
	"",
	"    Command <help free rxunit rxdir list load unload exit>:",
	"",
	"To execute a command, enter its name then press <RETURN>. Command",
	"names may be abbreviated (enough letters to make the name unique).",
	"To get help, for example, you would enter:",
	"",
	"    Command < ... >: help <RETURN>",
	"    Command < ... >: h <RETURN>",
	0
};

char	*h_cmds[] =
{
	"You can execute the following commands, note that some commands",
	"may be invoked with multiple names:",
	"",
	"Command\t\tDescription",
	"-------\t\t-----------",
	"<CTRL/C>\tImmediate abort of any command in progress.",
	"help subject\tPrints the help text for `subject'.",
	"?\t\tSame as help.",
	"exit\t\tExit from the program.",
	"bye\t\tSame as exit.",
	"quit\t\tSame as exit.",
	"free\t\tGive amount of free disk space remaining.",
	"rxunit\t\tOverride the default RX50 unit number.",
	"rxdir\t\tList RX50 diskette names and contents.",
	"list\t\tDisplay list of optional software items.",
	"load\t\tLoad item(s) of optional software.",
	"unload\t\tRemove item(s) of optional software.",
	0
};

char	*h_exit[] =
{
	"Command < ... >: exit",
	"Command < ... >: bye",
	"Command < ... >: quit",
	"",
	"This command causes the program to terminate. Control returns to",
	"the setup program or the shell prompt (#), depending on how the",
	"program was invoked. This program can be called from the setup",
	"program or as the osload command.",
	"",
	"The commands `bye' and `quit' are synonymous with `exit'.",
	0
};

char	*h_free[] =
{
	"Command < ... >: free",
	"",
	"This command displays the amount of free space in each of your file",
	"systems, by executing the df(1) command. The free command is used",
	"to determine if there is enough free disk space to load an item of",
	"optional software. The list command shows the size of each item.",
	0
};

char	*h_rxunit[] =
{
	"Command < ... >: rxunit",
	"",
	"By default, unit two is used when loading optional software from",
	"RX50 diskettes. This command allows you to override the default and",
	"specify the RX50 unit number.",
	0
};

char	*h_rx_un[] =
{
	"Please enter the unit number of the RX50 floppy disk drive to be",
	"used for loading optional software. The available unit numbers are",
	"configuration dependent. For the Micro/PDP-11, they are:",
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
	0
};

char	*h_rxdir[] =
{
	"Command < ... >: rxdir",
	"",
	"This command lists the name of each diskette in the RX50 kit and",
	"the contents of each diskette. This command, in conjunction with the",
	"list command, helps you locate optional software items in the RX50",
	"distribution kit,",
	0
};

char	*h_list[] =
{
	"Command < ... >: list [name1 name2 ... nameN]",
	"",
	"This command lists the items of optional software, including:",
	"",
	"  o  The name of the item.",
	"",
	"  o  Disk space required in Kbytes.",
	"",
	"  o  If the item is currently loaded or not.",
	"",
	"  o  File system where item resides (ROOT or /USR).",
	"",
	"  o  A description of the optional software item.",
	"",
	"You can list all items or only selected items, for example:",
	"",
	"    Command < ... >: list (list all items)",
	"",
	"    Command < ... >: list f77 spell (list selected items)",
	0
};

char	*h_load[] = 
{
	"Command < ... >: load [name1 name2 ... nameN]",
	"",
	"This command loads an item, or items, of optional software from",
	"the distribution kit onto the system disk. You can enter a list of",
	"items on the command line with the load command or just type load.",
	"If no item names are given, the load command will prompt you for",
	"the list for items",
	"",
	"To load usep, f77, spell, learn, for example, you would enter:",
	"",
	"    Command < ... >: load usep f77 spell learn",
	"",
	"If no items were given, the above command would be:",
	"",
	"    Command < ... >: load",
	"",
	"    List: usep, f77, spell, learn <RETURN>",
	"",
	"In place of the list of items, you can enter the word `all' to",
	"specify loading of all items not currently loaded.",
	0
};

char	*h_unload[] = 
{
	"Command < ... >: unload [name1 name2 ... nameN]",
	"",
	"This command unloads an item, or items, of optional software from",
	"the system disk. You can enter a list of items or just unload.",
	"If no item names are given, the unload command will prompt you for",
	"the list for items",
	"",
	"To unload usep, f77, spell, learn, for example, you would enter:",
	"",
	"    Command < ... >: unload usep f77 spell learn",
	"",
	"If no items were given, the above command would be:",
	"",
	"    Command < ... >: unload",
	"",
	"    List: usep, f77, spell, learn <RETURN>",
	"",
	"In place of the list of items, you can enter the word `all' to",
	"specify unloading of all items (unconditionally).",
	0
};

char	*h_ctrlc[] =
{
	"Typing <CTRL/c> will terminate what ever command is in progress",
	"and return you to the `Command < ... >:' prompt. You generate the",
	"<CTRL/c> combination by holding down the CTRL key and pressing c.",
	0
};

char	*h_dtden[] =
{
	"The distribution tape density will be either 800 BPI or 1600 BPI.",
	"If you are not sure of the tape density, check the second line of",
	"the magtape label. The second line will end with \"8mt9\" for an 800",
	"BPI tape or \"16mt9\" for a 1600 BPI tape.",
	"",
	"Enter the density, either 800 or 1600, then press <RETURN>.",
	0
};

char	*h_glist[] =
{
	"Enter the list of optional software items, as follows:",
	"",
	"	item1 item2 item3 ... itemN",
	"",
	"Terminate the list by pressing <RETURN>, for example:",
	"",
	"	saprog usep usat f77 <RETURN>",
	0
};

char	*h_rsl[] =
{
	"You use symbolic links if your /usr file system does not have",
	"enough free space to hold the optional software you want to load.",
	"Symbolic links allow loading of optional software onto secondary",
	"disk drives when the system disk is one of the smaller disks, such",
	"as, RL01, RL02, RD51, RK06, or RP02.",
	"",
	"Without symbolic links, optional software files are loaded into",
	"their target directories in the /usr file system. For example,",
	"the manuals files are loaded into the /usr/man directory.",
	"",
	"With symbolic links, the files can be loaded into another file",
	"system, but appear to be loaded in the /usr file system. For",
	"example, a symbolic link from /user1/man to /usr/man would cause",
	"the manuals files to be loaded into /user1/man, but appear to",
	"exist in their normal directory (/usr/man).",
	"",
	"Use of symbolic links requires the secondary file to exist and be",
	"mounted.",
	0
};

char	*h_rsld[] =
{
	"When loading optional software files using symbolic links, you",
	"need to supply a base directory name. This is the directory on",
	"which the file system where the files are to be loaded is mounted.",
	"",
	"The file system and base directory must exist. They are normally",
	"created during the \"setup user file systems\" step in phase 2",
	"of the setup program.",
	"",
	"For example, if a file system exists on RL02 unit 1 and is mounted",
	"on the /user1 directory, then /user1 would be the base directory",
	"for loading optional software files with symbolic links.",
	0
};

char	*h_suf[] =
{
	"Escaping to the shell (so you can preserve your files).",
	"",
	"CAUTION: do not copy files into the same directory where they",
	"         currently reside (or unload will remove them).",
	"",
	"Type <CTRL/D> to return from the shell to the osload program.",
	0
};

char	*sum1[] =
{
	"",
	"\t    ****** ULTRIX-11 Setup Program ******",
	0
};

char	*cmp_info[] =
{
	"Compare the above information to the configuration",
	"work sheet in the ULTRIX-11 Installation Guide.",
	"",
	0
};

char	*mu_warn[] =
{
	"",
	"The /usr file system appears to be mounted, indicating the system",
	"may be operating in multiuser mode. Setup phase 3 MUST only be",
	"entered with the system in single-user mode.",
	"",
	"DO NOT CONTINUE IF THE SYSTEM IS IN MULTIUSER MODE!",
	0
};

/* TODO: should print CPUs from cputyp[].p_type */
char	*tcpu[] = 
{
	"",
	"Please enter the processor type of the target CPU.",
	"",
	"Select one of the following supported processor types.",
	"(Use 23, 53, 73, or 83 for the Micro/PDP-11):",
	"",
	"   23, 24, 34, 40, 44, 45, 53, 55, 60, 70, 73, 83, 84",
	"",
	0
};

char	*abort[] =
{
	"",
	"****** INITIAL SETUP ABORTED ******",
	"",
	"You have the following options:",
	"",
	"    o  Restart the installation procedure at the beginning.",
	"",
	"    o  Execute the following steps to retry the initial setup:",
	"",
	"           Halt the processor.",
	"           Execute the hardware bootstrap for the system disk.",
	"           The setup program should restart automatically, if",
	"           it does not, execute: cd /.setup; setup.",
	"",
	"    o  Contact the Telephone Support Center or your local DIGITAL",
	"       software services office for assistance.",
	"",
	"",
	0
};

/* TODO: check usage */
char	*info_err[] =
{
	"You may supply the load device, system disk, and target pro-",
	"cessor types and continue, or abort the installation procedure.",
	"",
	0
};

char	*eop1_otp[] =
{
	"You can now run the sysgen program and generate a new ULTRIX-11",
	"kernel to match your system's hardware configuration. Return to",
	"the Installation Guide for instructions.",
	"",
	"",
	0
};

char	*eop1_ntp[] =
{
	"THIS IS NOT THE TARGET PROCESSOR!",
	"",
	"Halt the processor, move the system disk to the target CPU, and",
	"reboot the operating system. The system will enter setup phase 1",
	"and complete phase 1 on the target processor.",
	"",
	"",
	0
};

char	*eop2[] =
{
	"The automated portion of the ULTRIX-11 software installation is",
	"now complete. Return to the Installation Guide for instructions.",
	"",
	"",
	0
};

char	*eop3[] =
{
	"Setup changes complete. Use fsck to check your file systems, then",
	"type <CTRL/D> to enter multiuser mode.",
	"",
	"",
	0
};


struct	suhelp	suhelp[] = {
	"sum2",		&sum2,
	"ntp_warn",	&ntp_warn,
	"dsk_mntd",	&dsk_mntd,
	"ufs_fe1",	&ufs_fe1,
	"ufs_fe2",	&ufs_fe2,
	"ufs_warn",	&ufs_warn,
	"usrinfo",	&usrinfo,
	"msf_rec",	&msf_rec,
	"no_disks",	&no_disks,
	"h_pn",		&h_pn,
	"h_hz",		&h_hz,
	"h_tz",		&h_tz,
	"h_dst",	&h_dst,
	"h_dstarea",	&h_dstarea,
	"h_rmsf",	&h_rmsf,
	"h_ttys",	&h_ttys,
	"h_osl",	&h_osl,
	"h_cufs",	&h_cufs,
	"h_udisk",	&h_udisk,
	"h_mkfs",	&h_mkfs,
	"h_sdp",	&h_sdp,
	"h_hostn",	&h_hostn,
	"h_rxldun",	&h_rxldun,
	"h_mtldct",	&h_mtldct,
	"h_otp",	&h_otp,
	"h_date",	&h_date,
	"h_crt",	&h_crt,
	"h_csf",	&h_csf,
	"h_lpr",	&h_lpr,
	"h_ussl",	&h_ussl,
	"h_slbdn",	&h_slbdn,
	"h_gtt",	&h_gtt,
	"h_gta",	&h_gta,
	"h_help",	&h_help,	/* Start of setup_osl messages */
	"h_usage",	&h_usage,
	"h_cmds",	&h_cmds,
	"h_exit",	&h_exit,
	"h_free",	&h_free,
	"h_rxunit",	&h_rxunit,
	"h_rx_un",	&h_rx_un,
	"h_rxdir",	&h_rxdir,
	"h_list",	&h_list,
	"h_load",	&h_load,
	"h_unload",	&h_unload,
	"h_ctrlc",	&h_ctrlc,
	"h_dtden",	&h_dtden,
	"h_glist",	&h_glist,
	"h_rsl",	&h_rsl,
	"h_rsld",	&h_rsld,
	"h_suf",	&h_suf,
	"sum1",		&sum1,		/* All the entries below this moved from
					 * setup.c to fit within space limit */
	"cmp_info",	&cmp_info,
	"mu_warn",	&mu_warn,
	"tcpu",		&tcpu,
	"abort",	&abort,
	"info_err",	&info_err,
	"eop1_otp",	&eop1_otp,
	"eop1_ntp",	&eop1_ntp,
	"eop2",		&eop2,
	"eop3",		&eop3,
	0
};

main(argc, argv)
int argc;
char *argv[];
{
	register int i;
	register char **p;

	if(argc != 2) {
		printf("\nsetup_help: help message name missing!\n");
		exit(1);
	}
	for(i=0; suhelp[i].suh_name; i++)
		if(strcmp(argv[1], suhelp[i].suh_name) == 0)
			break;
	if(suhelp[i].suh_name == 0) {
		printf("\nsetup_help: Can't find `%s' help text!\n", argv[1]);
		exit(1);
	}
	p = suhelp[i].suh_addr;
	for(i=0; p[i]; i++)
		if(p[i] == -1) {
			printf("\n\nPress <RETURN> for more:");
			while(getchar() != '\n') ;
		} else
			printf("\n%s", p[i]);
	printf("\n");
	fflush(stdout);
}
