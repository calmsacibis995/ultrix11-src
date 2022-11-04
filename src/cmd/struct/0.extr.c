
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)0.extr.c	3.0	4/22/86";
#include <stdio.h>
#include "def.h"
struct lablist	{long labelt;  struct lablist *nxtlab; };
struct lablist *endlab, *errlab, *reflab, *linelabs, *newlab;

int nameline;			/* line number of function/subroutine st., if any */
int stflag;		/* determines whether at beginning or middle of block of straight line code */



int   nlabs, lswnum, swptr, flag,
	 counter, p1, p3, begline, endline, r1,r2, endcom;
long begchar, endchar, comchar;


char *pred, *inc, *prerw, *postrw, *exp, *stcode;

#define maxdo	20	/* max nesting of do loops */
long dostack[maxdo];		/* labels of do nodes */
int doloc[maxdo];		/* loc of do node */
int doptr;


struct list *FMTLST;		/* list of FMTVX's generated */
struct list *ENTLST;		/* list of STLNVX nodes corresponding to entry statements */
long rtnbeg;	/* number of chars up to beginning of current routine */
