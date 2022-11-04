
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)odir.h	3.0	4/22/86
 */

#define	ODIRSIZ	14

struct	olddirect {
	ino_t	od_ino;
	char	od_name[ODIRSIZ];
};

#define	OENTSIZ ((sizeof(struct direct)-(MAXNAMLEN+1)) + ((ODIRSIZ+1+3) & ~3))
#define	RDSZ	((DIRBLKSIZ/OENTSIZ)*sizeof(struct olddirect))
