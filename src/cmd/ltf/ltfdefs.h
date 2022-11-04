
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)ltdefs.h
 *
 */
/**/
/*
 *
 *	File name:
 *
 *		ltfdefs.h
 *
 *	Source file description:
 *
 *		Contains common defintions of GLOBAL constants,
 *		structures, and external routine/variable
 *		declarations for the Labeled Tape Facility (LTF).
 *
 *
 *
 *	Functions:
 *
 *		n/a
 *
 *	Usage:
 *
 *		n/a
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
 *	  01.0		10-April-85	Ray Glaser
 *			Create orginal version.
 *	
 */
/**/
/*	-----------------
 * +-->  LOCAL  INCLUDES  <--+
 *	-----------------
 *
 * 	Remaining "local" includes.. Other than this module so
 *	that every LTF module does not have to do repetative
 *	includes.
 */

#include "ltferrs.h"	/* LTF error message macros and messages */
#include "filetypes.h"	/* LTF file type definitions */


/*		----------------
 * +-->  	SYSTEM  INCLUDES		<--+
 *
 *	UNCONDITIONAL  and  CONDITIONAL
 *	--------------------------------
 */

#include	<a.out.h>
#include	<ctype.h>
#include	<stdio.h>
#include	<sys/errno.h>
#include	<sys/ioctl.h>
#include	<sys/param.h>	/* Defines system params, MAXPATHLEN */
#include	<sys/types.h>
#include	<sys/stat.h>	/* NOTE: stat.h depends on types.h */
#include	<sys/mtio.h>	/* NOTE: mtio.h depends on types.h */

/*
 * +-->	Match correct versions of include files and structures
 *	to the defined Ultrix system "type" (32, 32m, 11, PRO)..
 */

#ifndef U11
/*
 * +-->  FOR  ULTRIX 32/32m  SYSTEMS  <--+
 */

#include	<sys/time.h>
#include	<sys/dir.h>	/* NOTE: dir.h defpends on ?? */

#else

/*
 * +-->  FOR  ULTRIX-11 / PRO  SYSTEMS  <--+
 */

#include	<ndir.h>
#include	<time.h>
#define MAXPATHLEN 256
#endif
/**/
/*	------------------------------------
 * +-->  UNCONDITIONAL  GLOBAL  DEFINITIONS  <--+
 *	------------------------------------
 */
	/*	+-->  NOTE <--+
	 *	      ----
	 * The following defintions  MUST COME
	 * before all others. Further definitions
 	 * are dependant on their value(s).
	 */

/*_MUST_COME_FIRST_*/
	/*
	 * Insert position dependant definitions here..
	 */
/*_END_OF_MUST_COME_FIRST_*/

/*_A_*/
#define ALL	3

struct ALINKBUF	{	/* Link table used when writing files
			 * to tape. Contains information
			 * about all hard linked files. */
	ino_t	a_inum;		/* inode number */
	dev_t	a_dev;		/* device */
	int	a_fsecno;	/* file section number */
	int	a_fseqno;	/* file sequence number */
	char	*a_pathname;	/* real file name */ 
	struct	ALINKBUF *a_next; /* Point to next in chain */

};/*E struct ALINKBUF */

#define A_SPECIALS 21	/* Number of allowable special characters
			 * in an "a"-chctr string */
/*_B_*/
#define	BINARY	10	/* File is some form of binary data */
#define	BLKSP	060000	/* Block Special File */
#define BUFSIZE	80	/* ANSI tape label buffer size */

/*_C_*/
#define	CHCTRSP	020000	/* Character Special File */
#define CREATE	1	/* Create a new tape - function flag */
#define COUNTED	-11	/* Counted record file */
#define CPIO	070707	/* CPIO data */

/*_D_*/
#define DD	02	/* Direct (image) dump file */
#define DELNL	1	/* Delete new line function */

struct DIRE {
	dev_t	rdev;		/* device */
	ino_t	inode;		/* inode number */
	struct	DIRE *dir_next;	/* Next in list or 0= end */
};

#define DIRECT	04	/* A directory file */

/*_E_*/
#define EMPTY	27	/* An empty file */
#define ENGLISH 28	/* English text */
#define EXTRACT	3	/* Extract data from  tape - function flag */

