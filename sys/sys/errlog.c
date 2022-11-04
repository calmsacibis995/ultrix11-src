
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)errlog.c	3.0	5/5/86
 */
/*
 * ULTRIX-11 kernel error log functions
 * Fred Canter 5/11/83
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/tmscp.h>	/* must preceed errlog.h (EL_MAXSZ) */
#include <sys/errlog.h>
#include <sys/buf.h>
#include <sys/reg.h>
#include <sys/seg.h>
#include <sys/dir.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <sys/uba.h>

/*
 * Block I/O device error log buffer structure.
 */

struct	elbuf
{
	struct	elrhdr	bd_hdr;
	struct	el_bdh	e_bdh;
	int	e_dreg[];
};

/*
 * (fmtbde) - Format a block I/O device error log record.
 *
 * bp	= I/O buffer pointer
 * ebuf	= pointer to driver error log buffer
 * csr	= device CSR address
 * nreg	= number of device registers
 * dbr	= `data buffer' register offset
 *
 * The contents of the `data buffer' register are not
 * logged, because reading this type of register with
 * no data in the SILO causes a DATA LATE error.
*/

fmtbde(bp, ebuf, csr, nreg, dbr)
register struct buf *bp;
physadr	csr;
{
	register struct elbuf *ebp;
/*	register caddr_t dp; 	OHMS: caused System V C compiler error */
	register physadr dp;
	int i;

	ebp = ebuf;
	dp = csr;
	ebp->bd_hdr.e_time = time;	/* time stamp */
	ebp->e_bdh.bd_dev = bp->b_dev;	/* data from buffer header */
	ebp->e_bdh.bd_flags = bp->b_flags;
	ebp->e_bdh.bd_bcount = bp->b_bcount;
	if(bp->b_flags&B_MAP) {
		struct {
			short ubm_lo;
			short ubm_hi;
		} *ubmp = UBMAP;
		i = (((unsigned)bp->b_un.b_addr >> 13) & 7) 
			| (((unsigned)bp->b_xmem << 3) & 030);
		ebp->e_bdh.bd_addr = ubmp[i].ubm_lo;	/* real physical addr */
		ebp->e_bdh.bd_xmem = ubmp[i].ubm_hi;	/* from UB map reg */
	} else {
		ebp->e_bdh.bd_addr = bp->b_un.b_addr;
		ebp->e_bdh.bd_xmem = bp->b_xmem & 077;
	}
	ebp->e_bdh.bd_blkno = bp->b_blkno;
	ebp->e_bdh.bd_errcnt = 0;	/* driver loads retry counter later */
	ebp->e_bdh.bd_act = el_bdact;	/* block device activity */
	ebp->e_bdh.bd_csr = csr;	/* device CSR address */
	ebp->e_bdh.bd_nreg = nreg;	/* # of device registers */
	ebp->e_bdh.bd_mcact = ra_mcact;	/* MSCP cntlr activity */
	for(i=0; i<nreg; i++) {		/* log device registers */
		if(dbr && (i == dbr))		/* except for data buffer */
			ebp->e_dreg[i] = 0;
		else
			ebp->e_dreg[i] = dp->r[i];
	}
}

/*
 * (logerr) - Add a header & copy record to error log buffer.
 *
 * et	= type of error log record
 * ebuf	= pointer to driver error log buffer
 * sz	= size of driver's error log buffer in bytes
 *
 */

int	el_bact;	/* 0 = buffer empty, else # of records in buffer */
char	*el_bpi = &el_buf;
char	*el_bpo = &el_buf;

logerr(et, ebuf, sz)
struct elbuf *ebuf;
{
	register struct elbuf *debp, *kebp;
	register int *b;
	extern int el_buf[];
	int	hs, pri;
	int	*p;
	mapinfo	map;
	int	nelip;

