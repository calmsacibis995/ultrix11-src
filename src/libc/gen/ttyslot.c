
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)ttyslot.c	3.0	4/22/86
 */
/*
 * Return the number of the slot in the utmp file
 * corresponding to the current user: try for file 0, 1, 2.
 * Definition is the line number in the /etc/ttys file.
 *
 * Lines in the /etc/ttys file that begin with a # are ignored,
 * and anything on a line after a tab or space is ignored.  Blank
 * lines are also ignored.
 * -Dave Borman, 10/18/85
 */


char	*ttyname();
char	*getttys();
char	*rindex();
static	char	ttys[]	= "/etc/ttys";

#define	NULL	0

ttyslot()
{
	register char *tp, *p;
	register s, tf;

	if ((tp=ttyname(0))==NULL && (tp=ttyname(1))==NULL && (tp=ttyname(2))==NULL)
		return(0);
	if ((p = rindex(tp, '/')) == NULL)
		p = tp;
	else
		p++;
	if ((tf=open(ttys, 0)) < 0)
		return(0);
	s = 0;
	while (tp = getttys(tf)) {
		if (*tp == '\0' || *tp == '#')	/* Skip blanks and comments */
			continue;
		s++;
		if (strcmp(p, tp)==0) {
			close(tf);
			return(s);
		}
	}
	close(tf);
	return(0);
}

static char *
getttys(f)
{
	static char line[32];
	register char *lp;

	for (lp = line; ; lp++) {
		if (read(f, lp, 1) != 1)
			return(NULL);
		if (*lp == '\n' || *lp == '\t' || *lp == ' ')
			break;
		if (lp >= &line[31])
			break;
	}
	/* zap the rest of the line */
	while (*lp != '\n') {
		if (read(f, lp, 1) != 1) {
			if (lp >= &line[31])	/* backwards compatability */
				break;
			return(NULL);
		}
	}
	*lp = '\0';
	return(line+2);
}
