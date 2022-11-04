
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)d_imag.c	3.0	4/22/86
 */
#include "complex"

double d_imag(z)
dcomplex *z;
{
return(z->dimag);
}
