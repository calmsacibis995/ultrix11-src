
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*	SCCSID: @(#)atoi.c	3.0	4/22/86	*/
/*	(System 5)	2.1	*/
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
#include <ctype.h>

#define ATOI

#ifdef	ATOI
typedef int TYPE;
#define NAME	atoi
#else
typedef long TYPE;
#define NAME	atol
#endif

TYPE
NAME(p)
register char *p;
{
	register TYPE n;
	register int c, neg = 0;

	if (!isdigit(c = *p)) {
		while (isspace(c))
			c = *++p;
		switch (c) {
		case '-':
			neg++;
		case '+': /* fall-through */
			c = *++p;
		}
		if (!isdigit(c))
			return (0);
	}
	for (n = '0' - c; isdigit(c = *++p); ) {
		n *= 10; /* two steps to avoid unnecessary overflow */
		n += '0' - c; /* accum neg to avoid surprises at MAX */
	}
	return (neg ? n : -n);
}
