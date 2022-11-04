
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#
/*
 * SCCSID: @(#)palloc.c	3.0	4/22/86
 */

#include "0x.h"
#include "E.h"

extern char *sbrk(), *brk();

alloc(need)
	char *need;
{
	register cnt, *wp;
	register char *have;

	need = (((int)need+1) &~ 1);
	if ((have=high-memptr) < need) {
		if (sbrk(need > have + 1024 ? need-have:1024) == -1)
			error(EOUTOFMEM);
		high = sbrk(0);
	}
	wp = memptr;
	cnt = (((int)need >> 1) & 077777);
	do {
		*wp++ = 0;
	} while (--cnt);
	wp = memptr;
	memptr =+ need;
	stklim();
	return(wp);
}

setmem()
{
	high = bottmem = memptr = sbrk(0);
	stklim();
}

free(cptr)
	char *cptr;
{
}

stklim()
{
	maxstk = (((int)memptr + 07777) &~ 07777) + 512;
}
