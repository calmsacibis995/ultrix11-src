
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)chown.c	3.0	4/22/86
 */
chown(name, owner, group)
char *name;
int owner, group;
{
	return(syscall(16, 0, 0, name, (group<<8)|(owner&0377), 0));
}
