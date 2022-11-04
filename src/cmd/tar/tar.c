
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#ifndef lint
static	char	*sccsid = "@(#)tar.c	3.0	(ULTRIX)	4/22/86";
#endif	lint

#ifdef PRO
#define U11
#endif

#define TARmln 1

/*
 *
 *
 *	Modification/Revision history:
 *	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
int revwhole = 19;
int revdec   =  1; /* Current revision # of tar.
			   * Printed out when the "V" switch is given.
 *
 *	revision			comments
 *	--------	-----------------------------------------------
 *
 *	19.1		Ray Glaser, 28-Jan-86
 *			Do not allow non-super-user setting of pflag
 *
 *	19.0		Ray Glaser, Nov-85 - Jan-86
 *			Install multi-archive logic
 *			& /usr/group standard header logic
 *
 *	15.3		Ray Glaser, 24-Sep-85
 *			Correct the "-f" switch logic further. ie.
 *			Preserve the original modes/ownership of a
 *			named output file and do not allow overwrite
 *			of an existing write-protected "-f" output
 *			file. Also remove an extraneous carriage
 *			return on output that caused script problems
 *			and checksum errors when going over the net.
 *
 *	15.2		Ray Glaser, 09-Sep-85
 *			Correct bug in -f switch logic that allows
 *			root to destroy special files. ie. A check
 *			to see if a special file is named vs. a
 *			regular file.
 *
 *	15.1		Ray Glaser, 26-Aug-85
 *			Change output format of several items to 
 *			avoid breaking scripts.
 *			1. On extract - print  blocks  vs. block(s)
 *			2. Put the "file" type indicator on modes
 *			   info under the big VERBOSE switch.
 *			   NOTE: This is for Ultrix-32 only. Ultrix-11
 *			   does it the way we have illustrated below.
 *
 *				ie.  drwx------
 *				     crw-------
 *				     brw------- 
 *				     -rwx------
 *
 *				will come out as --
 *
 *				     rwx-------  etc..
 *			3. Put print of file size in blocks under
 *			   big VERBOSE switch.
 *
 *				ie..  720/001  for bytes/blocks
 *
 *				will be -
 *
 *				      720
 *
 *			4. Change print of uid/gid to print minimum
 *			   number of characters required.
 *
 *			   ie. It was printing  0/00   when the old
 *			   version had printed  0/0
 *
 *
 *	15.0		Ray Glaser, 29-Jul-85
 *			Fix a "broken pipe" problem when tar'ing out
 *			to stdout and tar'ing back in thru stdin, tar
 *			would "sometimes" exit with a broken pipe error.
 *			Due to a race condition when shutting down the
 *			pipe and the fact that the end of archive detect
 *			logic was not reading "both" eoa (all zeroes)
 *			blocks from the input tar, it would sometimes
 *			break the pipe.
 *
 *	14.0		Ray Glaser, 24-Jul=85
 *			Correct extraneous error message print out
 *			when extracting linked files. Caused by a
 *			small oversite when applying fix for 
 *			IPR-00014.
 *
 *	13.0		Ray Glaser, 10-Jul-85
 *			Fixed the bug reported in IPR-00014.
 *			If a tar archive has a symbolic or hard which
 *			had the same name as an existing non-empty
 *			directory, an extraction of the link from the
 *			archive as  root  would unlink the directory from
 *			the system. All files in that directory would
 *			be allocated but unreferenced.
 *			This corrupts the filesystem and  fsck  must be 
 *			run to repair it. Tar now outputs an error
 *			message and the directory is not removed.
 *
 *	12.0		Ray Glaser, 10-Jul-85
 *			Fixed a	 bug identified by IPR-00006.
 *			If the same linked file was output twice by
 *			the user -
 *
 *			ie.	ln  alpha beta
 *			tar cvf tarfile alpha beta alpha
 *			tar xvf tarfile
 *
 *			Would produce error message that alpha was not
 *			found when the extract was done. The fix ignores
 *			subsequent occurances of alpha & outputs a
 *			message to the effect that the linked file has
 *			already been output & is being skipped.
 *
 *	11.0		Ray Glaser, 01-Jul-85
 *			Allow user to specify both  c and r  switch in
 *			the same command without complaint
 *			 (as it used to be).
 *			Fixed a bug that caused tar to hang in "bread"
 *			routine	when zero byte read was encountered.
 *			Zero byte read indicates either eoa  or
 *			that an error occured on the read and no
 *			data is available.
 */

