
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)sg_help.c	3.1	3/26/87";
/*
 * Program called by sysgen to print help messages (sg_help).
 *
 * Fred Canter
 */
#include "sysgen.h"

main(argc, argv)
int argc;
char *argv[];
{
	register int i;
	register char **p;

	if(argc != 2) {
		printf("\nsg_help: help message name missing!\n");
		exit(1);
	}
	for(i=0; sghelp[i].sgh_name; i++)
		if(strcmp(argv[1], sghelp[i].sgh_name) == 0)
			break;
	if(sghelp[i].sgh_name == 0) {
		printf("\nsg_help: Can't find `%s' help text!\n", argv[1]);
		exit(1);
	}
	p = sghelp[i].sgh_addr;
	for(i=0; p[i]; i++)
		if(p[i] == -1) {
			printf("\n\nPress <RETURN> for more:");
			while(getchar() != '\n') ;
		} else
			printf("\n%s", p[i]);
		printf("\n");
}

char	*sg_c[] =
{
	"",
	"The \"c\" command creates an ULTRIX-11 kernel configuration file.",
	"The \"c\" command asks for a configuration name. Enter the name",
	"you wish to use, or press <RETURN> to use the default name (unix).",
	"",
	"Sysgen will ask a series of questions about: the processor type,",
	"peripheral devices, and system parameter values. The prompts",
	"consist of the question followed by information enclosed in < >.",
	"A single item enclosed in < > represents the default answer to the",
	"question. To use the default answer, press <RETURN>. A list of",
	"items enclosed in < > indicates a multiple choice answer to the",
	"question. Select one item from the list as the answer. You must",
	"supply an answer, because these questions do not have defaults.",
	"",
	"If you are not sure how to answer a question, type \"?\", then",
	"press <RETURN>. A help message will be printed and sysgen will",
	"repeat the question.",
	"",
	"You can abort the \"c\" command and return to the \"sysgen>\" prompt",
	"at any time, by typing <CTRL/C>. You can back up to the previous",
	"question by typing <CTRL/D>. Typing <CTRL/D> will cause some of",
	"the information you have already entered to be erased. The amount",
	"depends on how far you backup.",
	0
};

/***** OBSOLETE!
char	*sg_cbsiz[] =
{
	"",
	"CANBSIZ specifies the size of the terminal canonicalization buffer",
	"in the kernel. This buffer is used for erase and kill processing",
	"when the system is accepting input from a terminal. That is, when",
	"you type <DELETE> to erase a character or <CTRL/U> to kill an",
	"entire line of input.",
	"",
	"CANBSIZ limits the length of a terminal input line. The default",
	"of 256 should be large enough for most applications. The cost of",
	"each CANBSIZ is one byte.",
	0
};
**********/

char	*sg_cmn[] =
{
	"",
	"Enter the number of communications device units to be configured,",
	"then press <RETURN>. To use the default value of one unit press",
	"<RETURN>.",
	"",
	"You can use the \"d\" command to list the maximum number of units",
	"allowed for each type of communications device.",
	0
};

char	*sg_cmt[] =
{
	"",
	"Enter the name of one of the communications devices listed below,",
	"then press <RETURN>. Sysgen will ask questions about the device.",
	"Answer these questions, then enter the name of the next device to",
	"be configured. If there are no more communications devices, press",
	"<RETURN> to terminate the list of devices.",
	"",
	"Name  Device      Description",
	"----  ------      ----------",
	"dz    DZ11        8 line multiplexer",
	"dzv   DZV11       4 line DZ11 for Q bus",
	"dzq   DZQ11       4 line multiplexer (DZV11 replacement)",
	"dh    DH11        16 line multiplexer",
	"dhdm  DM11-BB     DH11 modem control",
	"dhu   DHU11       16 line multiplexer",
	"dhv   DHV11       8 line DHU11 for Q bus",
	"du    DU11        synchronous line interface",
	"dn    DN11        auto-call unit interface",
	"kl    DL11/DLV11  (CSR 776500) single line unit",
	"dl    DL11/DLV11  (CSR 775610) single line unit",
	-1,
	"The first \"kl\" is reserved for the console terminal. The console",
	"terminal is automatically configured, do not count it in the \"kl\"",
	"specification. Use the \"kl\" and \"dl\" names for the equivalent",
	"DLV11 Q bus devices.",
	0
};

char	*sg_cn[] =
{
	"",
	"To use the default configuration name of \"unix\", press <RETURN>.",
	"",
	"To use an alternate configuration name, enter the name and press",
	"<RETURN>. The configuration name is limited to a maximum length",
	"of eight characters. Digital recommends you use only alphanumeric",
	"characters in the configuration name.",
	0
};

char	*sg_cpu[] =
{
	"",
	"If the new kernel is being generated for the current CPU, press",
	"<RETURN>. The number enclosed in < >, which is the current CPU",
	"type, will be used. If the new kernel is for another system",
	"enter the numeric portion of the processor type name, then press",
	"<RETURN>. The numbers enclosed in ( ) list the supported CPU types.",
	"",
	"For example, you would enter 70 for the PDP11/70 or 23+ for a",
	"PDP11/23 plus processor.",
	"",
	"The Micro/PDP-11 may be any of the following processor types:",
	"",
	"	23+ - KDF11-B (F11)",
	"	53  - KDJ11-D (J11)",
	"	73  - KDJ11-A (J11)",
	"	83  - KDJ11-B (J11)",
	"",
	"If the target processor is not listed, select the processor type",	
	"that most closely resembles your processor. Remember, separate I",
	"and D space is the most important processor feature.",
	0
};

char	*sg_memsz[] =
{
	"",
	"Sysgen is requesting the amount of memory on the target processor.",
	"The memory size is specified in K bytes, where, K is 1024 bytes.",
	"If the new kernel is being generated for the current CPU, press",
	"<RETURN> to use the value enclosed in < >, which is the current",
	"processor's memory size. If the new kernel is for another system,",
	"enter the memory size, then press <RETURN>.",
	"",
	"For example, if the processor has 256K bytes of memory, you would",
	"enter 256, if the processor has one megabyte of memory you would",
	"enter 1024.",
	"",
	"The absolute minimum memory size is 248K bytes. The maximum memory",
	"size is the processor's physical address space minus its I/O page.",
	"",
	"    4088K bytes - 22 bit Qbus processors (4MB - 8KB I/O page)",
	"    3840K bytes - 22 bit unibus processors (4MB - 256KB I/O page)",
	"     248K bytes - 18 bit processors (256KB - 8KB I/O page)",
	0
};

char	*sg_nomap[] =
{
	"",
	"Because the specified target processor does not have a unibus",
	"map, you can save approximately 650 bytes of kernel instruction",
	"space by omitting the unibus map support code.",
	"",
	"CAUTION: if the map support code is omitted, the kernel will not",
	"boot on any processor with a unibus map. These processors have",
	"a unibus map: PDP-11/24 (if KT24 installed), PDP-11/44, PDP-11/70",
	"and PDP-11/84.",
	"",
	"To omit the unibus map code, type yes<RETURN> or just <RETURN>.",
	"To force inclusion of the unibus map code, type no<RETURN>.",
	0
};

char	*sg_csr[] =
{
	"",
	"The number enclosed in < > is the default CSR address for the",
	"device. To use the default CSR address, press <RETURN>. If the",
	"device is not configured at the default CSR address, enter the",
	"actual address, then press <RETURN>. CSR addresses are always",
	"entered as octal numbers.",
	"",
	"A device's CSR address specifies the I/O page address used by",
	"the operating system to access the device. The term CSR actually",
	"denotes the Control and Status Register, which is normally the",
	"first in a group of I/O page registers for the device.",
	0
};

