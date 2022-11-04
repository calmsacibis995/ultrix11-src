
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#ifndef lint
static	char	*sccsid = "@(#)ltf.c	3.0	(ULTRIX)	4/21/86";
#endif	lint

/**/
/*
 * Summary of the modules that comprise the Labeled Tape Facility,
 * (LTF) in alpha-numeric order.
 *
 *
 *	NAME			Function
 *	------------	----------------------------------------
 *
 *	filenames.c	Contains the routines that deal with
 *			file-name conversions.
 *
 *	filetype.c	Contains the logic to determine the "type"
 *			of Ultrix disk file being processed.
 *
 *	initvol.c	Contains the logic to initialize the
 *			output volume.
 *
 *	ltf.c		The main-line logic of the LTF. Interprets
 *			the command line and calls the appropriate
 *			sub-module to deal with input or output.
 *
 *	ltfdefs.h	Contains the definitions of Global
 *			constants, external declarations,
 *			and data structures.
 *
 *			NOTE:	It also "includes" the system
 *				include files that are required
 *				by all  LTF  modules so that each
 *				module need only "include" ltfdefs.h
 *				to bring in all required system
 *				includes, correct declarations
 *				and variable definitions.
 *
 *	ltferrs.h	Contains the error message macros and error
 *			messages themselves.
 *
 *	ltfvars.c	Contains the declarations of the Global
 *			variables used by the LTF.
 *
 *	makefile	The MAKEFILE for the LTF.
 * 
 *	mstrcmp.c	Contains the logic to compare file names.
 *			ie. Interprets and expands meta-characters
 *			allowed in LTF commands. ("*", "?", etc..)
 *			
 *	odm.c		Contains the routines that deal with physical
 *			output device manipulations.
 *			Centralizes the "ioctl" logic that is used on
 *			Ultrix-32, 32m, 11, etc..
 *
 *	putdir.c	Contains the logic to output directory files
 *			for the LTF.
 *
 *	putfile.c	Contains the routines that output files to
 *			the given volume.
 *
 *	README.1	Important documentation concerning the design
 *			and implementation of the LTF. Includes file
 *			and ANSI volume/label diagrams.
 *
 *	scantape.c	Scans the input volume for  TABLE 
 *			and  EXTRACT  functions.
 *
 *	xtractf.c	Extracts (reads) files from the input volume.
 */
/**/
/*
 *		 GENERAL COMPILE PATH
 *		-----------------------
 *
 * source				 / GLOBAL defs & externals
 *	  \				/
 *	+-------+		+--------------------------+
 *	|  x.c  |--#includes--->|  	ltfdefs.h	   |
 *	+-------+		+--------------------------+
 *	   | 		    	    |			|
 * 	   |			 #includes	    #includes
 *	   |			    |			|
 *	   V		    	    V			V
 *	+-------+		+-----------+	+------------------+
 *	|  x.o  |		| ltferrs.h |	| (system.h	   |
 *	+-------+		+-----------+	|	   files)  |
 *	   /					+------------------+
 * object /
 *
 *
 *			LINKAGE
 *			-------
 *
 *	The object modules produced by the above compilations
 *	(see makefile) are linked together producing the
 *	 executable file  -->  ltf
 */
/**/
/*
 *
 *	File name:
 *
 *		ltf.c
 *
 *	Source file description:
 *
 *		This module is the main-line logic of/for the
 *		Labeled Tape Facility (LTF).
 *
 *
 *	Functions:
 *
 *	    main()	The main-line logic. "main"  parses the
 *			command line arguments. If any errors are
 *			detected, an appropriate message is output &
 *			an exit back to the system is taken.
 *			If a valid command is entered, control is passed
 *			to the appropriate  input  -or-  output 
 *			top-level logic module.
 *
 *
 *	    expnum()	Expands a numeric character string into an
 *			integer with optional "block" or "k" value
 *			multiplication.
 *
 *	    response()	Waits for the user to press the RETURN
 *			before proceeding
 *
 *	    showhelp()	Displays help messages explaining briefly the
 *			usage of the switches of ltf
 *
 *	    usage()	Outputs the desired command format when the
 *			occasion arises.
 *
 *	Usage:
 *
 *	    General command format is:
 *
 *		ltf  function(s)  -qualifiers  [ filename ] ... [ filename ]
 *
 *	    The LTF reads & writes  ANSI compatible tape formats.
 *
 *
 *	Compile:
 *
 *	    cc -O -c ltf.c		<- For Ultrix-32/32m
 *
 *	    cc CFLAGS=-DU11-O ltf.c	<- For Ultrix-11
 *
 *
 *		In point of fact, the complete ltf can only be built
 *		by its' makefile as the facility is in multi-module
 *		format as opposed to single source.
 *
 *
 *	Modification history:
 *	~~~~~~~~~~~~~~~~~~~~
 *
 *	revision			comments
 *	--------	-----------------------------------------------
 *	  01.0		04-Apr-85	Ray Glaser
 *			Create orginal version.
 *	
 *	  01.1		22-Aug-85	Suzanne Logcher
 *			Reorganize ltf.c to look more like tar.
 *	
 *	  01.2		28-Oct-85	Suzanne Logcher
 *			Add tar's magtape defaults
 */
