
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/* SCCSID: @(#)show.c	3.0	4/22/86 */
#include <curses.h>
#include <signal.h>

#undef LINES
#define LINES 20

main(argc, argv)
char **argv;
{
	FILE *fd;
	char linebuf[512];
	int line;
	int done();

	if (argc < 2) {
		(void) fprintf(stderr, "Usage: show file\n");
		exit(1);
	}
	fd = fopen(argv[1], "r");
	if (fd == NULL) {
		perror(argv[1]);
		exit(2);
	}
	(void) signal(SIGINT, done);	/* die gracefully */

	initscr();			/* initialize curses */
	noecho();			/* turn off tty echo */
	crmode();			/* enter cbreak mode */

	for (;;) {			/* for each screen full */
		(void) move(0, 0);
		/* werase(stdscr); */
		for (line=0; line<LINES; line++) {
			if (fgets(linebuf, sizeof linebuf, fd) == NULL) {
				clrtobot();
				done();
			}
			(void) mvprintw(line, 0, "%s", linebuf);
		}
		(void) refresh();	/* sync screen */
		printf(" --more-- [q to quit]\n");
		if(getch() == 'q')	/* wait for user to read it */
			done();
	}
}

/*
 * Clean up and exit.
 */
done()
{
	(void) move(LINES-1,0);		/* to lower left corner */
	clrtoeol();			/* clear bottom line */
	(void) refresh();		/* flush out everything */
	endwin();			/* curses cleanup */
	exit(0);
}
