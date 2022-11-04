
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)sg_tab.c	3.3	8/6/87";
/*
 * Processor and peripheral device information
 * tables for sysgen.
 *
 * Fred Canter
 */

#include "sysgen.h"

/*
 * Processor type information
 */
struct	cputyp	cputyp[] = {
	"23",	23,	NSID,	QBUS,	A18BIT,
	"23+",	23,	NSID,	QBUS,	A22BIT,
	"24",	24,	NSID,	UBUS,	(A22BIT|A18BIT|UBMAP),
	"34",	34,	NSID,	UBUS,	A18BIT,
	"40",	40,	NSID,	UBUS,	A18BIT,
	"44",	44,	SID,	UBUS,	(A22BIT|UBMAP),
	"45",	45,	SID,	UBUS,	A18BIT,
	/* TODO: remove LATENT flag after 11/53 announced */
	"53",	53,	SID,	QBUS,	(A22BIT|LATENT),	
	"55",	55,	SID,	UBUS,	A18BIT,
	"60",	60,	NSID,	UBUS,	A18BIT,
	"70",	70,	SID,	UBUS,	(A22BIT|UBMAP),
	"73",	73,	SID,	QBUS,	A22BIT,
	"83",	83,	SID,	QBUS,	A22BIT,
	"84",	84,	SID,	UBUS,	(A22BIT|UBMAP),
	0
};

/*
 * Disk drive type information
 */
struct	ddtype	ddtype[] = {
	"DUMMY",   0,	      0,     0, -1,	0,	0,
	"rm02",	"hp",	0176700,  0254,  11,	8,	MASSBUS,
	"rm03",	"hp",	0176700,  0254,  11,	8,	MASSBUS,
	"rm05",	"hp",	0176700,  0254,  12,	8,	MASSBUS,
	"rp04",	"hp",	0176700,  0254,  1,	8,	MASSBUS,
	"rp05",	"hp",	0176700,  0254,  1,	8,	MASSBUS,
	"rp06",	"hp",	0176700,  0254,  1,	8,	MASSBUS,
	"rp02", "rp",	0176710,  0254,  3,	8,	0,
	"rp03", "rp",	0176710,  0254,  3,	8,	0,
	"rk06",	"hk",	0177440,  0210,  2,	8,	0,
	"rk07",	"hk",	0177440,  0210,  2,	8,	0,
	"rl01",	"rl",	0174400,  0160,  5,	8,	0,
	"rl02",	"rl",	0174400,  0160,  5,	8,	0,
	"rx02",	"hx",	0177170,  0264, -1,	1,	0,
	"rk05",	"rk",	0177400,  0220,  4,	1,	0,
	"ra60",	"ra",	0172150,  0154,  7,	8,	0,
	"ra80",	"ra",	0172150,  0154,  7,	8,	0,
	"ra81",	"ra",	0172150,  0154,  7,	8,	0,
	"ml11",	"hp",	0176400,  0204,  6,	8,	(MASSBUS|FIXED),
	"rx33",	"rq",	0172150,  0154, -1,	8,	0,
	"rx50",	"rq",	0172150,  0154, -1,	8,	0,
	"rd31",	"rq",	0172150,  0154,  13,	8,	0,
	"rd32",	"rq",	0172150,  0154,  9,	8,	0,
	"rd51",	"rq",	0172150,  0154,  8,	8,	0,
	"rd52",	"rq",	0172150,  0154,  9,	8,	0,
	"rd53",	"rq",	0172150,  0154,  9,	8,	0,
	"rd54",	"rq",	0172150,  0154,  9,	8,	0,
	"rc25",	"rc",	0172150,  0154,  10,	8,	0,
	0
};

/*
 * Standard file system layouts for system disk
 * rootdev, pipedev, swapdev, and eldev.
 */
/* WARNING: if sdfsl changed, must also check cdtab[] (crash dump dev table) */
/* WARNING: values must match sdfsl[] in boot (see /usr/sys/sas/boot.c). */

