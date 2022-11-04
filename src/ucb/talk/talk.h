
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)talk.h	3.0	(ULTRIX-11)	4/22/86
 */

#include <curses.h>
#include <utmp.h>

#define	forever		for(;;)

#define	BUF_SIZE	512

FILE *popen();
int quit(), sleeper();

extern int sockt;
extern int curses_initialized;
extern int invitation_waiting;

#ifdef pdp11
#define	current_state	curstate
#define	current_line	curline
#endif pdp11
extern char *current_state;
extern int current_line;

typedef struct xwin {
	WINDOW *x_win;
	int x_nlines;
	int x_ncols;
	int x_line;
	int x_col;
	char kill;
	char cerase;
	char werase;
} xwin_t;

extern xwin_t my_win;
extern xwin_t his_win;
extern WINDOW *line_win;
