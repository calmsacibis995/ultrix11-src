
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 *	Print system stats
 *
 *	Modified to understand new file flags from <sys/file.h>
 *	Specifically the elimination of the FPIPE flags and
 *	the addition of FNDELAY and FAPPEND.
 *	Pipes are now flagged in the inode struct (IFIFO == 010000)
 *	Bill Burns  3/15/85
 *
 *	Modified to read NPROC, NINODE, NTEXT, NOFILE,
 *	NSIG, and NFILE from the kernel via nlist(3) and
 *	the /dev/mem driver. Modified to use malloc()
 *	to allocate it's tables dynamically. This was done in
 *	order to free pstat from it's dependence on param.h .
 *	Also modified to report on tty lines (-t) for DZ11.
 *	Also modified to support multiple kl & dl lines.
 * 	Also modified to report on dhu/dhv lines - Bill 3/84
 *
 *	The tty structures are no longer hard wired to the drivers,
 *	they are now associated by logical assignment.
 *
 *	Fred Canter 11/11/82
 */

static char Sccsid[] = "@(#)pstat.c	3.0	(ULTRIX-11)	4/22/86";
#define mask(x) (x&0377)

#include <nlist.h>
#include <sys/param.h>
#include <sys/conf.h>
#include <sys/tty.h>
#include <sys/inode.h>
#include <sys/text.h>
#include <sys/proc.h>
#include <sys/file.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/callo.h>

char	*fcore	= "/dev/mem";
char	*fnlist	= "/unix";
int	fc;

struct nlist setup[] = {
#define	SINODE	0
#define	SNINODE	1
	{ "_inode", 0, 0, }, { "_ninode", 0, 0, },
#define	STEXT	2
#define	SNTEXT	3
	{ "_text", 0, 0, }, { "_ntext", 0, 0, },
#define	SPROC	4
#define	SNPROC	5
	{ "_proc", 0, 0, }, { "_nproc", 0, 0, },
#define	SDH	6
	{ "_dh11", 0, 0, },
#define	SNDH	7
	{ "_ndh11", 0, 0, },
#define	SKL	8
	{ "_kl11", 0, 0, },
#define	SNKL	9
	{ "_nkl11", 0, 0, },
#define	SNDL	10
	{ "_ndl11", 0, 0, },
#define	SFIL	11
#define	SNFILE	12
	{ "_file", 0, 0, }, { "_nfile", 0, 0, },
#define	SNOFILE	13
	{ "_nofile", 0, 0, },
#define	SNSIG	14
	{ "_nsig", 0, 0, },
#define	SDZ	15
	{ "_dz_tty", 0, 0, },
#define	SNDZ	16
	{ "_dz_cnt", 0, 0, },
#define	SUH	17
	{ "_uh11", 0, 0, },
#define	SNUH	18
	{ "_nuh11", 0, 0, },
#define	SPTY	19
	{ "_pt_tty", 0, 0, },
#define	SNPTY	20
	{ "_npty", 0, 0, },
#define	SCALLO	21
#define	SNCALLO	22
	{ "_callout" },	{ "_ncall" },
#define	SBOOT	23
	{ "_boottime" },
#define	STIME	24
	{ "_time" },
	0,
};

int	inof;
int	txtf;
int	prcf;
int	ttyf;
int	usrf;
long	ubase;
int	filf;
int	allflg;
int	nofile;
int	nsig;
int	nfile;
int	ninode, ntext, nproc;
int	uptime, resource;

