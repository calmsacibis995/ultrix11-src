
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/* SCCSID: @(#)besj0_.c	3.0	4/22/86	*/
/*
 * Based on:
 *	"@(#)  (2.9BSD)  besj0_.c	1.1"
 */

double j0();

float besj0_(x)
float *x;
{
	return((float)j0((double)*x));
}
