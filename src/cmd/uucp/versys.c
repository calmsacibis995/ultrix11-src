
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)versys.c	3.0	4/22/86";

#include "uucp.h"


#define SNAMESIZE 7

/*******
 *	versys(name)	verify system names n1 and n2
 *	char *name;
 *
 *	return codes:  0  |  FAIL
 */

versys(name)
char *name;
{
	FILE *fp;
	char line[1000];
	char s1[SNAMESIZE + 1];
	char myname[SNAMESIZE + 1];

	sprintf(myname, "%.7s", Myname);
	sprintf(s1, "%.7s", name);
	if (strcmp(s1, myname) == 0)
		return(0);
	fp = fopen(SYSFILE, "r");
	if (fp == NULL)
		return(FAIL);
	
	while (cfgets(line, sizeof(line), fp) != NULL) {
		char *targs[100];

		getargs(line, targs);
		targs[0][7] = '\0';
		if (strcmp(s1, targs[0]) == SAME) {
			fclose(fp);
			return(0);
		}
	}
	fclose(fp);
	return(FAIL);
}