main(argc, argv)
char **argv;
{

	while (--argc && **++argv == '-') {
	    while (*++*argv)
		switch (**argv) {

		case 'a':
			allflg++;
			break;

		case 'i':
			inof++;
			break;

		case 'x':
			txtf++;
			break;
		case 'p':
			prcf++;
			break;

		case 't':
			ttyf++;
			break;

		case 'u':
			if (--argc == 0)
				break;
			ubase = oatoi(*++argv);
			--argv;
			if(ubase == 'e') {	/* err, arg not octal number */
				argc++;
				break;
			}
			usrf++;
			break;

		case 'f':
			filf++;
			break;

		case 'T':
			resource++;
			break;

		case 'U':
			uptime++;
			break;
		default:
			printf("Unknown flag '%c'\n", **argv);
			break;
		}
	}
	if(usrf)
		++argv;
	if (argc>0)
		fcore = argv[0];
	if ((fc = open(fcore, 0)) < 0) {
		printf("Can't find %s\n", fcore);
		exit(1);
	}
	if (argc>1)
		fnlist = argv[1];
	nlist(fnlist, setup);
	if (setup[SINODE].n_type == 0) {
		printf("no namelist\n");
		exit(1);
	}
	if (uptime)
		douptime();
	if (inof)
		doinode();
	if (txtf)
		dotext();
	if (ttyf)
		dotty();
	if (prcf)
		doproc();
	if (usrf)
		dousr();
	if (filf)
		dofil();
	if (resource)
		doresource();
}

doinode()
{
	register struct inode *ip;
	struct inode *xinode;
	register int nin, loc;
	int	i;
	char	*calloc();

	lseek(fc, (long)setup[SNINODE].n_value, 0);
	read(fc, (char *)&ninode, sizeof(ninode));
	nin = 0;
	xinode = (struct inode *) calloc(ninode, sizeof(struct inode));
	lseek(fc, (long)setup[SINODE].n_value, 0);
	read(fc, (char *)xinode, (sizeof(struct inode) * ninode));
	for (ip=xinode, i=0; i<ninode; ip++, i++)
		if (ip->i_count)
			nin++;
	printf("\n%d active inodes\n", nin);
	printf("   LOC  FLAGS    CNT DEVICE   INO   MODE NLK UID  SIZE/DEV\n");
	loc = setup[SINODE].n_value;
	for (ip=xinode, i=0; i<ninode; ip++, i++, loc += sizeof(*xinode)) {
		if (ip->i_count == 0)
			continue;
		printf("%7.1o ", loc);
		putf(ip->i_flag&ILOCK, 'L');
		putf(ip->i_flag&IUPD, 'U');
		putf(ip->i_flag&IACC, 'A');
		putf(ip->i_flag&IMOUNT, 'M');
		putf(ip->i_flag&IWANT, 'W');
		putf(ip->i_flag&ITEXT, 'T');
		printf("%6d", ip->i_count);
		printf("%3d,%3d", major(ip->i_dev), minor(ip->i_dev));
		printf("%6u", ip->i_number);
		printf("%7o", ip->i_mode);
		printf("%4d", ip->i_nlink);
		printf("%4d", ip->i_uid);
		if ((ip->i_mode&IFMT)==IFBLK || (ip->i_mode&IFMT)==IFCHR)
			printf("%6d,%3d", major(ip->i_rdev), minor(ip->i_rdev));
		else
			printf("%10ld", ip->i_size);
		printf("\n");
	}
}

putf(v, n)
{
	if (v)
		printf("%c", n);
	else
		printf(" ");
}

dotext()
{
	register struct text *xp;
	struct text *xtext;
	register loc, i;
	int ntx;
	char	*calloc();

	lseek(fc, (long)setup[STEXT].n_value, 0);
	read(fc, (char *)&ntext, sizeof(ntext));
	ntx = 0;
	xtext = (struct text *) calloc(ntext, sizeof(struct text));
	lseek(fc, (long)setup[STEXT].n_value, 0);
	read(fc, (char *)xtext, (sizeof(struct text) * ntext));
	for (xp=xtext, i=0; i<ntext; xp++, i++)
		if (xp->x_iptr!=NULL)
			ntx++;
	printf("\n%d text segments\n", ntx);
	printf("   LOC FLAGS DADDR  CADDR SIZE   IPTR  CNT CCNT LCNT\n");
	loc = setup[STEXT].n_value;
	for (xp=xtext, i=0; i<ntext; xp++, i++, loc += sizeof(*xtext)) {
		if (xp->x_iptr == NULL)
			continue;
		printf("%7.1o", loc);
		printf(" ");
		putf(xp->x_flag&XTRC, 'T');
		putf(xp->x_flag&XWRIT, 'W');
		putf(xp->x_flag&XLOAD, 'L');
		putf(xp->x_flag&XLOCK, 'K');
		putf(xp->x_flag&XWANT, 'w');
		printf("%5u", xp->x_daddr);
		printf("%7.1o", xp->x_caddr);
		printf("%5d", xp->x_size);
		printf("%8.1o", xp->x_iptr);
		printf("%4d", xp->x_count&0377);
		printf("%5d", xp->x_ccount);
		printf("%5d", xp->x_lcount);
		printf("\n");
	}
}

