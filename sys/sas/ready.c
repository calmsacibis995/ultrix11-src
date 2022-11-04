/*
 * SCCSID: @(#)ready.c	3.0	4/21/86
 */

/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/
/*
 * ULTRIX-11 ready command
 *
 * Asks if you are ready then waits for a y or n
 * response. Used by shell procedures like OSLOAD.
 *
 * Fred Canter 9/20/83
 */
#include <stdio.h>

char	line[150];

main()
{
	register int cc;

loop:
	printf("\nReady <y or n> ? ");
	fflush(stdout);
	cc = read(0, (char *)&line, 132);
	switch(cc) {
	case 2:
		if(line[0] == 'y') {
			printf("\n");
			exit(0);
		}
		break;
	case 4:
		line[3] = 0;
		if(strcmp(line, "yes") == 0) {
			printf("\n");
			exit(0);
		}
		break;
	default:
		break;
	}
	goto loop;
}
