
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/* SCCSID: @(#)hypot.c	3.0	4/22/86 */
/*
 * sqrt(a^2 + b^2)
 *	(but carefully)
 */

double sqrt();
double
hypot(a,b)
double a,b;
{
	double t;
	if(a<0) a = -a;
	if(b<0) b = -b;
	if(a > b) {
		t = a;
		a = b;
		b = t;
	}
	if(b==0) return(0.);
	a /= b;
	/*
	 * pathological overflow possible
	 * in the next line.
	 */
	return(b*sqrt(1. + a*a));
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
