
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)sys.c	3.0	4/21/86
 */
/*
 *	indirect driver for controlling tty.
 */
#include <sys/param.h>
#include <sys/conf.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/tty.h>
#include <sys/proc.h>

syopen(dev, flag)
{

	if(u.u_ttyp == NULL) {
		u.u_error = ENXIO;
		return;
	}
	(*cdevsw[major(u.u_ttyd)].d_open)(u.u_ttyd, flag);
}

syread(dev)
{
#ifdef	UCB_NET
	if(u.u_ttyp == NULL) {
		u.u_error = ENXIO;
		return;
	}
#endif	UCB_NET

	(*cdevsw[major(u.u_ttyd)].d_read)(u.u_ttyd);
}

sywrite(dev)
{
#ifdef	UCB_NET
	if(u.u_ttyp == NULL) {
		u.u_error = ENXIO;
		return;
	}
#endif	UCB_NET

	(*cdevsw[major(u.u_ttyd)].d_write)(u.u_ttyd);
}

sysioctl(dev, cmd, addr, flag)
caddr_t addr;
{
#ifdef	UCB_NET
	if (cmd == TIOCNOTTY) {
		u.u_ttyp = 0;
		u.u_ttyd = 0;
		u.u_procp->p_pgrp = 0;
		return;
	}

	if (u.u_ttyp == NULL) {
		u.u_error = ENXIO;
		return;
	}
#endif	UCB_NET

	(*cdevsw[major(u.u_ttyd)].d_ioctl)(u.u_ttyd, cmd, addr, flag);
}

#ifdef	SELECT
syselect(dev, flag)
{
	if (u.u_ttyp == NULL) {
		u.u_error = ENXIO;
		return(0);
	}
	return ((*cdevsw[major(u.u_ttyd)].d_select)(u.u_ttyd, flag));
}
#endif	SELECT