char	*sg_d[] =
{
	"",
	"The \"d\" command lists the devices that may be configured into",
	"the ULTRIX-11 kernel. Please note that not all of these devices",
	"are fully supported, refer to the ULTRIX-11 Software Product",
	"Description to resolve any questions about device support.",
	"",
	"The device list is used in conjunction with the \"c\" command to",
	"create the ULTRIX-11 kernel configuration file. The list has four",
	"columns of information:",
	"",
	"Device      - The type of device: CPU, disk, tape, etc.",
	"type",
	"",
	"Number      - The quantity of each device type that may be",
	"allowed       configured into the kernel.",
	"",
	"ULTRIX-11   - The ULTRIX mnemonic used to refer to the device.",
	"name",
	"",
	"Device      - Information about the device, such as its generic",
	"description   name and number of units per controller.",
	0
};

char	*sg_dc[] =
{
	"",
	"Sysgen is requesting a list of all the disk controllers to be",
	"configured into the kernel. Enter the name of the system disk",
	"controller first, then enter the names of the other controllers",
	"on your system. When you have entered all your disk controllers,",
	"terminate the list by pressing <RETURN>. Consult the list below",
	"for the names and usage each type of disk controller.",
	"",
	"When you enter a disk controller name, sysgen will ask a series",
	"of questions about the controller and the drives connected to it.",
	"Type the answer to each question, then press <RETURN>. Remember,",
	"you can just press <RETURN> to use the default answer or ?<RETURN>",
	"for help.",
	"",
	"Note -	all of the Q22 bus controllers may be used on processors",
	"	with the 18 bit Q bus (jumper selectable). CAUTION, if a",
	"	Q bus controller (rxv21, rlv11) is used on a processor",
	"	with the Q22 bus, the disk may be accessed in buffered I/O",
	"	mode only. Attempting RAW I/O transfers will cause errors.",
	"	The PDP11/23 has an 18 bit Q bus, PDP11/23+ has a Q22 bus.",
	-1,
	"Name	Usage		Disk Drives Supported",
	"----	-----		---------------------",
	"rh11	Unibus		RM02, RP04/5/6, ML11",
	"rh70	11/70 Massbus	RM02/3/5, RP04/5/6, ML11",
	"rp11	Unibus		RP02/3",
	"rk611	Unibus		RK06/7",
	"rk711	Unibus		RK06/7",
	"rl11	Unibus		RL01/2",
	"rlv11	Q bus		RL01/2  (* specify rl11)",
	"rlv12	Q22 bus		RL01/2  (* specify rl11)",
	"rx211	Unibus		RX02",
	"rxv21	Q bus		RX02    (* specify rx211)",
	"rk11	Unibus		RK05",
	"uda50	Unibus		RA60, RA80, RA81",
	"kda50	Q22 bus		RA60, RA80, RA81",
	"rqdx1	Q22 bus		RX50, RX33, RD31-32, RD51-54",
	"rqdx2	Q22 bus		RX50, RX33, RD31-32, RD51-54",
	"rqdx3	Q22 bus		RX50, RX33, RD31-32, RD51-54",
	"rux1	Unibus		RX50",
	"klesi	Unibus/Q22 bus	RC25",
	0
};

char	*sg_ddt[] =
{
	"",
	"Sysgen is requesting a list of the drives connected to the disk",
	"controller. The names enclosed in < > are the drive types that may",
	"be attached to the specified disk controller. Enter the type of",
	"each drive, in order, starting with unit zero. To terminate the",
	"list of drive types, just press <RETURN>.",
	"",
	"Sysgen assumes the disk units are numbered sequentially, starting",
	"with unit zero. To allow for non-sequential unit numbering, a",
	"drive type may be entered even if the disk drive is not physically",
	"present. The operating system will ignore any non-existent units.",
	"For example, if three RP06 disks are to be numbered 0, 1, and 4,",
	"you would also specify drives two and three as RP06 disks. Drives",
	"two and three would be ignored by the system. Non-sequential unit",
	"numbering is not recommended, because it wastes kernel data space",
	"by allocating slots in the disk driver information tables for non-",
	"existent drives.",
	0
};

char	*sg_dn[] =
{
	"",
	"Sysgen is requesting an ULTRIX-11 device mnemonic. This is the",
	"two character device name used by the operating system to refer",
	"to a device. The name enclosed in < > is the default mnemonic",
	"to use it press <RETURN>. You can use the \"d\" command to print",
	"a list of all the ULTRIX-11 device mnemonics.",
	0
};

char	*sg_dst[] =
{
	"",
	"If your local area uses daylight savings time, enter yes, if not,",
	"enter no, then press <RETURN>. If you enter yes, your ULTRIX-11",
	"system will automatically change to daylight savings time and back",
	"to standard time on the appropriate dates for your time zone.",
	0
};

char	*sg_fpsim[] =
{
	"",
	"If your processor is equipped with floating point hardware, type",
	"no<RETURN>. The floating point simulator is not needed if the CPU",
	"has floating point hardware.",
	"",
	"If your processor does not have the floating point hardware, type",
	"yes<RETURN>. Including the floating point simulation code in the",
	"kernel allows programs to execute floating point instructions on",
	"a processor without floating point hardware.",
	"",
	"If the processor does not have floating point hardware and the",
	"floating point simulation code is not included in the kernel, any",
	"program that executes floating point instructions will be core",
	"dumped with an illegal instruction trap. Many system programs use",
	"floating point instructions. If your CPU does not have floating",
	"point hardware, you should include the simulator code.",
	0
};

char	*sg_hz[] =
{
	"",
	"Enter the AC power line frequency for your local area, then press",
	"<RETURN>. The line frequency is expressed in hertz, also known as",
	"cycles per second.",
	"",
	"The AC power line frequency for the United States of America and",
	"Canada is 60 hertz. Australia, England, and Europe use 50 hertz AC",
	"power. Japan uses 50 hertz in some areas and 60 hertz in others.",
	"If you are unsure about your AC power line frequency, you should",
	"contact your local power company.",
	"",
	"Although 50 and 60 hertz are the standard line frequencies, you",
	"can enter any value, suggested range of 45 - 65 hertz. This allows",
	"you to compensate for AC line frequency variations that sometimes",
	"occur in local areas. CAUTION, if you enter an incorrect AC line",
	"frequency value, your computer system will not keep accurate time.",
	0
};

char	*sg_i[] =
{
	"",
	"The \"i\" command does not actually install the new kernel, instead",
	"it prints the step by step procedure for installing the new kernel.",
	"To ensure proper installation, follow the instructions exactly.",
	"",
	"To install a new kernel you must be superuser. To become superuser,",
	"type \"su\" then <RETURN>. If the \"su\" command asks for a password,",
	"enter the password for the \"root\" account, then press <RETURN>.",
	"",
	"Next you move the new kernel into the root directory, shutdown the",
	"system, and boot the new kernel. Finally, you save a copy of the old",
	"kernel, rename the new kernel to \"unix\", set the date and time,",
	"check your file systems, and enter multi-user mode.",
	"",
	"For more information, you can refer to Section 2.4 of the ULTRIX-11",
	"System Management Guide.",
	0
};

char	*sg_l[] =
{
	"",
	"The \"l\" command lists the names of all configuration files that",
	"exist in the current directory, usually /sys/conf. Each file will",
	"have a name of the form name.cf, for example, if the configuration",
	"name is unix the file would be unix.cf. The \".cf\" extension is",
	"added to the configuration file name, by the sysgen program, to",
	"provide unique and easily identified names. When using the sysgen",
	"program, you refer to the configuration by its name only, that is,",
	"do not type the \".cf\" extension.",
	0
};

