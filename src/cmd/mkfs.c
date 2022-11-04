
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * Make a file system prototype.
 * usage: /etc/mkfs filsys proto/size [ [disk cpu] or [m n] ] [fsname volname]
 *
 * Modified for ULTRIX-11
 *
 * Fred Canter
 *
 * Due to file system changes (i.e., 1024 byte logical block size),
 * the free list interleaving factors are now double the values in
 * the table ((f_m+1)/2 and f_n/2), with the following exceptions:
 *
 *	If the value of f_m is one.
 *	RQDX2 - RD52/RD53/RD54: f_m = 2	(j11 class CPUs only)
 *	RQDX3 - RD52/RD53/RD54: f_m = 7	"	"	"
 *
 * This means mkfs must know the type of disk controller.
 * The operating system version of mkfs looks at ra_ctid in the kernel.
 * The stand-alone version gets the controller type from a magic
 * physical location (RQ_CTID = 0140006), where it is saved by the
 * RA disk driver. Mkfs must zero this location before opening the disk.
 */
static char Sccsid[] = "@(#)mkfs.c 3.1 3/26/87";
#define	NIPB	(BSIZE/sizeof(struct dinode))
#define	NINDIR	(BSIZE/sizeof(daddr_t))
#define	NDIRECT	(BSIZE/sizeof(struct direct))
#ifndef STANDALONE
#define	NORMAL	0
#define	FATAL	1
#include <stdio.h>
#include <a.out.h>
#else
#include "/usr/sys/sas/sa_defs.h" /* exit status codes (NORMAL = 0, FATAL = 1) */
#endif
#include <sys/param.h>
#include <sys/ino.h>
#include <sys/inode.h>
#include <sys/filsys.h>
#include <sys/fblk.h>
#include <sys/dir.h>
#ifndef STANDALONE
#include <sys/stat.h>
#include <sys/devmaj.h>
#include <sys/ra_info.h>
#else
#include "/usr/sys/sas/ra_saio.h" /* stand-alone RA disk defines */
#endif
#ifdef	UCB_NKB
#define	MAXFN	357	/* RA81 has 714 blocks per cylinder */
#define	DEFFN	250	/* default value for blocks per cylinder */
#define	DEFFM	5	/* default value for interleave factor (m) */
#else
#define	MAXFN	714	/* RA81 has 714 blocks per cylinder */
#define	DEFFN	500	/* default value for blocks per cylinder */
#define	DEFFM	9	/* default value for interleave factor (m) */
#endif	UCB_NKB
/*
 * Following caused a "multiply defined"
 * since itoo is defined in <param.h>
 * Fred Canter
#ifndef	UCB_NKB
#define	itoo(x)	(int)((x+15)&07)
#endif	UCB_NKB
 *
 */

#ifdef UCB_NKB
#define	LADDR	(NADDR-3)
#else
#define	LADDR	10
#endif

time_t	utime;
#ifndef STANDALONE
FILE 	*fin;
struct	stat statb;
#else
int	fin;
#endif
int	fsi;
int	fso;
char	*charp;
char	buf[BSIZE];
union {
	struct fblk fb;
	char pad1[BSIZE];
} fbuf;
#ifndef STANDALONE
struct exec head;
#endif
char	string[50];
union {
	struct filsys fs;
	char pad2[BSIZE];
} filsys;
char	*fsys;
char	*proto;
char	*mstrg;
char	*nstrg;
char	*dsktyp;
char	*cputyp;
int	f_n	= DEFFN;
int	f_m	= DEFFM;	/* was 3, that was much to small */
int	error;
ino_t	ino;
long	getnum();
daddr_t	alloc();

#ifndef	STANDALONE

struct	nlist	nl[] =
{
	{ "_ra_ctid" },
	{ "" },
};

struct stat ra_statb;
int	ra_cntlr = -1;	/* MSCP cntlr #, -1 if none */
char	ra_ctid[] = {0377, 0377, 0377, 0377};

#endif

