
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#
/*	SCCSID: @(#)cset.c	3.0	4/22/86	*/
/*
 * pxp - Pascal execution profiler
 *
 * Bill Joy UCB
 * Version 1.2 January 1979
 */

#include "0.h"
#include "tree.h"

/*
 * Constant sets
 */
cset(r)
int *r;
{
	register *e, *el;

	ppbra("[");
	el = r[2];
	if (el != NIL)
		for (;;) {
			e = el[1];
			el = el[2];
			if (e == NIL)
				continue;
			if (e[0] == T_RANG) {
				rvalue(e[1], NIL);
				ppsep("..");
				rvalue(e[2], NIL);
			} else
				rvalue(e, NIL);
			if (el == NIL)
				break;
			ppsep(", ");
		}
	ppket("]");
}
