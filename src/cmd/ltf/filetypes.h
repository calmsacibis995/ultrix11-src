
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)filetypes.h	3.0	4/21/86
 */
/**/
/*
 *
 *	File name:
 *
 *		filetypes.h
 *
 *	Source file description:
 *
 *		Include file for use by modules that are
 *		a part of the Labeled Tape Facility.		
 *			(LTF)
 *
 *	Functions:
 *
 *		Provide common definitions of general file types
 *		used during  LTF  operations.
 *
 *	Compile:
 *
 *		n/a
 *
 *	Modification history:
 *	~~~~~~~~~~~~~~~~~~~~
 *
 *	revision			comments
 *	--------	-----------------------------------------------
 *	  01.0		04-April-85	Ray Glaser
 *			Create orginal version.
 *	
 */

/*
 * ->	Global definitions of  LTF file TYPES
 */

#define	BINARY	10	/* File is some form of binary data */
#define	BLKSP	060000	/* Block Special File */
#define	CHCTRSP	020000	/* Character Special File */
#define COUNTED	-11	/* Counted record file */
#define CPIO	070707	/* CPIO data */
#define DD	02	/* Direct (image) dump file */
#define DIRECT	04	/* A directory file */
#define EMPTY	27	/* An empty file */
#define ENGLISH 28	/* English text */
#define FIXED	'F'	/* Fixed length record file */
#define	FUF	 1	/* File is a Fortran Unformatted File */
#define REGULAR	0100000 /* Regular ASCII byte stream) file */
#define SEGMENT 'S'	/* Segmented/spanned record file */
#define SOCKET	0140000	/* File is a socket */
#define SYMLNK	0120000	/* File is a symbolic link */
#define TEXT	-10	/* File is some variant of ASCII data */
#define UFORMAT 'U'	/* Undefined record file */
#define VARIABLE 'D'	/* Variable length record file */

/*E filetypes.h */

