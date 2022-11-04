
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)3.main.c	3.0	4/22/86";
#include <stdio.h>
#include "def.h"

structure()
	{
	VERT v, *head;

	if (progress)
		fprintf(stderr,"	getreach:\n");
	getreach();
	if (routerr) return;
	if (progress)
		fprintf(stderr,"	getflow:\n");
	getflow();
	if (progress)
		fprintf(stderr,"	getthen:\n");
	getthen(START);
	head = challoc(nodenum * sizeof(*head));
	for (v = 0; v < nodenum; ++v)
		head[v] = UNDEFINED;
	for (v = START; DEFINED(v); v = RSIB(v))
		fixhd(v,UNDEFINED,head);
			/* fixhd must be called before getloop so that
				it gets applied to IFVX which becomes NXT(w) for UNTVX w */
	if (progress)
		fprintf(stderr,"	getloop:\n");
	getloop();
	if (progress)
		fprintf(stderr,"	getbranch:\n");
	getbranch(head);
	chfree(head,nodenum * sizeof(*head));
	head = 0;
	}
