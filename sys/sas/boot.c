
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)boot.c	3.2	7/11/87
 *
 * ULTRIX-11 V2.1 stand-alone bootstrap (Boot:)
 *
 * Boot is loaded into low memory by the primary bootstrap
 * and started via jump to location zero.
 * It then relocates itelf to hi memory (64 kw boundry)
 * and executes in hi memory in user mode.
 * The files to be booted are loaded into low memory
 * and executed in kernel mode.
 * The following types of files can be loaded:
 *
 * Type 407 - non separated I & D space
 *	Standalone programs, such as scat, mkfs, & restor only.
 *	Type 407 Unix files CANNOT be loaded !
 * Type 411 - separated I & D space
 *	Type 411 Unix files only !
 *	No type 411 standalone programs.
 * Type 430 - overlay files
 *	Type 430 Unix overlay text files only !
 *	No type 430 standalone files.
 * Type 431 - Split I & D space overlay
 * No other type files can be loaded !
 *
 * Fred Canter
 *
 * We now also have support for the following Unix files:
 *	Type 450 - Non-split I & D, up to 15 overlays
 *	Type 451 - Split I & D, up to 15 overlays
 * Support is #ifdef'ed with K450.  BIGKERNEL must also be defined
 * to load these. With BIGKERNEL defined the overall size can be
 * up to 198Kb-0200, rather than 128K-0200.  This also means you
 * need 256K to boot...
 *
 * Dave Borman
 */
#if	defined(K450) && !defined(BIGKERNEL)
#define	BIGKERNEL
#endif

#include <sys/param.h>
#include <sys/ino.h>
#include <sys/inode.h>
#include <sys/filsys.h>
#include <sys/dir.h>
#define	SEP_ID		/* so we get the right addr for KDSA6 in seg.h */
#include <sys/seg.h>
#include <sys/tty.h>
#include <sys/devmaj.h>
#include <sys/conf.h>
#include <sys/utsname.h>
#include "ra_saio.h"
#include "saio.h"
#include <a.out.h>

/*
 * These are defined in seg.h now.
 *	#define KDSA6 ((physadr)0172374)
 *	#define	KISA6 ((physadr)0172354)
 */
#define KISD0 ((physadr)0172300)

#define	MAXNBUF	72	/* Maximum number of I/O buffers, if CPU has UB map */
			/* Also defined in /usr/src/cmd/sysgen/sysgen.h */

struct nlist nl[] =
{
	{ "_el_prcw" },
#define			X_EL_PRCW	0
	{ "_cputype" },
#define			X_CPUTYPE 	1
	{ "_sepid" },
#define			X_SEPID		2
	{ "_maxmem" },
#define			X_MAXMEM	3
	{ "_ubmaps" },
#define			X_UBMAPS	4
	{ "_nmser" },
#define			X_NMSER		5
	{ "_cdreg" },
#define			X_CDREG		6
	{ "_rn_ssr3" },
#define			X_RN_SSR3	7
	{ "_mmr3" },
#define			X_MMR3		8
	{ "_cpereg" },
#define			X_CPEREG	9
	{ "ova" },
#define			X_OVA		10
	{ "_ndh11" },
#define			X_NDH11		11
	{ "_dh11" },
#define			X_DH11		12
	{ "_nkl11" },
#define			X_NKL11		13
	{ "_ndl11" },
#define			X_NDL11		14
	{ "_kl11" },
#define			X_KL11		15
	{ "_dz_cnt" },
#define			X_DZ_CNT	16
	{ "_dz_tty" },
#define			X_DZ_TTY	17
	{ "_ntty" },
#define			X_NTTY		18
	{ "_tty_ts" },
#define			X_TTY_TS	19
	{ "_pbaddr" },
#define			X_PBADDR	20
	{ "_nbuf" },
#define			X_NBUF		21
	{ "_mb_end" },
#define			X_MB_END	22
	{ "_nuh11" },
#define			X_NUH11		23
	{ "_uh11" },
#define			X_UH11		24
	{ "_io_csr" },
#define			X_IO_CSR	25
	{ "_rootdev" },
#define			X_ROOTDEV	26
	{ "_swapdev" },
#define			X_SWAPDEV	27
	{ "_pipedev" },
#define			X_PIPEDEV	28
	{ "_el_dev" },
#define			X_EL_DEV	29
	{ "dmp_dn" },
#define			X_DMP_DN	30
	{ "dmp_csr" },
#define			X_DMP_CSR	31
	{ "dmp_rx" },
#define			X_DMP_RX	32
	{ "_ub_end" },
#define			X_UB_END	33
	{ "_bpaddr" },
#define			X_BPADDR	34
	{ "_maplock" },
#define			X_MAPLOCK	35
	{ "klin" },
#define			X_KLIN		36
	{ "klou" },
#define			X_KLOU		37
	{ "raio" },
#define			X_RAIO		38
	{ "tsio" },
#define			X_TSIO		39
	{ "uhin" },
#define			X_UHIN		40
	{ "uhou" },
#define			X_UHOU		41
	{ "dzin" },
#define			X_DZIN		42
	{ "dzou" },
#define			X_DZOU		43
	{ "_nodev" },
#define			X_NODEV		44
	{ "_nulldev" },
#define			X_NULLDEV	45
	{ "_bdevsw" },
#define			X_BDEVSW	46
	{ "_cdevsw" },
#define			X_CDEVSW	47
	{ "_icode" },
#define			X_ICODE		48
	{ "_szicode" },
#define			X_SZICODE	49
	{ "_generic" },
#define			X_GENERIC	50
	{ "_swplo" },
#define			X_SWPLO		51
	{ "_nswap" },
#define			X_NSWAP		52
	{ "_el_sb" },
#define			X_EL_SB		53
	{ "_el_nb" },
#define			X_EL_NB		54
	{ "hkio" },
#define			X_HKIO		55
	{ "hpio" },
#define			X_HPIO		56
	{ "rlio" },
#define			X_RLIO		57
	{ "rpio" },
#define			X_RPIO		58
	{ "ovd" },
#define			X_OVD		59
	{ "ovend" },
#define			X_OVEND		60
	{ "tmio" },
#define			X_TMIO		61
	{ "htio" },
#define			X_HTIO		62
	{ "_nht" },
#define			X_NHT		63
	{ "_nts" },
#define			X_NTS		64
	{ "_ntm" },
#define			X_NTM		65
	{ "_tk_ctid" },
#define			X_TK_CTID	66
	{ "_nuda" },
#define			X_NUDA		67
	{ "_nra" },
#define			X_NRA		68
	{ "_ra_ctid" },
#define			X_RA_CTID	69
	{ "tkio" },
#define			X_TKIO		70
	{ "_utsname" },
#define			X_UTSNAME	71    /*GMM*/
	{ "_ntk" },
#define			X_NTK		72
	{ "" },
};

/*
 * "sepid" is set up by M.s and indicates the class
 * of CPU being used, i.e., seperate I & D space
 * or I space only.
 *
 * sepid = 1, for separate I & D space CPU's.
 * sepid = 0, for non-separate I & D space CPU's.
 *
 */

/*
 * WHEN BOOT LOADS SDLOAD
 * The following two variables are used to pass the boot device
 * ID from Boot: to sdload. Because both programs use M.s these
 * variables have the same address in both programs.
 * The boot device ID tells sdload where to find the boot and
 * stand-alone programs.
 *
 * WHEN SDLOAD LOADS BOOT
 * They contain the root device ID and kernel name and
 * cause Boot: to auto-boot ??(0,0)??unix.
 * ?? = rd, hp, hk, etc.
 * If boot not loaded by sdload they are zero.
 */

int	sdl_bdn;	/* two char device name (ht, ts, mt, rx, md, tk) */
int	sdl_bdu;	/* bood device unit number */

/*
 * WHEN BOOT LOADS SDLOAD
 * The following two variables are used to pass the load device ID
 * to sdload. This tells sdload where to find the root and /usr
 * file systems. The boot and load devices can be different, e.g.,
 * for the ULCM-16, boot is the memory disk and load is the floppy.
 * Again, this kludge works because both programs use M.s, which
 * ensures that the addresses of the variables are the same in both
 * programs.
 *
 * WHEN SDLOAD LOADS BOOT
 * Sdload uses these locations to pass the load device ID back to boot.
 * This allows the gktape() routine to automatically select the type
 * of tape to be used when booting the generic kernel, just prior to
 * setup phase 1. The load device ID is ignored if it is invalid or
 * not a magtape.
 */

int	sdl_ldn;
int	sdl_ldu;

int	bdcode;		/* boot device type */
int	bdunit;		/* booted from unit number */
int	bdmtil;		/* mscp media type ID low */
int	bdmtih;		/* mscp media type ID high */
int	bdcsr;		/* boot device CSR address */
			/* passed from block zero boot */
struct	devsw	*ldp;	/* pointer to devsw entry for load device */
int	ld_bmaj;	/* Selected load device for generic kernel */
char	gline[50];	/* holds magtape name, see gktape() */
int	sepid;
int	ubmaps;
int	cputype;
int	cdreg;
int	nmser;
unsigned maxmem;
unsigned maxseg;
int	el_prcw;
int	rn_ssr3;
int	mmr3;
int	cpereg;
int	pbaddr;
char	line[100];
char	bootfs[] = "??(#,0)unix\0";	/* auto-boot of unix */
char	sdl_ld[] = "??(#,0)sdload\0";	/* Install command - load sdload */
char	sdl_sap[20];			/* filled in later with ??(#,0)saprog */
char	md_scf[] = "md(0,0)syscall\0";
char	rx_scf[] = "rx(0,0)syscall\0";
char	mt_scf[] = "ht(0,0)syscall\0";
char	dk_scf[] = "hp(0,0)/sas/syscall\0";
#ifdef	UMAX
char	init[] = "??(#,0)/etc/init\0";
#endif	UMAX

int	magic;
int	saflags;	/* SA program flag bits (see a.out.h) */
extern	struct	ra_drv	ra_drv[3][4];	/* see ra_saio.h (size must be 3 by 4) */
int	ra_openf[];
int	rl_openf;
int	first;
int	helpmsg;
int	scload;		/* syscall file loaded */
int	sdlflag;	/* sdload program, don't need syscall */
			/* also, must pass boot device ID to sdload */
int	rabflag;	/* rabads program does not need syscall */
int	autounit = 1;	/* modify kernel to run on unit number booted from */
int	autocsr = 1;	/* modify kernel to use CSR from devsw[] entry */
#define	CBSIZ	2048	/* Copy Buffer size */
#define	CBCNT	(512/(CBSIZ/512))	/* # of records to copy */
char	copybuf[CBSIZ];	/* Buffer for copying to memory disk */

