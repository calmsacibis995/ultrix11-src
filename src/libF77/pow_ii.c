
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)pow_ii.c	3.0	4/22/86
 */
long int pow_ii(ap, bp)
long int *ap, *bp;
{
long int pow, x, n;
if(n = *bp) { if (n<0) return(0);
	else {
	pow=1;
	x = *ap;
	for( ; ; )
		{
		if(n & 01)
			pow *= x;
		if(n >>= 1)
			x *= x;
		else
			return(pow);
		}
	}
     }
else return(1);
}
