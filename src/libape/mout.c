
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*	SCCSID: @(#)mout.c	3.0	4/22/86	*/
/* 	(2.9BSD)  mout.c	2.2	8/13/82 */

#include "lint.h"
#include <ctype.h>
#include <stdio.h>
#include <ape.h>

m_out(a,b,f)  /* output a base b onto file f */
MINT *a; FILE *f;
{	int sign,xlen,i,outlen;
	short r;
	MINT x;
	char *obuf, *malloc(), *sprintf(), tempeh[7];
	register char *bp;

	sign=1;
	xlen=a->len;
	if(xlen<0)
	{	xlen= -xlen;
		sign= -1;
	}
	if (xlen==0)
	{	fprintf(f,"0");
		return;
	}
	x.len=0;
	move(a,&x);
	x.len=xlen; /* now x is the absolute value of a */
	outlen=outlength(a,b);
	obuf =  malloc((unsigned)outlen);
		/* Advance bp to the end of the buffer
		 * and start (end) with a null: */
	bp=obuf+outlen-1;
	*bp--='\0';
		/* Now generate the digits in reverse
		 * order by taking remainders from
		 * successive divisions by the base b: */
	while(x.len>0)
	{
		sdiv(&x,b,&x,&r);
			/* For bases greater than 16, the number
			 * is printed in "hybrid" notation as
			 * "clumps" of (base 10) numbers.
			 * For bases between 10 and 16, the
			 * letters abcdef are used for 10 through 15
			 */
		if (b>16) *bp-- = ' ';
		if (r<10) *bp--=r+'0';
		else if (b<17) *bp-- = r+'a'-10;
		else {
			ignors(sprintf(tempeh,"%d",r));
			for (i=strlen(tempeh); i >0; --i)
					*bp-- = tempeh[i-1];
		}
	}
	if (sign == -1) *bp-- = '-';
	fprintf(f,"%s",bp+1);
	free(obuf);
	xfree(&x);
	return;
}

sm_out(a,b,s)
PMINT a;
int b;
char *s;
{
	int sign,xlen,i,outlen;
	short r;
	MINT x;
	char *sprintf(), tempeh[7];
	register char *bp;

	sign=1;
	xlen=a->len;
	if(xlen<0)
	{	xlen= -xlen;
		sign= -1;
	}
	if (xlen==0)
		{
		*s='0';
		*(s+1)='\0';
		return;
		}
	x.len=0;
	move(a,&x);
	x.len=xlen; /* now x is the absolute value of a */
	outlen = outlength(a,b);
		/* Advance bp to the end of the string required
		 * and start (end) with a null: */
	bp = s+outlen-1;
	*bp--='\0';
		/* Now generate the digits in reverse
		 * order by taking remainders from
		 * successive divisions by the base b: */
	while(x.len>0)
	{
		sdiv(&x,b,&x,&r);
			/* For bases greater than 16, the number
			 * is printed in "hybrid" notation as
			 * "clumps" of (base 10) numbers.
			 * For bases between 10 and 16, the
			 * letters abcdef are used for 10 through 15
			 */
		if (b>16) *bp-- = ' ';
		if (r<10) *bp--=r+'0';
		else if (b<17) *bp-- = r+'a'-10;
		else {
			ignors(sprintf(tempeh,"%d",r));
			for (i=strlen(tempeh); i >0; --i)
					*bp-- = tempeh[i-1];
		}
	}
	if (sign == -1) *bp-- = '-';
	if ((++bp) != s) /* do we have to adjust the string? */
		{
		if (bp < s) fprintf(stderr,"outlength estimate %d off by %d",
				outlen,s-bp); /* OOPS! */
		else while (*bp != '\0')
			*s++ = *bp++;	/* move the string into the right
					 * place */
		}
	xfree(&x);
	return;
}

outlength(a,b)
PMINT a;	/* determine an approximation for the number of characters
		 * required to represent a base b. */
int b;
{
	int wordlen, alen;

		/* Determine the length of a single short int:
		 * Magic numbers are [log base b TOPSHORT] + 1 */
	switch(b) {
		case 2: wordlen=15; break;
		case 3: wordlen=10; break;
		case 4: wordlen=8; break;
		case 5: wordlen=7; break;
		case 6: case 7: wordlen=6;
		default:	if (b<13) wordlen=5;
				else if (b<17) wordlen=4;
				else wordlen=12;  /* ?? */
				break;
		}
	alen = (a->len > 0 ? a->len : -a->len);
	return(wordlen*alen + 1);
}

om_out(a,f)	/* Output a base 8 onto file f; this is about
		 * thirty times faster than m_out(a,8,f) since
		 * sdiv needn't be invoked (numbers are stored
		 * base 2^15= 8^5, so conversion can be done
		 * word by word.)
		 */
MINT *a;
FILE *f;
{
	int alen;

	if (a->len <0)
		{
		putc('-',f);
		alen = -a->len;
		}
	else alen = a->len;
	if (alen == 0)
		{
		putc('0',f);
		return;
		}
	--alen;
	fprintf(f,"%o",a->val[alen--]);
	while (alen >= 0)
		fprintf(f,"%05o",a->val[alen--]);
}

	/* The following are some useful "special cases" for
		the I/O routines: */

minput(a) MINT *a;
{
	return(m_in(a,10,stdin));
}
omin(a) MINT *a;
{
	return(m_in(a,8,stdin));
}
mout(a) MINT *a;
{
	m_out(a,10,stdout);
}
omout(a) MINT *a;
{
	om_out(a,stdout);
}
fmout(a,f) MINT *a; FILE *f;
{	m_out(a,10,f);
}
fmin(a,f) MINT *a; FILE *f;
{
	return(m_in(a,10,f));
}