struct	{			/* disk types by generic name */
	char	*dkname;	/* disk type name, rp04, ra80, etc. */
	int	dkindx;		/* index into FS param array */
} dktype[] = {
	{ "rk05", 0 },
	{ "rl01", 1 },
	{ "rl02", 1 },
	{ "rk06", 2 },
	{ "rk07", 2 },
	{ "rp02", 3 },
	{ "rp03", 3 },
	{ "rm02", 4 },
	{ "rm03", 5 },
	{ "rm05", 6 },
	{ "rp04", 7 },
	{ "rp05", 7 },
	{ "rp06", 7 },
	{ "ra60", 8 },
	{ "ra80", 9 },
	{ "ra81", 10 },
	{ "rx33", 11 },
	{ "rx50", 11 },
	{ "rd31", 12 },
	{ "rd32", 12 },
	{ "rd51", 12 },
	{ "rx02", 13 },
	{ "rd52", 12 },
	{ "rd53", 12 },
	{ "rd54", 12 },
	{ "rc25", 14 },
	{ "ml11", 11 },
	{ 0, -1 },
};

struct	{			/* File system parameters M/N by cpu tpye */
	int	fsp_cpu;	/* CPU type, 23, 24, 70, etc. */
	int	fsp_m[15];	/* m - interleave factor */
	int	fsp_n[15];	/* n - blocks per cylinder */
} fsparam[] = {
/*
 * FOREIGN - disk not expected on this CPU
 *           m/n are guess work.
 */
/*
 * NOTE - for RC25 n = 62. 31 is the correct value but mkfs
 *	  does not like odd numbers. 62 will work just fine!
 */
/*
 * FOREIGN - rk06/7, rp02/3, rm02/3/5, rp04/5/6, ra60, ra80, ra81
 */
{ 23,
	/* RD31/RD32/RD51/RX50/RX33/RX02 (m = 1) hardware does interleaving */
   8,  13,  15,   8,  21,  31,  31,  21,  42,  31,  51,   1,   1,   1,	30,
  24,  20,  66, 200, 160, 160, 608, 418, 168, 434, 714,  10,  72,  13,	62,
},

/*
 * FOREIGN - rm03/5, rd51, rd52
 */
{ 24,
   7,  12,  14,   7,  20,  30,  30,  20,  42,  31,  51,   1,   1,   1,	30,
  24,  20,  66, 200, 160, 160, 608, 418, 168, 434, 714,  10,  72,  13,	62,
},

/*
 * FOREIGN - rm03/5, rd51, rd52
 */
{ 34,
   6,  11,  12,   6,  16,  24,  24,  16,  34,  25,  41,   1,   1,   1,	25,
  24,  20,  66, 200, 160, 160, 608, 418, 168, 434, 714,  10,  72,  13,	62,
},

/*
 * FOREIGN - rm03/5, rd51, rd52
 */
{ 40,
   6,  11,  12,   6,  16,  24,  24,  16,  34,  25,  41,   1,   1,   1,	25,
  24,  20,  66, 200, 160, 160, 608, 418, 168, 434, 714,  10,  72,  13,	62,
},

/*
 * FOREIGN - rm03, rm05, rx50, rd51, rd52
 */
{ 44,
   4,   7,   8,   4,  11,  16,  16,  11,  23,  17,  28,   1,   1,   1,	17,
  24,  20,  66, 200, 160, 160, 608, 418, 168, 434, 714,  10,  72,  13,	62,
},

/*
 * FOREIGN - rm03/5, rd51, rd52
 */
{ 45,
   5,   9,  10,   5,  14,  21,  21,  14,  30,  22,  36,   1,   1,   1,	22,
  24,  20,  66, 200, 160, 160, 608, 418, 168, 434, 714,  10,  72,  13,	62,
},

/*
 * FOREIGN - rk06/7, rp02/3, rm02/3/5, rp04/5/6, ra60, ra80, ra81
 * ****** (used 11/44 values for m/n) ******
 */
{ 53,
   4,   7,   8,   4,  11,  16,  16,  11,  23,  17,  28,   1,   1,   1,	17,
  24,  20,  66, 200, 160, 160, 608, 418, 168, 434, 714,  10,  72,  13,	62,
},

/*
 * FOREIGN - rm03/5, rd51, rd52
 */
{ 55,
   5,   9,  10,   5,  14,  21,  21,  14,  30,  22,  36,   1,   1,   1,	22,
  24,  20,  66, 200, 160, 160, 608, 418, 168, 434, 714,  10,  72,  13,	62,
},

/*
 * FOREIGN - rm03/5, rd51, rd52
 */
{ 60,
   5,   9,  10,   5,  14,  21,  21,  14,  30,  22,  36,   1,   1,   1,	22,
  24,  20,  66, 200, 160, 160, 608, 418, 168, 434, 714,  10,  72,  13,	62,
},

/*
 * FOREIGN - rd51, rd52
 */
{ 70,
   4,   6,   6,   4,   9,  13,  13,   9,  19,  14,  23,   1,   1,   1,	14,
  24,  20,  66, 200, 160, 160, 608, 418, 168, 434, 714,  10,  72,  13,	62,
},
/*
 * FOREIGN - rk06/7, rp02/3, rm02/3/5, rp04/5/6, ra60, ra80, ra81
 * ****** (used 11/44 values for m/n) ******
 */
{ 73,
   4,   7,   8,   4,  11,  16,  16,  11,  23,  17,  28,   1,   1,   1,	17,
  24,  20,  66, 200, 160, 160, 608, 418, 168, 434, 714,  10,  72,  13,	62,
},
/*
 * FOREIGN - rk06/7, rp02/3, rm02/3/5, rp04/5/6, ra60, ra80, ra81
 * ****** (used 11/44 values for m/n) ******
 *	M is probably too high, but that's ok!
 */
{ 83,
   4,   7,   8,   4,  11,  16,  16,  11,  23,  17,  28,   1,   1,   1,	17,
  24,  20,  66, 200, 160, 160, 608, 418, 168, 434, 714,  10,  72,  13,	62,
},
/*
 * FOREIGN - rm03/5, rd51, rd52
 * ****** (used 11/70 values for m/n) ******
 */
{ 84,
   4,   6,   6,   4,   9,  13,  13,   9,  19,  14,  23,   1,   1,   1,	14,
  24,  20,  66, 200, 160, 160, 608, 418, 168, 434, 714,  10,  72,  13,	62,
},
{ 0 },

};

