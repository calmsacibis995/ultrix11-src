
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/* SCCSID: @(#)box.c	3.0	4/22/86 */
# include	"curses.ext"

/*
 *	This routine draws a box around the given window with "vert"
 * as the vertical delimiting char, and "hor", as the horizontal one.
 *
 * @(#) 2/16/82 (Berkeley) box.c	1.2
 */
box(win, vert, hor)
reg WINDOW	*win;
char		vert, hor; {

	reg int		i;
	reg int		endy, endx;
	reg char	*fp, *lp;

	endx = win->_maxx;
	endy = win->_maxy - 1;
	fp = win->_y[0];
	lp = win->_y[endy];
	for (i = 0; i < endx; i++)
		fp[i] = lp[i] = hor;
	endx--;
	for (i = 0; i <= endy; i++)
		win->_y[i][0] = (win->_y[i][endx] = vert);
	if (!win->_scroll && (win->_flags&_SCROLLWIN))
		fp[0] = fp[endx] = lp[0] = lp[endx] = ' ';
	touchwin(win);
}