/*_F_*/
#define FALSE	0	/* BOOLEAN value for error conditions */
#define FAIL	-1	/* Exit with failure status */
struct	FILESTAT	{
	char	*f_src;		/* On output, contains the file 
				 * name only, minus path name */
	int	f_numleft;	/* f_numleft is initially set 
				 * to a non-zero value. 
				 * When the first occurance of the
				 * named file is found, f_numleft is
				 * set to zero to terminate searching
				 * the input volume for further copies
				 * of the file. Wild cards may be
				 * used to extract all copies of a
				 * given file with possible rename of
				 * duplicates by the user if the -W
				 * switch is given. Else, the last
				 * copy found is the final result.
				 * When wild cards are used, f_numleft
				 * remains non-zero.
				 */
	int	f_found;	/* Number of occurances of f_src found.
				 * Used because wild cards make it hard
				 * to determine if any files were found
				 * on the input volume unless we have
				 * this flag because f_numleft stays
				 * non-zero for a wildcard file name
				 * on extract.
				 */
	int	f_flags;	/* Used when extracting files to
				 * indicate if the file should be
				 * directly dumped (dd) from tape,
				 * "fuf" converted, or extracted
				 * normally.
				 */
	struct	FILESTAT *f_next;/* Point to next in chain */

};/*E struct FILESTAT */

#define FIRST	1
#define FIXED	'F'	/* Fixed length record file */
#define FOREVER for (;;)/* A section of code executed until
			 * forcefully exited by a break, goto,
			 * system exit() call, or etc...
			 */
#define	FUF	 1	/* File is a Fortran Unformatted File */

/*_G_*/
/*_H_*/
/*_I_*/
#define IGNORE_ERRORS 0	/* General pupose flag */

/*_J_*/
/*_K_*/

/*_L_*/
#define LAST	2

/*_M_*/
#ifndef U11
#define MAXBLKSIZE 20480 /* Maximum tape block size for reading
			  * Ultrix-32 */
#define MAXBLKWRT 20480 /* Maximum tape block size for writing
			 * Ultrix-32 */
#else
#define MAXBLKSIZE 10240 /* Maximum tape block size for reading
			  * Ultrix-11 */
#define MAXBLKWRT 2048 /* Maximum tape block size for writing
			 * Ultrix-11 */
#endif

#define MAXRECFUF 130
#define MAXRECSIZE 512	/* Maximum tape record size */
#define MAXREC4	126
#define MAXREC6	124
#define MIDDLE	0
#define MINBLKSIZE 18	/* Minimum tape block size */

/*_N_*/
#define NASC	128	/* Number of ASCII characters recognized
			 * by Filetypes.c */
#define NO	0	/* BOOLEAN constant */

/*_O_*/
/*_P_*/
#define PAD	'^'	/* Padding character */

/*_Q_*/

/*_R_*/
#define RECOFF	6	/* fuf record offset */
#define REGULAR	0100000 /* Regular ASCII byte stream) file */
#define REPORT_ERRORS 1	/* General purpose flag */
#define RGRP	040
#define ROTH	04
#define ROWN	0400

/*_S_*/
#define SEGMENT 'S'	/* Segmented/spanned record file */
#define SGID	02000
#define SOCKET	0140000	/* File is a socket */
#define STXT	01000
#define SYMLNK	0120000	/* File is a symbolic link */
#define SUCCEED 0	/* Exit with success status */
#define SUID	04000

/*_T_*/
#define TABLE	4	/* Table of contents - function flag */
#define TEXT	-10	/* File is some variant of ASCII data */
#define TM 023		/* A tape mark */
#define TRUE	1	/* BOOLEAN value for non-error indication */
#define TRUNCATE 2	/* Truncate a string function */

/*_U_*/

/*_V_*/
#define VARIABLE 'D'	/* Variable length record file */

/*_W_*/
#define WGRP	020
#define WOWN	0200
#define WOTH	02
#define WRITE	2

/*_X_*/
#define XGRP	010
#define XOTH	01
#define XOWN	0100

struct XLINKBUF	{
	/* Link table used when reading an input volume.
	 * Contains information about all potential hard link files
	 * that have been extracted.  The hard link file is determined 
	 * by a match on the file sequence number that exists in HDR2
	 * and the hlink field which is set when the volume is created 
	 * indicating that a file has hard links.
	 */
	int	x_fseqno;	  /* File sequence number */
	int	x_fsecno;	  /* File section number */
	char	*x_pathname;	  /* Real file name */
	struct	XLINKBUF *x_next; /* Point to next in chain */

};/*E struct XLINKBUF */

