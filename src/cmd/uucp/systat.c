
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)systat.c	3.0	4/22/86";


/*******
 *	systat(name, type, text)	make system status entry
 *	char *name, *text;
 *	int type.
 *
 *	return codes:  none
 */


/******************
 * Mods:
 *	decvax!larry - clear count when conversation completes successfully
 *		     - place STST. files in SPOOL/STST. directory
 *	  3/8/84     - put last successful connect time in STST. file
 *****************/

#include "uucp.h"
#include <sys/types.h>
#include <errno.h>


extern	time_t	time();
extern int errno;

#define STATNAME(f, n) sprintf(f, "%s/STST./%s.%.7s", SPOOL, "STST", n)
#define S_SIZE 150


systat(name, type, text)
char *name, *text;
int type;
{
	char filename[MAXFULLNAME], line[S_SIZE];
	int count;
	FILE *fp;
	time_t lasttime, prestime, successtime;
	long retrytime;

	line[0] = '\0';
	time(&prestime);
	count = 0;
	successtime = (time_t)0;
	STATNAME(filename, name);

	fp = fopen(filename, "r");
	if (fp != NULL) {
		fseek(fp, 0L, 0);
		fscanf(fp, "%*d%d%*ld%*ld%ld", &count, &successtime); 
		if (count <= 0)
			count = 0;
		fclose(fp);
	}
	fp = fopen(filename, "w+");
	ASSERT2(fp != NULL, "SYSTAT OPEN FAIL", filename, errno);
		

	switch(type) {
	case SS_FAIL:
		count++;
		break;
	case SS_COMPLETE:
		count = 0;
		break;
	case SS_INPROGRESS:
		time(&successtime);
		count = 0;
		break;
	case SS_CONN_OK:
		time(&successtime);
		break;
	}


	fseek(fp, 0L, 0 );
	chmod(filename, 0666);
	fprintf(fp, "%d %d %ld %ld %ld %s %s\n", type, 
		count, prestime, Retrytime, successtime, text, name);
	fclose(fp);
	return;
}

/***
 *	rmstat(name)	remove system status entry
 *	char *name;
 *
 *	return codes:  none
 */

rmstat(name)
char *name;
{
	char filename[MAXFULLNAME];

	STATNAME(filename, name);
	unlink(filename);
}

/***
 *	callok(name)	check system status for call
 *	char *name;
 *
 *	return codes  0 - ok | >0 system status
 */

callok(name)
char *name;
{
	char filename[MAXFULLNAME], line[S_SIZE];
	FILE *fp;
	time_t lasttime, prestime;
	long retrytime;
	int count, type;

	STATNAME(filename, name);
	fp = fopen(filename, "r");
	if (fp == NULL)
		return(SS_OK);

	if (fgets(line, S_SIZE, fp) == NULL) {
		/*  no data  */
		fclose(fp);
		unlink(filename);
		return(SS_OK);
	}

	fclose(fp);
	time(&prestime);
	sscanf(line, "%d%d%ld%ld", &type, &count, &lasttime, &retrytime);

	switch(type) {
	case SS_BADSEQ:
	case SS_CALLBACK:
	case SS_NODEVICE:
	case SS_INPROGRESS:	/*let LCK take care of it */
		return(SS_OK);

	case SS_FAIL:
		if (count > MAXRECALLS) {
			logent("MAX RECALLS", "NO CALL");
			DEBUG(4, "MAX RECALL COUNT %d\n", count);
			return(type);
		}

		if (prestime - lasttime < retrytime) {
			logent("RETRY TIME NOT REACHED", "NO CALL");
			DEBUG(4, "RETRY TIME (%d) NOT REACHED\n", (long)  RETRYTIME);
			return(type);
		}

		return(SS_OK);
	default:
		return(SS_OK);
	}
}
