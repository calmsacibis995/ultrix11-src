
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)mkc_str.c	3.0	4/21/86";

char	*stra40[] =
{
	"/ low core",
	"",
	0
};

char	*stra70[] =
{
	"/ low core",
	"",
	".data",
	0
};

char	*stra1[] =
{
	"ZERO:",
	"",
	"br4 = 200",
	"br5 = 240",
	"br6 = 300",
	"br7 = 340",
	"",
	". = ZERO+0",
	0
};

/*
 * WARNING WARNING WARNING WARNING WARNING
 *
 * DO NOT change the following string without
 * first checking /usr/src/cmd/cc.c, first
 * lines of code after main().
 * Location zero identifies the type of kernel.
 */
char	*stra2id[] =
{
	"	svi0; br7+0.		/ stray vector thru zero logger",
	"				/ loc 0 = 4, says SID kernel",
	0
};

/*
 * WARNING WARNING WARNING WARNING WARNING
 *
 * DO NOT change the following string without
 * first checking /usr/src/cmd/cc.c, first
 * lines of code after main().
 * Location zero identifies the type of kernel.
 */
char	*stra2ov[] = 
{
	"	3; br7+0.		/ BPT to catch jump/vector -> 0",
	"				/ loc 0 = 3, says OV kernel",
	0
};

char	*stra3[] =
{
	"",
	"/ trap vectors",
	"	trap; br7+0.		/ bus error",
	"	trap; br7+1.		/ illegal instruction",
	"	trap; br7+2.		/ bpt-trace trap",
	"	trap; br7+3.		/ iot trap",
	"	trap; br7+4.		/ power fail",
/*@
	"	trap; br7+5.		/ emulator trap",
@*/
/*@*/	"	ovint; br5+5.		/ emulator trap",
	"	start;br7+6.		/ system  (overlaid by 'trap')",
	"",
	". = ZERO+40",
	"",
	0,
};

char	*strb[] =
{
	"",
	". = ZERO+240",
	"	trap; br7+7.		/ programmed interrupt",
	"	trap; br7+8.		/ floating point",
	"	trap; br7+9.		/ segmentation violation",
	0
};

char	*strc[] =
{
	"",
	"/ floating vectors",
	"",
	". = ZERO+300",
	0,
};

/*
char	*osizcmd = {"size ../ovsys/*.o ../ovdev/*.o ../ovnet/*.o > text.sizes"};
char	*ssizcmd = "size ../sys/LIB1_id ../dev/LIB2_id ../net/*.o > text.sizes";
*/
char	*osizcmd = "(cd ../ovsys; size *.o; cd ../ovdev; size *.o;\
			cd ../ovnet; size *.o)> text.sizes";
char	*ssizcmd = "(cd ../sys; size LIB1_id; size *.o; cd ../dev;\
			size LIB2_id; cd ../net; size *.o )> text.sizes";
char	*cmcmd = {"chmod 744 ovload"};

char	*strovh[] =
{
	"ld -X -k -n -o unix_ov l.o dump_ov.o mch_ov.o c_ov.o dds_ov.o tds_ov.o nds_ov.o dksizes_ov.o \\",
	0,
};

char	*strovh1[] =
{	"ld -X -k -i -o unix_id l.o dump_id.o mch_id.o c.o dds.o tds.o nds.o dksizes.o \\",
	0
};

char	*strovl[] =
{
	"-L \\",
	"\t../ovsys/trap.o \\",
	"\t../ovsys/slp.o \\",
/*	"\t../ovsys/rdwri.o \\", */
	"\t../ovsys/clock.o \\",
/*	"\t../ovsys/alloc.o \\", */
	"\t../ovsys/sysent.o \\",
	"\t../ovsys/prim.o \\",
	"\t../ovsys/prf.o \\",
/*	"\t../ovsys/pipe.o \\",	*/
	"\tec.o",
	0,
};

char	*strovl1[] =
{
	"-L \\",
	"\t../sys/LIB1_id \\",
	"\t../dev/LIB2_id \\",
	"\tec.o",
	0
};

