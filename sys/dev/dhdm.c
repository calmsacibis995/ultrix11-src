
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)dhdm.c	3.0	4/21/86
 */
/*
 *	DM-BB driver
 *
 *	Modifed to allow use of DM11-BB MCU
 *	equiped with an M7147 in place of M7808.
 *
 *	The origonal dhdm driver did not wait for
 *	the MCU scan busy bit to go to zero prior
 *	to changing the line number in the MCU CSR.
 *	The origonal driver functions properly with the
 *	M7808 module, however, if the M7147 is used the
 *	modems will not be properly initialized.
 *	This dhdm driver waits for MCU scan busy to
 *	go to zero prior to changing the line number.
 *
 * 	Also modified to allow for local or remote
 *	operaton of terminals.
 *
 *	Fred Canter 11/6/82
 *
 *	Changes for O_NDELAY flag in open(): George Mathew 7/24/85
 */
#include <sys/param.h>
#include <sys/tty.h>
#include <sys/conf.h>
#include <sys/devmaj.h>
#include <sys/file.h>	/* needed for FREAD & FWRITE */

int	io_csr[];	/* CSR address now in config file (c.c) */

struct	tty *dh11[];
int	ndh11;		/* Set by dh.c to number of lines */
int	dh_local[];	/* bit per line, 1 = local tty */

#define	DONE	0200
#define	SCENABL	040
#define	CLSCAN	01000
#define	TURNON	03	/* CD lead, line enable */
#define	SECX	010	/* secondary xmit */
#define	RQS	04	/* request to send */
#define	TURNOFF	1	/* line enable only */
#define	CARRIER	0100
#define	CLS	040	/* clear to send */
#define	SECR	020	/* secondary receive */
#define SBUSY	020	/* MCU scan busy */

struct device
{
	int	dmcsr;
	int	dmlstat;
	int	junk[2];
};

#define	B1200	9
#define	B300	7

/*
 * Turn on the line associated with the (DH) device dev.
 */
dmopen(dev, flag)
{
	register struct tty *tp;
	register struct device *addr;
	register d;

	d = minor(dev);
	tp = dh11[d];
	if(dh_local[(d>>4) & 7] & (1 << (d&017))) {
		tp->t_state |= CARR_ON;
		return;
		}
	addr = io_csr[DM_RMAJ];
	addr += d>>4;
	spl5();
	addr->dmcsr &= ~SCENABL;	/* stop MCU scan */
	while (addr->dmcsr & SBUSY);	/* wait for busy = 0 */
	addr->dmcsr = d&017;
	addr->dmlstat = TURNON;
	if (addr->dmlstat&CARRIER) {
		tp->t_state |= CARR_ON;
	}
	addr->dmcsr = IENABLE|SCENABL;
	if((flag&FNDELAY) == 0)
		while ((tp->t_state&CARR_ON)==0)
			sleep((caddr_t)&tp->t_rawq, TTIPRI);
	addr->dmcsr &= ~SCENABL;
	while (addr->dmcsr & SBUSY);
	addr->dmcsr = d&017;
	if (addr->dmlstat&SECR) {
		tp->t_ispeed = B1200;
		tp->t_ospeed = B1200;
		dhparam(dev);
	}
	addr->dmcsr = IENABLE|SCENABL;
	spl0();
}

/*
 * Dump control bits into the DM registers.
 */
dmctl(dev, bits)
{
	register struct device *addr;
	register d, s;

	d = minor(dev);
	addr = io_csr[DM_RMAJ];
	addr += d>>4;
	s = spl5();
	addr->dmcsr &= ~SCENABL;
	while (addr->dmcsr & SBUSY);
	addr->dmcsr = d&017;
	addr->dmlstat = bits;
	addr->dmcsr = IENABLE|SCENABL;
	splx(s);
}

/*
 * DM11 interrupt.
 * Mainly, deal with carrier transitions.
 */
dmint(dev)
{
	register struct tty *tp;
	register struct device *addr;
	register d;
	int ln;

	d = minor(dev);
	addr = io_csr[DM_RMAJ];
	addr += d;
	while (addr->dmcsr & SBUSY);
	if (addr->dmcsr&DONE) {
		ln = (d<<4) + (addr->dmcsr&017);
		tp = dh11[ln];
		if ((ln < ndh11) && tp) {
			wakeup((caddr_t)&tp->t_rawq);
			if ((addr->dmlstat&CARRIER)==0 && 
				(dh_local[d] & (1 << (ln&017))) == 0) {
				if ((tp->t_state&WOPEN)==0) {
					gsignal(tp->t_pgrp, SIGHUP);
					addr->dmlstat = 0;
					flushtty(tp, FREAD|FWRITE);
				}
				tp->t_state &= ~CARR_ON;
			} else {
				tp->t_state |= CARR_ON;
			}
		}
		addr->dmcsr = IENABLE|SCENABL;
	}
}
