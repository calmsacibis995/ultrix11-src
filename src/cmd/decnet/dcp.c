
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#ifndef lint
static char *sccsid = "@(#)dcp.c	3.0	(ULTRIX-11)	4/21/86";
#endif

/*
 * dcp
 *
 * This Ultrix-11 version of dcp should try to emulate as closely
 * as possible the functionality of dcp in Ultrix-32.  
 * 
 * Ultrix-11 dcp does not support wild cards or local-to-local
 * file transfers.  Transfers from a local file to a remote 
 * directory result in a file named STDIN in the specified directory,
 * since the remote dcp has no idea what the file name is.
 */

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/param.h>
#include <sys/dir.h>
#include <unistd.h>
#include <fcntl.h>

#define	BLEN	8192

int Aflag;
char	*rindex(), *sprintf();

main(ac, av)
	int ac;
	char **av;
{
	struct stat stb;
	int rc, i, argc;
	char **argv, *dummy[2], *flagp, flags[32];

	/*
	Create a fake arglist so that ps will not show such
	things as passwords, which decnet passes in clear text.
	 */
	argc = ac;
	argv = av;
	ac = 1;
	dummy[0] = argv[0];
	dummy[1] = 0;
	av = dummy;

	/*
	 * Process any switches.  Currently, only '-A' is enacted locally.
	 * In future other switches, such as '-v', should be 
	 * implemented.
	 */

	flagp = flags;
	argc--, argv++;
	while (argc > 0 && **argv == '-') {
		*flagp++ = '-';
		while (*(++(*argv))) switch (**argv) {
			case 'A':
				Aflag++;
			case 'i':
			case 'a':
			case 'P':
			case 'S':
			case 'v':
			case 'd':
				*flagp++ = **argv; /* append flags to flagp */
				break;
			default:
				goto usage;
		}
		argc--; argv++;
	}
	if (argc < 2) 
		goto usage;
	if (argc > 2) {
		if (((rc = stat(argv[argc-1], &stb)) < 0) || 
			((stb.st_mode&S_IFMT) != S_IFDIR)) {
/*
 * Having > 2 args and destination file not a directory means user wants
 * file1...fileN copied onto fileX .  Copy file1 onto fileX, then set Aflag
 * and copy the rest to fileX.  The result is the appended contents of file1 
 * through fileN in a file named fileX.  This is true whether or not fileX
 * previously existed.  However, if the '-A' flag was set by the user then 
 * the old contents of fileX will still be in the file, in front of the new 
 * stuff.  Contrary to the documentation, Ultrix-32 dcp allows the user to
 * use the '-A' switch even if fileX does not exist.  This version of dcp
 * will do the same in the name of compatibility.
 */
			rc = copy(*argv, argv[argc-1], flags);
			argc--; argv++;
			if (rc < 0)
				exit(rc);
			Aflag++;
			if (flags[0] == '\0') {
				flags[0] = '-';
				flags[1] = 'A';
			}
			else *flagp = 'A';
		}
		else if ((stb.st_mode&S_IFMT) != S_IFDIR)
			/* pre-existing--must be a directory */
			goto usage;			
	}
	rc = 0;
	for (i = 0; i < argc-1; i++)
		rc |= copy(argv[i], argv[argc-1], flags);
	exit(rc);
usage:
	fprintf(stderr,
		"Usage: dcp [-A][-P][-S][-v][-a][-d][-i] input output\n");
	exit(1);
}