/*
 * Boot device type codes
 * Passed to Boot: form block zero boot via M.s
 */

#define	BC_RA	2
#define	BC_HP	3
#define	BC_HK	4
#define	BC_RL	5
#define	BC_RP	6
#define	BC_RK	7
#define	BC_ML	8
#define	BC_HT	9
#define	BC_TS	10
#define	BC_TM	11
#define	BC_TK	12

/*
 * WARNING:
 *
 *	The values in this structure MUST match those in sysgen.
 *	See the sdfsl[] initialization in /usr/src/cmd/sysgen/sg_tab.c.
 *	I will take the heat for not using a header file -- Fred Canter.
 *	The reasons are: the structures are not exactly the same,
 *	these values are only used when booting a generic kernel, and
 *	I was pressed for time.
 *
 * CAUTION:
 *
 *	The values for swplo and el_sb must not exceed 16 bit quantities,
 *	mpti() has trouble loading a long. This is a safe kludge because
 *	the values are small.
 *
 * System Disk File System Layout
 *
 * If, and only if, booting a generic kernel, these values are used
 * to set the root, pipe, swap, and error log devices, swap and error
 * log starting block number and length.
 * Three "hp" entries due to different disk types, code selects the
 * correct entry. Two "rd" entries for the same reason.
 */

struct sdfsl {
	char	*sdf_type;	/* ULTRIX-11 device menmonic */
	char	sdf_root;	/* rootdev - disk partition */
	char	sdf_pipe;	/* pipedev - disk partition */
	char	sdf_swap;	/* swapdev - disk partition */
	char	sdf_elog;	/* elogdev - disk partition */
	unsigned sdf_swplo;	/* swplo - swap area start block */
	int	sdf_nswap;	/* nswap - swap area length */
	unsigned sdf_elsb;	/* el_sb - error log start block */
	int	sdf_elnb;	/* el_nb - error log length */
} sdfsl[] = {
	"hk", 0, 0, 1, 1,	100,	2936,	0,	100,	/* RK06/7 */
	"hp", 0, 0, 2, 2,	200,	6070,	0,	200,	/* RP04/5/6 */
	"hp", 0, 0, 2, 2,	200,	5400,	0,	200,	/* RM02/3 */
	"hp", 0, 0, 2, 2,	300,	6388,	0,	300,	/* RM05 */
	"ra", 0, 0, 2, 2,	200,	6000,	0,	200,	/* RA60/80/81 */
	"rc", 0, 0, 1, 1,	200,	4000,	0,	200,	/* RC25 */
	"rd", 0, 0, 0, 0,	7500,	2200,	7460,	40,	/* RD51 */
	"rd", 0, 0, 2, 2,	100,	3000,	0,	100,	/* RD32/52/53/54 */
	"rd", 0, 0, 5, 5,	100,	3000,	0,	100,	/* RD31 */
	"rl", 0, 0, 0, 0,	8040,	2200,	8000,	40,	/* RL01/2 */
	"rp", 0, 0, 1, 1,	100,	3100,	0,	100,	/* RP02/3 */
	0,
};

/*
 * Boot options help message.
 */

char	*options[] =
{
	"",
/* FARKLE: boot too big again!!!!!
	"To execute one of the boot options (listed below), enter the name",
	"of the option using only lowercase characters then press <RETURN>.",
	"SEE ALSO: Chapter 3 of the ULTRIX-11 System Management Guide.",
	"          ULTRIX-11 Installation Guide.",
	"",
	"aus	 Enable/disable Auto Unit Select (default = enable)",
	"	 Allows booting of system from a disk drive other than unit",
	"	 zero (changes root device unit number in the booted kernel).",
	"",
	"acs	 Enable/disable Auto CSR Select (default = enable)",
	"	 Allows booting of disks with non standard CSR, by passing",
	"	 CSR from boot's device address table to the booted kernel.",
	"",
	"csr	 List and/or change a device's CSR address entry boot's",
	"	 device address table. Boot will prompt for device name.",
	"",
	"install Begin the auto-install procedure.",
*/
	0
};

char	*gkt_help[] =
{
	"You are booting a generic kernel. This kernel is configured with",
	"all possible software load devices. You need to specify which,",
	"if any, of the possible load devices you have. You should specify",
	"the device used to load the software from the distribution kit.",
	"",
	"If the system does not have a load device, just press <RETURN>.",
	"Otherwise, enter the ULTRIX-11 mnemonic for your load device:",
	"",
	"	ht = TM02/3-TU16/TE16/TU77",
	"	tm = TM11-TU10/TE10",
	"	ts = TS11/TU80/TS05",
	"	tk = TK50 (or TU81)",
	"	rx = RX50",
	"	rl = RL02",
	"	rc = RC25",
	"",
	0
};

main()
{
	register struct devsw *dp;
	int i, j;
	int abflag;
	char	*p;

/*
 * Set the segflag for a 64k word boundry.
 */
#ifdef	BIGKERNEL
	segflag = 3;
#else	BIGKERNEL
	segflag = 2;
#endif	BIGKERNEL

/*
 * Check booted from device type and attempt to
 * auto-boot unix if possible.
 */
	if(first == 0) {	/* auto-boot first time thru only */
		first++;
		/*
		 * Load maxmem into the dv_csr field of the "md"
		 * entry in the devsw[] table. This tells the memory
		 * disk driver how much memory is available.
		 * The maxmem value is set by the startup code (see M.s).
		 */
		for(i=0; devsw[i].dv_name; i++)
			if(match("md", devsw[i].dv_name))
				break;
		if(devsw[i].dv_name != 0)
			devsw[i].dv_csr = maxmem;
		abflag = 1;
		if(sdl_bdn) {	/* loaded by SDLOAD, auto-boot */
				/* ??(#,0)unix */
			bootfs[0] = sdl_bdn & 0177;
			bootfs[1] = (sdl_bdn >> 8) & 0177;
			bootfs[3] = sdl_bdu + '0';
			goto boot;
		}
	} else
		abflag = 0;
	bootfs[3] = bdunit + '0';
				/* All valid load devices fill in their name */
	sdl_ld[0] = '?';	/* Say not booted from a valid load device */
	sdl_ld[3] = bootfs[3];
	switch(bdcode) {
	case BC_RA:		/* UDA50, KDA50, RQDX, RC25 */
		if((bdmtih == 022544) &&
		    ((bdmtil == 040063) || (bdmtil == 040064) ||
		     (bdmtil == 040065) || (bdmtil == 040040) ||
		     (bdmtil == 040066) || (bdmtil == 040037))) {
			bootfs[0] = 'r';    /* RD31/RD32/RD51/RD52/RD53/RD54 */
			bootfs[1] = 'd';
		} else if((bdmtih == 020144) &&
			((bdmtil == 030031) || (bdmtil == 031431))) {
			bootfs[0] = 'r';	/* RC25/RCF25 */
			bootfs[1] = 'c';
			sdl_ld[0] = 'r';
			sdl_ld[1] = 'c';
		} else if((bdmtih == 021244) && (bdmtil == 010074)) {
			bootfs[0] = 'r';	/* RA60 */
			bootfs[1] = 'a';
		} else if((bdmtih == 022544) &&
			((bdmtil == 010120) ||
			(bdmtil == 010121))) {
				bootfs[0] = 'r';	/* RA80/RA81 */
				bootfs[1] = 'a';
		} else if((bdmtih == 022545) &&
			  ((bdmtil == 0100062) || (bdmtil == 0100041))) {
				sdl_ld[0] = 'r';	/* RX33/RX50 */
				sdl_ld[1] = 'x';
				bootfs[0] = 'r';
				bootfs[1] = 'x';
				abflag = 0;
		} else
			abflag = 0;
		break;
	case BC_HP:		/* RM02/3/5, RP04/5/6 (first RH) */
		bootfs[0] = 'h';
		bootfs[1] = 'p';
		break;
	case BC_HK:		/* RK06/7 */
		bootfs[0] = 'h';
		bootfs[1] = 'k';
		break;
	case BC_RL:		/* RL01/2 */
		bootfs[0] = 'r';
		bootfs[1] = 'l';
		sdl_ld[0] = 'r';
		sdl_ld[1] = 'l';
		break;
	case BC_RP:		/* RP02/3 */
		bootfs[0] = 'r';
		bootfs[1] = 'p';
		break;
	case BC_HT:		/* TM02/3 */
		abflag = 0;	/* no auto-boot */
		sdl_ld[0] = 'h';
		sdl_ld[1] = 't';
		bootfs[0] = 'h';
		bootfs[1] = 't';
		break;
	case BC_TS:		/* TS11/TK25/TSV05/TU80 */
		abflag = 0;	/* no auto-boot */
		sdl_ld[0] = 't';
		sdl_ld[1] = 's';
		bootfs[0] = 't';
		bootfs[1] = 's';
		break;
	case BC_TM:		/* TM11 */
		abflag = 0;	/* no auto-boot */
		sdl_ld[0] = 't';
		sdl_ld[1] = 'm';
		bootfs[0] = 't';
		bootfs[1] = 'm';
		break;
	case BC_TK:		/* TK50/TU81 */
		abflag = 0;	/* no auto-boot */
		sdl_ld[0] = 't';
		sdl_ld[1] = 'k';
		bootfs[0] = 't';
		bootfs[1] = 'k';
		break;
	case BC_RK:		/* RK05 */
	case BC_ML:		/* ML11 */
				/* FALL THROUGH */
	default:		/* no auto-boot */
		abflag = 0;
		break;
	}
boot:
	ctrlc(1);	/* get rid of any extra CTRL/Cs */
	ra_openf[0] = 0;
	ra_openf[1] = 0;
	ra_openf[2] = 0;
	rl_openf = 0;
	/*
	 * Don't auto-boot RL02/RC25 distribution disks.
	 * The magic cookie is a file called unix linked to boot, i.e.,
	 * unix will have a magic number of 0401 (stand-alone program),
	 * which can't happen on a system disk.
	 */
	if(abflag && (bootfs[0]=='r') && ((bootfs[1]=='l')||(bootfs[1]=='c'))) {
		/*
		 * If auto-booting via block zero boot,
		 * use CSR passed from block zero boot via M.s.
		 * If auto-booting via sdload, CSR is loaded by
		 * sdload (devsw[] copied back).
		 */
		if(bdcsr && !sdl_bdn) {
			dp = (struct devsw *)dp_get(&bootfs);	/* find CSR in devsw[] */
			dp->dv_csr = bdcsr;
		}
		if((i = open(bootfs, 0)) >= 0) {
			if(getw(i) == 0401)
				abflag = 0;
			close(i);
		}
	}
	if((abflag == 0) && (helpmsg == 0)) {
		helpmsg++;
		printf("\n\nTo list options, type help then press <RETURN>");
	}
	printf("\n\nBoot: ");
	if(abflag) {
		printf("%s", bootfs);
		printf("    (CTRL/C will abort auto-boot)\n");
		for(j=0; bootfs[j]; j++)
			line[j] = bootfs[j];
		/*
		 * If auto-booting via block zero boot,
		 * use CSR passed from block zero boot via M.s.
		 * If auto-booting via sdload, CSR is loaded by
		 * sdload (devsw[] copied back).
		 */
		if(bdcsr && !sdl_bdn) {
			dp = (struct devsw *)dp_get(&line);	/* find CSR in devsw[] */
			dp->dv_csr = bdcsr;
		}
	} else
		gets(line);
	if(line[0] == '\0')
		goto boot;
	if(cmds())		/* see if user typed a command (like csr) */
		goto boot;
	i = open(line, 0);
	if(i < 0) {
		if(abflag)
			abflag = 0;
		goto boot;
	}
	if(ctrlc(0)) {	/* CTRL/C abort */
	abcancel:
		abflag = 0;
		goto boot;
	}

	if (copyunix(i))
		goto abcancel;
	p = &line[7];
	if(*p == '/')
	    p++;
	if((abflag == 0) && (strcmp("unix", p) != 0) &&
	   ((magic == 0411) || (magic == 0430) || (magic == 0431) ||
	    (magic == 0450) || (magic == 0451))) {
	    printf("\n\7\7\7WARNING:");
	    printf("\n    The kernel must be named unix for proper ");
	    printf("system operation!");
	    printf("\n    Save the existing kernel, then rename %s to unix:",p);
	    printf("\n        # mv /unix /ounix");
	    printf("\n        # mv /%s /unix  or  ln /%s /unix\n", p, p);
	}

}


