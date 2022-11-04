
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)offsets.c	3.0	4/22/86
 */
#include <stdio.h>
extern ggi;

offsets(dir, x) 	/* offset display in dir direction  x pixels */
{						/*  3  2  1 */
	int cnt;				/*   \ | /  */
						/*  4-----0 */
	for(cnt = 1; cnt <= x; cnt++)		/*   / | \  */
		fprintf(ggi, "s%d\n", dir);	/*  5  6  7 */
}
