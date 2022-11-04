/* SCCSID: @(#)arc.c	3.0	4/22/86 */

/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
	The circle algorithm used here is that due to J. Michener

	xi is the x coordinate of the centre.
	yi is the y coordinate of the centre.
	radius is radius
*/

#include <stdio.h>
#define DEBUG 0
#define DEBUG2 0
#define X 0
#define Y 1
extern float scalex,scaley,botx,boty,obotx,oboty;
float factor;
float sc_xobot, sc_yobot;
float start[2] [9], aend[2] [9];
int drw_oct[9];
arc(xi,yi,x0,y0,x1,y1)
int	xi,yi,x0,y0,x1,y1;
{
	int	x,y,ox,oy,oyx,d,xradius,arc_points(),octant();
	int	st_x,st_y,en_x,en_y,st_oct,en_oct;
	/* The arrays start and end points for the arc of each octant of the
	   possible circle. The arrays are dimensioned as 2 x 9, when
	   2 x 8 would suffice. This is so that the column number corresponds
	   to the octant number  */
	int value;
	float dx,dy;
	double radius,limit,sqrt();
#if DEBUG
	fprintf(stderr,"Arc called with centre xi,yi = %d,%d\n",xi,yi);
	fprintf(stderr,"Arc starts at x0,y0 = %d,%d\n",x0,y0);
	fprintf(stderr,"Arc ends at x1,y1 = %d,%d\n",x1,y1);
#endif
	factor = scaley / scalex;
	/* initalize the arrays  - use value as a temporary variable */
	for (value = 0; value <= 9; value++)
		 start[X] [value] = start[Y] [value] = aend[X] [value]
				  = aend[Y] [value] = drw_oct[value] = 0.0;
	value = 4;
	/* Calculate the radius */

	dx = xi - x0;
	dy = yi - y0;
	radius = sqrt(dx * dx + dy * dy);
	xradius = radius * scalex;	/* the radius on the x space */
	ox = xi * scalex;		/* the x coord of the centre in x */
	oyx = yi * scalex;		/* the y coord of the centre in x */
#ifdef	DEBUG
	oy = yi * scaley;		/* the y coord of the centre in y */
#endif

	/* now patch in the arc start and end ... */
	st_x = x0 * scalex;		/* the start x coord in the x scale */
	st_y = y0 * scalex;		/* start y coord in the x scale */
	en_x = x1 * scalex;		/* end x coord in the x scale */
	en_y = y1 * scalex;		/* end y coord in the x scale */
	st_oct = octant(ox,oyx,st_x,st_y); /* start octant of the arc */
	en_oct = octant(ox,oyx,en_x,en_y); /* end octant of the arc */
	/* now initialise some elements of start and end - others are
	   always zero. */

	limit = xradius / sqrt(2.0);	 /* the point at which x = y */
	start[X][1] = start[Y][2] = start[Y][3] = start[X][8] = (int) xradius;

	aend[X][1]  = aend[Y][1] = aend[X][2] = aend[Y][2] = aend[Y][3]
		    = aend[X][7] = aend[Y][4] = aend[X][8] = (int) limit;

	aend[X][3]  = aend[X][4] = aend[X][5] = aend[Y][5] = aend[X][6]
		    = aend[Y][6] = aend[Y][7] = aend[Y][8] = (int) -limit;


	start[X][4] = start[X][5] = start[Y][6] = start[Y][7] = (int) -xradius;


#if DEBUG
	fprintf(stderr,"centre in x is ox,oyx = %d,%d \n",ox,oyx);
	fprintf(stderr,"st_x,st_y  is %d,%d  \n",st_x,st_y);
	fprintf(stderr,"x0,y0 is %d,%d	 \n",x0,y0);
	fprintf(stderr,"en_x,en_y  is %d,%d  \n",en_x,en_y);
	fprintf(stderr,"x1,y1 is %d,%d \n",x1,y1);
	fprintf(stderr,"st_oct is %d, en_oct is %d \n",st_oct,en_oct);
#endif
	if ( st_oct == 2.0 * (int) ( st_oct / 2.0 ) )
			/* start octant is 'even' */
			{
#if DEBUG
			fprintf(stderr,"start octant is even \n");
#endif
			aend[X][st_oct] = st_x - ox;
			aend[Y][st_oct] = st_y - oyx;
			}
	else
			/* start octant is 'odd' */
			{
#if DEBUG
			fprintf(stderr,"start octant is odd \n");
#endif
			start[X][st_oct] = st_x - ox;
			start[Y][st_oct] = st_y - oyx;
			}

	if ( en_oct == 2.0 * (int) ( en_oct / 2.0 ) )
			/* end octant is 'even' */
			{
#if DEBUG
			fprintf(stderr,"end octant is even\n");
#endif
			start[X][en_oct] = en_x - ox;
			start[Y][en_oct] = en_y - oyx;
			}
	else
			/* end octant is 'odd' */
			{
#if DEBUG
			fprintf(stderr,"end octant is odd\n");
#endif
			aend[X][en_oct] = en_x - ox;
			aend[Y][en_oct] = en_y - oyx;
			}
#define min(A,B) ( (A < B) ? A : B )

	if ( st_oct < en_oct )
		for ( value = st_oct; value <= min(en_oct,8); value++)
			drw_oct[value] = 1;
	else
		{
		for ( value = st_oct; value <= 8; value++)
			drw_oct[value] = 1;
		for ( value = 1; value <= en_oct; value++)
			drw_oct[value] = 1;
		}
#if DEBUG
	for (value = 1; value <9;value++)
	  {
	  fprintf(stderr,"start[X][%d],[Y][%d] is %f,%f \n",value,value,
			  start[X][value],start[Y][value]);

	  fprintf(stderr,"aend[X][%d],[Y][%d] is %f, %f \n",value,value,
			  aend[X][value],aend[Y][value]);

	  fprintf(stderr,"drw_oct[%d] is %d \n",value,drw_oct[value]);
	  }
#endif
	sc_xobot = obotx * scalex;	/* the origin of the window in x */
	sc_yobot = oboty * scaley;
#if DEBUG
    fprintf(stderr,"sc_xobot is %f, sc_yobot is %f \n",sc_xobot,sc_yobot);
#endif
	y = xradius ;
	d = 3 - 2 * xradius ;
#if DEBUG
	fprintf(stderr,"xradius = %d\n",xradius);
	fprintf(stderr,"ox = %d, oy = %d , y = xradius\n",ox,oy);
#endif
	x = 0 ;

	while( x < y )
		{
		arc_points(x,y,ox,oyx,value) ;
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
		arc_points(x,y,ox,oyx,value) ;
}

#ifdef DEBUG2
arc_points(x,y,ox,oyx,value)
#else
arc_points( x, y, ox, oyx )
#endif
register int	x,y,	      /* x and y coordinates of the point */
		ox,oyx; 	 /* x and y coordinates of the centre */


#ifdef DEBUG2
int value;
#endif

{
	extern float factor;
	int setbit(),xcoord(),ycoord(),px,py,mx,my,xtemp,ytemp;
#if DEBUG2
	fprintf(stderr,"arcpoints(%d,%d,%d)\n",x,y,value);  /*DEBUG*/
#endif
#define X_coord(A)     (A + xtemp)
#define Y_coord(A)     (ytemp + ( ( A + oyx )  * factor))
	xtemp = botx - sc_xobot + ox;
	ytemp = boty - sc_yobot;
	px = X_coord(x);
	py = Y_coord(y);
	mx = X_coord(-x);
	my = Y_coord(-y);

	/* now for each possible octant we test to see if we need to draw
	   the arc for that octant */

	if (( drw_oct[2] > 0 && x >= start[X][2] &&  x <= aend[X][2]
		  && y <= start[Y][2] && y >= aend[Y][2]))
		  setbit(px,py);
#if DEBUG2
	else
		fprintf(stderr," %d,%d not for octant 2\n",x,y);
#endif
	if (( drw_oct[7] > 0 && x >= start[X][7] && x <= aend[X][7]
		  && -y >= start[Y][7] && -y <= aend[Y][7]))
		  setbit(px,my);
#if DEBUG2
	else
		fprintf(stderr," %d,%d not for octant 7\n",x,-y);
#endif
	if (( drw_oct[6] > 0 && -x <= start[X][6] && -x >= aend[X][6]
		  && -y >= start[Y][6] && -y <= aend[Y][6]))
		  setbit(mx,my);
#if DEBUG2
	else
		fprintf(stderr," %d,%d not for octant 6\n",-x,-y);
#endif
	if ( drw_oct[3] > 0 && start[X][3] >= -x && -x >= aend[X][3] &&
		  start[Y][3] >= y && y >= aend[Y][3])
		  setbit(mx,py);
#if DEBUG2
	else
		fprintf(stderr," %d,%d not for octant 3\n",-x,y);
#endif
	px = Y_coord(x);
	py = X_coord(y);
	mx = Y_coord(-x);
	my = X_coord(-y);

	if ( drw_oct[1] > 0 && y <= start[X][1] && y >= aend[X][1] &&
		  x >= start[Y][1] && x <= aend[Y][1])
		  setbit(py,px);
#if DEBUG2
	else
		fprintf(stderr," %d,%d not for octant 1\n",y,x);
#endif
	if ( drw_oct[8] > 0 && y <= start[X][8] && y >= aend[X][8] &&
		  -x <= start[Y][8] && -x >= aend[Y][8])
		  setbit(py,mx);
#if DEBUG2
	else
		fprintf(stderr," %d,%d not for octant 8\n",y,-x);
#endif
	if ( drw_oct[5] > 0 && -y >= start[X][5] && -y <= aend[X][5] &&
		  -x <= start[Y][5] && -x >= aend[Y][5])
		  setbit(my,mx);
#if DEBUG2
	else
		fprintf(stderr," %d,%d not for octant 5\n",-y,-x);
#endif
	if ( drw_oct[4] > 0 && -y >= start[X][4] && -y <= aend[X][4] &&
		  x >= start[Y][4] && x <= aend[Y][4])
		  setbit(my,px);
#if DEBUG2
	else
		fprintf(stderr," %d,%d not for octant 4\n",-y,x);
#endif
}

octant(x,y,xp,yp)

int x,y,xp,yp;

/* This function returns a number indicating the octant of the circle
   centre (x,y) contains the point (xp,yp)

*/

{
#if DEBUG
	fprintf(stderr,"Octant,centre is x,y = %d,%d,point is xp,yp= %d,%d\n",
			x,y,xp,yp);
#endif

	if ( x <= xp )

		/* x is positive */

		if ( y <= yp )

			/* y is positive */
			if ( xp >= yp )
				return(1);
			else
				return(2);
		else

			/* y is negative */

			if ( -yp >= xp )
				return(7);
			else
				return(8);
	else
		/* x is negative */

		if ( y <= yp )

			/* y is positive */
			if ( -xp >= yp )
				return(4);
			else
				return(3);
		else

			/* y is negative */

			if ( -yp >= -xp )
				return(6);
			else
				return(5);
}

