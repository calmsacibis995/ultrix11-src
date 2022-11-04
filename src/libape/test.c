
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*	SCCSID: @(#)test.c	3.0	4/22/86	*/

#include <ape.h>

main()
{
	PMINT a,b;
	int i;

	a = itom(2);
	new(&b);
	for (i=1; i < 11; ++i) {
		square(a,b);
		mout(a); putchar('\t'); mout(b); putchar('\n');
		move(b,a); xfree(b);
		}
}
