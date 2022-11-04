
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*	SCCSID: @(#)msqrt.c	3.0	4/22/86	*/
/*	(2.9BSD)  msqrt.c	2.1	7/6/82 */

#include <ape.h>

msqrt(a,b,r) /* b = square root of a - r, where r is the smallest
		possible positive integral remainder.
		The algorithm used is Newton's method.
		The value returned is the length of the
		remainder. */
MINT *a,*b,*r;
{	MINT x,junk,y;
	int j;

	x.len=junk.len=y.len=0;
	if (a->len<0) aperror("msqrt: neg arg");
	if (a->len==0)
		{
		b->len=0;
		r->len=0;
		return(0);
		}
/* Generate the initial approximation: */
	if (a->len%2==1) x.len=(1+a->len)/2;
	else x.len=1+a->len/2;
	x.val=xalloc(x.len,"msqrt");
	for (j = 0; j < x.len; x.val[j++] = 0)
		;
	if (a->len%2==1) x.val[x.len-1]=0400;
	else x.val[x.len-1]=1;
loop:
	mdiv(a,&x,&y,&junk);
	xfree(&junk);
	madd(&x,&y,&y);
	sdiv(&y,2,&y,(short *)&j);
	if (mcmp(&x,&y)>0)
		{
		move(&y,&x);
		xfree(&y);
		goto loop;
		}
	xfree(&y);
	move(&x,b);
	mult(&x,&x,&x);
	msub(a,&x,r);
	xfree(&x);
	return(r->len);
}
