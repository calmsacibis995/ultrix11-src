
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)cpmv.c	3.0	4/22/86";

#include "uucp.h"
#include <sys/types.h>
#include <sys/stat.h>


/***
 *	xcp(f1, f2)	copy f1 to f2
 *	char *f1, *f2;
 *
 *	return - 0 ok  |  FAIL failed
 *
 *
 *	decvax!larry -  copy modified path back to pointer
 */

xcp(f1, f2)
register char *f1, *f2;
{
	char buf[BUFSIZ];
	int len;
	register FILE *fp1, *fp2;
	char *lastpart();
	char full[100];
	struct stat s;

	if ((fp1 = fopen(subfile(f1), "r")) == NULL) {
		DEBUG(9, "can't open to read %s", subfile(f1));
		return(FAIL);
	}
	strcpy(full, f2);
	if (stat(subfile(f2), &s) == 0) {
		/* check for directory */
		if ((s.st_mode & S_IFMT) == S_IFDIR) {
			strcat(full, "/");
			strcat(full, lastpart(f1));
			/* calling program should know new name - decvax!larry*/
			strcpy(f2, full);
		}
	}
	DEBUG(4, "full %s\n", full);
	if ((fp2 = fopen(subfile(full), "w")) == NULL) {
		fclose(fp1);
		return(FAIL);
	}
	while((len = fread(buf, sizeof (char), BUFSIZ, fp1)) > 0)
		fwrite(buf, sizeof (char), len, fp2);
	fclose(fp1);
	fclose(fp2);
	return(0);
}


/*
 *	xmv(f1, f2)	move f1 to f2
 *	char * f1, *f2;
 *
 *	return  0 ok  |  FAIL failed
 */

xmv(f1, f2)
register char *f1, *f2;
{
	int ret;

	if (link(subfile(f1), subfile(f2)) < 0) {
		/*  copy file  */
		ret = xcp(f1, f2);
		if (ret == 0)
			unlink(subfile(f1));
		return(ret);
	}
	unlink(subfile(f1));
	return(0);
}
