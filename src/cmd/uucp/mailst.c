
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)mailst.c	3.0	4/22/86";

#include "uucp.h"



/*******
 *	mailst(user, str, file, ferr)
 *
 *	mailst  -  this routine will fork and execute
 *	a mail command sending string (str) to user (user).
 *	If file is non-null, the file is also sent.
 *	(this is used for mail returned to sender.)
 */

/* decvax!larry - ifdef ULTRIX use "uucp" id on from line for mail */

mailst(user, str, file, ferr)
char *user, *str, *file, *ferr;
{
	FILE *fp, *fi;
	extern FILE *popen(), *pclose();
	char cmd[100], buf[BUFSIZ];
	int nc;

#ifdef ULTRIX
	sprintf(cmd, "/bin/mail -r uucp %s", user);
#else
	sprintf(cmd, "mail %s", user);
#endif
	
	if ((fp = popen(cmd, "w")) == NULL)
		return;
	fprintf(fp, "%s", str);

/* 

	if (*ferr != '\0' && (fi = fopen(subfile(ferr), "r")) != NULL) {
		fprintf(fp, "\n\n%s\n\n",  "****** the following lines are the standard error output for the command *******");
		while ((nc = fread(buf, sizeof (char), BUFSIZ, fi)) > 0)
			fwrite(buf, sizeof (char), nc, fp);
		fclose(fi);
	}

*/
	if (*file != '\0' && (fi = fopen(subfile(file), "r")) != NULL) {
		fprintf(fp, "\n\n%s\n\n",  "****** the following is the original input file *******");
		while ((nc = fread(buf, sizeof (char), BUFSIZ, fi)) > 0)
			fwrite(buf, sizeof (char), nc, fp);
		fclose(fi);
	}

	pclose(fp);
	return;
}