struct	sdfsl	sdfsl[] = {
/* The first slot is filled in and used if the users specifies the layout */
/* user  */ 0, 0, 0, 0, 0, 0,	0,	0,	0,	0,	0,	0,
/* rp456 */ 0, 0, 0, 0, 0, 0,	2,	200,	6070,	2,	0,	200,
/* hk    */ 0, 0, 0, 0, 0, 0,	1,	100,	2936,	1,	0,	100,
/* rp    */ 0, 0, 0, 0, 0, 0,	1,	100,	3100,	1,	0,	100,
/* rk    */ 0, 0, 0, 0, 0, 0,	0,	4000,	822,	0,	4822,	50,
/* rl01/2*/ 0, 0, 0, 0, 0, 0,	0,	8040,	2200,	0,	8000,	40,
/* ml    */ 0, 0, 0, 0, 0, 0,	0,	7092,	1100,	0,	7000,	92,
/* uda50 */ 0, 0, 0, 0, 0, 0,	2,	200,	6000,	2,	0,	200,
/* rd51  */ 0, 0, 0, 0, 0, 0,	0,	7500,	2200,	0,	7460,	40,
/* rd52-4*/ 0, 0, 0, 0, 0, 0,	2,	100,	3000,	2,	0,	100,
/* rd32  */
/* klesi */ 0, 0, 0, 0, 0, 0,	1,	200,	4000,	1,	0,	200,
/* rm2/3 */ 0, 0, 0, 0, 0, 0,	2,	200,	5400,	2,	0,	200,
/* rm05  */ 0, 0, 0, 0, 0, 0,	2,	300,	6388,	2,	0,	300,
/* rd31  */ 0, 0, 0, 0, 0, 0,	5,	100,	3000,	5,	0,	100,
};
/* WARNING: if sdfsl changed, must also check cdtab[] (crash dump dev table) */
/* WARNING: values must match sdfsl[] in boot (see /usr/sys/sas/boot.c). */

/*
 * Disk controller types
 */
struct	dctype	dctype[] = {
	"rh11",		RH11,	(UBUS|MASSBUS),
	"rh70",		RH70,	(UBUS|MASSBUS),
	"rp11",		RP11,	UBUS,
	"rk611",	RK711,	UBUS,
	"rk711",	RK711,	UBUS,
	"rl11",		RL11,	UBUS,
	"rx211",	RX211,	(UBUS|Q22WARN),
	"rk11",		RK11,	(UBUS|Q22WARN),
	"uda50",	UDA50,	(UBUS|MSCP),
	"kda50",	UDA50,	(UBUS|MSCP),
	"rqdx1",	RQDX1,	(UBUS|MSCP),
	"rqdx2",	RQDX1,	(UBUS|MSCP),
	"rqdx3",	RQDX1,	(UBUS|MSCP),
	"klesi",	KLESI,	(UBUS|MSCP),
	"rux1",		RUX1,	(UBUS|MSCP),
	0
};

/*
 * Drive types allowed on each controller
 */
struct	doc	doc[] = {
	RH11,	RM02, RP04, RP05, RP06, ML11, 0, 0, 0,
	RH70,	RM02, RM03, RM05, RP04, RP05, RP06, ML11, 0,
	RK611,	RK06, RK07, 0, 0, 0, 0, 0, 0,	/* NO LONGER USED */
	RK711,	RK06, RK07, 0, 0, 0, 0, 0, 0,
	RP11,	RP02, RP03, 0, 0, 0, 0, 0, 0,
	RL11,	RL01, RL02, 0, 0, 0, 0, 0, 0,
	RX211,	RX02, 0, 0, 0, 0, 0, 0, 0,
	RK11,	RK05, 0, 0, 0, 0, 0, 0, 0,
	UDA50,	RA60, RA80, RA81, 0, 0, 0, 0, 0,
	RQDX1,	RX33, RX50, RD31, RD32, RD51, RD52, RD53, RD54,
	KLESI,	RC25, 0, 0, 0, 0, 0, 0, 0,
	RUX1,	RX50, 0, 0, 0, 0, 0, 0, 0,
	0
};

