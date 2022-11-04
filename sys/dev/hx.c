
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)hx.c	3.0	4/21/86
 */
/*
 * RX02 floppy disk device driver
 *
 * Bill Shannon   CWRU   05/29/79
 *
 * Modified for Version 7 - 08/13/79 - Bill Shannon
 *
 * Modified for Unix/v7m - 02/27/82 - Fred Canter
 *
 * Modified for ULTRIX-11 RAW Format - 06/04/84 - John Dustin
 *
 *	Layout of logical devices:
 *
 *	name	min dev		unit	density	format
 *	----	-------		----	------- ------
 *	hx0	   0		  0	single	 CWR
 *	hx1	   1		  1	single	 CWR
 *	hx2	   2		  0	double	 CWR
 *	hx3	   3		  1	double	 CWR
 *	hx4	   4		  0	single	 RAW
 *	hx5	   5		  1	single	 RAW
 *	hx6	   6		  0	double	 RAW
 *	hx7	   7		  1	double	 RAW
 *	hx8	   8		  0	single
 *	hx9	   9		  1	single
 *
 *	Stty function call may be used to format a disk.
 *	To enable this feature, define HX_CTRL in this module.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/buf.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/conf.h>
#include <sgtty.h>
/*	#include <sys/tty.h> */
#include <sys/errlog.h>
#include <sys/devmaj.h>

/*
 * below caused new C compiler to blow up.
 * instead we just define seccnt to be av_back and place an int cast
 * in the one place that blows up.
 * Ohms 5/10/85
 */
/* The following structure is used to access av_back as an integer */
/*
 *struct {
 *	int	dummy0;
 *	struct	buf *dummy1;
 *	struct	buf *dummy2;
 *	struct	buf *dummy3;
 *	int	seccnt;
 *};
 */
#define seccnt av_back

struct device {
	int	rx2cs;
	int	rx2db;
};

#define	MULTFMT		/* allow multiple formats on the disk */

/*
 *	the following defines use some fundamental
 *	constants of the RX02.
 */
int	io_csr[];	/* CSR address is now in config file (c.c) */
#define	NSPB	((minor(bp->b_dev)&2) ? 2 : 4)	/* Number of floppy sectors per unix block */
#define	NBPS	((minor(bp->b_dev)&2) ? 256 : 128)	/* Number of bytes per floppy sector	*/
	/* Number of unix blocks on a floppy */
#define	NRXBLKS	((minor(bp->b_dev)&2) ? 1001 : (minor(bp->b_dev)&8) ? 501 : 500)
#define RRX01	(minor(bp->b_dev)&8)
#define	DENSITY	(minor(bp->b_dev)&2)	/* Density: 0 = single, 2 = double */
#define	UNIT	(minor(bp->b_dev)&1)	/* Unit Number: 0 = left, 1 = right */

#ifdef	MULTFMT
#define	DFORMAT	((minor(bp->b_dev)>>2)&077) /* 0=CWRFMT, 1=RAWFMT */
#define	CWRFMT	0
#define	RAWFMT	1
#define NSPT	26
#define NTPD	77	
#endif

/*
 * Block device error record buffer
 */
struct
{
	struct	elrhdr	rx_hdr;	/* record header */
	struct	el_bdh	rx_bdh;	/* block device header */
	int	rx_reg[NRXREG];	/* device registers at error time */
} hx_ebuf;


#define	HX_CTRL	HX_CTRL			/* define for control functions (formatting) */

#ifdef	HX_CTRL
#define	B_CTRL	040000
#endif

struct hx_foo {		/* used with TRWAIT below */
	char 	lobyte;
	char	hibyte;
};

/* #define TRWAIT	while (ptcs->lobyte >= 0) */
#define	TRWAIT		while (((struct hx_foo *)ptcs)->lobyte >= 0)

struct	buf	rx2tab;

struct	buf	rrx2buf;

#ifdef HX_CTRL
struct	buf	crx2buf;	/* buffer header for control functions */
#endif

