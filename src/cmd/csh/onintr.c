
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/* SCCSID: @(#)onintr.c	3.0	4/21/86	*/
#include "sh.h"
#include <sys/ioctl.h>

doonintr(v)
	char **v;
{
	register char *cp;
	register char *vv = v[1];

	if (parintr == SIG_IGN)
		return;
	if (setintr && intty)
		bferr("Can't from terminal");
	cp = gointr, gointr = 0, xfree(cp);
	if (vv == 0) {
		if (setintr)
			sighold(SIGINT);
		else
			sigset(SIGINT, SIG_DFL);
		gointr = 0;
	} else if (eq((vv = strip(vv)), "-")) {
		sigset(SIGINT, SIG_IGN);
		gointr = "-";
	} else {
		gointr = savestr(vv);
		sigset(SIGINT, pintr);
	}
}

