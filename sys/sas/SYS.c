/*
 * SCCSID: @(#)SYS.c	3.0	4/21/86
 */
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/
/*
 * ULTRIX-11 standalone bootstrap I/O routines
 *
 * IMPORTANT CHANGE -- Fred Canter 6/20/85
 *
 * The stand-alone programs now can only access files with
 * up to one indirect block, i.e., 260K bytes. This change was
 * necessary to shrink the stand-alone programs back to the
 * size they were before the 1K file system was implemented.
 * See code changes in sbmap() and saio.h.
 * These routines are not used in two different ways:
 *
 * 1.	Boot: uses them in /lib/libsa.a as before.
 *
 * 2.	All other standalone programs have the system calls
 *	and device drivers removed from /lib/libsa.a, only
 *	the prf.o, exit, and trap routines are used from /lib/libsa.a.
 *
 * The syscalls and drivers are now in the file syscall.
 * When creating /lib/libsa.a and the syscall file, the parameter
 * NOSYSCALL is not defined and all code is compiled.
 * When creating a standalone program, NOSYSCALL is defined and
 * the code for the system calls is not compiled.
 * See /usr/sys/sas/README for more information.
 *
 * #ifdef NO_FIO
 *
 *	NO_FIO is defined when building programs like rabads
 *	that only access physical devices and don't need all the
 *	code and buffers used to deal with the file system.
 */
#include <sys/param.h>
#include <sys/ino.h>
#include <sys/inode.h>
#include <sys/filsys.h>
#include <sys/dir.h>
#ifndef	NOSYSCALL
#include "saio.h"

/* int	segflag = 0; */


static
openi(n,io)
register struct iob *io;
{
	register struct dinode *dp;
#ifdef	UCB_NKB
	daddr_t	tdp;
#endif	UCB_NKB

	io->i_offset = 0;
#ifdef	UCB_NKB
	tdp = itod(n);
	io->i_bn = fsbtodb(tdp) + io->i_boff;
	io->i_cc = BSIZE;
#else
	io->i_bn = (daddr_t)((n+15)/INOPB) + io->i_boff;
	io->i_cc = 512;
#endif	UCB_NKB
	io->i_ma = io->i_buf;
	devread(io);

	dp = io->i_buf;
#ifdef	UCB_NKB
	dp = &dp[itoo(n)];
#else
	dp = &dp[(n-1)%INOPB];
#endif	UCB_NKB
	io->i_ino.i_number = n;
	io->i_ino.i_mode = dp->di_mode;
	io->i_ino.i_size = dp->di_size;
	l3tol((char *)io->i_ino.i_addr,(char *)dp->di_addr,NADDR);
}


#ifndef	NO_FIO
static
find(path, file)
register char *path;
struct iob *file;
{
	register char *q;
	char c;
	int n;

	if (path==NULL || *path=='\0') {
		printf("null path\n");
		return(0);
	}

	openi((ino_t) 2, file);
	while (*path) {
		while (*path == '/')
			path++;
		q = path;
		while(*q != '/' && *q != '\0')
			q++;
		c = *q;
		*q = '\0';

		if ((n=dlook(path, file))!=0) {
			if (c=='\0')
				break;
			openi(n, file);
			*q = c;
			path = q;
			continue;
		} else {
			printf("%s not found\n",path);
			return(0);
		}
	}
	return(n);
}


