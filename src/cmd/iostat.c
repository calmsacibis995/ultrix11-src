
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * ULTRIX-11 iostat command (iostat.c)
 *
 * Prints statistics about disk I/O rates
 * and CPU times.
 *
 * Fred Canter
 *
 */
#include <a.out.h>
#include <sys/types.h>
#include <sys/ra_info.h>

static char Sccsid[] = "@(#)iostat.c 3.1 3/26/87";
int	bflg;
int	dflg;
int	tflg;
int	iflg;
int	aflg;
int	sflg;
int	dsflag;

#define	NSYMCK	6	/* # of symbols to check (must be in /unix) */
			/* check goes from nl[0] -> nl[NSYMCK-1] */
struct	nlist	nl[] = {
	{ "_cp_time" },
	{ "_io_info" },
	{ "_dk_iop" },
	{ "_dk_nd" },
	{ "_tk_nin" },
	{ "_tk_nout" },
	{ "_nuda" },
	{ "_nra" },
	{ "_ra_inde" },
	{ "_ra_drv" },
	{ "" },
};

/*
 * WARNING !,
 * DK_NC is defined here and in /usr/include/sys/systm.h,
 * if one is changed the other must also be changed.
 * There are currently 8 possible "normal" controllers
 * plus 4 possible MSCP controllers.
 */
#define DK_NC	12	/* number of possible disk controllers */

long	cp_t[5];
long	cp_t1[5];

/*
 * Iostat structures,
 * two for each possible drive.
 * Assumes 8 units per controller.
 */
struct	ios
{
	char	dk_tr;		/* drive xfer rate */
	char	dk_busy;	/* drive activity flag */
	long	dk_numb;	/* drive xfer count */
	long	dk_wds;		/* drive words xfer'd (32 word clicks) */
	long	dk_time;	/* drive active time tally */
 } dk_ios[DK_NC*8], dk_ios1[DK_NC*8];

struct	ios	*dk_iop[DK_NC];	/* iostat structure pointers */
char	dk_nd[DK_NC];		/* number of drives per controller */

/*
 * Disk controller information.
 *
 * CAUTION, order must not change,
 * add new disks at the end.
 */
#define	MSCP	1
#define	RA_OFF	8	/* Position of first MSCP controller */
struct	dk_info {
	char	*di_name;	/* 2 char disk name */
	int	di_flag;	/* flags (like MSCP or not) */
	int	di_dkn;		/* DK_N controller number */
				/* (set dynamically for MSCP controllers */
	int	di_units;	/* selected units (bit0 = unit 0, etc. */
} dk_info[] = {
	"hp",	0,	0,	0377,
	"hm",	0,	1,	0377,
	"hj",	0,	2,	0377,
	"hk",	0,	3,	0377,
	"rp",	0,	4,	0377,
	"rl",	0,	5,	0377,
	"rk",	0,	6,	0377,
	"hs",	0,	7,	0377,
	"ra",	MSCP,	-1,	0,
	"rc",	MSCP,	-1,	0,
	"rx",	MSCP,	-1,	0,
	"rd",	MSCP,	-1,	0,
	0
};

char	dname[10];	/* drive select name */
char	ds[7];		/* drive select numbers */

/*
 * Number of tty char's in/out.
 */

long	tin, tin1;
long	tout, tout1;

/* usec per word for the various disks */
double	xf[] = {
	0.0,	/* dummy - no drive available */
	2.48,	/* RM02 and RP04/5/6 */
	1.65,	/* RM03, RM05 */
	4.3,	/* RK06/7 */
	1.0,	/* ML11 - 2.0  mb */
	2.0,	/* ML11 - 1.0  mb */
	4.0,	/* ML11 - 0.5  mb */
	8.0,	/* ML11 - 0.25 mb */
	7.5,	/* RP03 */
	3.9,	/* RL01/2 */
	11.1,	/* RK03/5 */
	4.0,	/* RS03 non sector interleaved */
	8.0,	/* RS03 sector interleaved */
	2.0,	/* RS04 non sector interleaved */
	4.0,	/* RS04 sector interleaved */
	2.3,	/* NOT USED - was RA81 */
	2.3,	/* RA60/RA80/RA81 (see note below) */
	68.5,	/* RX50 */
	3.2,	/* RD31/RD32/RD51/RD52/RD53/RD54 */
	3.1,	/* RC25 */
};
/*
 * Transfer rate values for MSCP disks are only very
 * rough estimates (by scoping NPG). Actual transfer
 * rate varies between CPU types.
 * All the drives have the same peek transfer rate (1MB/SEC)
 * 4 Micro-sec per long word. Tha average rate slows down from
 * there, depending on the speed of the disk and system load.
 */
