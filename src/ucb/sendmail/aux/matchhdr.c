
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

# include <stdio.h>
# include <ctype.h>
# include "useful.h"

SCCSID(@(#)matchhdr.c	4.1		7/25/83);

/*
**  MATCHHDR -- Match header line
**
**	Matches a header line in arpanet format (case and white
**	space is ignored).
**
**	This routine is used by arpa-mailer and sendmail.
**
**	Parameters:
**		line -- the line to match against.
**		pat -- the pattern to match against; must be in
**			lower case.
**
**	Returns:
**		address of the 'value' of the pattern (the beginning
**			of the non-white string following the delim).
**		NULL if none found.
**
**	Side Effects:
**		none
**
**	Called By:
**		maketemp
**		sendmail [arpa.c]
**
**	Deficiencies:
**		It doesn't handle folded lines.
*/

char *
matchhdr(line, pat)
	char *line;
	char *pat;
{
	register char *p;
	register char *q;

	for (q = pat, p = line; *q != '\0'; p++, q++)
		if (lowercase(*p) != *q)
			return (NULL);
	while (isspace(*p))
		p++;
	if (*p != ':')
		return (NULL);
	while (isspace(*++p))
		continue;
	return (*p == '\0' ? NULL : p);
}
/*
**  LOWERCASE -- Convert a character to lower case
**
**	If the argument is an upper case letter, it is converted
**	to a lower case letter, otherwise it is passed through
**	unchanged.
**
**	Parameters:
**		c -- the character to check.
**
**	Returns:
**		c converted to lower case.
**
**	Side Effects:
**		none
**
**	Called By:
**		matchhdr
*/

lowercase(c)
	register char c;
{
	if (isupper(c))
		c -= 'A' - 'a';
	return (c);
}
