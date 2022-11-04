
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)d_tan.c	3.0	4/22/86
 */
double d_tan(x)
double *x;
{
double tan();
return( tan(*x) );
}
