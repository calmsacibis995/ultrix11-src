
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * 18-Dec-85	tar.h	tar include file
 */
/*
 * SCSSID: @(#)tar.h	3.0	(ULTRIX-11)	4/22/86
 */
#ifdef PRO
#define U11
#endif

/*
 *
 *	Modification/Revision history:
 *	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 
 *	revision			comments
 *	--------	-----------------------------------------------
 *
 *	17.x		Ray Glaser, 18-Dec-85
 *			Create orginal version
 *
 *
 *	File name:
 *
 *		tar.h
 *
 *	Source file description:
 *
 *		This file contains variable & constant
 *		declarations & definitions for the
 *		various tar source modules.
 */
/*.sbttl Includes, #defines, & Structure declarations */
 
/*	Generic includes..
 */
#include <stdio.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/dir.h>
#include <grp.h>
#include <pwd.h>
#include <signal.h>
#include <errno.h>
#include <a.out.h>

/*
 *
 *	Ultrix-32  specific includes...
 */
#ifndef U11
#include <sys/ioctl.h>
#include <sys/time.h>
#endif

#ifdef DEBUG
#include "mtio.h"
#else
#include <sys/mtio.h>
#endif

/*	typedefs - for better readability.
 */

typedef		int	COUNTER;
typedef		int	COUNT_INDEX;
#ifndef PRO
typedef	short	int	FLAG;
#else
typedef		int	FLAG;
#endif
typedef		int	FILE_D;
typedef		int	INDEX;
typedef		int	SIZE_I;
typedef		long	SIZE_L;
typedef		char	*STRING_POINTER;
/*
 *
 *	Ultrix-11 Specific includes...and auxilary definitions
 */
#ifdef U11
#include <time.h>

#ifdef PRO
#define	rmdir	unlink
#define S_IFLNK 0120000
#include <sys/ioctl.h>
#endif

/*	ACCESS(2) 
 */
#define MAXPATHLEN 256
#endif

/*-------*\
 Constants
\*-------*/

#define A_WRITE_ERR	0
#define	FAIL	-1
#define	FALSE	0
#define	MAXARCHIVE	99
#define	N	200
#define NAMSIZ	100	/* Maximum length a path/file name allowed
			 * in a tar header block.
			 */

#define NBLOCK	20	/* Number of TBLOCKs blocked for one 
			 * read/write call.
			 */

/* Symbolically define various "types" of tar headers liable to be
 * encountered.
 *			NOTE:
 *
 *	The logic depends on form "OTA" being the lowest
 *	defined value.
 */
#define OTA 1	/* File was written by a very old tar. Does not
 		 * contain any fields defined after linkname[].
		 * ie. Original Tar Archive format.
 		 */
#define OUA 2	/* File was written by an older version of Ultrix tar.
		 * ie. The field following linkname[] may contain
		 * special device (major/minor) numbers.
 		 */
#define UGS 3	/* File was written by a tar conforming to the
 		 * User group standard. Contains fields up to
		 * UEGdummy[] in the header.
 		 */
#define UMA 4	/* File was written by Ultrix tar using multi-archive
		 * extension fields in the header.
		 */

#define	RGRP	040
#define	ROTH	04
#define	ROWN	0400
#define	SGID	02000
#define	SUCCEED	1
#define	SUID	04000
#define	STXT	01000
#define TBLOCK	512	/* Size of a tape block */
#define TCKSLEN 8
#define TDEVLEN 8
#define TGIDLEN 8
#define TGNMLEN 32
#define TMAGIC  "ustar  "
#define TMAGLEN 8
#define TMTMLEN 12
#define TMODLEN 8
#define TRUE	1
#define TSIZLEN 12
#define TUIDLEN 8
#define TUNMLEN 32
#define TVLEN 3
#define	WGRP	020
#define	WOTH	02
#define	WOWN	0200
#define	XGRP	010
#define	XOTH	01
#define XOWN	0100

/* Values used in the "typeflag" field of tar header block
 */
#define REGTYPE	 '0'	/* Regular file */
#define AREGTYPE ' '	/* Regular file */
/* binary 0		   Regular file */
#define LNKTYPE  '1'	/* Hard link */
#define SYMTYPE  '2'	/* Symbolic link */
#define CHRTYPE  '3'	/* Character special */
#define BLKTYPE  '4'	/* Block special */
#define DIRTYPE  '5'	/* Directory */
#define FIFOTYPE '6'	/* FIFO special */
#define CONTTYPE '7'	/* Contiguous file */