struct iostat {
	int	nbuf;
	long	nread;
	long	nreada;
	long	ncache;
	long	nwrite;
	long	bufcount[100];
} io_info, io_delta;
double	etime;

int	mf;
int	nuda;
char	nra[MAXUDA];
char	ra_index[MAXUDA];
struct	ra_drv	ra_drv[MAXUDA*8];

main(argc, argv)
char *argv[];
{
	struct ios *iop;
	extern char *ctime();
	register  i, j, k;
	int interval, count, lc;
	int	cn, dn, ind;
	char	*p;
	double f1, f2;
	long t;

	nlist("/unix", nl);
	for(i=0; i<NSYMCK; i++)
		if(nl[i].n_type == 0) {
			printf("\niostat: can't find %s in /unix namelist\n",
				nl[i].n_name);
			exit(1);
		}
	mf = open("/dev/mem", 0);
	if(mf < 0) {
		printf("\niostat: can't open /dev/mem\n");
		exit(1);
	}
	if((nl[6].n_value==0)||(nl[7].n_value==0)||(nl[8].n_value==0))
		nuda = 0;	/* no MSCP controllers to worry about */
	else {
		lseek(mf, (long)nl[6].n_value, 0);	/* nuda */
		read(mf, (char *)&nuda, sizeof(nuda));
		if(nuda == 0)
			goto no_uda;	/* just in case */
		lseek(mf, (long)nl[7].n_value, 0);	/* nra */
		read(mf, (char *)&nra, nuda);
		lseek(mf, (long)nl[8].n_value, 0);	/* ra_index */
		read(mf, (char *)&ra_index, MAXUDA);
		for(i=0, j=0; i<nuda; i++)
			j += nra[i];
		lseek(mf, (long)nl[9].n_value, 0);	/* ra_drv */
		read(mf, (char *)&ra_drv, sizeof(struct ra_drv) * j);
		for(cn=0; cn<nuda; cn++)
			for(dn=0; dn<nra[cn]; dn++) {
				ind = ra_index[cn] + dn;
				switch(ra_drv[ind].ra_dt) {
				case RX33:
				case RX50:
					p = "rx";
					break;
				case RD31:
				case RD32:
				case RD51:
				case RD52:
				case RD53:
				case RD54:
					p = "rd";
					break;
				case RA60:
				case RA80:
				case RA81:
					p = "ra";
					break;
				case RC25:
					p = "rc";
					break;
				default:
					p = "xx";
					break;
				}
				for(i=0; i<DK_NC; i++)
					if(equal(p, dk_info[i].di_name)) {
						dk_info[i].di_dkn = cn + RA_OFF;
						dk_info[i].di_units |= (1<<dn);
						break;
					}
			}
	}
no_uda:
	interval = 0;
	count = 0;
	for(i=1; i<argc; i++) {
		if(argv[i][0] == '-') {		/* option */
			if (argv[i][1]=='d')
				dflg++;
			else if (argv[i][1]=='s')
				sflg++;
			else if (argv[i][1]=='a')
				aflg++;
			else if (argv[i][1]=='t')
				tflg++;
			else if (argv[i][1]=='i')
				iflg++;
			else if (argv[i][1]=='b')
				bflg++;
			else
				printf("\niostat: (%s) bad option - ignored\n",
					argv[i]);
			continue;
		}
		if((argv[i][0] >= '0') && (argv[i][0] <= '9')) { /* number */
			if(interval == 0) {
				interval = atoi(argv[i]);
				if(interval == 0)
					intervall++;
			} else
				count = atoi(argv[i]);
			continue;
		}
		/* not option or number, must be drive select */
		if(strlen(argv[i]) != 3) {
	bad_ds:
			printf("\niostat: (%s) invalid drive - ignored\n",
				argv[i]);
			continue;
		}
		dname[0] = argv[i][0];
		dname[1] = argv[i][1];
		dname[2] = '\0';
		for(j=0; j<DK_NC; j++)
			if(equal(&dname, dk_info[j].di_name))
				break;
		if(dk_info[j].di_name == 0)
			goto bad_ds;
		if((argv[i][2] < '0') || (argv[i][2] > '7'))
			goto bad_ds;
		if(dsflag == 0) {
			dsflag++;
			for(k=0; k<DK_NC; k++)
				dk_info[k].di_units = 0;
		}
		dk_info[j].di_units |= (1 << (argv[i][2] - '0'));
		continue;
	}
/*
 * Locate and read in the pointers to the
 * iostat structures for each controller and
 * the number of drives on each controller.
 */
	lseek(mf, (long)nl[2].n_value, 0);
	read(mf, (char *)&dk_iop, sizeof(dk_iop));
	lseek(mf, (long)nl[3].n_value, 0);
	read(mf, (char *)&dk_nd, sizeof(dk_nd));
loop:
	lseek(mf, (long)nl[0].n_value, 0);
	read(mf, (char *)&cp_t, sizeof(cp_t));
	for(i=0; i<5; i++) {
		t = cp_t[i];
		cp_t[i] -= cp_t1[i];
		cp_t1[i] = t;
	}
	lseek(mf, (long)nl[4].n_value, 0);
	read(mf, (char *)&tin, sizeof(tin));
	lseek(mf, (long)nl[5].n_value, 0);
	read(mf, (char *)&tout, sizeof(tout));
	t = tin;
	tin -= tin1;
	tin1 = t;
	t = tout;
	tout -= tout1;
	tout1 = t;
	for(i=0; i<DK_NC; i++) {
		if(dk_iop[i] == 0)
			continue;
		iop = &dk_ios[0];
		iop += (i*8);
		lseek(mf, (long)dk_iop[i], 0);
		read(mf, (char *)iop, (sizeof(struct ios)*dk_nd[i]));
	}
	for(i=0; dk_info[i].di_name; i++) {
		if(dk_info[i].di_dkn < 0)
			continue;
		for(j=0; j<8; j++)
			if(dk_info[i].di_units&(1 << j)) {
				k = (dk_info[i].di_dkn * 8) + j;
				dk_ios[k].dk_busy++;
			}
	}
	for(i=0; i<(DK_NC * 8); i++) {
		t = dk_ios[i].dk_numb;
		dk_ios[i].dk_numb -= dk_ios1[i].dk_numb;
		dk_ios1[i].dk_numb = t;
		t = dk_ios[i].dk_wds;
		dk_ios[i].dk_wds -= dk_ios1[i].dk_wds;
		dk_ios1[i].dk_wds = t;
		t = dk_ios[i].dk_time;
		dk_ios[i].dk_time -= dk_ios1[i].dk_time;
		dk_ios1[i].dk_time = t;
		}
	if(lc == 0) {
		if (!(sflg|iflg|bflg)) {
			if (dflg) {
				long tm;
				time(&tm);
				printf("\n%s", ctime(&tm));
			}
		printf("\n");
		if(tflg)
			printf("         TTY");
		if (bflg==0) {
			for(i=0; dk_info[i].di_name; i++) {
				if(dk_info[i].di_dkn < 0)
					continue;
				for(j=0; j<8; j++) {
				    k = (dk_info[i].di_dkn * 8) + j;
					if(dk_ios[k].dk_tr &&
					   dk_ios[k].dk_busy &&
					   (dk_info[i].di_units&(1<<j)))
						printf("   %s%d            ",
						  dk_info[i].di_name, j);
				}
			}
			printf("  PERCENT\n");
			}
		if(tflg)
			printf("   tin  tout");
		if(bflg==0) {
			for(i=0; dk_info[i].di_name; i++) {
				if(dk_info[i].di_dkn < 0)
					continue;
				for(j=0; j<8; j++) {
				    k = (dk_info[i].di_dkn * 8) + j;
					if(dk_ios[k].dk_tr &&
					   dk_ios[k].dk_busy &&
					   (dk_info[i].di_units&(1<<j)))
						printf("   tpm  msps  mspt");
				}
			}
			printf("  user  nice systm  idle\n");
			}
		}
	}
	if(++lc == 10)
		lc = 0;
	t = 0;
	for(i=0; i<4; i++)
		t += cp_t[i];
	etime = t;
	if(etime == 0.)
		etime = 1.;
	if (bflg) {
		biostats();
		goto contin;
	}
	if (sflg) {
		stats2(etime);
		goto contin;
	}
	if (iflg) {
		stats3(etime);
		goto contin;
	}
	etime /= 60.;
	if(tflg) {
		f1 = tin;
		f2 = tout;
		printf("%6.1f", f1/etime);
		printf("%6.1f", f2/etime);
	}
	for(i=0; dk_info[i].di_name; i++) {
		if(dk_info[i].di_dkn < 0)
			continue;
		for(j=0; j<8; j++) {
		    k = (dk_info[i].di_dkn * 8) + j;
			if(dk_ios[k].dk_tr &&
			   dk_ios[k].dk_busy &&
			   (dk_info[i].di_units&(1<<j)))
				stats(k, i);
		}
	}
	for(i=0; i<4; i++)
		stat1(i);
	printf("\n");
	if (aflg)
		printf("%.2f minutes total\n", etime/60);
contin:
	--count;
	if(count)
		if(interval) {
			sleep(interval);
			goto loop;
		}
}


