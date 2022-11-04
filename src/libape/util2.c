
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*	SCCSID: @(#)util2.c	3.0	4/22/86	*/
/*	(2.9BSD)  util2.c	2.1	8/13/82 */

#include "lint.h"
#include <stdio.h>
#include <ape.h>

PMINT stom(str)
char *str;	/* This allows initialization of a MINT by
		 * a number represented by a string of arbitrary
		 * length.  The usual C conventions are used for
		 * determining the input base. */
{
	PMINT local;

	new(&local);
	if (*str == '0')
		{
		++str;
		if (*str == 'x' || *str == 'X')
			{
			++str;
			sm_in (local, 16, str);
			}
		else
			sm_in (local, 8, str);
		}
	else
		sm_in (local, 10, str);
	return (local);
}

mcmp(a,b)
MINT *a,*b;
{	MINT c;
	int res;

	if(a->len!=b->len) return(a->len-b->len);
	c.len=0;
	msub(a,b,&c);
	res=c.len;
	xfree(&c);
	return(res);
}
