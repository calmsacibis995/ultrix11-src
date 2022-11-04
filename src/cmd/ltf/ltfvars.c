
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#ifndef lint
static	char	*sccsid = "@(#)ltfvars.c	3.0	(ULTRIX)	4/21/86";
#endif	lint

/**/
/*
 *
 *	File name:
 *
 *		ltfvars.c
 *
 *	Source file description:
 *
 *		Contains common definitions and GLOBAL declarations
 *		for variables used by the Labeled Tape Facility
 *		(LTF).
 *
 *			*-*  NOTE  *-*
 *			     ----
 *	When adding a new variable, do not forget to also include
 *	an external declaration in  ltfdefs.h  so that all other
 *	modules may access the new variable(s).
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
 *	  01.0		14-April-85	Ray Glaser
 *			Create orginal version.
 *	
 */
/**/
/*			*-*  NOTE  *-*
 *			     ----
 *	The following "define"  MUST  preceed the  include  ltfdefs.h
 *	for the reasons given below.
 */

#define	VARSC	/* So that ltfdefs.h knows that we are compiling
		 * the variables module and actually compiles
		 * (allocates) variable storage space where
		 * required.
		 */

/*	-----------------
 * +-->  LOCAL  INCLUDES  <--+
 *	-----------------
 */

#include	"ltfdefs.h"	/* Common GLOBAL constants, etc */

/*	----------------------
 * +-->  GLOBAL  DECLARATIONS  <--+
 *	----------------------
 */

/*_A_*/
struct	ALINKBUF *A_head; /* Allocate a pointer to start of links
			   * list for appending file(s) function */
char	Ansiv = '4';	/* ANSI Version number of volume.
			   (default value for output, set to actual
			    on input). User may specify version 3
			    output to override default. */
char	A_specials[A_SPECIALS+1] =" !\"%&\'()*+,-_./:;<=>?";

/*_B_*/
char	*Bb;		/* "fuf"  buffer (block) pointer */
long	Blocks;		/* Number of tape blocks read/written */
int	Blocksize = 2048; /* Tape default blocksize  - for
			   * non-tape devices, sb a multiple
			   * of 80 bytes. ie. 2080 is good.
			   * Else, the dummy tape marks used
			   * in/by odm.c don't work so good. */

/*_C_*/
char	ch;		/* Temporary character variable */
char	*cp, *cp2;	/* Temporary character pointers */
char	Carriage = ' ';	/* Carriage control attribute from/to
			 * HDR2 label.
			 */
