#include <sys/param.h>
#include <sys/systm.h>
#include <sys/buf.h>
#include <sys/tty.h>
#include <sys/conf.h>
#include <sys/proc.h>
#include <sys/text.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/file.h>
#include <sys/inode.h>
#include <sys/acct.h>
#include <sys/mount.h>
#include <sys/map.h>
#include <sys/callo.h>
#include <sys/mbuf.h>
#include "dds.h"

int	nulldev();
int	nodev();

int fpemulation = 2;

/*
 * Dummy function required if floating point simulator
 * is not used.
 */

fptrap()
{
	return(SIGILL);
}


/*
 * Utsname definition
 */

#include <sys/utsname.h>
struct utsname utsname = {
	{'U','L','T','R','I','X','-','1','1'},
	{''},
	{'3'},
	{'0'},
	{''},
};


#include <sys/maus.h>
struct mausmap mausmap[] = {
	0,	2,
	2,	128,
	130,	128,
	258,	128,
	-1,	-1
};
int	raopen(), raclose(), rastrategy();
struct	buf	ratab;
int	rlopen(), rlclose(), rlstrategy();
struct	buf	rltab;
int	htopen(), htclose(), htstrategy();
struct	buf	httab;
int	hpopen(),hpclose(), hpstrategy();
struct	buf	hptab[];
struct	bdevsw	bdevsw[] =
{
	nodev, nodev, nodev, 0, /* rk = 0 */
	nodev, nodev, nodev, 0, /* rp = 1 */
	raopen, raclose, rastrategy, &ratab,	/* ra = 2 */
	rlopen, rlclose, rlstrategy, &rltab,	/* rl = 3 */
	nodev, nodev, nodev, 0, /* hx = 4 */
	nodev, nodev, nodev, 0, /* tm = 5 */
	nodev, nodev, nodev, 0, /* tk = 6 */
	nodev, nodev, nodev, 0, /* ts = 7 */
	htopen, htclose, htstrategy, &httab,	/* ht = 8 */
	hpopen, hpclose, hpstrategy, &hptab[0],	/* hp = 9 */
	hpopen, hpclose, hpstrategy, &hptab[1],	/* hm = 10 */
	nodev, nodev, nodev, 0, /* hj = 11 */
	nodev, nodev, nodev, 0, /* hk = 12 */
	nodev, nodev, nodev, 0, /* u1 = 13 */
	nodev, nodev, nodev, 0, /* u2 = 14 */
	nodev, nodev, nodev, 0, /* u3 = 15 */
	nodev, nodev, nodev, 0, /* u4 = 16 */
	0
};

int	klopen(), klclose(), klread(), klwrite(), klioctl();
int	lpopen(), lpclose(), lpwrite(), lpioctl();
int	dhopen(), dhclose(), dhread(), dhwrite(), dhioctl(), dhstop();

int	dzopen(), dzclose(), dzread(), dzwrite(), dzioctl();
int	syopen(), syread(), sywrite(), sysioctl(), syselect();
int	mmread(), mmwrite();
int	ptcopen(), ptcclose(), ptcread(), ptcwrite(), ptyioctl(), ptcselect();
int	ptsopen(), ptsclose(), ptsread(), ptswrite(), ptsstop();
int	raread(), rawrite();
int	rlread(), rlwrite();
int	htread(), htwrite(), htioctl();
int	hpread(), hpwrite();

int	nkl11 = 1;
int	ndl11 = 0;
struct	tty	*kl11[1+0];

struct	tty	*dh11[16];
int	dh_local[1];
char	dhcc[16];
int	dhchars[1];
int	dhsar[1];
int	ndh11 = 16;

struct	tty	*dz_tty[24];
char	dz_local[3];
char	dz_brk[3];
int	dz_cnt = 24;
char	dz_shft = 3;
char	dz_mask = 7;

int	ntty = 41;
struct	tty	tty_ts[41];

int	npty = 10;
struct	pt_ioctl	pt_ioctl[10];
struct	tty	pty_ts[10];
struct	tty	*pt_tty[] = {
	&pty_ts[0],
	&pty_ts[1],
	&pty_ts[2],
	&pty_ts[3],
	&pty_ts[4],
	&pty_ts[5],
	&pty_ts[6],
	&pty_ts[7],
	&pty_ts[8],
	&pty_ts[9],
};

netattach()
{
	deattach(0, 0174510, 0120);
}

int ttselect(), seltrue();