#define	GO	0000001	/* execute command function	*/
#define	UNIT1	0000020	/* unit select (drive 0=0, 1=1)	*/
#define	RXDONE	0000040	/* function complete		*/
#define	INTENB	0000100	/* interrupt enable		*/
#define	TRANREQ	0000200	/* transfer request (data only)	*/
#define	RXINIT	0040000	/* rx211 initialize		*/
#define	RXERROR	0100000	/* general error bit		*/

/*
 *	rx211 control function bits 1-3 of rx2cs
 */
#define	FILL	0000000	/* fill buffer			*/
#define	EMPTY	0000002	/* empty buffer			*/
#define	WRITE	0000004	/* write buffer to disk		*/
#define	READ	0000006	/* read disk sector to buffer	*/
#define	FORMAT	0000010	/* set media density (format)	*/
#define	RSTAT	0000012	/* read disk status		*/
#define	WSDD	0000014	/* write sector deleted data	*/
#define	RDERR	0000016	/* read error register function	*/

/*
 *	states of driver, kept in b_active
 */
#define	SREAD	1	/* read started  */
#define	SEMPTY	2	/* empty started */
#define	SFILL	3	/* fill started  */
#define	SWRITE	4	/* write started */
#define	SINIT	5	/* init started  */
#define	SFORMAT	6	/* format started */


hxopen(dev, flag)
{
	if(minor(dev) >= 10)
		u.u_error = ENXIO;
}

hxstrategy(bp)
register struct buf *bp;
{
	long sz;

	mapalloc(bp);
	sz = bp->b_bcount;
	sz = (sz + 511) >> 9;
	if((bp->b_blkno < 0) || ((bp->b_blkno+sz) > NRXBLKS)) {
		bp->b_flags =| B_ERROR;
		bp->b_error = ENXIO;
		iodone(bp);
		return;
	}
	if (RRX01 && (bp->b_blkno == 501))
		clrbuf(bp);
	bp->av_forw = (struct buf *) NULL;
	/*
	 * seccnt is actually the number of floppy sectors transferred,
	 * incremented by one after each successful transfer of a sector.
	 */
	bp->seccnt = 0;
	/*
	 * We'll modify b_resid as each piece of the transfer
	 * successfully completes.  It will also tell us when
	 * the transfer is complete.
	 */
	bp->b_resid = bp->b_bcount;
	spl5();
	if(rx2tab.b_actf == NULL)
		rx2tab.b_actf = bp;
	else
		rx2tab.b_actl->av_forw = bp;
	rx2tab.b_actl = bp;
	if(rx2tab.b_active == NULL)
		hxstart();
	spl0();
}

static
hxstart()
{
	register int *ptcs, *ptdb;
	register struct buf *bp;
	int sector, track;
	char *addr, *xmem;

	if((bp = rx2tab.b_actf) == NULL) {
		rx2tab.b_active = NULL;
		return;
	}

	ptcs = io_csr[HX_RMAJ];
	ptdb = io_csr[HX_RMAJ];
	ptdb++;


#ifdef HX_CTRL
	if (bp->b_flags&B_CTRL) {	/* is it a control request ? */
		rx2tab.b_active = SFORMAT;
		*ptcs = FORMAT | GO | INTENB | (UNIT << 4) | (DENSITY << 7);
		TRWAIT;
		*ptdb = 'I';
	} else
#endif
	if(bp->b_flags&B_READ) {
		rx2tab.b_active = SREAD;
#ifdef	MULTFMT
		rx2factr((int)bp->b_blkno * NSPB + (int)bp->seccnt, &sector,
			&track, DFORMAT);
#else
		rx2factr((int)bp->b_blkno * NSPB + (int)bp->seccnt, &sector, &track);
#endif
		*ptcs = READ | GO | INTENB | (UNIT << 4) | (DENSITY << 7);
		TRWAIT;
		*ptdb = sector;
		TRWAIT;
		*ptdb = track;
	} else {
		rx2tab.b_active = SFILL;
		rx2addr(bp, &addr, &xmem);
		*ptcs = FILL | GO | INTENB | ((int)xmem << 12) | (DENSITY << 7);
		TRWAIT;
		*ptdb = (bp->b_resid >= NBPS ? NBPS : bp->b_resid) >> 1;
		TRWAIT;
		*ptdb = addr;
	}
	el_bdact |= (1 << HX_BMAJ);
}

