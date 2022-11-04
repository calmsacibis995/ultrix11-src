
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)cksize.c	3.0	4/22/86
 *
 * This file contains things that cannot be xstr'd
 */
#include <nlist.h>

struct	nlist	nl[] =
{
	{ "_realmem" },
	{ "" },
};

/*
 * routine to check if memory size of current machine allows
 * running of /usr/lib/sendmail.  Memory requirements are > 256KB
 * or else, we just look like ucbmail instead.
 * Global flag "sm_toobig" is set to 1 if cannot run SENDMAIL.
 */

#include "rcv.h"
unsigned int	realmem;	/* realmem size in clicks */

checktoobig()
{
	int mem;

	sm_toobig = 0;	/* initialize */

	if(nlist("/unix", nl) < 0) {
	    if (debug)
		printf("\nMail: cannot access namelist in /unix!\n");
	    return(1);
	}
	if(nl[0].n_value == 0) {
	    if (debug)
		printf("\nMail: cannot get realmem symbol from /unix namelist!\n");
	    return(1);
	}
	if((mem = open("/dev/mem", 0)) < 0) {
	    if (debug)
		printf("\nMail: cannot open /dev/mem for reading!\n");
	    return(1);
	}
	if (nl[0].n_value) {
	    lseek(mem, (long)nl[0].n_value, 0);
	    read(mem, (char *)&realmem, sizeof(realmem));
	} else {
	    if (debug)
		printf("Mail: cannot get realmem size from kernel!\n");
	    sm_toobig=0;	/* assume sendmail can run */
	    return(1);
	}
	if (realmem <= 4096) {	/* 256KB */
	    if (debug)
		printf("\nMail: /usr/lib/sendmail too big!  Using \"/bin/mail -d\" for delivery.\n");
	    sm_toobig=1;	/* cannot run sendmail -- too big */
	}
	return(0);
}