/**/

/*
 * ->	Local defines  "required for / defined by"   this module
 *
 *	NOTE:	This statement must come before the local
 *		includes to correctly define the compilation
 *		of GLOBALS..
 *
 */

#define MAINC		/* So that we don't doubly define message
			 * strings and other global variables when
			 * we compile/link sub-modules.
			 */
/*
 * ->	Local includes
 */

#include	"ltfdefs.h"	/* Common GLOBAL constants & structs */


/**/
/*
 *
 * Function:
 *
 *	main
 *
 * Function Description:
 *
 *	Processes the command line, rejects invalid commands, and
 *	calls appropriate top-level routine for input/output.
 *
 * Arguments:
 *
 *	int	argc		Argument count
 *	char	*argv[]		Pointer to array of pointers to args
 *
 *
 * Return values:
 *
 *	n/a
 *
 *
 * Side Effects:
 *
 *	n/a
 *	
 */

main (argc, argv)
	int	argc;
	char	*argv[];
{
/*
 * ->	Local variables
 */

char	*a;			/* pointer to *argv */
int	dumpflag = 0;		/* Flags word used to set up
				 * f_flags in  filestat structure
				 * indicating dump mode of this
				 * file */
int	iflag = 0;		/* The I option.  iflag = 0 if
				 * no arguments othan ther the
				 * command-line arguments are
				 * to be processed.  iflag = -1 
				 * if the standard input is to
				 * be read for more file names.
				 * iflag = 1 if a file
				 * containing additional file
				 * arguments is given. */
FILE	*ifp;			/* File pointer to inputfile
				 * (if iflag = 1) */

char	inputfile[MAXPATHLEN+1];	/* If iflag = 1, inputfile is
				 * the file containing additional
				 * file arguments.  If iflag = -1,
				 * inputfile[0] = '-' to indicate
				 * that the user is to be
				 * prompted for the other file
				 * names; otherwise,  inputfile[0] = 0
				 * for no prompting. */
int	mag = 0;		/* Flag to check if f qualifier used */
char	magtape[MAXPATHLEN+1];	/* magtape name from switches */
char	*nmbr1;			/* Temporary variable for use with
				 * strings read in to be converted to
				 * numbers */
int	pflag = 0;		/* Flag to check that P is only used
				 * with EXTRACT */
int	ret = TRUE;		/* Boolean to check success of
				 * rec_file */
char	unit = -1;		/* Unit number of named tape device */

/**/
/*------*\
   Code
\*------*/

/* Set warning on if all links to a file are not resolved */
Rep_misslinks = NO;

for (i=0; i < BUFSIZE; i++)
	Spaces[i] = ' ';

/* So that error routines know the name of this program
 */
Progname = argv[0];
if (--argc <= 0) 
	usage();
else {
	Func = NO;
	argc--;
	argv++;
	for (a = *argv++; *a; a++) {
		switch(*a) {
				/* Optional '-' in front of functions
				*/
			case '-':
				break;

				/* List a help message with all 
				 * functions, switches, and qualifiers
				 */
			case 'H':
				showhelp();

				/* Create a new volume (output)
				 * of files and directories.
				 */
			case 'c':
				if (Func != NO) {
					PERROR "\n%s: %s\n", Progname, CONFF);
					usage();
				}
				Func = CREATE;
				break;

				/* Provide a table of contents
				 * (index) of an input volume.
				 */
			case 't':
				if (Func != NO) {
					PERROR "\n%s: %s\n", Progname, CONFF);
					usage();
				}
				Func = TABLE;
				break;

				/* Extract (read in) files and 
				 * and directories from an
				 * input volume.
				 */
			case 'x':
				if (Func != NO) {
					PERROR "\n%s: %s\n", Progname, CONFF);
					usage();
				}
				Func = EXTRACT;
				break;

				/* Output ANSI version 3 volume
				 */

			case 'a':
				Ansiv = '3';
				break;

				/* Select 6250 GCR device
		 		 */
			case 'g':
#ifdef U11
				strcpy(magtape, "/dev/rgt0");
				mag = 1;
#endif
				break;

				/* Output the file a symbolic link
				 * points to instead of the link file
				 */

			case 'h':
				Nosym = TRUE;
				break;

				/* Select TK50 device
		 		 */
			case 'k':
#ifdef U11
				strcpy(magtape, "/dev/rtk0");
				mag = 1;
#endif
				break;

				/* Select 800 bpi tape
		 		 */
			case 'n':
				strcpy(magtape, "/dev/rmt0");
				mag = 1;
				break;

				/* Omit directory blocks from output
				 * volume
				 */

			case 'o':
				Nodir = TRUE;
				break;

				/* No HDR3 switch will not output
				 * optional headers 3 thru 9
				 */

			case 'O':
				Noheader3 = TRUE;
				break;

				/* Permission flag, set when extracting
				 * files and super user, will change
				 * owner and mode to info on tape volume
				 */

			case 'p':
				permission = TRUE;
				break;

				/* Verbose - give the user more
				 * information about what is going
				 * to/from a volume, not including
				 * directories.
				 */
			case 'v':
				Verbose = YES;
				break;

				/* Big Verbose also provides information
				 * about directory files on the volume
				 * as well as the normal Verbose
				 * information about files.
				 */
			case 'V':
				Verbose = Dverbose = YES;
				break;

				/* Set warning on if a file of the same
				 * name exists
				 */
			case 'w':
				Warning = YES;
				break;

				/* Select an alternate unit number.
		 		 */
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				unit = *a;
				break;

				/* Set new blocksize by input
				 */	
			case 'B':
 				if (*argv == 0) {
					PERROR "\n%s: %s\n", Progname, NOBLK);
					usage();
				}
				nmbr1 = *argv;
				Blocksize = expnum(nmbr1);
				if (error_status) {
					PERROR "\n%s: %s %s\n\n", Progname, INVNF, nmbr1);
					exit(FAIL);
				}
				if (Blocksize > MAXBLKWRT || Blocksize < MINBLKSIZE) {
					PERROR "\n%s: %s %d%s\n\n", Progname, INVBS, MAXBLKWRT, BYTES);
					exit(FAIL);
				}
				*argv++;
				argc--;
				break;
	
				/* Get devicefilename argument
				 */
			case 'f':
 				if (*argv == 0) {
					PERROR "\n%s: %s\n", Progname, NOFIL);
					usage();
				}
				else
				    /*
				     * The name following 'f' is used
				     * literally as the tape device name
				     */
				    if (strlen(*argv) < MAXNAMLEN)
				   	strcpy(magtape, *argv++);
				    else {	
				    	PERROR "\n%s: %s %s\n", Progname, FNTL, a);
				    	exit(FAIL);
				    }
				mag = 2;
				argc--;
				break;

				/* Get filename of list of files to 
				 * be operated on or by STDIN
				 */	
			case 'I':
 				if (*argv == 0) {
					PERROR "\n%s: %s\n", Progname, NOINP);
					usage();
				}
				if (strlen(*argv) < MAXNAMLEN)
					strcpy(inputfile, *argv++);
				else {
				   	 PERROR "\n%s: %s %s\n", Progname, FNTL, a);
				    	exit(FAIL);
				}
				if (*inputfile == '-')
					iflag = -1;
				else {
					iflag = 1;
					if (!(ifp = fopen(inputfile, "r"))) {
					    PERROR "\n%s: %s %s\n\n", Progname,
						CANTOPEN, inputfile);
					    exit(FAIL);
					}
				    }/*F if *inputfile == '-'  */
				argc--;
				break;
	
				/* Get new volume identification 
				 */
			case 'L':
 				if (*argv == 0) {
					PERROR "\n%s: %s\n", Progname, NOVOL);
					usage();
				}
				/* Make sure it is not too long
				*/
				if (strlen(*argv) > 6) {
					PERROR "\n%s: %s\n", Progname, VOLIDTL);
				        exit(FAIL);
				}

				strcpy(Volid,*argv++);
	
				if (!filter_to_a(Volid,REPORT_ERRORS)) {
					PERROR "\n%s: %s", Progname, INVVID1);
					PERROR "\n%s: %s %s\n\n", Progname, INVVID2, Volid);
					exit(FAIL);
				}
				argc--;
				break;
	
				/* Position to an ANSI file sequence 
				 * number before beginning requested 
				 * operations.
				 */
			case 'P':
 				if (*argv == 0) {
					PERROR "\n%s: %s\n", Progname, NOPOS);
					usage();
				}
				nmbr1 = *argv;
				while ((isdigit(*nmbr1)) || (*nmbr1 == ',') || (*nmbr1 == '-'))
				    nmbr1++;
				if (*nmbr1 != '\0') {
					PERROR "\n%s: %s %s\n\n", Progname, INVNF, *argv);
					exit(FAIL);
				}
				sscanf(*argv++, "%d,%d", &Seqno, &Secno);
				if (Seqno <= 0) {
					PERROR "\n%s: %s %d\n\n", Progname, INVPN, Seqno);
					exit(FAIL);
				}
				if (Secno <= 0) {
					PERROR "\n%s: %s %d\n\n", Progname, INVPS, Secno);
					exit(FAIL);
				}
				++pflag;
				argc--;
				break;
	
				/* Set new record length
				 */
			case 'R':
 				if (*argv == 0) {
					PERROR "\n%s: %s\n", Progname, NOREC);
					usage();
				}
				nmbr1 = *argv;
				Reclength = expnum(nmbr1);
				if (error_status) {
					PERROR "\n%s: %s %s\n\n", Progname, INVNF, nmbr1);
					exit(FAIL);
				}
				if (Reclength > MAXRECSIZE || Reclength < 1) {
					PERROR "\n%s: %s %d%s\n\n", Progname, INVRS, MAXRECSIZE, BYTES);
					exit(FAIL);
				}
				*argv++;
				argc--;
				break;

				/* Else error
				 */	
			default:
				PERROR "\n%s: %s %c\n", Progname, UNQ, *a);
				usage();
	}/*E switch *a */
    }/* for */
}/*T if --argc > 0 */
/**/