char	*sg_m[] =
{
	"",
	"The \"m\" command is used to make an ULTRIX-11 kernel based on a",
	"configuration file (name.cf, created with the \"c\" command). The",
	"sysgen program asks for the configuration name. Enter the name of",
	"your configuration (don't type the \".cf\"), then press <RETURN>.",
	"To use the default name (unix), just press <RETURN>.",
	"",
	"Sysgen makes the new kernel by executing other programs. These",
	"programs produce a large volume of informational messages, which",
	"can be ignored unless the make fails. In that case they should",
	"indicate the cause of the failure. If the make succeeds, the new",
	"kernel will be in a file named \"name.os\", for example, unix.os",
	"if the default configuration name was used.",
	"",
	"If the make fails, refer to Section 2.7 of the ULTRIX-11 System",
	"Management Guide for help with resolving the error.",
	0
};
char	*sg_mapsz[] =
{
	"",
	"MAPSIZE sets the size of the core and swap maps in the ULTRIX-11",
	"kernel. These maps are used for keeping track of free segments of",
	"memory and swap space. The default value is a function of the",
	"number of processes (NPROC); 30+(NPROC/2). The worst case value",
	"for MAPSIZE would be (2*NPROC)+2, though the maps rarely get to",
	"that size. MAPSIZE should only be changed if you get \"mapsize",
	"exceeded\" messages on the system console terminal; this is most",
	"likely on processors that have large memory sizes and do not",
	"have separate Instruction and Data space.",
	"",
	"Press <RETURN> to use the default MAPSIZE or enter an alternate",
	"value, then press <RETURN>.",
	"",
	"The cost of each MAPSIZE is 4 bytes of kernel data space.",
	0
};

char	*sg_ulimit[] =
{
	"",
	"ULIMIT controls the maximum size a file can grow to by limiting",
	"the file write pointer offset. The default file size limit is",
	"1 Megabyte (1024 Kbytes). The value of ulimit is always in terms",
	"of Kbytes.",
	"",
	"Press <RETURN> to use the default ULIMIT or enter an alternate",
	"value, then press <RETURN>.",
	0
};

/* NO LONGER USED
char	*sg_mbcn[] =
{
	"",
	"Each RH11/RH70 disk controller must be assigned a unique number,",
	"because they are accessed via the same software driver, that is,",
	"the general MASSBUS disk driver. The controller number identifies",
	"the RH11/RH70 to the general MASSBUS disk driver. Each controller",
	"also has a unique ULTRIX-11 mnemonic:",
	"",
	"	HP = (first ) RH11/RH70 - RM02/3/5, RP04/5/6, ML11",
	"	HM = (second) RH11/RH70 - RM02/3/5, RP04/5/6, ML11",
	"	HJ = (third ) RH11/RH70 - RM02/3/5, RP04/5/6, ML11",
	"",
	"Sysgen displays the default controller number enclosed in < >.",
	"To use the default number, just press <RETURN>. Sysgen allows you",
	"to configure up three RH11/RH70 controllers. If the system disk",
	"is connected to an RH11/RH70, that controller should be assigned",
	"controller number zero by specifying it first.",
	0
};
*/

char	*sg_md[] =
{
	"",
	"Press <RETURN> to use the default minor device number or enter a",
	"minor device number followed by <RETURN>. The minor device number",
	"specifies the disk unit number and partition where the file system",
	"will reside.",
	"",
	"For non-partitioned disks (RX50, RX02, RK05, ML11), the minor",
	"device number is the disk unit number. For all other disks, the",
	"minor device number specifies a disk partition as well as the disk",
	"unit number. Bits 0-2 are the partition number and bits 3-5 are",
	"the unit number.",
	"",
	"Refer to Section 1 of the ULTRIX-11 System Management Guide for a",
	"description of the ULTRIX-11 I/O system, disk partitioning, and",
	"minor device numbers.",
	0
};

char	*sg_mseg[] =
{
	"",
	"MAXSEG limits the maximum amount of memory that the operating",
	"system will use. To ensure that the system uses all available",
	"memory, use the default MAXSEG value by pressing <RETURN>.",
	"There is no harm is setting MAXSEG larger than the physical",
	"memory size.",
	"",
	"MAXSEG is only changed for maintenance purposes, that is, to",
	"force swapping or avoid a known faulty section of memory. The",
	"value of MAXSEG is the number of 64 bytes memory segments to",
	"be used. The system will use all available memory up to and",
	"including the limit set by MAXSEG. For example, the default",
	"MAXSEG of 61440 allows the system to use up to 3.75 megabytes",
	"of memory (4Mb - I/O page). Setting MAXSEG to 16384 would limit",
	"the memory size to 1 megabyte.",
	0
};

char	*sg_msgb[] =
{
	"",
	"MSGBUFS specifies the size of the system error message buffer in",
	"the ULTRIX-11 kernel. All system error messages, printed on the",
	"console terminal, are also saved in this buffer for collection at",
	"a later time by the DMESG program. DMESG runs every 10 minutes and",
	"transfers the error messages from the kernel buffer to a file",
	"(/usr/adm/messages). If more than MSGBUFS characters of error",
	"message text are printed in a 10 minute period, some previous error",
	"messages will be overwritten. This is due to circular buffering.",
	"",
	"Use the default MSGBUFS value by pressing <RETURN>, or enter an",
	"alternate value, then press <RETURN>. The cost of each MSGBUFS is",
	"one byte of kernel data space.",
	0
};

char	*sg_mtc[] =
{
	"",
	"Sysgen is requesting a list of the magtape controllers to be",
	"configured into the new kernel. Most systems will have only a",
	"single magtape controller, however, multiple controllers may be",
	"included. That is, one TM02/3, one TM11, four TK50/TU81,",
	"four TS11/TU80/TSV05/TSU05.",
	"",
	"Enter a magtape controller name, from the list below, then press",
	"<RETURN>. Sysgen will ask several questions about the controller",
	"and the drives connected to it. Answer each question, then press",
	"<RETURN>. After you have entered the last magtape controller,",
	"terminate the list of controllers by pressing <RETURN>.",
	-1,
	"Name	Usage		Tape Drives Supported",
	"----	-----		---------------------",
	"tm02/3	Unibus		TU16, TE16, TU77",
	"tm11	Unibus		TU10, TE10, TS03",
	"ts11	Unibus		TS11",
	"tsv05	Qbus/Q22bus	TSV05",
	"tsu05	Qbus/Q22bus	TSU05",
	"tu80	Unibus		TU80",
/*	"tk25	Qbus/Q22bus	TK25",	*/
	"tk50	Unibus/Q22bus	TK50",
	"tu81	Unibus		TU81",
	0
};

char	*sg_cdd[] =
{
	"",
	"Sysgen is requesting the name of the crash dump device. Select",
	"the crash dump device from the list of names enclosed in < >.",
	"Enter the name, then press <RETURN>. There is no default crash",
	"dump device, you must enter one of the names from the list.",
	"",
	"The ULTRIX-11 system takes a crash dump by writing an image of",
	"memory to the crash dump device. The \"memory image\" is copied",
	"to a file on the system disk for analysis by the CDA (Crash Dump",
	"Analysis) program.",
	"",
	"Digital recommends you select a magtape for the crash dump device",
	"if one is available. This will ensure that all of the system's",
	"memory will be saved in the crash dump.",
	"",
	"If a magtape is not available, the crash dump can be written to",
	"the swap area of the system disk. If the system disk controller",
	"is an RQDX1/2/3, you may select one of the floppy disk drives",
	"as the crash dump device.",
	"",
	"Depending on the type of disk and the amount of memory, the swap",
	"area may not be large enough to hold the entire \"memory image\".",
	"In this case, some crash dump data may be lost.",
	0
};

