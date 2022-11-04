
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)setruid.c	3.0	4/22/86
 */

setruid(ruid)
int ruid;
{
	return(setreuid(ruid, -1));
}