/* +-->	Begin processing the requested function(s) ..*/

if (Func == NO) {
    PERROR "\n%s: %s\n", Progname, NOFUNC);
    usage();
}
if (permission) {
    if ((i = getuid()) > 0) {
	PERROR "\n%s: %s %d\n", Progname, NOTSU, i);
	permission = FALSE;
    }
    else
	if (Func != EXTRACT) {
	    PERROR "\n%s: %s\n", Progname, CANTPER);
	    permission = FALSE;
	}
}
if (mag > 0) {
    strcpy(Magtdev, magtape);
    if (mag == 1 && unit != -1)
	Magtdev[8] = unit;
}
Tape = tape(Magtdev);	/* Decide if using tape and set flag */
rew(0);

if (Func == TABLE || Func == EXTRACT) {
    if (Nodir)
	Dverbose = NO;
    dumpflag = 0;
    for ( ; argc > 0; argc--, argv++)
	rec_args(*argv, dumpflag);
    if (iflag == 1)
	ret = rec_file(ifp, iflag, "");
    else
	if (iflag == -1)
	    ret = rec_file(stdin, iflag, "");
    if (ret)
	scantape();
    else
	printf("\n");

}/*T if Func == TABLE || Func == EXTRACT  */

else { /* if Func == CREATE */
    /* If we are to create a new volume, Magtdev will be opened
     * for writing in initvol().
     */
    if (pflag != 0) {
	PERROR "\n%s: %s\n\n", Progname, INVPNUSE);
    }

    if (iflag == 0 && argc == 0) {
	PERROR "\n%s: %s\n", Progname, NOARGS);
	usage();
    }
    if (Func == WRITE) {
	if (iflag == 1)
	    fclose(ifp);
#if 0
	writetp(argc, argv, iflag, inputfile);
#endif
    }/*E if Func = WRITE */
    else
	if (Func == CREATE) {
	    if (iflag == 1)
		fclose(ifp);

	   /* Go initialize the "volume" on the output
	    * device of choice. We won't be coming back.
	    */
	    printf("\n");
	    initvol(argc, argv, iflag, inputfile);

	}/*T if Func == CREATE */
}/*F if Func == TABLE || Func == EXTRACT  */
}/*E main() */
/**/
/*
 *
 * Function:
 *
 *	expnum
 *
 * Function Description:
 *
 *	Expand a numeric character string into an integer
 *	multiple of the format expressed. ie.. Allow the user
 *	to input a value in blocks (or k) and we convert it
 *	to the real number of bytes implied.
 *
 *	For example ->	10b = ten 512 byte blocks
 *	  -or-		3k  = three 1024 byte blocks
 *
 * Arguments:
 *
 *	char	*numstring	Pointer to the null terminated numeric
 *				character string.
 *	int	error_status	TRUE indicates invalid numeric string
 *				FALSE indicates no error
 *
 * Return values:
 *
 *	Returns a numeric value if the conversion was valid.
 *
 *
 *	
 */


/* moved expnum() into filenames.c to fit overlay version */
/* moved usage() into filenames.c */
/* moved showhelp() into filenames.c */

/**/
/*
 *
 * Function:
 *
 *	response()
 *
 * Function Description:
 *
 *	Waits for a response of RETURN until exiting
 *
 * Arguments:
 *
 *	n/a
 *
 * Return values:
 *
 *	none
 *
 * Side Effects:
 *
 *	none
 *	
 */

response()
{

/*------*\
   Code
\*------*/

ch = getchar();
if (ch != '\n')
    while (getchar() != '\n')
	;
else
    ch = 'n';
return(ch);
}/* E response */

/**\\**\\**\\**\\**\\**  EOM  ltf.c  **\\**\\**\\**\\**\\*/
/**\\**\\**\\**\\**\\**  EOM  ltf.c  **\\**\\**\\**\\**\\*/