/*--------*\
 Structures 
\*--------*/

#ifdef TARmln

/*	OLD style tar header format.
 *	(+ Ultrix added  rdev[6]  field for special devices)
 */
#if 0
 union hblock {
 	char dummy[TBLOCK];
	struct header {
		char name[NAMSIZ];
		char mode[8];
 		char uid[8];
 		char gid[8];
 		char size[12];
 		char mtime[12];
 		char chksum[8];
 		char linkflag;
 		char linkname[NAMSIZ];
 		char rdev[6];
 	} dbuf;
 };
#endif 
/*
 *	NEW TAR HEADER FORMAT:     User Group Standard
 *				 + Ultrix extensions for multi-archive.
 */
union hblock {
	char dummy[TBLOCK];
	struct header {
		char name[NAMSIZ];	/* Pathname of file */
		char mode[TMODLEN];	/* Permissions/modes of file */
		char uid[TUIDLEN];	/* User ID of file owner */
		char gid[TGIDLEN];	/* Group ID of file owner */
		char size[TSIZLEN];	/* Number of bytes in file */
		char mtime[TMTMLEN];	/* Modification time of file */
		char chksum[TCKSLEN];	/* Checksum of header values */
		char typeflag;		/* Specifies this files' type 
					 * Formerly named -> linkflag */
		char linkname[NAMSIZ];	/* Linked-to file name */
		/*
		 * Point of departure from very old original
		 * tar  header format and older Ultrix format.
		 * Start of User Group standard extension fields.
		 */
		char magic[TMAGLEN];	/* Value == TMAGIC to identify
					 * new archive format */
					/* Is rdev[6] field for older
					 * Ultrix archive formats */
		char uname[TUNMLEN];	/* File owner user name */
		char gname[TGNMLEN];	/* File owner group name */
					/* Next 2 fields apply only to
					 * device special files. */
		char devmajor[TDEVLEN];	/* Major device number */
		char devminor[TDEVLEN];	/* Minor device number */
		/*
		 * Point of departure from User Group standard
		 * extension fields.
		 * Start of  ULTRIX  multi-archive extension fields.
		 */
		char UEGdummy[1];/* Dummy to align Ultrix fields  */
				 /* Archive numbers are ASCII 01 - 99 */
		char carch[TVLEN];/* Number of this (current) archive */
		char oarch[TVLEN];/* Orginal archive # on which this
				  * file was begun */
		char org_size[TSIZLEN];/* Original size of file if
					* this is a continued file. */
	} dbuf;
};
/*
 * As an EOA (End Of Archive) indicator, tar will write the above form
 * of a directory block with these changes to identify it as an EOA.
 * The EOA block is written AFTER the 2 normal zero blocks that older
 * versions of tar use to indicate the end of archive. This is done
 * in order to prevent them crashing when reading a multi-archive
 * archive produced by this version of tar/mdtar.
 *
 *	a. The name field will contain the name of the file
 *	   that has been "split" across an archive.
 *
 *	b. All other fields will contain ASCII zeroes (as opposed
 *	   to real zero bytes) to flag this as the EOA record.
 *
 *	c. The last archive of a set contains an EOA block filled
 *	   with actual zeroes to indicate the end of the set.
 */

struct linkbuf {
	int	count;
	dev_t	devnum;
	ino_t	inum;
	struct	linkbuf	*nextp;
	char	pathname[NAMSIZ];
};

struct DIRE {
		dev_t	rdev;		/* device for directory */
		ino_t	inode;		/* i-node number of file */
	struct	DIRE	*dir_next;	/* Pointer to next entry*/
};
/*.sbttl GLOBAL variable declarations */

/*--------------*\
 Global Variables ~ Globals
\*--------------*/

char	Archive[8] = "archive";
FLAG	AFLAG	= 0;
FLAG	Bflag	= 0;
FLAG	bflag	= 0;
SIZE_I	blocks;
SIZE_L	blocks_used = 0L;
daddr_t	bsrch();
SIZE_L	bytes;
union	hblock	cblock;
COUNT_INDEX	CARCH = 1;/* Current archive number reading/writting.
			   */