doproc()
{
	struct proc *xproc;
	register struct proc *pp;
	register loc, np;
	int	i;
	char	*calloc();

	lseek(fc, (long)setup[SNPROC].n_value, 0);
	read(fc, (char *)&nproc, sizeof(nproc));
	xproc = (struct proc *) calloc(nproc, sizeof(struct proc));
	lseek(fc, (long)setup[SPROC].n_value, 0);
	read(fc, (char *)xproc, (sizeof(struct proc) * nproc));
	np = 0;
	for (pp=xproc, i=0; i<nproc; pp++, i++)
		if (pp->p_stat)
			np++;
	printf("\n%d processes\n", np);
	printf("   LOC S    F PRI SIGNAL UID TIM CPU NI  PGRP   PID  PPID   ADDR SIZE  WCHAN   LINK  TEXTP CLKT\n");
	for (loc=setup[SPROC].n_value,pp=xproc,i=0; i<nproc; pp++, i++, loc+=sizeof(*xproc)) {
		if (pp->p_stat==0 && allflg==0)
			continue;
		printf("%6o", loc);
		printf("%2d", pp->p_stat);
		printf("%5o", pp->p_flag);
		printf("%4d", pp->p_pri);
		printf("%7O", (long)pp->p_sig);
		printf("%4d", pp->p_uid&0377);
		printf("%4d", pp->p_time&0377);
		printf("%4d", pp->p_cpu&0377);
		printf("%3d", pp->p_nice);
		printf("%6d", pp->p_pgrp);
		printf("%6d", pp->p_pid);
		printf("%6d", pp->p_ppid);
/*
 * If process in memory (SLOAD flag set),
 * addr & size are in 64 byte clicks, otherwise
 * addr is block number in swap area and size is
 * size of image in 512 byte blocks. The paddr-1 is
 * because p_addr is swap block number +1.
 */
		if(pp->p_flag&SLOAD) {
			printf("%7o", pp->p_addr);
			printf("%5d", pp->p_size);
		} else {
			if(pp->p_addr)
				printf("%7d", pp->p_addr - 1);
			else
				printf("%7d", pp->p_addr); /* proc slot empty */
			printf("%5d", (pp->p_size+7)>>3);
		}
		printf("%7o", pp->p_wchan);
		printf("%7o", pp->p_link);
		printf("%7o", pp->p_textp);
		printf(" %u", pp->p_clktim);
		printf("\n");
	}
}

/*
 * tty structures must be handled on a per line basis
 * because the may not be contiguous.
 */

