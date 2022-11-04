
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)perror.c	3.0	4/22/86
 */
/*
 * Print the error indicated
 * in the cerror cell.
 */

int	errno;
int	sys_nerr;
char	*sys_errlist[];
perror(s)
char *s;
{
	register char *c;
	register n;

	c = "Unknown error";
	if(errno < sys_nerr)
		c = sys_errlist[errno];
	if (s != 0) {
		n = strlen(s);
		if(n) {
			write(2, s, n);
			write(2, ": ", 2);
		}
		write(2, c, strlen(c));
		write(2, "\n", 1);
	}
	return(c);
}