char	*strovz[] =
{
	"-Z \\",
	0,
};

char	*strd1[] =
{
	"",
	"/ Interface to stray vector error logging",
	"",
	".text",
	".globl _sv0, _sv100, _sv200, _sv300, _sv400, _sv500, _sv600, _sv700",
	"",
	0
};

char	*strd2[] =
{
	"/ BPT required for jump/vector -> zero error handling.",
	"3\t/ BPT - DO NOT MOVE !",
	"0\t/ Place holder, DO NOT MOVE !",
	"",
	0
};

char	*strd3[] =
{
	"svi0:\tjsr\tr0,call; jmp _sv0\t/stray vector from 0-74",
	"svi100:\tjsr\tr0,call; jmp _sv100",
	"svi200:\tjsr\tr0,call; jmp _sv200",
	"svi300:\tjsr\tr0,call; jmp _sv300",
	"svi400:\tjsr\tr0,call; jmp _sv400",
	"svi500:\tjsr\tr0,call; jmp _sv500",
	"svi600:\tjsr\tr0,call; jmp _sv600",
	"svi700:\tjsr\tr0,call; jmp _sv700",
	"",
	"//////////////////////////////////////////////////////",
	"/		interface code to C",
	"//////////////////////////////////////////////////////",
	"",
	".globl	call, trap",
	0
};

char	*strd4[] = 
{
	"",
	"/ Dummy instructions to force loading of certain data",
	"/ structures first in BSS, so they can be mapped by the",
	"/ first unibus map register (_ub_end used for size check).",
	"",
	".globl _cfree",
	"",
	"\tmov\t_cfree,r0\t/ WARNING: _cfree must be first",
	"",
	0
};

char	*stre[] =
{
	"#include <sys/param.h>",
	"#include <sys/systm.h>",
	"#include <sys/buf.h>",
	"#include <sys/tty.h>",
	"#include <sys/conf.h>",
	"#include <sys/proc.h>",
	"#include <sys/text.h>",
	"#include <sys/dir.h>",
	"#include <sys/user.h>",
	"#include <sys/file.h>",
	"#include <sys/inode.h>",
	"#include <sys/acct.h>",
	"#include <sys/mount.h>",
	"#include <sys/map.h>",
	"#include <sys/callo.h>",
	"#include <sys/mbuf.h>",
	"#include \"dds.h\"",
	"",
	"int	nulldev();",
	"int	nodev();",
	0
};

char	*stre1[] =
{
	"struct	bdevsw	bdevsw[] =",
	"{",
	0,
};

char	*strf[] =
{
	"	0",
	"};",
	"",
	0,
};

char	*strf1[] =
{
	"",
	"int ttselect(), seltrue();",
	"",
	"struct	cdevsw	cdevsw[] =",
	"{",
	0,
};

char	strg[] =
{
"	0\n\
};\n\
int	rootdev	= makedev(%d, %d);\n\
int	swapdev	= makedev(%d, %d);\n\
int	pipedev = makedev(%d, %d);\n\
int	el_dev = makedev(%d, %d);\n\
int	nldisp = %d;\n\
daddr_t	swplo	= %ld;\n\
int	nswap	= %u;\n\
daddr_t	el_sb = %ld;\n\
int	el_nb = %u;\n\
"};

char	strg1[] =
{
"	\n\
struct	buf	buf[NBUF];\n\
struct	inode	inode[%d];\n\
struct	mount	mount[%d];\n"
};

char	strg2[] =
{
"struct	buf	bfreelist;\n\
struct	acct	acctbuf;\n\
struct	inode	*acctp;\n\
struct	map	coremap[%d];\n\
struct	map	swapmap[%d];\n\
int	ub_end;\n\
#ifndef	UCB_CLIST\n\
struct	cblock	cfree[%d];\n\
#else	UCB_CLIST\n\
struct	cblock	*cfree;\n\
#endif	UCB_CLIST\n\
char	msgbuf[%d];\n"
};

