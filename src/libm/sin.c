
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*	SCCSID: @(#)sin.c	3.0	4/22/86	*/
/*	(System 5)  sin.c	1.13	*/
/*LINTLIBRARY*/
/*
 *	sin and cos of double-precision argument.
 *	Returns ERANGE error and value 0 if argument too large.
 *	Algorithm and coefficients from Cody and Waite (1980).
 */

#include <math.h>
#include <values.h>
#include <errno.h>

double
sin(x)
double x;
{
	extern double sin_cos();

	return (sin_cos(x, 0));
}

static double
sin_cos(x, cosflag)
register double x;
int cosflag;
{
	register double y;
	register int neg = 0;
	struct exception exc;
	
	exc.arg1 = x;
	if (x < 0) {
		x = -x;
		neg++;
	}
	y = x;
	if (cosflag) {
		neg = 0;
		y += M_PI_2;
		exc.name = "cos";
	} else
		exc.name = "sin";
	if (y > X_TLOSS) {
		exc.type = TLOSS;
		exc.retval = 0.0;
		if (!matherr(&exc)) {
			(void) write(2, exc.name, 3);
			(void) write(2, ": TLOSS error\n", 14);
			errno = ERANGE;
		}
		return (exc.retval);
	}
	y = y * M_1_PI + 0.5;
	if (x <= MAXLONG) { /* reduce using integer arithmetic if possible */
		register long n = (long)y;

		y = (double)n;
		if (cosflag)
			y -= 0.5;
		_REDUCE(long, x, y, 3.1416015625, -8.908910206761537356617e-6);
		neg ^= (int)n % 2;
	} else {
		extern double modf();
		double dn;

		x = (modf(y, &dn) - 0.5) * M_PI;
		if (modf(0.5 * dn, &dn))
			neg ^= 1;
	}
	if (x < 0) {
		x = -x;
		neg ^= 1;
	}
	if (x > X_EPS) { /* skip for efficiency and to prevent underflow */
		static double p[] = {
			 0.27204790957888846175e-14,
			-0.76429178068910467734e-12,
			 0.16058936490371589114e-9,
			-0.25052106798274584544e-7,
			 0.27557319210152756119e-5,
			-0.19841269841201840457e-3,
			 0.83333333333331650314e-2,
			-0.16666666666666665052e0,
		};

		y = x * x;
		x += x * y * _POLY7(y, p);
		if (x > 1.0) /* inhibit roundoff out of range */
			x = 1.0;
	}
	exc.retval = neg ? -x : x;
	if (exc.arg1 > X_PLOSS || exc.arg1 < -X_PLOSS) {
		exc.type = PLOSS;
		if (!matherr(&exc))
			errno = ERANGE;
	}
	return (exc.retval);
}

double
cos(x)
register double x;
{
	return (x ? sin_cos(x, 1) : 1.0);
}
