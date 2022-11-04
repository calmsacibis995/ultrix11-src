
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)open.c	3.0	4/22/86
 */
#include <stdio.h>
FILE	*ggi;
int xnow;
int ynow;
int multi;
float boty;
float botx;
float oboty;
float obotx;
float scalex;
float scaley;
opengg ()
{
	if((ggi = freopen("/dev/gigi","w", stdout)) == NULL)
	{
		printf("could not open /dev/gigi\n");
		exit(1);
	}
	boty = 0.;
	botx = 0.;
	oboty = 0.;
	obotx = 0.;
	scalex = 1.;
	scaley = 1.;
	fprintf(ggi,"\033Pp\n");
	wrtng("int,white noblink noshade nonegat pat,1 replace multi,1 ");
	return;
}
openpl()
{
	ggi = stdout;
	boty = 0.;
	botx = 0.;
	oboty = 0.;
	obotx = 0.;
	scalex = 1.;
	scaley = 1.;
	fprintf(ggi,"\033Pp\n");
	wrtng("int,white noblink noshade nonegat pat,1 replace multi,1 ");
	return;
}