#ifdef	STANDALONE
int	argflag;	/* 0=interactive, 1=called by SDLOAD, see prf.c */
#endif

main(argc, argv)
char *argv[];
{
	int a3num, a4num;
	int f, c, i, j;
	long n;
	int mem, maj, ct;

#ifndef STANDALONE
	time(&utime);
	if((argc < 3) || (argc > 7) || (argc == 4) || (argc == 6)) {
	usage:
		printf("\nusage: /etc/mkfs filsys proto/size");
		printf(" [ [disk cpu] or [m n] ] [fsname volname]\n");
		exit(FATAL);
	}
	fsys = argv[1];
	proto = argv[2];
/*
 * Find out whether arg3 and arg4 are alpha or numeric,
 * this tells mkfs if they represent [disk cpu],
 * [m n], and/or [filsys volume].
 */
	if((argv[3][0] >= '0') && (argv[3][0] <= '9'))
		a3num = 1;
	else
		a3num = 0;
	if((argv[4][0] >= '0') && (argv[4][0] <= '9'))
		a4num = 1;
	else
		a4num = 0;
	/*
	 * Get ra_ctid (MSCP cntlr type ID) from kernel.
	 * If we can't find cntlr type, leave it blank so
	 * default f_m & f_n values will be used.
	 */
	while(1) {
		if(nlist("/unix", nl) < 0)
			break;
		if(nl[0].n_value == 0)
			break;
		if((mem = open("/dev/mem", 0)) < 0)
			break;
		lseek(mem, (long)nl[0].n_value, 0);
		read(mem, (char *)&ra_ctid, sizeof(ra_ctid));
		break;
	}
#else
	{
		static char protos[60];
		static char mstrg[60];
		static char nstrg[60];
		static char dtstrg[60];
		static char ctstrg[60];
		static char fsname[60];
		static char voln[60];

		printf("File system size: ");
		gets(protos);
		proto = protos;
	sf1:
		printf("Disk type: ");
		gets(dtstrg);
		dsktyp = dtstrg;
		if(*dsktyp == 0)
			goto sf3;	/* typed return, go ask for m/n */
		for(i=0; dktype[i].dkname; i++)
			if(strcmp(dktype[i].dkname, dsktyp) == 0)
				break;
		if(dktype[i].dkindx < 0)
			goto sf1;
	sf2:
		printf("Processor type: ");
		gets(ctstrg);
		cputyp = atoi(ctstrg);
		for(j=0; fsparam[j].fsp_cpu; j++)
			if(fsparam[j].fsp_cpu == cputyp)
				break;
		if(fsparam[j].fsp_cpu == 0)
			goto sf2;
		f_m = fsparam[j].fsp_m[dktype[i].dkindx];
		f_n = fsparam[j].fsp_n[dktype[i].dkindx];
#ifdef	UCB_NKB
		if(f_m != 1) {
			f_m = (f_m + 1)/2;
			if((f_n & 1) == 0)
				f_n /= 2;
		}
#endif	UCB_NKB
		if((f_m == 0) || (f_n == 0))
/* NOTREACHED */	printf("disk and cpu not compatible\n");
		else
			goto sf4;
	sf3:
		printf("Interleave factor: ");
		gets(mstrg);
		f_m = atoi(mstrg);
		printf("Blocks per cylinder: ");
		gets(nstrg);
		f_n = atoi(nstrg);
	sf4:
		printf("File system name: ");
		gets(fsname);
		if(strlen(fsname) > 6)
			goto sf4;
	sf5:
		printf("Volume name: ");
		gets(voln);
		if(strlen(voln) > 6)
			goto sf5;
		if(fsname[0] != '\0')
			strncpy(filsys.fs.s_fname, fsname, 6);
		if(voln[0] != '\0')
			strncpy(filsys.fs.s_fpack, voln, 6);
		if(f_n <= 0 || f_n > MAXFN)
			f_n = DEFFN;
		if(f_m <= 0 || f_m > f_n)
			f_m = DEFFM;	/* 3 was much too small */
	}
#endif
#ifdef STANDALONE
	{
		char fsbuf[100];

		RQ_CTID->r[0] = 0;	/* clear magic loc, only once */
		do {
			printf("File system: ");
			gets(fsbuf);
			fso = open(fsbuf, 1);
			fsi = open(fsbuf, 0);
			if(argflag && ((fso < 0) || (fsi < 0)))
				exit(FATAL);
		} while (fso < 0 || fsi < 0);
		/* adjust f_m for RD52/RD53/RD54, for faster Qbus CPUs */
		while(1) {
			if(*dsktyp == 0)	/* user supplied f_m */
				break;
			if((cputyp!=53) && (cputyp!=73) && (cputyp!=83))
				break;		/* slower CPU */
			if((strcmp(dsktyp, "rd52") == 0) ||
			   (strcmp(dsktyp, "rd53") == 0) ||
			   (strcmp(dsktyp, "rd54") == 0)) {
				if(RQ_CTID->r[0] == RQDX1)
					f_m = 2;
				if(RQ_CTID->r[0] == RQDX3)
					f_m = 7;
			}
			break;
		}
	}
	fin = NULL;
	argc = 0;
#else
	fso = creat(fsys, 0666);
	if(fso < 0) {
		printf("%s: cannot create\n", fsys);
		exit(FATAL);
	}
	fsi = open(fsys, 0);
	if(fsi < 0) {
		printf("%s: cannot open\n", fsys);
		exit(FATAL);
	}
	fin = fopen(proto, "r");
	/*
	 * Stat fsys, if it is a special file
	 * for an MSCP disk, get and save controller number.
	 * Cntlr # in bits 6 & 7 of minor device.
	 * Dont't need error checking!
	 */
	while(1) {
		if(stat(fsys, &ra_statb) < 0)
			break;
		maj = (ra_statb.st_rdev >> 8) & 0377;
		if(((ra_statb.st_mode&S_IFMT) == S_IFBLK) && (maj == RA_BMAJ))
			ra_cntlr = (ra_statb.st_rdev >> 6) & 3;
		if(((ra_statb.st_mode&S_IFMT) == S_IFCHR) && (maj == RA_RMAJ))
			ra_cntlr = (ra_statb.st_rdev >> 6) & 3;
		break;
	}
#endif
	if(fin == NULL) {
		n = 0;
		for(f=0; c=proto[f]; f++) {
			if(c<'0' || c>'9') {
				printf("%s: cannot open\n", proto);
				exit(FATAL);
			}
			n = n*10 + (c-'0');
		}
		filsys.fs.s_fsize = n;
		n = n/25;
		if(n <= 0)
			n = 1;
		if(n > 65500/NIPB)
			n = 65500/NIPB;
		filsys.fs.s_isize = n + 2;
		printf("isize = %D\n", n*NIPB);
		charp = "d--777 0 0 $ ";
		goto f3;
	}

#ifndef STANDALONE
	/*
	 * get name of boot load program
	 * and read onto block 0
	 * Fred Canter -- 8/12/85
	 *	Boot block must begin with 0407 or 0240,
	 *	ULTRIX-11 boot blocks begin with nop (0240).
	 *	Size of boot block must be <= 512 bytes.
	 */

	getstr();
	f = open(string, 0);
	if(f < 0) {
		printf("%s: cannot open\n", string);
		goto f2;
	}
	read(f, (char *)&head, sizeof head);
	if(head.a_magic == 0240) {	/* ULTRIX-11 boot block */
		fstat(f, &statb);
		if(statb.st_size > 512L)
			c = 513;	/* make size test fail */
		else
			c = (int)statb.st_size;
		lseek(f, 0L, 0);
	} else if(head.a_magic == A_MAGIC1) {
		c = head.a_text + head.a_data;
	} else {
		printf("%s: invalid boot block format\n", string);
		goto f1;
	}
	if(c > 512) {
		printf("%s: size exceeds 512 bytes\n", string);
		goto f1;
	}
	for(j=0; j<BSIZE; j++)
		buf[j] = 0;
	read(f, buf, c);
	wtfs((long)0, buf);

f1:
	close(f);

	/*
	 * get total disk size
	 * and inode block size
	 */

f2:
	filsys.fs.s_fsize = getnum();
	n = getnum();
	n /= NIPB;
	filsys.fs.s_isize = n + 3;

#endif
f3:
	if((argc != 5) && (argc != 7))
		goto f5;
	if(argc == 7) {
		if((strlen(argv[5]) > 6) || (strlen(argv[6]) > 6)) {
			printf("\nmkfs: fsname/volname 6 characters maximum\n");
#ifdef STANDALONE
			exit(FATAL);
#else
			goto usage;
#endif
		}
		strncpy(filsys.fs.s_fname, argv[5], 6);
		strncpy(filsys.fs.s_fpack, argv[6], 6);
	}
	if(a3num && a4num) {
		f_m = atoi(argv[3]);
		f_n = atoi(argv[4]);
	} else {
		dsktyp = argv[3];
		cputyp = atoi(argv[4]);
		for(i=0; dktype[i].dkname; i++)
			if(strcmp(dktype[i].dkname, dsktyp) == 0)
				break;
		if(dktype[i].dkindx < 0) {
			printf("\nmkfs: Unknown disk type\n");
			goto f4;	/* use default m/n */
		}
		for(j=0; fsparam[j].fsp_cpu; j++)
			if(fsparam[j].fsp_cpu == cputyp)
				break;
		if(fsparam[j].fsp_cpu == 0) {
			printf("\nmkfs: Unknown cpu type\n");
			goto f4;	/* use default m/n */
		}
		f_m = fsparam[j].fsp_m[dktype[i].dkindx];
		f_n = fsparam[j].fsp_n[dktype[i].dkindx];
#ifdef	UCB_NKB
		if(f_m != 1) {
			f_m = (f_m + 1)/2;
			if((f_n & 1) == 0)
				f_n /= 2;
		}
#endif	UCB_NKB
		if((f_m == 0) || (f_n == 0))
/* NOTREACHED */	printf("\nmkfs: disk and cpu not compatible\n");
	}
f4:
	if(f_n <= 0 || f_n > MAXFN)
		f_n = DEFFN;
	if(f_m <= 0 || f_m > f_n)
		f_m = DEFFM;	/* 3 was much too small */
f5:
#ifndef	STANDALONE
	/*
	 * Adjust f_m for RQDX - RD52/RD53/RD54 if faster Qbus CPU.
	 */
	while(1) {
		if(ra_cntlr < 0)
			break;	/* no MSCP cntlr on system */
		if((cputyp!=53) && (cputyp!=73) && (cputyp!=83))
			break;	/* not one of the faster Qbus CPUs */
		if((a3num && a4num) || (dktype[i].dkindx < 0))
			break;	/* unknown disk, use default f_m */
		if((strcmp(dsktyp, "rd52") == 0) ||
		   (strcmp(dsktyp, "rd53") == 0) ||
		   (strcmp(dsktyp, "rd54") == 0)) {
			if(((ra_ctid[ra_cntlr] >> 4) & 017) == RQDX1)
				f_m = 2;
			if(((ra_ctid[ra_cntlr] >> 4) & 017) == RQDX3)
				f_m = 7;
		}
		break;
	}
#endif
	filsys.fs.s_m = f_m;
	filsys.fs.s_n = f_n;
	printf("m/n = %d %d\n", f_m, f_n);
	if(filsys.fs.s_isize >= filsys.fs.s_fsize) {
	    printf("%D/%u: bad ratio\n",filsys.fs.s_fsize,filsys.fs.s_isize-2);
	    exit(FATAL);
	}
	filsys.fs.s_magic[0] = S_0MAGIC;
	filsys.fs.s_magic[1] = S_1MAGIC;
	filsys.fs.s_magic[2] = S_2MAGIC;
	filsys.fs.s_magic[3] = S_3MAGIC;
	filsys.fs.s_tfree = 0;
	filsys.fs.s_tinode = 0;
	for(c=0; c<BSIZE; c++)
		buf[c] = 0;
	for(n=2; n!=filsys.fs.s_isize; n++) {
		wtfs(n, buf);
		filsys.fs.s_tinode += NIPB;
	}
	ino = 0;

	bflist();

	cfile((struct inode *)0);

	filsys.fs.s_time = utime;
	wtfs((long)1, (char *)&filsys);
	exit(error);
}

