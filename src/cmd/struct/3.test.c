
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)3.test.c	3.0	4/22/86";
#include <stdio.h>
#
/* for testing only */
#include "def.h"

testreach()
	{
	VERT v;
	for (v = 0; v < nodenum; ++v)
		fprintf(stderr,"REACH(%d) = %d\n",v,REACH(v));
	}
