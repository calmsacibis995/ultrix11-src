
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)rewind.c	3.0	4/22/86
 */
#include "fio.h"
f_rew(a) alist *a;
{
	unit *b;
	if(a->aunit>=MXUNIT || a->aunit<0) err(a->aerr,101,"rewind");
	b = &units[a->aunit];
	if(b->ufd == NULL) return(0);
	if(!b->useek) err(a->aerr,106,"rewind")
	if(b->uwrt)
	{	nowreading(b);
		t_runc(b);
	}
	rewind(b->ufd);
	b->uend=0;
	return(0);
}
