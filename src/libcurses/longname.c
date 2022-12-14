
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/* SCCSID: @(#)longname.c	3.0	4/22/86 */
# define	reg	register

/*
 *	This routine fills in "def" with the long name of the terminal.
 *
 * 1/26/81 (Berkeley) @(#)longname.c	1.1
 */
char *
longname(bp, def)
reg char	*bp, *def; {

	reg char	*cp;

	while (*bp && *bp != ':' && *bp != '|')
		bp++;
	if (*bp == '|') {
		bp++;
		cp = bp;
		while (*cp && *cp != ':' && *cp != '|')
			cp++;
		*cp = 0;
		return bp;
	}
	return def;
}

