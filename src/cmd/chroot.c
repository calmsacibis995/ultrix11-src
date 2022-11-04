
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * Chroot - CHange ROOT directory
 *
 * chroot newdir
 */

static char Sccsid[] = "@(#)chroot.c	3.0	4/21/86";

#include <stdio.h>

char *newroot = ".";
char prompt[] = "PS1=(subroot)-> ";
char *shell;
char **newenv;

main(argc, argv, envp)
int argc;
char *argv[], *envp[];
{
	char *sbrk();

	if (argc > 2) {
		fprintf(stderr, "Usage: chroot [directory]\n");
		exit(1);
	} else if (argc == 2)
		newroot = *++argv;
	if ((chdir(newroot) == -1)
	    || (chroot((*newroot == '/') ? newroot : ".") == -1)) {
		perror(newroot);
		exit(1);
	}
	newenv = envp;
	while (*envp) {
		if (strncmp(*envp, "PS1=", 4) == 0) {
			*envp = prompt;
			break;
		}
		++envp;
	}
	if (!*envp) {	/* PS1 wasn't in the enviornment, need to add it */
		char **tp, **i = envp - newenv + 2;
		envp = newenv;
		tp = (char **)sbrk((int)i * sizeof(char **));
		if (tp > 0) { /* we have the space */
			newenv = tp;
			*tp++ = prompt;
			while (*tp++ = *envp++)
				;
		} else
			fprintf(stderr,"Couldn't change prompt...\n");
	}
	if ((shell = getenv("SHELL")) == NULL)
		shell = "/bin/sh";
	execle(shell, "-sh", 0, newenv);
	perror(shell);
	exit(1);
}
