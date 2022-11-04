
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/


/*	SCCSID: @(#)gcd.c	3.0	4/22/86	*/
/*	(2.9BSD)  gcd.c	2.2	8/13/82 */

#include <ape.h>

gcd(a,b,c)
MINT *a,*b,*c; /* c = gcd of a and b; computed by
		the Euclidean algorithm */
{	MINT x,y,z,w;
	x.len=y.len=z.len=w.len=0;

	move(a,&x);
	move(b,&y);

	while(y.len!=0)
	{	mdiv(&x,&y,&w,&z);
		move(&y,&x);
		move(&z,&y);
	}
	move(&x,c);
	xfree(&x);
	xfree(&y);
	xfree(&z);
	xfree(&w);
	return;
}

/* Until I know what this ought to do I'm leaving it out

invert(a, b, c)
MINT *a, *b, *c;
{	MINT x, y, z, w, Anew, Aold;
	int i = 0;
	x.len = y.len = z.len = w.len = Aold.len = 0;
	Anew.len = 1;
	Anew.val = xalloc(1,"invert");
	*Anew.val = 1;
	move(b, &x);
	move(a, &y);
	while(y.len != 0)
	{	mdiv(&x, &y, &w, &z);
		move(&Anew, &x);
		mult(&w, &Anew, &Anew);
		madd(&Anew, &Aold, &Anew);
		move(&x, &Aold);
		move(&y, &x);
		move(&z, &y);
		i++;
	}
	move(&Aold, c);
	if( (i&01) == 0) msub(b, c, c);
	xfree(&x);
	xfree(&y);
	xfree(&z);
	xfree(&w);
	xfree(&Aold);
	xfree(&Anew);
}
	I'm adding this, which is sort of an ``invert''! */

reciprocal(a,n,b)
PMINT a,b;		/* b = 10^n / a -- i.e., gives n decimal places
			 * of 1/a. */
int n;
{
	PMINT x;

	new(&x);
	lpow(10,(long)n,x);
	mdiv(x,a,b,x);
	afree(x);
	return;
}
