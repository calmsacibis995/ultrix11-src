
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/


#ifndef lint
static char *sccsid = "@(#)drm.c	3.0	drm gateway	4/21/86";
#endif

/*
 * ULTRIX program that acts as a front end to a decnet gateway 
 * for decnet rm (drm).  User interface here is the same as with
 * drm.  The program uses rsh to transfer the command to the
 * gateway where drm is actually run. Ultrix-11 does not have decnet. 
 */

#include "dgate.h"

main(argc, argv)
char *argv[];
int argc;
{
	char **argp;
	char gate_way[64];
	char gate_accnt[64];
	char *arglist[32];
	char *quote();

	getgateway(gate_way, gate_accnt);

	argp = arglist;
	*argp++ = RSH;
	*argp++ = gate_way;
	*argp++ = "-l";
	*argp++ = gate_accnt;
	*argp++ = DRM;
	++argv;
	while (--argc)
		*argp++ = quote(*argv++);
	*argp = NULL;

	execv(RSH, arglist);
	perror(RSH);
	exit(1);
}
