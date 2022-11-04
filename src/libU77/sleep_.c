
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)sleep_.c	3.0	4/22/86
char id_sleep[] = "(2.9BSD)  sleep_.c  1.1";
 *
 * sleep for awhile
 *
 * calling sequence:
 *	call sleep(seconds)
 * where:
 *	seconds is an integer number of seconds to sleep (see sleep(3))
 */

#include	"../libI77/fiodefs.h"

sleep_(sec)
ftnint *sec;
{
	sleep((unsigned int)*sec);
}
