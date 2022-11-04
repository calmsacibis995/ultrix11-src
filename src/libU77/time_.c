
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)time_.c	3.0	4/22/86
char id_time[] = "(2.9BSD)  time_.c  1.1";
 *
 * return the current time as an integer
 *
 * calling sequence:
 *	integer time
 *	i = time()
 * where:
 *	i will receive the current GMT in seconds.
 */

#include	<sys/types.h>
#include	"../libI77/fiodefs.h"

time_t time();

time_t time_()
{
	return(time((time_t *) 0));
}
