
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)dot.c	3.0	4/22/86
 */
#include <stdio.h>
extern ggi;
dot(xi,yi,dx,n,pat)
int pat[];
{/*
	struct {char pad,c; int xi,yi,dx;} p;
	p.c = 7;
	p.xi = xsc(xi);
	p.yi = ysc(yi);
	p.dx = xsc(dx);
	write(ggi,&p.c,7);
	write(ggi,pat,n?n&0377:256);	*/
}
