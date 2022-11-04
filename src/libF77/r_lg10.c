
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)r_lg10.c	3.0	4/22/86
 */
#define log10e 0.43429448190325182765

double r_lg10(x)
float *x;
{
double log();

return( log10e * log(*x) );
}
