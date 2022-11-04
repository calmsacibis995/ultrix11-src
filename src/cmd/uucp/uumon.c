
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)uumon.c	3.0	4/22/86";

/*
 * uumon.c
 *
 *	Program to monitor the uucp usage on a machine.
 *
 *					martin levy. (houxg!lime!martin).
 */


/************************
 * Mods:
 *	decvax!larry - handle subdirectories properly
 *		     - change format of output 
 ***********************/


#include	"uucp.h"
#include	<sys/types.h>
#ifdef NDIR
#include "ndir.h"
#else
#include <sys/dir.h>
#endif
#include	<time.h>
#include	<sys/stat.h>

#define	NMACH	200

struct m
{
	char mach[15];
	char locked;
	int ccount,xcount;
	int count,type;
	long retrytime;
	time_t lasttime;
	char stst[132];
} M[NMACH];

struct m *machine();
time_t time();

main()
{
struct direct *dirp, *Cdirp;
	struct m *m;
	DIR *ufd, *Cfd;
	FILE *sfd;
	char line[132],*c;
	int Ctotal=0;
	int Xtotal=0;
	struct stat statbuff;
	char tempname[16];
	char directory[250];
	FILE *dirlist;
	time_t successtime;

	if ((Cfd = opendir("/usr/spool/uucp/sys","r"))==NULL) {
		fprintf(stderr,"Can not open sys directory");
		exit(20);
	}
	/*  scan through the sys directory.  For each system process
	 *   the C. subdirectory then the X. subdirectory.
	 */
	while ((Cdirp=readdir(Cfd)) != NULL) {
		if(Cdirp->d_ino==(ino_t)0 || strncmp(".",Cdirp->d_name,1)==0)
			continue;

		/* process the C. directory corresponding to d_name */

		sprintf(directory,"/usr/spool/uucp/sys/%s/C.",Cdirp->d_name);
		if( (ufd=opendir(directory,"r")) == NULL ) {
			fprintf(stderr, "Can not open C. dir: %s\n",directory);
			exit(199);
		}
		while( (dirp=readdir(ufd))!=NULL)
		{
			if( dirp->d_ino == (ino_t)0 )
				continue;
			if( strncmp("C.",dirp->d_name,2) == 0 )
			{
				*(dirp->d_name+strlen(dirp->d_name)-5) = '\0';
				m = machine(dirp->d_name+2);
				m->ccount++;
				Ctotal++;
				continue;
			}
		}
		closedir(ufd);

		/* process X. subdirectory */

		sprintf(directory,"/usr/spool/uucp/sys/%s/X.",Cdirp->d_name);
		if( (ufd=opendir(directory,"r")) == NULL ) {
			fprintf(stderr,"can not open X. dir: %s\n", directory);
			exit(99);
		}
		while( (dirp=readdir(ufd))!=NULL)
		if( strncmp("X.",dirp->d_name,2) == 0 ) {
			*(dirp->d_name+strlen(dirp->d_name)-5) = '\0';
			m = machine(dirp->d_name+2);
			m->xcount++;
			Xtotal++;
			continue;
		}
		closedir(ufd);
	}
	closedir(Cfd);

	/* get the latest status for each system */

	if( (ufd=opendir("/usr/spool/uucp/STST.","r")) == NULL ) {
		fprintf(stderr, "can not open SPOOL/STST. directory");
		exit(299);
	}
	chdir("/usr/spool/uucp/STST.");
	while( (dirp=readdir(ufd))!=NULL) 
		if( strncmp("STST.",dirp->d_name,5) == 0 ) {
			m = machine(dirp->d_name+5);
			if( (sfd=fopen(dirp->d_name,"r")) != NULL ) {
				fscanf(sfd, "%d %d %ld %ld %*ld", &m->type,
					&m->count, &m->lasttime,
					&m->retrytime);
				if( fgets(m->stst,132,sfd) != NULL ) {
					/* remove remote name from STST file */
					c = m->stst + strlen(m->stst);
					while( *c != ' ' )
						*c-- = NULL;
				}
				fclose(sfd);
			}
		}

	closedir(ufd);
	if( (ufd=opendir(SPOOL,"r")) == NULL ) {
		fprintf(stderr, "can not open spool directory");
		exit(99);
	}

	/* record existing lock files */

	while( (dirp=readdir(ufd))!=NULL) {
		if( strncmp("LCK..",dirp->d_name,5) == 0 ) {
			strcpy(tempname,dirp->d_name+3);
			tempname[0]='L';
			m = machine(tempname);
			m->locked++;
			stat(dirp->d_name, &statbuff);
			m->lasttime = statbuff.st_ctime;
			continue;
		}
	}
	closedir(ufd);
	printf("C.total=%d      X.total=%d\n\n\n", Ctotal, Xtotal);
	for(m = &M[0];*(m->mach) != NULL;m++)
		printit(m);
}

struct m *machine(name)
char *name;
{
	static int first = -1;
	struct m *m;
	int i;

	if( first )
	{
		first = 0;
		for(i=0;i<NMACH;*(M[i++].mach)=NULL);
	}
	for(m = &M[0];*(m->mach)!=NULL;m++)
		if( strncmp(name,m->mach,7) == 0 )
			return(m);
	strncpy(m->mach,name,7);
	m->locked = 0;
	m->ccount = 0;
	m->xcount = 0;
	*(m->stst) = NULL;
	return(m);
}

printit(m)
struct m *m;
{
	time_t t;
	int min;
	struct tm *lt;

	printf("%s\t",m->mach);
	if(m->locked) {
		printf("\t\t\tlocked\t\t");
		lt = localtime(&m->lasttime);
		printf("\t(%d/%d-%d:%02d)", lt->tm_mon+1,
			lt->tm_mday, lt->tm_hour, lt->tm_min);
	}	
	else {
		printf("%3dC\t",m->ccount);
		printf(" %3dX\t",m->xcount);
	}
	if( *(m->stst) != NULL )
	{
		printf("%20s\t",m->stst);
		if( m->type == SS_FAIL )
			printf("CNT: %2d",m->count);
		lt = localtime(&m->lasttime);
		printf("\t(%d/%d-%d:%02d)", lt->tm_mon+1,
			lt->tm_mday, lt->tm_hour, lt->tm_min);
	}
	putchar('\n');
}


