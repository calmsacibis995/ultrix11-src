
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)ftime.c	3.0	4/22/86
 */
#include <sys/types.h>
#include <sys/timeb.h>

static struct timeb gorp = {
	0L,
	0,
	5*60,
	1
};

ftime(gorpp)
struct timeb *gorpp;
{
	*gorpp = gorp;
	return(0);
}
