
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)uurespool.c	3.0	4/22/86";

/*
 * uurespool.c
 *
 *	Program to move files spooled in old uucp formats
 *	to files spooled in the new uucp format.
 *	Also moves files from the new DEFAULT directory to per system
 *	directories.
 *	Three "old" formats are handled:
 *		1) original - all files are in /usr/spool/uucp
 *		2) rti's split spool -  contains the following subdirectories:
 *			C., X., D., D.local, D.localX
 *		3) modified rti - contains all subdirectories listed in 2) plus:
 *			STST., TM., C./OTHERS
 *		4) used when a new system directory has been created and
 *		 	spool files must be moved from the DEFAULT directory
 *			to the new system directory.
 *
 *	The new spool format is: /usr/spool/uucp/sys/systemname
 *	Each per system directory includes the following subdirectories:
 *		C., X., D., D.local, D.localX
 *				
 */

#include	"uucp.h"
#include	<sys/types.h>
#ifdef NDIR
#include "ndir.h"
#else
#include <sys/dir.h>
#endif
#include	<errno.h>
#include	<sys/stat.h>
#define ORIG	1
#define RTI	2
#define RTIMOD	3
#define NEW	4

char *getsubdirs();
extern int errno;
#define SYS 	"/usr/spool/uucp/sys"
char *Subprefix[10];

main(argc, argv)
int argc; char *argv[];
{
struct direct *dirp, *cdirp;
	struct passwd *pwd;
	DIR *ufd, *cfd;
	char newname[MAXFULLNAME];
	char oldname[MAXFULLNAME];
	char directory[MAXFULLNAME];
	FILE *dirlist;
	char spool[MAXFULLNAME];
	char thissys[50];
	char *subd;
	int ret;
	int help = 0;
	int sptype = 0;
	int i = 0;


	uucpname(thissys);  /* init subdir stuff */

	while (argc>1 && argv[1][0] == '-') {
		switch (argv[1][1]) {
		case 't':  /* type of spooling system */
			sptype = atoi(&argv[1][2]);
			break;
		case 'x':
			Debug = atoi(&argv[1][2]);
			break;
		case 'h': /* help */
			prhelp();
			break;
		default:
			printf("unknown flag %s\n", argv[1]); break;
		}
		--argc;  argv++;
	}


	/* check to see if chose valid spool format */
	if (sptype < 1 || sptype > 4) {
		printf("Invalid spool format\n\n");
		prhelp();
	}
	DEBUG(3, "START\n\n","");

	/* get subdirectory prefixes */
	while ((Subprefix[i++] = getsubdirs()) != NULL); 

	if (sptype == ORIG || sptype == RTI || sptype == NEW) {
		switch(sptype) {
		case ORIG:
			strcpy(spool, SPOOL);
			break;
		case RTI:
			sprintf(spool,"%s/C.",SPOOL);
			break;
		case NEW:
			sprintf(spool, "%s/sys/DEFAULT/C.", SPOOL);
			break;
		}
		ASSERT((cfd=opendir(spool,"r"))!=NULL,"can not open C. directory",
			spool, errno);
		chdir(spool);

		/* get next C.file and move to new directory */
		while( (cdirp=readdir(cfd))!=NULL) {
			if( cdirp->d_ino == (ino_t)0  || 
				strncmp("C.", cdirp->d_name,2) != SAME)
					continue;
			/* move to correct spool directory */
			movefile(cdirp->d_name, sptype);
		}
		closedir(cfd);
		moveXfiles(sptype);
		cleanup(0);
	}


	/* must be RTIMOD */
	sprintf(spool, "%s/C./",SPOOL);
	ASSERT((cfd=opendir(spool, "r"))!=NULL, "can not open C. - RTIMOD",
		spool, errno);
	
	while ((dirp=readdir(cfd)) != NULL) {
		if(dirp->d_ino==(ino_t)0 || strncmp(".",dirp->d_name,1)==0)
			continue;

		/* process the C. directory corresponding to d_name */

		sprintf(directory,"%s/C./%s", SPOOL, dirp->d_name);
		ufd=opendir(directory,"r");
		ASSERT(ufd != NULL,"Can not open C. dir: %s\n",directory, errno);
		chdir(directory);
		while( (cdirp=readdir(ufd))!=NULL) {
			if( cdirp->d_ino == (ino_t)0  || 
				strncmp("C.", cdirp->d_name,2) != SAME)
					continue;
			/*
			 * move to new spool directory 
			 */
			movefile(cdirp->d_name, sptype);
		}
		closedir(ufd);
	}
	closedir(cfd);
	moveXfiles(sptype);
}

#define W_TYPE		wvec[0]
#define W_OPTNS		wvec[4]
#define W_DFILE		wvec[5]
#define SNDFILE 'S'	/* send file (string) */

/* move cfile and associated spooled D.files */

movefile(cfile, type)
char *cfile; int type;
{
	FILE *fp;
	char dest[NAMESIZE];
	char newname[MAXFULLNAME];
	char newspool[MAXFULLNAME];
	char str[250];
	char *wvec[20];
	struct stat stbuf;

	strcpy(dest, cfile+2);
	*(dest+strlen(dest)-5) = '\0';
	DEBUG(5,"moving cfile: %s", cfile);
	DEBUG(5," to: %s\n", dest);
	sprintf(newspool,"%s", spoolname(dest));
	sprintf(newname,"%s/C./%s", newspool, cfile);
	DEBUG(5,"new cfile: %s\n", newname);

	/* move D.files first */
	ASSERT((fp=fopen(cfile, "r"))!=NULL, "can not open C.file to read",
		cfile, errno); 
	while (fgets(str, 250, fp) != NULL) {
		getargs(str, wvec);
		/*
		 * only send lines without the -c option will refer to 
		 * spooled  D.files 
		 */
		if (W_TYPE[0]==SNDFILE && index(W_OPTNS, 'c')==NULL)  
			moveDfile(W_DFILE,type,newspool);
	}
	fclose(fp);

	/* move C.file */
	if (type==NEW && stat(newname, &stbuf)==0)
		return(0); /* if new file exists dont overwrite */
	if (link(cfile, newname) < 0 )
		ASSERT(0,"can not link file ", newname, errno);
	ASSERT(unlink(cfile)==0,"can not unlink file ", cfile, errno);
}


