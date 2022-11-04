
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*	SCCSID: @(#)square.c	3.0	4/22/86	*/
/*	(2.9BSD)		*/

#include "lint.h"
#include <stdio.h>
#include <ape.h>

square(a,b)	/* b = a^2, recursive version */
PMINT a,b;
{
	MINT low, high, x, y;
	int half;

	if (a->len == 0) {	/* first base case -- a==0 */
		xfree(b);
		return;
		}
	half = a->len / 2;
	if (half == 0) {	/* second base case -- a->len == 1 */
		long answer;

		answer = ( (long) a->val[0] ) * ( (long) a->val[0] );
		makemint(b,answer);
		return;
		}
	if (half < 0) half = -half;
	low.len = half;
	low.val = a->val;
	high.len = a->len - half;
	high.val = &(a->val[half]);
	x.len = 0;
	y.len = 0;

	square(&low,&x);	/* (low+high)^2=low^2+high^2+2low*high */
	square(&high,&y);
	lshift(&y,half+half);
	madd(&x,&y,&x);
	mult(&low,&high,&y);
	lshift(&y,half);
	madd(&x,&y,&x);
	madd(&x,&y,&x);
	xfree(&y);
	*b = x;
}