char	*sg_mtn[] =
{
	"",
	"Sysgen is requesting the number of tape drives connected to the",
	"magtape controller. Enter the number of magtape units, then press",
	"<RETURN>. To use the default response of one unit, press <RETURN>.",
	"",
	"Sysgen asks for the number of magtape units instead of the type",
	"of each unit because the software drivers for magtapes adapt to",
	"the drive type automatically.",
	"",
	"Sysgen expects magtape units to be numbered sequentially. However,",
	"non-sequential numbering may be used by setting the number of units",
	"to one more than the highest numbered unit. For example, if three",
	"tape units were to be numbered 0, 1, and 4, you would specify 5",
	"magtape units. The system will ignore the nonexistent units. Non-",
	"sequential unit numbering is not recommended because the system",
	"must allocate space in the magtape driver information tables for",
	"nonexistent units.",
	0
};

char	*sg_muprc[] =
{
	"",
	"MAXUPRC sets the maximum number of processes that a user can have",
	"running simultaneously. MAXUPRC should be set just large enough",
	"that users can get work done but not so large that a user can",
	"consume all available processes, in the event of a programming",
	"error.",
	"",
	"Press <RETURN> to use the default value, or enter an alternate",
	"value, then press <RETURN>.",
	"",
	"There is no data space cost associated with MAXUPRC.",
	0
};

char	*sg_nb[] =
{
	"",
	"Sysgen is requesting the number of 512 byte logical blocks to be",
	"allocated to a file or file system, such as the swap area or the",
	"error log.",
	"",
	"For help with this question, refer to Section 1 and Appendix D of",
	"the ULTRIX-11 System Management Guide.",
	"",
	"To use the default number of blocks press <RETURN> or enter the",
	"number of blocks followed by <RETURN>.",
	0
};

char	*sg_nbuf[] =
{
	"",
	"NBUF sets the size of the I/O buffer cache in the ULTRIX-11",
	"kernel. Increasing the number of buffers should improve system",
	"performance. However, increasing NBUF also increases the amount",
	"of memory consumed by the operating system. Digital recommends you",
	"use the default NBUF for the initial system generation and delay",
	"experimenting with the size of the buffer cache until reliable",
	"system operation has been established.",
	"",
	"To use the default value for NBUF, press <RETURN>. To change the",
	"size of the I/O buffer cache, enter the number of buffers, then",
	"press <RETURN>.",
	"",
	"Each NBUF costs 30 bytes of kernel data space for the buffer",
	"header and 1024 bytes of memory (outside of kernel data space)",
	"for the actual buffer.",
	0
};

char	*sg_ncall[] =
{
	"",
	"NCALL sets the size of the callout table in the ULTRIX-11 kernel.",
	"Callouts are entered in this table when internal system timing",
	"must be done, such as carriage return delays for terminals.",
	"",
	"The default NCALL size should be sufficient for most systems. To",
	"use the default value, press <RETURN>. To change the size of the",
	"callout table, enter the new value, then press <RETURN>.",
	"",
	"The cost of each NCALL is eight bytes of kernel data space.",
	0
};

char	*sg_ncargs[] =
{
	"",
	"NCARGS is the maximum number of characters allocated for the",
	"argument list when a process is created via the \"exec\" system",
	"call. NCARGS limits the number of arguments that can be passed",
	"to a process. Each \"exec\" system call requires (NCARGS+511)/512",
	"contiguous blocks in the swap area, to hold the argument list.",
	"",
	"The default NCARGS value should be large enough for most systems.",
	"To use the default value press <RETURN> or enter an alternate",
	"value followed by <RETURN>.",
	"",
	"NCARGS use no kernel data space. However, setting NCARGS too high",
	"may cause swap space exhaustion or fragmentation.",
	0
};

char	*sg_nclist[] =
{
	"",
	"NCLIST sets the number of 30 character clist segments (cblocks)",
	"allocated to the clist in the ULTRIX-11 kernel. Clists are used",
	"to buffer characters for devices like terminals.",
	"",
	"NCLIST should be large enough that the clists does not become",
	"exhausted at times of high terminal I/O activity. Enough clists",
	"should be allocated so that every terminal can have one average",
	"length line pending (about 30 or 40 characters).",
	"",
	"The default NCLIST value should be adequate for most systems. To",
	"use the default value, press <RETURN>. To change the clist size,",
	"enter the new value, then press <RETURN>. Use the following rule",
	"to calculate the number of Clists:",
	"",
	"   NCLIST = 55 + (3 times average number of active terminals)",
	"",
	"The cost of each NCLIST is 32 bytes of kernel data space.",
	0
};

char	*sg_nfile[] =
{
	"",
	"NFILE sets the size of the \"open file\" table in the ULTRIX-11",
	"kernel. The size of this table limits the number of simultaneous",
	"open files the system may have.",
	"",
	"NFILE should be about the same size as NINODE. To use the default",
	"value, press <RETURN>. To change NFILE, enter the new value, then",
	"press <RETURN>.",
	"",
	"The cost of each NFILE is 8 bytes of kernel data space.",
	0
};

char	*sg_ninode[] =
{
	"",
	"NINODE sets the size of the \"in core inode\" table in the ULTRIX-11",
	"kernel. There will be an entry in this table for every open file,",
	"that is, device special file, current working directory, sticky",
	"text segment, open file, or mounted file system.",
	"",
	"NINODE should be approximately NPROC+NMOUNT+(number of terminals).",
	"You can use the default value by pressing <RETURN>, or enter the",
	"value of NINODE followed by <RETURN>.",
	"",
	"The cost of each NINODE is 54 bytes of kernel data space.",
	0
};

char	*sg_nmnt[] =
{
	"",
	"NMOUNT sets the size of the mount table in the ULTRIX-11 kernel.",
	"The size of this table limits the number of mounted file systems",
	"to NMOUNT. Each mounted file system requires an entry in the",
	"mount table and a buffer, from the I/O buffer cache, to hold its",
	"superblock.",
	"",
	"NMOUNT should be set to the number of permanently mounted file",
	"systems plus a number of temporary mounts. The permanent mounts",
	"can be determined by counting the active entries in the file",
	"system table (/etc/fstab). An active entry is one marked \"rw\" or",
	"\"ro\", not \"xx\". The number of temporary mounts depends on the",
	"system configuration and work load, two is generally enough.",
	"",
	"Press <RETURN> to use the default NMOUNT, or enter the number of",
	"mounts followed by <RETURN>.",
	"",
	"The cost of each NMOUNT is 6 bytes of kernel data space and the",
	"dynamic allocation of a buffer from the I/O buffer cache.",
	0
};

char	*sg_nproc[] =
{
	"",
	"NPROC sets the size of the process table in the ULTRIX-11 kernel.",
	"The size of this table limits the number of processes that can",
	"be active in the system. Each active process requires an entry in",
	"the process table.",
	"",
	"There is no set rule for the size of NPROC, it depends on how the",
	"system is being used. The default value should be sufficient for",
	"most systems, press <RETURN> to use the default NPROC. To change",
	"NPROC, enter the new value, then press <RETURN>.",
	"",
	"The cost of each NPROC is 44 bytes of kernel data space.",
	0
};

char	*sg_ntext[] =
{
	"",
	"NTEXT sets the size of the \"text\" table in the ULTRIX-11 kernel.",
	"The size of the text table limits the number of shared text (pure",
	"code) segments that may be active in the system.",
	"",
	"The default value for NTEXT should be sufficient for most systems,",
	"press <RETURN> to use the default value. NTEXT should be increased",
	"for systems with a large number of shared text processes. The",
	"NTEXT value can be changed by entering the new value followed by",
	"<RETURN>.",
	"",
	"The cost of each NTEXT is 12 bytes of kernel data space.",
	0
};

