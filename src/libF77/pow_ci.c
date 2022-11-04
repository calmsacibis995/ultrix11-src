
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)pow_ci.c	3.0	4/22/86
 */
#include "complex"

pow_ci(p, a, b) 	/* p = a**b  */
complex *p, *a;
long int *b;
{
dcomplex p1, a1;

a1.dreal = a->real;
a1.dimag = a->imag;

pow_zi(&p1, &a1, b);

p->real = p1.dreal;
p->imag = p1.dimag;
}
