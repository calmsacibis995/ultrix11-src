
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)uucpdefs.h	3.0	4/22/86
 */
char *Thisdir = THISDIR;
char *Spool = SPOOL;
char *Myname = MYNAME;
int Debug = 0;
int Pkdebug = 0;

char *Sysfiles[] = {
	SYSFILE,
	SYSFILECR,
	NULL
};
char *Devfile = DEVFILE;
char *Dialfile = DIALFILE;
int Packflg = 0;
int Pkdrvon = 0;
