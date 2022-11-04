
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#include "machine.h"
/*	sccsid: @(#)mode.h 3.0 4/21/86	*/
/*
 *	UNIX debugger
 *
 *	Modified for overlay text support,
 *	changes flagged by the word OVERLAY
 *
 *	Fred Canter 3/23/82
 */

#define MAXCOM	64
#define MAXARG	32
#define LINSIZ	1024
TYPE	int		INT;
TYPE	int		VOID;
TYPE	long int	L_INT;
TYPE	float		REAL;
TYPE	double		L_REAL;
TYPE	unsigned	POS;
TYPE	char		BOOL;
TYPE	char		CHAR;
TYPE	char		*STRING;
TYPE	char		MSG[];
TYPE	struct map	MAP;
TYPE	MAP		*MAPPTR;
TYPE	struct symtab	SYMTAB;
TYPE	SYMTAB		*SYMPTR;
TYPE	struct symslave SYMSLAVE;
TYPE	struct bkpt	BKPT;
TYPE	BKPT		*BKPTR;


/* file address maps */
struct map {
	L_INT	b1;
	L_INT	e1;
	L_INT	f1;
	L_INT	b2;
	L_INT	e2;
	L_INT	f2;
	INT	ufd;
};


/* slave table for symbols */
struct symslave {
	SYMV	valslave;
	L_INT	osmslave;	/* OVERLAY / map (core)  overlay real address */
	char	typslave;
	char	ovnslave;
	int	cntslave;
};

struct bkpt {
	INT	loc;
	INT	ins;
	INT	count;
	INT	initcnt;
	INT	flag;
	INT	ovly;
	CHAR	comm[MAXCOM];
	BKPT	*nxtbkpt;
};

TYPE	struct reglist	REGLIST;
TYPE	REGLIST		*REGPTR;
struct reglist {
	STRING	rname;
	INT	roffs;
};

struct {
	INT	junk[2];
	INT	fpsr;
	REAL	Sfr[6];
};

struct {
	INT	junk[2];
	INT	fpsr;
	L_REAL	Lfr[6];
};

