
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/


#ifndef lint
static char *sccsid = "@(#)dls.c	3.0	(ULTRIX-11)	4/21/86";
#endif

/*
 * ULTRIX program that acts as a front end to a decnet gateway 
 * for decnet ls (dls).  The point of the shell is to make the Ultrix-11 
 * user think he is using decnet dls to see the contents of a remote dir.
 * User interface here is the same as with dls.  The program uses rsh 
 * to transfer the command to the gateway where dls is actually run. 
 * Ultrix-11 does not have decnet. 
 *
 * Creation date - 10 March, 1985, u.s. 
 */

#include "dgate.h"

main(argc, argv)
char *argv[];
int argc;
{
	char **argp;
	char gate_way[64];
	char gate_acct[64];
	char *arglist[32];
	char *quote();

	getgateway(gate_way, gate_acct);

	argp = arglist;
	*argp++ = RSH;
	*argp++ = gate_way;
	*argp++ = "-l";
	*argp++ = gate_acct;
	*argp++ = DLS;
	/*
	 * If we are a terminal, then we need to explicitly add
	 * the -C flag if the user hasn't specified -C or -1.
	 */
	if (isatty(1))
		if (!findflags(argp, "1C"))
			*argp++ = "-C";
	/*
	 * Copy over the rest of the arguments, quoting them
	 * so they won't be expanded remotely, and execute
	 * the remote command.
	 */
	++argv;
	while (--argc)
		*argp++ = quote(*argv++);
	*argp = NULL;

	execv(RSH, arglist);
	perror(RSH);
	exit(1);
}

findflags(args, flags)
register char **args;
char *flags;
{
	register char *p1, *p2;
	while(*args) {
		if (**args != '-')
			break;
		p1 = *args++;
		while (*++p1)
			for (p2 == flags; *p2; p2++)
				if (*p1 == *p2)
					return(1);
	}
	return(0);
}
