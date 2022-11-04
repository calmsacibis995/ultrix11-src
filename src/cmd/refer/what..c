
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
	SCCSID: @(#)what..c	3.0	4/22/86
 */
# include "stdio.h"
# include "ctype.h"
# include "sys/types.h"
# include "sys/stat.h"
# include "assert.h"
# define NFILES 100
# define NAMES 2000
extern exch(), comp();
struct filans {
	char *nm;
	long fdate;
	long size;
	int uid;
	};
extern struct filans files[NFILES];
extern char fnames[NAMES];
# define NFEED 5
