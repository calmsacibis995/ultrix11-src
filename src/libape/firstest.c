
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*	SCCSID: @(#)firstest.c	3.0	4/22/86	*/

#include <ape.h>

main()
{
	PMINT a,b;
	long int i;

	new(&a);
	new(&b);
	for (i=100000; i < 2000000; i += 100000) {
		makemint(a,i);
		mout(a); putchar('\n');

		/***
		square(a,b);
		mout(b); putchar('\n');
	 	***/

		mult(a,a,b);
		mout(b); putchar('\n');
		putchar('\n');
		}
}
