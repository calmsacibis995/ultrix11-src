
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)circle.c	3.0	4/22/86
 */
#include <stdio.h>
double sqrt();
extern ggi;
extern float scalex;
extern float scaley;

circle(xi,yi,radius)
{
	int i,j,x,y,xs,ys,rs;

	if (scalex == scaley) {
		xs = xsc(xi);
		ys = ysc(yi);
		rs = xsc(radius);
		fprintf(ggi, "p[%d,%d]c[+%d]\n", xs, ys, rs);
	} else {
/* The reason not using this one to draw a circle is it takes twice the time
 * to do it than the one we are using right now.  Hope someday somebody will
 * have a better method.
 *
	int x1,y1,x2,y2,f;
	float r, rr, ri;
	double p;
		x1 = x2 = xi - radius;
		y1 = y2 = yi;
		rr = radius;
		j = -radius;
		for(i=j;i<=radius;i++) {
			ri = i;
			r = rr*rr-ri*ri;
			p = sqrt(r);
			f = p + 0.5;
			x = xi + i;
			y = yi + f;
			line(x1,y1,x,y);
			x1 = x;
			y1 = y;
			y = yi - f;
			line(x2,y2,x,y);
			x2 = x;
			y2 = y;
		}
*/
		x = xi - radius;
		y = yi;
		move(x,y);
		j = -radius;
		for(i=j;i<=radius;i++) {
			x = xi + i;
			y = yi + gsqrt(i,radius);
			cont(x,y);
		}
		for(i=radius;i>=j;i--) {
			x = xi + i;
			y = yi - gsqrt(i,radius);
			cont(x,y);
		}
	}
}
gsqrt(i,radius)
{
	int f;
	float r,ri,rr;
	double p;

	ri = i;
	rr = radius;
	r = rr*rr-ri*ri;
	p = sqrt(r);
	f = p + 0.5;
	return(f);
}
