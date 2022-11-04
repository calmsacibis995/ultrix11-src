
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)setrgid.c	3.0	4/22/86
 */

setrgid(rgid)
int rgid;
{
	return(setregid(rgid, -1));
}
