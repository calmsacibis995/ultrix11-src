
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#ifdef KEEPSCCSID
static char Sccsid[] = "@(#)pdp11x.c	3.0	4/21/86";
#endif
#include <stdio.h>
#include "defines"
#include "machdefs"



prchars(fp, s)
FILEP fp;
register int *s;
{
register int i;

fprintf(fp, ".byte ");
for(i = 0; i<SZSHORT ; ++i)
	fprintf(fp, "%o%c", *s++, (i==SZSHORT-1 ? '\n' : ',' ) );
}





pruse(fp, s)
FILEP fp;
char *s;
{
fprintf(fp, "%s\n", s);
}





prskip(fp, k)
FILEP fp;
ftnint k;
{
/* works around bug in the pdp11 assembler that bombs on  . = .+x  for x>= 32768 */
for( ; k > 30000 ; k -= 30000)
	fprintf(fp, "\t. = .+30000.\n");
fprintf(fp, "\t. = .+%ld.\n", k);
}


prcomblock(fp, name)
FILEP fp;
char *name;
{
fprintf(fp, LABELFMT, name);
}


