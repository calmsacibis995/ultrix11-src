
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 *	SCCSID: @(#)exit.c	3.0	4/22/86
 */
exit(c)
	int c;
{

/*
	if (fork() == 0) {
		char *cp = "-00";
		if (c > 10) {
			cp[1] |= (c / 10) % 10;
			cp[2] |= c % 10;
		} else {
			cp[1] |= c;
			cp[2] = 0;
		}
		execl("/usr/lib/gather", "gather", cp, "px", 0);
	}
*/
	_exit(c);
}
