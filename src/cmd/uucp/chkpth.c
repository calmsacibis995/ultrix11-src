
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)chkpth.c	3.0	4/22/86";

/*
 *  chkpth.c implements the security algorithms.
 *  Modified Doug Kingston, 30 July 82 to fix handling of	
 *	of the "userpath" structures.			
 * (brl-bmd) 
 * chkpth should not be called for implied Spool requests .
 * But explicit requests (foo!/usr/spoo/uucp/*) should use chkpth.
 * 
 *  decvax!larry - callcheck() checks to see if remote machine
 *		used a valed login id.
 *  decvax!larry - getxeqlevel() gets the execution level assigned
 *		to a remote machine.
 */


#include "uucp.h"
#include <sys/types.h>
#include <sys/stat.h>


#define DFLTNAME "default"	/* Not Implemented. ??? */

struct userpath {
	char *us_lname;
	char *us_mname;
	char us_callback;
	int  us_xeqlev;		/* remote execution level */
	char **us_path;
	struct userpath *unext;
};
struct userpath *Uhead = NULL;
struct userpath *Mchdef = NULL, *Logdef = NULL;
int Uptfirst = 1;


/*******
 *	chkpth(logname, mchname, path)
 *	char *path, *logname, *mchname;
 *
 *	chkpth  -  this routine will check the USERFILE for the
 *	machine or log name (non-null parameter) to see if the
 *	input path starts with an acceptable prefix.
 *
 *	return codes:  0  |  FAIL
 */

chkpth(logname, mchname, path)
char *path, *logname, *mchname;
{
	struct userpath *u;
	extern char *lastpart();
	char **p, *s;

	/* Allow only rooted pathnames.  Security wish.  rti!trt */
	if (*path != '/')
		return(FAIL);

	if (Uptfirst) {
		rdpth();
		ASSERT(Uhead != NULL, "INIT USERFILE, No entrys!", "", 0);
		Uptfirst = 0;
	}
	for (u = Uhead; u != NULL; ) {
		if (*logname != '\0' && strcmp(logname, u->us_lname) == SAME)
			break;
		if (*mchname != '\0' && strncmp(mchname, u->us_mname, 7) == SAME)
			break;
		u = u->unext;
	}
	if (u == NULL) {
		if (*logname == '\0')
			u = Mchdef;
		else
			u = Logdef;
		if (u == NULL)
			return(FAIL);
	}
	/* found user name */
	p = u->us_path;

	/*  check for /../ in path name  */
	for (s = path; *s != '\0'; s++) {
		if (prefix("/../",s))
			return(FAIL);
	}

	/* Check for access permission */
	for (p = u->us_path; *p != NULL; p++)
		if (prefix(*p, path))
			return(0);

	/* path name not valid */
	return(FAIL);
}


/***
 *	rdpth()
 *
 *	rdpth  -  this routine will read the USERFILE and
 *	construct the userpath structure pointed to by (u);
 *
 *	return codes:  0  |  FAIL
 *
 * 5/3/81 - changed to enforce the uucp-wide convention that system
 *	    names be 7 chars or less in length
 */

#define NO_XEQ_LEVEL -1

rdpth()
{
	char buf[100 + 1], *pbuf[50 + 1], *pc, **cp;
	FILE *uf;
	if ((uf = fopen(USERFILE, "r")) == NULL) {
		/* can not open file */
		ASSERT_NOFAIL(uf != NULL, "Can not open USERFILE", "", uf);
		return;
	}

	while (cfgets(buf, sizeof(buf), uf) != NULL) {
		int nargs, i;
		struct userpath *u;

		if ((u = (struct userpath *)malloc(sizeof (struct userpath))) == NULL) {
			DEBUG (1, "*** Userpath malloc failed\n", 0);
			fclose (uf);
			return;
		}
		if ((pc = calloc((unsigned)strlen(buf) + 1, sizeof (char)))
			== NULL) {
			/* can not allocate space */
			DEBUG (1, "Userpath calloc 1 failed\n", 0);
			fclose(uf);
			return;
		}

		strcpy(pc, buf);
		nargs = getargs(pc, pbuf);
		u->us_lname = pbuf[0];
		pc = index(u->us_lname, ',');
		if (pc != NULL)
			*pc++ = '\0';
		else
			pc = u->us_lname + strlen(u->us_lname);
		u->us_mname = pc;
		if (strlen(u->us_mname) > 7)
			u->us_mname[7] = '\0';
		if (strcmp(u->us_lname, "remote") == SAME) { 
			ASSERT(pbuf[1][0]=='X',"default xeq level undefined in USERFILE", u->us_lname, 0);
			Mchdef = u;
		}
		else
			if (strcmp(u->us_lname, "local") == SAME) {
				ASSERT(pbuf[1][0]=='X',"default xeq level undefined in USERFILE", u->us_lname, 0);
				Logdef = u;
			}
		i = 1;
		if (pbuf[1][0]=='X') {
			u->us_xeqlev = atoi(&pbuf[1][1]);
			i++;
		}
		else
			u->us_xeqlev = NO_XEQ_LEVEL;
/*
		DEBUG(5, "in chkpth, machine is:%s:, ", u->us_mname);
		DEBUG(5, "in chkpth, user is:%s:, ", u->us_lname);
		DEBUG(5, "xeqlevel is %d\n", u->us_xeqlev);
*/
		if (strcmp(pbuf[2], "c") == SAME) {
			u->us_callback = 1;
			i++;
			DEBUG(5,"callback for option on for user %s/n",
				u->us_lname);
		}
		else
			u->us_callback = 0;
		if ((cp = u->us_path =
		  (char **)calloc((unsigned)(nargs-i+1), sizeof(char *))) == NULL) {
			/*  can not allocate space */
			DEBUG (1, "Userpath calloc 2 failed!\n", 0);
			fclose(uf);
			return;
		}

		while (i < nargs)
			*cp++ = pbuf[i++];
		*cp = NULL;
		u->unext = Uhead;
		Uhead = u;
	}

	fclose(uf);
	ASSERT(Mchdef != NULL, "NO default in USERFILE for remote machines","",0);
	ASSERT(Logdef != NULL, "NO default in USERFILE for local users","",0);
	return;
}

