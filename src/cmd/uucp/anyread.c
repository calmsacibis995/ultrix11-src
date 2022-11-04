
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)anyread.c	3.0	4/22/86";

#include "uucp.h"
#include <sys/types.h>
#include <sys/stat.h>


/*******
 *	anyread		check if anybody can read
 *	return 0 ok: FAIL not ok
 */


anyread(file)
char *file;
{
	struct stat s;

	if (stat(subfile(file), &s) != 0)
		/* for security check a non existant file is readable */
		return(0);
	if (!(s.st_mode & ANYREAD))
		return(FAIL);
	return(0);
}
