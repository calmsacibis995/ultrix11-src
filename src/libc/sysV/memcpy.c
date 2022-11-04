
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*	SCCSID: @(#)memcpy.c	3.0	4/22/86	*/
/*	(System 5)	1.1	*/
/*LINTLIBRARY*/
/*
 * Copy s2 to s1, always copy n bytes.
 * Return s1
 */
char *
memcpy(s1, s2, n)
register char *s1, *s2;
register int n;
{
	register char *os1 = s1;

	while (--n >= 0)
		*s1++ = *s2++;
	return (os1);
}