cfile(par)
struct inode *par;
{
	struct inode in;
	int dbc, ibc;
	char db[BSIZE];
	daddr_t ib[NINDIR];
	int i, f, c;

	/*
	 * get mode, uid and gid
	 */

	getstr();
	in.i_mode = gmode(string[0], "-bcd", IFREG, IFBLK, IFCHR, IFDIR);
	in.i_mode |= gmode(string[1], "-u", 0, ISUID, 0, 0);
	in.i_mode |= gmode(string[2], "-g", 0, ISGID, 0, 0);
	for(i=3; i<6; i++) {
		c = string[i];
		if(c<'0' || c>'7') {
			printf("%c/%s: bad octal mode digit\n", c, string);
			error = FATAL;
			c = 0;
		}
		in.i_mode |= (c-'0')<<(15-3*i);
	}
	in.i_uid = getnum();
	in.i_gid = getnum();

	/*
	 * general initialization prior to
	 * switching on format
	 */

	ino++;
	in.i_number = ino;
	for(i=0; i<BSIZE; i++)
		db[i] = 0;
	for(i=0; i<NINDIR; i++)
		ib[i] = (daddr_t)0;
	in.i_nlink = 1;
	in.i_size = 0;
	for(i=0; i<NADDR; i++)
		in.i_addr[i] = (daddr_t)0;
	if(par == (struct inode *)0) {
		par = &in;
		in.i_nlink--;
	}
	dbc = 0;
	ibc = 0;
	switch(in.i_mode&IFMT) {

	case IFREG:
		/*
		 * regular file
		 * contents is a file name
		 */

		getstr();
		f = open(string, 0);
		if(f < 0) {
			printf("%s: cannot open\n", string);
			error = FATAL;
			break;
		}
		while((i=read(f, db, BSIZE)) > 0) {
			in.i_size += i;
			newblk(&dbc, db, &ibc, ib);
		}
		close(f);
		break;

	case IFBLK:
	case IFCHR:
		/*
		 * special file
		 * content is maj/min types
		 */

		i = getnum() & 0377;
		f = getnum() & 0377;
		in.i_addr[0] = (i<<8) | f;
		break;

	case IFDIR:
		/*
		 * directory
		 * put in extra links
		 * call recursively until
		 * name of "$" found
		 */

		par->i_nlink++;
		in.i_nlink++;
		entry(in.i_number, ".", &dbc, db, &ibc, ib);
		entry(par->i_number, "..", &dbc, db, &ibc, ib);
		in.i_size = 2*sizeof(struct direct);
		for(;;) {
			getstr();
			if(string[0]=='$' && string[1]=='\0')
				break;
			entry(ino+1, string, &dbc, db, &ibc, ib);
			in.i_size += sizeof(struct direct);
			cfile(&in);
		}
		break;
	}
	if(dbc != 0)
		newblk(&dbc, db, &ibc, ib);
	iput(&in, &ibc, ib);
}