/* routine to get execution level for a remote machine */
getxeqlevel(mch_name)
register char *mch_name;
{
	register struct userpath *u;

	if (Uptfirst) {
		rdpth();
		ASSERT(Uhead != 0, "INIT USERFILE, No entries!!", "", 0);
		Uptfirst = 0;
	}

	for (u = Uhead; u != NULL;) 
		if (strncmp(u->us_mname, mch_name, 7) == SAME) {
				DEBUG(4,"getxeqlevel %d\n",u->us_xeqlev);
			/*
			 * return default remote execution level if
			 * no execution level specified for this system
			 */
				if (u->us_xeqlev == NO_XEQ_LEVEL)
					return(Mchdef->us_xeqlev);
				return(u->us_xeqlev);
			}
			else 	{
				u = u->unext;
				continue;
				}
/*
 * machine name not found - return default execution level
 * 	for remote machines.
 * If the specified machine happens to be the local system
 * then return the default "local" execution level
 */
	if (strncmp(mch_name, Myname, 7) == SAME)
		return(Logdef->us_xeqlev); /* local xeq level returned */
	return(Mchdef->us_xeqlev);
}


/***
 *	chkperm(file, mopt)	check write permission of file
 *	char *mopt;		none NULL - create directories
 *
 *	if mopt != NULL and permissions are ok,
 *	a side effect of this routine is to make
 *	directories up to the last part of the
 *	filename (if they do not exist).
 *
 *	return 0 | FAIL
 */

chkperm(file, mopt)
char *file, *mopt;
{
	struct stat s;
	int ret;
	char dir[MAXFULLNAME];
	extern char *lastpart();

	if (stat(subfile(file), &s) == 0) {
		/* Forbid scribbling on a not-generally-writable file */
		/* rti!trt */
		if ((s.st_mode & ANYWRITE) == 0)
			return(FAIL);
		return(0);
	}

	strcpy(dir, file);
	*lastpart(dir) = '\0';
	if ((ret = stat(subfile(dir), &s)) == -1
	  && mopt == NULL)
		return(FAIL);

	if (ret != -1) {
		if ((s.st_mode & ANYWRITE) == 0)
			return(FAIL);
		else
			return(0);
	}

	/*  make directories  */
	return(mkdirs(subfile(file)));
}

/*
 * Check for sufficient privilege to request debugging.
 * Credit to seismo!stewart, John Stewart.
 */
chkdebug(uid)
int uid;
{
/* should put this somewhere else can't compile uustat */
/*
	if (uid > PRIV_UIDS) {
		fprintf(stderr, "Sorry, uid must be <= %d for debugging\n",
			PRIV_UIDS);
		cleanup(1);
		exit(1);
	}
*/
}




/*
 * check for callback and bad login/machine match
 *	log_name	-> login name
 *	mch_name	-> remote machine name
 * returns:
 *	0	-> no call back
 *	1	-> call back
 *	2	-> login/machine match failure
 */
callcheck(log_name, mch_name)
register char *log_name;
register char *mch_name;
{
	register struct userpath *u;
	register int i;
	int ret;
	int found_mch = 0;
	int found_log = 0;

	if (Uptfirst) {
		rdpth();
		ASSERT(Uhead != 0, "INIT USERFILE, No entries!!", "", 0);
		Uptfirst = 0;
	}

	for (u = Uhead; u != NULL;) {
		if (strncmp(u->us_mname, mch_name, 7) == SAME) {
			found_mch++; 
			/* now must get login/machine match */
			if (strcmp(u->us_lname, log_name) == SAME) {
				DEBUG(4,"callcheck1 %d\n",u->us_callback);
				return(u->us_callback);
			}
			else 	{
				u = u->unext;
				continue;
				}
		}
		if (u->us_mname[0] != '\0') {
			u = u->unext;
			continue;
		}

/* machine was null, check if login matches */

		if (strcmp(u->us_lname, log_name) != SAME) {
			u = u->unext;
			continue;
		}
		found_log++;
		if (found_mch) {
/* valid login wrong machine */
			u = u->unext;
			continue;
		}
		/* have found login name with null (default) machine name */
		DEBUG(4,"callcheck2 %d\n",u->us_callback);
		return(u->us_callback);
	}

	if (found_log==0 && found_mch==0) {
		struct stat statbuf;
		/*
	       	 * login has not been specifed in USERFILE 
	         * anything goes. 
	         */
		DEBUG(4, "login not in USERFILE - %s\n", log_name);
		if (stat("/usr/lib/uucp/INSECURE",&statbuf)==0)
			return(0);
		else
			return(2);
	}
	/*
	 * userid not found -- login/machine name do not match 
	 */
	return(2);
}


