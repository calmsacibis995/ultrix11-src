
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)r_dim.c	3.0	4/22/86
 */
double r_dim(a,b)
float *a, *b;
{
return( *a > *b ? *a - *b : 0);
}
