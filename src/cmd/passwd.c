
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * Enter a password in the password file,
 * or a password in the group file, using '-g'.
 * This program should be suid with owner
 * with an owner with write permission on /etc/passwd
 */
static char Sccsid[] = "@(#)passwd.c	3.0	4/22/86";
#include <stdio.h>
#include <signal.h>
#include <pwd.h>
#include <grp.h>

char	passwd[32]; /* holds one of the next 2 */
char	ppasswd[] = "/etc/passwd";
char	gpasswd[] = "/etc/group";
char	temp[32];  /* holds one of the next 2 */
char	ptemp[]	 = "/etc/ptmp";
char	gtemp[]	 = "/etc/gtmp";
/* group file changes use "/etc/gtmp" for lockfile */
struct	passwd	*pwd;
struct	passwd	*getpwent();
struct	group	*grp;
struct	group	*getgrent();
int	endpwent();
int	endgrent();
char	*strcpy();
char	*crypt();
char	*getpass();
char	*getlogin();
char	usage[] = "Usage: passwd user\n       passwd -g group\n";
char	*pw;
char	pwbuf[10];
char	buf[512];
int	gfile = 0;  /* = 1, entered '-g' option */
int	u,fi,fo;
int	ok, flags;
char	saltc[2];
long	salt;
FILE	*tf;
char	*uname;
int	pwlen;
char	*p;
int	i;
int	insist;
int	c;
int	retval;

main(argc, argv)
char *argv[];
{
	insist = 0;
	if(argc < 2) {
		if ((uname = getlogin()) == NULL) {
			printf(usage);
			goto bex;
		} else {
			printf("Changing password for %s\n", uname);
		}
	} else {
		/*
		 * Got >= 2 args, check if '-g'
		 * option was entered.
		 */
		if (strcmp(argv[1], "-g") == 0) {
			gfile = 1;
			if (argc != 3) {
				printf(usage);
				goto bex;
			}
			else {	/* OK, found third arg */
				uname = argv[2];
			}
		/*
		 * Second argument was not '-g',
		 * so it must be a login name.
		 */
		} else {
			uname = argv[1];
		}
	}
	if (gfile) {	/* /etc/group file */
	    if (u = getuid()) {
		printf("passwd: must be superuser for '-g' option.\n");
		goto bex;
	    }
	    while (((grp=getgrent()) != NULL)&&(strcmp(grp->gr_name,uname)!=0))
		;  /* search for the name */

	    if (grp==NULL) {	/* no matching name was found */
		    printf("passwd: %s not in %s.\n", uname, gpasswd);
		    goto bex;
	    }
	    endgrent();
	}	/* end gfile stuff */

	else {	  /* /etc/passwd file */
	    while (((pwd=getpwent()) != NULL)&&(strcmp(pwd->pw_name,uname)!=0))
		;	/* get the name */
	    u = getuid();
	    if((pwd==NULL) || (u!=0 && u != pwd->pw_uid)) {
		printf("Permission denied.\n");
		goto bex;
	    }
	    endpwent();
	    if (pwd->pw_passwd[0] && u != 0) {
		strcpy(pwbuf, getpass("Old password:"));
		pw = crypt(pwbuf, pwd->pw_passwd);
		if(strcmp(pw, pwd->pw_passwd) != 0) {
		    printf("Sorry.\n");
		    goto bex;
		}
	    }
	}  /* end NOT gfile */

	for (;;) {   /* forever until error or correct... */
	    /*
	     * NOTE: pwbuf and pwlen are used
	     * interchangeably for both /etc/passwd
	     * and /etc/group.  When there is a difference,
	     * the specific routines know about passwd
	     * files and group files by using "if (gfile)..."
	     */
	    strcpy(pwbuf, getpass("New password:"));
	    pwlen = strlen(pwbuf);
	    if (pwlen == 0) {
		printf("Password unchanged.\n");
		goto bex;
	    }
	    if ((retval = checkpw()) < 0)
		goto bex;
	    else if (retval == 1)
		continue;

	    signal(SIGHUP, SIG_IGN);
	    signal(SIGINT, SIG_IGN);
	    signal(SIGQUIT, SIG_IGN);

	    if (tmpinit() < 0) {
		goto bex;  /* goto bex */
	    }

	    /* passwd file... */
	    if ((! gfile) && copy2passwd() < 0) {
		break;	/* goto out */
	    }

	    /* group file... */
	    if (gfile && copy2group() < 0) {
		break;	/* goto out */
	    }

	    if (copyback() < 0)
		goto bex;  /* goto bex */

	    exit(0);  /* status OK */

	}  /* end forever */

out:
	unlink(temp);
bex:
	exit(1);
}

