
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/* SCCSID: @(#)overlay.c	3.0	4/22/86 */
/*
 * In the versions of libcurses I have seen, none has
 * contained bug-free versions of routines overlay and
 * overwrite. Some don't work if the windows are not
 * starting at 0,0; some don't work if the overlaid window
 * is not completely within the boundary of the other, and
 * some just don't work.
 *
 * Below is a my version of overlay.c and, if needed, you
 * can create a working version of overwrite by removing
 * the line "if( !isspace(*cp) )". It is not the most
 * efficient way to do overwrite but few seem to use it.
 *
 * From: probe@mm730.uq.OZ (Cameron Davidson)
 * Organization: Mining&Metal. Eng; Univ of Qld; Brisbane; Aus
 */

# include	"curses.ext"
# include	<ctype.h>

# define	min(a,b)	(a < b ? a : b)
# define	max(a,b)	(a > b ? a : b)

/*
 * find the maximum line and column of a window in screen coords
 */
#define		MAX_LINE(win)	(win->_maxy + win->_begy)
#define		MAX_COL(win)	(win->_maxx + win->_begx)

/*
 *  This routine writes win1 on win2 non-destructively.
 *  Rewritten CJD UQ Min & Met Eng, March 85
 */
overlay(win1, win2)
reg WINDOW *win1, *win2;
{
	reg char	*cp, *end;
	reg int 	x2, i,
			endline, ytop,  y1,  y2,
			ncols,	xleft, x1start, x2start;
# ifdef DEBUG
	fprintf(outf, "OVERLAY(%0.2o, %0.2o);\n", win1, win2);
# endif

	/*
	 * ytop and xleft are starting line and col in terms
	 * of screen coordinates
	 */
	ytop = max( win1->_begy, win2->_begy );
	xleft = max( win1->_begx, win2->_begx );

	/* last line of  window 1 to look at */
	endline = min( MAX_LINE(win1),  MAX_LINE(win2) ) - win1->_begy;
	ncols	= min( MAX_COL(win1),  MAX_COL(win2) ) - xleft;

	/*
	 * y1 and x1start are starting row and line relative to window 1
	 */
	y1 = ytop - win1->_begy;
	x1start = xleft - win1->_begx;
	y2 = ytop - win2->_begy;
	x2start = xleft - win2->_begx;

	while ( y1 < endline ) {
		cp = &win1->_y[y1++][x1start];
		end = cp + ncols;
		x2 = x2start;
		for ( ; cp < end; cp++ ) {
			if ( !isspace( *cp ) ) {
				wmove( win2, y2, x2 );
				waddch( win2, *cp );
			}
			x2++;
		}
		y2++;
	}
}
