
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * date+5 - print the time, plus 5 mins, on the standard
 * output, ie. output 1947 when the time is 7:42pm
 * Only here so can compile and test something for USAT.
 */

#include <time.h>
char *asctime();
main()
{
	struct tm *ls;
	long tloc;

	time(&tloc);
	ls = localtime(&tloc);
	ls->tm_min+=5;		/* add 5 minutes */
	if (ls->tm_min>=60) {
		ls->tm_min -= 60;
		ls->tm_hour += 1;
	}
	if (ls->tm_hour>=24)
		ls->tm_hour -= 24;

	printf("%02d%02d\n",ls->tm_hour,ls->tm_min);
}
