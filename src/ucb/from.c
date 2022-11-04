
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#ifndef lint
static char *sccsid = "@(#)from.c	3.0	4/22/86";
#endif

/*
 * from.c
 *
 * static char *sccsid = "@(#)from.c	4.1 (Berkeley) 10/1/80";
 *
 *	5-Mar-84	mah.  Removed the option of looking at someone
 *			else's mailbox.  Added the option of using from
 *			on a specified mailbox.
 *
 */

#include <stdio.h>
#include <ctype.h>
#include <pwd.h>

struct	passwd *getpwuid();

main(argc, argv)
int argc;
register char **argv;
{
	char lbuf[BUFSIZ];
	char lbuf2[BUFSIZ];
	register struct passwd *pp;
	int stashed = 0,fflag=0;
	register char *name;
	char *sender;
	char *getlogin(),*getenv();
	char mbox[100];

	sender = NULL;		/*setup default sender*/
	while (argc > 1 && (*++argv)[0] == '-') {
		argc--;
		switch((*argv)[1]) {
			case 's':
				argc--;
				sender = *++argv;
				for (name = sender; *name; name++)
					if (isupper(*name))
						*name = tolower(*name);
				break;
			case 'f':
				fflag++;
				if(((*++argv)[0] == '-') || argc == 1) {
					--argv;
					name = getenv("HOME");
					strcpy(mbox,name);
					strcat(mbox,"/mbox");
					if(freopen(mbox,"r",stdin) == NULL) {
						fprintf(stderr,"Open error on mbox\n");
						exit(0);
					}
					break;
				}
				argc--;
				if(freopen(*argv,"r",stdin) == NULL) {
					fprintf(stderr,"Open error on mailbox\n");
					exit(0);
				}
				break;
			default:
				fprintf(stderr,"Usage: from [-f [mailbox]] [-s sender]\n");
				exit (1);
		}

	}
	if (argc > 1) {
		fprintf(stderr,"Usage: from [-f [mailbox]] [-s sender] \n");
		exit(0);
	}
	if (!fflag) {			/*if not given mbox calc spool mbox*/
		if (chdir("/usr/spool/mail") < 0)
			exit(1);
		name = getlogin ();
		if (name == NULL || strlen(name) == 0) {
			pp = getpwuid(getuid());
			if (pp == NULL) {
				fprintf(stderr, "Who are you?\n");
				exit(1);
			}
			name = pp->pw_name;
		}
	if (freopen(name, "r", stdin) == NULL)
		exit(0);
	}
	while(fgets(lbuf, sizeof lbuf, stdin) != NULL)
		if (lbuf[0] == '\n' && stashed) {
			stashed = 0;
			printf("%s", lbuf2);
		}
		else if (bufcmp(lbuf, "From ", 5) &&
		    (sender == NULL || match(&lbuf[4], sender))) {
			strcpy(lbuf2, lbuf);
			stashed = 1;
		}
	if (stashed)
		printf("%s", lbuf2);
	exit(0);
}

bufcmp (b1, b2, n)
register char *b1, *b2;
register int n;
{
	while (n-- > 0)
		if (*b1++ != *b2++)
			return (0);
	return (1);
}

match (line, str)
register char *line, *str;
{
	register char ch;

	while (*line == ' ' || *line == '\t')
		++line;
	if (*line == '\n')
		return (0);
	while (*str && *line != ' ' && *line != '\t' && *line != '\n') {
		ch = isupper(*line) ? tolower(*line) : *line;
		if (ch != *str++)
			return (0);
		line++;
	}
	return (*str == '\0');
}
