
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)r_cosh.c	3.0	4/22/86
 */
double r_cosh(x)
float *x;
{
double cosh();
return( cosh(*x) );
}
