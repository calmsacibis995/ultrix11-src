
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)dounit.c	3.0	4/21/86";
#include "stdio.h"
#include "lrnref"

dounit()
{
	char tbuff[100];
	char tbuff2[100];

	if (todo == 0)
		return;
	wrong = 0;
retry:
	start(todo);
	sprintf(tbuff, "../../%s/L%s", sname, todo);	/* script = lesson */
	scrin = fopen(tbuff, "r");
	if (scrin == NULL) {
		fprintf(stderr, "No script.\n");
		wrapup(1);
	}

	copy(0, scrin);
	if (more == 0)
		return;
	copy(1, stdin);
	if (more == 0)
		return;
	copy(0, scrin);

	if (comfile >= 0)
		close(comfile);
	wait(&didok);
	didok = (status == 0);
	if (!didok) {
		wrong++;
		printf("\nSorry, that's %snot right.  Do you want to try again?  ",
			wrong > 1 ? "still " : "");
		fflush(stdout);
		for(;;) {
			gets(tbuff);
			if (tbuff[0] == 'y') {
				printf("\nDo you want the instructions printed again? ");
				fflush(stdout);
				gets(tbuff2);
				if (tbuff2[0] == 'y') {
					retype = 1;
					goto retry;
				} else printf("Try the problem again.\n");
				fflush(stdout);
				retype = 0;
				goto retry;
			} else if (strcmp(tbuff, "bye") == 0) {
				wrapup(1);
			} else if (tbuff[0] == 'n') {
				wrong = 0;
				printf("\nOK.  Lesson %s (%d)\n", todo, speed);
				printf("Skipping to next lesson.\n\n");
				fflush(stdout);
				break;
			} else {
				printf("Please type yes, no or bye:  ");
				fflush(stdout);
			}
		}
	}
	setdid(todo, sequence++);
}