/*_Y_*/
#define YES	1	/* BOOLEAN constant */

/*_Z_*/

/**/
/*	----------------------------------------------------
 * +-->  CONDITIONALIZED  GLOBAL DEFINITIONS / DECLARATIONS  <--+
 *	----------------------------------------------------
 *
 */

#ifndef VARSC	/* If we are compiling any module other than
		 * "ltfvars.c" (which allocates the actual
		 * storage) - define as external data.
		 */

/*_A_c_*/
extern	char	Ansiv;	/* ANSI Version Number of volume */
extern	struct	ALINKBUF *A_head; /* Header for list of linked files */
extern	char	A_specials[A_SPECIALS+1]; /* Array of special chctrs
					     * allowed in an "a"-chctr
					     * string. */
/*_B_c_*/
extern	char	*Bb;		/* FUF buffer (block) pointer */
extern	long	Blocks;		/* Number of tape blocks read/written */
extern	int	Blocksize;	/* Tape transfer block size */

/*_C_c_*/
extern	char	Carriage;	/* Carriage control attribute field
				 * from/to HDR2 label.
				 */
extern	char	ch;		/* One character temporary variable */
extern	char	*cp, *cp2;	/* Temporary character pointers */

/*_D_c_*/
extern	int	Days[2][13];	/* Days in the months table */
extern	int	Dfiletype;	/* Current default/user designated
				   file type for an output function */
extern	struct	DIRE *Dhead;	/* Linked list of directorys on vol */
extern	int	Dircre;		/* Flags wether a directory had to
				 * be created on extract.
				 */
extern	struct	stat Dnode;	/* Inode structure for directories */
extern	char	Dummy[80];	/* Dummy area for string scans */
extern	int	Dverbose;	/* True if user requested display of
				 * volume directory information.
				 */
/*_E_c_*/
extern	int	errno;		/* Error status for system routines */
extern	int	error_status;	/* Error status for expnum */

/*_F_c_*/
extern	struct	FILESTAT *F_head;/* Link list pointer to filestats */
extern	char	Format;		/* Record format, either F, D, or S */
extern	int	Fsecno;		/* fsecno of next file to append */
extern	int	Fseqno;		/* fseqno of next file to append */
extern	int	Func;		/* The users' requested function */

/*_G_c_*/

/*_H_c_*/
extern	char	Hostname[21];	/* Host name from/to HDR3 */

/*_I_c_*/
extern	char	IMPID[14];	/* LTF Implementation ID - Used to
				 * recognize whether or not a 
				 * Volume was created by Ultrix */
extern	struct	stat Inode;	/* Inode structure */
extern	int	i;		/* Fast integer variable */

/*_J_c_*/
extern	int	j;		/* Fast integer variable */

/*_K_c_*/

/*_L_c_*/
extern	char	Labelbuf[BUFSIZE+1]; /* Work area for ANSI labels */
extern	int	Leofl;		/* Number (0-9) identifying the
				 * last eof label used to contain
				 * an Ultrix path name component.
				 */
extern	int	Lhdrl;		/* Number (3-9) identifying the
				 * last hdr label used to contain
				 * an Ultrix path name component.
				 */
extern struct FILESTAT * Lookup(); /* Looks up a given ANSI volume
				    * file name among user input
				    * file arguments. */
extern	int	L_blklen;	/*[05] block length -
			 	 * number of chctrs per block */
extern	char	L_crecent;	/*[01] creation date century */
extern	char	L_credate[6];	/*[05] creation date */
extern	char	L_expirdate[6];	/*[06] expiration date */
extern	char	L_filename[18];	/*[17] file identifier */
extern	int	L_fsecno;	/*[04] file section number */
extern	int	L_fseqno;	/*[04] file sequence number */
extern	int	L_gen;		/*[04] generation number */
extern	int	L_genver;	/*[02] genration vrsn number */
extern	char	L_labid[4];	/*[03] label identifier -
				 *     "HDR" or "EOF" */
