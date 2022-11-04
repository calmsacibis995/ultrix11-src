
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/* SCCSID: @(#)besjn_.c	3.0	4/22/86	*/
/*
 * Based on:
 *	"@(#)  (2.9BSD  besjn_.c	1.1")
 */

double jn();

float besjn_(n, x)
long *n; float *x;
{
	return((float)jn((int)*n, (double)*x));
}
