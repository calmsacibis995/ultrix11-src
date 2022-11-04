
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)bcopy.c	3.0	4/22/86
 */
#include <sys/types.h>
/*
 * copy count bytes from from to to.
 */
bcopy(from, to, count)
register caddr_t from, to;
register count;
{
	if (count == 0)
		return;
	if(((int)from|(int)to|count)&1)
		do	/* copy by bytes */
			*to++ = *from++;
		while(--count);
	else {		/* copy by words */
		count >>= 1;
		do
			*((short *)to)++ = *((short *)from)++;
		while(--count);
	}
}