dotty()
{
	struct tty *ttyp[128];	/* MAX of 8 DH11 */
				/* contains address of tty structure */
	int ndh, ndz, nkl, ndl, nuh, npty;
	register char *mesg;
	register int i;

	lseek(fc, (long)setup[SNKL].n_value, 0);
	read(fc, (char*)&nkl, sizeof(nkl));
	lseek(fc, (long)setup[SNDL].n_value, 0);
	read(fc, (char*)&ndl, sizeof(ndl));
	lseek(fc, (long)setup[SKL].n_value, 0);
	read(fc, (char *)ttyp, sizeof(int) * (nkl + ndl));
mesg = " # RAW CAN OUT   MODE   ADDR   DEL COL  STATE     PGRP  ERRCNT  LASTEC\n";
	printf("\n%d kl lines\n", nkl);
	printf(mesg);
	for(i=0; i<nkl; i++)
		ttyprt(i, ttyp[i]);
	if(ndl) {
		printf("%d dl lines\n", ndl);
		printf(mesg);
		for(i=nkl; i<(nkl+ndl); i++)
			ttyprt(i, ttyp[i]);
	}
	if (setup[SNDH].n_type != 0) {
		lseek(fc, (long)setup[SNDH].n_value, 0);
		read(fc, (char *)&ndh, sizeof(ndh));
		printf("%d dh lines\n", ndh);
		printf(mesg);
		lseek(fc, (long)setup[SDH].n_value, 0);
		read(fc, (char *)ttyp, sizeof(int) * ndh);
		for(i=0; i<ndh; i++)
			ttyprt(i, ttyp[i]);
		}
	if (setup[SNUH].n_type != 0) {
		lseek(fc, (long)setup[SNUH].n_value, 0);
		read(fc, (char *)&nuh, sizeof(nuh));
		printf("%d dhu/dhv lines\n", nuh);
		printf(mesg);
		lseek(fc, (long)setup[SUH].n_value, 0);
		read(fc, (char *)ttyp, sizeof(int) * nuh);
		for(i=0; i<nuh; i++)
			ttyprt(i, ttyp[i]);
		}
	if (setup[SNDZ].n_type != 0) {
		lseek(fc, (long)setup[SNDZ].n_value, 0);
		read(fc, (char *)&ndz, sizeof(ndz));
		printf("%d dz lines\n", ndz);
		printf(mesg);
		lseek(fc, (long)setup[SDZ].n_value, 0);
		read(fc, (char *)ttyp, sizeof(int) * ndz);
		for(i=0; i<ndz; i++)
			ttyprt(i, ttyp[i]);
		}
	if (setup[SNPTY].n_type != 0) {
		lseek(fc, (long)setup[SNPTY].n_value, 0);
		read(fc, (char *)&npty, sizeof(npty));
		printf("%d pseudo ttys\n", npty);
		printf(mesg);
		lseek(fc, (long)setup[SPTY].n_value, 0);
		read(fc, (char *)ttyp, sizeof(int) * npty);
		for(i=0; i<npty; i++)
			ttyprt(i, ttyp[i]);
		}
}

ttyprt(n, atp)
caddr_t	atp;
{
	register struct tty *tp;
	struct	tty	tty_ts;

	if(atp == 0)
		return;
	lseek(fc, (long)atp, 0);	/* find actual tty struct for line */
	read(fc, (char *)&tty_ts, sizeof(struct tty));
	tp = &tty_ts;
	printf("%2d", n);
	printf("%4d", tp->t_rawq.c_cc);
	printf("%4d", tp->t_canq.c_cc);
	printf("%4d", tp->t_outq.c_cc);
	printf("%8.1o", tp->t_flags);
	printf("%8.1o", tp->t_addr);
	printf("%3d", tp->t_delct);
	printf("%4d ", tp->t_col);
	putf(tp->t_state&TIMEOUT, 'T');
	putf(tp->t_state&WOPEN, 'W');
	putf(tp->t_state&ISOPEN, 'O');
	putf(tp->t_state&CARR_ON, 'C');
	putf(tp->t_state&BUSY, 'B');
	putf(tp->t_state&ASLEEP, 'A');
	putf(tp->t_state&XCLUDE, 'X');
	putf(tp->t_state&TTSTOP, 'S');
	putf(tp->t_state&TBLOCK, 'M');
	putf(tp->t_state&HUPCLS, 'H');
	printf("%6d", tp->t_pgrp);
	printf("  %6u  %6o", tp->t_errcnt, tp->t_lastec);
	printf("\n");
}