char	CARCHS[3]  = "1";
FLAG	cflag	= 0;
SIZE_L	chctrs_in_this_chunk;
int	chksum;
SIZE_L	cmtime;
SIZE_L	corgsize;
SIZE_L	cremain;
COUNTER	dcount1 = 0;
COUNTER	dcount2 = 0;
COUNTER	dcount3 = 0;
struct	DIRE	*Dhead;	/* Head pointer to list of directories */
char	DIRECT[13]  = " (directory)";
union	hblock	dblock;
FLAG	DFLAG	= 0;
FLAG	dflag	= 0;
char	eoa[4]	= "EOA";
FLAG	EOTFLAG = 0;	/* End Of Tape flag */
FLAG	EODFLAG = 0;	/* End Of Disk flag */
extern	int	errno;
SIZE_L	extracted_size = 0L;/* Size of a file extracted/tabled so far.
			     * Used when dealing
			     * with files split across archives.
			     */
FLAG	FEOT	= 0;	/* EOT detected in flushtape() */
FLAG	FILE_CONTINUES = 0;
FLAG	Fflag	= 0;
FLAG	fflag	= 0;
char	file_name[NAMSIZ+1];
FLAG	first	= 0;
int	found = 0;
STRING_POINTER	getcwd();
STRING_POINTER	getwd();
struct	passwd	*getpwnam();
struct	group	*getgrgid();
struct	group	*getgrnam();
struct	group	*gp;
FLAG	hdrtype	= UMA;	/* Defines the type of tar header block.
			 * See above defines or getdir();
			 */
FLAG	header_flag = 0;/* Normally true (1) when doing a multi-archive
			 * output to cause a continuation header for
			 * the current file to be written on the next
			 * archive. When a file EXACTLY fills the currnt
			 * physical media, this flag is set false (0)
			 * to inhibit the writting of a continuation
			 * header on the next archive as a NEW file will
			 * start fresh on the next archive.
			 */
FLAG	HELP;
FLAG	hflag	= 0;
daddr_t	high;
char	hdir[NAMSIZ];
FLAG	iflag	= 0;
struct linkbuf	*ihead;
struct linkbuf *lp;
COUNTER	lcount1 = 0;
COUNTER	lcount2 = 0;
char	iobuf[TBLOCK];
FLAG	lflag	= 0;
daddr_t	low;

#ifndef U11
	/* Ultrix-32/32m default device name string.
	 */
	char	magtape[13] = "/dev/rmt8\0\0\0";
#endif

#ifdef U11
#ifdef PRO
	/* Pro-350 default device name string.
	 */
	char	magtape[11] = "/dev/rrx1\0";
#else
	/* Ultrix-11 default device name string.
	 */
	char	magtape[10] = "/dev/rht0";
#endif
#endif

char	*malloc();
SIZE_I	MAXAR = MAXARCHIVE;
FLAG	MDTAR	= 0;	/* Flags whether or not we are invoked
			 * with the name "MDTAR" so that the
			 * code can assume that the output device
			 * is likely to be a diskette.
			 * The code is sensitive to whether the
			 * output device is a tape or diskette.
			 */
char	mdtar[6] =  "mdtar";
FLAG	MFLAG = 0;
FLAG	mflag = 0;
FILE_D	mt;
struct	mtget	mtsts;
struct	mtop	mtops;
FLAG	MULTI	= 0;
SIZE_I	nblock	= NBLOCK; /* Number of blocks user desires in a single
			   * read/write system call.  */
FLAG	new_file;
FLAG	nextvol;
FLAG	NFLAG	= 0;	/* Set -> No multi-archive or file-splitting
			 * across archives functions please.
			 * notset -> Perform multi-archive and
			 * file splitting across archives (default).
			 */
int	njab;
char	NULS[1]	= "";
FLAG	NMEM4D	= 0;
FLAG	NMEM4L	= 0;
FLAG	OARCH	= 0;	/* Original archive number the current file
			 * began on. Used for multi-archive file
			 * operations.
			 */
FLAG	OFLAG = 0;
FLAG	oflag = 0;
int	onhup();
int	onintr();
int	onquit();
int	onterm();
SIZE_L	original_size = 0L;/* Original size of a file if split
			    * across archives
			    */
