
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)d_abs.c	3.0	4/22/86
 */
double d_abs(x)
double *x;
{
if(*x >= 0)
	return(*x);
return(- *x);
}
