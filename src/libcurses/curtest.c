
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/* SCCSID: @(#)curtest.c	3.0	4/22/86	*/
/*
 * ( cc cursetest.c -lcurses -ltermlib )
 *
 *	cursetest.c
 *
 * a program to test overlay/overwrite routines in libcurses
 * the program should draw the following pattern in 4 places on the screen:

	 + + + + + + + + + +
	 + wwwwwwwwww+ + + +
	 + w        w+ + + +
	 + w    llllllllll +
	 + w    l   w+ + l +
	 + wwwwwlwwww+ + l +
	 + + + +l+ + + + l +
	 + + + +llllllllll +
	 + + + + + + + + + +
 *
 * and finally in the centre it should draw another pattern with the 'w'
 * and 'l' windows overlapping opposite corners of the base
 *	NOTE: only two sides of each should be visible.
 *
 * From: probe@mm730.uq.OZ (Cameron Davidson)
 * Organization: Mining&Metal. Eng; Univ of Qld; Brisbane; Aus
 */

#include	<stdio.h>
#include	<curses.h>


WINDOW	*Wbase,
	*Woverw,
	*Woverl;

main( ac, av )
	char	**av;
{
	initscr();
	normal_pattern( 0, 0 );
	normal_pattern( 10, 0 );
	normal_pattern( 0, 50 );
	normal_pattern( 10, 50 );
		/* now test when windows extend beyond base window */
	Wbase = newwin( 9, 19, 5, 25 );
	Woverw = newwin( 5, 10, 3, 22);
	Woverl = newwin( 5, 10, 11, 37);
	draw_pattern();
	mvcur( 0, COLS-1, LINES-1, 0);
}

	/*
	 * setup windows to draw patterns completely within base area
	 */
normal_pattern( base_y_begin, base_x_begin )
	int	base_y_begin, base_x_begin;
{
	if ( base_y_begin + 9 >= LINES  || base_x_begin + 19 >= COLS ) {
		fputs( "WON'T FIT\n", stderr);
		exit(1);
	}
	Wbase = newwin( 9, 19, base_y_begin, base_x_begin );
	Woverw = newwin( 5, 10, base_y_begin + 1, base_x_begin + 2);
	Woverl = newwin( 5, 10, base_y_begin + 3, base_x_begin + 7);
	draw_pattern();
}

draw_pattern( )
{
	register	int	line;

	for( line=0 ; line < 9 ; line++ )
		mvwaddstr( Wbase, line, 0, "+ + + + + + + + + +");
	wrefresh(Wbase);
	box( Woverw, 'w', 'w');
	box( Woverl, 'l', 'l');
	overwrite( Woverw, Wbase );
	wrefresh(Wbase);
	overlay( Woverl, Wbase );
	wrefresh(Wbase);
	delwin( Wbase );
	delwin( Woverw );
	delwin( Woverl );
}