char	strg3[] =
{
"	\n\
/*\n\
 * System table sizes, used by commands like\n\
 * ps & pstat to free them from param.h.\n\
 */\n\
	\n"
};

char	strg4[] =
{
"int	nbuf	= NBUF;\n\
int	nproc	= NPROC;\n\
int	ninode	= %d;\n\
int	ntext	= NTEXT;\n\
int	nofile	= NOFILE;\n\
int	nsig	= NSIG;\n\
int	nfile	= NFILE;\n\
int	nmount	= %d;\n\
int	mapsize	= %d;\n\
int	ncall	= %d;\n\
int	nclist	= %d;\n\
int	elbsiz	= %d;\n\
int	maxuprc = %d;\n\
int	timezone = (%d*60);\n\
int	dstflag = %d;\n\
int	ncargs = %d;\n\
int	hz = %d;\n\
int	msgbufs = %d;\n\
unsigned int maxseg = %u;\n\
long	cdlimit = %ld;\n"
};

char	*strh[] =
{
	"	0",
	"};",
	"",
	"int	ttstart();",
	"int	ntyclose(), ntread(), ntwrite(), ntyinput();",
	"int	ttioctl();",
	0
};

char	*strj[] =
{
	"struct	linesw	linesw[] =",
	"{",
	"	nulldev, nulldev, ntread, ntwrite, ttioctl,",
	"	ntyinput, nodev, nulldev, ttstart, nulldev,	/* 0 */",
	"	nulldev, ntyclose, ntread, ntwrite, ttioctl,",
	"	ntyinput, nodev, nulldev, ttstart, nulldev,	/* 1 */",
	0
};

char	*strl1[] =
{
	"",
	"/*",
	" * Block I/O mode major device numbers",
	" */",
	"",
	0
};

char	*strl2[] =
{
	"",
	"/*",
	" * Character mode major device numbers",
	" */",
	"",
	0
};

char	*strm[] =
{
	"",
	"/*",
	" * I/O device CSR addresses",
	" * Indexed by RAW major device number",
	" */",
	"",
	"int\tio_csr[] =",
	"{",
	0
};

char	*strm1[] =
{
	"",
	"/*",
	" * I/O device bus addr ext. register offset.",
	" * Zero if no BAE register. Index is BLOCK major dev.",
	" */",
	"",
	"char	io_bae[] =",
	"{",
	"	0,	/* rk */",
	"	0,	/* rp */",
	"	0,	/* ra MBZ */",
	"	010,	/* rl */",
	"	0,	/* hx */",
	"	0,	/* tm */",
	"	0,	/* tk */",
	"	0,	/* ts */",
	"	034,	/* ht */",
	"	050,	/* hp */",
	"	050,	/* hm */",
	"	050,	/* hj */",
	"	0,	/* hk */",
	"	0,	/* u1 MBZ */",
	"	0,	/* u2 MBZ */",
	"	0,	/* u3 MBZ */",
	"	0,	/* u4 MBZ */",
	"};",
	0
};

char	*strm2[] =
{
	"",
	"/*",
	" * MSCP disk controller CSR and VECTOR addresses.",
	" */",
	"",
	"#if NUDA > 0",
	"int	ra_csr[] = {C0_CSR, C1_CSR, C2_CSR, C3_CSR};",
	"int	ra_ivec[] = {C0_VEC, C1_VEC, C2_VEC, C3_VEC};",
	"#else",
	"int	ra_csr[] = {0,0,0,0};",
	"#endif",
	"",
	"/*",
	" * TS magtape controller CSR and VECTOR addresses.",
	" */",
	"",
	"#if NTS > 0",
	"int	ts_csr[] = {S0_CSR, S1_CSR, S2_CSR, S3_CSR};",
	"#else",
	"int	ts_csr[] = {0,0,0,0};",
	"#endif",
	"",
	"/*",
	" * TMSCP magtape controller CSR and VECTOR addresses.",
	" */",
	"",
	"#if NTK > 0",
	"int	tk_csr[] = {K0_CSR, K1_CSR, K2_CSR, K3_CSR};",
	"int	tk_ivec[] = {K0_VEC, K1_VEC, K2_VEC, K3_VEC};",
	"#else",
	"int	tk_csr[] = {0,0,0,0};",
	"#endif",
	0
};

