
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)ofmove.c	3.0	4/22/86
 */
#include <stdio.h>
extern ggi;
extern xnow,ynow;
extern multi;

ofmove(dir, x)	/* move cursor in offset direction x number of pixels */
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
			fprintf(ggi,"p%d", dir);
		fprintf(ggi,"\n");
	}
}
