
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)maus.c	3.1	3/4/87
 */

#include <sys/types.h>
#include <sys/param.h>
#include <signal.h>
#include <sys/systm.h>
#include <sys/dir.h>
#include <errno.h>
#include <sys/user.h>
#include <sys/inode.h>
#include <sys/maus.h>
#include <sys/conf.h>
#include <sys/seg.h>
#include <sys/reg.h>
#include <sys/map.h>
/* #include <sys/sysinfo.h> */
/* #include <sys/sysmacros.h> */

#define	READM	0
#define	WRITEM	1
#define	RDWRM	2
#define	FREEM	3
#define	SWITM	4

/*
 * maus system call
 */

maus()
{
	register i, j;
	register	struct	inode	*ip;
	int min;
	extern uchar();

/*	sysinfo.maus++; */
	switch(i=u.u_ar0[R0]) {
	case READM:
	case WRITEM:
	case RDWRM:
#ifdef	UCB_SYMLINKS
		if((ip = namei(uchar, LOOKUP, 1)) == NULL)
#else	UCB_SYMLINKS
		if((ip = namei(uchar, LOOKUP)) == NULL)
#endif	UCB_SYMLINKS
			return;
		if(i == READM || i == RDWRM)
			access(ip, IREAD);
		if(i == WRITEM || i == RDWRM)
			access(ip, IWRITE);
		min = (minor(ip->i_rdev) - 8) & 0377;
		if(major(ip->i_rdev) != MEMDEV || min >= nmausent)
			u.u_error = EINVAL;
		iput(ip);
		if(u.u_error)
			return;
		for(j=0; u.u_msav[j].ms_uisd;)
			if(++j >= NMSAV) {
				u.u_error = EMFILE;
				return;
			}
		if(i == READM)
			i = RO;
		else
			i = RW;
		u.u_msav[j].ms_uisd = i | ABS | (mausmap[min].bsize-1)<<8;
		u.u_msav[j].ms_uisa = mausmap[min].boffset + mauscore;
		u.u_rval1 = j;
		return;

	case FREEM:
		if((j=u.u_arg[0]) >= NMSAV || u.u_msav[j].ms_uisd == 0) {
			u.u_error = EBADF;
			return;
		}
		u.u_msav[j].ms_uisd = 0;
		u.u_msav[j].ms_uisa = 0;
		return;

	case SWITM:
		if((j = u.u_arg[0]) != -1) {
			j = ((j >> 13) & 07) + 8 ;
			if((u.u_mbitm & (1 << (j-8))) == 0 && u.u_uisd[j]) {
				u.u_error = EINVAL;	/* vaddr illegal */
				return;
			}
		} else for(j=14; u.u_uisd[j];)
			if(--j < 8) {
				u.u_error = ENOMEM;	/* no reg left */
				return;
			}
		if(u.u_ar0[R1] == -1) {		/* disable case */
			u.u_uisd[j] = 0;
			u.u_uisa[j] = 0;
			if(sepid) {
				UISD->r[j] = 0;
				UISA->r[j] = 0;
			}
			j -= 8;
			if(u.u_sep == 0) {
				UISD->r[j] = 0;
				UISA->r[j] = 0;
				u.u_uisd[j] = 0;
				u.u_uisa[j] = 0;
			}
			u.u_mbitm &= ~(1 << j);
			return;
		}
		if((i = u.u_ar0[R1]) >= NMSAV || u.u_msav[i].ms_uisd == 0) {
			u.u_error = EINVAL;	/* mdes illegal */
			return;
		}
		u.u_uisd[j] = u.u_msav[i].ms_uisd;
		u.u_uisa[j] = u.u_msav[i].ms_uisa;
		if(sepid) {
			UISD->r[j] = u.u_uisd[j];
			UISA->r[j] = u.u_uisa[j];
		}
		j -= 8;
		if(u.u_sep == 0) {
			UISD->r[j] = u.u_uisd[j] = u.u_uisd[j+8];
			UISA->r[j] = u.u_uisa[j] = u.u_uisa[j+8];
		}
		u.u_mbitm |= 1 << j;
		u.u_rval1 = j << 13;
		return;

	default:
		u.u_error = EINVAL;
		return;
	}
}

/*
 * allocate core for MAUS common regions
 */
mausinit()
{
	register unsigned i, bs, maussize;

	bs = 0;
	for(i=0; mausmap[i].boffset != -1; i++) {
		if((maussize=mausmap[i].boffset+mausmap[i].bsize)>bs)
			bs = maussize;
	}
	if (mauscore = malloc(coremap, bs)) {
		mausend = mauscore + bs;
		nmausent = i;
		return(bs);
	}
	return(0);
}

/*  mmoff pulled out from mem.c and put here since it is easier
 *  to make maus sysgenable this way.  George Mathew */

long
mmoff(dev)
register dev;
{
	register cn;	/* click number */

	cn = u.u_offset >> 6;
	if ((dev -= 8) < 0 || dev >= nmausent || cn < 0
		|| cn >= mausmap[dev].bsize) {
			u.u_error = ENXIO;
			return(-1L);
	}
	else
		return( ((long)(mauscore + mausmap[dev].boffset)<<6) + u.u_offset);
}
