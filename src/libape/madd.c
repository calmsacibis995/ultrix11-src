
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*	SCCSID: @(#)madd.c	3.0	4/22/86	*/
/*	(2.9BSD)  madd.c	2.1	7/6/82 */

#include "lint.h"
#include <ape.h>

m_add(a,b,c)
MINT *a,*b,*c;  /* c = a + b for a->len >= b->len >= 0 */
{	int carry,i;
	int x;
	short *cval; /* just used for convenience */

	cval=xalloc(a->len+1,"m_add");
	carry=0;
	for(i=0;i<b->len;i++)	/* do addition, propagating carries */
	{	x=carry+a->val[i]+b->val[i];
		if(x&CARRYBIT)
		{	carry=1;
			cval[i]=x&TOPSHORT;
		}
		else
		{	carry=0;
			cval[i]=x;
		}
	}
	for(;i<a->len;i++)	/* propagate carries the rest of the way */
	{	x=carry+a->val[i];
		if(x&CARRYBIT) cval[i]=x&TOPSHORT;
		else
		{	carry=0;
			cval[i]=x;
		}

	}
	if(carry==1)
	{	cval[i]=1;
		c->len=i+1;
	}
	else c->len=a->len;
	c->val=cval;
	if(c->len==0) shfree(cval);
	return;
}

madd(a,b,c)  /* c = a + b; keeps track of sign, etc. */
MINT *a,*b,*c;
{	MINT x,y,z;
	int sign;

	x.len=a->len;
	x.val=a->val;
	y.len=b->len;
	y.val=b->val;
	z.len=0;
	sign=1;
	if(x.len>=0)
		if(y.len>=0)
			if(x.len>=y.len) m_add(&x,&y,&z);
			else m_add(&y,&x,&z);
		else
		{	y.len= -y.len;
			msub(&x,&y,&z);
		}
	else	if(y.len<=0)
		{	x.len = -x.len;
			y.len= -y.len;
			sign= -1;
			madd(&x,&y,&z);
		}
		else
		{	x.len= -x.len;
			msub(&y,&x,&z);
		}
	xfree(c);
	c->val=z.val;
	c->len=sign*z.len;
	return;
}

m_sub(a,b,c)
MINT *a,*b,*c;	/* c = a - b for a->len >= b->len >= 0 */
{	int x,i;
	int borrow;

	c->val=xalloc(a->len,"m_sub");
	borrow=0;
	for(i=0;i<b->len;i++) /* do subtraction, propagating borrows */
	{	x=borrow+a->val[i]-b->val[i];
		if(x&CARRYBIT)
		{	borrow= -1;
			c->val[i]=x&TOPSHORT;
		}
		else
		{	borrow=0;
			c->val[i]=x;
		}
	}
	for(;i<a->len;i++)	/* propagate borrows the rest of the way */
	{	x=borrow+a->val[i];
		if(x&CARRYBIT) c->val[i]=x&TOPSHORT;
		else
		{	borrow=0;
			c->val[i]=x;
		}
	}
		/* if a borrow is left over (b>a) we have to take the 2's
		 * complement by flipping c's bits and adding one
		 */
	if (borrow<0)
	{
	short one;
	MINT mone;

		one=1; mone.len= 1; mone.val= &one; /* simple conversion */
		for(i=0;i<a->len;i++) c->val[i] ^= TOPSHORT;
		c->len=a->len;
		madd(c,&mone,c);
	}
	for (i=a->len-1;i>=0;--i)
		if(c->val[i]>0)	/* determine c's length by finding last non-
				 * zero word */
			{	if(borrow==0) c->len=i+1;
				else c->len= -i-1;
				return;
			}
	shfree(c->val); /* Can only get here if the result is zero */
	return;
}

msub(a,b,c)
MINT *a,*b,*c;	/* c = a - b; keeps signs straight, etc. */
{	MINT x,y,z;
	int sign;

	x.len=a->len;
	y.len=b->len;
	x.val=a->val;
	y.val=b->val;
	z.len=0;
	sign=1;
	if(x.len>=0)
		if(y.len>=0)
			if(x.len>=y.len) m_sub(&x,&y,&z);
			else
			{	sign= -1;
				msub(&y,&x,&z);
			}
		else
		{	y.len= -y.len;
			madd(&x,&y,&z);
		}
	else	if(y.len<=0)
		{	sign= -1;
			x.len= -x.len;
			y.len= -y.len;
			msub(&y,&x,&z);
		}
		else
		{	x.len= -x.len;
			madd(&x,&y,&z);
			sign= -1;
		}
	xfree(c);
	c->val=z.val;
	c->len=sign*z.len;
	return;
}
