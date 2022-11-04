
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)uucompact.c	3.0	4/22/86";

/*
 * uucompact.c
 *
 *	Program to compact the uucp spool directories
 *
 *					
 */

#include	"uucp.h"
#include	<sys/types.h>
#ifdef NDIR
#include "ndir.h"
#else
#include <sys/dir.h>
#endif
#include	<time.h>
#include	<pwd.h>
#include	<errno.h>
#include	<sys/stat.h>

extern int errno;
int uuid, ugid;
#define SYS 	"/usr/spool/uucp/sys"
char *getsubdirs();
#define FIND 	0
#define UPDATE	1
#define DONE	2
#define BEGIN	1
#define CURRENT	0
#define TRUE 	0
#define FALSE	-1

main(argc, argv)
int argc; char *argv[];
{
struct direct *dirp, *Cdirp;
	struct passwd *pwd;
	DIR *ufd, *Cfd;
	struct stat statbuff;
	char newname[MAXFULLNAME];
	char oldname[MAXFULLNAME];
	char directory[MAXFULLNAME];
	FILE *dirlist;
	char compdir[MAXFULLNAME];
	char thissys[50];
	char *csys;
	char *subd;
	int ret;
	

	if ((pwd=getpwnam("uucp"))==NULL)
		ASSERT(pwd != NULL, "could not get passwd entry for uucp", "", errno);
	uuid = pwd->pw_uid;
	ugid = pwd->pw_gid;
	umask(WFMASK);

	csys = "ALL";
	uucpname(thissys);  /* init subdir stuff */

	while (argc>1 && argv[1][0] == '-') {
		switch (argv[1][1]) {
		case 's':  /* system to compact */
			csys = &argv[1][2];
			break;
		case 'x':
			Debug = atoi(&argv[1][2]);
			break;
		default:
			printf("unknown flag %s\n", argv[1]); break;
		}
		--argc;  argv++;
	}


	DEBUG(3, "START\n\n","");
	if (strcmp(csys,"ALL") != SAME) {
		sprintf(compdir,"%s/%s",SYS, csys);
		if (stat(compdir, &statbuff)) {
			fprintf(stderr, "sys directory does not exist: %s\n",
					compdir);
			exit(1);
		}
		DEBUG(3, "compact system: %s\n", csys);
		chdir(compdir);
		while((subd=getsubdirs())!=CNULL) 
			compact(subd);
		exit(0);
	}
		
	if ((Cfd = opendir(SYS,"r"))==NULL) 
		ASSERT(Cfd != NULL, "Can no open directory", SYS, errno);

	DEBUG(4, "try to compact all systems\n","");
	/* process all systems in SYS */
	while ((Cdirp=readdir(Cfd)) != NULL) {
		if(Cdirp->d_ino==(ino_t)0 || strncmp(".",Cdirp->d_name,1)==0)
			continue;

		/* if system has not been processed then proceed */

		ret = process(Cdirp->d_name, FIND);
		if (ret) /* update progress file if not found */
			process(Cdirp->d_name, UPDATE);
		sprintf(compdir,"%s/%s",SYS,Cdirp->d_name);
		DEBUG(3,"compact system dir: %s\n", compdir);
		chdir(compdir);
		while((subd=getsubdirs())!=CNULL) {

			/* if subdirectory has not been processed then
			 * proceed.
			 */

			if (ret) { 
				/* we have not processed this system yet
				 * therefore we did not process any of
				 * its subdirectories.  So dont
				 * check to see if they have been
			  	 * processed.
				 */
				compact(subd);
				process(subd, UPDATE);
			}
			else 
				/* this system has been processed previously */
				/* check to see if all subdirs have been 
				 * processed.
				 */
				if (process(subd, FIND)==FALSE) {
					/* not processed yet */
					DEBUG(3,"compact subdir: %s\n",subd);
					compact(subd);
					process(subd, UPDATE);
				}
		}
	}
	closedir(Cfd);

	chdir(SPOOL);
	if (process("TM.", FIND)==FALSE) {
		compact("TM.");
		process("TM.",UPDATE);
		DEBUG(3,"have compacted TM.\n","");
	}

	if (process("STST.", FIND)==FALSE) {
		compact("STST.");
		process("STAT.", UPDATE);
		DEBUG(3,"have compacted STST.\n","");
	}

	process("", DONE);  /* remove logging file */
}

