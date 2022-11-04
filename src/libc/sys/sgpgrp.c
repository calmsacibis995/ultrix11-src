/* SCCSID: @(#)sgpgrp.c	3.0	4/22/86	*/

/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * System III/V Style s/getpgrp
 *
 * uses the code in spgrp.s to access the real [sg]etpgrp syscall
 */

setpgrp()
{
	spgrp(0,getpid());
	return(getpgrp());
}

getpgrp()
{
	return(spgrp(0,-1));
}