gmode(c, s, m0, m1, m2, m3)
char c, *s;
{
	int i;

	for(i=0; s[i]; i++)
		if(c == s[i])
			return((&m0)[i]);
	printf("%c/%s: bad mode\n", c, string);
	error = FATAL;
	return(0);
}

long
getnum()
{
	int i, c;
	long n;

	getstr();
	n = 0;
	i = 0;
	for(i=0; c=string[i]; i++) {
		if(c<'0' || c>'9') {
			printf("%s: bad number\n", string);
			error = FATAL;
			return((long)0);
		}
		n = n*10 + (c-'0');
	}
	return(n);
}

getstr()
{
	int i, c;

loop:
	switch(c=getch()) {

	case ' ':
	case '\t':
	case '\n':
		goto loop;

	case '\0':
		printf("EOF\n");
		exit(FATAL);

	case ':':
		while(getch() != '\n');
		goto loop;

	}
	i = 0;

	do {
		string[i++] = c;
		c = getch();
	} while(c!=' '&&c!='\t'&&c!='\n'&&c!='\0');
	string[i] = '\0';
}

rdfs(bno, bf)
daddr_t bno;
char *bf;
{
	int n;

	lseek(fsi, bno*BSIZE, 0);
	n = read(fsi, bf, BSIZE);
	if(n != BSIZE) {
		printf("read error: %D\n", bno);
		exit(FATAL);
	}
}

