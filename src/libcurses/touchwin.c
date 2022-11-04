
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/* SCCSID: @(#)touchwin.c	3.0	4/22/86 */
# include	"curses.ext"

/*
 * make it look like the whole window has been changed.
 *
 * 4/22/86 (Berkeley) @(#)touchwin.c	3.0
 */
touchwin(win)
reg WINDOW	*win;
{
	reg WINDOW	*wp;

	do_touch(win);
	for (wp = win->_nextp; wp != win; wp = wp->_nextp)
		do_touch(wp);
}

/*
 * do_touch:
 *	Touch the window
 */
static
do_touch(win)
reg WINDOW	*win; {

	reg int		y, maxy, maxx;

	maxy = win->_maxy;
	maxx = win->_maxx - 1;
	for (y = 0; y < maxy; y++) {
		win->_firstch[y] = 0;
		win->_lastch[y] = maxx;
	}
}
