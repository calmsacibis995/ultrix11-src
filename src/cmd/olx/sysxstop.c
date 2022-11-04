
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)sysxstop.c	3.0	4/22/86";
/*
 * Program to kill off exercisers that are not running under
 * sysx control. This is necessatated by the addition of job control
 * and the csh which do funny thing with process groups.
 *
 * Bill Burns 5/23/80
 *
 */

#include <signal.h>

main()
{
	killpg(31111, SIGQUIT);
	sleep(3);
}
