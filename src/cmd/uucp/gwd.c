
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)gwd.c	3.0	4/22/86";

#include "uucp.h"

/*******
 *	gwd(wkdir)	get working directory
 *
 *	return codes  0 | FAIL
 */

gwd(wkdir)
char *wkdir;
{
	FILE *fp;
	extern FILE *popen(), *pclose();
	char *c;

#ifdef ULTRIX
	ASSERT2(getwd(wkdir) != 0 , "could not determine working directory",
		wkdir, 0);
	return(0);
#else
	*wkdir = '\0';
	if ((fp = popen("pwd 2>&-", "r")) == NULL)
		return(FAIL);
	if (fgets(wkdir, 100, fp) == NULL) {
		pclose(fp);
		return(FAIL);
	}
	if (*(c = wkdir + strlen(wkdir) - 1) == '\n')
		*c = '\0';
	pclose(fp);
	return(0);
#endif
}
