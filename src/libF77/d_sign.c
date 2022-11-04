
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)d_sign.c	3.0	4/22/86
 */
double d_sign(a,b)
double *a, *b;
{
double x;
	x = *a;
	if (x <= 0)
		x = -x;
	if (*b < 0)
		x = -x;
	return (x);
/*
 *  x = (*a >= 0 ? *a : - *a);
 * return( *b >= 0 ? x : -x);
 */
}