FLAG	pflag	= 0;
FLAG	pipein	= 0;	/* Flag set if input from pipe. */
char	*progname;
FLAG	PUTE	= 0;	/* Set when putting end of archive (empty)
			 * blocks out at end of all data 
			 */
INDEX	recno	= 0;
SIZE_L	remaining_chctrs;/* Number of chctrs in the file that have not
			  * yet been placed on the output archive.
			  */
SIZE_L	written = 0L;	 /* Number of chctrs of the file written
			  */
FLAG	rflag	= 0;
char	*rindex();
FLAG	SFLAG	= 0;	/* Flags that we want to output User Group
			 * standard archive format
			 */
FLAG	sflag	= 0;
SIZE_L	size_of_media[MAXARCHIVE+1];/* Default # of blocks on the device
				 * Zero assumes unlimited space.
				 */
char	*sprintf();
COUNTER	start_archive = 1;	/* Number of the archive to start with
				 * on output. Primarily intended to be
				 * used for error recovery when dumping
				 * a lot of files across many media.
				 */
struct	stat	stbuf;
char	*strcat();
char	*strfind();
union	hblock	*tbuf;
FLAG	term	= 0;
FILE	*tfile;
FLAG	tflag	= 0;
time_t	modify_time;
char	tname[15] = "/tmp/tarXXXXXX";
FLAG	unitflag = 0;	/* Flags that a unit # has been given */
char	unitc;		/* Unit number character. Used so that we can
			 * accept unit numbers and device character
			 * specifiers in any order */
char	*usefile;
FLAG	VFLAG	= 0;	/* Big VERBOSE  requested flag */
FLAG	vflag	= 0;	/* Little verbose requested flag */
FLAG	volann	= -1;	/* Flags wether or not tar has announced the
			 * format / archive # of the current archive.
			 */
FLAG	VOLCHK	= 0;
char	wdir[NAMSIZ];
FLAG	wflag	= 0;
FLAG	xflag	= 0;
#endif
/*.sbttl External declarations for overlay modules */

#ifndef TARmln
extern union hblock {
	char dummy[TBLOCK];
	struct header {
		char name[NAMSIZ];	/* Pathname of file */
		char mode[TMODLEN];	/* Permissions/modes of file */
		char uid[TUIDLEN];	/* User ID of file owner */
		char gid[TGIDLEN];	/* Group ID of file owner */
		char size[TSIZLEN];	/* Number of bytes in file */
		char mtime[TMTMLEN];	/* Modification time of file */
		char chksum[TCKSLEN];	/* Checksum of header values */
		char typeflag;		/* Specifies this files' type 
					 * Formerly named -> linkflag */
		char linkname[NAMSIZ];	/* Linked-to file name */
		/*
		 * Point of departure from very old original
		 * tar  header format and older Ultrix format.
		 * Start of User Group standard extension fields.
		 */
		char magic[TMAGLEN];	/* Value == TMAGIC to identify
					 * new archive format */
					/* Is rdev[6] field for older
					 * Ultrix archive formats */
		char uname[TUNMLEN];	/* File owner user name */
		char gname[TGNMLEN];	/* File owner group name */
					/* Next 2 fields apply only to
					 * device special files. */
		char devmajor[TDEVLEN];	/* Major device number */
		char devminor[TDEVLEN];	/* Minor device number */
		/*
		 * Point of departure from User Group standard
		 * extension fields.
		 * Start of  ULTRIX  multi-archive extension fields.
		 */
		char UEGdummy[1];/* Dummy to align Ultrix fields  */
				 /* Archive numbers are ASCII 01 - 99 */
		char carch[TVLEN];/* Number of this (current) archive */
		char oarch[TVLEN];/* Orginal archive # on which this
				  * file was begun */
		char org_size[TSIZLEN];/* Original size of file if
					* this is a continued file. */
	} dbuf;
};
struct linkbuf {
	int	count;
	dev_t	devnum;
	ino_t	inum;
	struct	linkbuf	*nextp;
	char	pathname[NAMSIZ];
};
struct DIRE {
		dev_t	rdev;
		ino_t	inode;
	struct	DIRE	*dir_next;
};

/*--------------*\
 Global Variables ~ Globals
\*--------------*/