/* 
 * initialize a temporary file; returns -1
 * on error, otherwise 0.
 */
tmpinit()
{
	/*
	 * decide if passwd or group
	 */
	if (gfile)
	    strcpy(temp, gtemp);
	else
	    strcpy(temp, ptemp);

	if(access(temp, 0) >= 0) {
		printf("Temporary file busy (%s) -- try again\n", temp);
		return(-1);
	}
	close(creat(temp,0600));
	if ((tf=fopen(temp,"w")) == NULL) {
		printf("Cannot create temporary file (%s)\n", temp);
		return(-1);
	}
	return(0);
}

/*
 * Copy the /etc/passwd file to the tempfile,
 * replacing matching lines with new password.
 * return -1 on error, else 0.
 */
copy2passwd()
{
	while((pwd=getpwent()) != NULL) {
		if(strcmp(pwd->pw_name,uname) == 0) {
			u = getuid();
			if(u != 0 && u != pwd->pw_uid) {
				printf("Permission denied.\n");
				return(-1);
			}
			pwd->pw_passwd = pw;
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
	return(0);
}

/*
 * Copy the /etc/group file to the group tempfile,
 * replacing matching lines with new password.
 * Return -1 on error, else 0.
 */
copy2group()
{
	char **p;	/* tmp pointer */

	while((grp=getgrent()) != NULL) {
		if(strcmp(grp->gr_name,uname) == 0) {
			u = getuid();
			if (u != 0) {	/* this should never happen! */
				printf("Permission denied.\n");
				return(-1);
			}
			grp->gr_passwd = pw;
		}
		fprintf(tf, "%s:%s:%d:",
			grp->gr_name,
			grp->gr_passwd,
			grp->gr_gid);

		/*
		 * print the alternate names in the group,
		 * comma separated.
		 */
		p = grp->gr_mem;
		if (*p != NULL) {
		    fprintf(tf, "%s", *p++);
		    while(*p != NULL)
		   	fprintf(tf, ",%s", *p++);
		}
		fprintf(tf, "\n");
	}
	endgrent();
	fclose(tf);
	return(0);
}

/*
 * copy a temporary file back to a passwd file;
 * returns -1 on error, else 0
 */
copyback()
{
	/*
	 * decide if passwd or group
	 */
	if (gfile) {
	    strcpy(temp, gtemp);
	    strcpy(passwd, gpasswd);
	}
	else {
	    strcpy(temp, ptemp);
	    strcpy(passwd, ppasswd);
	}

	if((fi=open(temp,0)) < 0) {
		printf("Temporary file (%s) disappeared!\n", temp);
		return(-1);
	}
	if((fo=creat(passwd, 0644)) < 0) {
		printf("Cannot recreate %s file.\n", passwd);
		return(-1);
	}
	while((u=read(fi,buf,sizeof(buf))) > 0) write(fo,buf,u);
	unlink(temp);
	return(0);
}

/*
 * check new password for robustness -- new password
 * in pwbuf.  Returns -1 on error, else 0 if OK,
 * or 1 if "try again".  Returns  global 'pw', the
 * encrypted password.
 */
checkpw()
{
	ok = 0;
	flags = 0;
	p = pwbuf;
	while (c = *p++) {
		if(c>='a' && c<='z') flags |= 2;
		else if(c>='A' && c<='Z') flags |= 4;
		else if(c>='0' && c<='9') flags |= 1;
		else flags |= 8;
	}
	if (flags >=7 && pwlen>= 4) ok = 1;
	if (((flags==2)||(flags==4)) && pwlen>=6) ok = 1;
	if (((flags==3)||(flags==5)||(flags==6))&&pwlen>=5) ok = 1;

	if ((ok==0) && (insist<2)){
		if(flags==1)
		printf("Please use at least one non-numeric character.\n");
		else
		printf("Please use a longer password.\n");
		insist++;
		return(1);	/* tryagain */
	}

	if (strcmp(pwbuf,getpass("Retype new password:")) != 0) {
		printf("Mismatch - password unchanged.\n");
		return(-1);	/* goto bex */
	}

	time(&salt);
	salt += getpid();

	saltc[0] = salt & 077;
	saltc[1] = (salt>>6) & 077;
	for(i=0;i<2;i++){
		c = saltc[i] + '.';
		if(c>'9') c += 7;
		if(c>'Z') c += 6;
		saltc[i] = c;
	}
	pw = crypt(pwbuf, saltc);
	return(0);
}