hxintr() {
	register int *ptcs, *ptdb;
	register struct buf *bp;
	int sector, track;
	char *addr, *xmem;

	/* fix for error recovery by duke!phs!dennis */
	if (rx2tab.b_active == SINIT) {
		hxstart();
		return;
	}

	if((bp = rx2tab.b_actf) == NULL) {
		logsi(io_csr[HX_RMAJ]);
		return;
	}

	ptcs = io_csr[HX_RMAJ];
	ptdb = io_csr[HX_RMAJ];
	ptdb++;

	if(*ptcs < 0) {
		if(rx2tab.b_errcnt == 0)
			fmtbde(bp, &hx_ebuf, ptcs, NRXREG, RXDBOFF);
		if(rx2tab.b_errcnt++ > 10 || rx2tab.b_active == SFORMAT) {
			bp->b_flags =| B_ERROR;
			hx_ebuf.rx_bdh.bd_errcnt = rx2tab.b_errcnt;
			if(!logerr(E_BD, &hx_ebuf, sizeof(hx_ebuf)))
			    deverror(bp, hx_ebuf.rx_reg[0], hx_ebuf.rx_reg[1]);
			el_bdact &= ~(1 << HX_BMAJ);
			rx2tab.b_errcnt = 0;
			rx2tab.b_actf = bp->av_forw;
			iodone(bp);
		}
		*ptcs = RXINIT;
		*ptcs = INTENB;
		rx2tab.b_active = SINIT;
		return;
	}
	switch (rx2tab.b_active) {

	case SREAD:			/* read done, start empty */
		rx2tab.b_active = SEMPTY;
		rx2addr(bp, &addr, &xmem);
		*ptcs = EMPTY | GO | INTENB | ((int)xmem << 12) | (DENSITY << 7);
		TRWAIT;
		*ptdb = (bp->b_resid >= NBPS ? NBPS : bp->b_resid) >> 1;
		TRWAIT;
		*ptdb = addr;
		return;

	case SFILL:			/* fill done, start write */
		rx2tab.b_active = SWRITE;
#ifdef	MULTFMT
		rx2factr((int)bp->b_blkno * NSPB + (int)bp->seccnt, &sector,
			&track, DFORMAT);
#else
		rx2factr((int)bp->b_blkno * NSPB + (int)bp->seccnt, &sector, &track);
#endif	MULTFMT
		*ptcs = WRITE | GO | INTENB | (UNIT << 4) | (DENSITY << 7);
		TRWAIT;
		*ptdb = sector;
		TRWAIT;
		*ptdb = track;
		return;

	case SWRITE:			/* write done, start next fill */
	case SEMPTY:			/* empty done, start next read */
		/*
		 * increment amount remaining to be transferred.
		 * if it becomes positive, last transfer was a
		 * partial sector and we're done, so set remaining
		 * to zero.
		 */
		if (bp->b_resid <= NBPS) {
done:
			el_bdact &= ~(1 << HX_BMAJ);
			bp->b_resid = 0;
			rx2tab.b_errcnt = 0;
			rx2tab.b_actf = bp->av_forw;
			iodone(bp);
			break;
		}
		bp->b_resid -= NBPS;
		((int)bp->seccnt)++;
		/* special case */
		if (RRX01 && ((int)bp->b_blkno == 501) && ((int)bp->seccnt > 1))
			goto done;
		break;

#ifdef HX_CTRL
	case SFORMAT:			/* format done (whew!!!) */
		goto done;		/* driver's getting too big... */
#endif
	}
	/* end up here from states SWRITE, SEMPTY, and SINIT */

	hxstart();
}

