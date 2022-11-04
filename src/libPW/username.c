
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[]="@(#)username.c 3.0 4/22/86";
/*
	Gets user's login name.
	Note that the argument must be an integer.
	Returns pointer to login name on success,
	pointer to string representation of used ID on failure.

	Remembers user ID and login name for subsequent calls.
*/

char *
username(uid)
register int uid;
{
	char pw[200];
	static int ouid;
	static char lnam[9], *lptr;
	register int i;

	if (ouid!=uid || ouid==0) {
		if (getpw(uid,pw))
			sprintf(lnam,"%d",uid);
		else {
			for (i=0; i<8; i++)
				if ((lnam[i] = pw[i])==':') break;
			lnam[i] = '\0';
		}
		lptr = lnam;
		ouid = uid;
	}
	return(lptr);
}
