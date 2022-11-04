/*
 * SCCSID: @(#)subr.c	3.0	4/22/86
 */

/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*

  Facility:	 Part of the plot package for the LA100 printer.

  Called by:	 driver()

  Abstract:  This contains all the subroutines need for handling
	     the bitmap data structure and scaling.

  Author:	 Kevin J. Dunlap

  Creation:	 July 1984

*/

#include "la100.h"

#define min(A,B) ( (A < B) ? A : B )
#define max(A,B) ( (A > B) ? A : B)
#define DEBUG 0 
/* scale X */
xsc(xi){
	int xa;
	xa = (xi-obotx)*scalex+botx;
	return(xa);
}

/* scale Y */
ysc(yi){
	int ya;
	ya = (yi-oboty)*scaley+boty;
	return(ya);
}

/***********************************************************************

The bitmap used in the plot library routine for the LA100 is an array          
of charactors. This is done to save space.  We only need to address
six bits in the Y direction coresponding to the X direction.

The LA100 print head is addressable in sixels along the Y coordinates.
This means for every move of in the X direction we can turn on six bits
EK-0LA50-RM-002 page 44-46 for more information.
***********************************************************************/

setbit(x,y)
{
   int b;
   y = y0 - y;
   x = x0 + x;
   if (x >= MAXX || x < 0 || y >= MAXY || y < 0)
	return;
   b = y % 6;
   y = y / 6;
   dminx = min(dminx, x);
   dmaxx = max(dmaxx, x);
   dminy = min(dminy, y);
   dmaxy = max(dmaxy, y);
   bitmap[x][y] |= bits[b];
#if DEBUG
	fprintf(stderr,"bitmap(%d,%d) %d\n",x,y,bitmap[x][y]);
   fflush(stderr);
#endif
}
/* drawmap()
 * 		Display the bitmap on the printer.
 *
 *		This routine is called by closeplt() and traverses
 *		thru the bitmap calling putbit() to display the bits.
 */
drawmap()
{
   int		x, y;
#if DEBUG
   fprintf(stderr,"enter draw map\n");
#endif
   for (y = dminy; y <= dmaxy; ++y)
   {
	for (x = dminx; x <= dmaxx; ++x){
	putbit(bitmap[x][y] + 077); putbit(bitmap[x][y] + 077);
	}
	putbit('-');
   }
}

/* putbit(b)
 *		Send the bits to the printer.
 *
 *		This routine is called by drawmap to display the bits on
 *		the printer.  The LA50 is able to repeat a given bit sequence
 *		by passing to it the number of times you want the sequence
 *		repeated and the bit pattern.  This is faster then sending
 *		a given bit sequence to the printer several times
 *		in a row.
 *
 *		Inorder to take advantage of this feature, this routine
 *		stores the bitpattern. If the bitpattern is the same as
 *		the last bitpattern passed to it, a counter is incremented
 *		and we return to drawmap.  If the bitpattern isn't the same
 *		we send to the printer the number of times we want
 *		the pattern repeated and the bitpattern and store the
 *		new bitpattern.
 */
putbit(b)
char b;
{
	static char last = 0;
	static n = 0;

	if (b == last)	 /* new bitpattern the same as the last? */
	{ 
		++n;	 /* Yes, increment counter and return */
		return; 
	}

	if ( n == 2)	 /* If count is only 2 */
	{ 
		putchar(last); /* just send the bitpattern twice */
		putchar(last); /* This is done for optimazation  */
#if DEBUG
		fprintf(stderr,"putbit: %d %d \n",last,last);
#endif
	}
	else if (n > 2) {  /* count is greater the 2 */
		printf("!%d%c", n, last); /* send the repeat introducer "!"*/
		/* the count and bitpattern */
#if DEBUG
		fprintf(stderr,"putbit:!%d %d \n",n,last);
#endif
	}
	else if ( n == 1 ) { /* count is one */
		putchar(last); /* just print the bitpattern */
#if DEBUG
		fprintf(stderr,"putbit: %d %d \n",last,last);
#endif
	}
	if (b == '-')	  /* do we want to print a graphic new line? */
	{ 
		putchar('-'); /* yes, print a new line */
#if DEBUG
		fprintf(stderr,"putbit: - \n");
#endif
		last = n = 0; /* clear the last bitpattern and count */
	}
	else
		last=b, n = 1; /* store the bitpattern and set count to one */
}
