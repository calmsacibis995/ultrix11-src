
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#include <stdio.h>

char line[500];
char bline[500];

main()
{
	double p;
	register char * l;
	long t;
	FILE *f;

	f = fopen("/usr/games/lib/fortunes", "r");
	if (f == NULL) {
		printf("Memory fault -- core dumped\n");
		exit(1);
	}
	time(&t);
	srand(getpid() + (int)((t>>16) + t));
	p = 1.;
	for(;;) {
		l = fgets(line, 500, f);
		if(l == NULL)
			break;
		if(rand() < 32768./p)
			strcpy(bline, line);
		p += 1.;
	}
	fputs(bline, stdout);
	return(0);
}