char	*sg_p[] =
{
	"",
	"The \"p\" command prints an ULTRIX-1 kernel configuration. When",
	"the \"c\" command creates the configuration (name.cf) file, it",
	"also creates a formatted text printout of the configuration. This",
	"information is stored in the configuration print file (name.cf_p).",
	"For example, the default configuration (unix) would have files",
	"unix.cf and unix.cf_p. The \"p\" command simply prints the unix.cf_p",
	"file. The printout consists of the following information:",
	"",
	"  o  Type of kernel (split I & D, or overlay)",
	"",
	"  o  Configured devices, number of units, CSR/VECTOR addresses",
	"",
	"  o  ROOT, PIPE, SWAP, and ERROR LOG file placements",
	"",
	"  o  Values of all system parameters",
	"",
	"  o  Timezone and daylight savings time flags",
	0
};

char	*sg_r[] = 
{
	"",
	"The \"r\" command is used to remove old or unwanted configuration",
	"files. Both the name.cf and name.cf_p files are removed. The \"r\"",
	"command asks for the configuration file name. Enter the name, then",
	"press <RETURN>. Enter only the name, not the \".cf\" or \".cf_p\".",
	0
};

char	*sg_s[] =
{
	"",
	"The \"s\" command is used to compile a kernel source code module",
	"and, in the case of the split I & D kernel, replace the object",
	"module in the appropriate library. Because source code is not",
	"supplied with the ULTRIX-11 system, the only use for this command",
	"is for compiling user written device drivers.",
	"",
	"ULTRIX-11 source files are grouped in one of two libraries. The",
	"kernel library (LIB1) or the device driver library (LIB2). For",
	"the split I & D kernel, LIB1 and LIB2 are actual object module",
	"libraries. For the overlay kernel, LIB1 and LIB2 are directories",
	"containing the object modules.",
	-1,
	"The following is a list of the questions asked by the \"s\" command",
	"and the possible responses to each question:",
	"",
	"  o  Libraries to be rearchived ?",
	"",
	"     s - split I & D libraries (LIB1_id & LIB2_id)",
	"     o - overlay kernel objects (ovsys & ovdev)",
	"     b - rebuild both libraries",
	"",
	"  o  LIB1 - system library source files to remake ?",
	"",
	"     all - rebuild all objects in the library",
	"     <RETURN> - none of the objects in the library",
	"     file1 file2 ... fileN - list of objects to rebuild",
	"     (example: sys1 sys2 sys3, note the .c is omitted)",
	"",
	"  o  LIB2 - device driver library source files to remake ?",
	"",
	"     Same as for LIB1, but use driver module names,",
	"     (example: u1 u2 u3, note the .c is omitted)",
	0
};

char	*sg_sb[] =
{
	"",
	"Sysgen is asking for the starting logical block number of a file",
	"system, such as the swap area or error log. The starting block",
	"number is relative to the start of the disk partition, not the",
	"start of the physical disk unit.",
	"",
	"For help with this question, refer to Section 1 and Appendix D of",
	"the ULTRIX-11 System Management Guide.",
	"",
	"To use the default number of blocks press <RETURN>, or enter the",
	"number of blocks followed by <RETURN>.",
	0
};

char	*sg_sd[] =
{
	"",
	"Sysgen is asking if the system disk is connected to the current",
	"disk controller. The system disk is where the ULTRIX-11 ROOT file",
	"system is located. Sysgen will ask this question only if the",
	"system disk has not already been specified.",
	"",
	"If the system disk is on this controller, enter yes<RETURN> or",
	"just <RETURN>. If not, enter no<RETURN>.",
	0
};

char	*sg_sdn[] =
{
	"",
	"Sysgen is asking for the unit number of the system disk. The",
	"default unit number is <1> for the RC25 and <0> for all other",
	"disks. To use the default unit number, press <RETURN>.",
	"",
	"You can specify a different unit number by entering the number",
	"followed by <RETURN>. If you do not use the default unit number,",
	"the following items must be considered:",
	"",
	"  o  Not all hardware bootstraps can boot from units other than",
	"     zero. You can load the boot from the distribution tape.",
	"",
	"  o  The boot file specification will change. For example, unit",
	"     two would be ??(2,0)unix, where ?? is the disk mnemonic.",
	"",
	"  o  The /etc/fstab must be modified, see fstab(5) in the",
	"     ULTRIX-11 Programmer's Manual, Volume 1.",
	"",
	"  o  You must remake the file /dev/swap so that commands can",
	"     access the swap area, see /dev/makefile.",
	0
};

char	*sg_sh[] =
{
	"",
	"The \"!\" is used to escape from sysgen to the shell. This allows",
	"you to execute system commands without exiting from sysgen. The",
	"\"!\" feature may only be used at the \"sysgen>\" prompt.",
	"",
	"The usage of the \"!\" command is:",
	"",
	"	!command",
	"",
	"where \"command\" is the system command to be executed. For a",
	"description of the system commands refer to Volume One of the",
	"ULTRIX-11 Programmer's Manual.",
	"",
	"For example:",
	"",
	"	!ls -l",
	"",
	"would print a long directory listing of the current directory.",
	0
};

char	*sg_sl[] =
{
	"",
	"Sysgen contains tables that define the standard location of the",
	"ULTRIX-11 ROOT, PIPE, SWAP, and ERROR LOG file systems on each",
	"type of disk. Digital strongly recommends you use the standard",
	"placements for these file systems. Type yes<RETURN> or just",
	"<RETURN> to use the standard placements.",
	"",
	"If you intend to experiment with the placement of these file",
	"systems, wait until the initial system installation has been",
	"completed and reliable system operation is established before",
	"generating a system with nonstandard placements. Also, backup",
	"your disks before booting a kernel with nonstandard placements!",
	"",
	"To use nonstandard placements, answer no<RETURN>. Sysgen will ask",
	"a series of questions about the placement of the ROOT, SWAP, PIPE,",
	"and ERROR LOG file systems. Along with each question sysgen will",
	"print a default value, enclosed in < >, which is the standard",
	"placement for the item in question. You may use the default value",
	"or enter a new value. WARNING, sysgen accepts your answers without",
	"checking them for errors!",
	-1,
	"The following hints may be helpful:",
	"",
	"  o  Placing the ROOT and SWAP on separate disk controllers will",
	"     improve system performance. Placing them on different drives",
	"     on the same controller is of little benefit.",
	"",
	"  o  If the system makes heavy use of pipes, placing the PIPE",
	"     device on a separate disk controller should improve system",
	"     performance. Otherwise PIPE and ROOT should be the same.",
	"",
	"  o  All four file systems may exist within the same partition.",
	"     Care must be taken to prevent file system overlap.",
	"",
	"  o  The mkconf program (called by sysgen to make the kernel)",
	"     does some checking of file system placements.",
	"",
	"  o  Some of the auto-boot features may not function with non-",
	"     standard placements of the ROOT, PIPE, SWAP, and ERROR LOG.",
	"     Refer to Section 3 of the ULTRIX-11 System Management Guide.",
	"",
	"  o  If the standard placements are not used, the only available",
	"     crash dump devices will be magtape and RQDX1/RX50 (unit 2).",
	0
};

