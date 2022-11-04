/* SCCSID: @(#)cont.c	3.0	4/22/86 */

/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/


/*
 Facility:	Part of the plot package for the LA50 Printer.

 Called by:	driver(),line(),point()

 Calls: 	setbit()

 Abstract:	cont(x1,y1) - draws a line from the current point to (x1,y1)

	This function uses Bresenham's alogorithm to set the bits in
	bitmap data struture for the LA50. (for details of this, see
	'Fundamentals of Interactive Computer Graphics'
	by Foley and Van Dam).

 Author:	Kevin J. Dunlap

 Creation:	March 1984.

 Modified by:

 Date:		By:		Reason:

*/

#include <stdio.h>
#define DEBUG 0
extern xnow,ynow;
/* an implementation of Bresenham's Algorithm for the Pro/350 */
cont(x1,y1)
{
	int	dx,dy,incr1,incr2,d,x,y,xend,yend ;
	int	sdx, sdy,x2,y2,new_xnow,new_ynow;
	x2 = xsc(x1);
	y2 = ysc(y1);
	new_xnow = x2;
	new_ynow = y2;
#if DEBUG
fprintf(stderr,"cont: xnow,ynow - (%d,%d) to x2,y2 - (%d,%d)\n",xnow,ynow,x2,y2);
	fflush(stderr);
#endif
	sdx = x2 - xnow ;
	dx = abs(sdx) ;
	dy = abs(y2-ynow) ;
	d = (2*dy) - dx ;
	incr1 = 2 * dy ;
	incr2 = 2 * (dy - dx) ;
	if (dx == 0 || dy == 0)
		if (dx == 0)
			{
			if (ynow > y2) 
				for(; y2 <=ynow; ++y2){ setbit(xnow,y2);
				}
			else
				for(; ynow <= y2; ++ynow) setbit(xnow,ynow);
			}
		else
			{
			if (xnow < x2)
				for(; xnow <= x2; ++xnow) setbit(xnow,ynow);
			else
				for(;x2 <= xnow; ++x2) setbit(x2,ynow);
			}
       else if ( dx > dy )	     /* More Horizontal */
		{
#if DEBUG
fprintf(stderr,"Hor: dx=%d,dy=%d,d=%d,incr1=%d,incr2=%d\n",dx,dy,d,incr1,incr2);
fflush(stderr);
#endif
		if( xnow > x2 )
			{
			x = x2 ;
			y = y2 ;
			sdy = ynow - y2 ;
			xend = xnow ;
			}
		else
			{
			x = xnow ;
			y = ynow ;
			sdy = y2 - ynow ;
			xend = x2 ;
			}

		setbit(x,y) ;

		while ( x < xend )
			{
			x++ ;
			if ( d < 0 )
				d += incr1 ;
			else
				{
				if(sdy < 0)
					y-- ;
				else
					y++ ;
				d += incr2 ;
				}
		  /*	printf("\nd = %d",d) ;		*/
			setbit(x,y) ;
			}
		}
	else			/* More Vertical */
		{
		d = (2*dx) - dy ;
		incr1 = 2 * dx ;
		incr2 = 2 * (dx - dy) ;
#if DEBUG
fprintf(stderr,"Vert: dx=%d,dy=%d,d=%d,incr1=%d,incr2=%d\n",dx,dy,d,incr1,incr2);
fflush(stderr);
#endif
		if( ynow > y2 )
			{
			x = x2 ;
			y = y2 ;
			yend = ynow ;
			sdx = -sdx ;
			}
		else
			{
			x = xnow ;
			y = ynow ;
			yend = y2 ;
			}

		setbit(x,y) ;
		while ( y < yend )
			{
			y++ ;
			if ( d < 0 )
				d += incr1 ;
			else
				{
				if(sdx < 0)
					x-- ;
				else
					x++ ;
				d += incr2 ;
				}
		/*	printf("\nd = %d",d) ;	/*DEBUG*/
			setbit(x,y) ;
			}
		}
	xnow = new_xnow;   /* update coordinates of current point */
	ynow = new_ynow;

}	/* End bresenham() */