/*
 * check to see if the directory corresponding to name has been
 * compacted yet.  
 */

#define PROGRESS "/usr/spool/uucp/UUCOMPLOG"

process(name, command)
int command;
char *name;
	{
	char buf[NAMESIZE];
	int namesiz;
	struct stat stbuf;
	static FILE *prog = NULL;
	if (prog == NULL)  /* create progress file if first time through */
		/* if progress file exists then a previous uucompact process
		 * was aborted - open the progress file and continue
		 * processing.
		 */
		if (stat(PROGRESS, &stbuf)) 
			prog = fopen(PROGRESS, "w+");
		else 
			/* create progress file */
			prog = fopen(PROGRESS, "r+");
	
	ASSERT(prog != NULL, "Could not open progress file", "", errno);

	switch(command) {
	case FIND:
		namesiz = strlen(name);
		while (fgets(buf,NAMESIZE,prog) != NULL) {
			if (strncmp(name,buf,namesiz) == SAME) {
				DEBUG(4," found name: %s\n", name);
				return(TRUE);
			}
		}
		DEBUG(4, "name not found: %s\n", name);
		return(FALSE);
		break;
	case UPDATE:
		DEBUG(4, "update : %s\n", name);
		fputs(name, prog);
		fputc('\n', prog);
		break;
	case DONE:
		DEBUG(4, "done : %s\n", "");
		unlink(PROGRESS);
		break;
	}
}

			

newdir(ndir, tempdir)
char *ndir, *tempdir;
{
#ifdef V7M11
	int ret;
	char cmd[MAXFULLNAME];
	sprintf(cmd, "rm -r %s", ndir);
	system(cmd);
	sprintf(cmd, "mv %s %s", tempdir, ndir);
	ret = system(cmd);
	ASSERT(ret==0, "can not rename", tempdir, errno);
#else
	ASSERT(rename(tempdir,ndir)==0, "can not rename", tempdir, errno);
#endif
	chown(ndir, uuid, ugid);
	chmod(ndir, 0755);
}

compact(directory)
char *directory;
{
	char oldname[MAXFULLNAME];
	char newname[MAXFULLNAME];
	char temp[MAXFULLNAME];
#ifdef V7M11
	char cmd[MAXFULLNAME];
#endif
	DIR *ufd;
	struct direct *dirp;
	errno = 0;
	if( (ufd=opendir(directory,"r")) == NULL )
		ASSERT(ufd != NULL, "can not open directory", directory, errno);
	sprintf(temp,"temp.%s", directory);
#ifdef V7M11
	sprintf(cmd,"mkdir %s", temp);
	system(cmd);
#else
	if (mkdir(temp) && errno != EEXIST) 
		ASSERT(0, "can not make temp", temp, errno);
#endif
	chmod(temp,0755);
	while( (dirp=readdir(ufd))!=NULL)
	{
		if( dirp->d_ino==(ino_t)0 || strncmp(".", dirp->d_name,1)==0)
			continue;
		sprintf(oldname,"%s/%s",directory,dirp->d_name);
		DEBUG(8,"oldname=[%s]",oldname);
		sprintf(newname,"%s/%s",temp, dirp->d_name);
		DEBUG(8,"  newname=[%s]\n",newname);

		/* if this is a restart of uucompact there may be file
		 * which has been linked to a temp file but has
		 * not been unlinked yet
		 */
		
		if (link(oldname, newname) < 0 && errno!=EEXIST)
			ASSERT(0,"can not link file to temp", newname, errno);
		ASSERT(unlink(oldname)==0,"can not unlink file ",
			oldname, errno);
	}
	closedir(ufd);
	newdir(directory, temp);
}

cleanup()
{
exit(1);
}