	if(!el_on && (et != E_SU))
		return(0);	/* error logging disabled */
	if(el_on && (et == E_SU))
		return(0);	/* no startup if error logging already on */
	kebp = el_bpi;		/* kernel error log buffer pointer */
	debp = ebuf;		/* deivce error log buffer pointer */
	hs = sizeof(struct elrhdr);	/* size of record header */
	if(et == E_BD) {		/* block device error, */
		hs = 2;			/* time stamp already in buffer */
		sz -= 2;		/* if error on error log device, */
					/* don't try to log it, instead */
					/* print a message and disable logging */
/* the if statement was using wrong offsets:
		if((debp->e_bdh.b_dev == el_dev)
		 && (debp->e_bdh.b_blkno >= el_sb)
		 && (debp->e_bdh.b_blkno < (el_sb + el_nb))) {
***/
		if((debp->e_bdh.bd_dev == el_dev)
		 && (debp->e_bdh.bd_blkno >= el_sb)
		 && (debp->e_bdh.bd_blkno < (el_sb + el_nb))) {
			printf("\nlogerr: ERROR LOG DEVICE\n");
			el_on = 0;	/* disable error logging */
			return(0);
		}
	}
	/*
	 * Copy the error log record to the kernel buffer.
	 * If the buffer is full, return zero to cause the
	 * `missed error' message to be printed on the console.
	 * The kernel error log buffer is a circular buffer
	 * and is managed by the el_bpi & el_bpo pointers.
	 * If the input and output pointers are equal,
	 * then the buffer is empty.
	 */

	pri = spl7();	/* don't want to be interrupted */
	nelip = el_bpi + hs + sz;
	savemap(map);
	if(el_bact && (el_bpi == el_bpo))
		goto bad;
	if((el_bpi < el_bpo) && (nelip > el_bpo))
		goto bad;
	if(nelip > &el_buf[elbsiz]) {
		kebp->bd_hdr.e_type = 0;	/* mark end of buffer */
		kebp = &el_buf;		/* set pointer to start of buffer */
		el_bpi = &el_buf;
		if((el_bpi + hs + sz) > el_bpo) {
	bad:
			restormap(map);
			splx(pri);
			printf("\nlogerr: MISSED ERROR\n");
			return(0);
		}
	}
	el_bact++;				/* fill in record header */
	kebp->bd_hdr.e_type = et;		/* error record type */
	kebp->bd_hdr.e_size = (hs + sz);	/* total record size in bytes */
	if(et != E_BD)		/* time stamp, except for block device errors */
		kebp->bd_hdr.e_time = time;
	p = el_bpi+sizeof(struct elrhdr);	/* pointer to record body */
	b = debp;				/* pointer to driver buffer */
	if(et == E_BD) {	/* block device buffer has dummy header */
		b++;		/* skip over it (2 bytes) */
		p -= 2;		/* BD buf has time stamp in it */
	}
	sz /= 2;
	while(sz--)
		*p++ = *b++;	/* copy record body to kernel buffer */
	el_bpi = p;			/* now points to start of next record */
	if(et == E_SU)
		el_on = 1;		/* if startup record, enable logging */
	if(et == E_SD)
		el_on = 0;		/* if shutdown, disable logging */
	restormap(map);
	splx(pri);
	wakeup((caddr_t)&el_buf);	/* wakeup error copy process */
	return(1);
}

/*
 * errlog() - error logging control and status system call.
 * 
 *
 * func	= function code (see errlog.h)
 * bufp	= user buffer pointer, for argument return
 */

char	ra_ctid[];	/* RA controller type ID, see ra.c */

