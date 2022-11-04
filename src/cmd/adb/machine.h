
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#
/*	sccsid: @(#)machine.h 3.0 4/21/86	*/
/*
 *	UNIX/INTERDATA debugger
 */

/* unix parameters */
#define PROMPT "adb> "
#define DBNAME "adb command error\n"
#define LPRMODE "%Q"
#define OFFMODE "+%o"
#define TXTRNDSIZ 8192L

TYPE	unsigned TXTHDR[8];
TYPE	unsigned SYMV;

/* symbol table in a.out file */
struct symtab {
	char	symc[8];
	char	symf;
	char	symo;
	SYMV	symv;
};
#define SYMTABSIZ (sizeof (struct symtab))

#define SYMCHK 047
#define SYMTYPE(symflg) (( symflg>=041 || (symflg>=02 && symflg<=04))\
				?  ((symflg&07)>=3 ? DSYM : (symflg&07))\
				: NSYM\
			)