stats(dn, dt)
{
	double f1, f2, f3;
	double f4, f5, f6;
	long t;

	t = dk_ios[dn].dk_time;
	f1 = t;
	f1 = f1/60.;
	f2 = dk_ios[dn].dk_numb;
	if(f2 == 0.) {
		printf("%6.0f%6.1f%6.1f", 0.0, 0.0, 0.0);
		return;
	}
	f3 = dk_ios[dn].dk_wds;
	f3 = f3*32.;
	f4 = xf[dk_ios[dn].dk_tr];
	f4 = f4*1.0e-6;
	f5 = f1 - f4*f3;
	f6 = f1 - f5;
	printf("%6.0f", f2*60./etime);
	f1 = (f5*1000.)/f2;
	if(f1 < 0.)
		f1 = 0.0;
	if(dk_info[dt].di_flag&MSCP)
		printf("%6.1f", f1*1.2);	/* fudge msps on MSCP disks */
	else
		printf("%6.1f", f1);
	printf("%6.1f", f6*1000./f2);
}

stat1(o)
{
	register i;
	long t;
	double f1, f2;

	t = 0;
	for(i=0; i<4; i++)
		t += cp_t[i];
	f1 = t;
	if(f1 == 0.)
		f1 = 1.;
	f2 = cp_t[o];
	printf("%6.2f", f2*100./f1);
}

