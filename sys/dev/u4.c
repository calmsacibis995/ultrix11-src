
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985.	      *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/include/COPYRIGHT" for applicable restrictions.  *
 **********************************************************************/
/*
 * SCCSID: @(#)u4.c	3.0	4/21/86
 */

/*
 * u4.c - prototype user device driver
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

/*#define U4_BAEOFF 012	/* Offset from base CSR of Bus Address Extension */
			/* register if the device has one, otherwise */
			/* define as ZERO. */
#define U4_BAEOFF 0

struct device
{
	int	u4cs;		/* Control and status register */
	int	u4wc;		/* Word count register */
	caddr_t	u4ba;		/* Bus address register */
	int	u4da;		/* Disk address register */
	int	u4err;		/* Error register */
	int	u4bae;		/* Bus address extension register */
};

extern int io_csr[];	/* CSR address of device, see c.c */

extern char io_bae[];

#define	U4_NUNIT 0

/* IF DRIVER REQUIRES TTY STRUCTURE(S) */
extern struct tty tty_ts[];
struct tty *u4_tty[2/*#*/] = { &tty_ts[1], &tty_ts[2] /* , etc. */ };
/* # = number of TTY structures required */

/* IF DRIVER DOES NOT REQUIRE TTY STRUCTURES */
/*	struct tty *u4_tty[1];	*/

struct	buf	u4tab;	/* DO NOT REMOVE */

#define U4_BUFSIZ 4	/* TOTAL size of all buffers combined */

union {
	char	u4_mbs[U4_BUFSIZ];	/* TOTAL size of buffers */
	int	u4_mb1;			/* Driver's real definitions */
	/*
	 |
	 v
	 */
} u4_mbuf;	/* DO NOT remove this symbol or change its name!!!! */

/*#define U4_FPUSED 1	/* Define if driver needs floating point support */

#ifdef	U4_FPUSED

struct {
	int	u4_fps;		/* FP status register */
	double	u4_fpr[6];		/* FP registers */
} u4_fpsav;

#endif	U4_FPUSED

int	u4_dead;	/* 1 = device down or not present */

u4open(dev, flag)
{
	register struct device *u4addr;

	if(flag == -1) {	/* Do controller initialization */
		if(fuiword((caddr_t)io_csr[U4_RMAJ]) == -1) {
			u4_dead = 1;	/* Driver configured but device not */
			return;	/* really there - prevent crash */
		}
		/*
		 |
		 |
		 v
		 */
		io_bae[U4_BMAJ] = U4_BAEOFF; /* BAE register offset or zero */
		return;	/* Return when controller init done */
	}
	if(u4_dead) {		/* Device down on not present, return error */
	bad:
		u.u_error=ENXIO;/* to prevent system crash! */
		return;
	}
	u4addr = io_csr[U4_RMAJ];	/* Device's CSR base address */
					/* If device has multiple units, */
					/* may need to adjust CSR address */
					/* for appropriate unit. Remember */
					/* u4addr is a structure pointer! */
	if(minor(dev) >= U4_NUNIT)	/* Example of unit number validation, */
		goto bad;		/* actual code is device dependent */
}

/*
 * The u4select() function services the select system call.
 *
 * The select system call tells a process whether or not an
 * open file descriptor has any I/O pending, i.e., will an I/O
 * request on that file descriptor cause the process to block,
 * waiting for I/O. See select(2) in the ULTRIX-11 Programmer's
 * Manual, Volume 1 for more information on the select system call.
 *
 * The u4select() function should return 0 if there is no I/O pending,
 * i.e., the process would block on an I/O request.  Return 1 if there
 * is I/O ready, i.e., the process will not block.  Also, if there is an
 * error condition that would cause the read/write to fail, return 1.
 *
 * Block mode I/O devices like disks and tapes (assuming the media is
 * on-line) always return 1. Character devices like TTYs return 1 if
 * there are characters waiting to be processed.
 */
u4select(dev, rw)
dev_t	dev;	/* major/minor device number */
int	rw;	/* read/write flag  (FREAD or FWRITE) */
{
	/*
	 * for a Character device like a TTY
		return(ttselect(dev, rw));
	 */
	return(1);
}

u4close(dev, flag)
{
}

u4strategy(bp)
register struct buf *bp;
{
#ifdef	U4_FPUSED

	int	u4_pri;
	double	u4_f1, u4_f2;

#endif	U4_FPUSED
/*	if(!io_bae[U4_BMAJ])	*/
/*		mapalloc(bp);	*/

/*
 * SAMPLE FLOATING POINT CODE
 */
#ifdef	U4_FPUSED

	u4_pri = spl7();	/* Cannot be interrupted */
	savfp(&u4_fpsav);	/* Save FP hardware registers */
	u4_f1 = 1.43;
	u4_f2 = u4_f1 * 345.567;
	restfp(&u4_fpsav);
	splx(u4_pri);

#endif	U4_FPUSED
}

u4start(tp)
register struct tty *tp;
{
	register struct device *u4addr;

	u4addr = io_csr[U4_RMAJ];
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

u4rint(dev)
{
/*
 * If this vector is used and an unexpected interrupt
 * occurs, report the stray interrupt as follows:
 *
 *	if(u4_active == 0) {
 *		logsi(io_csr[U4_RMAJ]);
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

u4xint(dev)
{
/*
 * If this vector is used and an unexpected interrupt
 * occurs, report the stray interrupt as follows:
 *
 *	if(u4_active == 0) {
 *		logsi(io_csr[U4_RMAJ]);
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

u4read(dev)
{
}

u4write(dev)
{
}



u4ioctl(dev, cmd, addr, flag)
caddr_t	addr;
{
}

u4stop(tp, flag)
register struct tty *tp;
{
}

