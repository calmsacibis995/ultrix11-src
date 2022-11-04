
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * csf command now obsolete.
 * Use msf command instead.
 *
 * Fred Canter 7/21/85
 */
static char Sccsid[] = "@(#)csf.c 3.0 4/21/86";

#include <stdio.h>

main()
{
	fprintf(stderr, "\ncsf obsolete, new command is /etc/msf!\n");
	system("/etc/msf -h");
	exit(1);
}