struct	cdevsw	cdevsw[] =
{
	klopen,   klclose,  klread,   klwrite,
	klioctl,  nulldev,  kl11,     ttselect,	/* console = 0 */
	nodev,    nodev,    nodev,    nodev,
	nodev,    nulldev,  0,        nodev,	/* ct = 1 */
	lpopen,   lpclose,  nodev,    lpwrite,
	lpioctl,  nulldev,  0,        seltrue,	/* lp = 2 */
	nodev,    nodev,    nodev,    nodev,
	nodev,    nulldev,  0,        nodev,	/* dc = 3 */
	dhopen,   dhclose,  dhread,   dhwrite,
	dhioctl,  dhstop,   dh11,     ttselect,	/* dh = 4 */
	nodev,    nodev,    nodev,    nodev,
	nodev,    nulldev,  0,        nodev,	/* dp = 5 */
	nodev,    nodev,    nodev,    nodev,
	nodev,    nulldev,  0,        nodev,	/* uh|uhv = 6 */
	nodev,    nodev,    nodev,    nodev,
	nodev,    nulldev,  0,        nodev,	/* dn = 7 */
	dzopen,   dzclose,  dzread,   dzwrite,
	dzioctl,  nulldev,  dz_tty,   ttselect,	/* dz = 8 */
	nodev,    nodev,    nodev,    nodev,
	nodev,    nulldev,  0,        nodev,	/* du = 9 */
	syopen,   nulldev,  syread,   sywrite,
	sysioctl, nulldev,  0,        syselect,	/* tty = 10 */
	nulldev,  nulldev,  mmread,   mmwrite,
	nodev,    nulldev,  0,        seltrue,	/* mem = 11 */
	ptcopen,  ptcclose, ptcread,  ptcwrite,
	ptyioctl, nulldev,  pt_tty,   ptcselect,	/* ptc = 12 */
	ptsopen,  ptsclose, ptsread,  ptswrite,
	ptyioctl, ptsstop,  pt_tty,   ttselect,	/* pts = 13 */
	nodev,    nodev,    nodev,    nodev,
	nodev,    nulldev,  0,        nodev,	/* dummy = 14 */
	nodev,    nodev,    nodev,    nodev,
	nodev,    nulldev,  0,        nodev,	/* dummy = 15 */
	nodev,    nodev,    nodev,    nodev,
	nodev,    nulldev,  0,        nodev,	/* dummy = 16 */
	nodev,    nodev,    nodev,    nodev,
	nodev,    nulldev,  0,        nodev,	/* rk = 17 */
	nodev,    nodev,    nodev,    nodev,
	nodev,    nulldev,  0,        nodev,	/* rp = 18 */
	raopen,   nulldev,  raread,   rawrite,
	nodev,    nulldev,  0,        seltrue,	/* ra = 19 */
	rlopen,   nulldev,  rlread,   rlwrite,
	nodev,    nulldev,  0,        seltrue,	/* rl = 20 */
	nodev,    nodev,    nodev,    nodev,
	nodev,    nulldev,  0,        nodev,	/* hx = 21 */
	nodev,    nodev,    nodev,    nodev,
	nodev,    nulldev,  0,        nodev,	/* tm = 22 */
	nodev,    nodev,    nodev,    nodev,
	nodev,    nulldev,  0,        nodev,	/* tk = 23 */
	nodev,    nodev,    nodev,    nodev,
	nodev,    nulldev,  0,        nodev,	/* ts = 24 */
	htopen,   htclose,  htread,   htwrite,
	htioctl,    nulldev,  0,        seltrue,	/* ht = 25 */
	hpopen,   nulldev,  hpread,   hpwrite,
	nodev,    nulldev,  0,        seltrue,	/* hp = 26 */
	hpopen,   nulldev,  hpread,   hpwrite,
	nodev,    nulldev,  0,        seltrue,	/* hm = 27 */
	nodev,    nodev,    nodev,    nodev,
	nodev,    nulldev,  0,        nodev,	/* hj = 28 */
	nodev,    nodev,    nodev,    nodev,
	nodev,    nulldev,  0,        nodev,	/* hk = 29 */
	nodev,    nodev,    nodev,    nodev,
	nodev,    nulldev,  0,        nodev,	/* u1 = 30 */
	nodev,    nodev,    nodev,    nodev,
	nodev,    nulldev,  0,        nodev,	/* u2 = 31 */
	nodev,    nodev,    nodev,    nodev,
	nodev,    nulldev,  0,        nodev,	/* u3 = 32 */
	nodev,    nodev,    nodev,    nodev,
	nodev,    nulldev,  0,        nodev,	/* u4 = 33 */
	0
};

int	ttstart();
int	ntyclose(), ntread(), ntwrite(), ntyinput();
int	ttioctl();
struct	linesw	linesw[] =
{
	nulldev, nulldev, ntread, ntwrite, ttioctl,
	ntyinput, nodev, nulldev, ttstart, nulldev,	/* 0 */
	nulldev, ntyclose, ntread, ntwrite, ttioctl,
	ntyinput, nodev, nulldev, ttstart, nulldev,	/* 1 */
	0
};
int	rootdev	= makedev(9, 0);
int	swapdev	= makedev(9, 2);
int	pipedev = makedev(9, 0);
int	el_dev = makedev(26, 2);
int	nldisp = 2;
daddr_t	swplo	= 200;
int	nswap	= 6070;
daddr_t	el_sb = 0;
int	el_nb = 200;
	
