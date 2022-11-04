
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#
static char *sccsid = "@(#)message.c	3.0	4/21/86";
/*
 *
 *	UNIX debugger
 *
 */

#include	"defs.h"

MSG		version =	"Version 2.0 10/31/83\n";
MSG		BADMOD	=	"bad modifier";
MSG		BADCOM	=	"bad command";
MSG		BADSYM	=	"symbol not found";
MSG		BADLOC	=	"automatic variable not found";
MSG		NOCFN	=	"c routine not found";
MSG		NOMATCH	=	"cannot locate value";
MSG		NOBKPT	=	"no breakpoint set";
MSG		BADKET	=	"unexpected ')'";
MSG		NOADR	=	"address expected";
MSG		NOPCS	=	"no process";
MSG		BADVAR	=	"bad variable";
MSG		BADTXT	=	"text address not found";
MSG		BADDAT	=	"data address not found";
MSG		ODDADR	=	"odd address";
MSG		EXBKPT	=	"too many breakpoints";
MSG		A68BAD	=	"bad a68 frame";
MSG		A68LNK	=	"bad a68 link";
MSG		ADWRAP	=	"address wrap around";
MSG		BADEQ	=	"unexpected `='";
MSG		BADWAIT	=	"wait error: process disappeared!";
MSG		ENDPCS	=	"process terminated";
MSG		NOFORK	=	"try again";
MSG		BADSYN	=	"syntax error";
MSG		NLERR	=	"newline expected";
MSG		SZBKPT	=	"bkpt: command too long";
MSG		BADFIL	=	"bad file format";
MSG		SLOINI	=	"slow initialize. Please wait.";
MSG		BADNAM	=	"not enough space for symbols";
MSG		LONGFIL	=	"filename too long";
MSG		NOTOPEN	=	"cannot open";
MSG		ADB_BADMAG =	"bad core magic number";
MSG		ALTMAP	=	"mapping for memory or core dump";
MSG		NOTOV	=	"not an overlay file";
MSG		NOCORE	=	"no core file";
MSG		BADOVN	=	"bad overlay number";



/* instruction printing */

#define	DOUBLE	0
#define DOUBLW	1
#define	SINGLE	2
#define SINGLW	3
#define	REVERS	4
#define	BRANCH	5
#define	NOADDR	6
#define	DFAULT	7
#define	TRAP	8
#define	SYS	9
#define	SOB	10
#define JMP	11
#define JSR	12

struct optab {
	int	mask;
	int	val;
	int	itype;
	char	*iname;
} 
optab[] = {
	0107777, 0010000, DOUBLE, "mov",
	0107777, 0020000, DOUBLE, "cmp",
	0107777, 0030000, DOUBLE, "bit",
	0107777, 0040000, DOUBLE, "bic",
	0107777, 0050000, DOUBLE, "bis",
	0007777, 0060000, DOUBLE, "add",
	0007777, 0160000, DOUBLE, "su",
	0100077, 0005000, SINGLE, "clr",
	0100077, 0005100, SINGLE, "com",
	0100077, 0005200, SINGLE, "inc",
	0100077, 0005300, SINGLE, "dec",
	0100077, 0005400, SINGLE, "neg",
	0100077, 0005500, SINGLE, "adc",
	0100077, 0005600, SINGLE, "sbc",
	0100077, 0005700, SINGLE, "tst",
	0100077, 0006000, SINGLE, "ror",
	0100077, 0006100, SINGLE, "rol",
	0100077, 0006200, SINGLE, "asr",
	0100077, 0006300, SINGLE, "asl",
	0000077, 0000100, JMP,	"jmp",
	0000077, 0000300, SINGLE, "swab",
	0000077, 0170100, SINGLW, "ldfps",
	0000077, 0170200, SINGLW, "stfps",
	0000077, 0170300, SINGLW, "stst",
	0000077, 0170400, SINGLW, "clrf",
	0000077, 0170500, SINGLW, "tstf",
	0000077, 0170600, SINGLW, "absf",
	0000077, 0170700, SINGLW, "negf",
	0000077, 0006700, SINGLW, "sxt",
	0000077, 0006600, SINGLW, "mtpi",
	0000077, 0106600, SINGLW, "mtpd",
	0000077, 0006500, SINGLW, "mfpi",
	0000077, 0106500, SINGLW, "mfpd",
	0000777, 0070000, REVERS, "mul",
	0000777, 0071000, REVERS, "div",
	0000777, 0072000, REVERS, "ash",
	0000777, 0073000, REVERS, "ashc",
	0377,	0000400, BRANCH, "br",
	0377,	0001000, BRANCH, "bne",
	0377,	0001400, BRANCH, "beq",
	0377,	0002000, BRANCH, "bge",
	0377,	0002400, BRANCH, "blt",
	0377,	0003000, BRANCH, "bgt",
	0377,	0003400, BRANCH, "ble",
	0377,	0100000, BRANCH, "bpl",
	0377,	0100400, BRANCH, "bmi",
	0377,	0101000, BRANCH, "bhi",
	0377,	0101400, BRANCH, "blos",
	0377,	0102000, BRANCH, "bvc",
	0377,	0102400, BRANCH, "bvs",
	0377,	0103000, BRANCH, "bcc",
	0377,	0103400, BRANCH, "bcs",
	0000000, 0000000, NOADDR, "halt",
	0000000, 0000001, NOADDR, "wait",
	0000000, 0000002, NOADDR, "rti",
	0000000, 0000003, NOADDR, "bpt",
	0000000, 0000004, NOADDR, "iot",
	0000000, 0000005, NOADDR, "reset",
	0377,	0171000, REVERS, "mulf",
	0377,	0171400, REVERS, "modf",
	0377,	0172000, REVERS, "addf",
	0377,	0172400, REVERS, "movf",
	0377,	0173000, REVERS, "subf",
	0377,	0173400, REVERS, "cmpf",
	0377,	0174000, DOUBLW, "movf",
	0377,	0174400, REVERS, "divf",
	0377,	0175000, DOUBLW, "movei",
	0377,	0175400, DOUBLW, "movfi",
	0377,	0176000, DOUBLW, "movfo",
	0377,	0176400, REVERS, "movie",
	0377,	0177000, REVERS, "movif",
	0377,	0177400, REVERS, "movof",
	0000000, 0170000, NOADDR, "cfcc",
	0000000, 0170001, NOADDR, "setf",
	0000000, 0170002, NOADDR, "seti",
	0000000, 0170011, NOADDR, "setd",
	0000000, 0170012, NOADDR, "setl",
	0000777, 0004000, JSR,	"jsr",
	0000777, 0074000, DOUBLE, "xor",
	0000007, 0000200, SINGLE, "rts",
	0000017, 0000240, DFAULT, "cflg",
	0000017, 0000260, DFAULT, "sflg",
	0377,	0104000, TRAP,	"emt",
	0377,	0104400, SYS,	"sys",
	0000077, 0006400, TRAP,	"mark",
	0000777, 0077000, SOB,	"sob",
	0000007, 0000230, TRAP,	"spl",
	0177777, 0000000, DFAULT, "",
};

