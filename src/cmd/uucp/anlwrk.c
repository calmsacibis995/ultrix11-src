
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)anlwrk.c	3.0	4/22/86";

#include "uucp.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "uust.h"
#ifdef NDIR
#include "ndir.h"
#else
#include <sys/dir.h>
#endif

/* Re-written to be reasonable
 * Mon Nov 15 17:19:52 EST 1982
 * Alan S. Watt (ittvax!swatt)
 *
 * Tom Truscott (rti!trt):
 * Priority ordering cleaned up.  New 'pcompar' subroutine.
 * 'stat' removed (speeds things up).
 * Possible infinite loop in gtwvec defended against.
 * Feb 23, 1983
 *
 * Changes:
 *
 *  1)	The check for work is much faster; the first filename
 *	that matches the prefix causes a "yes" return.
 *
 *  2)	The filename is not "stat" ed , so
 *	there is no massive delay while the list of potential
 *	names is built.
 *
 *  3)	Requesting work for a new system is now detected so
 *	internal variables are re-initialized properly.  In
 *	particular, the stream pointer for the current work
 *	file is properly closed so work for a system which
 *	hangs up will not be sent to the next system called.
 *
 * Fri Dec  3 09:31:45 EST 1982
 *
 *  5)	As new work files are requested, a check is made
 *	every TLIMIT seconds (5 minutes at present) to see
 *	if new files have entered the spool area.  Since
 *	work file names are now cached up to LLEN, this can
 *	represent a very long transmission time before new
 *	work enters the list to be processed.  If people want
 *	to use the "grade" character to specify a higher
 *	priority, the list must be re-built and re-sorted for
 *	higher priority stuff to have an immediate effect.
 *
 *  6)  small changes to run under new spooling system - decvax!larry
 */

#define LLEN 20
#define MAXRQST 250
#define TLIMIT	(5*60L)
#define NITEMS(X)	(sizeof (X) / sizeof ((X)[0]))

/* These are all used only locally
 */
static	int Nfiles = 0;
static	char Filent[LLEN][NAMESIZE];


/*******
 *	anlwrk(file, wvec)	create a vector of command arguments
 *	char *file, **wvec;
 *
 *	return codes:
 *		0  -  no more work in this file
 *		positive number  -  number of arguments
 */

/* LOCAL only */
int
anlwrk(file, wvec)
char *file, **wvec;
{
	static char str[MAXRQST];
	static FILE *fp = NULL;

	/* If called with a null string, force a shutdown
	 * of the current work file.
	 * John Levine, ima.247, related change in cntl.c
	 */
	if (file[0] == '\0') {
		DEBUG(9, "anlwrk, file is null, fp=%d\n", fp);
		if (fp != NULL)
			fclose (fp);
		fp = NULL;
		return(0);
	}
	if (fp == NULL) {
		fp = fopen(subfile(file), "r");
		DEBUG(9, "anlwrk, fp is null but file exists: %s\n", file);
		if (fp == NULL) {
			unlink(subfile(file));	/* Try to zap the thing. rti!trt */
			return(0);
		}
		Usrf = 0;	/* for UUSTAT stuff */
	}

	/* This is what deletes the current work file when EOF
	 * is reached.  As this is called from gtwvec, which is
	 * in turn called externally, it is not possible to save
	 * "C." files in case of error, except for line errors,
	 * which shuts down the whole system.
	 */
	if (fgets(str, MAXRQST, fp) == NULL) {
		fclose(fp);
		DEBUG(9,"unlinking work file: %s\n", file);
		unlink(subfile(file));
		USRF(USR_COMP);
		US_RRS(file, Usrf);
		Usrf = 0;
		file[0] = '\0';
		fp = NULL;
		return(0);
	}
	return(getargs(str, wvec));
}


/***
 *	bldflst - build list of work files for given system
 *	 Nfiles, Filent are global
 *
 *	return value - 1 if work was found, else 0
 *
 * Jul 26 19:17 1982 (ittvax!swatt). fixed this obnoxious
 * routine to NOT read all the way through the damned directory
 * "stat"'ing every file in sight just to get 10 names!!!
 *
 * It still reads through the directory from the beginning until
 * the list is filled, but this is only once every LLEN names.
 */

