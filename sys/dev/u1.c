
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985.	      *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/include/COPYRIGHT" for applicable restrictions.  *
 **********************************************************************/
/*
 * SCCSID: @(#)u1.c	3.0	4/21/86
 */

/*
 * u1.c - prototype user device driver
 *
 *	*********************************************************
 *	*  This file is a device driver template. It contains	*
 *	*  the empty functions and data structures that allow	*
 *	*  you to interface your device driver to the ULTRIX-11	*
 *	*  operating system. For a commentary on how to write a	*
 *	*  device driver refer to Appendix H in the ULTRIX-11	*
 *	*  System Management Guide.				*
 *	*  For instructions on installing your driver refer to	*
 *	*  Section 2.8 in ULTRIX-11 System Management Guide.	*
 *	*********************************************************
 *
 * ULTRIX-11 device drivers consist of a variable number
 * of lines of C code, sometimes interspersed with comments,
 * arranged in the usual fashion!
 *
 * 1.	Do not remove or rename any of the functions in this
 *	prototype driver, they are required in order to satisfy
 *	references in c.c.
 *
 * 2.	User drivers cannot interface to the error logging system.
 *	Use deverror() or printf to send messages to the console.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/devmaj.h>	/* defines major/minor device numbers */
#include <sys/buf.h>
#include <sys/tty.h>
#include <sys/dir.h>
#include <sys/user.h>
/*
 * Add other header files as required.
 */

/*#define U1_BAEOFF 012	/* Offset from base CSR of Bus Address Extension */
			/* register if the device has one, otherwise */
			/* define as ZERO. */
#define U1_BAEOFF 0

struct device
{
	int	u1cs;		/* Control and status register */
	int	u1wc;		/* Word count register */
	caddr_t	u1ba;		/* Bus address register */
	int	u1da;		/* Disk address register */
	int	u1err;		/* Error register */
	int	u1bae;		/* Bus address extension register */
};

extern int io_csr[];	/* CSR address of device, see c.c */

extern char io_bae[];

#define	U1_NUNIT 0

/* IF DRIVER REQUIRES TTY STRUCTURE(S) */
extern struct tty tty_ts[];
struct tty *u1_tty[2/*#*/] = { &tty_ts[1], &tty_ts[2] /* , etc. */ };
/* # = number of TTY structures required */

/* IF DRIVER DOES NOT REQUIRE TTY STRUCTURES */
/*	struct tty *u1_tty[1];	*/

struct	buf	u1tab;	/* DO NOT REMOVE */

#define U1_BUFSIZ 4	/* TOTAL size of all buffers combined */

union {
	char	u1_mbs[U1_BUFSIZ];	/* TOTAL size of buffers */
	int	u1_mb1;			/* Driver's real definitions */
	/*
	 |
	 v
	 */
} u1_mbuf;	/* DO NOT remove this symbol or change its name!!!! */

/*#define U1_FPUSED 1	/* Define if driver needs floating point support */

#ifdef	U1_FPUSED

struct {
	int	u1_fps;		/* FP status register */
	double	u1_fpr[6];		/* FP registers */
} u1_fpsav;

#endif	U1_FPUSED

int	u1_dead;	/* 1 = device down or not present */

u1open(dev, flag)
{
	register struct device *u1addr;

	if(flag == -1) {	/* Do controller initialization */
		if(fuiword((caddr_t)io_csr[U1_RMAJ]) == -1) {
			u1_dead = 1;	/* Driver configured but device not */
			return;	/* really there - prevent crash */
		}
		/*
		 |
		 |
		 v
		 */
		io_bae[U1_BMAJ] = U1_BAEOFF; /* BAE register offset or zero */
		return;	/* Return when controller init done */
	}
	if(u1_dead) {		/* Device down on not present, return error */
	bad:
		u.u_error=ENXIO;/* to prevent system crash! */
		return;
	}
	u1addr = io_csr[U1_RMAJ];	/* Device's CSR base address */
					/* If device has multiple units, */
					/* may need to adjust CSR address */
					/* for appropriate unit. Remember */
					/* u1addr is a structure pointer! */
	if(minor(dev) >= U1_NUNIT)	/* Example of unit number validation, */
		goto bad;		/* actual code is device dependent */
}