dousr()
{
	union {
		struct	user rxu;
		char	fxu[ctob(USIZE)];
	} xu;
	register struct user *up;
	register i;
	int	nofile, nsig;

	lseek(fc, (long)setup[SNOFILE].n_value, 0);
	read(fc, (char *)&nofile, sizeof(nofile));
	lseek(fc, (long)setup[SNSIG].n_value, 0);
	read(fc, (char *)&nsig, sizeof(nsig));
	lseek(fc, ubase<<6, 0);
	read(fc, (char *)&xu, sizeof(xu));
	up = &xu.rxu;
	printf("\nrsav %.1o %.1o\n", up->u_rsav[0], up->u_rsav[1]);
	printf("segflg, error %d, %d\n", up->u_segflg, up->u_error);
	printf("uids %d,%d,%d,%d\n", up->u_uid,up->u_gid,up->u_ruid,up->u_rgid);
	printf("procp %.1o\n", up->u_procp);
	printf("base, count, offset %.1o %.1o %ld\n", up->u_base,
		up->u_count, up->u_offset);
	printf("cdir %.1o\n", up->u_cdir);
	printf("dbuf %.14s\n", up->u_dbuf);
	printf("dirp %.1o\n", up->u_dirp);
	printf("dent %d %.14s\n", up->u_dent.d_ino, up->u_dent.d_name);
	printf("pdir %.1o\n", up->u_pdir);
	printf("dseg");
	for (i=0; i<8; i++)
		printf("%8.1o", up->u_uisa[i]);
	printf("\n    ");
	for (i=0; i<8; i++)
		printf("%8.1o", up->u_uisd[i]);
	if (up->u_sep) {
		printf("\ntseg");
		for (i=8; i<16; i++)
			printf("%8.1o", up->u_uisa[i]);
		printf("\n    ");
		for (i=8; i<16; i++)
			printf("%8.1o", up->u_uisd[i]);
	}
	printf("\nfile");
	for (i=0; i<10; i++)
		printf("%8.1o", up->u_ofile[i]);
	printf("\n    ");
	for (i=10; i<nofile; i++)
		printf("%8.1o", up->u_ofile[i]);
	printf("\nargs");
	for (i=0; i<5; i++)
		printf(" %.1o", up->u_arg[i]);
	printf("\nsizes %.1o %.1o %.1o\n", up->u_tsize, up->u_dsize, up->u_ssize);
	printf("sep %d\n", up->u_sep);
	printf("qsav %.1o %.1o\n", up->u_qsav[0], up->u_qsav[1]);
	printf("ssav %.1o %.1o\n", up->u_ssav[0], up->u_ssav[1]);
	printf("sigs");
	for (i=0; i<nsig; i++)
		printf(" %.1o", up->u_signal[i]);
	printf("\ntimes %ld %ld\n", up->u_utime/60, up->u_stime/60);
	printf("ctimes %ld %ld\n", up->u_cutime/60, up->u_cstime/60);
	printf("ar0 %.1o\n", up->u_ar0);
/*
	printf("prof");
	for (i=0; i<4; i++)
		printf(" %.1o", up->u_prof[i]);
*/
	printf("\nintflg %d\n", up->u_intflg);
	printf("ttyp %.1o\n", up->u_ttyp);
	printf("ttydev %d,%d\n", major(up->u_ttyd), minor(up->u_ttyd));
	printf("comm %.14s\n", up->u_comm);
}

oatoi(s)
char *s;
{
	register v;

	v = 0;
	while (*s) {
		if((*s < '0') || (*s > '7'))
			return('e');	/* error, not octal digit */
		v = (v<<3) + *s++ - '0';
	}
	return(v);
}

dofil()
{
	struct file *xfile;
	register struct file *fp;
	register nf;
	int loc;
	int	i;
	char	*calloc();

	lseek(fc, (long)setup[SNFILE].n_value, 0);
	read(fc, (char *)&nfile, sizeof(nfile));
	nf = 0;
	xfile = (struct file *) calloc(nfile, sizeof(struct file));
	lseek(fc, (long)setup[SFIL].n_value, 0);
	read(fc, (char *)xfile, (sizeof(struct file) * nfile));
	for (fp=xfile, i=0; i<nfile; fp++, i++)
		if (fp->f_count)
			nf++;
	printf("\n%d open files\n", nf);
	printf("  LOC   FLG      CNT   INO    OFFS\n");
	for (fp=xfile,loc=setup[SFIL].n_value,i=0; i<nfile; fp++, i++, loc+=sizeof(*xfile)) {
		if (fp->f_count==0)
			continue;
		printf("%7.1o ", loc);
		putf(fp->f_flag&FREAD, 'R');
		putf(fp->f_flag&FWRITE, 'W');
		putf(fp->f_flag&FNDELAY, 'D');
		putf(fp->f_flag&FAPPEND, 'A');
		putf(fp->f_flag&FSOCKET, 'S');
		putf(fp->f_flag&FPENDING, 'P');
		printf("%6d", fp->f_count);
		printf("%8.1o", fp->f_inode);
		printf("  %ld\n", fp->f_un.f_offset);
	}
}

