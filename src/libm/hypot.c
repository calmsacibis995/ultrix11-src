
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*	SCCSID: @(#)hypot.c	3.0	4/22/86	*/
/*	(System 5)  hypot.c	1.10	*/
/*LINTLIBRARY*/
/*
 *	hypot(a, b) returns sqrt(a^2 + b^2), avoiding unnecessary overflows.
 *	Returns ERANGE error and value HUGE if the correct value would overflow.
 */

#include <math.h>
#include <values.h>
#include <errno.h>
#define ITERATIONS	4

double
hypot(a, b)
register double a, b;
{
	register double t;
	register int i = ITERATIONS;
	struct exception exc;

	if ((exc.arg1 = a) < 0)
		a = -a;
	if ((exc.arg2 = b) < 0)
		b = -b;
	if (a > b) {				/* make sure |a| <= |b| */
		t = a;
		a = b;
		b = t;
	}
	if ((t = a/b) < X_EPS)			/* t <= 1 */
		return (b);			/* t << 1 */
	a = 1 + t * t;				/* a = 1 + (a/b)^2 */
	t = 0.5 + 0.5 * a;			/* first guess for sqrt */
	do {
		t = 0.5 * (t + a/t);
	} while (--i > 0);			/* t <= sqrt(2) */
	if (b < MAXDOUBLE/M_SQRT2)		/* result can't overflow */
		return (t * b);
	if ((t *= 0.5 * b) < MAXDOUBLE/2)	/* result won't overflow */
		return (t + t);
	exc.type = OVERFLOW;
	exc.name = "hypot";
	exc.retval = HUGE;
	if (!matherr(&exc))
		errno = ERANGE;
	return (exc.retval);
}

struct	complex
{
	double	r;
	double	i;
};

double
cabs(arg)
struct complex arg;
{
	double hypot();

	return(hypot(arg.r, arg.i));
}