/*
 * The u1select() function services the select system call.
 *
 * The select system call tells a process whether or not an
 * open file descriptor has any I/O pending, i.e., will an I/O
 * request on that file descriptor cause the process to block,
 * waiting for I/O. See select(2) in the ULTRIX-11 Programmer's
 * Manual, Volume 1 for more information on the select system call.
 *
 * The u1select() function should return 0 if there is no I/O pending,
 * i.e., the process would block on an I/O request.  Return 1 if there
 * is I/O ready, i.e., the process will not block.  Also, if there is an
 * error condition that would cause the read/write to fail, return 1.
 *
 * Block mode I/O devices like disks and tapes (assuming the media is
 * on-line) always return 1. Character devices like TTYs return 1 if
 * there are characters waiting to be processed.
 */
u1select(dev, rw)
dev_t	dev;	/* major/minor device number */
int	rw;	/* read/write flag  (FREAD or FWRITE) */
{
	/*
	 * for a Character device like a TTY
		return(ttselect(dev, rw));
	 */
	return(1);
}

u1close(dev, flag)
{
}

u1strategy(bp)
register struct buf *bp;
{
#ifdef	U1_FPUSED

	int	u1_pri;
	double	u1_f1, u1_f2;

#endif	U1_FPUSED
/*	if(!io_bae[U1_BMAJ])	*/
/*		mapalloc(bp);	*/

/*
 * SAMPLE FLOATING POINT CODE
 */
#ifdef	U1_FPUSED

	u1_pri = spl7();	/* Cannot be interrupted */
	savfp(&u1_fpsav);	/* Save FP hardware registers */
	u1_f1 = 1.43;
	u1_f2 = u1_f1 * 345.567;
	restfp(&u1_fpsav);
	splx(u1_pri);

#endif	U1_FPUSED
}

u1start(tp)
register struct tty *tp;
{
	register struct device *u1addr;

	u1addr = io_csr[U1_RMAJ];
	/* may need to add the unit number depending on device type */

	/*
	 * For a character device like a TTY, you will probably wake
	 * up anyone who is sleeping on the output queue if the
	 * character count has dropped below the high water mark.
	 * At that point you need to wake up anyone who is selecting
	 * this line.
	 */
		if (tp->t_outq.c_cc <= TTLOWAT(tp)) {
			/*
			 * Regular code to wake up people sleeping
			 * on the output queue goes here...
			 */
			/* BEGIN SELECT CODE */
			if (tp->t_wsel) {
				selwakeup(tp->t_wsel, tp->t_state & TS_WCOLL);
				tp->t_wsel =0;
				tp->t_state &= ~TS_WCOLL;
			}
			/* END SELECT CODE */
		}
}

u1rint(dev)
{
/*
 * If this vector is used and an unexpected interrupt
 * occurs, report the stray interrupt as follows:
 *
 *	if(u1_active == 0) {
 *		logsi(io_csr[U1_RMAJ]);
 *		return;
 *	}
 *
 * If this vector is not used and an interrupt occurs,
 * report it as follows:
 *
 *	logsi(vector);
 *	return;
 *
 * Where (vector) is the device's interrupt vector address.
 */
}

u1xint(dev)
{
/*
 * If this vector is used and an unexpected interrupt
 * occurs, report the stray interrupt as follows:
 *
 *	if(u1_active == 0) {
 *		logsi(io_csr[U1_RMAJ]);
 *		return;
 *	}
 *
 * If this vector is not used and an interrupt occurs,
 * report it as follows:
 *
 *	logsi(vector+04);
 *	return;
 *
 * Where (vector) is the device's interrupt vector address.
 */
}

u1read(dev)
{
}

u1write(dev)
{
}



u1ioctl(dev, cmd, addr, flag)
caddr_t	addr;
{
}

u1stop(tp, flag)
register struct tty *tp;
{
}

