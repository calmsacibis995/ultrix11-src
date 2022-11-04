
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*	SCCSID: @(#)mdiv.c	3.0	4/22/86	*/
/*	(2.9BSD)  mdiv.c	2.1	7/6/82 */

#include "lint.h"
#include <ape.h>

sdiv(a,n,q,r)  /* q is quotient, r remainder of a/n for n int */
MINT *a,*q; short *r;
{	MINT x,y;
	int sign;

	if (n==0) aperror("sdiv: zero divisor");
	sign=1;
	x.len=a->len;
	x.val=a->val;
	if (n<0)
	{	sign= -sign;
		n= -n;
	}
	if (x.len<0)
	{	sign = -sign;
		x.len= -x.len;
	}
	s_div(&x,n,&y,r);
	xfree(q);
	q->val=y.val;
	q->len=sign*y.len;
	*r = *r*sign;
	return;
}

s_div(a,n,q,r)  /* dirtywork function for sdiv */
MINT *a,*q; short *r;
{	int qlen,i;
	long int x;
	short *qval;

	if (n==0) aperror("sdiv: zero divisor");
	x=0L;
	qlen=a->len;
	qval=xalloc(qlen,"s_div");	/* I guess qlen and qval are only used
				  	 * to cut down on the amount of typing
				  	 * required. */
	for(i=qlen-1;i>=0;i--)
	{
		x=x*LONGCARRY+a->val[i];
		qval[i]=x/n;
		x=x%n;
	}
	*r=x;
	if (qval[qlen-1]==0) qlen--;
	q->len=qlen;
	q->val=qval;
	if (qlen==0) shfree(qval);
	return;
}

mdiv(a,b,q,r)  /* q = quotient, r = remainder, of a/b */
MINT *a,*b,*q,*r;
{	MINT x,y,w,z;
	int sign;

	if (b->len == 0) aperror("mdiv: zero divisor");
	sign=1;
	x.val=a->val;
	y.val=b->val;
	x.len=a->len;
	if(x.len<0) {sign= -1; x.len= -x.len;}
	y.len=b->len;
	if(y.len<0) {sign= -sign; y.len= -y.len;}
	z.len = w.len = 0;
	m_div(&x,&y,&z,&w);
	xfree(q);
	xfree(r);
	q->val = z.val; q->len = z.len;
	r->val = w.val; r->len = w.len;
	if(sign==-1)
	{	q->len = -q->len;
		r->len = -r->len;
	}
	return;
}

m_dsb(q,n,a,b)
short *a,*b;
{	long int x,qx;
	int borrow,j,u;

	qx=q;
	/* qx is used merely to force arithmetic
	 * to be done long (double integral precision) */
	borrow=0;
	for(j=0;j<n;j++)
	{	x=borrow-a[j]*qx+b[j];
		b[j]=x&TOPSHORT;
		borrow=x>>15;
	}
	x=borrow+b[j];
	b[j]=x&TOPSHORT;
	if (x>>15 ==0)  return(0);
	borrow=0;
	for(j=0;j<n;j++)
	{	u=a[j]+b[j]+borrow;
		if(u<0) borrow=1;
		else borrow=0;
		b[j]=u&TOPSHORT;
	}
	return(1);
}

m_trq(v1,v2,u1,u2,u3)
{	long int d;
	long int x1;

	if(u1==v1) d=TOPSHORT;
	else d=(u1*LONGCARRY+u2)/v1;
	while(1)
	{	x1=u1*LONGCARRY+u2-v1*d;
		x1=x1*LONGCARRY+u3-v2*d;
		if(x1<0) d=d-1;
		else return(d);
	}
}

m_div(a,b,q,r)  /* basic "long division" routine */
MINT *a,*b,*q,*r;
{	MINT u,v,x,w;
	short d,*qval;
	int qq,n,v1,v2,j;

	u.len=v.len=x.len=w.len=0;
	if (b->len==0) { aperror("mdiv divide by zero"); return;}
	if (b->len==1)
	{	r->val=xalloc(1,"m_div1");
		sdiv(a,b->val[0],q,r->val);
		if (r->val[0]==0)
			{
			xfree(r);
			return;
			}
		else r->len=1;
		return;
	}
	if (mcmp(a,b) <0)
	{	xfree(q);
		move(a,r);
		return;
	}
	x.len=1;
	x.val = &d;
	n=b->len;
	d=LONGCARRY/(b->val[n-1]+1L);
	mult(a,&x,&u); /*subtle: relies on fact that mult allocates extra space */
	mult(b,&x,&v);
	v1=v.val[n-1];
	v2=v.val[n-2];
	qval=xalloc(a->len-n+1,"m_div3");
	for(j=a->len-n;j>=0;j--)
	{	qq=m_trq(v1,v2,u.val[j+n],u.val[j+n-1],u.val[j+n-2]);
		if(m_dsb(qq,n,v.val,&(u.val[j]))) qq -= 1;
		qval[j]=qq;
	}
	x.len=n;
	x.val=u.val;
	mcan(&x);
	sdiv(&x,d,&w,(short *)&qq);
	r->len=w.len;
	r->val=w.val;
	q->val=qval;
	qq=a->len-n+1;
	if(qq>0 && qval[qq-1]==0) qq -= 1;
	q->len=qq;
	if(qq==0) shfree(qval);
	if(x.len!=0) xfree(&u);
	xfree(&v);
	return;
}
