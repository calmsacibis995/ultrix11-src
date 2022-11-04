static char Sccsid[] = "@(#)compact.c	3.0	4/22/86";

/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * compact.c
 *
 *	Program to compact the directories
 *	   - creates temp directory, links old files to temp directory,
 *		unlinks old files, renames temp directory to old directory
 *
 *					
 */

#include	<stdio.h>
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
char temp[250];

main(argc,argv)
char **argv; int argc;
{
struct direct *dirp, *Cdirp;
	struct passwd *pwd;
	DIR *ufd, *Cfd;
	struct stat statbuff;
	char newname[250];
	char oldname[250];
	char *directory;
	FILE *dirlist;
	char Ddir[250];
	char thissys[20];


	while(argc>1) {
		directory = *++argv;
		argc--; 
		printf("directory to compact is: %s\n", directory);
		compact(directory);
		printf("compaction of: %s  is complete\n", directory);
	}
}

pexit(msg)
char *msg;
{
	fprintf(stderr,"%s, errno=%d\n",msg, errno);
	exit(errno);
}

newdir(ndir)
char *ndir;
{
	if (rmdir(ndir))
		pexit("can not remove directory");
	if (rename(temp, ndir))
		pexit("rename");
}

compact(directory)
char *directory;
{
	char oldname[100];
	char newname[100];
	DIR *ufd;
	struct direct *dirp;
	sprintf(temp,"temp.%s",directory);
	if( (ufd=opendir(directory,"r")) == NULL )
		pexit(directory);
	if( mkdir(temp, 0755) )
		pexit("Can not make temp");
	while( (dirp=readdir(ufd))!=NULL)
	{
		if( dirp->d_ino==(ino_t)0 || strncmp(".", dirp->d_name,1)==0)
			continue;
		sprintf(oldname,"%s/%s",directory,dirp->d_name);
		sprintf(newname,"%s/%s",temp, dirp->d_name);
		if (link(oldname, newname))
			pexit(newname);
		if (unlink(oldname))
			pexit(oldname);
	}
	closedir(ufd);
	newdir(directory);
}

