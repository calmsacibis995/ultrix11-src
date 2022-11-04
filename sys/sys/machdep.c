
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)machdep.c	3.4	1/14/88
 */
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/acct.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/inode.h>
#include <sys/proc.h>
#include <sys/seg.h>
#include <sys/map.h>
#include <sys/reg.h>
#include <sys/buf.h>
#include <sys/uba.h>

/*
 * ULTRIX-11 System Version Number Definitions
 *
 * The kernel version number is stored in two forms.
 * It is contained in the kernel startup message 
 * as an ascii string and encoded into bits 6->15
 * of rn_ssr3 for use by the error log print program.
 *
 * rn_ssr3 encoding
 *
 *	BITS 0->5	saved memory management SSR3 (lo 6 bits)
 *	BITS 6->7	00 = V, 01 = X, 10 = Y
 *	BITS 8->11	.#
 *	BITS 12->15	#.
 * Note:	The following #defines makes dealing with
 *		this a bit easier, but takes away some of
 *		Fred's fun with magic bit twiddling.
 *			-Dave Borman, 4/24/84
 */
#define	V	0000
#define	X	0100
#define	Y	0200
#define	VERS(a,b,c)	(a|((b&017)<<12)|((c&017)<<8))

#define	OS_VN	VERS(V,3,1)
#define	SU_MSG "ULTRIX-11 Kernel V3.1"

/*
 * UMAX limits the number of terminals available for
 * user logins. For the binary kits, UMAX is defined
 * by the makefile otherwise a default of 100 is used.
 * The UMAX values are intentionally strange.
 *
 * 8	(!&) 023041
 * 16	(.:) 035056
 * 32	(%$) 022045
 * 100	(;#) 021473
 *
 * This magic number becomes an argument to `init'.
 * The `boot' matches this number against its idea of UMAX.
 *
 * Fred Canter (King of strangeness!)
 */

#ifndef	UMAX
#define	UMAX	021473
#endif

segm	seg5;		/* filled in by initialization */

/*
 * Icode is the octal bootstrap
 * program executed in user mode
 * to bring up the system.
 */
int	icode[] =
{
	0104413,	/* sys exec; init; initp */
	0000016,
	0000010,
	0000777,	/* br . */
	0000016,	/* initp: init; umax; 0 */
	0000030,
	0000000,
	0062457,	/* init: </etc/init\0> */
	0061564,
	0064457,
	0064556,
	0000164,
	UMAX,		/* umax: maximum number of user logins */
	0000000,	/* zero to end UMAX string */
};
int	szicode = sizeof(icode);

memaddr	bpaddr;
#ifdef	UCB_NET
memaddr mbbase;
unsigned mbsize;
#endif
#ifdef	UCB_CLIST
extern memaddr	clststrt;
extern int	nclist;
unsigned	clstdesc;
#endif	UCB_CLIST

/*
 * Machine-dependent startup code
 */
#define	MPCSR ((physadr)0172100)
#define	LOCZERO ((physadr)0)
startup()
{
	register unsigned int i;
	register unsigned int maxmeml;
/*
 * `maxmem' is the total amount of memory available.
 * `maxseg' is a parameter which may be used to limit
 * the amount of memory actually used by unix.
 * Both are expressed in 64 byte segments.
 */
	saveseg5(seg5);
	if(maxmem > maxseg)
		maxmem = maxseg;
	realmem = maxmem;
	rn_ssr3 |= OS_VN;		/* load kernel version number */
	printf("\n%s\n", SU_MSG);	/* startup message */
	printf("\nrealmem = %D", ctob((long)realmem));
	i = ka6->r[0] + USIZE;
	usermem = maxmem - i;
	MAPINIT(coremap, mapsize);	/* initialize the core map */
	mfree(coremap, usermem, i);
	calloinit();			/* initialize the callout table */
#define	B  (size_t)(((long)nbuf << BSHIFT) / ctob(1))
	if ((bpaddr = malloc(coremap, B)) == 0)
		panic("buffers");
	usermem -= B;
	printf("\nbuffers = %D", ctob((long)(B)));
#undef	B
#ifdef	UCB_CLIST
#define	C	(nclist * sizeof(struct cblock))
	if (C > UBPAGE) {
		printf("clist area too large (reducing)\n");
		nclist = UBPAGE/sizeof(struct cblock);
	}
	if ((clststrt = malloc(coremap, btoc(C))) == 0)
		panic("clists");
	usermem -= btoc(C);
	printf("\nclists  = %D", (long)(C));
	clstaddr = (long)clststrt << 6;
	clstdesc = ((((btoc(nclist*sizeof(struct cblock)))-1) << 8) | RW);
#undef	C
#endif	UCB_CLIST

#ifdef	PROFILE
	usermem -= msprof();
#endif	PROFILE

#ifdef	UCB_NET
	if (mbsize) {
		if ((mbbase = malloc(coremap, btoc(mbsize))) == 0)
			panic("mbbase");
		usermem -= btoc(mbsize);
		printf("\nmbufs   = %D", (long)mbsize);
	}
#endif	UCB_NET

	/* call maus and message init routines: sysV stuff: 
	 * George Mathew 6/13/85 */

	usermem -= mausinit();
	usermem -= msginit();
	printf("\nusermem = %D", ctob((long)usermem));
/*
 * Fred Canter -- dynamic allocation of maxmem.
 *
 * Will become a sysgen question.
 */
	/* loc zero = 4 if split i/d kernel */
	if(LOCZERO->r[0] == 4)
		maxmeml = 6528;	/* 408KB for SID */
	else
		maxmeml = 3328;	/* 208KB for NSID */
/*	if(MAXMEM < usermem)	*/
/*		maxmem = MAXMEM;	*/
	if(maxmeml < usermem)
		maxmem = maxmeml;
	else
		maxmem = usermem;
	printf("\nmaxumem = %D\n", ctob((long)maxmem));
	MAPINIT(swapmap, mapsize);
	mfree(swapmap, nswap, 1);
	swplo--;

	/*
	 * Set last user space address register to
	 * point to unibus I/O space, for use by the sui/fui
	 * calls in some of the following functions, such
	 * as clkstart().
	 */

	UISA->r[7] = ka6->r[1]; /* io segment */
	UISD->r[7] = 077406;
}

