
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)main.c	3.1	4/8/87
 */
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/filsys.h>
#include <sys/mount.h>
#include <sys/map.h>
#include <sys/proc.h>
#include <sys/inode.h>
#include <sys/seg.h>
#include <sys/conf.h>
#include <sys/buf.h>
#include <sys/devmaj.h>

/*
 * Initialization code.
 * Called from cold start routine as
 * soon as a stack and segmentation
 * have been established.
 * Functions:
 *	clear and free user core
 *	turn on clock
 *	hand craft 0th process
 *	call all initialization routines
 *	fork - process 0 to schedule
 *	     - process 1 execute bootstrap
 *
 * loop at low address in user mode -- /etc/init
 *	cannot be executed.
 */

int	io_csr[];	/* I/O device CSR address, see c.c */
char	io_bae[];	/* I/O device BAE address offsets, see c.c */
int	stksize = sizeof(u.u_stack);	/* size of stack, used in mch.s */
int	*inEMT = &u.u_inemt;
extern	long cdlimit;	/* initialized in c.c */

main()
{

	startup();
	/*
	 * set up system process
	 */

	proc[0].p_addr = ka6->r[0];
	proc[0].p_size = USIZE;
	proc[0].p_stat = SRUN;
	proc[0].p_flag |= SLOAD|SSYS;
	proc[0].p_nice = NZERO;
	u.u_procp = &proc[0];
	u.u_cmask = CMASK;
	u.u_limit = cdlimit;

	/*
	 * Initialize devices and
	 * set up 'known' i-nodes
	 */

#ifdef	UCB_IHASH
	ihinit();
#endif
/*	clkstart();	*/
	cinit();
	binit();
/*
 * clkstart() was moved to here, because
 * if there is no clock on the system `panic no clock'
 * is called. This is incorrect because panic calls update
 * which will not work due to the buffers not having
 * been initialized yet.
 * Fred Canter 1/1/82
 */
#ifdef	UCB_NET
	netinit();
#endif	UCB_NET
	clkstart();
/*
 * Iinit needs to be after clkstart.
 * rl driver calls timeout and clock 
 * must have been started, otherwise
 * a system hang occurs.
 * Bill Burns - 3/15/84
 */
	iinit();
	rootdir = iget(rootdev, (ino_t)ROOTINO);
	rootdir->i_flag &= ~ILOCK;
	u.u_cdir = iget(rootdev, (ino_t)ROOTINO);
	u.u_cdir->i_flag &= ~ILOCK;
	u.u_rdir = NULL;

	/*
	 * call ipc init routines
	 * Ohms 3/21/85
	 * call file lock init routine: George  5/31/85
	 * maus and msg init routines moved to machdep.c: George 6/13/85
	 */
	seminit();
	flckinit();

	/*
	 * make init process
	 * enter scheduling loop
	 * with system process
	 */

	if(newproc()) {
		expand(USIZE + (int)btoc(szicode));
		estabur((unsigned)0, btoc(szicode), (unsigned)0, 0, RO);
		copyout((caddr_t)icode, (caddr_t)0, szicode);
		/*
		 * Return goes to loc. 0 of user init
		 * code just copied out.
		 */
		return;
	}
	sched();
}

/*
 * iinit is called once (from main)
 * very early in initialization.
 * It reads the root's super block
 * and initializes the current date
 * from the last modified date.
 *
 * panic: iinit -- cannot read the super
 * block. Usually because of an IO error.
 */
int	nuda;	/* (c.c) number of MSCP controllers */
int	nts;	/* (tds.c) number of TS11 controllers */
int	ntk;	/* (tds.c) number of TMSCP controllers */
long	boottime;

