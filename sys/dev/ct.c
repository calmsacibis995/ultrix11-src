
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)ct.c	3.0	4/21/86
 */
/*
 * GP DR11C driver used for C/A/T
 *
 * Modified for V7M-11
 * Fred Canter 7/13/83
 */

#include <sys/param.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/tty.h>
#include <sys/devmaj.h>

#define	PCAT	(PZERO+9)
#define	CATHIWAT	100
#define	CATLOWAT	30

struct {
	int	catlock;
	struct	clist	oq;
} cat;

struct catdev {
	int	catcsr;
	int	catbuf;
};

int	io_csr[];	/* CSR address, see c.c */

ctopen(dev)
{
	register struct catdev *cataddr;

	cataddr = io_csr[CT_RMAJ];
	if (cat.catlock==0) {
		cat.catlock++;
		cataddr->catcsr |= IENABLE;
	} else
		u.u_error = ENXIO;
}

ctclose()
{
	cat.catlock = 0;
	ctintr();
}

ctwrite(dev)
{
	register c;
	extern lbolt;

	while ((c=cpass()) >= 0) {
		spl5();
		while (cat.oq.c_cc > CATHIWAT)
			sleep((caddr_t)&cat.oq, PCAT);
		while (putc(c, &cat.oq) < 0)
			sleep((caddr_t)&lbolt, PCAT);
		ctintr();
		spl0();
	}
}

ctintr()
{
	register struct catdev *cataddr;
	register int c;

	cataddr = io_csr[CT_RMAJ];
	if (cataddr->catcsr&DONE) {
		if ((c = getc(&cat.oq)) >= 0) {
			cataddr->catbuf = c;
			if (cat.oq.c_cc==0 || cat.oq.c_cc==CATLOWAT)
				wakeup((caddr_t)&cat.oq);
		} else {
			if (cat.catlock==0)
				cataddr->catcsr = 0;
		}
	}
}