char	*sg_sp[] =
{
	"",
	"The ULTRIX-11 system parameters specify the size of the kernel's",
	"internal data structures, such as the process table. The values",
	"of these parameters are used to adjust the sizes of the internal",
	"data structures to match the expected system load, that is, the",
	"number of users and job mix.",
	"",
	"Sysgen contains a table of standard values for these parameters.",
	"Digital recommends that the standard parameters be used for the",
	"initial system generation, and that experimentation with the",
	"parameter values be delayed until reliable system operation is",
	"established. To use the standard parameters, answer yes<RETURN>",
	"or just <RETURN>.",
	"",
	"To change the parameters, answer no<RETURN>. Sysgen will ask for",
	"the value of each parameter. Sysgen will also print the standard",
	"value of each parameter, enclosed in < >.",
	-1,
	"The system parameters are:",
	"",
	"Param	OV_VAL	ID_VAL	COST	Description",
	"-----	------	------	----	-----------",
	"NINODE	    75	   150	  54	In-core inode table size",
	"NFILE	    75	   150	   8	Number of open files",
	"NMOUNT	     5	     8	   6	Number of mounted file systems",
	"MAXUPRC     15	    25	   0	Maximum processes per user",
	"NCALL	    30	    50	   6	Number of callouts",
	"NPROC	    75	   150	  42	Number of processes allowed",
	"NTEXT	    25	    40	  12	Number of shared text segments",
	"NCLIST	    50	    65	  32	Number of cblocks in clist",
/*	"CANBSIZ    256	   256	   1	TTY canon buffer size",	*/
	"NCARGS	  5120	  5120	   0	Exec() argument list size",
	"MSGBUFS    128	   128	   1	Error message buffer size",
	"MAXSEG	 61440	 61440	   0	Memory size limit",
	"MAPSIZE     67	   105	   4	Core/swap map size",
	"ULIMIT    1024	  1024	   0	Maximum file size",
	0
};

char	*sg_tz[] =
{
	"",
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
	0
};
char	*sg_udev[] =
{
	"",
	"Sysgen allows you to configure up to four user written device",
	"drivers into the ULTRIX-11 kernel. If there are no user devices,",
	"press <RETURN>. Otherwise, enter the name of the first user",
	"device (u1, u2, u3, u4). Sysgen will ask a series of questions",
	"about the user device. Answer these questions, then enter the name",
	"of the next user device. If there are no more user devices, press",
	"<RETURN> to terminate the list.",
	"",
	"To create a user written device driver, examine one of the user",
	"device driver prototype files (u1.c u2.c u3.c u4.c) in the",
	"/sys/dev directory. These files contain empty functions that",
	"define the interface to the ULTRIX-11 kernel. Edit your driver",
	"source code into these empty functions. Use the \"s\" command to",
	"compile and archive the new driver. Use the \"m\" command to make",
	"and install a new kernel.",
	"",
	"For additional information, refer to Section 2.8 of the ULTRIX-11",
	"System Management Guide.",
	0
};

char	*sg_vec[] =
{
	"",
	"The number enclosed in < > is the default interrupt vector address",
	"for the device. To use the default vector, press <RETURN>. If the",
	"device is not configured at the default vector address, enter the",
	"actual vector address followed by <RETURN>. The vector address is",
	"always entered as an octal number.",
	"",
	"A device's vector address is the address the processor will use",
	"to vector to the interrupt service routine for the device. The",
	"vector area is in low memory (locations 0 - 0776).",
	0
};

char	*sg_shuff[] =
{
	"",
	"Sysgen allow you to optionally configure the shuffle routine",
	"into the kernel. The shuffle routine is called upon to move",
	"processes in memory whenever one of the following occurs:",
	"",
	"	a process either locks or unlocks itself using",
	"		the plock or lock system calls.",
	"",
	"	a locked process grows in size.",
	"",
	"The shuffle code will attempt to place all locked processes",
	"at the beginning of user memory to avoid fragmentation.",
	"",
	"Process locking is a super-user only system call and should",
	"be used sparingly if at all. If you plan on using the plock",
	"system call you should include the shuffle routine. The shuffle",
	"routine takes up 222 bytes of text space.",
	"",
	0
};

char	*sg_mesg[] =
{
	"",
	"Sysgen allows you to optionally include the interprocess",
	"communication message facility. The message facility provides",
	"the following system calls:",
	"",
	"			msgctl",
	"			msgget",
	"			msgsnd",
	"			msgrcv",
	"",
	"as documented in Section 2 of the ULTRIX-11 Programmers Manual.",
	"",
	"There are no programs supplied with ULTRIX-11 that make use of",
	"the message facility. If any of your application programs utilize",
	"the message facility you must answer yes to this question.",
	"",
	"Utilizing the standard parameters, the message facility uses",
	"2090 bytes of kernel text space, 1280 bytes of kernel data",
	"space for split I&D machines and 870 bytes of kernel data space",
	"for non-split I&D machines as well as 8192 bytes of memory",
	"outside of the kernel.",
	0
};

char	*sg_sema[] =
{
	"",
	"Sysgen allows you to optionally include the interprocess",
	"communication semaphore facility. The semaphore facility",
	"provides the following system calls:",
	"",
	"			semctl",
	"			semget",
	"			semop",
	"",
	"as documented in Section 2 of the ULTRIX-11 Programmers Manual.",
	"",
	"There are no programs supplied with ULTRIX-11 that make use of",
	"the semaphore facility. If any of your application programs utilize",
	"the semaphore facility you must answer yes to this question.",
	"",
	"Utilizing the standard parameters, the semaphore facility uses",
	"3332 bytes of kernel text space and 3306 bytes of kernel data",
	"space for split I&D machines and 1856 bytes of kernel data space",
	"for non-split I&D machines.",
	0
};

char	*sg_smpar[] =
{
	"",
	"The standard semaphore parameters specify the size of the kernel's",
	"internal data structures and memory outside the kernel for ",
	"internal data structures for implementing the interprocess ",
	"communication semaphore facility.",
	"",
	"Sysgen contains a table of standard values for these parameters.",
	"Digital recommends that the standard parameters be used for the",
	"initial system generation, and that experimentation with the",
	"parameter values be delayed until reliable system operation is",
	"established. To use the standard parameters, answer yes<RETURN>",
	"or just <RETURN>.",
	"",
	"To change the parameters, answer no<RETURN>. Sysgen will ask for",
	"the value of each parameter. Sysgen will also print the standard",
	"value of each parameter, enclosed in < >.",
	"",
	-1,
	"The semaphore parameters are:",
	"",
	"Param   OV_VAL  ID_VAL  COST  Description",
	"-----   ------  ------  ----  -----------",
	"SEMMAP       5      10     4  Number of entries in semaphore map",
	"SEMMNI       5      10    28  Maximum number of semaphore",
	"                              identifiers in the system",
	"SEMMNS      30      60     8  Maximum number of semaphores ",
	"                              in the system",
	"SEMUME      10      10     6  Maximum number of undo entries",
	"                              per process",
	"SEMMNU      15      30  (10+6*SEMUME) Number of undo structures",
	"                              in the system",
	"SEMMSL      25      25     2  Maximum number of semaphores",
	"                              per semaphore id",
	"SEMOPM      10      10     6  Maximum number of operations",
	"                              per semop call",
	0
};

char	*sg_mgpar[] =
{
	"",
	"The standard message parameters specify the size of the kernel's",
	"internal data structures and memory outside the kernel for ",
	"implementing the interprocess communication message facility.",
	"The amount of memory required outside the kernel is equal to",
	"MSGSSZ * MSGSEG bytes",
	"",
	"Sysgen contains a table of standard values for these parameters.",
	"Digital recommends that the standard parameters be used for the",
	"initial system generation, and that experimentation with the",
	"parameter values be delayed until reliable system operation is",
	"established. To use the standard parameters, answer yes<RETURN>",
	"or just <RETURN>.",
	"",
	"To change the parameters, answer no<RETURN>. Sysgen will ask for",
	"the value of each parameter. Sysgen will also print the standard",
	"value of each parameter, enclosed in < >.",
	"",
	-1,
	"The message parameters are:",
	"",
	"Param   OV_VAL  ID_VAL    COST  Description",
	"-----   ------  ------    ----  -----------",
	"MSGMAP      50     100       4  Number of entries in message map",
	"MSGMAX    8192    8192       1  Maximum message size",
	"MSGMNB   16384   16384       1  Maximum number of bytes ",
	"                                on a message queue",
	"MSGMNI       5      10      42  Number of message queue",
	"                                identifiers in the system",
	"MSGSSZ       8       8       1  Message segment size in bytes",
	"MSGTQL      40      40      10  Maximum number of messages",
	"                                in the system",
	"MSGSEG    1024    1024  MSGSSZ  Number of message segments",
	0
};