static
iinit()
{
	register struct buf *cp, *bp;
	register struct filsys *fp;
	int	cn, dn, md;
/*
 * If RA disks configured, call raopen()
 * to force UDA/RQDX1/KLESI init, see errlog.c (case EL_ON:).
 * Flag equal to -1 says init controller,
 * do get unit status for drive.
 * Depends on SA register never actually 
 * containing -1 (don't think this can happen).
 */
	if(bdevsw[RA_BMAJ].d_tab == 0)
		goto ii_tk;	/* RA disk driver not configured */
	for(cn=0; cn<nuda; cn++) {
	    md = *((int *)io_csr[RA_RMAJ] + cn);
	    md += 2;	/* SA register addr */
	    if(fuiword((caddr_t)md) != -1)
		for(dn=0; dn<8; dn++) {
			md = (cn << 6) | (dn << 3);
			(*bdevsw[RA_BMAJ].d_open)(((RA_BMAJ<<8)|md), -1);
		}
	}
ii_tk:
	if(bdevsw[TK_BMAJ].d_tab == 0)
		goto ii_mnt;	/* TK tape driver not configured */
	for(cn=0; cn<ntk; cn++) {
	    md = *((int *)io_csr[TK_RMAJ] + cn);
	    md += 2;	/* SA register addr */
	    if(fuiword((caddr_t)md) != -1)
		(*bdevsw[TK_BMAJ].d_open)(((TK_BMAJ<<8)|cn), -1);
	}
/*
 * Open the swap device,
 * just in case it is not on the same device
 * as the root, i.e., ML11.
 * Required because swap I/O does not call open
 * before calling strategy, that's double plus ungood !
 *
 * Fred Canter 7/28/83 (should have been long ago but I forgot)
 */
ii_mnt:
	(*bdevsw[major(swapdev)].d_open)(swapdev, 1);
	(*bdevsw[major(rootdev)].d_open)(rootdev, 1);
	bp = bread(rootdev, SUPERB);
	if(u.u_error)
		panic("iinit");
	mount[0].m_bufp = bp;
	mount[0].m_inodp = (struct inode *) 1;
	bp->b_flags |= B_MOUNT;
	mount[0].m_dev = rootdev;
	fp = mapin(bp);
	fp->s_flock = 0;
	fp->s_ilock = 0;
	fp->s_ronly = 0;
	fp->s_lasti = 1;
	fp->s_nbehind = 0;
	boottime = time = fp->s_time;
	brelse(bp);
	mapout(bp);
	for(cn=0; cn<4; cn++) {	/* Call user device open routines */
		if(bdevsw[U1_BMAJ+cn].d_tab)	/* device is configured */
			(*bdevsw[U1_BMAJ+cn].d_open)(((U1_BMAJ+cn)<<8), -1);
	}
}

/*
 * This is the set of buffers proper, whose heads
 * were declared in buf.h.  There can exist buffer
 * headers not pointing here that are used purely
 * as arguments to the I/O routines to describe
 * I/O to be done-- e.g. swbuf for
 * swapping.
 */
memaddr	bpaddr;		/* physical click-address of buffers */

/*
 * Initialize the buffer I/O system by freeing
 * all buffers and setting all device buffer lists to empty.
 */

static
binit()
{
	register struct buf *bp;
	register struct buf *dp;
	register int i;
	struct bdevsw *bdp;
	long	paddr;
	int	j, k;

	bfreelist.b_forw = bfreelist.b_back =
	    bfreelist.av_forw = bfreelist.av_back = &bfreelist;
	paddr = ((long) bpaddr) << 6;
	for (i=0; i<nbuf; i++) {
		bp = &buf[i];
		bp->b_dev = NODEV;
		bp->b_un.b_addr = loint(paddr);
		bp->b_xmem = hiint(paddr);
		paddr += BSIZE;
		bp->b_back = &bfreelist;
		bp->b_forw = bfreelist.b_forw;
		bfreelist.b_forw->b_back = bp;
		bfreelist.b_forw = bp;
		bp->b_flags = B_BUSY;
		brelse(bp);
	}
	for (i=0, bdp = bdevsw; bdp->d_open; bdp++, i++) {
		dp = bdp->d_tab;
/*
 * If any massbus devices (hp, hm, hs, ml, ht) are in the
 * configuration table (bdevsw[]), check the controller
 * type. Set it to RH70 if the bus address extension
 * register is present.
 * Also set a bit in the block device configuration word
 * (el_bdcw) for device present in bdevsw[].
 *
 * NOTE: dp points to an array of device tables for drivers
 *	 that support multiple controllers (RA, TK, & TS).
 *	 Added 1/20/86 -- Fred Canter (to fix multi cntlr support)
 */
		if(io_bae[i])	/* see c.c */
			if((fuiword((caddr_t)(io_csr[i+RK_RMAJ]+io_bae[i])) < 0)
			|| (dp == 0))
				io_bae[i] = 0;
		if(dp) {
			if(i == TK_BMAJ)
				k = ntk;
			else if(i == TS_BMAJ)
				k = nts;
			else if(i == RA_BMAJ)
				k = nuda;
			else
				k = 1;
			for(j=0; j<k; j++) {
				dp->b_forw = dp;
				dp->b_back = dp;
				dp++;
			}
			el_bdcw |= (1 << i);
		}
		nblkdev++;
	}
	/*
	 * Set up first 10 unibus map registers, if map present.
	 * 0->(cfree, uda, hp_bads, hk_bads, TS (cmdpkt, chrbuf, mesbuf))
	 * 1 thru 9 -> buffers
	 */
	mapinit();
	/*
	 * so that io_info can be put in data space after mb_end
	 * see bio.c for more info.
	 */
	{
		extern struct {
			int nbufr;
			long nread, nreada, ncache;
			long nwrite, bufcount[1];
		} io_info;
		io_info.nbufr = nbuf;
	}
}
