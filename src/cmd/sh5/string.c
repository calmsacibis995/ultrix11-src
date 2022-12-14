
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)string.c	3.0	4/22/86
 *	(System V)  string.c  1.4
 */
/*
 * UNIX shell
 *
 * Bell Telephone Laboratories
 *
 */

#include	"defs.h"


/* ========	general purpose string handling ======== */


char *
movstr(a, b)
register char	*a, *b;
{
	while (*b++ = *a++);
	return(--b);
}

any(c, s)
register char	c;
char	*s;
{
	register char d;

	while (d = *s++)
	{
		if (d == c)
			return(TRUE);
	}
	return(FALSE);
}

cf(s1, s2)
register char *s1, *s2;
{
	while (*s1++ == *s2)
		if (*s2++ == 0)
			return(0);
	return(*--s1 - *s2);
}

length(as)
char	*as;
{
	register char	*s;

	if (s = as)
		while (*s++);
	return(s - as);
}

char *
movstrn(a, b, n)
	register char *a, *b;
	register int n;
{
	while ((n-- > 0) && *a)
		*b++ = *a++;

	return(b);
}