/*
 *	rx2factr -- calculates the physical sector and physical
 *	track on the disk for a given logical sector.
 *	call:
 *		rx2factr(logical_sector,&p_sector,&p_track);
 *	the logical sector number (0 - 2001) is converted
 *	to a physical sector number (1 - 26) and a physical
 *	track number (0 - 76).
 *	the logical sectors specify physical sectors that
 *	are interleaved with a factor of 2. thus the sectors
 *	are read in the following order for increasing
 *	logical sector numbers (1,3, ... 23,25,2,4, ... 24,26)
 *	There is also a 6 sector slew between tracks.
 *	Logical sectors start at track 1, sector 1; go to
 *	track 76 and then to track 0.  Thus, for example, unix block number
 *	498 starts at track 0, sector 21 and runs thru track 0, sector 2.
 */
static
#ifndef MULTFMT
rx2factr(sectr, psectr, ptrck)
#else	!MULTFMT
rx2factr(sectr, psectr, ptrck, fmt)
int fmt;
#endif	MULTFMT
register int sectr;
int *psectr, *ptrck;
{
	register int p1, p2;

#ifdef	MULTFMT
	switch(fmt) {

	case CWRFMT:
		p1 = sectr/NSPT;
		p2 = sectr%NSPT;
		/* 2 to 1 interleave */
		p2 = (2*p2 + (p2 >= NSPT/2 ? 1 : 0)) % NSPT;
		/* 6 sector per track slew */
		*psectr = 1 + (p2 + 6*p1) % NSPT;
		if (++p1 >= NTPD)
			p1 = 0;		/* wrap around */
		*ptrck = p1;
		break;

	case RAWFMT:
		*ptrck = sectr / NSPT;
		*psectr = (sectr % NSPT) + 1;
		break;
	}
#else	!MULTFMT
	p1 = sectr/26;
	p2 = sectr%26;
	/* 2 to 1 interleave */
	p2 = (2*p2 + (p2 >= 13 ? 1 : 0)) % 26;
	/* 6 sector per track slew */
	*psectr = 1 + (p2 + 6*p1) % 26;
	if (++p1 >= 77)
		p1 = 0;
	*ptrck = p1;
#endif	MULTFMT
}


/*
 *	rx2addr -- compute core address where next sector
 *	goes to / comes from based on bp->b_un.b_addr, bp->b_xmem,
 *	and bp->seccnt.
 */
static
rx2addr(bp, addr, xmem)
register struct buf *bp;
register char **addr, **xmem;
{
	*addr = bp->b_un.b_addr + (int)bp->seccnt * NBPS;
	*xmem = bp->b_xmem;
	if (*addr < bp->b_un.b_addr)			/* overflow, bump xmem */
		(*xmem)++;
}


hxread(dev)
{
	physio(hxstrategy, &rrx2buf, dev, B_READ, minor(dev)&2 ? 1001 : 500);
}


hxwrite(dev)
{
	physio(hxstrategy, &rrx2buf, dev, B_WRITE, minor(dev)&2 ? 1001 : 500);
}


#ifdef HX_CTRL
/*
 *	rx2sgtty -- format RX02 disk, single or double density.
 *	stty with word 0 == 010 does format.  density determined
 *	by device opened.
 */
hxioctl(dev, cmd, addr, flag)
{
	register struct buf *bp;
	struct rx2iocb {
		int	ioc_cmd;	/* command */
		int	ioc_res1;	/* reserved */
		int	ioc_res2;	/* reserved */
	} iocb;

	if (cmd != TIOCSETP) {
err:
		u.u_error = ENOTTY;
		return(0);
	}
	if (copyin(addr, (caddr_t)&iocb, sizeof (iocb))) {
		u.u_error = EFAULT;
		return(1);
	}
	if (iocb.ioc_cmd != FORMAT)
		goto err;
	bp = &crx2buf;
	spl6();
	while (bp->b_flags & B_BUSY) {
		bp->b_flags =| B_WANTED;
		sleep(bp, PRIBIO);
	}
	bp->b_flags = B_BUSY | B_CTRL;
	bp->b_dev = dev;
	bp->b_error = 0;
	hxstrategy(bp);
	iowait(bp);
	bp->b_flags = 0;
}
#endif

hxclose(dev,flag)
dev_t dev;
{

	bflclose(dev);
}