/*
 *	Modification history: (cont)
 *	~~~~~~~~~~~~~~~~~~~~
 *
 *	10.0		Ray Glaser, 22-May-85
 *			Fix bug in -p switch logic so that super user
 *			can restore original file ownership.
 *
 *	 09.1		Ray Glaser, xx-Mar-85
 *			Minor debug corrections to new code.
 *
 *	 09		Ray Glaser, 13-Mar-85
 *			Install conditionals and code for Ultrix-11.
 *
 *	 08		Ray Glaser, 15-Feb-85
 *			Install code for MDTAR function.
 *
 *	 07		Ray Glaser, 05-Nov-84
 *			- Replaced the 'k' switch with the 'h' switch.
 *			-or- Put tar back the way it was from Berkeley
 *			vis: links....
 *
 *	 06		Ray Lanza,  21-Aug-84
 *			- removed special case MicroVAX code.
 *
 *	 05		Greg Tarsa, 25-Apr-84
 *			- Fixed flushtape() to check status of final
 *			write to make sure that it succeeds.
 *
 *	 04		Greg Tarsa, 16-Apr-84
 *			- if MVAXI is defined then the default archive
 *			device 	will be	  /dev/rrx1.
 *
 *	 03		Greg Tarsa, 2-Mar-84
 *			- Changed -h flag to be the -k (keep symbolic
 *			links) flag.
 *
 *	 02		Greg Tarsa, 28-Nov-83
 *			- Fixed a bug that allowed SCCS files to be
 *			  extracted when FF was specified.
 *			- Made F[FF] to work with all command functions.
 *			- Code installed to allow -[cr]FFF to exclude
 *			  executable files.  This is tested, but
 *			  conditionalized out because the  -t & -x
 *			  options need more code to read magic numbers
 *			  from the file on tape.
 *
 *	 01		Greg Tarsa, 18-Nov-83
 *			- Fixed usage message to show proper functions.
 *			- Added units 2,3,6 to program.
 *			- Added miscellaneous comments.
 *			- Reversed sense of the -h flag (symlinks are
 *			  now followed by default)
 */

/*
 *
 *	File name:
 *
 *		tar.c
 *
 *	Source file description:
 *
 *		Contains root code for tar (if overlayed)
 *
 *	Functions:
 *
 *	main		Entry from system. Top level command
 *			processing and user error detection.
 *
 *	onhup,		Routines to catch "signals" and set the
 *	 onintr,	termination flag (term).
 *	  onquit,
 *	   onterm
 *
 *--------------------------
 */

#include "tar.h"

/*.sbttl main()  Main Line Logic */

/* Function:
 *
 *	main
 *
 * Function Description:
 *
 *	This function is entered from the system.
 *
 * Arguments:
 *
 *	int	argc	Input argument count
 *	char	**argv	Pointer to an array of the input args
 *
 * Exit values:
 *
 *	Zero		- Successful operation	
 *	Non-zero	- Incomplete or erroroneous function
 *
 * Side Effects:
 *
 *	n/a	
 */

main(argc, argv)
	int	argc;
	char	*argv[];
{
/*------*\
   Code
\*------*/

/*	Go parse the command
 */
chksum = parse(argc,argv);
argv += chksum;

if (rflag) {
	/* Go process an output to archive function.
 	*/
	while (dorep(argv) == A_WRITE_ERR)
		;/*NOP*/

	done(SUCCEED);
}

/* Does user want to extract files from archive ?
 */
if (xflag)
	doxtract(argv);	/*-yes- Go do the extract */
else
	dotable();	/*-no-  Must be a table function */

done(SUCCEED);

}/*E main() */

/*.sbttl onhup(), onintr(), onquit(), onterm() */

/* Function:
 *
 *	onxxxx	
 *
 * Function Description:
 *
 *	Routines to process termination signals.
 *	ie. Interrupt control flow for termination processing.
 *	(control-c, etc..)
 *
 * Arguments:
 *
 *	none
 *
 * Return values:
 *
 *	none
 *
 * Side Effects:
 *
 *	The request to terminate flag (term) is incremented.	
 */
onhup()
{
	signal(SIGHUP, SIG_IGN);
	term++;
}

onintr()
{
	signal(SIGINT, SIG_IGN);
	term++;
	done(FAIL);
}

onquit()
{
	signal(SIGQUIT, SIG_IGN);
	term++;
	done(FAIL);
}

onterm()
{
	signal(SIGTERM, SIG_IGN);
	term++;
	done(FAIL);
}
/*E onxxxx() */