char	strn[] =
{
"	\n\
/*\n\
 * Disk I/O instrumentation, see bio.c\n\
 */\n\
	\n\
struct {\n\
	int	nbufr;\n\
	long	nread;\n\
	long	nreada;\n\
	long	ncache;\n\
	long	nwrite;\n"
};

char	*stro[] =
{
	"",
	"/*",
	" * Dummy unibus map support functions.",
	" */",
	"",
	"int\tmapinit();",
	"int\tmapalloc();",
	"int\tmapfree();",
	"",
	"mapinit()",
	"{",
	"}",
	"",
	"mapalloc()",
	"{",
	"}",
	"",
	"mapfree()",
	"{",
	"}",
	"",
	0
};

char	*strnofpe[] =
{
	"",
	"int fpemulation = 2;",
	"",
	"/*",
	" * Dummy function required if floating point simulator",
	" * is not used.",
	" */",
	"",
	"fptrap()",
	"{",
	"	return(SIGILL);",
	"}",
	"",
	0
};

char	*strfpe[] =
{
	"",
	"int fpemulation = 1;",
	"",
	0
};

char	*strq[] =
{
	"",
	"/*",
	" * Disk driver data structure sizes header file (dds.h).",
	" * Controls number and size of disk data structures in",
	" * (dds.c), for MSCP (ra.c) and MASSBUS (hp.c) disk drivers.",
	" */",
	"",
	0
};

char	*strgk[] =
{
	"",
	"int\tgeneric;\t/* generic kernel */",
	"",
	0
};

/*
 * If the utsname structure is changed,
 * check the code in the setpar() routine
 * in /usr/sys/sas/boot.c. Boot loads the machine
 * name into machine[] as PDP-11/##.
 */

char	*strutsn[] =
{
	"",
	"/*",
	" * Utsname definition",
	" */",
	"",
	"#include <sys/utsname.h>",
	"struct utsname utsname = {",
	"\t{'U','L','T','R','I','X','-','1','1'},",
	"\t{''},",
	"\t{'3'},",
	"\t{'0'},",
	"\t{''},",
	"};",
	"",
	0
};

char	*strmausno[] =
{
	"",
	"/*",
	" * Dummy maus routines and structures",
	" */",
	"#include <sys/maus.h>",
	"maus()",
	"{",
	"	nosys();",
	"}",
	"mausinit()",
	"{",
	"	return(0);",
	"}",
	"long",
	"mmoff()",
	"{",
	"	u.u_error = ENXIO;",
	"	return(-1L);",
	"}",
	" ",
	"struct mausmap mausmap[] = {",
	"	-1,	-1",
	"};",
	"",
	0
};

char	*strsemno[] =
{
	"",
	"/*",
	" * Dummy semaphore routines",
	" */",
	"semsys()",
	"{",
	"	nosys();",
	"}",
	"seminit()",
	"{",
	"}",
	"semexit()",
	"{",
	"}",
	"",
	0
};

char	*strmsgno[] =
{
	"",
	"/*",
	" * Dummy message routines",
	" */",
	"msgsys()",
	"{",
	"	nosys();",
	"}",
	"msginit()",
	"{",
	"	return(0);",
	"}",
	"",
	0
};

char	*strflckno[] =
{
	"",
	"/*",
	" * Dummy record/file locking routines",
	" */",
	"flckinit()",
	"{",
	"	return(0);",
	"}",
	"cleanlocks()",
	"{",
	"	return(0);",
	"}",
	"setflck()",
	"{",
	"	return(1);",
	"}",
	"getflck()",
	"{",
	"	return(1);",
	"}",
	"",
	0
};

char	*strshuff[] =
{
	"",
	"/*",
	" * Dummy shuffle routine",
	" */",
	"shuffle()",
	"{",
	"	wakeup((caddr_t)&runlock);",
	"}",
	"",
	0
};