/*
 * Descriptor table for each
 * configured disk controller.
 */
struct	dcd	dcd[MAXDC+1];

/*
 * Magtape controller types
 */
struct	mttype	mttype[] = {
/* tm02 */	"tm02",	"ht",	0,	0,	0172440,	0224,
/* tm03 */	"tm03",	"ht",	0,	0,	0172440,	0224,
/* tm11 */	"tm11",	"tm",	0,	0,	0172520,	0224,
/* ts11 */	"ts11",	"ts",	0,	0,	0172520,	0224,
/* tsv05 */	"tsv05","ts",	0,	0,	0172520,	0224,
/* tsu05 */	"tsu05","ts",	0,	0,	0172520,	0224,
/* tu80 */	"tu80", "ts",	0,	0,	0172520,	0224,
/* tk25 */	"tk25", "ts",	0,	0,	0172520,	0224,
/* tk50 */	"tk50",	"tk",	0,	0,	0174500,	0260,
/* tu81 */	"tu81",	"tk",	0,	0,	0174500,	0260,
		0
};

/* ts magtape controller */

struct	mttype	mtts[] = {
	"xxxxxxx",	"ts",	-1,	0,	0172520,	0224,
	"xxxxxxx",	"ts",	-1,	0,	0172520,	0224,
	"xxxxxxx",	"ts",	-1,	0,	0172520,	0224,
	"xxxxxxx",	"ts",	-1,	0,	0172520,	0224,
	0
};

/* tk magtape controller */

struct	mttype	mttk[] = {
	"xxxxx",	"tk",	-1,	0,	0174500,	0260,
	"xxxxx",	"tk",	-1,	0,	0174500,	0260,
	"xxxxx",	"tk",	-1,	0,	0174500,	0260,
	"xxxxx",	"tk",	-1,	0,	0174500,	0260,
	0
};

/*
 * Crash dump device table
 */

/* WARNING: swplo hardwired, check sdfsl[] for changes! */
struct cdtab	cdtab[] =
{
	"ht", "tm02",	0,	0,	0,	0,	CD_TAPE,
	"ht", "tm03",	0,	0,	0,	0,	CD_TAPE,
	"ts", "ts11",	0,	0,	0,	0,	CD_TAPE,
	"ts", "tu80",	0,	0,	0,	0,	CD_TAPE,
	"ts", "tsv05",	0,	0,	0,	0,	CD_TAPE,
	"ts", "tsu05",	0,	0,	0,	0,	CD_TAPE,
	"ts", "tk25",	0,	0,	0,	0,	CD_TAPE,
	"tm", "tm11",	0,	0,	0,	0,	CD_TAPE,
	"tk", "tk50",	0,	0,	0,	0,	CD_TAPE,
	"tk", "tu81",	0,	0,	0,	0,	CD_TAPE,
	"rl", "rl01",	RL01,	0,	8340,	10240,	CD_DISK,
	"rl", "rl02",	RL02,	0,	8340,	10240,	CD_DISK,
	"hk", "rk06",	RK06,	0,	8320,	10956,	CD_DISK,
	"hk", "rk07",	RK07,	0,	8320,	10956,	CD_DISK,
	"rp", "rp02",	RP02,	0,	8800,	11600,	CD_DISK,
	"rp", "rp03",	RP03,	0,	8800,	11600,	CD_DISK,
	"hp", "rp04",	RP04,	0,	30178,	35948,	CD_DISK,
	"hp", "rp05",	RP05,	0,	30178,	35948,	CD_DISK,
	"hp", "rp06",	RP06,	0,	30178,	35948,	CD_DISK,
	"hp", "rm02",	RM02,	0,	29620,	34720,	CD_DISK,
	"hp", "rm03",	RM03,	0,	29620,	34720,	CD_DISK,
	"hp", "rm05",	RM05,	0,	32216,	38304,	CD_DISK,
	"rq", "rx50",	RX50,	2,	0,	800,	(CD_DISK|CD_RX50),
	"rq", "rx33",	RX33,	1,	0,	800,	(CD_DISK|CD_RX50),
	"rq", "rd51",	RD51,	0,	7800,	9700,	CD_DISK,
	"rq", "rd31",	RD31,	0,	10100,	12800,	CD_DISK,
	"rq", "rd32",	RD32,	0,	27400,	30100,	CD_DISK,
	"rq", "rd52",	RD52,	0,	27400,	30100,	CD_DISK,
	"rq", "rd53",	RD53,	0,	27400,	30100,	CD_DISK,
	"rq", "rd54",	RD54,	0,	27400,	30100,	CD_DISK,
	"rc", "rc25",	RC25,	1,	9500,	13200,	CD_DISK,
	"ra", "ra60",	RA60,	0,	30100,	35800,	CD_DISK,
	"ra", "ra80",	RA80,	0,	30100,	35800,	CD_DISK,
	"ra", "ra81",	RA81,	0,	30100,	35800,	CD_DISK,
	0
};

