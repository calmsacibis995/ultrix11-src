
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*	SCCSID: @(#)memccpy.c	3.0	4/22/86	*/
/*	(System 5)	1.1	*/
/*LINTLIBRARY*/
/*
 * Copy s2 to s1, stopping if character c is copied. Copy no more than n bytes.
 * Return a pointer to the byte after character c in the copy,
 * or NULL if c is not found in the first n bytes.
 */
char *
memccpy(s1, s2, c, n)
register char *s1, *s2;
register int c, n;
{
	while (--n >= 0)
		if ((*s1++ = *s2++) == c)
			return (s1);
	return (0);
}