char	*sg_flock[] =
{
	"",
	"Sysgen allows you to optionally include the advisory record and",
	"file locking facility. This facility provides the lockf system call",
	"and extends the functionality of the fcntl system call.",
	"",
	"There are no programs supplied with ULTRIX-11 that make use of",
	"the record and file locking facility. If any of your application",
	"programs utilize the record and file locking facility you must answer",
	"yes to this question.",
	"",
	"Utilizing the standard parameters, the record and file locking",
	"facility uses 3608 bytes of kernel text space and 2240 bytes of ",
	"kernel data space for split I&D machines and 1344 bytes of kernel",
	"data space for non-split I&D machines",
	0
};

char	*sg_maus[] =
{
	"",
	"Sysgen allows you to optionally include the maus (multiple access",
	"user space) facility. This facility provides the following system",
	"calls:",
	"",
	"			getmaus",
	"			freemaus",
	"			enabmaus",
	"			dismaus",
	"			switmaus",
	"",
	"Maus enables inter process communication through shared memory space.",
	"For more details refer to maus in Section 2 of ULTRIX-11 Programmer's",
	"Manual, Volume 1",
	"",
	"There are no programs supplied with ULTRIX-11 that make use of",
	"the maus facility. If any of your application programs utilize",
	"the maus facility you must answer yes to this question.",
	"",
	"Utilizing the standard parameters, the maus facility uses 780 bytes",
	"of kernel text space and 20 bytes of kernel data space, as well as",
	"24704 bytes of memory outside the kernel",
	0
};

char	*sg_mspar[] =
{
	"",
	"The standard maus parameters specify the size of the kernel's",
	"internal data structures for implementing multiple access user space.",
	"",
	"Sysgen contains a table of standard values for these parameters.",
	"There can be only a maximum of eight (8) maus segments and the ",
	"maximum size of each segment is 8K bytes (128*64). Size of the",
	"segment is to be given in 64 byte clicks.",
	"The number of maus segments and the size of each segment may be",
	"changed according to the application requirements.",
	"Increasing the size of the segments will reduce the total user",
	"memory available for applications. Irrespective of the size",
	"of each segment, when a process attaches a segment, 8K bytes ",
	"of data space is consumed by the process. ",
	"To use the standard parameters, answer yes<RETURN> or just <RETURN>",
	"",
	"To change the parameters, answer no<RETURN>. Sysgen will ask for",
	"the value of each parameter. Sysgen will also print the standard",
	"value of each parameter, enclosed in < >.",
	"",
	-1,
	"The maus parameters are:",
	"",
	"Param	  OV_VAL   ID_VAL    COST       Description",
	"-----	  ------   ------    ----	-----------",
	"NMAUS       4	      4                 Number of maus segments",
	"MAUS0       2	      2     MAUS0*64	Size of maus0 segment",
	"MAUS1	   128      128     MAUS1*64	Size of maus1 segment",
	"MAUS2	   128	    128     MAUS2*64	Size of maus2 segment",
	"MAUS3	   128	    128	    MAUS3*64	Size of maus3 segment",
	"MAUS4	     1	      1	    MAUS4*64	Size of maus4 segment",
	"MAUS5	     1	      1	    MAUS5*64	Size of maus5 segment",
	"MAUS6	     1	      1	    MAUS6*64	Size of maus6 segment",
	"MAUS7	     1	      1	    MAUS7*64	Size of maus7 segment",
	0
};
char	*ms_nmaus[] =
{
	"",
	"Nmaus is the number of maus segments to be configured in the ",
	"system. The maximum value is eight (8).",
	"",
	0
};
char	*ms_maus[] =
{
	"",
	"Maus is the size of the maus segment. The size is to be given",
	"in 64 byte clicks. The maxmimum value is 128 clicks (8K bytes)",
	"",
	0
};
char	*sg_flkpar[] =
{
	"",
	"The standard locking parameters specify the size of the kernel's",
	"internal data structures for implementing file and record locking.",
	"",
	"Sysgen contains a table of standard values for these parameters.",
	"Digital recommends that the standard parameters be used for the",
	"initial system generation, and that experimentation with the",
	"parameter values be delayed until reliable system operation is",
	"established. To use the standard parameters, answer yes<RETURN>",
	"or just <RETURN>.",
	"",
	"To change the parameters, answer no<RETURN>. Sysgen will ask for",
	"the value of each parameter. Sysgen will also print the standard",
	"value of each parameter, enclosed in < >.",
	"",
	"The parameters may be changed depending on the number of ",
	"applications using file and record locking and the number of",
	"locks required by these applications",
	-1,
	"The file lock parameters are:",
	"",
	"Param	  OV_VAL   ID_VAL    COST       Description",
	"-----	  ------   ------    ----	-----------",
	"FLCKREC     50	     100       20       Maximum number of record",
	"					locks in the system",
	"FLCKFIL     15	      20       12	Maximum number of file",
	"					locks in the system",
	0
};

char	*sm_max[] =
{
	"",
	"Msgmax is the maximum size of a message. ",
	"",
	0
};

char	*sm_mnb[] =
{
	"",
	"Msgmnb specifies the maximum number of bytes on a",
	"message queue. ",
	"",
	0
};

char	*sm_tql[] =
{
	"",
	"Msgtql specifies the number of system message",
	"headers, which is the maximum number of outstanding",
	"messages allowed on the system.",
	"",
	0
};

char	*sm_ssz[] =
{
	"",
	"Msgssz specifies the message segment size. A message",
	"consists of a set of contiguous message segments",
	"large enough to fit the text. The segments are used",
	"to help eliminate fragmentation and speed message buffer",
	"allocation. A message may span several segments.",
	"The segment size should be a multiple of two.",
	"The amount of memory allocated for messages outside the kernel",
	"is equal to (Msgssz * Msgseg)",
	"",
	0
};

char	*sm_seg[] =
{
	"",
	"Msgseg specifies the number of message segments in the",
	"system. The amount of memory allocated for messages outside the ",
	"kernel is equal to (Msgssz * Msgseg)",
	"",
	0
};

char	*sm_map[] =
{
	"",
	"Msgmap specifies the number of entries in the message map.",
	"The map is used by the system to allocate and free message segments",
	"",
	0
};

char	*sm_mni[] =
{
	"Msgmni specifies the maximum number of message queues",
	"system wide.",
	"",
	0
};

char	*ss_map[] =
{
	"",
	"Semmap specifies the number of entries in the semaphore",
	"map. The map is used by the system to allocate and free",
	"semaphore sets. This parameter should be changed to",
	"reflect changes in semmns.",
	"",
	0
};

char	*ss_mni[] =
{
	"",
	"Semmni specifies the number of semaphore identifiers, which",
	"is the number of semaphore sets.",
	"",
	0
};

char	*ss_mns[] =
{
	"",
	"Semmns specifies the number of semaphores in the system.",
	"",
	0
};

char	*ss_mnu[] =
{
	"",
	"Semmnu specifies the number of undo structures in the system.",
	"",
	0
};

char	*ss_ume[] =
{
	"",
	"Semume specifies the maximum number of undo entries per",
	"process.",
	"",
	0
};

char	*ss_msl[] =
{
	"",
	"Semmsl specifies the maximum number of semaphores per",
	"semaphore identifier.",
	"",
	0
};

char	*ss_opm[] =
{
	"",
	"Semopm specifies the maximum number of semaphore operations per",
	"semop(2) call.",
	"",
	0
};