wtfs(bno, bf)
daddr_t bno;
char *bf;
{
	int n;

	lseek(fso, bno*BSIZE, 0);
	n = write(fso, bf, BSIZE);
	if(n != BSIZE) {
		printf("write error: %D\n", bno);
		exit(FATAL);
	}
}

daddr_t
alloc()
{
	int i;
	daddr_t bno;

	filsys.fs.s_tfree--;
	bno = filsys.fs.s_free[--filsys.fs.s_nfree];
	if(bno == 0) {
		printf("out of free space\n");
		exit(FATAL);
	}
	if(filsys.fs.s_nfree <= 0) {
		rdfs(bno, (char *)&fbuf);
		filsys.fs.s_nfree = fbuf.fb.df_nfree;
		for(i=0; i<NICFREE; i++)
			filsys.fs.s_free[i] = fbuf.fb.df_free[i];
	}
	return(bno);
}

bfree(bno)
daddr_t bno;
{
	int i;

	if(bno != 0)
		filsys.fs.s_tfree++;
	if(filsys.fs.s_nfree >= NICFREE) {
		fbuf.fb.df_nfree = filsys.fs.s_nfree;
		for(i=0; i<NICFREE; i++)
			fbuf.fb.df_free[i] = filsys.fs.s_free[i];
		wtfs(bno, (char *)&fbuf);
		filsys.fs.s_nfree = 0;
	}
	filsys.fs.s_free[filsys.fs.s_nfree++] = bno;
}

