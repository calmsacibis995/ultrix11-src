
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)su.c	3.0	4/22/86";
#include <stdio.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define	DFLT_SHELL	"/bin/sh"
#define	PATH	"PATH=:/usr/ucb:/bin:/usr/bin:"		/* default path */
#define	SULOGFILE	"/usr/adm/sulog"

struct	passwd *pwd,*getpwnam(),*getpwuid();
char	*crypt();
char	*getpass();
char	*getenv();
char	**environ;
int	ruid, rgid;
char	homedir[64] = "HOME=";
char	term[64] = "TERM=unknown";
char	shell[64] = "SHELL=";
char	user[64] = "USER=";

char	*envinit[8] = {homedir, PATH, term, shell, user, 0, 0};
#define	E_TERMCAP	5		/* where to put termcap in envinit */

main(argc,argv)
int	argc;
char	**argv;
{
	register char **p;
	char *nptr;
	char oname[14];	/* orig name */
	char *password;
	register char *cp;
	register char *cp2;
	char hometmp[50];
	char *sh = DFLT_SHELL;
	int minusflag = 0;
	int new_uid, new_gid;

	if (argc > 1 && argv[1][0]=='-') {
		minusflag++;
		argc--;
		argv++;
	}
	ruid = getuid();
	rgid = getgid();
	nptr=getpwuid(ruid);
	strcpy(oname,nptr->pw_name);
	if(argc > 1)
		nptr = argv[1];
	else
		nptr = "root";
	if((pwd=getpwnam(nptr)) == NULL) {
		printf("Unknown id: %s\n",nptr);
		exit(1);
	}
	if(pwd->pw_passwd[0] == '\0' || ruid == 0
	    || (ruid == 1 && strcmp(nptr, "uucp") == 0))
		goto ok;
	password = getpass("Password:");
	if(strcmp(pwd->pw_passwd, crypt(password, pwd->pw_passwd)) != 0) {
		if (pwd->pw_uid == 0)
			sulog(oname, nptr, password);
		printf("Sorry\n");
		exit(2);
	}

ok:
	endpwent();

	/*
	 * ``All information is contained in a static area so it must  be
	 * copied if it is to be saved.'' - getpwent(3)
	 */
	new_uid = pwd->pw_uid;
	new_gid = pwd->pw_gid;
	     
	if (pwd->pw_uid == 0)
		sulog(oname, nptr, (char *) NULL);
	setgid(new_gid);
	setuid(new_uid);
	if (minusflag) {
		if (pwd->pw_shell && *pwd->pw_shell)
			sh = pwd->pw_shell;
		strncat(homedir, pwd->pw_dir, sizeof(homedir)-6);
		strncat(shell, sh, sizeof(shell)-7);
		strncat(user, pwd->pw_name, sizeof(user)-5);
		if ((cp=getenv("TERM")) != NULL)
			strncpy(term+5, cp, sizeof(term)-5);
		for (p=environ; *p; p++)
			if (strncmp(*p, "TERMCAP=", 8) == 0) {
				envinit[E_TERMCAP] = *p;
				break;
			}
		if (chdir(pwd->pw_dir) < 0)
			perror(pwd->pw_dir);
		environ = envinit;
	} else {
		if ((cp=getenv("SHELL")) != NULL)
		    sh = cp;
	}
	if(strcmp(sh,"/bin/csh") != 0)
		execl(sh, "su", 0);
	else
		execl(sh, "_su", 0);
	printf("No shell\n");
	exit(3);
}

sulog(whofrom, whoto, password)
	register char *whofrom, *whoto, *password;
{
	register FILE	*logf;
	int	i;
	long	now;
	char	*ttyn, *ttyname();
	struct	stat statb;
	
	if (stat(SULOGFILE, &statb) < 0)
		return;
	if ((logf = fopen (SULOGFILE, "a")) == NULL)
		return;
	
	for (i = 0; i < 3; i++)
		if ((ttyn = ttyname(i)) != NULL)
			break;
	time (&now);
	fprintf (logf, "%24.24s  %-8.8s  %-8.8s-> %-8.8s  ",
			ctime(&now), ttyn+5, whofrom, whoto);
	if (password == (char *) 0)
		fprintf(logf, "OK\n");
	else
		fprintf(logf, "FAILED\n");
	fclose (logf);
}