#ifdef	PROFILE
#define	SISD0	((u_short *)0172200)
#define	SISD1	((u_short *)0172202)
#define	SISD2	((u_short *)0172204)
#define	SISD3	((u_short *)0172206)
#define	SISD4	((u_short *)0172210)
#define	SISD5	((u_short *)0172212)
#define	SISD6	((u_short *)0172214)
#define	SISA0	((u_short *)0172240)
#define	SISA1	((u_short *)0172242)
#define	SISA2	((u_short *)0172244)
#define	SISA3	((u_short *)0172246)
#define	SISA4	((u_short *)0172250)
#define	SISA5	((u_short *)0172252)
#define	SISA6	((u_short *)0172254)
/*
 *  Allocate memory for system profiling.  Called
 *  once at boot time.  Returns number of clicks
 *  used by profiling.
 *
 *  The system profiler uses supervisor I space registers 2 and 3
 *  (virtual addresses 040000 through 0100000) to hold the profile.
 */

static int nproclicks;
memaddr proloc;

msprof()
{
	nproclicks = btoc(8192*5);
	proloc = malloc(coremap, nproclicks);
	if (proloc == 0)
		panic("msprof");

	*SISA2 = proloc;
	*SISA3 = proloc + btoc(8192);
	*SISA4 = proloc + btoc(16384);
	*SISA5 = proloc + btoc(24576);
	*SISA6 = proloc + btoc(32768);
	*SISD2 = 077400|RW;
	*SISD3 = 077400|RW;
	*SISD4 = 077400|RW;
	*SISD5 = 077400|RW;
	*SISD6 = 077400|RW;
	*SISD0 = RW;
	*SISD1 = RW;

	esprof();
	return (nproclicks);
}

/*
 *  Enable system profiling.
 *  Zero out the profile buffer and then turn the
 *  clock (KW11-P) on.
 */
esprof()
{
	clear(proloc, nproclicks);
	isprof();
	printf("\nprofiling on");
}
#endif  PROFILE

#ifdef	SYSPHYS
/*
 * set up a physical address
 * into users virtual address space.
 */
sysphys()
{
	register i, s, d;
	register struct a {
		int	segno;
		int	size;
		int	phys;
	} *uap;

	if(!suser())
		return;
	uap = (struct a *)u.u_ap;
	i = uap->segno;
	if(i < 0 || i >= 8)
		goto bad;
	s = uap->size;
	if(s < 0 || s > 128)
		goto bad;
	d = u.u_uisd[i+8];
	if(d != 0 && (d&ABS) == 0)
		goto bad;
	u.u_uisd[i+8] = 0;
	u.u_uisa[i+8] = 0;
	if(!u.u_sep) {
		u.u_uisd[i] = 0;
		u.u_uisa[i] = 0;
	}
	if(s) {
		u.u_uisd[i+8] = ((s-1)<<8) | RW|ABS;
		u.u_uisa[i+8] = uap->phys;
		if(!u.u_sep) {
			u.u_uisa[i] = u.u_uisa[i+8];
			u.u_uisd[i] = u.u_uisd[i+8];
		}
	}
	sureg();
	return;

bad:
	u.u_error = EINVAL;
}
#endif	SYSPHYS

