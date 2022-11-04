
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * who
 */

static char Sccsid[] = "@(#)who.c	3.0	4/22/86";
#include <stdio.h>
#include <utmp.h>
#include <pwd.h>
struct utmp utmp;
struct passwd *pw;
struct passwd *getpwuid();

char *ttyname(), *rindex(), *ctime(), *strcpy();
char obuf[BUFSIZ];
#define NMAX sizeof(utmp.ut_name)
#define LMAX sizeof(utmp.ut_line)
#define	HMAX sizeof(utmp.ut_host)

#ifdef	pdp11
#define	gethostname	ghostname
#endif	pdp11
char	hostname[32];

main(argc, argv)
char **argv;
{
	register char *tp, *s;
	register FILE *fi;

	setbuf(stdout, obuf);
	s = UTMP_FILE;
	if(argc == 2)
		s = argv[1];
	if (argc==3) {
		tp = ttyname(0);
		if (tp)
			tp = rindex(tp, '/') + 1;
		else {	/* no tty - use best guess from passwd file */
			pw = getpwuid(getuid());
			strncpy(utmp.ut_name, pw ? pw->pw_name : "?", NMAX);
			strcpy(utmp.ut_line, "tty??");
			time(&utmp.ut_time);
			putline();
			exit(0);
		}
	}
	if (argc==4){
		printf("%s\n", getpwuid(getuid())->pw_name);
		exit(0);
	}
	if ((fi = fopen(s, "r")) == NULL) {
		printf("who: cannot open %s\n",s);
		exit(1);
	}
	while (fread((char *)&utmp, sizeof(utmp), 1, fi) == 1) {
		if(argc==3) {
			gethostname(hostname, sizeof (hostname));
			if (strcmp(utmp.ut_line, tp))
				continue;
			printf("%s!", hostname);
			putline();
			exit(0);
		}
		if(utmp.ut_name[0] == '\0' && argc==1)
			continue;
		putline();
	}
}

putline()
{
	register char *cbuf;

	printf("%-*.*s %-*.*s",
		NMAX, NMAX, utmp.ut_name,
		LMAX, LMAX, utmp.ut_line);
	cbuf = ctime(&utmp.ut_time);
	printf("%.12s", cbuf+4);
	if (utmp.ut_host[0])
		printf("\t(%.*s)", HMAX, utmp.ut_host);
	putchar('\n');
}
