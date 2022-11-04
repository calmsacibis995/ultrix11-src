
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985.	      *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/include/COPYRIGHT" for applicable restrictions.  *
 **********************************************************************/
/*
 * SCCSID: @(#)u3.c	3.0	4/21/86
 */

/*
 * u3.c - prototype user device driver
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

/*#define U3_BAEOFF 012	/* Offset from base CSR of Bus Address Extension */
			/* register if the device has one, otherwise */
			/* define as ZERO. */
#define U3_BAEOFF 0

struct device
{
	int	u3cs;		/* Control and status register */
	int	u3wc;		/* Word count register */
	caddr_t	u3ba;		/* Bus address register */
	int	u3da;		/* Disk address register */
	int	u3err;		/* Error register */
	int	u3bae;		/* Bus address extension register */
};

extern int io_csr[];	/* CSR address of device, see c.c */

extern char io_bae[];

#define	U3_NUNIT 0

/* IF DRIVER REQUIRES TTY STRUCTURE(S) */
extern struct tty tty_ts[];
struct tty *u3_tty[2/*#*/] = { &tty_ts[1], &tty_ts[2] /* , etc. */ };
/* # = number of TTY structures required */

/* IF DRIVER DOES NOT REQUIRE TTY STRUCTURES */
/*	struct tty *u3_tty[1];	*/

struct	buf	u3tab;	/* DO NOT REMOVE */

#define U3_BUFSIZ 4	/* TOTAL size of all buffers combined */

union {
	char	u3_mbs[U3_BUFSIZ];	/* TOTAL size of buffers */
	int	u3_mb1;			/* Driver's real definitions */
	/*
	 |
	 v
	 */
} u3_mbuf;	/* DO NOT remove this symbol or change its name!!!! */

/*#define U3_FPUSED 1	/* Define if driver needs floating point support */

#ifdef	U3_FPUSED

struct {
	int	u3_fps;		/* FP status register */
	double	u3_fpr[6];		/* FP registers */
} u3_fpsav;

#endif	U3_FPUSED

int	u3_dead;	/* 1 = device down or not present */

u3open(dev, flag)
{
	register struct device *u3addr;

	if(flag == -1) {	/* Do controller initialization */
		if(fuiword((caddr_t)io_csr[U3_RMAJ]) == -1) {
			u3_dead = 1;	/* Driver configured but device not */
			return;	/* really there - prevent crash */
		}
		/*
		 |
		 |
		 v
		 */
		io_bae[U3_BMAJ] = U3_BAEOFF; /* BAE register offset or zero */
		return;	/* Return when controller init done */
	}
	if(u3_dead) {		/* Device down on not present, return error */
	bad:
		u.u_error=ENXIO;/* to prevent system crash! */
		return;
	}
	u3addr = io_csr[U3_RMAJ];	/* Device's CSR base address */
					/* If device has multiple units, */
					/* may need to adjust CSR address */
					/* for appropriate unit. Remember */
					/* u3addr is a structure pointer! */
	if(minor(dev) >= U3_NUNIT)	/* Example of unit number validation, */
		goto bad;		/* actual code is device dependent */
}

/*
 * The u3select() function services the select system call.
 *
 * The select system call tells a process whether or not an
 * open file descriptor has any I/O pending, i.e., will an I/O
 * request on that file descriptor cause the process to block,
 * waiting for I/O. See select(2) in the ULTRIX-11 Programmer's
 * Manual, Volume 1 for more information on the select system call.
 *
 * The u3select() function should return 0 if there is no I/O pending,
 * i.e., the process would block on an I/O request.  Return 1 if there
 * is I/O ready, i.e., the process will not block.  Also, if there is an
 * error condition that would cause the read/write to fail, return 1.
 *
 * Block mode I/O devices like disks and tapes (assuming the media is
 * on-line) always return 1. Character devices like TTYs return 1 if
 * there are characters waiting to be processed.
 */
u3select(dev, rw)
dev_t	dev;	/* major/minor device number */
int	rw;	/* read/write flag  (FREAD or FWRITE) */
{
	/*
	 * for a Character device like a TTY
		return(ttselect(dev, rw));
	 */
	return(1);
}

u3close(dev, flag)
{
}

u3strategy(bp)
register struct buf *bp;
{
#ifdef	U3_FPUSED

	int	u3_pri;
	double	u3_f1, u3_f2;

#endif	U3_FPUSED
/*	if(!io_bae[U3_BMAJ])	*/
/*		mapalloc(bp);	*/

/*
 * SAMPLE FLOATING POINT CODE
 */
#ifdef	U3_FPUSED

	u3_pri = spl7();	/* Cannot be interrupted */
	savfp(&u3_fpsav);	/* Save FP hardware registers */
	u3_f1 = 1.43;
	u3_f2 = u3_f1 * 345.567;
	restfp(&u3_fpsav);
	splx(u3_pri);

#endif	U3_FPUSED
}

u3start(tp)
register struct tty *tp;
{
	register struct device *u3addr;

	u3addr = io_csr[U3_RMAJ];
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

u3rint(dev)
{
/*
 * If this vector is used and an unexpected interrupt
 * occurs, report the stray interrupt as follows:
 *
 *	if(u3_active == 0) {
 *		logsi(io_csr[U3_RMAJ]);
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

u3xint(dev)
{
/*
 * If this vector is used and an unexpected interrupt
 * occurs, report the stray interrupt as follows:
 *
 *	if(u3_active == 0) {
 *		logsi(io_csr[U3_RMAJ]);
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

u3read(dev)
{
}

u3write(dev)
{
}



u3ioctl(dev, cmd, addr, flag)
caddr_t	addr;
{
}

u3stop(tp, flag)
register struct tty *tp;
{
}