douptime()
{
	long	boottime, curtime;

	lseek(fc, (long)setup[SBOOT].n_value, 0);
	read(fc, &boottime, sizeof(boottime));
	lseek(fc, (long)setup[STIME].n_value, 0);
	read(fc, &curtime, sizeof(curtime));
	curtime -= boottime;

	if (!strcmp(fcore, "/dev/mem"))
		printf("System up ");
	else
		printf("System ran for ");
	if (curtime >= 60L*60L*24L) {
		printf("%d days ", (int)(curtime/(60L*60L*24L)));
		curtime %= (60L*60L*24L);
	}
	printf("%d hours ", (int)(curtime/(60L*60L)));
	curtime %= 60L*60L;
	printf("%d minutes\n", (int)((curtime+59)/60L));
}

#define	INUSE	0
#define	FREE	1
#define	NOTUSED	2

int	c_proc(), c_text(), c_inode(), c_file(), c_callo();

struct tab {
	int		(*func)();
	unsigned	loc;
	unsigned	size;
	unsigned	count;
	char		*name;
} tab[] = {
	c_file,   SFIL,    sizeof(struct file),    0, "files",
	c_inode,  SINODE,  sizeof(struct inode),   0, "inodes",
	c_proc,   SPROC,   sizeof(struct proc),    0, "processes",
	c_text,   STEXT,   sizeof(struct text),    0, "texts",
	c_callo,  SCALLO,  sizeof(struct callo),   0, "callouts",
	0
};

doresource()
{
	register struct tab *tp;

#ifdef	notdef
	printf("Table    Slots      In Use        Free  Never Used\n");
	printf("-----    -----      ------        ----  ----------\n");
#endif	notdef
	for (tp = tab; tp->func; tp++) {
		if (tp->count == 0) {
			lseek(fc, (long)setup[tp->loc+1].n_value, 0);
			read(fc, &tp->count, sizeof(tp->count));
		}
		tp->loc = setup[tp->loc].n_value;
		sumup(tp);
	}
	return(0);
}

sumup(tp)
register struct tab *tp;
{
	char buf[512];
	register int i, inuse = 0, free = 0, notused = 0;

	lseek(fc, (long)tp->loc, 0);
	for (i = tp->count; i > 0; --i) {
		read(fc, &buf, tp->size);
		switch ((*tp->func)(&buf)) {
		case INUSE:
			++inuse;
			break;
		case NOTUSED:
			++notused;
			/*FALLTHROUGH*/
		case FREE:
			++free;
			break;
		}
	}
#ifdef	notdef
	printf("%-8.8s %5d  %5d %3d%%  %5d %3d%%  %5d %3d%%\n",
			tp->name,
			tp->count,
			inuse,
			(inuse*200 + tp->count)/(2*tp->count),
			free,
			(free*200 + tp->count)/(2*tp->count),
			notused,
			(notused*200 + tp->count)/(2*tp->count));
#endif	notdef
	printf("%4d/%4d %-9.9s %4d never used (%2d%%)\n",
			inuse, tp->count, tp->name,
			notused,
			(notused*200 + tp->count)/(2*tp->count));
}

c_proc(p)
register struct proc *p;
{
	if (p->p_stat)
		return(INUSE);
	if (!p->p_nice && !p->p_pri)
		return(NOTUSED);
	return(FREE);
}

c_text(tp)
register struct text *tp;
{
	if (tp->x_iptr)
		return(INUSE);
	if (!tp->x_daddr && !tp->x_caddr)
		return(NOTUSED);
	return(FREE);
}

c_inode(ip)
register struct inode *ip;
{
	if (ip->i_count)
		return(INUSE);
	if (!ip->i_mode && !ip->i_nlink && !ip->i_dev &&
	    !ip->i_size && !ip->i_uid && !ip->i_gid)
		return(NOTUSED);
	return(FREE);
}

c_file(fp)
register struct file *fp;
{
	if (fp->f_count)
		return(INUSE);
	if (!fp->f_flag && !fp->f_inode && !fp->f_un.f_offset)
		return(NOTUSED);
	return(FREE);
}

c_callo(cp)
register struct callo *cp;
{
	if (cp->c_time)
		return(INUSE);
	if (!cp->c_func)
		return(NOTUSED);
	return(FREE);
}
