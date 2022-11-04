
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)us_rrs.c	3.0	4/22/86";

 
/*
 * We get the job number from a command file "cfile".
 * using the jobn as the key to search thru "R_stat"
 * file and modify the corresponding status as indicated
 * in "stat".	"Stat" is defined in "uust.h".
 * return:
 *	0	-> success
 *	FAIL	-> failure
 */

/****************
 * Mods:
 * 	decvax!larry - store data in binary format
 ****************/

#include "uucp.h"
#ifdef UUSTAT
#include <sys/types.h>
#include "uust.h"



long	ftell();
us_rrs(cfilel,stat)
char *cfilel;
short stat;
{
	FILE	*fp;
	register short i;
	struct us_rsf u;
	char cfile[20],  *lxp, *name, buf[BUFSIZ];
	char *strcpy();
	long	pos;
	long time();
	short n;
 
	/*
	 * strip path info
	 */
	strcpy(cfile, lastpart(cfilel));
	DEBUG(9, "\nenter us_rrs, cfile: %s", cfile);
	DEBUG(9, "  request status: %o\n", stat);
	
	/*
	 * extract the last 4 digits
	 * convert to digits
	 */
	name = cfile + strlen(cfile) - 4;  
	for(i=0; i<=15; i++) {
		if (ulockf(LCKRSTAT, 15) != FAIL) 
			break;
		sleep(1);
	}
	if (i > 15) {
		DEBUG(3, "ulockf of %s failed\n", LCKRSTAT);
		return(FAIL);
	}
	if ((fp = fopen(R_stat, "r+")) == NULL) {
		DEBUG(3, "fopen of %s failed\n", R_stat);
		rmlock(LCKRSTAT);
		return(FAIL);
	}
	while(fread(&u, sizeof(u), 1, fp) != NULL){
		if (strncmp(name, u.jobn,4) == SAME) {
			u.jobn[4] = '\0';
			DEBUG(6, " jobn : %s\n", u.jobn);

			pos = ftell(fp);
			u.ustat = stat;
			u.stime = time((long *)0);
			fseek(fp, pos-(long)sizeof(u), 0);
			fwrite(&u, sizeof(u), 1, fp);
			break;
		}

	}
	fflush(fp);
	fclose(fp);
	rmlock(LCKRSTAT);
	return(FAIL);
}
#endif
