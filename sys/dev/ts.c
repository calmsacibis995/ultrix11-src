
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * ULTRIX-11 TS11 - 1600 BPI tape driver
 *
 * SCCSID: @(#)ts.c	3.0	4/21/86
 *
 * Chung-Wu Lee, Dec-04-85
 *
 *	Start supporting up to 4 controller per system.
 *
 * Chung-Wu Lee, Aug-09-85
 *
 *	add ioctl routine (from 2.9 BSD).
 *
 * Fred Canter, Aug-07-85
 *
 *	changes for 1K block file system.
 *
 * Fred Canter
 *
 * Thanks to Jerry Brenner for most of this driver.
 *
 *******************************************************************
 *								   *
 * If the system has both a TM11 and a TS11, set the TS11 CSR to   *
 * 172550. This allows the TS11 to be booted. Use ms0 for the TS11 *
 * at 172520 or ms6 for the TS11 at 172550. Use the Boot: csr      *
 * command to tell the stand-alone programs the TS11 address, if   *
 * it is not at the standard address (172520).                     *
 *								   *
 *******************************************************************
 */


#include <sys/param.h>
#include <sys/systm.h>
#include <sys/buf.h>
#include <sys/conf.h>
#include <sys/dir.h>
#include <sys/file.h>
#include <sys/user.h>
#include <sys/errlog.h>
#include <sys/devmaj.h>
#include <sys/mtio.h>
#include <sys/ts_info.h>

int cmdpkt[];			/* command packet */

struct	device
{
	int	tsdb;		/* TS data buffer and bus address reg */
	int	tssr;		/* TS status register */
};

struct	chrdat	chrbuf[];	/* characteristics buffer */
struct	mespkt	mesbuf[];		/* message buffer */
struct	buf	tstab[];
struct	buf	ctsbuf[];
struct	buf	rtsbuf[];


struct compkt *ts_cbp[];	/* command packet buffer pointer, set in tsopen() */
char	*ts_ubmo;	/* offsets packet addresses if unibus map used */
char	*ts_ubcba[];	/* unibus virtual address of command packet buffer */
			/* set in tsopen() to ts_cbp - ts_ubmo */

char	ts_openf[];
u_short	ts_flags[];
daddr_t ts_blkno[];
daddr_t ts_nxrec[];
u_short	ts_erreg[];
u_short	ts_dsreg[];
short	ts_resid[];

/*
 * Block device error log buffer, holds one
 * error log record until the error retry sequence
 * has been completed.
 */

struct tsebuf ts_ebuf[];

int	ts_csr[];	/* CSR address now in config file (c.c) */
/* int	ts_ivec[];	/* interrupt address now in config file (c.c) */
int	nts;		/* NIY number of drives, see c.c */
int	ts_mcact;	/* active flag of ts controller */

	/* bit definitions for command word in ts command packet */

#define	ACK	0100000		/* acknowledge bit */
#define	CVC	040000		/* clear volume check */
#define	OPP	020000		/* opposite. reverse recovery */
#define	SWB	010000		/* swap bytes. for data xfer */
	/* bit definitions for Command mode field during read command */
#define	RNEXT	0		/* read next (forward) */
#define	RPREV	0400		/* read previous (reverse) */
#define	RRPRV	01000		/* reread previous (space rev, read two) */
#define	RRNXT	01400		/* reread next (space fwd, read rev) */
	/* bit definitions for Command mode field during write command */
#define	WNEXT	0		/* Write data next */
#define	WDRTY	01000		/* write data retry , space rev, erase, write data) */
	/* bit definitions for command mode field during position command */
#define	SPCFWD	0		/* space records forward */
#define	SPCREV	0400		/* space records reverse */
#define	SKTPF	01000		/* skip tape marks forward */
#define	SKTPR	01400		/* skip tape marks reverse */
#define	RWIND	02000		/* rewind */

	/* bit definitions for command mode field during format command */
#define	WEOF	0		/* write tape mark */
#define	ERAS	0400		/* erase */
#define	WEOFE	01000		/* write tape mark entry */
	/* bit definitions for command mode field during control command */
#define	MBREAL	0		/* message buffer release */
#define	REWUNL	0400		/* Rewind and unload */
#define	CLEAN	01000		/* clean */
	/* additional definitions */
#define	IEI	0200		/* interrupt enable bit */

	/* command code definitions */
#define	NOP	0
#define	RCOM	01		/* read command */
#define	WCHAR	04		/* write characteristics */
#define	WCOM	05		/* write */
#define	WSUSM	06		/* write subsystem memory */
#define	POSIT	010		/* position command */
#define	FORMT	011		/* Format command */
#define	CONTRL	012		/* Control command */
#define	INIT	013		/* initialize */
#define	GSTAT	017		/* get status immediate */


	/* definition of tssr bits */
#define	SC	0100000		/* special condition */
#define	UPE	040000		/* unibus parity error */
#define	SPE	020000		/* Serial bus parity error */
#define	RMR	010000		/* register modify refused */
#define	NXM	04000		/* non-existent memory */
#define	NBA	02000		/* Need Buffer address */
#define	SSR	0200		/* Sub-System ready */
#define	OFL	0100		/* off-line */

	/* fatal termination class codes */
#define	FTC	030		/* use this as a mask to get codes */
	/* code = 00	see error code byte in tsx3 */
	/* code = 01	I/O seq Crom or main Crom parity error */
	/* code = 10	u-processor Crom parity error,I/O silo parity */
	/*		serial bus parity, or other fatal */
	/* code = 11	A/C low. drive ac low */

	/* termination class codes */
#define	TCC	016		/* mask for termination class codes */
	/* code = 000	normal termination */
	/* code = 001	Attention condition 
	/* code = 010	Tape status alert 
	/* code = 011	Function reject 
	/* code = 100	Recoverable error - tape pos = 1 record down from
			start of function 
	/* code = 101	Recoverable error - tape has not moved
	/* code = 110	Unrecoverable error - tape position lost
	/* code = 111	Fatal controller error - see fatal class bits */

	/* definition of message buffer header word */
#define	MAKC	0100000		/* acknowledge from controller */
#define	MCCF	07400		/* mask for class code field */
	/* class codes are */
	/*	0 = ATTN, on or ofline
		1 = ATTN, microdiagnostic failure
		0 = FAIL, serial bus parity error
		1 = FAIL, WRT LOCK
		2 = FAIL, interlock or non-executable function
		3 = FAIL, microdiagnostic error
	*/

#define	MMSC	037	/* mask for message code field */
	/* message codes are
		020	= end
		021	= FAIL
		022	= ERROR
		023	= Attention

	/* definition of extended status reg 0 bits */
#define	TMK	0100000		/* Tape mark detected */
#define	RLS	040000		/* Record length short */
#define	LET	020000		/* Logical end of Tape */
#define	RLL	010000		/* Record length long */
#define	WLE	04000		/* Write lock error */
#define	NEF	02000		/* Non-executable function */
#define	ILC	01000		/* Illegal command */
#define	ILA	0400		/* Illegal address */
#define	MOT	0200		/* Capistan is moving */
#define	ONL	0100		/* On Line */
#define	IE	040		/* state of interrupt enable bit */
#define	VCK	020		/* Volume Check */
#define	PED	010		/* Phase encoded drive */
#define	WLK	04		/* Write locked */
#define	BOT	02		/* Tape at bot */
#define	EOT	01		/* Tape at eot */

	/* definitions of xstat1 */
#define	DLT	0100000		/* Data late error */
#define	COR	020000		/* Correctable data error */
#define CRS	010000		/* Crease detected */
#define	TIG	04000		/* Trash in the gap */
#define	DBF	02000		/* Deskew Buffer Fail */
#define	SCK	01000		/* Speed check */
#define	IPR	0200		/* Invalid preamble */
#define	SYN	0100		/* Synch Failure */
#define	IPO	040		/* invalid postamble */
#define	IED	020		/* invalid end data */
#define	POS	010		/* postamble short */
#define	POL	04		/* postamble long */
#define	UNC	02		/* Uncorrectable data error */
#define	MTE	01		/* multi track error */

	/* Definitions of XSTAT2 bits */
#define	OPM	0100000		/* operation in progress (tape moving) */
#define	SIP	040000		/* Silo Parity error */
#define	BPE	020000		/* Serial bus Parity error at drive */
#define	CAF	010000		/* Capstan Acceleration fail */
#define	WCF	02000		/* Write card error */
#define	DTP	0477		/* mask for Dead track bits */

	/*	bit definitions for XSTAT3 */
#define	LMX	0200		/* Limit exceeded (tension arms) */
#define	OPI	0100		/* operation incomplete */
#define	REV	040		/* current operation in reverse */
#define	CRF	020		/* capstan response fail */
#define	DCK	010		/* density check */
#define	NOI	04		/* no tape mark or preamble */
#define	LXS	02		/* limit exceeded manual recovery */
#define	RIB	01		/* Reverse into BOT */


#define	SIO	01
#define SSEEK	02
#define	SINIT	04
#define	SRETRY	010
#define	SCOM	020
#define	SOK	040
#define SERROR	0100
#define SREWND	0200

#define	S_WRITTEN	01	/* last command is write command */
#define	S_EOT		010	/* EOT encountered */
#define	S_CSE		020	/* clear EOT */
#define	S_DEOT		040	/* disable EOT */
#define	S_BUSY		0100	/* ctsbuf is busy, switch for clx/cls */
#define	S_WAIT		0200	/* ctsbuf is waited, switch for clx/cls */
#define	S_BUSYF		01000	/* good termination of busy */
#define	S_WAITF		02000	/* good termination of wait */

tsopen(dev, flag)
{
	register char *cba;
	register int ds;
	register struct device *tsaddr;
	int ctrl;

/* if unit > nts or not configured then flag an error and return */
	ctrl = minor(dev) & 077;
	tsaddr = ts_csr[ctrl];
	if (ctrl >= nts || tsaddr == 0)
	{
		u.u_error = ENXIO;
		return;
	}
/*
 * Set up command and message buffer address offset
 * if unibus map present. Also setup command packet
 * buffer pointer (aligned on mod 4 boundry).
 */
	cba = &cmdpkt[5*ctrl];
	if(ubmaps) {	/* UNIBUS map present (much magic to follow) */
		ts_ubmo = (char *)&cfree;
		if(((int)ts_ubmo & 2) && (((int)cba & 2) == 0))
			cba += 2;
		if((((int)ts_ubmo & 2) == 0) && ((int)cba & 2))
			cba += 2;
	} else {
		ts_ubmo = 0;
		if((int)cba & 2)
			cba += 2;
	}
	ts_cbp[ctrl] = cba;	/* save packet buffer pointer for later use */
	ts_ubcba[ctrl] = (char *)cba - ts_ubmo;	/* set UB addr of cmd pkt buf */

	if (flag&FNDELAY)	/* forced open */
	{
		ts_openf[ctrl] = 1;
		return;
	}

/* if unit already open then flag an error and return */
	if (ts_openf[ctrl] > 0)
	{
		u.u_error = ETO;
		return;
	}

	if(tstab[ctrl].b_active & SREWND)
	{	/* this is a delay loop to allow rewind completion	*/
		/* it allows for 2400 feet of tape plus a fudge factor	*/
		/******** this loop assumes lbolt every 4 seconds *******/

		for(ds = 0; ds < 125; ds++)
		{
			if(tsaddr->tssr & SSR)
			{
				ds = 0; 	/* if ready then break	*/
				break;
			}
			else
				sleep(&lbolt, PZERO +1);
		}
		if(ds)
		{
			u.u_error = ETOL;	/* timeout, off-line */
			return;
		}
		else
			tstab[ctrl].b_active = 0;	/* rewind complete	*/
	}

		/* call init. if init returns 1 then failure.
			flag error and return */
	if(tsinit(ctrl, 0))
	{
		u.u_error = ENXIO;
		return;
	}

		/* some house keeping here */
	ts_blkno[ctrl] = 0;			/* set current block # to 0 */
	ts_nxrec[ctrl] = 1000000;		/* set max accessable block # */
	ts_flags[ctrl] &= S_DEOT;
	tstab[ctrl].b_flags |= B_TAPE;	/* Say we are a tape so no write
						ahead */
	if ((ds = tscommand(dev, NOP, 1)) == -1)     /* Get drive status     */
		u.u_error = ENXIO;

		/* if drive is off-line or open for write(read/write)
			and write locked then fatal error	 */
	if(ds & EOT)
		ts_flags[ctrl] |= S_EOT;
	if(tsaddr->tssr & OFL)
		u.u_error = ETOL;
	else if((flag & FWRITE) && (ds & WLK))
		u.u_error = ETWL;

	if(u.u_error == 0)	/* if no error then say opened	*/
		ts_openf[ctrl] = 1;
	else
		ts_openf[ctrl] = 0;
}

tsclose(dev, flag)
{
	int ctrl;

	ctrl = minor(dev) & 077;
	if (ts_openf[ctrl] <= 0)
		return;
	if(tstab[ctrl].b_active & SREWND)
	{
		ts_openf[ctrl] = 0;		/* say closed	*/
		return;
	}
		/* if opened for write or read/write and was
			written then write two tape marks
			and back up one */
	if(((flag & (FWRITE|FREAD)) == FWRITE) ||
	   ((flag & FWRITE) && (ts_flags[ctrl] & S_WRITTEN)))
	{
		if (tscommand(dev, (FORMT|WEOF), 1) == -1)
			goto badcls;
		if (tscommand(dev, (FORMT|WEOF), 1) == -1)
			goto badcls;
		if (tscommand(dev, (SPCREV|POSIT), 1) == -1)
			goto badcls;
	}

		/* if the no-rewind bit in minor is clear then rewind  */
	if ((minor(dev)&0200) ==0 )
	{
		if (tscommand(dev, (RWIND|POSIT), 0) == -1)
			goto badcls;
		ts_flags[ctrl] &= ~S_EOT;
	}
	ts_openf[ctrl] = 0;		/* say closed	*/
	return;

badcls:
	u.u_error = ENXIO;
	return;
}
tsioctl(dev, cmd, data, flag)
	caddr_t data;
	dev_t dev;
{
	register struct buf *bp;
	struct buf *wbp;
	register callcount;
	int ctrl, fcount;
	struct mtop *mtop;
	struct mtget *mtget;
	/* we depend of the values and order of the MT codes here */
	static tsops[] =
	 {FORMT|WEOF,POSIT|SKTPF,POSIT|SKTPR,POSIT|SPCFWD,POSIT|SPCREV,
	  POSIT|RWIND,CONTRL|REWUNL,NOP,NOP,NOP,NOP,INIT,INIT};

	ctrl = minor(dev) & 077;
	bp = &ctsbuf[ctrl];
	switch (cmd) {

	case MTIOCTOP:	/* tape operation */
		mtop = (struct mtop *)data;
		switch (mtop->mt_op) {

		case MTWEOF:
			callcount = mtop->mt_count;
			fcount = 1;
			break;

		case MTFSF: case MTBSF:
		case MTFSR: case MTBSR:
			callcount = 1;
			fcount = mtop->mt_count;
			break;

		case MTREW: case MTOFFL: case MTNOP:
			callcount = 1;
			fcount = 1;
			break;

		case MTCSE:
			ts_flags[ctrl] |= S_CSE;
			return(0);

		case MTENAEOT:
			ts_flags[ctrl] &= ~S_DEOT;
			return(0);

		case MTDISEOT:
			ts_flags[ctrl] |= S_DEOT;
			return(0);

		case MTCACHE: case MTNOCACHE:
		case MTASYNC: case MTNOASYNC:
			return(0);

		case MTCLX:
		case MTCLS:
			if((wbp = tstab[ctrl].b_actf) != 0) {
				tstab[ctrl].b_actf = wbp->av_forw;
				if (wbp != bp) {
					wbp->b_error = EIO;
					wbp->b_flags |= B_ERROR;
					iodone(wbp);
				}
			}
			if (ts_flags[ctrl]&S_BUSY) {
				bp->b_flags &= ~B_BUSY;
				ts_flags[ctrl] &= ~S_BUSYF;
				iodone(bp);
			}
			if (ts_flags[ctrl]&S_WAIT) {
				bp->b_flags &= ~B_WANTED;
				ts_flags[ctrl] &= ~S_WAITF;
				wakeup((caddr_t)bp);
			}
			if(tsinit(ctrl, 0))
			{
				u.u_error = ENXIO;
				return;
			}
			ts_blkno[ctrl] = 0;
			ts_nxrec[ctrl] = 1000000;
			ts_flags[ctrl] &= S_DEOT;
			tstab[ctrl].b_active = 0;
			ts_openf[ctrl] = 0;
			tsstart(ctrl);
			return(0);

		default:
			return (ENXIO);
		}
		if (callcount <= 0 || fcount <= 0)
			return (EINVAL);
		while (--callcount >= 0) {
			if (tscommand(dev, tsops[mtop->mt_op], fcount) == -1)
			{
				u.u_error = ENXIO;
				return(0);
			}
			if ((mtop->mt_op == MTFSR || mtop->mt_op == MTBSR) &&
			    bp->b_resid)
				return (EIO);
			if ((bp->b_flags&B_ERROR) || ts_erreg[ctrl]&BOT)
				break;
		}
		return (geterror(bp));

	case MTIOCGET:
		mtget = (struct mtget *)data;
		mtget->mt_dsreg = 0;
		mtget->mt_erreg = ts_erreg[ctrl];
		mtget->mt_resid = ts_resid[ctrl];
		mtget->mt_softstat = 0;
		if (ts_flags[ctrl]&S_EOT)
			mtget->mt_softstat |= MT_EOT;
		if (ts_flags[ctrl]&S_DEOT)
			mtget->mt_softstat |= MT_DISEOT;
		mtget->mt_type = MT_ISTS;
		break;

	default:
		return (ENXIO);
	}
	return (0);
}
static
tscommand(dev, com, count)
{
	register struct buf *bp;
	int ctrl;

	ctrl = minor(dev) & 077;
	bp = &ctsbuf[ctrl];		/* set buffer pointer to command table	*/
	spl5(); 		/* grab some priority for this	*/
	while(bp->b_flags&B_BUSY)	/* if command table is busy	*/
	{
		bp->b_flags |= B_WANTED;	/* then say we want it	*/
		ts_flags[ctrl] |= (S_WAIT|S_WAITF);
		sleep((caddr_t)bp, PRIBIO);	/* and sleep around	*/
		if ((ts_flags[ctrl]&S_WAITF) == 0)
			return(-1);
	}
	spl0(); 			/* have the buffer so drop pri	*/
	bp->b_dev = dev;		/* associate with this device	*/
	bp->b_resid = com;		/* load desired command here	*/
	bp->b_bcount = count;
	bp->b_blkno = 0;		/* don't need a block number	*/
	bp->b_flags =  B_BUSY|B_READ;	/* mark command buffer busy	*/
	ts_flags[ctrl] |= (S_BUSY|S_BUSYF);
	tsstrategy(bp); 		/* do it			*/
	iowait(bp);			/* wait until it's done 	*/
	if ((ts_flags[ctrl]&S_BUSYF) == 0)
		return(-1);
	if(bp->b_flags&B_WANTED)	/* if someone (me) wants it	*/
	{
		ts_flags[ctrl] &= ~S_WAIT;
		wakeup((caddr_t)bp);	/* then say he can have it	*/
	}
	bp->b_flags &= B_ERROR;		/* say command buffer avail	*/
	return(bp->b_resid);		/* return drive status		*/
}
tsstrategy(bp)
struct buf *bp;
{
	register daddr_t *p;
	int ctrl;

	mapalloc(bp);
	ctrl = minor(bp->b_dev) & 077;
	if(bp != &ctsbuf[ctrl])		/* if not command buffer	*/
	{
		if(ts_openf[ctrl] < 0)
		{
			bp->b_flags |= B_ERROR; /* mark buffer bad	*/
			iodone(bp);
			return;
		}
		p = &ts_nxrec[ctrl];		/* get pointer to max accessable
						block number	*/

#ifdef	UCB_NKB
		if(dbtofsb(bp->b_blkno) > *p)  /* if requested max access */
#else
		if(bp->b_blkno > *p)	/* if requested > max access	*/
#endif	UCB_NKB
		{
			bp->b_flags |= B_ERROR; /* thats a no-no	*/
			bp->b_error = ENXIO;	/* make sure it's known */
			iodone(bp);		/* return buffer	*/
			return; 		/* bye - bye		*/
		}

#ifdef	UCB_NKB
		if(dbtofsb(bp->b_blkno) == *p && bp->b_flags&B_READ)
#else
		if(bp->b_blkno == *p && bp->b_flags&B_READ)
#endif	UCB_NKB
		{
			/* if requested block = max accessable & read  */

			if((bp->b_flags&B_PHYS) == 0)
				clrbuf(bp);		/* zero the buffer */
			bp->b_resid = bp->b_bcount;	/* didn't read	*/
			iodone(bp);		/* return the buffer	*/
			return; 		/* bye - bye		*/
		}

		ts_flags[ctrl] &= ~S_WRITTEN;
		if((bp->b_flags&B_READ) == 0)	/* if a write command	*/
		{
#ifdef	UCB_NKB
			*p = dbtofsb(bp->b_blkno) + 1; /* set max access to requested + 1 */
#else
			*p = bp->b_blkno +1;	/* set max access to requested
							+ 1	*/
#endif	UCB_NKB
			ts_flags[ctrl] |= S_WRITTEN;	/* say it's been written */
		}
	}
	bp->av_forw = 0;		/* clear the available forward link */
	spl5(); 			/* grab some priority for this	*/

	if (tstab[ctrl].b_actf == NULL)	/* if no forward link on the table */
		tstab[ctrl].b_actf = bp;	/* then set this buffer as next */
	else
		tstab[ctrl].b_actl->av_forw = bp;	/* else link this one to
							the end 	*/

	tstab[ctrl].b_actl = bp;	/* set a reverse link in table	*/
	if (tstab[ctrl].b_active == 0)	/* if not active then		*/
		tsstart(ctrl);		/* start some activity		*/
	spl0();
}
static
tsstart(ctrl)
{
	register struct buf *bp;
	register struct device *tsaddr;
	register struct compkt *combuf;
	daddr_t blkno;
	int com;

	combuf = ts_cbp[ctrl];
    loop:
	if ((bp = tstab[ctrl].b_actf) == NULL)
		return; 	/* no link so return	*/
	tsaddr = ts_csr[ctrl];

	blkno = ts_blkno[ctrl];	/* get current block #	*/
	if(tsaddr->tssr & OFL)
	{				/* driver off-line. bad news	*/
		u.u_error = ETOL;	/* tell user, tape off-line */
/*
		printf("\nMagtape %o offline\n",minor(bp->b_dev)&077);
*/
		ts_openf[ctrl] = -1;		/* say this is fatal		*/
		bp->b_flags |= B_ERROR; /* mark buffer bad		*/
		goto next;		/* start the flush loop 	*/
	}
	if (bp == &ctsbuf[ctrl])	/* if this is command table	*/
	{
		switch(bp->b_resid)
		{
			case POSIT|SKTPF:  case POSIT|SKTPR:
			case POSIT|SPCFWD: case POSIT|SPCREV:
				combuf->tsba = bp->b_bcount;

	 		case FORMT|WEOF: case POSIT|RWIND: case CONTRL|REWUNL:
				break;

			case NOP:
				bp->b_resid = mesbuf[ctrl].mstsx0;
				goto next;

			case INIT:
				tsinit(ctrl,1);
				return;

			default:
				printf("Bad ioctl on ts\n");
				return;
		}
		tstab[ctrl].b_active = SCOM;	/* say just general maint command */
		combuf->tscom = (ACK|IEI|bp->b_resid);	/* load command */
		tsaddr->tsdb = ts_ubcba[ctrl]; /* feed driver packet	*/
		return; 		/* later			*/
	}
	if(ts_openf[ctrl] < 0)
	{
		bp->b_error = ETPL;	/* set fatal flag		*/
		bp->b_flags |= B_ERROR; /* mark buffer bad		*/
		goto next;		/* start the flush loop 	*/
	}
#ifdef	UCB_NKB
	if(dbtofsb(bp->b_blkno) > ts_nxrec[ctrl])
#else
	if(bp->b_blkno > ts_nxrec[ctrl])
#endif	UCB_NKB
	{
		bp->b_flags |= B_ERROR; /* block # out of range 	*/
		goto next;		/* start the flush loop 	*/
	}
#ifdef	UCB_NKB
	if(blkno != dbtofsb(bp->b_blkno))  /* if requested != current  */
#else
	if(blkno != bp->b_blkno)	/* if requested != current	  */
#endif	UCB_NKB
	{
		tstab[ctrl].b_active = SSEEK; /* say this is a seek	*/
#ifdef	UCB_NKB
		if(blkno < dbtofsb(bp->b_blkno))  /*we're not there yet so  */
#else
		if(blkno < bp->b_blkno) /* we're not there yet so	*/
#endif	UCB_NKB
		{			/* space forward to it		*/
			combuf->tscom = (ACK|IEI|SPCFWD|POSIT);
#ifdef	UCB_NKB
			combuf->tsba = dbtofsb(bp->b_blkno) - blkno;  /* load block */
#else
			combuf->tsba = bp->b_blkno - blkno; /* load block */
#endif	UCB_NKB
		}
		else
		{			/* need to backup		*/
			combuf->tscom = (ACK|IEI|SPCREV|POSIT);
#ifdef	UCB_NKB
			combuf->tsba = blkno - dbtofsb(bp->b_blkno);  /*load block */
#else
			combuf->tsba = blkno - bp->b_blkno; /* load block */
#endif	UCB_NKB
		}
		tsaddr->tsdb = ts_ubcba[ctrl]; /* feed driver packet	*/
		return; 			/* bye - bye		*/
	}
	tstab[ctrl].b_active = SIO;	/* say this is read or write	*/
	combuf->tsba = bp->b_un.b_addr; /* get low order address	*/
	combuf->tsbae = bp->b_xmem;	/* set extended bits		*/
	combuf->tswc = bp->b_bcount;	/* grab a positive byte count	*/
	combuf->tscom = (ACK|IEI);	/* set up acknowledge and IEI	*/
	if ((ts_flags[ctrl]&S_DEOT) == 0) {
		if ((mesbuf[ctrl].mstsx0&EOT) && (ts_flags[ctrl]&S_CSE) == 0) {
			bp->b_resid = bp->b_bcount;
			bp->b_error = ENOSPC;
			bp->b_flags |= B_ERROR;
			goto next;
		}
	}
	if(bp->b_flags &B_READ)
		combuf->tscom |= (RNEXT|RCOM);	/* read next record	*/
	else
		combuf->tscom |= (WNEXT|WCOM);	/* write next record	*/
	tsaddr->tsdb = ts_ubcba[ctrl]; /* feed packet to drive 	*/
	ts_mcact |= (1 << ctrl);	/* controller active */
	el_bdact |= (1 << TS_BMAJ);	/* device active */
	return;

	/* this loop is good for flushing buffers on fatal errors	*/
next:
	tstab[ctrl].b_active = 0;
	tstab[ctrl].b_actf = bp->av_forw;	/* grab next avail forward */
	if (bp == &ctsbuf[ctrl])
		ts_flags[ctrl] &= ~S_BUSY;
	iodone(bp);			/* say current buffer done	*/
	goto loop;			/* try some more		*/
}
tsintr(dev)
{
	register struct buf *bp;
	register struct device *tsaddr;
	register struct compkt *combuf;
	int	state, *ebp, ctrl, i;

	ctrl = minor(dev) & 077;
	combuf = ts_cbp[ctrl];
	tsaddr = ts_csr[ctrl];
	ts_erreg[ctrl] = mesbuf[ctrl].mstsx0;
	ts_resid[ctrl] = mesbuf[ctrl].msresid;
	if (mesbuf[ctrl].mstsx0&EOT)
		ts_flags[ctrl] |= S_EOT;
	else
		ts_flags[ctrl] &= ~(S_EOT|S_CSE);
	if((bp = tstab[ctrl].b_actf) == 0)	/* grab a buffer pointer */
	{
		logsi(tsaddr);		/* log stray interrupt */
		return;
	}
	state = tstab[ctrl].b_active; 	/* get a copy of status 	*/

	if(tsaddr->tssr & SC)		/* Special Condition Set ??	*/
	{
		/* If initial error, save reg.'s for error log */
		/* The two real registers are saved by fmtbde(), */
		/* the remaining data is loaded from the buffers. */

		if(tstab[ctrl].b_errcnt == 0)
		{
			fmtbde(bp, &ts_ebuf[ctrl], tsaddr, 2, TSDBOFF);
			ts_ebuf[ctrl].ts_bdh.bd_nreg = NTSREG;
			ebp = (char *)ts_cbp[ctrl];	/* addr of command packet */
			for(i=2; i<6; i++)
				ts_ebuf[ctrl].ts_reg[i] = *ebp++;
			ebp = &chrbuf[ctrl];	/* addr of characteristics buffer */
			for( ; i<10; i++)
				ts_ebuf[ctrl].ts_reg[i] = *ebp++;
			ebp = &mesbuf[ctrl];	/* addr of message buffer */
			for( ; i<17; i++)
				ts_ebuf[ctrl].ts_reg[i] = *ebp++;
		}
			/* look at the Termination Class code	*/
		switch ((tsaddr->tssr & TCC) >> 1)
		{
			case 07:	/* fatal sub-system error	*/
			case 06:	/* non-recoverable error	*/
				state = 0;	/* this means dead	*/
				break;	/* break out			*/

			case 05:	/* recoverable. no tape motion	*/
				++tstab[ctrl].b_errcnt;
				state = SRETRY; /* do a straight retry	*/
				break;

			case 04:	/* recoverable. tape position +1 */
				if (state == SIO) {
					if(++tstab[ctrl].b_errcnt < 10)
					{
						if(bp->b_flags & B_READ)
							/* re-read previous	*/
					  	combuf->tscom =(ACK|IEI|RRPRV|RCOM);
						else
							/* write data retry	*/
					   	combuf->tscom=(ACK|IEI|WDRTY|WCOM);
						tsaddr->tsdb = ts_ubcba[ctrl];
						return;
					}
				} else if (ts_openf[ctrl] > 0 && bp != &rtsbuf[ctrl])
					ts_openf[ctrl] = -1;
				break;

			case 03:	/* function reject		*/
					/* if VCK then someone diddled
						with the drive		*/
					/* this is fatal, but not ioctl	*/
				if (state != SCOM)
					state = 0;
				break;

			case 02:	/* Tape Status Alert		*/
				if (state == SCOM)
					break;
				if(mesbuf[ctrl].mstsx0 & (TMK|EOT))
				{	/* End of file/tape set max to this blk */
#ifdef	UCB_NKB
					ts_nxrec[ctrl] = dbtofsb(bp->b_blkno);
#else
					ts_nxrec[ctrl] = bp->b_blkno;
#endif	UCB_NKB
					state = SOK;
					break;
				}
				else if(mesbuf[ctrl].mstsx0&RLS)
				{	/* record length is short. do not
					have to treat this as fatal because
					the actual byte will be returned */

					state = SIO;	/* update block cnt */
					break;
				}
				else if(mesbuf[ctrl].mstsx0&RLL)
				{
					state = SIO;
					bp->b_flags |= B_ERROR;
					break;
				}
				break;
			case 01:	/* Attention Condition		*/
				if(tsaddr->tssr & OFL)
				{
					printf("\nMagtape %o offline\n", ctrl);
					state = 0;	/* drive offline */
				}
				break;
			default:
				break;
		}
		if(tstab[ctrl].b_errcnt >= 10 || state == 0)
		{
			/* bad news. FATAL ERROR or No recovery */
			bp->b_flags |= B_ERROR; /* mark this buffer bad */
			bp->b_error = ETPL;	/* fatal - tape position lost */
			ts_openf[ctrl] = -1;		/* say no more		*/
			if(state)
				tsinit(ctrl, 0);	/* init the drive. no rewind*/
			else
				tsinit(ctrl, 1);	/* init the drive. rewind */
			state = 0;		/* just to be sure	*/
		}
	}
	if((tstab[ctrl].b_active & SREWND) == 0)
		tstab[ctrl].b_active = 0;	/* say ok to do more	*/
	ts_mcact &= ~(1 << ctrl);		/* controller not active */
	if (ts_mcact == 0)
		el_bdact &= ~(1 << TS_BMAJ);	/* device not active */
	/* If possible log the error, if not print it on the console */

	if(tstab[ctrl].b_errcnt || state == 0)
	{
		ts_ebuf[ctrl].ts_bdh.bd_errcnt = tstab[ctrl].b_errcnt;
		if(!logerr(E_BD, &ts_ebuf[ctrl], sizeof(struct tsebuf)))
		{
			/* tssr, mstsx0 */
			deverror(bp, ts_ebuf[ctrl].ts_reg[1], ts_ebuf[ctrl].ts_reg[13]);
			/* mstsx1, mstsx2 */
			deverror(bp, ts_ebuf[ctrl].ts_reg[14], ts_ebuf[ctrl].ts_reg[15]);
			/* mstsx3, retry count */
			deverror(bp, ts_ebuf[ctrl].ts_reg[16], tstab[ctrl].b_errcnt);
			tstab[ctrl].b_errcnt = 0;	/* clear errcnt here, */
						/* because of state 0 in */	
						/* SCOM below. */
		}
	}
	if(state == 0)
		tstab[ctrl].b_errcnt = 0;
	switch (state)
	{
		case SRETRY:			/* straight retry	*/
			break;
		case SIO:			/* update block # and	*/
		case SOK:			/* fall through SCOM	*/
			ts_blkno[ctrl]++;
		case SCOM:
			tstab[ctrl].b_errcnt = 0;	/* no errors		*/
			tstab[ctrl].b_actf = bp->av_forw;	/* next buffer	*/
			if(combuf->tscom & POSIT)	/* if position	*/
				bp->b_resid = 0;	/* this could be
								garbage */
			else
			       bp->b_resid = mesbuf[ctrl].msresid;   /* get residual
								count	*/
			if (bp == &ctsbuf[ctrl])
				ts_flags[ctrl] &= ~S_BUSY;
			iodone(bp);			/* return buffer */
			break;
		case SSEEK:
#ifdef	UCB_NKB
			ts_blkno[ctrl] = dbtofsb(bp->b_blkno);   /* update blk no */
#else
			ts_blkno[ctrl] = bp->b_blkno; 	/* update blk no */
#endif	UCB_NKB
			break;
		default:
			break;
	}
	tsstart(ctrl);		/* start any pending I/O		*/
}

static
tsinit(ctrl, ini)
int ctrl;
int ini;
{
	register struct device *tsaddr;
	register struct compkt *combuf;
	int ds, cnt;

	combuf = ts_cbp[ctrl];
	tsaddr = ts_csr[ctrl];
	if(ini)
	{
		tsaddr->tssr = 0;		/* init with rewind	*/
		tstab[ctrl].b_active = SREWND;
		return(0);
	}
	else
	{
		combuf->tscom = (ACK|CVC|INIT); /* init no rewind	*/
		tsaddr->tsdb = ts_ubcba[ctrl]; /* feed drive packet	*/
		for(cnt = 0; cnt < 30000; cnt++)
		{				/* wait loop. hopefully */
			if(tsaddr->tssr & SSR)	/* 3 - 5 msecs long	*/
				break;		/* have ready. break out*/
		}
	}
	if(cnt >= 30000)
	{					/* wait loop timeout	*/
ts_iex:
		printf("TS11 init failed\n");
		deverror(&ctsbuf[ctrl], tsaddr->tssr, mesbuf[ctrl].mstsx0);
		return(1);			/* bad stuff. say so	*/
	}
	/* show where message buffer is	*/
	chrbuf[ctrl].msbptr = (char *)&mesbuf[ctrl] - ts_ubmo;
	chrbuf[ctrl].msbae = 0;			/* hope no extended addr */
	chrbuf[ctrl].msbsiz = 016;		/* size of message buf	*/
	chrbuf[ctrl].mschar = 0;		/* set characteristics	*/
	combuf->tscom = (ACK|CVC|WCHAR);	/* say this is write
							characteristics */
	combuf->tsba = (char*)&chrbuf[ctrl] - ts_ubmo;	/* addr of chars buf	*/
	combuf->tsbae = 0;			/* hope no ext addr	*/
	combuf->tswc = 010;			/* size of chars buf	*/
	tsaddr->tsdb = ts_ubcba[ctrl]; 	/* feed packet to drive */
	for(cnt = 0; cnt < 30000; cnt++)
	{
		if(tsaddr->tssr & SSR)
			break;
	}
	if(cnt >= 30000)
		goto ts_iex;
	if(((tsaddr->tssr & TCC) >>1) > 1)
		goto ts_iex;
	else
		return(0);			/* good init		*/
}
tsread(dev)
{
	int ctrl;

	ctrl = minor(dev) & 077;
	tsphys(dev);
	physio(tsstrategy, &rtsbuf[ctrl], dev, B_READ);
}
tswrite(dev)
{
	int ctrl;

	ctrl = minor(dev) & 077;
	tsphys(dev);
	physio(tsstrategy, &rtsbuf[ctrl], dev, B_WRITE);
}
static
tsphys(dev)
{
	daddr_t a;
	int ctrl;

	if((ctrl = (minor(dev) & 077)) < nts)	/* only if valid drive */
	{
#ifdef	UCB_NKB
		a = dbtofsb(u.u_offset >>9);  /* grab the block offset  */
#else
		a = u.u_offset >>9;	/* grab the block offset	*/
#endif	UCB_NKB
		ts_blkno[ctrl] = a;		/* set current block		*/
		ts_nxrec[ctrl] = a+1; 	/* set next max block # 	*/
	}
}
