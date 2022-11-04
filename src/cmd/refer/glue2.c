
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)glue2.c	3.0	4/22/86";
 char refdir[50];
savedir()
{
if (refdir[0]==0)
	corout ("", refdir, "/bin/pwd", "", 50);
trimnl(refdir);
}
restodir()
{
chdir(refdir);
}