char	*sf_recf[] =
{
	"",
	"Flckrec specifies the maximum number of records which can be locked ",
	"in the system at one time",
	"",
	0
};


char	*sf_filf[] =
{
	"",
	"Flckfil specifies the maximum number of files which can be locked",
	"in the system at one time",
	"",
	0
};

char	*sg_pty[] =
{
	"",
	"Pseudo TTYs allow a program to communicate with the system as if",
	"it was a terminal. The operating system requires a minimum of two",
	"pseudo TTYs. If you are using TCP/IP networking, the pseudo TTYs",
	"have the additional function of allowing users on remote systems",
	"to log in to your system. In other words, the number of pseudo",
	"TTYs limits the number of remote logins to your system.",
	"",
	"The system can support a large number of pseudo TTYs, however,",
	"you should not specify more than you really need. This is because",
	"pseudo TTYs consume operating system resources in the same manner",
	"as a real TTY would.",
	"",
	0
};

char	*sg_network[] =
{
	"",
	"If your system will be connected to a local area network via an",
	"ethernet, you must include the TCP/IP networking support code.",
	"This support code allows the operating system to communicate over",
	"the ethernet using a DEQNA or DEUNA ethernet interface.",
	"",
	"The TCP/IP support code is very large (50 - 60 Kbytes), so, you",
	"should not include it unless you really need ethernet support.",
	"",
	0
};

char	*sg_netdev[] =
{
	"",
	"If you included the TCP/IP networking support code, you need to",
	"specify the type of ethernet interface you will be using, that is,",
	"the DEQNA or DEUNA. The DEQNA is used with Q bus processors, such",
	"as the 11/23 or 11/73. The DEUNA is used with UNIBUS processors,",
	"such as the 11/44 or 11/84. The DEQNA is one dual height module,",
	"the DEUNA consists of two hex height modules. If you are using",
	"some other interface to which you have written a driver, it will",
	"be called either `n1' or `n2'.",
	"",
	0
};

char	*sg_netn[] =
{
	"",
	"Maximum of two DEQNAs and four DEUNAs are supported. In addition,",
	"two other interfaces for which you have written a driver (n1/n2) are",
	"supported",
	"",
	0
};

char	*sg_netv[] =
{
	"",
	"When configuring a user supplied network driver, the number of",
	"interrupt vectors used by the device must be specified. This",
	"will be either 2 or 1; some devices have separate transmit and",
	"receive interrupt vectors, other devices use one interrupt vector",
	"for both transmit and receive.",
	"",
	0
};

char	*sg_netpar[] =
{
	"",
	"The TCP/IP support code requires a significant number of buffers",
	"to handle ethernet send and receive packets. Sysgen has built in",
	"standard values for the amount of buffering required. These values",
	"are based on the type of processor, that is, split I/D space or",
	"non split I/D space.",
	"",
	"For your initial sysgen, you should use the standard values. Over",
	"time you can use the netstat command to monitor the usage of the",
	"network buffers, and adjust the number of buffers accordingly.",
	"",
	0
};

char	*sn_mbufs[] =
{
	"",
	"Mbufs, or message buffers, are used to buffer packets waiting to",
	"be sent over the ethernet and packets received over the ethernet",
	"waiting to be sent to the networking software. A packet can span",
	"more than one mbuf. The standard number of mubfs should be correct",
	"for most systems. Initially, you should use the default number of",
	"mubfs, then use the netstat command to monitor mbuf usage. The",
	"netstat command will help you determine if the number of mbufs",
	"need to be changed.",
	"",
	0
};

char	*sn_allocs[] =
{
	"",
	"Allocs are general data structures used by the networking support",
	"code. Allocs are used for: mbuf pointers, routing table entries,",
	"and socket structures. Initially, you should use the default value",
	"for allocs, then use the netstat command to monitor general network",
	"storage allocation usage. This will help you determine if allocs",
	"need to be increased, decreased, or remain unchanged.",
	"",
	0
};


struct	sghelp	sghelp[] =
{
	"sg_c",		&sg_c,
/*	"sg_cbsiz",	&sg_cbsiz, */
	"sg_cmn",	&sg_cmn,
	"sg_cmt",	&sg_cmt,
	"sg_cn",	&sg_cn,
	"sg_cpu",	&sg_cpu,
	"sg_memsz",	&sg_memsz,
	"sg_nomap",	&sg_nomap,
	"sg_csr",	&sg_csr,
	"sg_d",		&sg_d,
	"sg_dc",	&sg_dc,
	"sg_ddt",	&sg_ddt,
	"sg_dn",	&sg_dn,
	"sg_dst",	&sg_dst,
	"sg_fpsim",	&sg_fpsim,
	"sg_hz",	&sg_hz,
	"sg_i",		&sg_i,
	"sg_l",		&sg_l,
	"sg_m",		&sg_m,
	"sg_mapsz",	&sg_mapsz,
	"sg_ulimit",	&sg_ulimit,
/*	"sg_mbcn",	&sg_mbcn,	*/
	"sg_md",	&sg_md,
	"sg_mseg",	&sg_mseg,
	"sg_msgb",	&sg_msgb,
	"sg_mtc",	&sg_mtc,
	"sg_cdd",	&sg_cdd,
	"sg_mtn",	&sg_mtn,
	"sg_muprc",	&sg_muprc,
	"sg_nb",	&sg_nb,
	"sg_nbuf",	&sg_nbuf,
	"sg_ncall",	&sg_ncall,
	"sg_ncargs",	&sg_ncargs,
	"sg_nclist",	&sg_nclist,
	"sg_nfile",	&sg_nfile,
	"sg_ninode",	&sg_ninode,
	"sg_nmnt",	&sg_nmnt,
	"sg_nproc",	&sg_nproc,
	"sg_ntext",	&sg_ntext,
	"sg_p",		&sg_p,
	"sg_r",		&sg_r,
	"sg_s",		&sg_s,
	"sg_sb",	&sg_sb,
	"sg_sd",	&sg_sd,
	"sg_sdn",	&sg_sdn,
	"sg_sh",	&sg_sh,
	"sg_sl",	&sg_sl,
	"sg_sp",	&sg_sp,
	"sg_tz",	&sg_tz,
	"sg_udev",	&sg_udev,
	"sg_vec",	&sg_vec,
	"sg_shuff",	&sg_shuff,
	"sg_mesg",	&sg_mesg,
	"sg_mgpar",	&sg_mgpar,
	"sg_sema",	&sg_sema,
	"sg_smpar",	&sg_smpar,
	"sg_flock",	&sg_flock,
	"sg_flkpar",	&sg_flkpar,
	"sg_maus",	&sg_maus,
	"sg_mspar",	&sg_mspar,
	"sm_max",	&sm_max,
	"sm_mnb",	&sm_mnb,
	"sm_tql",	&sm_tql,
	"sm_ssz",	&sm_ssz,
	"sm_seg",	&sm_seg,
	"sm_map",	&sm_map,
	"sm_mni",	&sm_mni,
	"ss_map",	&ss_map,
	"ss_mni",	&ss_mni,
	"ss_mns",	&ss_mns,
	"ss_mnu",	&ss_mnu,
	"ss_ume",	&ss_ume,
	"ss_msl",	&ss_msl,
	"ss_opm",	&ss_opm,
	"sf_recf",	&sf_recf,
	"sf_filf",	&sf_filf,
	"ms_nmaus",	&ms_nmaus,
	"ms_maus",	&ms_maus,
	"sg_pty",	&sg_pty,
	"sg_network",	&sg_network,
	"sg_netdev",	&sg_netdev,
	"sg_netpar",	&sg_netpar,
	"sg_netn",	&sg_netn,
	"sg_netv",	&sg_netv,
	"sn_mbufs",	&sn_mbufs,
	"sn_allocs",	&sn_allocs,
	0
};