/*
 * Line printer
 */
struct	lptype	lptype[] = {
	"lp",	0177514, 0200, 0,
	0
};

/*
 * C/A/T phototypesetter interface
 */
struct	cttype	cttype[] = {
	"ct",	0167770, 0300, 0,
	0
};

/*
 * User devices
 */
struct	udtype	udtype[MAXUD+1] = {
	"u1",	0164000, 0400, 0,
	"u2",	0164010, 0410, 0,
	"u3",	0164020, 0420, 0,
	"u4",	0164030, 0430, 0,
	0
};

/*
 * Communications multiplexers
 */
struct	cmtype	cmtype[] = {
/* DZ11  */	"dz",	"dz",	"DZ11",	      16,   0,   0160100,	0300,
/* DZV11 */	"dzv",	"dzv",	"DZV11",       8,   0,   0160100,	0300,
/* DZQ11 */	"dzq",	"dzv",	"DZQ11",       8,   0,   0160100,	0300,
/* DH11  */	"dh",	"dh",	"DH11",        8,   0,   0160020,	0300,
/* DHU11  */	"dhu",	"uh",	"DHU11",       8,   0,   0160440,	0300,
/* DHV11  */	"dhv",	"uhv",	"DHV11",       4,   0,   0160440,	0300,
/* DHDM  */	"dhdm",	"dhdm",	"DM11-BB",     8,   0,   0170500,	0300,
/* DC11  */ /*	"dc",	"dc",	"DC11",        0,   0,   0174000,	0300, defunct */
/* DU11  */	"du",	"du",	"DU11",        4,   0,   0176040,	0300,
/* DN11  */	"dn",	"dn",	"DN11",        1,   0,   0175200,	0300,
/* KL11  */	"kl",	"kl",	"DL11-A/B",   16,   0,   0176500,	0300,
/* DL11  */	"dl",	"dl",	"DL11-C/D/E", 32,   0,   0175610,	0300,
	0
};

/*
 * Network devices
 */
static int qe_a[2] = { 0174440, 0174460 };
static int qe_v[2] = { 0400, 0410 };
static int de_a[4] = { 0174510, 0160010, 0160020, 0160030 };
static int de_v[4] = { 0120, 0300, 0300, 0300 };
static int n1_a[1] = { 0 };
static int n1_v[1] = { 0 };
static int n2_a[1] = { 0 };
static int n2_v[1] = { 0 };
/*
 * We use absolute value of nvec, if it is negative then
 * the user can't change it.
 */
struct	nettype nettype[] = {
	"deqna",	"qe", qe_a, qe_v, 2, 0, -1,
	"deuna",	"de", de_a, de_v, 4, 0, -1,
	"n1",		"n1", n1_a, n1_v, 1, 0, 0,
	"n2",		"n2", n2_a, n2_v, 1, 0, 0,
	0
};

