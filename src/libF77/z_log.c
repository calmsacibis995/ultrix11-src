
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)z_log.c	3.0	4/22/86
 */
#include "complex"

z_log(r, z)
dcomplex *r, *z;
{
double log(), cabs(), atan2();

r->dimag = atan2(z->dimag, z->dreal);
r->dreal = log( cabs( z->dreal, z->dimag ) );
}
