
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)uucpname.c	3.0	4/22/86";

/*******
 *	uucpname(name)		get the uucp name
 *
 *	return code - none
 */


/*********************
 * Mods:
 *	decvax!larry - create list of systems that have their own
 *			spool directory.
 ********************/

#include "uucp.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef NDIR
#include "ndir.h"
#else
#include <sys/dir.h>
#endif



#ifdef	CCWHOAMI
/*  compile in local uucp name of this machine */
#include <whoami.h>
#endif


uucpname(name)
register char *name;
{
	register char *s, *d;

	/* rti!trt
	 * Since some UNIX systems do not honor the set-user-id bit
	 * when the invoking user is root, we must change the uid here.
	 * So uucp files are created with the correct owner.
	 */
	if (geteuid() == 0 && getuid() == 0) {
		struct stat stbuf;
		stbuf.st_uid = 0;	/* In case the stat fails */
		stbuf.st_gid = 0;
		stat(UUCICO, &stbuf);	/* Assume uucico is correctly owned */
		setgid(stbuf.st_gid);
		setuid(stbuf.st_uid);
	}

#ifdef	UUNAME		/* This gets home site name from file  */
    {
	FILE *uucpf;
	char stmp[10];

	s = stmp;
	if (((uucpf = fopen("/etc/uucpname", "r")) == NULL &&
	     (uucpf = fopen("/local/uucpname", "r")) == NULL) ||
		fgets(s, 8, uucpf) == NULL) {
			s = "unknown";
	} else {
		for (d = stmp; *d && *d != '\n' && d < stmp + 8; d++)
			;
		*d = '\0';
	}
	if (uucpf != NULL)
		fclose(uucpf);
    }
#endif

#ifdef	GETHOST
    {
	char hostname[15];
	int hostlength=15;
	gethostname(hostname, &hostlength);
	s = hostname;
    }
#endif

#ifdef	CCWHOAMI
    {
	s = sysname;
    }
#endif


	d = name;
	while ((*d = *s++) && d < name + 7)
		d++;
	*(name + 7) = '\0';
#ifdef	UUDIR
	sprintf(DLocal, "D.%s", name);
	sprintf(DLocalX, "D.%sX", name);
	build_DIRLIST();
#endif
	return;
}

/****************
 *    initialize list of remote system subdirectories
 *
 *************/

#ifdef UUDIR

build_DIRLIST()
{
	struct direct *Cdirp;
	DIR *Cfd;
	char systname[NAMESIZE];
	char sysdir[MAXFULLNAME];

	Subdirs = 0;
	sprintf(sysdir,"%s/sys",SPOOL);
	Cfd=opendir(sysdir,"r");
	ASSERT(Cfd != NULL,"CAN NOT OPEN", sysdir, 0);
	while ((Cdirp=readdir(Cfd)) != NULL) {
                if(Cdirp->d_ino==(ino_t)0 || strcmp("DEFAULT",Cdirp->d_name)==0 			|| strcmp(".",Cdirp->d_name)==0 || strcmp("..",Cdirp->d_name)==0)
			continue;
		Dirlist[Subdirs] = malloc((unsigned)(strlen(Cdirp->d_name)+1));
		strcpy(Dirlist[Subdirs++], Cdirp->d_name);
		DEBUG(9, "In uucpname, Dirlist:%s:\n",Dirlist[Subdirs-1]);
	} 
	closedir(Cfd);
}
#endif