struct systab {
	int	argc;
	char	*sname;
	int	argtyp[5];
} 
systab[] = {
	{ 1, "indir",	DSYM, },
	{ 0, "exit", },
	{ 0, "fork", },
	{ 2, "read",	NSYM,	DSYM, },
	{ 2, "write",	NSYM,	DSYM, },
	{ 2, "open",	NSYM,	DSYM, },
	{ 0, "close", },
	{ 0, "wait",	DSYM, },
	{ 2, "creat",	NSYM,	DSYM, },
	{ 2, "link",	DSYM,	DSYM, },
	{ 1, "unlink",	DSYM, },
	{ 2, "exec",	DSYM,	DSYM, },
	{ 1, "chdir",	DSYM, },
	{ 0, "time", },
	{ 3, "mknod",	NSYM,	NSYM,	DSYM, },
	{ 2, "chmod",	NSYM,	DSYM, },
	{ 3, "chown",	NSYM,	NSYM,	DSYM, },
	{ 1, "break",	NSYM, },
	{ 2, "stat",	DSYM,	DSYM, },
	{ 3, "seek",	NSYM,	NSYM,	NSYM, },
	{ 0, "getpid", },
	{ 3, "mount",	DSYM,	DSYM,	NSYM, },
	{ 1, "umount",	DSYM, },
	{ 0, "setuid", },
	{ 0, "getuid", },
	{ 0, "stime", },
	{ 4, "ptrace",	NSYM,	NSYM,	DSYM,	NSYM, },
	{ 0, "alarm", },
	{ 1, "fstat",	DSYM, },
	{ 0, "pause", },
	{ 2, "utime",	NSYM,	DSYM, },
	{ 1, "stty",	DSYM, },
	{ 1, "gtty",	DSYM, },
	{ 2, "access",	NSYM,	DSYM, },
	{ 0, "nice", },
	{ 1, "ftime",	DSYM, },
	{ 0, "sync", },
	{ 1, "kill",	NSYM, },
	{ 0, "38", },
	{ 1, "setpgrp",	NSYM, },
	{ 0, "40", },
	{ 0, "dup", },
	{ 0, "pipe", },
	{ 1, "times",	DSYM, },
	{ 4, "profil",	NSYM,	NSYM,	NSYM,	DSYM, },
	{ 0, "45", },
	{ 0, "setgid", },
	{ 0, "getgid", },
	{ 2, "signal",	2,	NSYM, },
	{ 5, "msgsys",	NSYM,	NSYM,	NSYM,	NSYM,	NSYM, },
	{ 0, "50", },
	{ 1, "sysacct",	DSYM, },
	{ 3, "sysphys",	NSYM,	NSYM,	NSYM, },
	{ 1, "syslock",	NSYM, },
	{ 3, "ioctl",	DSYM,	NSYM,	NSYM, },
	{ 0, "55", },
	{ 0, "56", },
	{ 1, "utssys",	NSYM, },
	{ 1, "Berkeley indir",	DSYM, },
	{ 3, "exece",	DSYM,	DSYM,	DSYM, },
	{ 1, "umask",	NSYM, },
	{ 1, "chroot",	DSYM, },
	{ 2, "fcntl",	NSYM,	NSYM, },
	{ 3, "ulimit",	NSYM,	NSYM,	NSYM, },
	{ 4, "evntflg",	NSYM,	NSYM,	NSYM,	NSYM, },
	{ 2, "getfp",	NSYM,	NSYM, },
	{ 2, "ttlocl",	NSYM,	NSYM, },
	{ 2, "errlog",	NSYM,	NSYM, },
	{ 1, "bdflush",	NSYM, },
	{ 0, "zaptty", },
	{ 1, "fpsim",	NSYM, },
	{ 1, "nap",	NSYM, },
	{ 0, "72", },
	{ 0, "73", },
	{ 0, "74", },
	{ 0, "75", },
	{ 0, "76", },
	{ 0, "77", },
	{ 1, "maus",	NSYM, },
	{ 3, "semsys",	NSYM,	NSYM,	NSYM, },
	{ 0, "80", },
	{ 3, "login",	NSYM,	NSYM,	NSYM, },
	{ 2, "lstat",	NSYM,	NSYM, },
	{ 1, "submit",	NSYM, },
	{ 0, "nostk", },
	{ 2, "killbkg",	NSYM,	NSYM, },
	{ 2, "killpg",	NSYM,	NSYM, },
	{ 2, "renice",	NSYM,	NSYM, },
	{ 1, "fetchi",	NSYM, },
	{ 4, "ucall",	NSYM,	NSYM,	NSYM,	NSYM, },
	{ 5, "quota",	NSYM,	NSYM,	NSYM,	NSYM,	NSYM, },
	{ 2, "qfstat",	NSYM,	NSYM, },
	{ 2, "qstat",	NSYM,	NSYM, },
	{ 0, "setpgrp", },
	{ 1, "gldav",	NSYM, },
	{ 0, "fperr", },
	{ 0, "vhangup", },
	{ 0, "97", },
	{ 4, "select",	NSYM,	DSYM,	DSYM,	DSYM, },
	{ 2, "gethostname",	DSYM,	NSYM, },
	{ 2, "sethostname",	DSYM,	NSYM, },
	{ 3, "socket",	NSYM,	NSYM,	NSYM, },
	{ 3, "bind",	NSYM,	DSYM,	NSYM, },
	{ 2, "listen",	NSYM,	NSYM, },
	{ 3, "accept",	NSYM,	DSYM,	DSYM, },
	{ 3, "connect",	NSYM,	DSYM,	NSYM, },
	{ 5, "socketpair",	NSYM,	NSYM,	NSYM,	NSYM,	NSYM, },
	{ 0, "setreuid", },
	{ 0, "setregid", },
	{ 0, "109", },
	{ 0, "110", },
	{ 0, "gethostid", },
	{ 2, "sethostid",	NSYM,	NSYM, },
	{ 5, "setsockopt",	NSYM,	NSYM,	NSYM,	DSYM,	NSYM, },
	{ 5, "getsockopt",	NSYM,	NSYM,	NSYM,	DSYM,	DSYM, },
	{ 3, "getsockname",	NSYM,	DSYM,	DSYM, },
	{ 3, "getpeername",	NSYM,	DSYM,	DSYM, },
	{ 2, "shutdown",	NSYM,	NSYM, },
	{ 5, "sendit",	NSYM,	DSYM,	NSYM,	NSYM,	DSYM, },
	{ 5, "recvit",	NSYM,	DSYM,	NSYM,	NSYM,	DSYM, },
	{ 0, "120", },
	{ 0, "121", },
};

/*
 * Text used to print the cause of a user
 * process being core dumpped.
 */
char *traptypes[] = {
	"(4) - Bus error",
	"(10) - Reserved instruction",
	"(14) - Breakpoint trace",
	"(20) - Input/output trap",
	"(24) - Power fail",
	"(30) - Emulator trap",
	"(34) - Trap instruction",
	"(240) - Programmed interrupt request",
	"(244) - Floating point exception",
	"(250) - Memory management violation",
	"(114) - Memory/unibus parity error",
};

char *signals[] = {
	"",
	"hangup",
	"interrupt",
	"quit",
	"illegal instruction",
	"trace/BPT",
	"IOT",
	"EMT",
	"floating exception",
	"killed",
	"bus error",
	"memory fault",
	"bad system call",
	"broken pipe",
	"alarm call",
	"terminated",
};
