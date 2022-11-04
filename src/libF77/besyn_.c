
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/* SCCSID: @(#)besyn_.c	3.0	4/22/86	*/
/*
 * Based on:
 *	"@(#)  (2.9BSD)  besyn_.c	1.1"
 */

double yn();

float besyn_(n, x)
long *n; float *x;
{
	return((float)yn((int)*n, (double)*x));
}
