
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * ln [ -f ] target [ new name ]
 */

static char Sccsid[] = "@(#)ln.c 3.1 9/15/87";
#include <sys/localopts.h>	/* #define UCB_SYMLINKS */
#include <sys/types.h>
#include <sys/stat.h>
#include "stdio.h"
char	*rindex();
int fflag;
int sflag;

int	link();
#ifdef	UCB_SYMLINKS
int	symlink();
#endif

main(argc, argv)
char **argv;
{
	struct stat statb;
	register char *np;
	char nb[100], *name=nb, *arg2;
	int statres;
#ifdef	UCB_SYMLINKS
	int i;

	for (i=0; i< 2; i++)
	{
		if (argc >1) {
			if (strcmp(argv[1], "-f")==0)
				fflag++;
			else if (strcmp(argv[1], "-s")==0)
				sflag++;
			else
				break;
			argc--;
			argv++;
		}
		else
			break;
	}
#else	UCB_SYMLINKS
	if (argc >1 && strcmp(argv[1], "-f")==0) {
		argc--;
		argv++;
		fflag++;
	}
#endif	UCB_SYMLINKS

	if (argc<2 || argc>3) {
#ifdef	UCB_SYMLINKS
		printf("Usage: ln [ -s ] [ -f ] target [ newname ]\n");
#else	UCB_SYMLINKS
		printf("Usage: ln [ -f ] target [ newname ]\n");
#endif	UCB_SYMLINKS
		exit(1);
	}
	np = rindex(argv[1], '/');
	if (np==0)
		np = argv[1];
	else
		np++;
	if (argc==2)
		arg2 = np;
	else
		arg2 = argv[2];
	statres = stat(argv[1], &statb);
	if (statres<0) {
		printf ("ln: %s does not exist\n", argv[1]);
		exit(1);
	}
	if (sflag==0 && fflag==0 && (statb.st_mode&S_IFMT) == S_IFDIR) {
		printf("ln: %s is a directory\n", argv[1]);
		exit(1);
	}
	statres = stat(arg2, &statb);
	if (statres>=0 && (statb.st_mode&S_IFMT) == S_IFDIR)
		sprintf(name, "%s/%s", arg2, np);
	else
		name = arg2;
#ifdef	UCB_SYMLINKS
	if (linkit(argv[1], name)<0) {
#else	UCB_SYMLINKS
	if (link(argv[1], name)<0) {
#endif	UCB_SYMLINKS
		perror("ln");
		exit(1);
	}
	exit(0);
}
#ifdef	UCB_SYMLINKS
linkit(from, to)
char	*from,	*to;
{
	int (*linkf)() = sflag ? symlink : link;

	return((*linkf)(from, to));
}
#endif	UCB_SYMLINKS