copy(from, to, flags)
	char *from, *to, flags[32];
{
#include "dgate.h"

	char *last, buf[BLEN];
	char gate_way[64], gate_acct[64], *p1, *p2, *bp;
	char *remorig, *remdest;
	int rc;
	struct stat stfrom, stto;
	int output, input;

	char *arglist[80];
	char **argp;

	/* is target a directory? */
	if (stat(to, &stto) ==0 &&
	   (stto.st_mode&S_IFMT) == S_IFDIR) {
		p1 = from;
		p2 = to;
		bp = buf;
		while(*bp++ = *p2++)
			;
		bp[-1] = '/';
		p2 = bp;
		while(*bp = *p1++)
			if (*bp++ == '/')
				bp = p2;
		to = buf;
	}

	getgateway(gate_way, gate_acct);
	argp = arglist;
	*argp++ = RSH;
	*argp++ = gate_way;
	*argp++ = "-l";
	*argp++ = gate_acct;
	*argp++ = DCP;
	*argp++ = flags;

	remorig = remotename(from);
	remdest = remotename(to);

	if (remorig) {
		if (remdest) { /* remote to remote copy */
			*argp++ = from;
			*argp++ = to;
			*argp = 0;
			rshfork(stdin, stdout, arglist);
		}
		else {
			/*
			 * Remote to local copy:  set destination name to
			 * stdin, open the file locally, then fork and run 
			 * the cmd.
		 	 */
			*argp++ = from;
			*argp++ = "-";
			*argp = 0;
			if ((rc = strcmp(to, "-")) == 0)
				output = stdout;
			else 	{
				/* Check to see if destination file exists.
				 * If so, check if the append option was
				 * specified or if destination is a directory.
				 */

				if ((rc=stat(to, &stto)) < 0) {
					/* create new file */
					if (errno != ENOENT) {
						perror("dcp");
						exit(1);
						}
					setreuid(geteuid(), getuid());
					output = creat(to, 0666);
					setreuid(geteuid(), getuid());
					if (output < 0) {
						perror(to);
						exit(1);
					}
				} 
				else if (((stto.st_mode&S_IFMT) != 
						S_IFDIR) && (Aflag)) {
					/* file exists and user wants to 
					 * append to it.
					 */
					setreuid(geteuid(), getuid());
					output = open(to,O_WRONLY|O_APPEND);
					setreuid(geteuid(), getuid());
					if (output < 0) {
						perror(to);
						exit(1);
					}
				} 

			/* Check to see if file has write access.  */

				else 	{
					if ((rc = access(to, W_OK)) < 0) {
						perror(to);
						exit(1);
					}
					setreuid(geteuid(), getuid());
					output = open(to,O_WRONLY);
					setreuid(geteuid(), getuid());
					if (output < 0) {
						perror(to);
						exit(1);
					}
				}
			}
			rshfork(stdin, output, arglist);
		}
	}
	else if (remdest) {
			/*
			 * Local to remote copy:  set source name to stdin,
			 * open the file locally, then fork and run the cmd.
		 	 */
			*argp++ = "-";
			*argp++ = to;
			*argp = 0;
			if (strcmp(from, "-") != 0) {
				setreuid(geteuid(), getuid());
				input = open(from, 0);
				setreuid(geteuid(), getuid());
				if (input < 0) {
					perror(from);
					return(1);
				}
				rshfork(input, stdout, arglist);
				close(input);
			} else
				rshfork(stdin, stdout, arglist);
			return(0);
		}
		else {
			 /* local to local copy not allowed at present */
			fprintf(stderr, 
				"Local to local copies not supported.  Use cp.\n");
			exit(1);
		}

}


remotename(cp)
register char *cp;
{
	/*
	 * Determine whether a name is local or remote.  This
	 * is done by looking for a "::" in the path name, which
	 * should indicate a remote name.
	 */
	while (*cp)
		if (*cp++ == ':')
			if (*cp == ':')
				return(1);
	return(0);
}


rshfork(ifd, ofd, arglist)
int ifd, ofd; 
char **arglist;
{
	int pid, child, status, fail = 0;

	/*
	 * Fork off a sub-shell.  If the fork fails, sleep
	 * and then try again.  Try up to 5 times before
	 * we give up.
	 */
	while ((pid = fork()) < 0) {
		if (++fail > 5) {
			perror("fork");
			return;
		}
		sleep(2);
	}
	if (pid) {
		/* parent waits for the child */
		while ((child = wait(&status)) >= 0)
			if (child == pid)
				break;
		return;
	} else {
		/*
		 * Child dups ifd to stdin, ofd to
		 * stdout, and executes rsh command.
		 */
		if (ifd > 0) {
			dup2(ifd, 0);
			close(ifd);
		}
		if (ofd > 1) {
			dup2(ofd, 1);
			close(ofd);
		}
		execv(RSH, arglist);
		perror(RSH);
		exit(1);
	}
}
