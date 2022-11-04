
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)c_exp.c	3.0	4/22/86
 */
#include "complex"

c_exp(r, z)
complex *r, *z;
{
double expx;
double exp(), cos(), sin();

expx = exp(z->real);
r->real = expx * cos(z->imag);
r->imag = expx * sin(z->imag);
}
