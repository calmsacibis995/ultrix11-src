
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * chsh
 */
static char Sccsid[] = "@(#)chsh.c	3.0	4/22/86";
#include <stdio.h>
#include <signal.h>
#include <pwd.h>

char	passwd[] = "/etc/passwd";
char	temp[]	 = "/etc/ptmp";
struct	passwd *pwd;
struct	passwd *getpwent();
int	endpwent();
char	*crypt();
char	*getpass();
char	buf[BUFSIZ];

main(argc, argv)
char *argv[];
{
	int u,fi,fo;
	FILE *tf;
	int found = 0;

	if(argc < 2 || argc > 3) {
		printf("Usage: chsh user\n");
		printf("       chsh user /bin/csh\n");
		printf("       chsh user /bin/sh5\n");
		goto bex;
	}
	if (argc == 2)
		argv[2] = "";
	else if (getuid() && ! (! strcmp(argv[2], "/bin/csh")
			     || (! strcmp(argv[2], "/bin/sh5")))) {
		printf("Only /bin/csh or /bin/sh5 may be specified.\n");
		exit(1);
	}
	if (argv[2][0] && access(argv[2],1)){
		perror(argv[2]);
		goto out;
	}
	while((pwd=getpwent()) != NULL){
		if(strcmp(pwd->pw_name,argv[1]) == 0){
			u = getuid();
			if(u!=0 && u != pwd->pw_uid){
				printf("Permission denied.\n");
				goto bex;
				}
			found++;
			break;
			}
		}
	endpwent();
	if (found == 0) {
		printf("Unknown user: %s\n", argv[1]);
		goto bex;
	}
	signal(SIGHUP, 1);
	signal(SIGINT, 1);
	signal(SIGQUIT, 1);
#ifdef SIGTSTP
	signal(SIGTSTP, 1);
#endif

	if(access(temp, 0) >= 0) {
		printf("Temporary file busy -- try again\n");
		goto bex;
	}
	if((tf=fopen(temp,"w")) == NULL) {
		printf("Cannot create temporary file\n");
		goto bex;
	}

/*
 *	copy passwd to temp, replacing matching lines
 *	with new shell.
 */

	while((pwd=getpwent()) != NULL) {
		if(strcmp(pwd->pw_name,argv[1]) == 0) {
			u = getuid();
			if(u != 0 && u != pwd->pw_uid) {
				printf("Permission denied.\n");
				goto out;
			}
			pwd->pw_shell = argv[2];
		}
		fprintf(tf,"%s:%s:%d:%d:%s:%s:%s\n",
			pwd->pw_name,
			pwd->pw_passwd,
			pwd->pw_uid,
			pwd->pw_gid,
			pwd->pw_gecos,
			pwd->pw_dir,
			pwd->pw_shell);
	}
	endpwent();
	fclose(tf);

/*
 *	copy temp back to passwd file
 */

	if((fi=open(temp,0)) < 0) {
		printf("Temp file disappeared!\n");
		goto out;
	}
	if((fo=creat(passwd, 0644)) < 0) {
		printf("Cannot recreate passwd file.\n");
		goto out;
	}
	while((u=read(fi,buf,sizeof(buf))) > 0) write(fo,buf,u);

out:
	unlink(temp);

bex:
	exit(1);
}