/* LOCAL only */
int
bldflst (reqst, dir, pre)
char *reqst;
register char *dir, *pre;
{
	static DIR  *dirp = NULL;
	register nfound;
	char filename[NAMESIZE];	/* @@@ NB: this needs new dir stuff */
	int plen = strlen (pre);

	if (pre[0]  == '\0' ) {		    /* need to close if new system */
		DEBUG(9,"bldlst pre:%s:\n", pre);
		if (dirp != NULL) {    /* don't close if first time through */
			closedir(dirp);
			dirp = NULL;
		}
		return;
	}
	if (dirp == NULL) {
		if ((dirp = opendir(subdir(dir,pre[0]),"r")) == NULL) {
			DEBUG(9,"In bldflst: can not open:%s:\n", dir);
			return(0);
		}
	}
	else
		rewinddir(dirp);
	for (nfound = 0, Nfiles = 0; gnamef(dirp, filename);) {
		/* Check for two systems with the same prefix.
		 * Magic number "5" is 1 for "grade" character plus
		 * 4 for sequence number.  The point here is to not
		 * send work for a system which has as a prefix the
		 * name of the system called for.
		 */
		if (!prefix(pre, filename) || (strlen(filename) - plen) != 5)
			continue;
		nfound++;
		if (*reqst == 'c') {
			DEBUG(9, "in bldlst - found file for %s\n", pre);
			closedir(dirp);
			dirp = NULL;
			return (1);
		}
		entflst(filename);
	}
	return (nfound? 1: 0);
}

/***
 *	entflst - put new name if list is not full
 *		  or new name is less than the MAX
 *		  now in the list.
 *	Nfiles, Filent[] are modified.
 *	return value - none
 *
 */

/* LOCAL only */
int
entflst(file)
char *file;
{
	register int i;
	register char *p;

	/* If there is room in the table, just add it. */
	if (Nfiles < LLEN) {
		strcpy(Filent[Nfiles++], file);
		DEBUG(9,"in entflst, add another file: %s\n", file);
		return;
	}

	/* Find lowest priority file in table  */
	p = Filent[0];
	for (i = 1; i < Nfiles; i++)
		if (pcompar(Filent[i], p) < 0)
			p = Filent[i];

	/*
	 * If new candidate is of higher priority
	 * that the lowest priority file in the table,
	 * replace the table entry.
	 */
	if (pcompar(p, file) < 0)
		strcpy(p, file);
}

/*
  Compare priority of filenames p1 and p2.  Return:
 *	< 0	if p1 "has lower priority than" p2.
 *	= 0	if p1 "has priority equal to" p2.
 *	> 0	if p1 "has greater priority than" p2.
 * Priority:
 *	lower grade wins.
 *	lower sequence number wins (unless wrap-around is suspected).
 *
 * This code is a bit obscure. It is intending to be fast.
 */
/* LOCAL only */
int
pcompar(ap1, ap2)
char *ap1, *ap2;
{
	register char *p1, *p2;
	register int n, rc;

	p1 = ap1;
	p2 = ap2;
	while (*p1 == *p2++)
		if (*p1++ == '\0')
			return(0);	/* This should not happen */
	rc = *p1 - *--p2;
	n = 0;
	while (*p1++) {
		ASSERT(*p2 != '\0', "pcompar: name mismatch", ap1, n);
		p2++;
		n++;
	}
	ASSERT(n <= 5, "pcompar: length botch", ap1, n);
	if (n == 4 && (rc < -10 || rc > 10))
		return(rc);	/* Check for sequence number wrap-around */
	return(-rc);
}

/***
 *	gtwrkf - get next work file
 *	 Nfiles, Filent[] are modified.
 *
 *	return value:
 *
 *		0  - No file gotten
 *		1  - File successfully gotten.
 *
 */

