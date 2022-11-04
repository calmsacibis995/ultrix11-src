/* SCCSID: @(#)circle.c	3.0	4/22/86 */

/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * Facility: Part of the Plot package
 *
 * Called by: driver()
 *
 * Calls: setbit()
 *
 * Abstract:	circle(xi,yi,radius)
 *
 * 	xi is the x coordinate of the centre of the circle
 *
 *	yi is the y coordinate of the centre of the circle
 *
 *	radius is the radius of the circle.
 *
 *	The points on the circle are generated using J Michener's
 *	algorithm (See 'Fundamentals of Interactive Computer Graphics'
 *	by Foley & Van Dam, p445). The essential thing to understand
 *	is that only the points for one octant of the circle are calculated,
 *	the rest are derived by applying 'reflections'.
 *
 * Author: David Roberts.
 *
 * Creation: April 1984
 *
 * Modified by:
 * Date:		By:	     Reason:
 *
 * 24-JUN-84  Kevin J. Dunlap   Changed WRITE_PIXEL to setbit for LA50 and
 *				modifyed circle_points to work with the orgin
 *				in the bottom left hand corner.
 * 24-AUG-84  Kevin J. Dunlap   Add DEBUG messages.
 *
 */

#include <stdio.h>
extern float scalex,scaley,botx,boty,obotx,oboty;
float factor;
float sc_xobot, sc_yobot;

circle(xi,yi,radius)
int	radius,
	xi,yi;
{
	int	x,y,ox,oy,d,xradius;
#ifdef	DEBUG
	int value = 4;
#endif

	factor = scaley / scalex;
#ifdef DEBUG
	fprintf(stderr,"Circle called with %d,%d,%d\n",xi,yi,radius);
	fprintf(stderr,"scalex is %f , scaley is %f \n",scalex,scaley);
	fprintf(stderr,"factor is %f\n",factor);
#endif
	x = 0 ;
	xradius = radius * scalex;	/* the radius on the x space */
	ox = xi * scalex;		/* the x coord of the centre in x */
	oy = yi * scaley;		/* the y coord of the centre in y */
	sc_xobot = obotx * scalex;	/* the origin of the window in x */
	sc_yobot = oboty * scaley;
#ifdef DEBUG
      fprintf(stderr,"sc_xobot is %f, sc_yobot is %f \n",sc_xobot,sc_yobot);
#endif
	y = xradius ;
	d = 3 - 2 * xradius ;
#ifdef DEBUG
	fprintf(stderr,"xradius = %d\n",xradius);
	fprintf(stderr,"ox = %d, oy = %d , y = xradius\n",ox,oy); */
#endif

	while( x < y )
		{
		circle_points(x,y,ox,oy) ;
		if ( d < 0 )
			d += 4 * x + 6 ;
		else
			{
			d += 4 * ( x - y ) + 10 ;
			y-- ;
			}
		x++ ;
		} /* end while */

	if( x == y )
		circle_points(x,y,ox,oy) ;
}

circle_points(x,y,ox,oy)
register int	x,y,	      /* x and y coordinates of the point */
		ox,oy;		/* x and y coordinates of the centre */



{
	extern float factor;
	int setbit(),xcoord(),ycoord(),px,py,mx,my,xtemp,ytemp;
#ifdef DEBUG
	fprintf(stderr,"circle_points(%d,%d,%d)\n",x,y,value);
#endif
#define X_coord(A)     (A + xtemp)
#define Y_coord(A)     ((A * factor) + ytemp)
	xtemp = ox + botx;
	ytemp = boty +sc_yobot + oy;   /* changed for LA50 from   */
				       /*  ytemp = boty +sc_yobot + oy; */
	px = X_coord(x);
	py = Y_coord(y);
	mx = X_coord(-x);
	my = Y_coord(-y);
	setbit(px,py);/* write_pixel(x,y,value) */
	setbit(px,my);/* write_pixel(x,-y,value) */
	setbit(mx,my);/* write_pixel(-x,-y,value) */
	setbit(mx,py);/* write_pixel(-x,y,value) */
	px = Y_coord(x);
	py = X_coord(y);
	mx = Y_coord(-x);
	my = X_coord(-y);
	setbit(py,px);/* write_pixel(y,x,value) */
	setbit(py,mx);/* write_pixel(y,-x,value) */
	setbit(my,mx);/* write_pixel(-y,-x,value) */
	setbit(my,px);/* write_pixel(-y,x,value) */

}