/*
 * System parameters
 */
struct	syspar	syspar[] = {
	/*
	 * WARNING: the position of "nproc" MUST come before "mapsize".
	 *
	 * MAXSEG: changed on the fly to 65408 if CPU has Q22 bus.
	 */
	"nbuf",		"sg_nbuf",		0,	0,	0,	-1,
	"ninode",	"sg_ninode",		75,	150,	0,	0,
	"nfile",	"sg_nfile",		75,	150,	0,	0,
	"nmount",	"sg_nmnt",		5,	8,	0,	0,
	"maxuprc",	"sg_muprc",		15,	25,	0,	0,
	"ncall",	"sg_ncall",		30,	50,	0,	0,
	"nproc",	"sg_nproc",		75,	150,	0,	0,
	"ntext",	"sg_ntext",		25,	40,	0,	0,
	"nclist",	"sg_nclist",		50,	65,	0,	0,
	"hz",		"sg_hz",		60,	60,	0,	-1,
	"timezone",	"sg_tz",		5,	5,	0,	-1,
	"dstflag",	"sg_dst",		1,	1,	0,	-1,
	"ncargs",	"sg_ncargs",		5120,	5120,	0,	0,
	"maxseg",	"sg_mseg",		61440,	61440,	0,	0,
	"msgbufs",	"sg_msgb",		128,	128,	0,	0,
	"mapsize",	"sg_mapsz",		67,	105,	0,	0,
	"ulimit",	"sg_ulimit",		1024,	1024,	0,	0,
	0
};

struct	syspar	msgpar[] = {
	"msgmax",	"sm_max",		8192,	8192,	0,	0,
	"msgmnb",	"sm_mnb",		16384,	16384,	0,	0,
	"msgtql",	"sm_tql",		40,	40,	0,	0,
	"msgssz",	"sm_ssz",		8,	8,	0,	0,
	"msgseg",	"sm_seg",		1024,	1024,	0,	0,
	"msgmap",	"sm_map",		50,	100,	0,	0,
	"msgmni",	"sm_mni",		5,	10,	0,	0,
	0
};

struct	syspar	sempar[] = {
	"semmap",	"ss_map",		5,	10,	0,	0,
	"semmni",	"ss_mni",		5,	10,	0,	0,
	"semmns",	"ss_mns",		30,	60,	0,	0,
	"semmnu",	"ss_mnu",		15,	30,	0,	0,
	"semume",	"ss_ume",		10,	10,	0,	0,
	"semmsl",	"ss_msl",		25,	25,	0,	0,
	"semopm",	"ss_opm",		10,	10,	0,	0,
	0
};

struct	syspar	flckpar[] = {
	"flckrec",	"sf_recf",		50,	100,	0,	0,
	"flckfil",	"sf_filf",		15,	20,	0,	0,
	0
};

struct	syspar	netpar[] = {
	"mbufs",	"sn_mbufs",		60,	60,	0,	0,
	"allocs",	"sn_allocs",		1500,	2000,	0,	0,
	0
};

struct	syspar	mauspar[] = {
	"nmaus",	"ms_nmaus",		4,	4,	0,	0,
	"maus0",	"ms_maus",		2,	2,	0,	0,
	"maus1",	"ms_maus",		128,	128,	0,	0,
	"maus2",	"ms_maus",		128,	128,	0,	0,
	"maus3",	"ms_maus",		128,	128,	0,	0,
	"maus4",	"ms_maus",		1,	1,	0,	0,
	"maus5",	"ms_maus",		1,	1,	0,	0,
	"maus6",	"ms_maus",		1,	1,	0,	0,
	"maus7",	"ms_maus",		1,	1,	0,	0,
	0
};

struct	dst_table dst_table[] = {
	"USA",DST_USA,
	"Australia",DST_AUST,
	"Western Europe",DST_WET,
	"Central Europe",DST_MET,
	"Eastern Europe",DST_EET,
	0
};
