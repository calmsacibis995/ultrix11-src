
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)seteuid.c	3.0	4/22/86
 */

seteuid(euid)
int euid;
{
	return(setreuid(-1, euid));
}