struct	buf	buf[NBUF];
struct	inode	inode[150];
struct	mount	mount[12];
struct	buf	bfreelist;
struct	acct	acctbuf;
struct	inode	*acctp;
struct	map	coremap[93];
struct	map	swapmap[93];
int	ub_end;
#ifndef	UCB_CLIST
struct	cblock	cfree[65];
#else	UCB_CLIST
struct	cblock	*cfree;
#endif	UCB_CLIST
char	msgbuf[128];
	
/*
 * System table sizes, used by commands like
 * ps & pstat to free them from param.h.
 */

int	nbuf	= NBUF;
int	nproc	= NPROC;
int	ninode	= 150;
int	ntext	= NTEXT;
int	nofile	= NOFILE;
int	nsig	= NSIG;
int	nfile	= NFILE;
int	nmount	= 12;
int	mapsize	= 93;
int	ncall	= 40;
int	nclist	= 65;
int	elbsiz	= 350;
int	maxuprc = 25;
int	timezone = (5*60);
int	dstflag = 1;
int	ncargs = 5120;
int	hz = 60;
int	msgbufs = 128;
unsigned int maxseg = 16384;
long	cdlimit = 65536;

/*
 * Number of disk drives and 
 * related structures.
 */

int	nrl	= 2;
struct	ios	rl_ios[2];
struct	buf	rlutab[2];

/*
 * Number of tape drives 
 * and related arrays.
 */

int	nht	= 2;
u_short	ht_flags[2];
char	ht_openf[2];
daddr_t	ht_blkno[2];
daddr_t	ht_nxrec[2];
u_short	ht_erreg[2];
u_short	ht_dsreg[2];
short	ht_resid[2];
struct	buf	chtbuf[2];

/*
 * MSCP disk controller CSR and VECTOR addresses.
 */

#if NUDA > 0
int	ra_csr[] = {C0_CSR, C1_CSR, C2_CSR, C3_CSR};
int	ra_ivec[] = {C0_VEC, C1_VEC, C2_VEC, C3_VEC};
#else
int	ra_csr[] = {0,0,0,0};
#endif

/*
 * TS magtape controller CSR and VECTOR addresses.
 */

#if NTS > 0
int	ts_csr[] = {S0_CSR, S1_CSR, S2_CSR, S3_CSR};
#else
int	ts_csr[] = {0,0,0,0};
#endif

/*
 * TMSCP magtape controller CSR and VECTOR addresses.
 */

#if NTK > 0
int	tk_csr[] = {K0_CSR, K1_CSR, K2_CSR, K3_CSR};
int	tk_ivec[] = {K0_VEC, K1_VEC, K2_VEC, K3_VEC};
#else
int	tk_csr[] = {0,0,0,0};
#endif

/*
 * I/O device CSR addresses
 * Indexed by RAW major device number
 */

int	io_csr[] =
{

	0177560,	/* console	CSR address */
	0167770,	/* ct		CSR address */
	0177514,	/* lp		CSR address */
	0174000,	/* dc		CSR address */
	0160020,	/* dh		CSR address */
	0174770,	/* dp		CSR address */
	0160440,	/* uh		CSR address */
	0175200,	/* dn		CSR address */
	0160110,	/* dz		CSR address */
	0,		/* dummy	CSR address */
	0,		/* dummy	CSR address */
	0,		/* dummy	CSR address */
	0,		/* dummy	CSR address */
	0,		/* dummy	CSR address */
	0,		/* dummy	CSR address */
	0,		/* dummy	CSR address */
	0,		/* dummy	CSR address */
	0177400,	/* rk		CSR address */
	0176710,	/* rp		CSR address */
	&ra_csr[0],	/* ra		CSR address */
	0174400,	/* rl		CSR address */
	0177170,	/* hx		CSR address */
	0172520,	/* tm		CSR address */
	&tk_csr[0],	/* tk		CSR address */
	&ts_csr[0],	/* ts		CSR address */
	0172440,	/* ht		CSR address */
	0176700,	/* hp		CSR address */
	0176400,	/* hm		CSR address */
	0176400,	/* hj		CSR address */
	0177440,	/* hk		CSR address */
	0164000,	/* u1		CSR address */
	0164000,	/* u2		CSR address */
	0164000,	/* u3		CSR address */
	0164000,	/* u4		CSR address */
	0170500,	/* dhdm		CSR address */
	0176500,	/* kl		CSR address */
	0175610,	/* dl		CSR address */
};

/*
 * I/O device bus addr ext. register offset.
 * Zero if no BAE register. Index is BLOCK major dev.
 */

char	io_bae[] =
{
	0,	/* rk */
	0,	/* rp */
	0,	/* ra MBZ */
	010,	/* rl */
	0,	/* hx */
	0,	/* tm */
	0,	/* tk */
	0,	/* ts */
	034,	/* ht */
	050,	/* hp */
	050,	/* hm */
	050,	/* hj */
	0,	/* hk */
	0,	/* u1 MBZ */
	0,	/* u2 MBZ */
	0,	/* u3 MBZ */
	0,	/* u4 MBZ */
};

int	allocs[2900];
int	nwords = 2900;
int	miosize = 0;
int	mbsize = (60*MSIZE)+0;