errlog()
{
	register struct a {
		int	func;	/* function code */
		int	bufp;	/* user buffer pointer */
		} *uap;
	register char *ubp;
	register int *kbp;
	int	ebuf[5];
	int pri, i;

	uap = (struct a *)u.u_ap;
	if(!suser())
		return;		/* only super-user can use errlog() */
	switch(uap->func) {
	case EL_OFFNL:	/* disable error logging, don't log shutdown */
		el_on = 0;
		break;
	case EL_OFF:	/* disable error logging & log shutdown */
		logerr(E_SD, &ebuf, 0);
		break;
	case EL_ON:	/* enable error logging & and log a startup */
		ebuf[0] = cputype;
		ebuf[1] = rn_ssr3;	/* release # / M/M SSR 3 */
		ebuf[2] = el_bdcw;	/* block device config word */
		ebuf[3] = el_cdcw;	/* char device config word */
/*
 * Save MSCP cntlr types,
 * see conf/uda.c for ra_ctid[] format.
 */
		for(i=3; i>=0; i--) {
			ebuf[4] <<= 4;
			ebuf[4] |= ((ra_ctid[i] >> 4) & 017);
		}
		logerr(E_SU, &ebuf, sizeof(ebuf));	/* log a startup */
		break;
	case EL_INIT:	/* disable logging & init kernel buffer */
		el_on = 0;
		el_bpi = &el_buf;
		el_bpo = &el_buf;
		el_bact = 0;	/* say error log buffer empty */
		el_init = 1;	/* error log buffer reinit flag */
		break;
	case EL_WAIT:	/* sleep elc process at hi priority */
		el_c_pid = u.u_procp->p_pid;  /* save elc process ID */
					     /* so clock.c will not nice it */
		sleep((caddr_t)&el_buf, PZERO - 20);
		break;
	case EL_REC:	/* move an error log record to caller's buffer */
		ubp = uap->bufp;	/* user's buffer pointer */
		if(el_init) {		/* if error log has been init'ed */
			el_init = 0;	/* tell elc process about it  ! */
			suword((caddr_t)ubp, E_INIT);
			break;
		}
		if(el_bpi == el_bpo)
			suword((caddr_t)ubp, E_EOF);	/* no more records */
		else {
			(caddr_t)kbp = el_bpo;
			if((*kbp & 0377) == E_EOF)  /* end of kernel buffer */
				kbp = &el_buf;
			i = *kbp & 0377;	/* check for bad record */
			if(i < 0 || i > E_BD) {	/* bad error type */
		badrec:
				suword((caddr_t)ubp, E_BADR);
				break;
			}
			i = (*kbp >> 8) & 0377;	/* bad size */
			if((i <= 0) || (i & 1) || (i >EL_MAXSZ))
				goto badrec;
			pri = spl7();
			for(i=(((*kbp >> 8)&0377)/2); i; i--) {
				suword((caddr_t)ubp, *kbp++);
				ubp += 2;
			}
			el_bpo = kbp;
			if(el_bpi == el_bpo)
				el_bact = 0;
			splx(pri);
		}
		break;
	case EL_INFO:	/* pass system error log information to elc & elp */
			/* frees elc & elp form dependence on nlist */
		ubp = uap->bufp;	/* pointer to user's buffer */
		suword((caddr_t)ubp, el_dev);
		ubp += 2;
		suword((caddr_t)ubp, (int)(el_sb >> 16));	/* LONG */
		ubp += 2;
		suword((caddr_t)ubp, (int)el_sb);
		ubp += 2;
		suword((caddr_t)ubp, el_nb);
		ubp += 2;
		suword((caddr_t)ubp, nblkdev);
		break;
	default:	/* invalid function code or not super-user */
		psignal(u.u_procp, SIGSYS);
		break;
	}
}

/*
 * Log a stray device interrupt.
 *
 * A stray interrupt is defined as one that occurs for
 * a configured device through a valid vector address,
 * but is unexpected. In the case of big disks, a stray
 * interrupt is logged when the interrupt service routine
 * is entered and the device is not active and no attention
 * summary bits are set.
 */

logsi(csr)
{
	int	si_ebuf[3];

	si_ebuf[0] = csr;	/* device's CSR address */
	si_ebuf[1] = el_bdact;	/* other block device activity */
	si_ebuf[2] = ra_mcact;	/* MSCP controller activity */
	logerr(E_SI, &si_ebuf, sizeof(si_ebuf));
	printf("\nSI %o\n", si_ebuf[0]);
}

/*
 * Interface from unused vectors to the stray
 * vector logging routine.
 */

sv0(dev)
{
	logsv(dev, 0);
}

sv100(dev)
{
	logsv(dev, 0100);
}

sv200(dev)
{
	logsv(dev, 0200);
}

sv300(dev)
{
	logsv(dev, 0300);
}

sv400(dev)
{
	logsv(dev, 0400);
}

sv500(dev)
{
	logsv(dev, 0500);
}

sv600(dev)
{
	logsv(dev, 0600);
}

sv700(dev)
{
	logsv(dev, 0700);
}

/*
 * Log a stray vector, i.e., a vector thru
 * an unused vector location.
 * The most common of these is thru location zero.
 *
 * base	- base address of a group of 16 vector locations
 * dev	- offset into that group
 */

logsv(dev, base)
{
	int	sv_ebuf[3];

	sv_ebuf[0] = base + ((dev &017) << 2);	/* vector address */
	sv_ebuf[1] = el_bdact;	/* other block device activity */
	sv_ebuf[2] = ra_mcact;	/* MSCP controller activity */
	logerr(E_SV, &sv_ebuf, sizeof(sv_ebuf));
	printf("\nSV %o\n", sv_ebuf[0]);
}