extern	int	L_labno;	/*[01] label number */
extern	long	L_nblocks;	/*[06] block count */
extern	char	L_ownrid[15];	/*[14] owner id */
extern	int	L_reclen;	/*[05] record length */
extern	char	L_recformat;	/*[01] record format */
extern	char	L_systemid[14];	/* System ID of ANSI volume creator */
extern	char	L_volid[7];	/*[06] volume identifier
			 	 * (tape label) 
			 	 * (file set identifier) */

/*_M_c_*/
extern	char	Magtdev[MAXPATHLEN+1]; /* Output device name string */
extern	FILE	*Magtfp;	/* File pointer to output device */
extern	int	Maxrec;		/* Maximum record size */
extern	char	*Months[];	/* Months in the year table */
extern	int	M1[];
extern	int	M2[];
extern	int	M3[];
extern	int	M4[];
extern	int	M5[];
extern	int	M6[];
extern	int	M7[];
extern	int	M8[];
extern	int	M9[];
extern	int	*M[];

/*_N_c_*/
extern	char	Name[MAXPATHLEN+1];   /* File name extracted from 
				       * volume */
extern	int	NMEM4D;		/* Flag set if no memory for directory
				 * list */
extern	int	Nodir;		/* Flags output of directory blocks */
extern	int	Noheader3;	/* Flags whether user wants to use
				 * ANSI HDR3, etc.. data. */
extern	int	Nosym;		/* Flags output of symbolic link file */
extern	int	Numrecs;	/* Number of files to manipulate */
extern	int	numdir;		/* Number of directories in chdir */

/*_O_c_*/
extern	char	Owner[15];	/* Owner of volume and file id string */

/*_P_c_*/
extern	int	permission;	/* permission flag used if super user on
				 * extract for chown and chmod */
extern	char	*Progname;	/* Global pointer to our invoked name */

/*_Q_c_*/

/*_R_c_*/
extern	char	*Rb;		/* FUF record pointer */
extern	int	Reclength;	/* Record length of tape file(s) */
extern	int	Rep_misslinks;	/* Report, not_report undumped links */

/*_S_c_*/
extern	int	Secno;		/* Position volume to this ANSI file
				 * section number before beginning
				 * requested operation(s).
				 */
extern	int	Seqno;		/* Position volume to this ANSI file
				 * sequence number before beginning
				 * requested operation(s).
				 */
extern	int	skip;		/* If TRUE, skip non-existent filenames
				 * in inputfile */
extern	char	Spaces[80];	/* A string of spaces for blank fills */
extern	int	Symlf;		/* Flags whether a symbolic link was
				 * encountered during i/o process. */

/*_T_c_*/
extern	int	Tape;		/* Tape flag - True or False depending 
				 *  on the LTF i/o device being used */
extern	char	Tape_Mark[10];	/* Dummy tape mark used for
				 * non-tape devices */
extern	char	Tftypes[4];	/* True Ultrix disk file type as an
				 * ASCII string for HDR2 */
extern	int	Toggle;		/* Toggle switch for file types */
extern	int	Type;		/* Unix disk file type */

/*_U_c_*/
extern	int	Ultrixvol;	/* Flags wether or not an input volume
				 * was created by an Ultrix system.
				 */
extern	int	Use_versnum;	/* Interpret version numbers on
				 * tape files */
/*_V_c_*/
extern	int	Verbose;	/* Degree of desired verbosity */
extern	char	Volid[7];	/* Volume ID (ANSI name of) */
extern	int	Volmo;		/* Flags input volume announced and
				 * init input volume logic done.
				 */
/*_W_c_*/
extern	int	Warning;	/* Warnings (y/n) on file overwrites */
extern	int	Wildc;		/* Used by mstrcmp() to indicate if a
				 * wildcard was seen in a matched 
				 * string.
				 */
/*_X_c_*/
extern	long	xtractf();	/* Function returns a long value */
extern	char	Xname[MAXPATHLEN+1]; /* Alternate disk file name for 
				      * a file name as extracted from 
				      * the volume */
extern	struct	XLINKBUF *X_head;/* Head pointer to linked file(s)
				  * structures */
/*_Y_c_*/
/*_Z_c_*/

#endif VARSC

/**\\**\\**\\**\\**\\**  EOM  ltfdefs.h  **\\**\\**\\**\\**\\*/
/**\\**\\**\\**\\**\\**  EOM  ltfdefs.h  **\\**\\**\\**\\**\\*/


