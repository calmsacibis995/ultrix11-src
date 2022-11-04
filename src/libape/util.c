
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*	SCCSID: @(#)util.c	3.0	4/22/86	*/
/*	(2.9BSD)  util.c	2.2	8/13/82 */

#include "lint.h"
#include <stdio.h>
#include <ape.h>

move(a,b)
MINT *a,*b; /* copies a onto b; a is left unchanged */
{	int i,j;
	xfree(b);
	b->len=a->len;
	if((i=a->len)<0) i = -i;
	if(i==0) return;
	b->val=xalloc(i,"move");
	for(j=0;j<i;j++)
		b->val[j]=a->val[j];
	return;
}

short *xalloc(nint,s)
char *s;
{	short *i;
	char *malloc();

	i=(short *)malloc((unsigned)((nint+2)*sizeof(short)));
#ifdef DBG
	fprintf(stderr,"%s: %d words at %o\n",s,nint,i);
#endif
	if (i!=NULL) return(i);
	fprintf(stderr,"can\'t allocate for %s: %d words\n",s,nint);
	aperror("ape: no free space");
	return(0);
}
aperror(s) char *s;
{
	fprintf(stderr,"%s\n",s);
	ignore(fflush(stdout));
	sleep(2);
	abort();
}

new(pa)
PMINT *pa;
/* Pascal-like initializer; gives a zero structure */
{
	char *calloc();

	*pa =  (PMINT) calloc (1,sizeof(MINT));
}

mcan(a)
MINT *a; /* "removes" excess zeroes */
{	int i,j;

	if((i=a->len)==0) return;
	else if(i<0) i= -i;
	for(j=i;j>0 && a->val[j-1]==0;j--);
	if(j==i) return;
	if(j==0)
	{	xfree(a);
		return;
	}
	if(a->len > 0) a->len=j;
	else a->len = -j;
}

MINT *shtom(n)
short n;
{
	MINT *a;

	new(&a);
	if(n>0)
	{	a->len=1;
		a->val=xalloc(1,"itom1");
		*a->val=n;
		return(a);
	}
	else if(n<0)
	{	a->len = -1;
		a->val=xalloc(1,"itom2");
		*a->val= -n;
		return(a);
	}
	else	/* I think the new makes this unnecessary */
	{	/*a->len=0;*/
		return(a);
	}
}

MINT *ltom(ln)
long ln;
{
	if (ln < SHORT && (-ln) <SHORT)
		{
		return(itom((int)ln));
		}
	else
	{
	PMINT a;

	new(&a);
	if (ln < 0L)
		{
		a->len = -2;
		ln = -ln;
		}
	else a->len = 2;
	if (ln > 0L)
		{
		a->val = xalloc(2,"ltom1");
		a->val[1] = ln/SHORT;
		a->val[0] = ln%SHORT;
		return (a);
		}
	aperror("ltom problem");
	return (NULL);
	}
}

makemint(a,ln)
PMINT a;
long int ln;
{
	PMINT here;

	here = ltom(ln);
	move (here, a);
	afree(here);
}
