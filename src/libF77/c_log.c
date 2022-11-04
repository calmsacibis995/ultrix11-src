
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)c_log.c	3.0	4/22/86
 */
#include "complex"

c_log(r, z)
complex *r, *z;
{
double log(), cabs(), atan2();

r->imag = atan2(z->imag, z->real);
r->real = log( cabs(z->real, z->imag) );
}
