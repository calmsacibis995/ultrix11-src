/*
 * SCCSID: @(#)driver.c	3.0	4/22/86 
 */

/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*

 Facility:	Part of the Plot package for the REGIS terminals

 Calls: 	move(), line(), label(), point(), cont(), space(), arc()
		circle(), linemod(), dot(), color(), map()

 Abstract:	This is the main() function for plot.

		Plotting commands are read from standard input. A big
		case statement is used to select the appropriate action
		routine.

 Modified by:

 Date:		By:		Reason:

 ??-Feb-1984	David Roberts.	Add debugging statements.

 ??-Apr-1984	David Roberts.	Add extra plot command to select colour - j

 ??-Apr-1984	David Roberts.	Add extra plot command - k to load a colour
				register.

 06-Jun-1984	David Roberts. Comment code!

 01-Aug-1984	Kevin J. Dunlap, change call to colour to color.

 02-Apr-1985	Kevin J. Dunlap, Modify for REGIS terminals

*/

#include <stdio.h>
#define DEBUG 0 	
float deltx;
float delty;

main(argc,argv)  char **argv; {
	int std=1;
	FILE *fin;
#if DEBUG
#include "bitmap.h"
extern struct btregs *bitmap;
#endif
	while(argc-- > 1) {
		if(*argv[1] == '-')
			switch(argv[1][1]) {
				case 'l':
					deltx = atoi(&argv[1][2]) - 1;
					break;
				case 'w':
					delty = atoi(&argv[1][2]) - 1;
					break;
				}

		else {
			std = 0;
			if ((fin = fopen(argv[1], "r")) == NULL) {
				fprintf(stderr, "can't open %s\n", argv[1]);
				exit(1);
				}
			fplt(fin);
			}
		argv++;
		}
	if (std)
		fplt( stdin );
	exit(0);
	}


fplt(fin)  FILE *fin; {
	int c;
	char s[256];
	int xi,yi,x0,y0,x1,y1,r,dx,n,i;
	int pat[256];

	openpl();
	while((c=getc(fin)) != EOF){
#if DEBUG
		fprintf(stderr,"DRIVER: command is %c ( %o octal)\n",c,c);
#endif
		switch(c){
		case 'm':			/* m - move */
			xi = getsi(fin);	/* get the x coordinate */
			yi = getsi(fin);	/* and the y coordinate */
			move(xi,yi);		/* move to the new point */
			break;
		case 'l':			/* l - line */
			x0 = getsi(fin);	/* get the start x */
			y0 = getsi(fin);	/* and start y */
			x1 = getsi(fin);	/* then end x */
			y1 = getsi(fin);	/* and end y */
			line(x0,y0,x1,y1);	/* draw the line */
			break;
		case 't':			/* l - label */
			gets(s,fin);		/* read the string */
			label(s);		/* write the label */
			break;
		case 'e':			/* e - erase */
			erase();		/* clear the screen */
			break;
		case 'p':			/* p - point */
			xi = getsi(fin);	/* get the x */
			yi = getsi(fin);	/* and y coordinates */
			point(xi,yi);		/* make the point */
			break;
		case 'n':			/* n - cont */
			xi = getsi(fin);	/* destination x */
			yi = getsi(fin);	/* and y. Then draw a line */
			cont(xi,yi);		/* from current place to x,y*/
			break;
		case 's':			/* s - space */
			x0 = getsi(fin);	/* lower left corner x */
			y0 = getsi(fin);	/* lower left corner y */
			x1 = getsi(fin);	/* upper right corner x */
			y1 = getsi(fin);	/* upper right corner y */
			space(x0,y0,x1,y1);	/* set the new spacing */
			break;
		case 'a':			/* a - arc */
			xi = getsi(fin);	/* x coordinate of centre */
			yi = getsi(fin);	/* y coordinate of centre */
			x0 = getsi(fin);	/* start point x */
			y0 = getsi(fin);	/* start point y */
			x1 = getsi(fin);	/* end point x */
			y1 = getsi(fin);	/* end point y */
			arc(xi,yi,x0,y0,x1,y1); /* draw the arc */
			break;
		case 'c':			/* c - circle */
			xi = getsi(fin);	/* x coordinate of centre */
			yi = getsi(fin);	/* y coordinate of centre */
			r = getsi(fin); 	/* radius */
			circle(xi,yi,r);	/* draw the circle */
			break;
		case 'f':			/* f - linemod */
			gets(s,fin);		/* get the linemode */
			linemod(s);		/* and set it */
			break;
		case 'd':			/* d - dot */
			xi = getsi(fin);	/* x coordinate? */
			yi = getsi(fin);	/* y coordinate? */
			dx = getsi(fin);	/* increment in x?
			n = getsi(fin); 	/* get the pattern? */
			for(i=0; i<n; i++)pat[i] = getsi(fin);
			dot(xi,yi,dx,n,pat);	/* do dot...*/
			break;
		case 'j':
			xi = getsi(fin);	/* get the colour no */
			color(xi);		/* select it */
			break;
		case 'k':
			xi = getsi(fin);	/* the colour register no */
			yi = getsi(fin);	/* the value to load */
			map(xi,yi);		/* load it */
			break;
			}
		}
	closepl();
	}
getsi(fin)  FILE *fin; {	/* get an integer stored in 2 ascii bytes. */
	short a, b;
	if((b = getc(fin)) == EOF)
		return(EOF);
	if((a = getc(fin)) == EOF)
		return(EOF);
	a = a<<8;
	return(a|b);
}
gets(s,fin)  char *s;  FILE *fin; {
	for( ; *s = getc(fin); s++)
		if(*s == '\n')
			break;
	*s = '\0';
	return;
}
