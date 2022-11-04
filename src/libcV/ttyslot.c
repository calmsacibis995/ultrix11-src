
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*	SCCSID: @(#)ttyslot.c	3.0	4/22/86	*/
/*	(System 5)	1.4	*/
/*	based on  ttyslot.c  1.2  */
/*LINTLIBRARY*/
/*
 * Return the number of the slot in the utmp file
 * corresponding to the current user: try for file 0, 1, 2.
 * Returns -1 if slot not found.
 */
#include <sys/types.h>
#include "utmp.h"
#define	NULL	0

extern char *ttyname(), *strrchr();
extern int strncmp(), open(), read(), close();

int
ttyslot()
{
	struct utmp ubuf;
	register char *tp, *p;
	register int s, fd;

	if((tp=ttyname(0)) == NULL && (tp=ttyname(1)) == NULL &&
					(tp=ttyname(2)) == NULL)
		return(-1);

	if((p=strrchr(tp, '/')) == NULL)
		p = tp;
	else
		p++;

	if((fd=open(UTMP_FILE, 0)) < 0)
		return(-1);
	s = 0;
	while(read(fd, (char*)&ubuf, sizeof(ubuf)) == sizeof(ubuf)) {
		if(  
		/*
		 * Don't check for these modes being present since ULTRIX-11
		 * login.c doesn't fill them in with anything (yet).
		 *  (ubuf.ut_type == INIT_PROCESS ||
		 *  ubuf.ut_type == LOGIN_PROCESS ||
		 *  ubuf.ut_type == USER_PROCESS ||
		 *  ubuf.ut_type == DEAD_PROCESS ) &&
		 */
			strncmp(p, ubuf.ut_line, sizeof(ubuf.ut_line)) == 0){
			(void) close(fd);
			return(s);
		}
		s++;
	}
	(void) close(fd);
	return(-1);
}
