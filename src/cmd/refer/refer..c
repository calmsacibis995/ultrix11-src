
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
	Sccsid: @(#)refer..c	3.0	4/22/86
 */
# include "stdio.h"
# include "ctype.h"
# include "assert.h"
extern FILE *in;
extern int endpush, labels, sort, bare, keywant;
extern char *smallcaps;
extern char comname;
extern char *keystr;
extern int authrev;
extern int nmlen, dtlen;
extern char *data[], **search;
extern int refnum;
extern char *reftable[];
extern char *rtp, reftext[];
extern int sep;
extern char tfile[];
extern char gfile[];
extern char ofile[];
extern char hidenam[];
extern char *Ifile; extern int Iline;
extern FILE *fo, *ftemp;
# define FLAG 003
# define NRFTXT 2000
# define NTFILE 20
# define NRFTBL 200
# define LLINE 512
# define QLEN 300
# define ANSLEN 1000
# define TAGLEN 400
# define NSERCH 20