entry(inum, str, adbc, db, aibc, ib)
ino_t inum;
char *str;
int *adbc, *aibc;
char *db;
daddr_t *ib;
{
	struct direct *dp;
	int i;

	dp = (struct direct *)db;
	dp += *adbc;
	(*adbc)++;
	dp->d_ino = inum;
	for(i=0; i<DIRSIZ; i++)
		dp->d_name[i] = 0;
	for(i=0; i<DIRSIZ; i++)
		if((dp->d_name[i] = str[i]) == 0)
			break;
	if(*adbc >= NDIRECT)
		newblk(adbc, db, aibc, ib);
}

newblk(adbc, db, aibc, ib)
int *adbc, *aibc;
char *db;
daddr_t *ib;
{
	int i;
	daddr_t bno;

	bno = alloc();
	wtfs(bno, db);
	for(i=0; i<BSIZE; i++)
		db[i] = 0;
	*adbc = 0;
	ib[*aibc] = bno;
	(*aibc)++;
	if(*aibc >= NINDIR) {
		printf("indirect block full\n");
		error = FATAL;
		*aibc = 0;
	}
}

getch()
{

#ifndef STANDALONE
	if(charp)
#endif
		return(*charp++);
#ifndef STANDALONE
	return(getc(fin));
#endif
}

