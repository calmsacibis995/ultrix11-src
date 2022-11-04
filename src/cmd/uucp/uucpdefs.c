
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)uucpdefs.c	3.0	4/22/86";

/*****************
 *  definitions of global variables
 ****************/

#include "uucp.h"

char Progname[10];
int Ifn, Ofn;
char Rmtname[16];
char User[16];
char Loginuser[16];
char Myname[16];
int Bspeed;
char Wrkdir[WKDSIZE];

char *Thisdir = THISDIR;
char Spoolname[MAXFULLNAME];
char *Spool = SPOOL;
#ifdef	UUDIR
char DLocal[16];
char DLocalX[16];
char *Dirlist[MAXDIRS];
int Subdirs;
#endif
int Debug = 0;
int Pkdebug = 0;
int Packflg = 0;
int Pkdrvon = 0;
long Retrytime;
short Usrf = 0;			/* Uustat global flag */
char Seqlock[MAXFULLNAME];
char Seqfile[MAXFULLNAME];