/*
 * Determine which clock is attached, and start it.
 * panic: no clock
 * is printed if no clock can be found.
 *
 */
#define	CLOCK1	((physadr)0177546)
#define	CLOCK2	((physadr)0172540)
clkstart()
{
	lks = CLOCK1;
	if(suiword((caddr_t)lks, 0) == -1) {
		lks = CLOCK2;
		if(suiword((caddr_t)lks, 0) == -1)
			panic("no clock");
	}
	lks->r[0] = 0115;
}

/*
 * Let a process handle a signal by simulating an interrupt
 */
sendsig(p, signo)
caddr_t p;
{
	register unsigned n;

	n = u.u_ar0[R6] - 4;
	grow(n);
	suword((caddr_t)n+2, u.u_ar0[RPS]);
	suword((caddr_t)n, u.u_ar0[R7]);
	u.u_ar0[R6] = n;
	u.u_ar0[RPS] &= ~TBIT;
	u.u_ar0[R7] = (int)p;
}
/*
 * Simulate a return frominterrupt on return from the syscall
 * after popping n words of the users stack.
 */
dorti(n)
{
	register int opc, ops;

	u.u_ar0[R6] += n * 2;

	if (((opc = fuword((caddr_t)u.u_ar0[R6])) == -1)
	    || ((ops = fuword((caddr_t)(u.u_ar0[R6] + 2))) == -1))
		psignal(u.u_procp, SIGSEGV);
	else {
		u.u_ar0[R6] += 4;
		u.u_ar0[PC] = opc;
		ops |= PS_CURMOD | PS_PRVMOD;	/* assure user space */
		ops &= ~PS_USERCLR;		/* priority 0 */
		u.u_ar0[RPS] = ops;
	}
}

/*
 * Save the current kernel mapping and set it to the normal map.
 * Called at interrupt time to access proc, text or file structures
 * or the user structure.
 * Called only if *KDSA5 != seg5.se_addr from the macro savemap.
 */

Savemap(map)
register mapinfo map;
{
	map[0].se_addr = *KDSA5;
	map[0].se_desc = *KDSD5;
	if (kdsa6) {
		map[1].se_addr = *KDSA6;
		map[1].se_desc = *KDSD6;
		*KDSD6 = KD6;
		*KDSA6 = kdsa6;
	} else
		map[1].se_desc = NOMAP;
	restorseg5(seg5);
}

/*
 * Restore the mapping information saved above.
 * Called only if map[0].se_desc != NOMAP (from macro restormap).
 */

Restormap(map)
register mapinfo map;
{
	*KDSA5 = map[0].se_addr;
	*KDSD5 = map[0].se_desc;
	if (map[1].se_desc != NOMAP) {
		*KDSD6 = map[1].se_desc;
		*KDSA6 = map[1].se_addr;
	}
}

/*
 *	Map in an out-of-address space buffer.
 *	If this is done from interrupt level,
 *	the previous map must be saved before mapin,
 *	and restored after mapout; e.g.
 *		segm save;
 *		saveseg5(save);
 *		mapin(bp);
 *		...
 *		mapout(bp);
 *		restorseg5(save);
 */
#ifdef	UCB_NET
segm	Bmapsave;
#endif	UCB_NET

caddr_t
mapin(bp)
register struct buf *bp;
{
	register caddr_t paddr;
	register caddr_t offset;
	int x;

#ifdef	UCB_NET
	saveseg5(Bmapsave);
#endif	UCB_NET
	if (bp->b_flags & B_MOUNT)
		x = spl5();	/* keep phys addr from changing to virt addr */
	offset = ((unsigned)bp->b_un.b_addr & 077);
	paddr = ((unsigned)bp->b_un.b_addr >> 6) & 01777;
	(unsigned)paddr |= ((unsigned)bp->b_xmem << 10);
	if (bp->b_flags & B_MAP)	/* adjust the unibus virtual addr */
		paddr = paddr - (BUF_UBADDR>>6) + bpaddr;
	if (bp->b_flags & B_MOUNT)
		splx(x);
	
	mapseg5(paddr, (BSIZE << 2) | RW);
	return(SEG5 + offset);
}

mapout(bp)
register struct buf *bp;
{
#ifndef	UCB_NET
	normalseg5();
#else	UCB_NET
	restorseg5(Bmapsave);
#endif	UCB_NET
}