stats2(t)
double t;
{
	register i, j, k;

	if (dflg) {
		long tm;
		time(&tm);
		printf("\n\n%s", ctime(&tm));
	}
	printf("\ndisk\tdrive\tus/word\t%%time\txfer's\t  words\n");
	for(i=0; dk_info[i].di_name; i++) {
		if(dk_info[i].di_dkn < 0)
			continue;
		for (j=0; j<8; j++) {
			k = (dk_info[i].di_dkn * 8) + j;
			if(dk_ios[k].dk_tr == 0)
				continue;
			printf("\n%s\t%d\t", dk_info[i].di_name, j);
			printf("%4.2f\t", xf[dk_ios[k].dk_tr]);
			printf("%5.2f\t", dk_ios[i*8+j].dk_time/(t/100));
			printf("%D\t  ", dk_ios[k].dk_numb);
			printf("%D", (dk_ios[k].dk_wds)*32);
			}
	}
		printf("\n");
	if (aflg)
		printf("\n%.2f minutes total\n", etime/3600);
}

stats3(t)
double t;
{
	register i, j, k;
	double sum;

	if (dflg) {
		long tm;
		time(&tm);
		printf("\n\n%s", ctime(&tm));
	}
	printf("\n %%time state\n");
	t /= 100;
	sum = cp_t[3];
	printf("%6.2f idle\n", sum/t);
	sum = cp_t[0];
	printf("%6.2f user\n", sum/t);
	sum = cp_t[1];
	printf("%6.2f nice\n", sum/t);
	sum = cp_t[2];
	printf("%6.2f system\n", sum/t);
	sum = cp_t[4];
	printf("%6.2f IO wait\n", sum/t);
	sum = 0;
	for (i=0; i<(DK_NC * 8); i++)
		if(dk_ios[i].dk_tr)
			sum += dk_ios[i].dk_time;
	printf("%6.2f IO active\n", sum/t);
	for(i=0; dk_info[i].di_name; i++) {
		if(dk_info[i].di_dkn < 0)
			continue;
		sum = 0;
		for(j=0; j<8; j++)
			k = (dk_info[i].di_dkn * 8) + j;
			sum += dk_ios[k].dk_time;
		if(sum)
			printf("%6.2f %s active\n", sum/t, dk_info[i].di_name);
		}
	if (aflg)
		printf("\n%.2f minutes total\n", etime/3600);
}

biostats()
{
register i;

	if (dflg) {
		long tm;
		time(&tm);
		printf("\n%s", ctime(&tm));
	}
	lseek(mf,(long)nl[1].n_value, 0);
	read(mf, (char *)&io_info, sizeof(io_info));
	printf("\nnbuf\tnread\tnreada\tncache\tnwrite\n");
	printf("%d\t%D\t%D\t%D\t%D\n",
	 io_info.nbuf,
	 io_info.nread-io_delta.nread, io_info.nreada-io_delta.nreada,
	 io_info.ncache-io_delta.ncache, io_info.nwrite-io_delta.nwrite);

	printf("\nI/O operations per buffer (0->NBUF)\n");
	for(i=0; i<io_info.nbuf; ) {
		printf("%D\t",(long)io_info.bufcount[i]-io_delta.bufcount[i]);
		i++;
		if (i % 10 == 0)
			printf("\n");
	}
	io_delta = io_info;
	printf("\n");
	if (aflg)
		printf("\n%.2f minutes total\n", etime/3600);
}

equal(a, b)
char *a, *b;
{
	return(!strcmp(a, b));
}
