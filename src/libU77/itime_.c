
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)itime_.c	3.0	4/22/86
char id_itime[] = "(2.9BSD)  itime_.c  1.1";
 *
 * return the current time in numerical form
 *
 * calling sequence:
 *	integer iarray(3)
 *	call itime(iarray)
 * where:
 *	iarray will receive the current time; hour, min, sec.
 */

#include	"../libI77/fiodefs.h"
#include	<sys/types.h>
#include	<time.h>

itime_(iar)
struct { ftnint ihr; ftnint imin; ftnint isec; } *iar;
{
	struct tm *localtime(), *lclt;
	time_t time(), t;

	t = time(0);
	lclt = localtime(&t);
	iar->ihr = (ftnint) lclt->tm_hour;
	iar->imin = (ftnint) lclt->tm_min;
	iar->isec = (ftnint) lclt->tm_sec;
}