/* move D.file to new spool directory */

moveDfile(dfile, stype, nspool)
char *dfile, *nspool; 
int stype;
{
	char Dloc[MAXFULLNAME];
	char newfile[MAXFULLNAME];
	char *p;
	register int i = 0;
	struct stat stbuf;

	/* determine correct prefix: D., D.local, or D.localX */
	while ((p = Subprefix[i++]) != NULL) 
		if (strncmp(dfile, p, strlen(p))==0) 
		      break;
	if (p == NULL) {
		fprintf(stderr," could not find D.file prefix - using D.\n");
		p = "D.";
	} 

	/* determine dest. file */
	sprintf(newfile, "%s/%s/%s", nspool, p, dfile);
	DEBUG(4, "dest. Dfile is: %s\n", newfile);

	/* move D.file */
	if (stype == ORIG) { 
		if (link(dfile, newfile) < 0 && errno!=EEXIST && errno!=ENOENT) 
			ASSERT(0,"can not link type 0 file ", newfile, errno);
		if (unlink(dfile)<0 && errno!=ENOENT)
			ASSERT(0,"can not unlink type 0 Dfile ", dfile, errno); 
				
	}
	else { /* determine path of old D. file */ 
		if (stype == NEW)
			sprintf(Dloc, "%s/sys/DEFAULT/%s/%s",SPOOL, p, dfile);
		else
			sprintf(Dloc, "%s/%s/%s",SPOOL, p, dfile);
		DEBUG(5, "moving dfile: %s\n", Dloc);
		
		if (stype==NEW && stat(newfile, &stbuf)==0)
			return(0);
		if (link(Dloc, newfile)<0 && errno!=EEXIST && errno!=ENOENT) 
			ASSERT(0,"can not link file ", newfile, errno);
		if (unlink(Dloc)<0 && errno!=ENOENT)
			ASSERT(0,"can not unlink type  Dfile ", Dloc, errno); 
	}
		
}


/* move X.files and associated D.files */

moveXfiles(type)
int type;
{
	DIR *xdir;
	struct direct *xdirp;
	FILE *fp;
	struct stat stbuf;
	char spool[MAXFULLNAME];
	char buf[BUFSIZ], rqfile[MAXFULLNAME];
	char source[NAMESIZE], newxname[MAXFULLNAME];
	char xfile[NAMESIZE];
	switch(type) {
	case ORIG:
		sprintf(spool, "%s", SPOOL);
		break;
	case RTIMOD:
	case RTI:
		sprintf(spool, "%s/X.", SPOOL);
		break;
	case NEW:
		sprintf(spool, "%s/sys/DEFAULT/X.", SPOOL);
		break;
	}
	chdir(spool);
	ASSERT((xdir=opendir(spool))!=NULL, "can not open spool for X.files",
		spool, errno);
	while((xdirp=readdir(xdir)) != NULL) {
		strcpy(xfile, xdirp->d_name);
		if (xdirp->d_ino == (ino_t)0 || strncmp("X.",xfile,2) != SAME) 
			continue;
		/* determine source site of xfile */
		strcpy(source, xfile+2);
		*(source+strlen(source)-5) = '\0';
		DEBUG(5, "source of xfile: %s\n", source);
		
		/*
		 * xfile will contain the name of any Dfiles that it will
		 * need.  If they are residing in the old spool directory 
		 * they should be moved to the new spool directory.
		 */
		ASSERT((fp=fopen(xfile, "r"))!=NULL, "can not open xfile",
			xfile, errno);
		while (fgets(buf, BUFSIZ, fp) != NULL) {
			if (buf[0] != X_RQDFILE)
				continue;
			sscanf(&buf[1], "%s", rqfile); /* required file */
			if (strncmp(rqfile, "D.", 2)!=SAME) 
				continue;  /* not a spool file */
			moveDfile(rqfile, type, spoolname(source));
		}
		fclose(fp);
		/* move the X.file to the new spool directory */
		sprintf(newxname, "%s/X./%s", spoolname(source), xfile);
		if (type==NEW && stat(newxname, &stbuf)==0)
			return(0);
		if (link(xfile, newxname) < 0 && errno!=EEXIST)
			ASSERT(0,"can not link Xfile ", newxname, errno);
		ASSERT(unlink(xfile)==0,"can not unlink Xfile ", 
			xfile, errno);
	}
}
/*   print help message */

prhelp()
{
	printf("\nmv old spool files to new spool directories - old formats are:\n");
 	printf("  -t1  original - all files are in /usr/spool/uucp\n");
 	printf("  -t2  rti's split spool-  C., X., D., D.local, D.localX\n");  
 	printf("  -t3  modified rti - same as 2) plus: STST., TM. C./OTHERS\n");
	printf("  -t4  move files from DEFAULT to new system directory\n");
	cleanup(0);
}

cleanup(code) 
int code;
{
exit(code);
}