copyunix(io)
register io;
{
register addr,s;
long totsiz;
long phys;
long ovaddr;
unsigned phys_i;
#ifndef	K450
unsigned	txtsiz,datsiz,bsssiz,ovsize;
unsigned	symsiz;
unsigned ovsizes[8];
#else	K450
unsigned	txtsiz,datsiz,bsssiz;
long		ovsize;
unsigned	symsiz;
unsigned	maxov = 8;
unsigned ovsizes[16];
#endif	K450
int i,cnt;
int pdr, psz;
int sc;
char *p;
struct devsw *dp, *dpa;


	lseek(io, (off_t)0, 0);
	if(strcmp(&line[7], "boot") == 0) {	/* loading boot from tape */
		if((strncmp(line, "ht", 2) == 0) ||
		   (strncmp(line, "tm", 2) == 0) ||
		   (strncmp(line, "ts", 2) == 0) ||
		   (strncmp(line, "tk", 2) == 0))
			for(i=0; i<512; i++)	/* skip 2 MT boot blocks */
				getw(io);
	}
	magic = getw(io);
	txtsiz = getw(io);
	datsiz = getw(io);
	bsssiz = getw(io);
	symsiz = getw(io);
	getw(io);		/* a_entry (not used) */
	saflags = getw(io);	/* a_unused (see a.out.h) */
	getw(io);		/* ship over remainder of a.out header */
				/* could be tape, use getw() instead of lseek */
	switch (magic) {
#ifdef	BOOT411
	case 0411:
		scload = 0;
		if(symsiz == 0)
			goto badst;
		if(nlist(line, nl) < 0)
			goto badnl;
		if(ctrlc(0))
			goto errxit;
		if(nl[X_EL_PRCW].n_type == 0)
			goto badnl;
		/*
		 * Print an error message if loading of a seperate
		 * I and D space file is attempted on an I space
		 * only CPU, such as 11/40.
		 */
		if(!sepid) {
			printf("\nCan't load sep I&D ( %o) files\n", magic);
			goto errxit;
		}
		/*
		 * Check size of data segment,
		 * it cannot be > 49088 bytes.
		 * The size limit is 64 bytes less than
		 * it could be in order to insure that there will
		 * be an unmapped memory page just below the U block
		 * in virtual data space. This allows for better
		 * handling of stack overflow errors, stack has been
		 * moved to below the user structure in the U block.
		 */
		if((datsiz + bsssiz) > 49088) {
			printf("\ndata segment too large (%d)\n", datsiz+bsssiz);
			goto errxit;
		}
		/*
		 * DO NOT BOOT if the value of the symbol _mb_end
		 * exists and is > 0120000, i.e., new buffers and
		 * part of the protected data structures fall within
		 * the forbidden zone.
		 */
		if (nl[X_MB_END].n_value > 0120000)
			goto mb_err;
		/*
		 * DO NOT BOOT if _ub_end > (datsiz+8192),
		 * data structures covered by the first unibus map
		 * register are too large.
		 */
		if(ubmaps && (nl[X_UB_END].n_value > (datsiz+8192)))
			goto ub_err;
		/*
		 * DO NOT BOOT if the processor has a unibus map
		 * but the kernel is not configured to support it.
		 * The symbol _maplock indicates map code included.
		 */
		if(ubmaps && (nl[X_MAPLOCK].n_type == 0))
			goto ubm_err;
#ifndef	BIGKERNEL
		/*
		 * When loading a seperate I and D space file,
		 * move the kernel stack to the last 8 kb
		 * section of the first 128 kb of memory.
		 * This prevents the stack from overwriting unix.
		 */
		KDSA6->r[0] = 03600;
#else	BIGKERNEL
		KDSA6->r[0] = (segflag<<10) - 0200;
#endif	BIGKERNEL
		ssid();		/* Insure sep I & D space enabled */
		setseg(0);
		if(ctrlc(0))
			goto errxit;
		lseek(io, (long)(020+txtsiz), 0);

		for(addr=0; addr!=datsiz; addr+=2)  {
			mtpi(getw(io),addr);
		}
		if(ctrlc(0))
			goto errxit;
		clrseg(addr,bsssiz);

		setpar();

		phys = (long)datsiz + (long)bsssiz + 63L;
		phys =/ 64;
		setseg((int)phys);

		if(ctrlc(0))
			goto errxit;
		lseek(io, 020L, 0);

		for(addr=0; addr!=txtsiz; addr+=2) {
			mtpi(getw(io),addr);
		}
		if(ctrlc(0))
			goto errxit;
		close(io);
		return(0);
#endif	BOOT411
#ifdef	K450
	case 0451:
		maxov = 16;
#endif	K450
	case 0431:
		scload = 0;
		if(symsiz == 0)
			goto badst;
		if(nlist(line, nl) < 0)
			goto badnl;
		if(ctrlc(0))
			goto errxit;
		if(nl[X_EL_PRCW].n_type == 0)
			goto badnl;
		/*
		 * Print an error message if loading of a seperate
		 * I and D space file is attempted on an I space
		 * only CPU, such as 11/40.
		 */
		if(!sepid) {
			printf("\nCan't load sep I&D ( %o) files\n", magic);
			goto errxit;
		}
		/*
		 * Check size of data segment,
		 * it cannot be > 49088 bytes.
		 * The size limit is 64 bytes less than
		 * it could be in order to insure that there will
		 * be an unmapped memory page just below the U block
		 * in virtual data space. This allows for better
		 * handling of stack overflow errors, stack has been
		 * moved to below the user structure in the U block.
		 */
		if((datsiz + bsssiz) > 49088) {
			printf("\ndata segment too large (%d)\n", datsiz+bsssiz);
			goto errxit;
		}
		/*
		 * DO NOT BOOT if the value of the symbol _mb_end
		 * exists and is > 0120000, i.e., new buffers and
		 * part of the protected data structures fall within
		 * the forbidden zone.
		 */
		if (nl[X_MB_END].n_value > 0120000)
			goto mb_err;
		/*
		 * DO NOT BOOT if _ub_end > (datsiz+8192),
		 * data structures covered by the first unibus map
		 * register are too large.
		 */
		if(ubmaps && (nl[X_UB_END].n_value > (datsiz+8192)))
			goto ub_err;
		/*
		 * DO NOT BOOT if the processor has a unibus map
		 * but the kernel is not configured to support it.
		 * The symbol _maplock indicates map code included.
		 */
		if(ubmaps && (nl[X_MAPLOCK].n_type == 0))
			goto ubm_err;
		/*
		 * Check size of root text segment
		 */
		if(txtsiz < 0140000) {
			printf("\nRoot text segment too small (%u)\n", txtsiz);
			goto errxit;
		}
		if(txtsiz > 0160000) {
			printf("\nRoot text segment too large (%u)\n", txtsiz);
			goto errxit;
		}
#ifndef	BIGKERNEL
		/*
		 * When loading a seperate I and D space file,
		 * move the kernel stack to the last 8 kb
		 * section of the first 128 kb of memory.
		 * This prevents the stack from overwriting unix.
		 */
		KDSA6->r[0] = 03600;
#else	BIGKERNEL
		KDSA6->r[0] = (segflag<<10) - 0200;
#endif	BIGKERNEL
		ssid();		/* Insure sep I & D space enabled */
		lseek(io, (long)020, 0);	/* skip to overlay header */
#ifndef	K450
		for (i = 0, ovsize = 0; i < 8; i++) {
#else	K450
		for (i = 0, ovsize = 0; i < maxov; i++) {
#endif	K450
			ovsizes[i] = getw(io);
			ovsize += ovsizes[i];
		}
		ovsize -= ovsizes[0];
		if (ovsizes[0] > 020000) {
			printf("max overlay too big (0%o)\n", ovsizes[0]);
			goto errxit;
		}
		/*
		 * DO NOT BOOT if the 0431 kernel is > 129024 bytes total,
		 * this allows for a 2KB stack at the end of 128KB.
		 */
		totsiz = (long)datsiz+(long)bsssiz+(long)txtsiz;
		/*
		 * If loading run time only kernel, only add size of
		 * overlays actually loaded to total size.
		 */
		totsiz += (long)ovsize;
#ifndef	BIGKERNEL
		if(totsiz > 129024L) {
#else	BIGKERNEL
		/* max is 196096 bytes (512 byte kernel stack) */
		if(totsiz > ((long)segflag<<16) - 01000) {
#endif	BIGKERNEL
			printf("\nTotal kernel size too large (%D)\n", totsiz);
			goto errxit;
		}
		setseg(0);
		if(ctrlc(0))
			goto errxit;
		gktape();	/* If generic kernel, select magtape */
		printf("\n%s: ", line);
		/*
		 * Load data+bss segment at 0
		 */
		printf("%u", datsiz+bsssiz);
#ifndef	K450
		lseek(io, (long)(040L+(long)txtsiz+(long)ovsize), 0);
#else	K450
		lseek(io, 020L + maxov * 2L + (long)txtsiz + ovsize, 0);
#endif	K450
		for(addr=0; addr!=datsiz; addr+=2)
			mtpi(getw(io),addr);
		if(ctrlc(0))
			goto errxit;
		clrseg(addr,bsssiz);

		setpar();

		/*
		 * Load root text segment
		 */
		phys = (long)datsiz + (long)bsssiz + 63L;
		phys &= ~077;
		setseg((int)(phys/64));
		if(ctrlc(0))
			goto errxit;
		printf("+%u", txtsiz);
#ifndef	K450
		lseek(io, 040L, 0);
#else	K450
		lseek(io, (020L + maxov * 2L), 0);
#endif	K450
		for(addr=0; addr!=txtsiz; addr+=2)
			mtpi(getw(io),addr);
		/*
		 * Load the overlay segments after
		 * root text segment.
		 */
		phys += (long)txtsiz;
#ifndef	K450
		lseek(io, (long)(040L+(long)txtsiz), 0);
		ovaddr = phys;
		for(i=1; i<8; i++) {
#else	K450
		lseek(io, (020L + (maxov * 2L) +(long)txtsiz), 0);
		ovaddr = phys;
		for(i=1; i<maxov; i++) {
#endif	K450
			setseg((int)(phys/64));
#ifndef	BIGKERNEL
			KDSA6->r[0] = 03600; /* stack to end 128 kb */
					     /* not sure this is needed! */
#else	BIGKERNEL
			KDSA6->r[0] = (segflag<<10) - 0200;
#endif	BIGKERNEL
			phys += ovsizes[i];
			if(ctrlc(0))
				goto errxit;
			if(ovsizes[i])
				printf("+%u", ovsizes[i]);
			for(addr=0; addr<ovsizes[i]; addr += 2)
				mtpi(getw(io), addr);
		}
		printf("\n");
		setseg(0);
#ifndef	BIGKERNEL
		KDSA6->r[0] = 03600;	/* stack to end of 128 Kb */
#else	BIGKERNEL
		KDSA6->r[0] = (segflag<<10) - 0200;
#endif	BIGKERNEL
		addr = nl[X_OVA].n_value; /* _ova is where overlay tables are */
#ifndef	K450
		for(i=0; i<8; i++) {
#else	K450
		for(i=0; i<maxov; i++) {
#endif	K450
			if(i == 0)
				mtpi(0, addr);
			else {
				mtpi((int)(ovaddr/64), addr);
				ovaddr += ovsizes[i];
			}
			addr += 2;
		}
#ifndef	K450
		for(i=0; i<8; i++) {	/* overlay descriptor table */
#else	K450
		addr = nl[X_OVD].n_value;
		for(i=0; i<maxov; i++) {
#endif	K450
			if(i && (ovsizes[i] != 0))
				mtpi((((ovsizes[i]/64)-1)<<8) | RO, addr);
			else
				mtpi(0, addr);
			addr += 2;
		}
#ifdef	K450
		addr = nl[X_OVEND].n_value;
#endif	K450
		mtpi((int)(phys/64), addr);	/* start of free memory */
		phys = (long)datsiz + (long)bsssiz + 63L;
		phys &= ~077;
		setseg((int)(phys/64));
#ifndef	BIGKERNEL
		KDSA6->r[0] = 03600;	/* stack to end of 128 KB */
#else	BIGKERNEL
		KDSA6->r[0] = (segflag<<10) - 0200;
#endif	BIGKERNEL
		if(ctrlc(0))
			goto errxit;
		close(io);
		return(0);
	case 0401:	/* all ULTRIX-11 stand alone programs are now 0401 */
	case 0407:	/* (hopefully) allow user written SA programs to run */
		/*
		 * If the file has a symbol table, assume it is unix
		 * which can't be loaded.
		 */
		if(symsiz) {
			printf("\nCan't load ( %o) unix files\n", magic);
			goto errxit;
		}
		/*
		 * If loading a 407 type file on a separate I & D space
		 * type CPU, disable separate I & D space.
		 */
		if(sepid) {
			setseg(0);
			snsid();	/* disable sep I & D space */
		}
		if(ctrlc(0))
			goto errxit;
		/*
		 * Check SA program load flags to see
		 * if special loading needed.
		 */
		if(saflags & SA_SDLOAD) {
			sdlflag = 1;	/* say loading sdload program */
					/* save boot device ID to pass to sdload */
			sdl_bdn = (line[1] << 8) | line[0];	/* device name */
			sdl_bdu = line[3];			/* unit number */
					/* save load device ID to pass to sdload */
			sdl_ldn = (bootfs[1] << 8) | bootfs[0];	/* device name */
			sdl_ldu = bootfs[3];			/* unit number */
		} else
			sdlflag = 0;
		if(saflags & SA_RABADS)
			rabflag = 1;	/* say loading sdload program */
		else
			rabflag = 0;
		phys_i = txtsiz+datsiz;
		for (addr = 0; addr != phys_i; addr += 2)
			mtpi(getw(io),addr);
		if(ctrlc(0))
			goto errxit;
		clrseg(addr, bsssiz);
		if(ctrlc(0))
			goto errxit;
		close(io);
		if(sdlflag || rabflag) { /* sdload program, don't need syscall */
				/* also, pass boot & load device IDs */
				/* works because addresses same in both prog's */
			if(rabflag == 0) {
				mtpi(sdl_bdn, &sdl_bdn);
				mtpi(sdl_bdu, &sdl_bdu);
				mtpi(sdl_ldn, &sdl_ldn);
				mtpi(sdl_ldu, &sdl_ldu);
			}
			/*
			 * Copy the devsw[].dv_csr from this program to 
			 * the syscall segment of the stand-alone program.
			 * This updates any CSR address changes.
			 * location 100 in rabads/sdload has addr of devsw []
			 */
			dpa = mfpi(0100);
			for(dp = &devsw; dp->dv_name; dp++, dpa++)
				mtpi(dp->dv_csr, (char *)&dpa->dv_csr);
		}
		if((saflags&SA_SCSEG) == 0)
			return(0);	/* SA prog doesn't need syscall segment */
/*
 * MUST LOAD SYSCALL EVERY TIME (OH HECK DARN!)
 * This is because there is no clean way to clear
 * the RA open flag (ra_openf). Thus, causing the second
 * program (using RA disks) loaded to fail because the RA
 * driver does not init the MSCP controller.
 * The code remains in the hope that it will get fixed someday.
		if(scload)
			return(0);	/* syscall already loaded */
/*
 * Load /sas/syscall file at 64KB,
 * system calls for all 401 type standalone programs.
 */
		i = (line[1] << 8) | line[0];	/* two char device name */
		switch(i) {
		case 'md':	/* md (memory disk) */
			cnt = 6;
			p = &md_scf;
			break;
		case 'rx':	/* rx */
			cnt = 6;
			p = &rx_scf;
			break;
		case 'ht':	/* ht */
		case 'tm':	/* tm */
		case 'ts':	/* ts */
		case 'tk':	/* tk */
			cnt = 4;
			p = &mt_scf;
			break;
		case 'hk':	/* hk */
		case 'hm':	/* hm */
		case 'hp':	/* hp */
		case 'ml':	/* ml */
		case 'ra':	/* ra */
		case 'rc':	/* rc */
		case 'rd':	/* rd */
		case 'rk':	/* rk */
		case 'rl':	/* rl */
		case 'rp':	/* rp */
			cnt = 6;
			p = &dk_scf;
			break;
		default:
			/*
			 * NON DOCUMENTED ERROR MESSAGE!
			 * For debugging purposes only. This can't
			 * happen to a real life user.
			 */
			printf("\nCan't load syscall (bad device)\n");
			return(1);
		}
		for(i=0; i<cnt; i++)
			*(p+i) = line[i];
		sc = open(p, 0);
		if(sc < 0) {
			printf("\nCan't open %s file\n", p);
		errxit1:
			close(sc);
			return(1);
		}
		setseg(02000);	/* 64 KB */
		lseek(sc, (off_t)0, 0);
		getw(sc);
		txtsiz = getw(sc);
		datsiz = getw(sc);
		bsssiz = getw(sc);
		symsiz = getw(sc);
		getw(sc); getw(sc); getw(sc);
		phys_i = txtsiz+datsiz;
		for (addr = 0; addr != phys_i; addr += 2)
			mtpi(getw(sc),addr);
/*
 * Copy the devsw[].dv_csr from this program to 
 * the syscall segment of the stand-alone program.
 * This updates any CSR address changes.
 */
		dpa = mfpi(0);	/* syscall loc 0 has address of devsw */
		for(dp = &devsw; dp->dv_name; dp++, dpa++)
			mtpi(dp->dv_csr, (char *)&dpa->dv_csr);
		if(ctrlc(0)) {
			scload = 0;
			goto errxit1;
		}
/* Clears entire program not just bss segment ????? */
/* Not needed anyway. */
/*		clrseg(addr, bsssiz);	*/
		setseg(0);
		if(sepid)
			snsid();	/* Insure KISA7 set to I/O space */
		scload++;	/* say syscall file loaded */
		close(sc);
		return(0);

#ifdef	K450
	case 0450:
		maxov = 16;
#endif	K450
	case 0430:	/* overlayed text */
		scload = 0;
		if(symsiz == 0)
			goto badst;
		if(nlist(line, nl) < 0)
			goto badnl;
		if(ctrlc(0))
			goto errxit;
		if(nl[X_EL_PRCW].n_type == 0)
			goto badnl;
		/*
		 * DO NOT BOOT if the value of the symbol _mb_end
		 * is 0 or > 0120000, i.e., part of the protected
		 * data structures fall within the forbidden zone.
		 */
		if ((nl[X_MB_END].n_value == 0) ||
		    (nl[X_MB_END].n_value > 0120000))
			goto mb_err;
		/*
		 * DO NOT BOOT if _ub_end > (060000+datsiz+8192),
		 * data structures covered by the first unibus map
		 * register are too large.
		 */
		if(ubmaps && (nl[X_UB_END].n_value > (060000+datsiz+8192)))
			goto ub_err;
		/*
		 * DO NOT BOOT if the processor has a unibus map
		 * but the kernel is not configured to support it.
		 * The symbol _maplock indicates map code included.
		 */
		if(ubmaps && (nl[X_MAPLOCK].n_type == 0))
			goto ubm_err;
		/*
		 * If loading an overlay text kernel on a separate
		 * I & D space CPU, disable separate I & D space.
		 */
		if (sepid)
			snsid();
		
		/*
		 * check for consistency
		 */
		if (txtsiz < 020000) {
			printf("text segment too small (0%o)\n", txtsiz);
			goto errxit;
		}
		if (txtsiz > 040000) {
			printf("text segment too big (0%o)\n", txtsiz);
			goto errxit;
		}
		if (datsiz + bsssiz > 060000) {
			printf("data segment too big (0%o)\n", datsiz + bsssiz);
			goto errxit;
		}

		if(ctrlc(0))
			goto errxit;
		lseek(io, (long) 020, 0);	/* skip to overlay header */
#ifndef	K450
		for (i = 0, ovsize = 0; i < 8; i++) {
#else	K450
		for (i = 0, ovsize = 0; i < maxov; i++) {
#endif	K450
			ovsizes[i] = getw(io);
			ovsize += ovsizes[i];
		}
		ovsize -= ovsizes[0];
		if (ovsizes[0] > 020000) {
			printf("max overlay too big (0%o)\n", ovsizes[0]);
			goto errxit;
		}
		gktape();	/* If generic kernel, select magtape */
		printf("\n%s: ", line);
		ovsizes[0] = 0;
		/*
		 * load text segment at zero
		 */
		printf("%u", txtsiz);
		setseg(0);
		for (addr = 0; addr < txtsiz; addr +=2)
			mtpi(getw(io), addr);
		/*
		 * load data segment at 24Kb
		 */
		if(ctrlc(0))
			goto errxit;
		printf("+%u", datsiz+bsssiz);
		phys = 060000L;
		setseg((int)(phys/64));
#ifndef	K450
		lseek(io, 040L + (long)txtsiz + (long)ovsize, 0);
#else	K450
		lseek(io, 020L + (maxov * 2L) + (long)txtsiz + ovsize, 0);
#endif	K450
		for (addr = 0; addr < datsiz; addr += 2)
			mtpi(getw(io), addr);
		/*
		 * clear bss
		 */
		if(ctrlc(0))
			goto errxit;
		clrseg(addr, bsssiz);
		phys += datsiz + bsssiz;
		phys = (phys + 077) & ~077;
		/*
		 * new buffering scheme doesn't have pbaddr, and
		 * so we don't want to do this stuff.
		 */
		if (nl[X_PBADDR].n_value) {
			pbaddr = phys/64;
			phys += 8192;
			/*
			 * clear buffers
			 */
			setseg(pbaddr);
			clrseg(0, 8192);
		}
		/*
		 * load the overlays after the bss
		 * except for overlay 1, which is loaded at 16Kb
		 */
#ifndef	K450
		lseek(io, (long)(040+txtsiz), 0);
		ovaddr = phys;
		for (i = 1; i < 8; i++) {
#else	K450
		lseek(io, 020L + maxov * 2L + (long)txtsiz, 0);
		ovaddr = phys;
		for (i = 1; i < maxov; i++) {
#endif	K450
			if (i == 1)
				setseg(040000/64);
			else {
				setseg((int)(phys/64));
#ifndef	BIGKERNEL
			/*
			 * When loading an overlay text kernel,
			 * move the kernel stack to the last 8 kb
			 * section of the first 128 kb of memory.
			 * This prevents the stack from overwriting unix.
			 */
				KISA6->r[0] = 03600;
#else	BIGKERNEL
				KISA6->r[0] = (segflag<<10) - 0200;
#endif	BIGKERNEL
				phys += ovsizes[i];
			}
			if(ctrlc(0))
				goto errxit;
			if(ovsizes[i])
				printf("+%u", ovsizes[i]);
			for (addr = 0; addr < ovsizes[i]; addr += 2)
				mtpi(getw(io), addr);
		}
		printf("\n");

		/*
		 * build the overlay address and descriptor
		 * tables for the kernel.
		 */
		setseg(0);
#ifndef	BIGKERNEL
		/*
		 * When loading an overlay text kernel,
		 * move the kernel stack to the last 8 kb
		 * section of the first 128 kb of memory.
		 * This prevents the stack from overwriting unix.
		 */
		KISA6->r[0] = 03600;
#else	BIGKERNEL
		KISA6->r[0] = (segflag<<10) - 0200;
#endif	BIGKERNEL
		addr = nl[X_OVA].n_value;/* _ova, overlay tables start here */
#ifndef	K450
		for (i = 0; i < 8; i++) {
#else	K450
		for (i = 0; i < maxov; i++) {
#endif	K450
			if (i == 1)
				mtpi(040000/64, addr);
			else {
				mtpi((int)(ovaddr/64), addr);
				ovaddr += ovsizes[i];
			}
			addr += 2;
		}
#ifndef	K450
		for (i = 0; i < 8; i++) {
#else	K450
		addr = nl[X_OVD].n_value; /* overlay descriptors */
		for (i = 0; i < maxov; i++) {
#endif	K450
			if (ovsizes[i] != 0)
				mtpi((((ovsizes[i]/64)-1)<<8) | RO, addr);
			else
				mtpi(0, addr);
			addr += 2;
		}
#ifdef	K450
		addr = nl[X_OVEND].n_value;
#endif	K450
		mtpi((int)(phys/64), addr);	/* start of free memory */
		/*
		 * Correct the page length field in the last
		 * data space PDR to reflect the actual size
		 * of the data space segement so that data will not
		 * overlap the overlay text segments which
		 * are loaded immediately after data space.
		 */
		i = datsiz + bsssiz;
		pdr = 3 + (i / 8192);
		psz = i % 8192;
		if(psz > 0) {
			psz += 077;
			psz &= ~077;
			psz <<= 2;
			psz -= 0400;
			KISD0->r[pdr] = (psz | 6);
		}

		/*
		 * Although separate I & D space was disabled above,
		 * must call snsid() again to reset KISA7 to map to
		 * I/O space, because setseg(); changes it.
		 */
		if(sepid)
			snsid();
		setpar();
		if(ctrlc(0))
			goto errxit;
		close(io);
		return(0);

	default:
		printf("Can't load %o files\n", magic);
		goto errxit;
	}
	/* NOT REACHED */
/*
 * The following error messages and error exits
 * were moved here from the "case 0411:" code above
 * so that some of the cases could be conditionally included.
 */
badst:
	printf("\nUnix symbol table missing\n");
errxit:
	close(io);
	return(1);
badnl:
	printf("\nCan't access namelist in %s\n", line);
	goto errxit;
mb_err:
	printf("\nMAPPED BUFFERS - forbidden zone violation");
	printf("\n_mb_end = %o\n", nl[X_MB_END].n_value);
	goto errxit;
ub_err:
	printf("\nUNIBUS MAP - forbidden zone violation");
	printf("\n_ub_end = %o\n", nl[X_UB_END].n_value);
	goto errxit;
ubm_err:
	printf("\nUNIBUS MAP - no support code in kernel");
	goto errxit;
}

/*
 * Load the CPU descriptive parameters in locore.
 * Initialize TTY structures.
 */

setpar()
{
	register i;
	int	j, k;
	register char *p;
	register struct tty *ttysp;
	unsigned int ms;
	int addr, addr2;
	int nkl11, ndl11, ndh11, dz_cnt, ntty, nttys, nuh11;
	int nbuf, nb;
	int icode, szicode;
	int	io, sum, ts, ds;
	struct devsw *dp;
	int	rootdev, swapdev, pipedev, el_dev;
	struct sdfsl *sdp;
	int	generic, sd_bmaj;
	union {				/* GMM */
		int	cpuint[SYS_NMLN/2];
		char	cpuname[SYS_NMLN]; 
	} cpustr;

	if(nl[X_GENERIC].n_value)
		generic = 1;
	else
		generic = 0;
/*
 * OLD 0430 kernels ONLY!
 * Check the value of nbuf, for the overlay kernel only
 * if 16 boot is ok
 * if < 16 print warning & boot
 * if > 16 print warning & no boot.
 */
	nbuf = mfpi(nl[X_NBUF].n_value);
	if(magic == 0430 && nl[X_PBADDR].n_value) {
		if(nbuf != 16)
			printf("\nWARNING, nbuf is %d MUST be 16\n", nbuf);
		if(nbuf > 16)
			for( ;; ) ;
	}
/*
 * If CPU has unibus map and nbuf > MAXNBUF, reduce nbuf
 * to MAXNBUF (max that can be mapped with available registers).
 */
	if(ubmaps && (nbuf > MAXNBUF)) {
		nbuf = MAXNBUF;
		printf("\nUNIBUS MAP REGISTER LIMIT EXCEEDED: too many buffers!");
		printf("\nReducing number of buffers to %d!\n", MAXNBUF);
	}
/*
 * If new mapped buffer kernels (411, 430, 431),
 * compare the number of buffers with the available
 * memory and cut back if needed.
 *
 * CAUTION:	this code must match the code in
 *		/usr/src/cmd/sysgen/sg_ccf.c - g_gnb()
 */
	if(nl[X_BPADDR].n_value) {
		ms = maxmem / 16;
		nb = nbuf;
		if(ms < 248) {
			printf("\nCANNOT BOOT ULTRIX-11: ");
			printf("less than 248K bytes of memory!\n");
			for( ;; ) ;
		}
		if(nb < 16) {
			printf("\nCANNOT BOOT ULTRIX-11: ");
			printf("less than 16 buffers!\n");
			for( ;; ) ;
		}
		if((ms >= 248) && (ms <= 256) && (nb > 40))
			nbuf = 40;
		else if((ms > 256) && (ms < 384) && (nb > 50))
			nbuf = 50;
		else if((ms >= 384) && (ms < 512) && (nb > 100))
			nbuf = 100;
		else if((ms >= 512) && (ms < 768) && (nb > 200))
			nbuf = 200;
		else if((ms >= 768) && (ms < 1024) && (nb > 250))
			nbuf = 250;
		else if(nb > 300)
			nbuf = 300;
		if(nb != nbuf) {
		    printf("\nTOO MANY BUFFERS FOR %dK bytes of memory!", ms);
		    printf("\nReducing number of buffers to %d!\n", nbuf);
		}
	}
/*
 * Set up the TTY structure logical assignments, i.e.,
 *
 *	console
 *	all other KL's
 *	DL's
 *	DH's
 *	DHU/DHV's
 *	DZ's
 *
 * The parameter `ntty' is the number of available TTY structures,
 * `ntty' now always matches the number of communications ports.
 */
	nuh11=nkl11=ndl11=ndh11=dz_cnt=ntty = 0;
	if(nl[X_NDH11].n_value)			/* ndh11 */
		ndh11 = mfpi(nl[X_NDH11].n_value);
	if(nl[X_NKL11].n_value)			/* nkl11 */
		nkl11 = mfpi(nl[X_NKL11].n_value);
	if(nl[X_NDL11].n_value)			/* ndl11 */
		ndl11 = mfpi(nl[X_NDL11].n_value);
	if(nl[X_DZ_CNT].n_value)		/* dz_cnt */
		dz_cnt = mfpi(nl[X_DZ_CNT].n_value);
	if(nl[X_NTTY].n_value)			/* ntty */
		ntty = mfpi(nl[X_NTTY].n_value);
	if(nl[X_NUH11].n_value)			/* nuh11 */
		nuh11 = mfpi(nl[X_NUH11].n_value);
	if((nkl11==0)||(ntty==0)||
	   (nl[X_TTY_TS].n_value==0)||(nl[X_KL11].n_value==0)) {
		printf("\nCan't assign TTY structures in %s\n", line);
		return;
	}
#ifdef	UMAX
	icode = nl[X_ICODE].n_value;	/* address of icode[] in machdep.c */
	szicode = mfpi(nl[X_SZICODE].n_value);	/* size of icode[] */
	icode += (szicode - 4);		/* address of UMAX in icode[] */
	if(UMAX != mfpi(icode))		/* match UMAX limit */
		return;	/* will cause system to hang */
	init[0] = line[0];	/* ??(#,0)/etc/init - init pathname */
	init[1] = line[1];
	init[3] = line[3];
	io = open(init, 0);
	if(io < 0)
		return;	/* system will hang */
	getw(io);
	ts = getw(io);
	ds = getw(io);
	getw(io);
	getw(io);
	getw(io);
	getw(io);
	getw(io);
	sum = 0;
	for(i=0; i<(ts/2); i++)
		sum += getw(io);
	for(i=0; i<(ds/2); i++)
		sum += getw(io);
	close(io);
	if(sum != 0)
		return;	/* system will hang */
#endif	UMAX

	nttys = 0;
	ttysp = nl[X_TTY_TS].n_value;	/* points to first TTY structure */
	for(i=0; i<nkl11; i++) {	/* console and KL's */
		if(nttys >= ntty)
			goto loadpar;
		mtpi(ttysp, (nl[X_KL11].n_value + (sizeof(int)*i)));
		ttysp++;
		nttys++;
	}
	if(ndl11)			/* DL's */
		for(i=nkl11; i<(nkl11+ndl11); i++) {
			if(nttys >= ntty)
				goto loadpar;
			mtpi(ttysp, (nl[X_KL11].n_value + (sizeof(int)*i)));
			ttysp++;
			nttys++;
		}
	if(ndh11)			/* DH's */
		for(i=0; i<ndh11; i++) {
			if(nttys >= ntty)
				goto loadpar;
			mtpi(ttysp, (nl[X_DH11].n_value + (sizeof(int)*i)));
			ttysp++;
			nttys++;
		}
	if(nuh11)			/* DH's */
		for(i=0; i<nuh11; i++) {
			if(nttys >= ntty)
				goto loadpar;
			mtpi(ttysp, (nl[X_UH11].n_value + (sizeof(int)*i)));
			ttysp++;
			nttys++;
		}
	if(dz_cnt)			/* DZ's */
		for(i=0; i<dz_cnt; i++) {
			if(nttys >= ntty)
				goto loadpar;
			mtpi(ttysp, (nl[X_DZ_TTY].n_value + (sizeof(int)*i)));
			ttysp++;
			nttys++;
		}
loadpar:
/*
 * If autocsr enabled,
 * modify the booted kernel to use the disk CSR
 * address from the devsw[] entry.
 */
/* RESTRICTION - only works for first MSCP disk cntlr */
/* EXCEPT - for RX50 as load device (uses second cntlr, some times) */
	if(autocsr || generic) {
		dp = (struct devsw *)dp_get(&line);
		sd_bmaj = dp->dv_bmaj;
		addr = nl[X_IO_CSR].n_value;	/* addr of io_csr[] */
		addr += (dp->dv_rmaj * 2);	/* index to CSR */
		if(dp->dv_rmaj == RA_RMAJ) {	/* If MSCP disk cntlr */
			addr = mfpi(addr);	/* io_csr[] points to ra_csr[] */
			addr2 = addr+2;		/* points to 2nd MSCP cntrl CSR */
		}
		mtpi(dp->dv_csr, addr);
		if(nl[X_DMP_CSR].n_value)	/* fix crash dump dev CSR */
			mtpi(dp->dv_csr, nl[X_DMP_CSR].n_value);
	}
/*
 * If autounit enabled,
 * modify the booted kernel to run from the unit
 * specified in the boot command line, i.e., ra(#,0)unix.
 * Change rootdev, swapdev, pipedev, and el_dev, if they
 * are on the same major device as the boot device.
 */
	if(autounit || generic) {
	    dp = (struct devsw *)dp_get(&line);	/* find dev in devsw[] */
	    if(generic == 0) {
		rootdev = fixdev(dp, dp->dv_bmaj, mfpi(nl[X_ROOTDEV].n_value));
		swapdev = fixdev(dp, dp->dv_bmaj, mfpi(nl[X_SWAPDEV].n_value));
		pipedev = fixdev(dp, dp->dv_bmaj, mfpi(nl[X_PIPEDEV].n_value));
		el_dev = fixdev(dp, dp->dv_rmaj, mfpi(nl[X_EL_DEV].n_value));
	    } else {
		if((sdp = sdp_set(dp)) == 0) {
		    printf("\n%s: fatal generic kernel boot error!", line);
		    printf("\nCan't find boot device drive type!\n");
		    return;
		}
		rootdev = fixdev(dp,dp->dv_bmaj,(dp->dv_bmaj<<8)|sdp->sdf_root);
		pipedev = fixdev(dp,dp->dv_bmaj,(dp->dv_bmaj<<8)|sdp->sdf_pipe);
		swapdev = fixdev(dp,dp->dv_bmaj,(dp->dv_bmaj<<8)|sdp->sdf_swap);
		el_dev = fixdev(dp,dp->dv_rmaj,(dp->dv_rmaj<<8)|sdp->sdf_elog);
	    }
	    mtpi(rootdev, nl[X_ROOTDEV].n_value);
	    mtpi(swapdev, nl[X_SWAPDEV].n_value);
	    mtpi(pipedev, nl[X_PIPEDEV].n_value);
	    mtpi(el_dev, nl[X_EL_DEV].n_value);
	    /* change crash dump device unit #, unless it is RX50 */
	    if(nl[X_DMP_DN].n_value && (nl[X_DMP_RX].n_value == 0)) {
		i = line[3] - '0';
		mtpi(i, nl[X_DMP_DN].n_value);
	    }
	}
/*
 * If booting generic kernel,
 * load swplo, nswap, el_sb, and el_nb.
 * NOTE: swplo and el_sb are longs, but we only
 * 	 load the low word, safe because values small.
 */
	if(generic) {
		mtpi(sdp->sdf_swplo, nl[X_SWPLO].n_value+2);
		mtpi(sdp->sdf_nswap, nl[X_NSWAP].n_value);
		mtpi(sdp->sdf_elsb, nl[X_EL_SB].n_value+2);
		mtpi(sdp->sdf_elnb, nl[X_EL_NB].n_value);
	}
	mtpi(sepid, nl[X_SEPID].n_value);
	mtpi(ubmaps, nl[X_UBMAPS].n_value);
	mtpi(cputype, nl[X_CPUTYPE].n_value);
	mtpi(nmser, nl[X_NMSER].n_value);
	mtpi(cdreg, nl[X_CDREG].n_value);
	mtpi(maxmem, nl[X_MAXMEM].n_value);
	mtpi(nbuf, nl[X_NBUF].n_value);
	mtpi(el_prcw, nl[X_EL_PRCW].n_value);
	mtpi(rn_ssr3, nl[X_RN_SSR3].n_value);
	mtpi(mmr3, nl[X_MMR3].n_value);
	mtpi(cpereg, nl[X_CPEREG].n_value);
	if(nl[X_PBADDR].n_value)	/* pbaddr only in old 0430 type kernel */
		mtpi(pbaddr, nl[X_PBADDR].n_value);
	/*
	 * Fill in the machine name in the ustname structure (PDP-11/##).
	 */
	for(j=0; j<(SYS_NMLN/2); j++)
		cpustr.cpuint[j] = 0;
	sprintf(&cpustr.cpuname, "PDP-11/%d", cputype);
	for(j=0; j<(SYS_NMLN/2); j++)
		mtpi(cpustr.cpuint[j],
			nl[X_UTSNAME].n_value+(SYS_NMLN*4)+(j*2));
/*
 * If booting a generic kernel,
 * load system disk interrupt vector.
 * Also, zap the bdevsw and cdevsw entries for
 * all but the system disk.
 *
 * sd_bmaj is set to system disk's block major device
 * number, see autounit code above.
 */
	if(generic) {
		switch(sd_bmaj) {
		case HP_BMAJ:
			mtpi(nl[X_HPIO].n_value, 0254);		/* vector */
			mtpi(0240, 0254+2);			/* br5 */
			break;
		case HK_BMAJ:
			mtpi(nl[X_HKIO].n_value, 0210);		/* vector */
			mtpi(0240, 0210+2);			/* br5 */
			break;
		case RA_BMAJ:
			mtpi(nl[X_RAIO].n_value, 0154);		/* vector */
			mtpi(0240, 0154+2);			/* br5 */
			if(ld_bmaj != RA_BMAJ)	/* don't need 2nd MSCP cntlr */
			    zapra2(addr2);	/* zap second MSCP cntlr */
			break;
		case RL_BMAJ:
			mtpi(nl[X_RLIO].n_value, 0160);		/* vector */
			mtpi(0240, 0160+2);			/* br5 */
			break;
		case RP_BMAJ:
			mtpi(nl[X_RPIO].n_value, 0254);		/* vector */
			mtpi(0240, 0254+2);			/* br5 */
			break;
		default:
			/* CANNOT HAPPEN, would fail before we get here */
			return;
		}
		/* ldp points to devsw entry for load device */
		switch(ld_bmaj) {
		case RA_BMAJ:			/* Load device is RX50 or RC25 */
		    if(ldp->dv_flags&DV_RX) {	/* load device = RX50 */
			/*
			 * RX50 is the load device, must determine if
			 * second MSCP cntlr in generic kernel will be needed.
			 * Zap the CSR of 2nd cntlr if not needed.
			 *
			 * If system disk is RD, RX already config on cntlr 0.
			 * If system disk not MSCP, config RX on MSCP cntlr 0.
			 * If system disk RA or RC, config RX on MSCP cnltr 1.
			 */
			if(strcmp("rd", sdp->sdf_type) == 0) {
			    zapra2(addr2);	/* zap second MSCP cntlr */
			    break;	/* system disk is RD */
			} else if(sd_bmaj != RA_BMAJ) {
			    mtpi(nl[X_RAIO].n_value, 0154);	/* vector */
			    mtpi(0240, 0154+2);			/* br5+cntlr0 */
			    zapra2(addr2);	/* zap second MSCP cntlr */
			} else {
			    mtpi(nl[X_RAIO].n_value, 0150);	/* vector */
			    mtpi(0241, 0150+2);			/* br5+cnltr1 */
/*			    for(i=0; devsw[i].dv_name; i++) /* 2nd cntlr CSR */
/*				if(strcmp("rx", devsw[i].dv_name) == 0)	*/
/*				    break;	*/
/*			    mtpi(devsw[i].dv_csr, addr2);	/* load CSR */
			    mtpi(ldp->dv_csr, addr2);	/* load CSR */
						/* ^- value set above */
			}
		    }
		    if(ldp->dv_flags&DV_RC) {	/* load device = RC25 */
			/*
			 * RC25 is the load device, must determine if
			 * second MSCP cntlr in generic kernel will be needed.
			 * Zap the CSR of 2nd cntlr if not needed.
			 *
			 * If system disk is RC, RC already config on cntlr 0.
			 * If system disk not MSCP, config RC on MSCP cntlr 0.
			 * If system disk RA or RD, config RC on MSCP cnltr 1.
			 */
			if(strcmp("rc", sdp->sdf_type) == 0) {
			    zapra2(addr2);	/* zap second MSCP cntlr */
			    break;	/* system disk is RC */
			} else if(sd_bmaj != RA_BMAJ) {
			    mtpi(nl[X_RAIO].n_value, 0154);	/* vector */
			    mtpi(0240, 0154+2);			/* br5+cntlr0 */
			    zapra2(addr2);	/* zap second MSCP cntlr */
			} else {
			    mtpi(nl[X_RAIO].n_value, 0150);	/* vector */
			    mtpi(0241, 0150+2);			/* br5+cnltr1 */
/*			    for(i=0; devsw[i].dv_name; i++) /* 2nd cntlr CSR */
/*				if(strcmp("rc", devsw[i].dv_name) == 0)	*/
/*				    break;	*/
/*			    mtpi(devsw[i].dv_csr, addr2);	/* load CSR */
			    mtpi(ldp->dv_csr, addr2);	/* load CSR */
						/* ^- value set above */
			}
		    }
		    break;
		case RL_BMAJ:
			mtpi(nl[X_RLIO].n_value, 0160);		/* vector */
			mtpi(0240, 0160+2);			/* br5 */
			break;
		case HT_BMAJ:
			mtpi(nl[X_HTIO].n_value, 0224);		/* vector */
			mtpi(0240, 0224+2);			/* br5 */
			break;
		case TS_BMAJ:
			mtpi(nl[X_TSIO].n_value, 0224);		/* vector */
			mtpi(0240, 0224+2);			/* br5 */
			break;
		case TM_BMAJ:
			mtpi(nl[X_TMIO].n_value, 0224);		/* vector */
			mtpi(0240, 0224+2);			/* br5 */
			break;
		case TK_BMAJ:
			mtpi(nl[X_TKIO].n_value, 0260);		/* vector */
			mtpi(0240, 0260+2);			/* br5 */
			break;
		default:	/* -1, load dev not present or not selected */
			break;
		}
		if(sd_bmaj != HP_BMAJ)
			zapdev(0, HP_BMAJ, HP_RMAJ);
		if(sd_bmaj != HK_BMAJ)
			zapdev(0, HK_BMAJ, HK_RMAJ);
		if((sd_bmaj != RA_BMAJ) && (ld_bmaj != RA_BMAJ))
			zapdev(0, RA_BMAJ, RA_RMAJ);
		if((sd_bmaj != RL_BMAJ) && (ld_bmaj != RL_BMAJ))
			zapdev(0, RL_BMAJ, RL_RMAJ);
		if(sd_bmaj != RP_BMAJ)
			zapdev(0, RP_BMAJ, RP_RMAJ);
		if(ld_bmaj != HT_BMAJ) {
			zapdev(0, HT_BMAJ, HT_RMAJ);
			if(nl[X_NHT].n_value)
				mtpi(0, nl[X_NHT].n_value);
		}
		if(ld_bmaj != TS_BMAJ) {
			zapdev(0, TS_BMAJ, TS_RMAJ);
			if(nl[X_NTS].n_value)
				mtpi(0, nl[X_NTS].n_value);
		}
		if(ld_bmaj != TM_BMAJ) {
			zapdev(0, TM_BMAJ, TM_RMAJ);
			if(nl[X_NTM].n_value)
				mtpi(0, nl[X_NTM].n_value);
		}
		if(ld_bmaj != TK_BMAJ) {
			zapdev(0, TK_BMAJ, TK_RMAJ);
			if(nl[X_NTK].n_value)
				mtpi(0, nl[X_NTK].n_value);
		}
	}
}
/*
 * This function replaces the bdevsw[] and cdevsw[] entries
 * for a device with nodev() or nulldev(), to make it appear
 * to the kernel as if the device was not configured. This
 * prevents the kernel from accessing non-existent devices,
 * all possible devices are configured into the run time only kernel.
 *
 * The following is bad coding practice, but I will take the heat for it.
 * Fred Canter 1/19/85
 * The format of the bdevsw and cdevsw structures are hardwired into
 * the code below. If the format changes, boot will break. To cover that
 * unlikely event, a warning message was added to /usr/include/sys/conf.h,
 * where the bdevsw and cdevsw structures are defined.
 *
 * dev	device type code
 * bmaj block major device number (-1 if no block major)
 * rmaj	raw major device number
 */

zapdev(dev, bmaj, rmaj)
{
	register int i, j;

	i = nl[X_CDEVSW].n_value;		/* address of cdevsw[0] */
	i += (rmaj * sizeof(struct cdevsw));	/* address of device's entry */
	for(j=0; j<5; j++) {			/* zap driver entry points */
		mtpi(nl[X_NODEV].n_value, i);
		i += 2;
	}
	mtpi(nl[X_NULLDEV].n_value, i);		/* zap stop routine entry */
	i += 2;
	mtpi(0, i);				/* zap tty struct pointer */
	if(bmaj < 0)
		return;				/* not block mode device */
	i = nl[X_BDEVSW].n_value;		/* address of bdevsw[0] */
	i += (bmaj * sizeof(struct bdevsw));	/* address of device's entry */
	for(j=0; j<3; j++) {			/* zap driver entry points */
		mtpi(nl[X_NODEV].n_value, i);
		i += 2;
	}
	mtpi(0, i);				/* zap device's table addr */
}

/*
 * Check for a character typed on the console terminal.
 * Return 1 if it is CRTL/C, 0 otherwise.
 * If argment (clr) is nonzero, clear the console KL
 * receive CSR to get rid of any extra CTRL/C or other
 * garbage characters.
 */

struct	kldev {
	int	klrcsr, klrbuf;
	int	kltcsr, kltbuf;
};

struct	kldev	*KLCSR {0177560};

ctrlc(clr)
{
	if(clr)
		KLCSR->klrcsr = 0;
	if(KLCSR->klrcsr & 0200)
		if((KLCSR->klrbuf & 0177) == '\03') {
		printf("^C");
			return(1);
		}
	return(0);
}

match(s1,s2)
register char *s1,*s2;
{
	register cc;

	cc = DIRSIZ;
	while (cc--) {
		if (*s1 != *s2)
			return(0);
		if (*s1++ && *s2++)
			continue; else
			return(1);
	}
	return(1);
}

/*
 * Check for and execute user commands.
 * Return 0 if no command or command is install,
 * otherwise return 1.
 */

cmds()
{
	int	i, j, k;
	int	fi, fo;
	long	boff;

	if(match("help", line))
		goto c_help;
	if(match("csr", line))
		goto c_csr;
	if(match("aus", line))
		goto c_aus;
	if(match("acs", line))
		goto c_acs;
	if(match("install", line))
		goto c_instal;
	return(0);
c_help:
	for(i=0; options[i]; i++) {
		if(options[i] == -1) {
			printf("\n\nPress <RETURN> to continue:");
			while(getchar() != '\n') ;
		} else
			printf("\n%s", options[i]);
	}
	return(1);
c_aus:
	printf("\nAuto unit select <y or n> ? ");
	gets(line);
	if(match("y", line) || match("yes", line)) {
		autounit = 1;
		return(1);
	} else if(match("n", line) || match("no", line)) {
		autounit = 0;
		return(1);
	} else
		goto c_aus;
c_acs:
	printf("\nAuto CSR select <y or n> ? ");
	gets(line);
	if(match("y", line) || match("yes", line)) {
		autocsr = 1;
		return(1);
	} else if(match("n", line) || match("no", line)) {
		autocsr = 0;
		return(1);
	} else
		goto c_acs;
c_csr:
	printf("\nList all current CSR addresses <y or n> ? ");
	gets(line);
	if(match("no", line) || match("n", line))
		goto c_csr1;
	if(match("yes", line) || match("y", line)) {
		printf("\nDevice  CSR Address");
		printf("\n------  -----------");
		for(i=0; devsw[i].dv_name; i++) {
		    if(devsw[i].dv_flags&DV_MD)
			continue;		/* memory disk - no CSR */
		    printf("\n%s      %o",devsw[i].dv_name,devsw[i].dv_csr);
		}
		printf("\n");
	} else
		goto c_csr;
c_csr1:
	printf("\nEnter device name or press <RETURN> for no change.\n");
	printf("\nDevice < ");
	for(i=0; devsw[i].dv_name; i++) {
		if(devsw[i].dv_flags&DV_MD)
			continue;		/* memory disk - no CSR */
		printf("%s ", devsw[i].dv_name);
	}
	printf(">: ");
	gets(line);
	if(line[0] == '\0')
		goto c_csr3;
	for(i=0; devsw[i].dv_name; i++) {
		if(devsw[i].dv_flags&DV_MD)
			continue;		/* memory disk - no CSR */
		if(match(line, devsw[i].dv_name))
			break;
	}
	if(devsw[i].dv_name == 0)
		goto c_csr1;
	printf("\nEnter new CSR address or press <RETURN> for no change.\n");
	printf("\nCSR <%o>: ", devsw[i].dv_csr);
	gets(line);
	if(line[0] == '\0')
		goto c_csr3;
	j = 0;
	for(k=0; line[k]; k++) {
		j = j << 3;
		j |= line[k] & 7;
	}
c_csr2:
	printf("\nNew (%s) CSR address is %o <y or n> ? ",
		devsw[i].dv_name, j);
	gets(line);
	if(match("y", line) || match("yes", line))
		devsw[i].dv_csr = j;
	else if(match("n", line) || match("no", line))
		goto c_csr3;
	else
		goto c_csr2;
c_csr3:
	return(1);
/*
 * Install command, do some setup here then
 * return 0. This will cause sdload to be loaded as if
 * the user had typed ??(#,0)sdload.
 */

c_instal:
	if(sdl_ld[0] == '?') {	/* Not booted from a valid load device */
		printf("\nSorry - install only allowed from distr media!\n");
		return(1);
	}
	for(i=0; i<50; i++) {	/* copy ??(#,0)sdload to line */
		line[i] = sdl_ld[i];
		if(line[i] == '\0')
			break;
	}
	if((sdl_ld[0] == 'r') && ((sdl_ld[1] == 'c') || (sdl_ld[1] == 'l')))
		return(0);	/* RL02/RC25 - don't use memory disk */
	if(maxmem >= 8192) {	/* use memory disk if CPU has 512 KB */
		printf("\nCopying auto-install programs to memory disk...\n");
		sprintf(sdl_sap, "??(#,0)saprog");
		sdl_sap[0] = sdl_ld[0];
		sdl_sap[1] = sdl_ld[1];
		sdl_sap[3] = sdl_ld[3];
		if((sdl_sap[0] == 'r') && (sdl_sap[1] == 'x'))
			sdl_sap[7] = '\0';
		if((fi = open(sdl_sap, 0)) < 0) {
			printf("\nCan't open %s!\n", sdl_sap);
			return(1);
		}
		if((fo = open("md(0,0)", 1)) < 0) {
			printf("\nCan't open md(0,0)!\n");
			return(1);
		}
		for(i=0; i<CBCNT; i++) {
			boff = (long)i * (long)CBSIZ;
			lseek(fi, (long)boff, 0);
			if(read(fi, (char *)&copybuf, CBSIZ) < 0) {
				printf("\n%s: read error\n", sdl_sap);
				return(1);
			}
			lseek(fo, (long)boff, 0);
			if(write(fo, (char *)&copybuf, CBSIZ) < 0) {
				printf("\nmd(0,0): write error\n");
				return(1);
			}
		}
		line[0] = 'm';
		line[1] = 'd';
		line[3] = '0';
	}
	return(0);
}

/*
 * Modify the unit number portion of a major/minor
 * device specification, if the major device number matches.
 */

fixdev(dp, maj, dev)
struct devsw *dp;
{
	int	unit;

	unit = line[3] - '0';
	if((dev >> 8) != maj)
		return(dev);	/* no change */
	if(dp->dv_flags & DV_NPDSK)	/* non-partitioned disk */
		return((dev & ~0377) | unit);	/* minor = unit */
	else
		return((dev & ~070) | (unit << 3));
}

/*
 * Search the devsw[] table for the device name
 * located in the first two characters of str[].
 * If found return pointer ot devsw entry, if not
 * found return -1.
 */

dp_get(str)
char	*str;
{
	register struct devsw *dp;
	char	bdname[4];

	bdname[0] = str[0];
	bdname[1] = str[1];
	bdname[2] = '\0';
	for(dp = &devsw; dp->dv_name; dp++)
		if(match(&bdname, dp->dv_name))
			break;
	if(dp->dv_name == 0)
		return(-1);
	else
		return(dp);
}

/*
 * Find the system disk type in sdfsl[] and return a pointer
 * to its system disk layout entry. Return 0 if disk cannot
 * be found. Some magic needed if disk is "hp", because RP04/5/6,
 * RM02/3, and RM05 have different layouts.
 * The HP drive type is found in hpxdt[], see /usr/sys/sas/hp.c.
 * We assume drive type is in hpxdt[0] because the kernel is the only
 * file that is open during a boot, may be two file descriptors (nlist)
 * but they should both be for the kernel.
 * Same magic for "rd", because RD51, RD31, and RD52/RD53/RD54 disk layouts
 * have been changed and are also now different.
 */

extern	int	hpxdt[2];

#define	RP04	020
#define	RP05	021
#define	RP06	022
#define	RM02	025
#define	RM03	024
#define	RM05	027
/* MSCP disk types in ra_saio.h */

sdp_set(dp)
register struct devsw *dp;
{
	register struct sdfsl *sdp;
	register int unit;

	for(sdp=sdfsl; sdp->sdf_type; sdp++)
		if(match(dp->dv_name, sdp->sdf_type))
			break;
	if(sdp->sdf_type == 0)
		return(0);
	if(match("hp", sdp->sdf_type)) {
		switch(hpxdt[0]) {
		case RP04:
		case RP05:
		case RP06:
			break;
		case RM02:
		case RM03:
			sdp++;
			break;
		case RM05:
			sdp += 2;
			break;
		default:
			sdp = 0;
			break;
		}
	}
	if(match("rd", sdp->sdf_type)) {
		unit = line[3] - '0';	/* rd(#,0)unix */
		if(unit >= 4)
			return(0);
		switch(ra_drv[dp->dv_cn][unit].ra_dt) {
		case RD51:
			break;
		case RD31:
			sdp += 2;
			break;
		case RD32:
		case RD52:
		case RD53:
		case RD54:
			sdp++;
			break;
		default:
			sdp = 0;
			break;
		}
	}
	return(sdp);
}

/*
 * gktape - generic kernel load device select routine.
 *
 * The generic kernel has all possible system disks and
 * magtapes configured. This routine selects the load device.
 *
 * If boot was loaded by sdload magtape ID is in sdl_ldn.
 * Otherwise, ask the user to select the load device.
 *
 * TODO:
 *	Should set up the crash dump device. This will require all
 *	crash dump device code be configured into the generic kernel,
 *	and boot will need a duplicate of the crash dump disk table
 *	in sysgen (sg_tab.c).
 */

gktape()
{
	register struct devsw *dp;
	register int i;

	ld_bmaj = -1;				/* say no load device */
	if(nl[X_GENERIC].n_value == 0)
	    return;				/* not a generic kernel */
	if(sdl_bdn) {				/* boot loaded by sdload */
	    gline[0] = sdl_ldn & 0177;
	    gline[1] = (sdl_ldn >> 8) & 0177;
	    gline[2] = '\0';
	    if((dp = (struct devsw *)dp_get(&gline)) != -1)
		if((dp->dv_flags&DV_TAPE) || (dp->dv_flags&DV_RX) ||
		   (dp->dv_flags&DV_RC) || (dp->dv_flags&DV_RL))
		    ld_bmaj = dp->dv_bmaj;
	} else {				/* ask user to select load dev */
	    while(1) {
		printf("\nLoad device (? for help, <RETURN> if none)");
		printf(" < ht tm ts tk rx rl rc > ? ");
		gets(gline);
		if(gline[0] == '\0')
		    return;			/* user says no load dev */
		if(match("?", gline) || match("help", gline)) {
		    for(i=0; gkt_help[i]; i++)
			printf("\n%s", gkt_help[i]);
		    continue;
		}
		if((dp = (struct devsw *)dp_get(&gline)) == -1) {
		    printf("\n%s - bad device!\n", gline);
		    continue;
		}
		if(((dp->dv_flags&DV_TAPE)==0) && ((dp->dv_flags&DV_RX)==0) &&
		   ((dp->dv_flags&DV_RC)==0) && ((dp->dv_flags&DV_RL)==0)) {
		    printf("\n%s - invalid load device!\n", gline);
		    continue;
		}
		ld_bmaj = dp->dv_bmaj;
		break;
	    }
	}
	ldp = dp;	/* save devsw entry pointer */
}

/*
 * Routine to ZAP the second MSCP disk controller in it will not
 * be used. Set nuda to 0, nra[1] to 0, ra_ctid[1] to -1.
 * Set the CSR address in io_csr to 0160000 (never responds).
 *
 * Unions for dealing with character arrays nra[] & ra_ctid[].
 * Assumes byte ordering, but that is just though!
 *
 * (addr) is address of 2nd cntlr's entry in io_csr[] array.
 */

zapra2(addr)
{
	register int i;
	union {
		int	w[2];
		char	c[4];
	} nra;
	
	union {
		int	w[2];
		char	c[4];
	} ra_ctid;

	mtpi(0160000, addr);		/* zap CSR address (no response) */
	i = mfpi(nl[X_NUDA].n_value);	/* get current value of _nuda */
	if(i > 1)			/* if more than one cntlr configed */
	    mtpi(1, nl[X_NUDA].n_value);	/* clobber all but first */
	nra.w[0] = mfpi(nl[X_NRA].n_value);	/* zap nra[1] to zero */
	nra.w[1] = mfpi(nl[X_NRA].n_value + sizeof(int));
	nra.c[1] = 0;
	mtpi(nra.w[0], nl[X_NRA].n_value);
	mtpi(nra.w[1], nl[X_NRA].n_value + sizeof(int));
	ra_ctid.w[0] = mfpi(nl[X_RA_CTID].n_value);	/* zap ra_ctid[1] to -1 */
	ra_ctid.w[1] = mfpi(nl[X_RA_CTID].n_value + sizeof(int));
	ra_ctid.c[1] = 0377;
	mtpi(ra_ctid.w[0], nl[X_RA_CTID].n_value);
	mtpi(ra_ctid.w[1], nl[X_RA_CTID].n_value + sizeof(int));
}
