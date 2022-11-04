
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*	SCCSID: @(#)shift.c	3.0	4/22/86	*/
/*	(2.9BSD)		*/

#include "lint.h"
#include <stdio.h>
#include <ape.h>

#define odd(n)		((n)&01)
#define even(n)		!odd(n)

lshift(a,amount)	/* left shift a by amount words --
			 * i.e., multiply by 2^(15*amount) */
PMINT a;
int amount;
{
	short *temp;
	int i, negative, alen;
	char *realloc();

	negative = ( a->len < 0 );
	if (negative) a->len = -a->len;
	alen = a->len+amount;
	fmout(a,stderr);
	fputc('\n',stderr);
	temp = (short *) realloc ((char *)a->val, (unsigned) alen * sizeof(short) );
	fmout(a,stderr);
	fputc('\n',stderr);
	for (i = alen - 1; i >= amount; --i)
		temp[i] = temp[i-amount];
	for (i=0; i < amount; ++ i)
		temp[i] = 0;
	xfree(a);
	a->len = negative ? -alen : alen;
	a->val = temp;
}

mshift(a,answer,amount) /* same as lshift, except puts answer in answer */
PMINT a, answer;
int amount;
{
	short *temp;
	int i, negative, alen;

	negative = ( a->len < 0 );
	if (negative) 
		alen = amount - a->len;
	else
		alen = a->len+amount;
	temp =  xalloc (alen, "mshift");
	for (i = alen - 1; i >= amount; --i)
		temp[i] = a->val[i-amount];
	for (i=0; i < amount; ++ i)
		temp[i] = 0;
	xfree(answer);
	answer->len = negative ? -alen : alen;
	answer->val = temp;
}
rshift(a,amount)	/* right shift a by amount words --
			 * i.e., divide by 2^(15*amount) */
PMINT a;
int amount;
{
	char *realloc();
	int i, negative, alen;

	negative = ( a->len < 0 );
	if (negative) a->len = -a->len;
	alen = a->len-amount;
	if (alen <= 0) {
		xfree(a);
		return;
		}
	for (i=amount; i < a->len; ++i)
		a->val[i-amount] = a->val[i];
	a->val = (short *) realloc ((char *)a->val, (unsigned) alen * sizeof(short));
	a->len = negative ? -alen : alen;
}

powerof2(a,n)		/* a = a*2^n */
PMINT a;
int n;
{
	int wordshift, negative, bitshift, right,  i, oldcarry, newcarry;
	short mask;

	negative = a->len < 0;
	if (negative) a->len = -a->len;
	right = ( n < 0 );
	if (right) n = -n;
	wordshift = n/WORDLENGTH;
	bitshift = n%WORDLENGTH;
	if (right) {
		rshift(a, wordshift);
		mask = ~(~0 << bitshift);
		oldcarry = 0;
		for (i=a->len-1; i >= 0 ; --i) {
			newcarry = (a->val[i] & mask) << (WORDLENGTH-bitshift);
			a->val[i] = oldcarry + (a->val[i]>>bitshift);
			oldcarry = newcarry;
			}
		if (a->val[a->len-1] == 0)
			--a->len;
		}
	else {
		lshift(a, wordshift);
		mask = (~0 << WORDLENGTH-bitshift);
		oldcarry = 0;
		for (i=0; i < a->len; ++i) {
			newcarry = (a->val[i] & mask) >> (WORDLENGTH-bitshift);
			a->val[i] = oldcarry + (a->val[i]<<bitshift);
			oldcarry = newcarry;
			}
		if (oldcarry) {	/* rely on extra space given by xalloc */
			a->len += 1;
			a->val[a->len-1] = oldcarry;
			}
		}
	if (negative) a->len = -a->len;
	return;
}

oddpart(a)	/* replace a by its odd part, returning the no. of
		 * 2's in it */
PMINT a;
{
	int zerowords, zerobits;
	short nonzero;

	mcan(a);
	if (a->len == 0) return(0);
	for (zerowords=0; a->val[zerowords] == 0; ++zerowords)
		;
	nonzero = a->val[zerowords];
	zerobits = WORDLENGTH*zerowords;
	while (even(nonzero)) {
		++zerobits;
		nonzero <<= 1;
		}
	powerof2(a,zerobits);
	return(zerobits);
}