static daddr_t
sbmap(io, bn)
register struct iob *io;
daddr_t bn;
{
	register i;
	register struct inode *ip;
	int j, sh;
	daddr_t nb, *bap;

/*
 * Clear blknos[], in case it should ever get reused.
 */
	for(j=0; j<NBUFS; j++)
		blknos[j] = 0L;
	ip = &io->i_ino;;
	if(bn < 0) {
		printf("bn negative\n");
		return((daddr_t)0);
	}

	/*
	 * blocks 0..NADDR-4 are direct blocks
	 */
	if(bn < NADDR-3) {
		i = bn;
		nb = ip->i_addr[i];
		return(nb);
	}

	/*
	 * addresses NADDR-3, NADDR-2, and NADDR-1
	 * have single, double, triple indirect blocks.
	 * the first step is to determine
	 * how many levels of indirection.
	 * CAN ONLY HANDLE FILES WITH SINGLE INDIRECT BLOCK
	 */
	sh = 0;
	nb = 1;
	bn -= NADDR-3;
	for(j=3; j>0; j--) {
		sh += NSHIFT;
		nb <<= NSHIFT;
		if(bn < nb)
			break;
		bn -= nb;
	}
#ifdef	UCB_NKB
	if(j != 3) {	/* must be single indirect block (j == 3) */
#else
	if(j == 0) {
#endif	UCB_NKB
		printf("bn ovf %D\n",bn);
		return((daddr_t)0);
	}

	/*
	 * fetch the address from the inode
	 */
	nb = ip->i_addr[NADDR-j];
	if(nb == 0) {
		printf("bn void %D\n",bn);
		return((daddr_t)0);
	}

	/*
	 * fetch through the indirect blocks
	 */
	for(; j<=3; j++) {
#ifdef	UCB_NKB
		if (blknos[0] != nb) {
			io->i_bn = fsbtodb(nb) + io->i_boff;
			io->i_ma = &sbmapb;
			io->i_cc = BSIZE;
#else
		if (blknos[j] != nb) {
			io->i_bn = nb + io->i_boff;
			io->i_ma = sbmapb[j];
			io->i_cc = 512;
#endif	UCB_NKB
			devread(io);
#ifdef	UCB_NKB
			blknos[0] = nb;
#else
			blknos[j] = nb;
#endif	UCB_NKB
		}
#ifdef	UCB_NKB
		bap = &sbmapb;
#else
		bap = sbmapb[j];
#endif	UCB_NKB
		sh -= NSHIFT;
		i = (bn>>sh) & NMASK;
		nb = bap[i];
		if(nb == 0) {
			printf("bn void %D\n",bn);
			return((daddr_t)0);
		}
	}

	return(nb);
}

static ino_t
dlook(s, io)
char *s;
register struct iob *io;
{
	register struct direct *dp;
	register struct inode *ip;
	daddr_t bn;
	int n,dc;
#ifdef	UCB_NKB
	daddr_t	tbn;
#endif	UCB_NKB

	if (s==NULL || *s=='\0')
		return(0);
	ip = &io->i_ino;
	if ((ip->i_mode&IFMT)!=IFDIR) {
		printf("not a directory\n");
		return(0);
	}

	n = ip->i_size/sizeof(struct direct);

	if (n==0) {
		printf("zero length directory\n");
		return(0);
	}

#ifdef	UCB_NKB
	dc = BSIZE;
	bn = (daddr_t)0;
	while(n--) {
		if (++dc >= BSIZE/sizeof(struct direct)) {
			tbn = sbmap(io,bn++);
			io->i_bn = fsbtodb(tbn) + io->i_boff;
			io->i_ma = io->i_buf;
			io->i_cc = BSIZE;
#else
	dc = 512;
	bn = (daddr_t)0;
	while(n--) {
		if (++dc >= 512/sizeof(struct direct)) {
			io->i_bn = sbmap(io, bn++) + io->i_boff;
			io->i_ma = io->i_buf;
			io->i_cc = 512;
#endif	UCB_NKB
			devread(io);
			dp = io->i_buf;
			dc = 0;
		}

		if (match(s, dp->d_name))
			return(dp->d_ino);
		dp++;
	}
	return(0);
}
#endif	NO_FIO

static
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

lseek(fdesc, addr, ptr)
int	fdesc;
off_t	addr;
int	ptr;
{
	register struct iob *io;

	if (ptr != 0) {
		printf("Seek not from beginning of file\n");
		return(-1);
	}
	fdesc -= 3;
	if (fdesc < 0 || fdesc >= NFILES || ((io = &iob[fdesc])->i_flgs&F_ALLOC) == 0)
		return(-1);
	io->i_offset = addr;
#ifdef	UCB_NKB
	if((io->i_flgs&F_FILE) == 0)
		io->i_bn = addr/512 + io->i_boff;
	else
		io->i_bn = fsbtodb(addr/BSIZE) + io->i_boff;
#else
	io->i_bn = addr/512 + io->i_boff;
#endif	UCB_NKB
	io->i_cc = 0;
	return(0);
}

getc(fdesc)
int	fdesc;
{
	register struct iob *io;
	register unsigned char *p;
	register  c;
	int off;
#ifdef	UCB_NKB
	daddr_t	tbn;
	int	mtfile;	/* file being read from a magtape */
#endif	UCB_NKB


	if (fdesc >= 0 && fdesc <= 2)
		return(getchar());
#ifdef	NO_FIO
	return(-1);
#else
	fdesc -= 3;
	if (fdesc < 0 || fdesc >= NFILES || ((io = &iob[fdesc])->i_flgs&F_ALLOC) == 0)
		return(-1);
#ifdef	UCB_NKB
	mtfile = devsw[io->i_ino.i_dev].dv_flags&DV_TAPE;
#endif	UCB_NKB
	p = io->i_ma;
	if (io->i_cc <= 0) {
#ifdef	UCB_NKB
		/*
		 * The block number is not used by tape drivers,
		 * but it might be used someday, so we deal with it.
		 */
		if(mtfile)
			io->i_bn = io->i_offset/(off_t)512;
		else
			io->i_bn = fsbtodb(io->i_offset/(off_t)BSIZE);
		if (io->i_flgs&F_FILE) {
			tbn = sbmap(io, dbtofsb(io->i_bn));
			io->i_bn = fsbtodb(tbn) + io->i_boff;
		}
		io->i_ma = io->i_buf;
		if(mtfile)
			io->i_cc = 512;
		else
			io->i_cc = BSIZE;
		devread(io);
		if (io->i_flgs&F_FILE) {
			off = io->i_offset % (off_t)BSIZE;
			if (io->i_offset+(BSIZE-off) >= io->i_ino.i_size)
#else
		io->i_bn = io->i_offset/(off_t)512;
		if (io->i_flgs&F_FILE)
			io->i_bn = sbmap(io, io->i_bn) + io->i_boff;
		io->i_ma = io->i_buf;
		io->i_cc = 512;
		devread(io);
		if (io->i_flgs&F_FILE) {
			off = io->i_offset % (off_t)512;
			if (io->i_offset+(512-off) >= io->i_ino.i_size)
#endif	UCB_NKB
				io->i_cc = io->i_ino.i_size - io->i_offset + off;
			io->i_cc -= off;
			if (io->i_cc <= 0)
				return(-1);
		} else
			off = 0;
		p = &io->i_buf[off];
	}
	io->i_cc--;
	io->i_offset++;
	c = *p++;
	io->i_ma = p;
	return(c);
#endif	NO_FIO
}
getw(fdesc)
int	fdesc;
{
	register w,i;
	register char *cp;
	int val;

	for (i = 0, val = 0, cp = &val; i < sizeof(val); i++) {
		w = getc(fdesc);
		if (w == -1) {
			if (i == 0)
				return(-1);
			else
				return(val);
		}
		*cp++ = w;
	}
	return(val);
}

read(fdesc, buf, count)
int	fdesc;
char	*buf;
int	count;
{
	register i;
	register struct iob *file;

	if (fdesc >= 0 && fdesc <= 2) {
		i = count;
		do {
			*buf = getchar();
		} while (--i && *buf++ != '\n');
		return(count - i);
	}
	fdesc -= 3;
	if (fdesc < 0 || fdesc >= NFILES || ((file = &iob[fdesc])->i_flgs&F_ALLOC) == 0)
		return(-1);
	if ((file->i_flgs&F_READ) == 0)
		return(-1);
	if ((file->i_flgs&F_FILE) == 0) {
		file->i_cc = count;
		file->i_ma = buf;
		i = devread(file);
/*
 * Following code was in error and caused continuous
 * read/write of a device to fail, i.e., a lseek needed
 * prior to each read/write. We always deal with physical
 * device in 512 byte chuncks, so CLSIZE not used.
 * All program have the lseeks, no reason to remove them.
 *
#ifdef	UCB_NKB
		file->i_bn += CLSIZE;
#else
		file->i_bn++;
#endif	UCB_NKB
 *
 * END OF OLD CODE
 */
		file->i_bn += (i/512);
		return(i);
	}
	else {
		if (file->i_offset+count > file->i_ino.i_size)
			count = file->i_ino.i_size - file->i_offset;
		if ((i = count) <= 0)
			return(0);
		do {
			*buf++ = getc(fdesc+3);
		} while (--i);
		return(count);
	}
}

write(fdesc, buf, count)
int	fdesc;
char	*buf;
int	count;
{
	register i;
	register struct iob *file;

	if (fdesc >= 0 && fdesc <= 2) {
		i = count;
		while (i--)
			putchar(*buf++);
		return(count);
	}
	fdesc -= 3;
	if (fdesc < 0 || fdesc >= NFILES || ((file = &iob[fdesc])->i_flgs&F_ALLOC) == 0)
		return(-1);
	if ((file->i_flgs&F_WRITE) == 0)
		return(-1);
	file->i_cc = count;
	file->i_ma = buf;
	i = devwrite(file);
/*
 * See comment above same statement in read().
#ifdef	UCB_NKB
	file->i_bn += CLSIZE;
#else
	file->i_bn++;
#endif	UCB_NKB
*/
	file->i_bn += (i/512);
	return(i);
}

/*
 * Names of files that can be loaded from magtape,
 * position in the list must be that same as their
 * position on the tape.
 *
 * !!!!!! ORDER MUST MATCH TADE DIRECTORY FILES IN /SAS !!!!!!
 */

static	char	*mtfiles[] =
{
	"boot",
	"syscall",
	"sdload",
	"scat",
	"contents",
	"mkfs",
	"restor",
	"dskinit",
	"bads",
	"rabads",
	"copy",
	"icheck",
	"saprog",
	"rcmds",
	"f77",
	"pascal",
	"plot",
	"sccs",
	"usat",
	"usep",
	"tcpip",
	"uucp",
	"spell",
	"userdev",
	"docprep",
	"learn",
	"libsa",
	"dict",
	"orphans",
	"games",
	"manuals",
	"sysgen",
	"root",
	"usr",
	0
};

open(str, how)
char *str;
int	how;
{
	register char *cp;
	int i;
	register struct iob *file;
	register struct devsw *dp;
	int	fdesc;
	static first = 1;
	int	c;
	long	atol();

	if (first) {
		for (i = 0; i < NFILES; i++)
			iob[i].i_flgs = 0;
		first = 0;
	}

	for (fdesc = 0; fdesc < NFILES; fdesc++)
		if (iob[fdesc].i_flgs == 0)
			goto gotfile;
	_stop("No more file slots");
gotfile:
	(file = &iob[fdesc])->i_flgs |= F_ALLOC;
	for (cp = str; *cp && *cp != '('; cp++)
			;
	if (*cp != '(') {
		printf("Bad device\n");
		file->i_flgs = 0;
		return(-1);
	}
	*cp++ = '\0';
	for (dp = devsw; dp->dv_name; dp++) {
		if (match(str, dp->dv_name))
			goto gotdev;
	}
	printf("Unknown device\n");
	file->i_flgs = 0;
	return(-1);
gotdev:
	*(cp-1) = '(';
	file->i_ino.i_dev = dp-devsw;
	file->i_unit = *cp++ - '0';
	if (file->i_unit < 0 || file->i_unit > 7) {
		printf("Bad unit specifier\n");
		file->i_flgs = 0;
		return(-1);
	}
	if (*cp++ != ',') {
badoff:
		printf("Missing offset specification\n");
		file->i_flgs = 0;
		return(-1);
	}
	file->i_boff = atol(cp);
	for (;;) {
		if (*cp == ')')
			break;
		if (*cp++)
			continue;
		goto badoff;
	}
/*
 * Files may be loaded from magtape either by offset
 * or by name. If the offset is zero the "mtfiles" table
 * is serarched to find the offset. If the offset is not
 * zero, it is used and the name is ignored. If the offset
 * is zero and the name is zero, the offset is used, i.e., ht(0,0).
 */
	if(dp->dv_flags&DV_TAPE) {
		c = *(cp+1);	/* save first char of file name */
		if(file->i_boff != 0)
			*(cp+1) = '\0';
		else if((file->i_boff == 0) && *(cp+1)) {
			for(i=0; mtfiles[i]; i++)
				if(match((cp+1), mtfiles[i])) {
					file->i_boff = i;
					*(cp+1) = '\0';
					break;
				}
			if(mtfiles[i] == 0) {
				printf("Bad magtape file name\n");
				*(cp+1) = c;
				file->i_flgs = 0;
				return(-1);
			}
		}
	}
	if(devopen(file) < 0)
		return(-1);
	if (*++cp == '\0') {
		*cp = c;
		file->i_flgs |= how+1;
		file->i_cc = 0;
		file->i_offset = 0;
		return(fdesc+3);
	}
#ifdef	NO_FIO
	printf("\nDEBUG: Sorry - no file I/O code!\n");
	return(-1);
#else
	if ((i = find(cp, file)) == 0) {
		file->i_flgs = 0;
		return(-1);
	}
	if (how != 0) {
		printf("Can't write files yet.. Sorry\n");
		file->i_flgs = 0;
		return(-1);
	}
	openi(i, file);
	file->i_offset = 0;
	file->i_cc = 0;
	file->i_flgs |= F_FILE | (how+1);
	return(fdesc+3);
#endif	NO_FIO
}

close(fdesc)
int	fdesc;
{
	struct iob *file;

	fdesc -= 3;
	if (fdesc < 0 || fdesc >= NFILES || ((file = &iob[fdesc])->i_flgs&F_ALLOC) == 0)
		return(-1);
	if ((file->i_flgs&F_FILE) == 0)
		devclose(file);
	file->i_flgs = 0;
	return(0);
}
#endif	NOSYSCALL

/*
 * Returned status is saved in location rtnstat.
 * For boot and sdload, rtnstat is a local variable and is not used.
 * For all standalone programs, rtnstat is physically in srt0.s @ 1010.
 * The sdload program gets the returned status from the rtnstat
 * location via mfpi().
 */
int	rtnstat;
exit(status)
int	status;
{
	rtnstat = status;
	_stop("Exit called");
}

_stop(s)
char	*s;
{
	printf("%s\n", s);
	_rtt();
}

char	*ttm[] =
{
	"bus error",
	"illegal instruction",
	"break point trace",
	"IOT instruction",
	"power fail",
	"EMT instruction",
	"TRAP instruction",
	"Programmed interrupt request",
	"floating point exception",
	"memory management segmentation",
	"memory parity",
	"",
	"",
	"",
	"",
	"stray vector",
	0
};

trap(tt,sp,r5,r4,r3,r2,r1,r0,pc,ps)
{
	printf("\n\nTrap - %s\n", ttm[tt&017]);
	printf("\nps = %o\npc = %o", ps, pc);
	printf("\nr0 = %o\nr1 = %o\nr2 = %o", r0, r1, r2);
	printf("\nr3 = %o\nr4 = %o\nr5 = %o", r3, r4, r5);
	printf("\nsp = %o\n", sp);
	for (;;)
		;
}
