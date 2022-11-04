
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*	SCCSID: @(#)llib.c	3.0	4/22/86	*/

/* LINTLIBRARY */

Ignore(a)
int a;
{
	a=a;
}

Ignors(s)
char *s;
{
	s=s;
}

Forget(s)
char *s;
{
	Ignors(s);
}
#include <stdio.h>

/* VARARGS */
FILE *popenp(a,b,c)
char *a, *b, *c;
{
	a=a;b=b;if (c==NULL) printf("have a nice day!\n");
	return(stdin);
}
