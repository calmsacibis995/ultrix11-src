
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)c_sin.c	3.0	4/22/86
 */
#include "complex"

c_sin(r, z)
complex *r, *z;
{
double sin(), cos(), sinh(), cosh();

r->real = sin(z->real) * cosh(z->imag);
r->imag = cos(z->real) * sinh(z->imag);
}
