
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)sg_str.c	3.2	8/6/87";
/*
 * Strings used by sysgen
 *
 * Fred Canter
 */

char	*sg_help[] =
{
	"",
	"The \"sysgen>\" prompt indicates the sysgen program is ready to",
	"accept commands. To execute a command you type the first letter",
	"of the command, then press <RETURN>. Some of the commands will",
	"ask you for additional information, such as a file name. For",
	"more help with a command, type h followed by the command letter",
	"then press <RETURN>. For example, \"h c\" for the create command.",
	"",
	"Command    Description",
	"-------    -----------",
	"<CTRL/D>   Exit from sysgen (backup one question in \"c\" command).",
	"<CTRL/C>   Cancel current command, return to \"sysgen>\" prompt.",
	"!command   Escape to the shell and execute \"command\".",
	"[c]reate   Create an ULTRIX-11 kernel configuration file.",
	"[r]emove   Remove an existing configuration file.",
	"[l]ist     List names of all existing configuration files.",
	"[p]rint    Print a configuration file.",
	"[m]ake     Make the ULTRIX-11 kernel.",
	"[i]nstall  Print instructions for installing the new kernel.",
	"[d]evice   List configurable processors and peripherals.",
	"[s]ource   Compile and archive a source code module (u1.c, etc.).",
	"",
	"The sysgen sequence is: use \"c\" to create the configuration file,",
	"\"m\" to make the new kernel, and \"i\" for installation instructions.",
	0
};

char	*install[] =
{
	"",
	"Use the following procedure to install the new kernel:",
	"",
	"  o  Type <CTRL/D> to exit from the sysgen program.",
	"",
	"  o  Become superuser (type \"su\", then enter the root password).",
	"",
	"  o  Move the new kernel to the root (mv unix.os /nunix).",
	"",
	"  o  Type <CTRL/D> twice (to logout), then login to the operator",
	"     account. Use the operator services \"s\" command to shutdown",
	"     the system and \"halt\" command to halt the processor.",
	"",
	"  o  Use the manual boot procedure, described in section 3.4 of",
	"     the ULTRIX-11 System Management Guide, to boot the new kernel.",
	"     For example, \"Boot: rl(0,0)nunix\" from an RL02 disk.",
	"",
	"  o  Save the old kernel then rename the new kernel \"unix\".",
	"     (mv unix ounix; mv nunix unix; chmod 644 unix)",
	"",
	"  o  Set the date (date command), check the file systems (fsck),",
	"     then type <CTRL/D> to enter multi-user mode.",
	0
};

char	*devlist[] =
{
	"",
	"Memory managed PDP-11 processors:",
	"",
	"\t(23, 23+, 24, 34, 40, 44, 45, 55, 60, 70, 73, 83, 84)",
	"",
	"Disk Controllers:",
	"",
	"Number\tName\tDescription",
	"------\t----\t-----------",
	"1\thp\t(first)  RH11/RH70 - 8 RM02/3/5, RP04/5/6, ML11",
	"1\thm\t(second) RH11/RH70 - 8 RM02/3/5, RP04/5/6, ML11",
	"1\thj\t(third)  RH11/RH70 - 8 RM02/3/5, RP04/5/6, ML11",
	"1\thk\tRK611/RK711 with up to 8 RK06/7",
	"1\tra\tUDA50/KDA50 with up to 4 RA80/RA81/RA60",
	"1\trc\tKLESI with up to 2 RC25 (4 units)",
	"1\trd/rx\tRQDX1/2/3 with up to 4 (RD31-32, RD51-54, RX50/RX33)",
	"1\trx\tRUX1 with up to 4 RX50",
	"1\trp\tRP11 with up to 8 RP02/3",
	"1\trl\tRL11 with up to 4 RL01/2",
	"1\trk\tRK11 with up to 8 RK05",
	"1\thx\tRX211 with one dual RX02 drive",
	-1,
	"Magtape Controllers:",
	"",
	"Number\tName\tDescription",
	"------\t----\t-----------",
	"1\tht\tTM02/3 with up to 64 TU16/TE16/TU77",
	"1\ttm\tTM11 with up to 8 TU10/TE10/TS03",
	"4\tts\tSingle TS11/TSV05/TSU05/TU80",
	"4\ttk\tSingle TK50/TU81",
	"",
	"Miscellaneous Devices:",
	"",
	"Number\tName\tDescription",
	"------\t----\t-----------",
	"1\tlp\tLP11/LPV11 controller with 1 LP11/LPV11 type printer",
	"1\tct\tC/A/T phototypesetter interface via DR11-C",
	-1,
	"Communications Devices:",
	"",
	"Number\tName\tDescription",
	"------\t----\t-----------",
	"8\tdh\tDH11 16 line asynchronous multiplexer",
	"8\tdhdm\tDM11-BB modem control option for DH11",
	"8\tdhu\tDHU11 16 line asynchronous multiplexer",
	"4\tdhv\tDHV11 8 line asynchronous multiplexer",
	"16\tdz\tDZ11 8 line asynchronous multiplexer",
	"8\tdzv\tDZV11 4 line asynchronous multiplexer",
	"8\tdzq\tDZQ11 4 line asynchronous multiplexer",
	"16\tkl\tDL11/DLV11 single line unit (CSR 776500)",
	"32\tdl\tDL11/DLV11 single line unit (CSR 775610)",
	"4\tdu\tDU11 single line synchronous interface",
	"1\tdn\tDN11 4 line auto call unit interface",
	0
};

char	*wm_q22[] =
{
	"",
	"\7\7\7\t     ****** CAUTION ******",
	"",
	"This device does not support 22 bit addressing.",
	"Use of this device with a Q22 bus processor is",
	"restricted, that is, it may only be accessed via",
	"the BUFFERED I/O mechanism. Attempting RAW I/O",
	"transfers will causes nonexistent memory errors.",
	"",
	0
};

char	*wm_ml11[] =
{
	"",
	"\7\7\7\t     ****** CAUTION ******",
	"",
	"You have specified an ML11 as the system disk,",
	"this is not recommended because the ML11 is a",
	"solid state disk. Data stored on the ML11 disk",
	"is volatile, that is, all data is lost when power",
	"is removed.",
	"",
	"Digital recommends you use the ML11 as the swap",
	"device or as a file system mounted on /tmp.",
	"",
	0
};

char	*sg_mtu[] =
{
	"",
	"Sysgen is requesting the tape drive unit number for the tk/ts type",
	"magtape controller. Enter the tape unit number, then press",
	"<RETURN>. There is no default unit number. After same type of",
	"tk/ts magtape drives are configured, press <RETURN> for next magtape",
	"controller.",
	"",
	"Sysgen asks for the unit number instead of the controller number",
	"because each tk/ts controller can have single unit only and the",
	"software driver will use unit number as controller number.",
	"",
	"Sysgen will keep on asking for the unit number that allows the",
	"same type of tk/ts magtape can be configured without repeating the",
	"typing of the magtape name and also allows the unit be configured",
	"non-sequentially.",
	"",
	0
};

char	*sg_dstarea[] =
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