extern	char	Archive[8];
extern	FLAG	AFLAG;
extern	FLAG	Bflag;
extern	FLAG	bflag;
extern	SIZE_I	blocks;
extern	SIZE_L	blocks_used;
extern	daddr_t	bsrch();
extern	SIZE_L	bytes;
extern	union	hblock	cblock;
extern	COUNT_INDEX	CARCH;
extern	char	CARCHS[3];
extern	FLAG	cflag;
extern	SIZE_L	chctrs_in_this_chunk;
extern	int	chksum;
extern	SIZE_L	cmtime;
extern	SIZE_L	corgsize;
extern	SIZE_L	cremain;
extern	COUNTER	dcount1;
extern	COUNTER	dcount2;
extern	COUNTER	dcount3;
extern	struct	DIRE	*Dhead;
extern	char	DIRECT[13];
extern	union	hblock	dblock;
extern	FLAG	DFLAG;
extern	FLAG	dflag;
extern	char	eoa[4];
extern	FLAG	EOTFLAG;
extern	FLAG	EODFLAG;
extern	int	errno;
extern	SIZE_L	extracted_size;
extern	FLAG	FEOT;
extern	FLAG	FILE_CONTINUES;
extern	FLAG	Fflag;
extern	FLAG	fflag;
extern	char	file_name[NAMSIZ+1];
extern	FLAG	first;
extern	int	found;
extern	STRING_POINTER	getcwd();
extern	STRING_POINTER	getwd();
extern	struct	passwd	*getpwnam();
extern	struct	group	*getgrgid();
extern	struct	group	*getgrnam();
extern	struct	group	*gp;
extern	FLAG	hdrtype;
extern	FLAG	header_flag;
extern	FLAG	HELP;
extern	FLAG	hflag;
extern	daddr_t	high;
extern	char	hdir[NAMSIZ];
extern	FLAG	iflag;
extern	struct linkbuf	*ihead;
extern	struct linkbuf *lp;
extern	COUNTER	lcount1;
extern	COUNTER	lcount2;
extern	char	iobuf[TBLOCK];
extern	FLAG	lflag;
extern	daddr_t	low;

#ifndef U11
	/* Ultrix-32/32m default device name string.
	 */
extern	char	magtape[13];
#endif

#ifdef U11
#ifdef PRO
	/* Pro-350 default device name string.
	 */
	char	magtape[11];
#else
	/* Ultrix-11 default device name string.
	 */
	char	magtape[10];
#endif
#endif
extern	char	*malloc();
extern	SIZE_I	MAXAR;
extern	FLAG	MDTAR;
extern	char	mdtar[6];
extern	FLAG	MFLAG;
extern	FLAG	mflag;
extern	FILE_D	mt;
extern	struct	mtget	mtsts;
extern	struct	mtop	mtops;
extern	FLAG	MULTI;
extern	SIZE_I	nblock;
extern	FLAG	new_file;
extern	FLAG	nextvol;
extern	FLAG	NFLAG;
extern	int	njab;
extern	char	NULS[1];
extern	FLAG	NMEM4D;
extern	FLAG	NMEM4L;
extern	FLAG	OARCH;
extern	FLAG	OFLAG;
extern	FLAG	oflag;
extern	int	onhup();
extern	int	onintr();
extern	int	onquit();
extern	int	onterm();
extern	SIZE_L	original_size;
extern	FLAG	pflag;
extern	FLAG	pipein;
extern	char	*progname;
extern	FLAG	PUTE;
extern	INDEX	recno;
extern	SIZE_L	remaining_chctrs;
extern	int	revwhole;
extern	int	revdec;
extern	SIZE_L	written;
extern	FLAG	rflag;
extern	char	*rindex();
extern	FLAG	SFLAG;
extern	FLAG	sflag;
extern	SIZE_L	size_of_media[MAXARCHIVE+1];
extern	char	*sprintf();
extern	COUNTER	start_archive;
extern	struct	stat	stbuf;
extern	char	*strcat();
extern	char	*strfind();
extern	union	hblock	*tbuf;
extern	FLAG	term;
extern	FILE	*tfile;
extern	FLAG	tflag;
extern	time_t	modify_time;
extern	char	tname[15];
extern	FLAG	unitflag;
extern	char	unitc;	
extern	char	*usefile;
extern	FLAG	VFLAG;
extern	FLAG	vflag;
extern	FLAG	volann;
extern	FLAG	VOLCHK;
extern	char	wdir[NAMSIZ];
extern	FLAG	wflag;
extern	FLAG	xflag;

#endif
