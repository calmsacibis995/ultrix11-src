
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)line.c	3.0	4/22/86
 */
#include <stdio.h>
extern ggi;
extern xnow,ynow;
line(x0,y0,x1,y1){
	int nx0,ny0;
	nx0 = xsc(x0);
	ny0 = ysc(y0);
	xnow = xsc(x1);
	ynow = ysc(y1);
	fprintf(ggi, "P[%d,%d] V[%d,%d]\n", nx0, ny0, xnow, ynow);
}

cont(x0,y0){
	xnow = xsc(x0);
	ynow = ysc(y0);
	fprintf(ggi, "V[%d,%d]\n", xnow, ynow);
}

extern multi;
ofcont(dir, x)
{						/*  3  2  1 */
	int cnt;				/*   \ | /  */
						/*  4-----0 */
	if(x != 0)				/*   / | \  */
	{					/*  5  6  7 */
		switch(dir)
		{
			case '\0':
				xnow = xsc(xnow+(x*multi));
				break;
			case '\1':
				xnow = xsc(xnow+(x*multi));
				ynow = xsc(ynow-(x*multi));
				break;
			case '\2':
				ynow = xsc(ynow-(x*multi));
				break;
			case '\3':
				xnow = xsc(xnow-(x*multi));
				ynow = xsc(ynow-(x*multi));
				break;
			case '\4':
				xnow = xsc(xnow-(x*multi));
				break;
			case '\5':
				xnow = xsc(xnow-(x*multi));
				ynow = xsc(ynow+(x*multi));
				break;
			case '\6':
				ynow = xsc(ynow+(x*multi));
				break;
			case '\7':
				xnow = xsc(xnow+(x*multi));
				ynow = xsc(ynow+(x*multi));
			default:
				return;
		}
		for(cnt = 0; cnt < x; cnt++)
			fprintf(ggi,"v%d", dir);
		fprintf(ggi,"\n");
	}
}