bflist()
{
	struct inode in;
	daddr_t ib[NINDIR];
	int ibc;
	char flg[MAXFN];
	int adr[MAXFN];
	int i, j;
	daddr_t f, d;

	for(i=0; i<f_n; i++)
		flg[i] = 0;
	i = 0;
	for(j=0; j<f_n; j++) {
		while(flg[i])
			i = (i+1)%f_n;
		adr[j] = i+1;
		flg[i]++;
		i = (i+f_m)%f_n;
	}

	ino++;
	in.i_number = ino;
	in.i_mode = IFREG;
	in.i_uid = 0;
	in.i_gid = 0;
	in.i_nlink = 0;
	in.i_size = 0;
	for(i=0; i<NADDR; i++)
		in.i_addr[i] = (daddr_t)0;

	for(i=0; i<NINDIR; i++)
		ib[i] = (daddr_t)0;
	ibc = 0;
	bfree((daddr_t)0);
	d = filsys.fs.s_fsize-1;
	/*
	 * Fred Canter -- 7/13/85
	 *	Mkfs left the last block missing from the free list
	 *	whenever d was an even multiple of f_n.
	 */
	if(d%f_n == 0)
		d++;
	while(d%f_n)
		d++;
	for(; d > 0; d -= f_n)
	for(i=0; i<f_n; i++) {
		f = d - adr[i];
		if(f < filsys.fs.s_fsize && f >= filsys.fs.s_isize)
			if(badblk(f)) {
				if(ibc >= NINDIR) {
					printf("too many bad blocks\n");
					error = FATAL;
					ibc = 0;
				}
				ib[ibc] = f;
				ibc++;
			} else
				bfree(f);
	}
	iput(&in, &ibc, ib);
}

iput(ip, aibc, ib)
struct inode *ip;
int *aibc;
daddr_t *ib;
{
	struct dinode *dp;
	daddr_t d;
	int i;

	filsys.fs.s_tinode--;
	d = itod(ip->i_number);
	if(d >= filsys.fs.s_isize) {
		if(error == NORMAL)
			printf("ilist too small\n");
		error = FATAL;
		return;
	}
	rdfs(d, buf);
	dp = (struct dinode *)buf;
	dp += itoo(ip->i_number);

	dp->di_mode = ip->i_mode;
	dp->di_nlink = ip->i_nlink;
	dp->di_uid = ip->i_uid;
	dp->di_gid = ip->i_gid;
	dp->di_size = ip->i_size;
	dp->di_atime = utime;
	dp->di_mtime = utime;
	dp->di_ctime = utime;

	switch(ip->i_mode&IFMT) {

	case IFDIR:
	case IFREG:
		for(i=0; i<*aibc; i++) {
			if(i >= LADDR)
				break;
			ip->i_addr[i] = ib[i];
		}
		if(*aibc >= LADDR) {
			ip->i_addr[LADDR] = alloc();
			for(i=0; i<NINDIR-LADDR; i++) {
				ib[i] = ib[i+LADDR];
				ib[i+LADDR] = (daddr_t)0;
			}
			wtfs(ip->i_addr[LADDR], (char *)ib);
		}

	case IFBLK:
	case IFCHR:
		ltol3(dp->di_addr, ip->i_addr, NADDR);
		break;

	default:
		printf("bad mode %o\n", ip->i_mode);
		exit(FATAL);
	}
	wtfs(d, buf);
}

badblk(bno)
daddr_t bno;
{

	return(0);
}