/*_D_*/
int Days[2][13] =
	{
	{ 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
	{ 0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
	};
struct	DIRE	*Dhead;	/* Head pointer for directorys on the
			 * volume list.
			 */
int	Dircre	= 0;	/* Flags wether a directory had to be
			 * created on extract.
			 */
int	Dfiletype = 0;	/* Current default/user designated
			   file type for an output function. */
struct	stat Dnode;	/* Inode structure for directories */
char	Dummy[MAXPATHLEN+1];/* Dummy area for string scans and
			   * general label i/o work.
			   */
int	Dverbose = 0;	/* Set true if we want to see the directory
			 * information on the volume.
			 */

/*_E_*/
int	error_status;	/* error status for expnum */

/*_F_*/
struct  FILESTAT *F_head; /* Allocate a pointer to start of
			   * append file list. */
char	Format;		/* Format for records, F, D, or S */
int	Fsecno;		/* fsecno of next file to append */
int	Fseqno;		/* fseqno of next file to append */
int	Func;		/* Function (crxt) */

/*_G_*/

/*_H_*/
char	Hostname[21];	/* Hostname from/to HDR3 */

/*_I_*/
char	IMPID[14] = "DECULTRIX0000"; /* Ultrix system implementation 
				      * id */
struct	stat Inode;		/* Inode structure */
int	i;			/* Temporary integer */

/*_J_*/
int	j;			/* Temporary integer */

/*_K_*/

/*_L_*/
char	Labelbuf[BUFSIZE+1];	/* Work area for r/w ANSI labels */
int	Leofl = 0;	/* Last EOF label number used to contain a
			 * component of an Ultrix path name.
			 */
int	Lhdrl = 3;	/* Last HDR label number used to contain a
			 * component of an Ultrix path name.
			 */
int	L_blklen;	/*[05] block length -
			 * number of chctrs per block */
char	L_crecent;	/*[01] creation date century */
char	L_credate[6];	/*[05] creation date */
char	L_labid[4];	/*[03] Label identifier (HDR or EOF) */
int	L_labno;	/*[01] label number */
char	L_expirdate[6];		/*[06] expiration date */
char	L_filename[18];	/*[17] file identifier */
int	L_fsecno;	/*[04] file section number */
int	L_fseqno;	/*[04] file sequence number */
int	L_gen;		/*[04] generation number */
int	L_genver;	/*[02] genration vrsn number */
long	L_nblocks;	/*[06] block count */
char	L_ownrid[15];	/*[14] owner id */
char	L_recformat;	/*[01] record format */
int	L_reclen;	/*[05] record length */
char	L_systemid[14];	/* Identifies the ID of the system
			 * that recorded the file */
char	L_volid[7];	/*[06] volume identifier
			 *     (tape label) 
			 *     (file set identifier) */

/*_M_*/
#ifndef U11
char	Magtdev[MAXPATHLEN+1] = "/dev/rmt8"; /* Default i/o device name 
					      * for Ultrix-32 */
#else
char	Magtdev[MAXPATHLEN+1] = "/dev/rht0"; /* Default i/o device name 
					      * for Ultrix-11 */
#endif
FILE	*Magtfp;	/* File pntr to output device */
int	Maxrec;		/* Maximum record size */
char	*Months[] =
	{ 0, "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug",
	"Sep", "Oct", "Nov", "Dec" };

int	M1[] = { 1, ROWN, 'r', '-' };
int	M2[] = { 1, WOWN, 'w', '-' };
int	M3[] = { 2, SUID, 's', XOWN, 'x', '-' };
int	M4[] = { 1, RGRP, 'r', '-' };
int	M5[] = { 1, WGRP, 'w', '-' };
int	M6[] = { 2, SGID, 's', XGRP, 'x', '-' };
int	M7[] = { 1, ROTH, 'r', '-' };
int	M8[] = { 1, WOTH, 'w', '-' };
int	M9[] = { 2, STXT, 't', XOTH, 'x', '-' };
int	*M[] = { M1, M2, M3, M4, M5, M6, M7, M8, M9 };

/*_N_*/
char	Name[MAXPATHLEN+1];/* File name as extracted from volume */
int	NMEM4D = 0;	/* No memory for directories flag clear */
int	Nodir = FALSE;	/* Do not omit directory blocks from output */
int	Noheader3 = FALSE; /* Indicates user does not want to
			    * use ANSI  HDR3, HDR9 data. If present
			    * on input, data will be discarded.
			    */
int	Nosym = FALSE;	/* Output the file with symbolic link */
int	Numrecs = 0;	/* Number of records to process */
int	numdir = 0;	/* Number of directories to chdir */

/*_O_*/
char	Owner[15];	/* Owner of vol and file name string */

/*_P_*/
int	permission = FALSE; /* permission flag used if super user
			     * on extract for chown and chmod */
char	*Progname;	/* Program name for error messages */

/*_Q_*/

/*_R_*/
char	*Rb;		/* "fuf"  record pointer */
int	Reclength = MAXRECSIZE;	/* Default record length */
int	Rep_misslinks = YES;	/* Reports of undumped links
				 * are suppressed if the
				 *  -M flag is set */

/*_S_*/
int	Secno=0;	/* Position the volume to this ANSI file
			 * section number before beginning
			 * requested operation(s).
			 */
int	Seqno=0;	/* Position the volume to this ANSI file
			 * sequence number before beginning
			 * requested operation(s).
			 */
int	skip = FALSE;	/* Set skip to false and assume stop when
			 * checking inputfile for non-existent files */
char	Spaces[80];	/* A string of spaces for blank fills */
int	Symlf = FALSE;	/* Flags whether or not a Symbolic Link
			 * was encountered during i/o processing.
			 */
/*_T_*/
int	Tape = TRUE;	/* Flags (if TRUE) that i/o device is tape,
			 * else (FALSE) is not. */
char	Tape_Mark[10]	= "TAPE_MARK";
char	Tftypes[4]	= "???"; /* True Ultrix disk file type as
				  * an ASCII string for HDR2 */
int	Toggle = NO;	/* Toggles file type specs */
int	Type;		/* Unix disk file type */

/*_U_*/
int	Ultrixvol = YES;/* Flags wether or not the input volume was
			 * thought to be created by an Ultrix system.
			 */
int	Use_versnum = NO; /* Indicates whether to interpret
			   * version numbers  from input file
			   * names (-N flag). */
/*_V_*/
int	Verbose = NO;	/* Verbose flag */
char	Volid[7] = "ULTRIX"; /* Tape label (ANSI VOL ID)
				(default for output, set to actual
				 on input) */
int	Volmo	= NO;	/* Indicates we need to announce the volume
			 * info and set init flags on input.
			 */
/*_W_*/
int	Warning = NO;	/* Default mode is to overwrite
			 * existing files without any
			 * notification. 
			 * (normal Unix procedure)
			 * The user may request that a
			 * warning message be issued &
			 * verification request issued before
			 * overwrting the file by setting the
			 * -W qualifier in the command. */

int	Wildc= FALSE;	/* Used to indicate if Wildcards were
			 * seen in a string given to mstrcmp()
			 */

/*_X_*/
struct	XLINKBUF *X_head; /* Allocate a pointer to start of
			   * the extracted "links" list. */
char	Xname[MAXPATHLEN+1]; /* Alternate disk file name for a file
				name extracted from volume. */

/*_Y_*/
/*_Z_*/

/**\\**\\**\\**\\**\\**  EOM  ltfvars.c  **\\**\\**\\**\\**\\*/
/**\\**\\**\\**\\**\\**  EOM  ltfvars.c  **\\**\\**\\**\\**\\*/
