
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)dhfdm.c	3.0	4/21/86
 */
/*
 *	DM-BB fake driver
 */
#include <sys/param.h>
#include <sys/tty.h>
#include <sys/conf.h>

struct	tty	*dh11[];

dmopen(dev)
{
	register struct tty *tp;

	tp = dh11[minor(dev)];
	tp->t_state |= CARR_ON;
}

dmctl(dev, bits)
{
}
