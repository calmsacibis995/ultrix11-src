
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/* SCCSID: @(#)besy0_.c	3.0	4/22/86	*/
/*
 * Based on:
 *	"@(#)  (2.9BSD)  besy0_.c	1.1"
 */

double y0();

float besy0_(x)
float *x;
{
	return((float)y0((double)*x));
}
