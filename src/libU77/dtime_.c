
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)dtime_.c	3.0	4/22/86
char id_dtime[] = "(2.9BSD)  dtime_.c  1.1";
 *
 * Returns the delta time since the last call to dtime.
 *
 * calling sequence:
 * 	real time(2)
 * 	call dtime(time)
 * where:
 * 	the 2 element array time will receive the user and system
 * 	elapsed time since the last call to dtime, or since the start
 * 	of execution.
 *
 * This routine can be called as function, and returns the sum of
 * user and system times. The time_array argument must always be given.
 *
 * The resolution for all timing is 1/60 second.
 */

#include <sys/types.h>
#include <sys/times.h>

struct tb { float usrtime; float systime; };

time_t dutime=0, dstime=0;

float
dtime_(dt) struct tb *dt;
{	struct tms clock;

	times(&clock);
	dt->usrtime = (float)(clock.tms_utime - dutime) / 60.0;
	dt->systime = (float)(clock.tms_stime - dstime) / 60.0;
	dutime = clock.tms_utime;
	dstime = clock.tms_stime;
	return(dt->usrtime + dt->systime);
}
