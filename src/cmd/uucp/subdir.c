
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)subdir.c	3.0	4/22/86";

/************************
 *  routines that implement subdirectory spooling 
 ************************/

/************************
 * Mods:
 *	- change spooling scheme, now: seperate spool directories
 *		for each system plus one DEFAULT directory
 *	- add support routines mkspname, getsubdirs, spoolname, mkspooldirs
 ************************/

#include "uucp.h"
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <errno.h>
#ifdef	UUDIR
/* By Tom Truscott, March 1983 
 * THIS VERSION IS FOR USE ONLY
 * WITH THE 'UUDIR' VERSION OF UUCP.
 *
 * There once was a separate 'uudir' package to retrofit
 * versions of uucp, but that is no longer recommended.
 */

/*
 * Prefix table.
 * If a prefix is "abc", for example,
 * then any file Spool/abc... is mapped to Spool/abc/abc... .
 * The first prefix found is used, so D.foo should preceed D. in table.
 *
 * Each prefix must be a subdirectory of Spool, owned by uucp!
 * Remember: use cron to uuclean these directories daily,
 * and check them manual every now and then.  Beware complacency!
 */

static char *prefix[] = {
	DLocalX,	/* Outbound 'xqt' request files (set in uucpname) */
	DLocal,		/* Outbound data files (set in uucpname) */
	"D.",		/* Other "D." files (remember the "."!) */
	"C.",		/* work file directory */
	"X.",		/* "X." subdirectory */
	0
};

/*
 * filename mapping code to put uucp work files in other directories.
 */

#define	BUFLEN	100
#define DIRMODE 0755
/* assert(strlen(Spool)+1+14+1+14 <= BUFLEN) */

static	int	inspool;		/* true iff working dir is Spool */
static	char fn1[BUFLEN], fn2[BUFLEN];	/* remapped filename areas */

/*
 * return (possibly) remapped string s
 */
char *
SubFile(as)
char *as;
{
	register char *s, **p;
	register int n;
	static char *tptr = NULL;
	char sysdir[MAXFULLNAME];
	char systname[NAMESIZE];
	struct stat statbuf;
	FILE *dp;
	int syslength;
	int found=0;
	syslength=strlen(Rmtname);
	/* Alternate buffers so "link(subfile(a), subfile(b))" works */
	if (tptr != fn1)
		tptr = fn1;
	else
		tptr = fn2;

	s = as;
	tptr[0] = '\0';

	/* if s begins with Spool/, copy that to tptr and advance s */
	if (strncmp(s, Spool, n = strlen(Spool)) == 0 && s[n] == '/') {
		s += n + 1;
	}
	else
		if (!inspool)
			return(as);
			
	/* look for first prefix which matches, and make subdirectory */

	for (p = &prefix[0]; *p; p++) {
		if (strncmp(s, *p, n = strlen(*p))==0 && s[n] && s[n] != '/') {
			sprintf(tptr, "%s/%s/%s",Spool,*p,s);
			DEBUG(9, "Subfile with prefix:%s:\n",tptr);
			return(tptr);
		}
	}
	return(as);
}

/*
 * save away filename
 */
SubChDir(s)
register char *s;
{
	inspool = (strcmp(s, Spool) == 0);
	return(chdir(s));
}

/*
 * return possibly corrected directory for searching
 */
char *
SubDir(d, pre)
register char *d, pre;
{
char sysdir[MAXFULLNAME];
struct stat *stbuf;
	if (strcmp(d, Spool) == 0)
		if (pre == CMDPRE) {
			sprintf(sysdir,"%s/C.",Spool);
			DEBUG(9,"In SubDir, directory is:%s:\n", sysdir);
			return(sysdir);
		}
		else if (pre == XQTPRE) {
			sprintf(sysdir,"%s/X.",Spool);
			DEBUG(9,"In SubDir, directory is:%s:\n", sysdir);
			return(sysdir);
		}
	return(d);
}


/***
 *	mkonedir(name)	make specified directory
 *	char *name;
 *
 *	return 0  |  FAIL
 */

mkonedir(name)
char *name;
{
	int ret, mask;
	char cmd[100];

		if (isdir(name))
			return(0);
		sprintf(cmd, "mkdir %s", name);
		DEBUG(4, "mkonedir - %s\n", name);
		mask = umask(0);
		ret = shio(cmd, CNULL, CNULL, CNULL, CNULL);
		umask(mask);
		if (ret != 0)
			return(FAIL);
}


mkspname(dir)
char *dir;
{
	sprintf(Spoolname,"%s/sys/%s",SPOOL,dir);
	if (!isdir(Spoolname)) {
		sprintf(Spoolname,"%s/sys/DEFAULT",SPOOL);
		ASSERT(isdir(Spoolname), "NO DEFAULT SPOOL DIRECTORY", "subdir", 0);
	}
	Spool = Spoolname;
	/* create names for per system sequence files and locks */
	sprintf(Seqlock,"%s/%s",Spool,SEQLOCK);
	sprintf(Seqfile,"%s/%s",Spool,SEQFILE);
	DEBUG(9,"Spoolname is: %s\n", Spool);
} 
	
mkspooldirs(sysname)
char *sysname;
{
char dirname[MAXFULLNAME];	
register char **p;
extern int errno;
	sprintf(Spoolname,"%s/sys/%s/",SPOOL,sysname);
	errno = 0;
	if (!isdir(Spoolname)) {
		DEBUG(9, "mkspooldir1 %s\n",Spoolname);
		ASSERT2(mkdirs(Spoolname) != FAIL, "can not make spool dir ", Spoolname, errno);
		chmod(Spoolname, DIRMODE);
	}
	for (p = &prefix[0]; *p; p++) {
		sprintf(dirname,"%s%s",Spoolname,*p);
		if (isdir(dirname))
			continue;
		DEBUG(9, "mkspooldir2 %s\n",dirname);
		ASSERT2(mkonedir(dirname) != FAIL, "can not make spool subdir ",
			dirname,errno);
		chmod(dirname, DIRMODE);
	}
}

/* determine name of spool directory for the specified system */ 

char syspoolname[MAXFULLNAME];
char *
spoolname(sysname)
char *sysname;
{
	sprintf(syspoolname,"%s/sys/%s",SPOOL,sysname);
	if (!isdir(syspoolname))
		sprintf(syspoolname,"%s/sys/DEFAULT",SPOOL);
	return(syspoolname);	
}

/* return pointer to subdirectories: 0 implies end */
char *
getsubdirs()
{
	static char **pref = prefix;
	if (*pref)
		return(*pref++);
	else 
		pref = prefix;
	return(CNULL);
}
	
#else
static	int	subdir_here;	/* quiet 'ranlib' command */
#endif
