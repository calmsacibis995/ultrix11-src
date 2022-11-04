
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/


#ifndef lint
static char *sccsid = "@(#)dcat.c	3.0	dcat gateway	4/21/86";
#endif

/*
 * ULTRIX program that acts as a front end to a decnet gateway
 * for decnet cat (dcat).  The point of the shell is to make the Ultrix-11
 * user think he is using decnet dcat to see the contents of a remote file.
 * User interface here is the same as with dcat.  The program uses rsh
 * to transfer the command to the gateway where dcat is actually run.
 * Ultrix-11 does not have decnet.
 *
 * Creation date - 10 March, 1985, u.s. 
 */

#include <stdio.h>
#include <pwd.h>
#include "dgate.h"

main(argc, argv)
register char **argv;
register int argc;
{
	register char **argp;
	char gate_way[32];
	char gate_accnt[32];
	char *arglist[32];
	char *quote();

	getgateway(gate_way, gate_accnt);

	argp = arglist;
	*argp++ = RSH;
	*argp++ = gate_way;
	*argp++ = "-l";
	*argp++ = gate_accnt;
	*argp++ = DCAT;
	argv++;
	while (--argc)
		*argp++ = quote(*argv++);
	execv(RSH, arglist);
	perror(RSH);
	exit(1);
}
