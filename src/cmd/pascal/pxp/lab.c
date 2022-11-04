
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#
/*	SCCSID: @(#)lab.c	3.0	4/22/86	*/
/*
 * pxp - Pascal execution profiler
 *
 * Bill Joy UCB
 * Version 1.2 January 1979
 */

#include "0.h"

/*
 * Label declaration part
 */
label(r, l)
	int *r, l;
{
	register *ll;

	if (nodecl)
		printoff();
	puthedr();
	setline(l);
	ppnl();
	indent();
	ppkw("label");
	ppgoin(DECL);
	ppnl();
	indent();
	ppbra(NIL);
	ll = r;
	if (ll != NIL)
		for (;;) {
			pplab(ll[1]);
			ll = ll[2];
			if (ll == NIL)
				break;
			ppsep(", ");
		}
	else
		ppid("{label list}");
	ppket(";");
	putcml();
	ppgoout(DECL);
}

/*
 * Goto statement
 */
gotoop(s)
	char *s;
{

	gocnt++;
	ppkw("goto");
	ppspac();
	pplab(s);
}

/*
 * A label on a statement
 */
labeled(s)
	char *s;
{

	linopr();
	indentlab();
	pplab(s);
	ppsep(":");
}
