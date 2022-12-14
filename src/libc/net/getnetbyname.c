
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)getnetbyname.c	3.0	4/22/86
 * Base on	getnetbyname.c	4.2	82/10/05
 */

#include <netdb.h>

struct netent *
getnetbyname(name)
	register char *name;
{
	register struct netent *p;
	register char **cp;

	setnetent(0);
	while (p = getnetent()) {
		if (strcmp(p->n_name, name) == 0)
			break;
		for (cp = p->n_aliases; *cp != 0; cp++)
			if (strcmp(*cp, name) == 0)
				goto found;
	}
found:
	endnetent();
	return (p);
}