/* LOCAL only */
gtwrkf(dir, file)
char *file, *dir;
{
	register char *p;
	register int i;

	if (Nfiles == 0)
		return(0);
	/* Find highest priority file in table */
	p = Filent[0];
	for (i = 1; i < Nfiles; i++) 
		if (pcompar(Filent[i], p) > 0)
			p = Filent[i];
	sprintf(file, "%s/%s", dir, p);
	strcpy(p, Filent[--Nfiles]);
	return(1);
}

/***
 *	gtwvec(file, dir, wkpre, wrkvec)	get work vector 
 *	char *file, *dir, *wkpre, **wrkvec;
 *
 *	return codes:
 *		positive number  -  number of arguments
 *		0 -  no arguments - fail
 */

/* EXTERNALLY CALLED */
int
gtwvec(file, dir, wkpre, wrkvec)
char *file, *dir, *wkpre, **wrkvec;
{
	register int nargs, n;

	n = 0;		/* Break possible infinite loop.  rti!trt */
	while ((nargs = anlwrk(file, wrkvec)) == 0) {
		if (++n > 3 || !iswrk(file, "get", dir, wkpre))
			return(0);
	}
	return(nargs);
}

/***
 *	iswrk(file, reqst, dir, pre)
 *	char *file, *reqst, *dir, *pre;
 *
 *	iswrk  -  this routine will check the work list (list).
 *	If it is empty or the present work is exhausted, it
 *	will call gtwrk to generate a new list.
 *	The "reqst" field will be the string "chk" or "get" to
 *	check for work, or get the next work file respectively.
 *
 *	return codes:
 *		0  -  no more work (or some error)
 *		1  -  there is work
 *
 */

/* EXTERNALLY CALLED */
int
iswrk(file, reqst, dir, pre)
register char *file, *reqst, *dir, *pre;
{
	static char *lastpre = 0;
	register ret;

	/* Starting new system; re-init */
	if (lastpre == 0 || strcmp(lastpre,pre) != 0) {
		anlwrk ("", (char **)0);	/* Force close of work file */
		bldflst (reqst, dir, "");    /* Force close old dir pointer */

		/* Save last worked-on prefix */
		if (lastpre != 0)
			free (lastpre);
		lastpre = malloc((unsigned)(strlen(pre)+1));
		strcpy (lastpre, pre);

		/* Set the external indexes properly
		 */
		Nfiles = 0;
	}

	/* If the list is empty or new files have entered
	 * the spool area, call "bldflst" to read
	 * some file names into it.  Because names can
	 * be put in the list that later turn out to
	 * be unusable (from "gtwrkf"), this operation
	 * continues until either "bldflst" can't find
	 * any new files, or "gtwrkf" signals success.
	 */
	for (;;) {
		ret = 0;
		if (Nfiles == 0 || newspool((time_t)TLIMIT))
			ret = bldflst (reqst, dir, pre);

		/* If they only wanted to check, return
		 * boolean list not empty.  NB: the list
		 * will be forcibly emptied as soon as
		 * a new system name is mentioned.
		 */
		if (*reqst == 'c') {
			return (ret);
		}

		if (Nfiles == 0)
			return(0);

		if (gtwrkf(dir, file))
			return (1);
	}
}

/* Return non-zero if there is new work in the spool
 * area since last check.  Assumes that if the sequence
 * file has been modified, there is new work. This is
 * not absolutely correct, but should be close enough.
 * Only checks every <limit> seconds at most.  Called
 * from "iswrk()" when a new work file is requested.
 */
/* LOCAL only */
int
newspool(limit)
time_t	limit;
{
	static time_t lastcheck = 0, lastmod = 0;
	time_t check;
	struct stat mod;
	int ret = 0;

	/* (void) */ time (&check);
	if (check - lastcheck > limit || lastcheck - check > limit) {
		mod.st_mtime = 0;
		/* (void) */ stat (SEQFILE, &mod);
		if (mod.st_mtime != lastmod)
			ret = 1;
		lastmod = mod.st_mtime;
	}
	lastcheck = check;
	return (ret);
}
