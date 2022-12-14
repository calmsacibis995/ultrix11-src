
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)hunt8.c	3.0	4/22/86";
# include "stdio.h"
# include "assert.h"
# define unopen(fil) {if (fil!=NULL) {fclose(fil); fil=NULL;}}
extern long indexdate, gdate();
runbib (s)
	char *s;
{
/* make a file suitable for fgrep */
char tmp[200];
sprintf(tmp, "/usr/lib/refer/mkey %s >%s.ig", s,s);
system(tmp);
}
makefgrep(indexname)
	char *indexname;
{
	FILE *fa =NULL, *fb =NULL;
	if (ckexist(indexname, ".ig"))
		{
		/* existing gfrep -type index */
# if D1
		fprintf(stderr, "found fgrep\n");
# endif
		fa = iopen(indexname, ".ig");
		fb = iopen(indexname, "");
		if (gdate(fb)>gdate(fa))
			{
			if (fa!=NULL)
				fclose(fa);
			runbib(indexname);
			fa= iopen(indexname, ".ig");
			}
		indexdate = gdate(fa);
		unopen(fa); unopen(fb);
		}
	else
	if (ckexist(indexname, ""))
		{
		/* make fgrep */
# if D1
			fprintf(stderr, "make fgrep\n");
# endif
		runbib(indexname);
		time(&indexdate);
		unopen(fb);
		}
	else /* failure */
		return(0);
return(1); /* success */
}
ckexist(s, t)
	char *s, *t;
{
char fnam[100];
strcpy (fnam, s);
strcat (fnam, t);
return (access(fnam, 04) != -1);
}
iopen (s, t)
	char *s, *t;
{
char fnam[100];
FILE *f;
strcpy (fnam, s);
strcat (fnam, t);
f = fopen (fnam, "r");
if (f == NULL)
	{
	err("Missing expected file %s", fnam);
	exit(1);
	}
return(f);
}
