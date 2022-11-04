
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*	SCCSID: @(#)pow.c	3.0	4/22/86	*/
/*	(2.9BSD)  pow.c	2.3	7/24/82 */

#include "lint.h"
#include <ape.h>

pow(a,b,c,d) /* d = a^b mod c; c needed to prevent overflow */
MINT *a,*b,*c,*d;
{	int j,n;
	MINT junk, *z;
	short port; /* Portable short length indicator */

	if (b->len < 0)
		{
		xfree(d);
		return;
		}
	else if (b->len == 0)
		{
		xfree(d);
		d->len = 1;
		d->val = xalloc(1,"pow");
		*d->val = 1;
		return;
		}
	junk.len=0;
	z = itom(1);
	for(j=0;j<b->len;j++)	/* Go backwards through each bit
				 * of each word */
	{	
		port = ~0;
		n=b->val[b->len-j-1];
		while ((port <<= 1))
		{	mult(z,z,z); /* Square what we have so far */
			mdiv(z,c,&junk,z);
			if ((n <<= 1) & CARRYBIT) /* Put in another a */
			{	mult(a,z,z);
				mdiv(z,c,&junk,z);
			}
		}
	}
	xfree(&junk);
	xfree(d);
	d->len = z->len;
	d->val = z->val;
	return;
}


rpow(a,n,b)
MINT *a,*b; /* b = a^n */
int n;
{
	MINT *z;
	int port = ~0; /* portable wordlength indicator */

	if (n < 0)
		{
		xfree(b);
		return;
		}
	else if (n == 0)
		{
		xfree(b);
		b->len = 1;
		b->val = xalloc(1,"rpow");
		*b->val = 1;
		return;
		}
	z = itom(1);
	while ((port <<= 1))
		{
		mult(z,z,z); /* Square what we have so far */
		if ((n <<= 1) < 0) /* Put in another a */
			mult(a,z,z);
		}
	xfree(b);
	b->len = z->len;
	b->val = z->val;
	return;
}

lpow(m,n,b)
MINT *b; /* b = m^n */
int m;		/* Specialized hacky version */
long int n;
{
	PMINT mintm;
	long int port = ~0; /* portable longword length indicator */

	xfree(b);
	if (n < 0L) return;
	b->len = 1;
	b->val = xalloc(1,"lpow");
	*b->val = 1;
	if (n == 0L) return;
	mintm = itom(m);
	while ((port <<= 1))
		{
		mult(b,b,b); /* Square what we have so far */
		if ((n <<= 1) < 0) /* Put in another m */
			mult(mintm,b,b);
		}
	afree(mintm); /* tidy up */
	return;
}
