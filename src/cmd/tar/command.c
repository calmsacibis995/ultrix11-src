
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#ifndef lint
static	char	*sccsid = "@(#)command.c	3.1	(ULTRIX)	9/5/87";
#endif	lint

/**/
/*
 *
 *	File name:
 *
 *		command.c
 *
 *	Source file description:
 *
 *		Tar command line processing
 *		& other user interface routines.
 *		Also contains not often used routines
 *		for read/write in order to minimize the
 *		size of overlays when built for non-split
 *		i/d PDP-11s.
 *
 *	Functions:
 *
 *	Usage:
 *
 *	Compile:
 *
 *	Modification history:
 *	~~~~~~~~~~~~~~~~~~~~
 *
 *	revision			comments
 *	--------	-----------------------------------------------
 *	III		28-Jan-86 rjg/
 *			Do not allow non-super user to set pflag
 *
 *	II		Ray Glaser, 09-Jan-86
 *			Do not set block size to 10 for disks (d flag)
 *
 *	I		19-Dec-85 rjg/
 *			Create original version.
 *	
 */
#include "tar.h"

/*.sbttl checkf() */

/* Function:
 *
 *	checkf
 *
 * Function Description:
 *
 *   Routine to determine whether the specified file should
 *   be skipped or not. Implements the F, FF and FFF modifiers.
 *
 *   The set of files skipped depends upon the value of howmuch.
 *
 *   When howmuch > 0 the set of skip files consists of:
 *
 *		SCCS directories
 *		core files
 *		errs files
 *
 *    Howmuch > 1 increases the skip set to include:
 *
 *		a.out
 *		*.o
 *
 *    Howmuch > 2 causes executable files to be skipped
 *    as well. (A file is determined to be executable by
 *    looking at its "magic numbers")
 *
 *
 * Arguments:
 *
 *	char	*longname	Pointer to name string of file to
 *				to be checked
 *	int	mode		File mode bits of the file
 *	int	howmuch		Skip set modifier
 *
 * Return values:
 *
 *	1	The file is to be skipped
 *	0	The file is not to be skipped
 *
 * Side Effects:
 *
 *	The routine assumes that the file is in the current directory.
 *	The routine is not fully implemented as described above. See
 *	the #ifdef below.. 
 *	
 */
checkf(longname, mode, howmuch)
	STRING_POINTER	longname;
	int	mode;
	int	howmuch;
{
/*------*\
  Locals
\*------*/

	STRING_POINTER	shortname;

/*------*\
   Code
\*------*/

/* Strip off any directory and get the base filename.
 */ 
if ((shortname = rindex(longname, '/')))
	shortname++;
else
	shortname = longname;

/* Basic skip set ?
 */
if (howmuch > 0) {
	/*
	 * Skip SCCS directories on input
	 */
	if ((mode & S_IFMT) == S_IFDIR)
		return (strcmp(shortname, "SCCS") != 0);

	/*
	 * Skip SCCS directory files on extraction
	 * Check SCCS as a toplevel directory and
	 * as a subdirectory.
	 */
	if (strfind(longname,"SCCS/") == longname || strfind(longname,"/SCCS/"))
		return (0);
	/*
	 * Skip core and errs files. 
	 */
	if (strcmp(shortname,"core")==0 || strcmp(shortname,"errs")==0)
		return (0);

}/*E if howmuch > 0 */

/* First level additions ?
 */
if (howmuch > 1) {
	int l;

	l = strlen(shortname);	/* get string len */

	/* Skip .o files
	 */
	if (shortname[--l] == 'o' && shortname[--l] == '.')
		return (0);

	/* Skip a.out
	 */
	if (strcmp(shortname, "a.out") == 0)
		return (0);

}/*E if howmuch > 1 */

#ifdef notdefFFF
/*
 * This routine works for -c and -r options, but is
 * not sufficent for -x option.  Since it cannot be implemented
 * fully, it is not implemented at all at this time.
 */
/*
 * Second level additions ?
 */
if (howmuch > 2) {
	/*
	 * Open the file to examine the magic numbers.
	 * If the file cannot be opened, then assume
	 * that it is not to be skipped and that another
	 * routine will perform the error handling for it.
	 * Otherwise, read from the file and check the
	 * magic numbers, indicate skipping if they are good.
	 */
	int	ifile, in;
	struct exec	hdr;

	ifile = open(shortname,O_RDONLY,0);

	if (ifile >= 0)	{
		in = read(ifile, &hdr, sizeof (struct exec));
		close(ifile);
		if (in > 0 && ! N_BADMAG(hdr))	/* executable: skip */
			return (0);
	}
}/* end if howmuch > 2 */
#endif notdefFFF

return (1);	/* Default action is not to skip */

}/*E checkf() */

/*.sbttl done() */

/* Function:
 *
 *	done
 *
 * Function Description:
 *
 *
 * Arguments:
 *
 *	int	n	Desired exit status
 *
 * Return values:
 *
 *	none
 *
 * Side Effects:
 *
 */
done(n)
{
/*------*\
  Locals
\*------*/

	int	i,status;

/*------*\
   Code
\*------*/

status = unlink(tname);
close(mt);

if (NFLAG)
	exit(n);

if (n == A_WRITE_ERR) {

	struct linkbuf	*lihead;
	struct linkbuf	*linkp;
	

	/* Return malloc'd linked file list to freemem
	 */
	for (lihead = ihead; lihead;) {
		linkp = lihead->nextp;	
		free ((char *)lihead);
		lihead = linkp;
	}
	ihead = 0;

	/* Return malloc'd directory list to freemem
	 */
	fdlist();

	start_archive = CARCH;
	size_of_media[CARCH] = size_of_media[0];
	blocks_used = 0L;
	PUTE = AFLAG = EOTFLAG = EODFLAG = recno = NMEM4L = NMEM4D = 0;
	dcount1 = dcount2 = dcount3 = lcount1 = lcount2 = 0;

	if (chdir(hdir) < 0) {
		fprintf(stderr, "%s: Can't change directory back ?", progname);
		perror(hdir);
		exit(FAIL);
	}

REOPEN:
/**/
	fprintf(stderr,"\007%s: Please press  RETURN  to re-write %s %d ",progname, Archive, CARCH);

	CARCH = 1;
	sprintf(CARCHS, "%d", CARCH);
	response();
	fprintf(stderr,"\n%s: Starting error recovery\n",progname);

	if ((mt = open(usefile, O_RDWR))< 0) {
		fprintf(stderr,"\n%s: Can't open: %s\n",progname,usefile);
		perror(usefile);
		goto REOPEN;
	}
	return(A_WRITE_ERR);
}
if (n == SUCCEED)
	n = 0;

exit(n);

}/*E done() */

/*.sbttl fdlist() */

/* Function:
 *
 *	fdlist
 *
 * Function Description:
 *
 *	Return the directory list structures to free mem.
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
 *	none	
 */

fdlist()
{
/*------*\
  Locals
\*------*/

	struct DIRE	*lDhead;
	struct DIRE	*dlinkp;


if (Dhead)
	dcount3++;

/* Return malloc'd directory list to freemem
 */
for (lDhead = Dhead; lDhead;) {

	dlinkp = lDhead->dir_next;	
	free ((char *)lDhead);
	lDhead = dlinkp;
}
Dhead = 0;

}/*E fdlist() */

/*.sbttl flushtape() */

/* Function:
 *
 *	flushtape
 *
 * Function Description:
 *
 *
 * Arguments:
 *
 *
 * Return values:
 *
 *
 * Side Effects:
 *
 *	
 */

flushtape()
{
/*------*\
   Code
\*------*/

if (CARCH >= start_archive) {
	if (recno) {
		if (write(mt, tbuf, TBLOCK*nblock) < 0) {

			if ((ioctl(mt, MTIOCGET, &mtsts)<0) ||
				size_of_media[CARCH] ||
				(errno != ENOSPC) || NFLAG) {
FERR:
/**/
				fprintf(stderr,"\n\n\007%s: Archive %d write error on last block\n",
					progname, CARCH);

				fprintf(stderr,"%s: Blocks used = %ld\n",progname, blocks_used);
				perror(usefile);
				done(A_WRITE_ERR);
				return(A_WRITE_ERR);
			}
			else {
				if (!(mtsts.mt_softstat & MT_EOT))
					goto FERR;
				else {
					mtops.mt_op = MTCSE;
					if (ioctl(mt,MTIOCTOP,&mtops)< 0)
						goto FERR;
					else {

						OARCH = CARCH;
						EOTFLAG++;
						MULTI++;
						FEOT++;

						if (writetape(tbuf,recno,recno,
						(char *)cblock.dbuf.name,
						(char *)cblock.dbuf.name) == A_WRITE_ERR)
							goto FERR;
					}
				}
			}
		}/*E if write ..*/
	}/*E if recno */
}/*E if CARCH >= start_archive */

if (vflag) {
	if (CARCH >= start_archive) {
		if (MULTI)
	        	fprintf(stderr,"\n%s: End of %s media",
				progname,Archive);
		if (VFLAG)
			fprintf(stderr,"\n%s: %ld Blocks used on %s for %s %d\n",
				progname, blocks_used, usefile, Archive, CARCH);
	}
	else
		if (AFLAG || ((start_archive > CARCH) && VFLAG))
			fprintf(stderr,"%s: %s %d  skipped.\n",
				progname, Archive, CARCH);
}/*E if vflag */

recno = 0;
return(SUCCEED);

}/*E flushtape() */

/*.sbttl parse() - Parse the command line */

/* Parse the command line and set up control variables.
 */
parse(argc,argv)
	int	argc;
	char	*argv[];
{

/*------*\
  Locals
\*------*/
	STRING_POINTER	cp;
	INDEX	i;

/*------*\
   Code
\*------*/

chksum = 2;

progname = argv[0];	/* Get our name*/

#ifdef PRO
set_size(800L);
#else
set_size(0L);	/* Default media size to tapes/files = unlimited */
#endif

if (!(cp = rindex(progname,'/')))
	cp = progname;
else 
	cp++;


if ((strcmp(cp,mdtar))) {
	MDTAR = FALSE;
}
else {
	/* Task name MDTAR implies multiple archive function
	 * with 800 block rx50 as the default output device.
	 */
	set_size(800L);
	MDTAR = TRUE;
}

if (argc < 2)
	usage();

tfile = NULL;

/* Setup the default archive
 */
usefile =  magtape;
argv[argc] = 0;
argv++;

for (cp = *argv++; *cp; cp++) {
	int uid;

	switch(*cp) {
		/*
	 	 *  Switches that either TAR or MDTAR will accept..
	 	 */

		/* A: Archive. Number of the archive, counting from 1,
		 * to begin physically writing.
		 */
		case 'A':
			if (!*argv) {
				fprintf(stderr, "%s: Archive number must be specified with 'A' option\n", progname);
				usage();
			}
			start_archive = atoi(*argv++);
			chksum++;
			if ((start_archive < 1) ||
				(start_archive > MAXARCHIVE) ) {

				fprintf(stderr, "%s: Invalid archive number:  %d\n", progname,start_archive);
				done(FAIL);
			}
			AFLAG++;
			break;

		/* b: use next argument as blocksize
		 */
		case 'b':
			bflag++;
			if (!*argv) {
				fprintf(stderr, "%s: Blocksize must be specified with 'b' option\n", progname);
				usage();
			}
			nblock = atoi(*argv++);
			chksum++;

			if (nblock <= 0 || nblock > NBLOCK) {
				fprintf(stderr, "%s: Invalid blocksize \"%s\"\n", progname, *argv);
				done(FAIL);
			}
			break;

		/* B: Force I/O blocking to 20 blocks per record
		 */
		case 'B':
			Bflag++;
			break;

		/* -c: create new archive. Note that -r is implied
		 * by this switch also.
		 */
		case 'c':
			cflag++;
			rflag++;
			break;

		/* -D: Directory output in old style to conserve
		 * memory and archive utilization.
		 */
		case 'D':
			DFLAG++;
			break;

		/* -d: select default RX50 diskette device.
		 * -e: select default RX33 diskette device.
		 * "device" means diskette type, not drive type.
		 */
		case 'd':
		case 'e':
			/* Set up default media size table.
 			*/
			if(*cp == 'd')
				set_size(800L);
			else
				set_size(2400L);
			dflag++;
#ifdef U11
#ifdef PRO
			magtape[6] = 'r';
			magtape[7] = 'x';
			magtape[8] = '0';
#else
			magtape[6] = 'r';
			magtape[7] = 'x';
			magtape[8] = '1';
#endif
#endif
#ifndef U11
			magtape[6] = 'r';
			magtape[7] = 'a';
			magtape[8] = '1';
			magtape[9] = 'a';
#endif
			if (unitflag)
				magtape[8] = unitc;
			break;


		/* -f: use next argument as the archive of choice
		 * instead of the default.
		 */
		case 'f':
			if (!*argv) {
				fprintf(stderr, "%s: Archive file  must be specified with 'f' option\n", progname);
				usage();
			}
			usefile = *argv++;
			chksum++;
			if (cflag) {
				i = stat(usefile, &stbuf);
				if ((i<0) || ((stbuf.st_mode & S_IFMT) == S_IFREG)) {
					if (!sflag && !MDTAR)
						set_size(0L);

					if (open(usefile,O_TRUNC,0)<0) {
						if (errno != ENOENT) {
							fprintf(stderr, "%s: Can't open:  %s\n", progname, usefile);
							perror(usefile);
							done(FAIL);
						}
					}
				}
			}
			fflag++;
			break;

		/* F: Fast. F causes SCCS dirs, core & errs files
		 * to be skipped.
		 * FF:	Skip  .o & a.out's also
		 * FFF: Skip executable files.
		 */
		case 'F':
			Fflag++;
			break;
#ifdef U11
		/* g: select 6250 GCR device
		 */
		case 'g':
			set_size(0L);
			magtape[6] = 'g';
			magtape[7] = 't';
			magtape[8] = '0';
			if (unitflag)
				magtape[8] = unitc;
			break;
#endif

		/* H: User has requested the help function. Provide
		 * expanded information about the switches/options.
		 */
		case 'H':
			HELP++;
			usage();

		/* h: Treat symbolic links as if they were normal
		 * files. ie. Put a physical copy of the linked to
		 * file on the archive.
		 */
		case 'h':
			hflag++;
			break;
#ifdef U11
		/* k: select TK50 device
		 */
		case 'k':
			set_size(0L);
			magtape[6] = 't';
			magtape[7] = 'k';
			magtape[8] = '0';

			if (unitflag)
				magtape[8] = unitc;
			break;
#endif
		/* l: Print errors if all links to a file cannot
		 * be resolved.
		 */
		case 'l':
			lflag++;
			break;

		/* -r: Write named files at the END of the archive.
		 *     Implied in -u & -c
		 */
		case 'r':
			rflag++;
			break;

		/* -u: Add to archive only those files
		 *     not already in/on it.
		 */
		case 'u':
			mktemp(tname);
			if ((tfile = fopen(tname, "w")) == NULL) {
	    		    fprintf(stderr, "%s: Can't create temporary file (%s)\n", progname, tname);
			    perror(tname);
			    done(FAIL);
			}
			fprintf(tfile, "!!!!!/!/!/!/!/!/!/! 000\n");

			/* IMPLIED 'r' FUNCTION 
			 */
			rflag++;
			break;

		/* V: VERBOSE mode. Big verbose displays directory
		 * information not displayed by little verbose.
		 * Note that 'V' implies 'v'.
		 */
		case 'V':
			VFLAG++;
			/*_FALL THRU_*/

		/* v: Verbose mode.  Display the filenames as they
		 * are processed. Also supply further information
		 * about the archive and operation. See man page.
		 */
		case 'v':
			vflag++;
			if (VFLAG)
				fprintf(stderr,"\n%s: rev. %d.%-d\n", progname,revwhole,revdec);
			break;

		/* w: Wait mode. Request user confirmation prior to
		 * processing each file argument.
		 */
		case 'w':
			wflag++;
			break;

		/* n: Select an alternate unit number.
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
			unitflag++;
			unitc = *cp;

			if (!fflag)
				magtape[8] = *cp;

				/* w/o dflag Ultrix-11 dev = /dev/rht1
				 *	     Ultrix-Pro dev= /dev/rrx1
				 *           Ultrix-32 dev = /dev/rmt8
				 */ 
			usefile = magtape;

			break;

		/* -: ignored. To allow some degree of consistency
		 * with other unix commands that do not insist on the
		 * leading -.
		 */
		case '-':
			break;

		/* i: Do not terminate if checksum errors are detected
		 * during an archive read.
		 */
		case 'i':
			iflag++;
			break;

		/* m: Do not restore modification times from the input
		 * archive file header block.
		 */
		case 'm':
			mflag++;
			break;

		/* M: Specify maximum writtable archive number.
		 * (for debugging)
		 */
		case 'M':
			if (!*argv) {
				fprintf(stderr, "%s: Archive number must be specified with 'M' option\n", progname);
				usage();
			}
			MAXAR = atoi(*argv++);
			chksum++;
			if ((MAXAR < 1) ||
				(MAXAR > MAXARCHIVE) ) {
				fprintf(stderr, "%s: Invalid archive number:  %d\n", progname,MAXAR);
				done(FAIL);
			}
			MFLAG++;
			break;

		/* N: No multi-archive, file splitting, or
		 * new header format.
		 */
		case 'N':
			NFLAG++;
			set_size(0L);
			if (MDTAR) {
				fprintf(stderr,"\n\007\007%s: Warning: intended  mdtar  functionality is disabled by the 'N' switch.\n\n",progname);
				MDTAR = FALSE;
			}
			break;

		/* n: Select 800 bpi tape. By default this is assumed 
		 * to be  /dev/rmt0.
		 */
		case 'n':
			set_size(0L);
			magtape[6] = 'm';
			magtape[7] = 't';
			magtape[8] = '0';

			if (unitflag)
				magtape[8] = unitc;
			break;

		/* O: Include file owner and group names in 
		 * verbose output if present in this archive format.
		 */
		case 'O':
			OFLAG++;
			break;

		/* o: create the archive w/o directory information
		 * to produce archives that early version of tar
		 * can process.
		 */
		case 'o':
			oflag++;
			break;

		/* p: Restore files to original their orginal modes
		 * and owners as recorded in the file header block.
		 * NOTE: This is only allowed if you are the super-user.
		 */
		case 'p':
			uid = getuid();
			if (uid == 0)
				pflag++;
			else {
				fprintf(stderr,"\n%s: \007Warning: Only the super-user may invoke the 'p' option\n\n", progname);
			}
			break;

		/* S: Output User Group Standard archive format.
		 */
		case 'S':
			SFLAG++;
			break;

		/* s:  Size of the media in blocks.
		 */
		case 's':
			if (!*argv) {
				fprintf(stderr, "%s: Media size must be specified with 's' option\n", progname);
				usage();
			}
			size_of_media[0] = atoi(*argv++);
			chksum++;

			if (size_of_media[0] <= 4) {
		    		fprintf(stderr, "%s: Invalid media size:  %s\n", progname, *argv);
		    		done(FAIL);
			}
			set_size(size_of_media[0]);
			sflag++;
			break;


		/* t: List the names of the files that are in/on the
		 * given archive.
		 */
		case 't':
			tflag++;
			break;

		/* x: eXtract the named files from the given
		 * input archive.
		 */
		case 'x':
			xflag++;
			break;

		default:
			fprintf(stderr, "%s: Unknown option:  %c\n", progname, *cp);
			usage();

		}/*E switch(*cp)*/
}/*E for (cp = *argv++; *cp; cp++) */


NOARGS:
/*----:
 */
if (MAXAR < start_archive) {
	fprintf(stderr,"\n%s: Max archive < start_archive\n",progname);
	done(FAIL);
}

if (AFLAG && !size_of_media[0]) {
	fprintf(stderr,"\n%s: Media size conflict\n",progname);
	done(FAIL);
}

/*
 * Check to see that a function letter was specified, 
 * either directly or by implication.
 */
if (!rflag && !xflag && !tflag)
	usage();

if (cflag || rflag)
	if (argc < 3)
		usage();

tbuf = (union hblock *)malloc(nblock*TBLOCK);

if (tbuf == NULL) {
	fprintf(stderr, "%s: Blocksize %d too big, can't get memory\n", progname, nblock);
		done(FAIL);
}

/* Does the user want to add files to the end of the current
 * archive device ?
 */
if (rflag) {

	if (cflag && tfile != NULL) {
		fprintf(stderr,"%s: c and u functions may not be given in the same command\n", progname);
		done(FAIL);
	}
	if (signal(SIGHUP, SIG_IGN) != SIG_IGN)
		signal(SIGHUP, onhup);

#ifdef notdef
	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, onintr);

	if (signal(SIGQUIT, SIG_IGN) != SIG_IGN)
		signal(SIGQUIT, onquit);

	if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
		signal(SIGTERM, onterm);
#endif

	/* Has user specified  stdout  as the archive meadia ?
 	 */
	if (strcmp(usefile, "-") == 0) {
		if (!cflag) {
			fprintf(stderr, "%s: Can only CREATE standard output archives\n", progname);
			done(FAIL);
		}
		/* When pipe output has been specified,
		 * ignore invocation name of MDTAR
		 * and any possible size of device given by
		 * cancelling the flags.
 		 */
		MDTAR = FALSE;
		sflag = FALSE;
		set_size(0L);
		mt = dup(1);
		nblock = 1;
	} else
		{
		int status;
TRYOPA:
/**/
	 	if ((mt = open(usefile, O_RDWR)) < 0) {
			if (!cflag || (mt = creat(usefile, 0666)) < 0) {
				fprintf(stderr, "\n\007%s: Can't open:  %s\n", progname, usefile);
				perror(usefile);
				fprintf(stderr,"%s: Please press  RETURN  to retry ", progname);
				response();
				goto TRYOPA;
			}
		}
	}
	return(chksum);

}/*E if rflag */

/* Has user specified  stdin  as the archive device ?
 */
if (!strcmp(usefile, "-")) {
	pipein++;
	mt = dup(0);
	nblock = 1;
} else
	{
OPMTA:
/**/
	if ((mt = open(usefile, O_RDONLY)) < 0) {
		fprintf(stderr, "\n\007%s: Can't open:  %s\n", progname,usefile);
		perror(usefile);
		fprintf(stderr,"%s: Please press RETURN  to retry ", progname);	
		response();
		goto OPMTA;
	}
}

return(chksum);

}/*E parse() */

/*.sbttl putempty()  Put empty (EOF) blocks on archive */

/* Function:
 *
 *	putempty
 *
 * Function Description:
 *
 *	Writes an empty block on the output archive.
 *	Used when writting the end of archive blocks.
 *
 * Arguments:
 *
 *	none
 *
 * Return values:
 *
 *
 * Side Effects:
 *
 */
putempty()
{
/*------*\
  Locals
\*------*/

	INDEX	i;

/*------*\
   Code
\*------*/

for (i=0; i < sizeof(iobuf); i++)
	iobuf[i] = 0;

/* Use to debug code.. flags our empty block as opposed
 * to just bumping into one.
 *
 *strcpy(iobuf+(TBLOCK-6),"eoa");
 */

if (writetape(iobuf,1,1,eoa,eoa) == A_WRITE_ERR)
	return(A_WRITE_ERR);

return(SUCCEED);

}/*E putempty() */

/*.sbttl puteoa()  Put an end of archive block in the buffer */

/* Function:
 *
 *	puteoa
 *
 * Function Description:
 *
 *	This function formats and puts the END OF ARCHIVE
 *	block in the buffer when splitting a file across
 *	an archive.
 *
 * Arguments:
 *	none
 *
 * Return values:
 *
 * Side Effects:
 *
 */

puteoa()
{
/*------*\
  Locals
\*------*/

	STRING_POINTER	cp;
	STRING_POINTER	from;
	COUNTER	nc;
	STRING_POINTER	to;

/*------*\
   Code
\*------*/

for (to = (char *)&tbuf[recno++], nc = TBLOCK; nc; nc--)
	*to++ = 0;
for (to = (char *)&tbuf[recno++], nc = TBLOCK; nc; nc--)
	*to++ = 0;

/* Copy file name into output buffer.
 */
for (from = (char *)dblock.dbuf.name, to = (char *)&tbuf[recno], nc=NAMSIZ; nc; nc--)
	*to++ = *from++;

/* Indicate End of Archive by ascii zero fill of remaining header
 * block fields.
 */
for (nc=TBLOCK-NAMSIZ-1,cp = (char *)&tbuf[recno]+NAMSIZ; nc ; nc--)
	*cp++ = '0';

return(flushtape());

}/*E puteoa() */

/*.sbttl response()  Get a yes/no answer from user */

/* Function:
 *
 *	response
 *
 * Function Description:
 *
 *	Gets a yes/no response from the user at the terminal.
 *	Used in conjunction with the "wait" function to selectively
 *	include/exclude files from this operation.
 *	It also is used when we are waiting for the user to
 *	physically change an archive when doing multi-archive i/o.
 *
 * Arguments:
 *
 *	none
 *
 * Return values:
 *
 *	The character user typed. If none typed, then 'n'.
 *
 * Side Effects:
 *
 *	
 */
response()
{
/*------*\
  Locals
\*------*/

	char	c;

/*------*\
   Code
\*------*/

c = getchar();
if (c != '\n')
	while (getchar() != '\n')
		;/*_NOP_*/
else
	c = 'n';

if (term)
	done(FAIL);

fprintf(stderr,"\n\n");
return (c);

}/*E response() */

/*.sbttl set_size() */

/* Function:
 *
 *	set_size
 *
 * Function Description:
 *
 *	Set up the media size table.
 *	
 * Arguments:
 *
 *	SIZE_L	size	Number of blocks on the media.
 *
 * Return values:
 *
 *
 * Side Effects:
 *
 *	
 */
set_size(size)
	SIZE_L	size; 
{
/*------*\
  Locals
\*------*/

	INDEX	i;

/*------*\
   Code
\*------*/

for (i = MAXARCHIVE; i >= 0; i--)
	size_of_media[i] = size;

return;

}/*E set_size() */

/*.sbttl strfind() */

/* Function:
 *
 *	strfind
 *
 * Function Description:
 *
 *	Searches for  substr  in text.
 *	
 * Arguments:
 *
 *	char	*text		Text to search
 *	char	*substr		String to locate
 *
 * Return values:
 *
 *	Pointer to  substr  starting position in text if found.
 *	NULL if not found.
 *
 * Side Effects:
 *
 *	
 */
STRING_POINTER	strfind(text,substr)
		STRING_POINTER text, substr;
{
/*------*\
  Locals
\*------*/

	COUNTER	i;	/* counter for possible fits */
	SIZE_I	substrlen;/* len of substr--to avoid recalculating */

/*------*\
   Code
\*------*/

substrlen = strlen(substr);

/* Loop through text until not found or match.
 */
for (i = strlen(text) - substrlen; i >= 0 &&
     strncmp(text, substr, substrlen); i--)

	text++;

/* Return NULL if not found, ptr else NULL.
 */ 
return ((i < 0 ? NULL : text));

}/*E strfind() */

/*.sbttl usage()  Show the user correct command format(s) */

/* Function:
 *
 *	usage	aka useage
 *
 * Function Description:
 *
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
 *	Always exits to the system with error status
 */

usage()
{
/*------*\
   Code
\*------*/

#ifndef PRO
if (!HELP)
	fprintf(stderr,"\n%s: specify H (Help) for expanded definition of switches\n", progname);
#endif
#ifdef U11
	fprintf(stderr, "\nusage:\n%s [-]{crtux} [ABb-CDdFfghiklMmNnOopsVvw] [archivefile] [blocksize]\n   [archivenumber] [maxarchive #] [mediasize] [archivefile]\n    directory/file1 directory/file2 ..\n\n", progname);
#else
	fprintf(stderr, "\nusage:\n%s [-]{crtux} [ABb-CDdFfHhilMmNnOopsVvw] [archivefile] [blocksize]\n   [archivenumber] [maxarchive #] [mediasize] [archivefile]\n   directory/file1 directory/file2 ..\n\n", progname);
#endif

#ifndef PRO
	/* If user has selected HELP mode, give an expanded version
	 * of the letters and switches. Useful on small systems
	 * that don't have man pages and user doesn't have a
	 * manual set handy.
	 */
if (HELP) {
	printf("%s: One of the function keys enclosed in  {}  is required.\n\n", progname);
	printf("%s: c = create new archive, previous content is overwritten\n", progname);
	printf("%s: r = revise archive by adding files to end of current content\n", progname);
	printf("%s: t = give table of contents with verbosity defined by v or V\n", progname);
	printf("%s: u = update archive. Add files to end either if they are not already there\n", progname);
	printf("%s:     or if they have been modified since last put to archive. \n", progname);
	printf("%s: x = extract files from the named archive\n", progname);
	
	printf("\n%s: Items enclosed in  []  are optional\n\n", progname);
	printf("\n\n\n\n\n%s: Press RETURN to continue ..", progname); 
	response();
	fprintf(stderr,"\n\n");

	printf("%s: A = use next argument as archive number with which to begin output\n", progname);

	printf("%s: B = Invoke proper blocking logic for tar functions across machines\n", progname);
	printf("%s: b = use next argument as blocking factor for archive records\n", progname);
	printf("%s:-C = change directory to following file name (-C dirname)\n", progname);
	printf("%s: D = Directory output in original tar style\n", progname);
	printf("%s: d = select rx50 as output\n", progname);
#ifndef U11
	printf("%s:     (/dev/rra1a)\n", progname);
#endif

#ifdef U11
	printf("%s:     (/dev/rrx1)\n", progname);
#endif

	printf("%s: F[F] = operate in fast mode. Skip all SCCS directories, core files,\n", progname);
	printf("%s:        & errs file. FF also skips a.out and *.o files\n", progname);
	printf("%s: f = use following argument as the name of the archive\n", progname);
#ifdef U11
	printf("%s: g = use  /dev/rgt0  (6250 GCR)\n", progname);
#endif
	printf("%s: H = Help mode. Print this summary\n", progname);
	printf("%s: h = have a copy of a symbolically linked\n", progname);
	printf("%s:     file placed on archive instead of symbolic link\n", progname);
	printf("%s: i = ignore checksum errors in header blocks\n", progname);
#ifdef U11
	printf("%s: k = use  /dev/rtk0  (TK50)\n", progname);
#endif
	printf("%s: l = output link resolution error message summary\n", progname);
	printf("%s: M = Next arg specifies maximum archive number to be written and\n",progname);
	printf("%s:     prints current archive number on output line\n",progname);
	printf("%s: m = do not restore modification times. Use time of extraction\n", progname);

	printf("\n\n%s: Press RETURN to continue ..", progname); 
	response();
	fprintf(stderr,"\n\n");

	printf("%s: N = No multi-archive, file splitting, or new header format on output\n", progname);
	printf("%s:     Output directories in previous tar format. On input, set file\n",progname);
	printf("%s:     UID & GID from file header vs. values in /etc/passwd & group files\n",progname);


	printf("%s: n = select 800 bpi tape device (/dev/rmt0)\n", progname);
	printf("%s: O = include file owner & group names in verbose output (t & x functions)\n",progname);
	printf("%s:     if present in archive header.\n",progname);
	printf("%s:     Output warning message if owner or group name not found in\n",progname);
	printf("%s:     /etc/passwd or /etc/group file (cru functions)\n",progname);
	printf("%s: o = omit directory blocks from output archive\n", progname);

	printf("%s: p = change permissions and owner of extracted files to original values\n", progname);
	printf("%s:     (only works if you are the super user)\n", progname);
	printf("%s: S = Output User Group Standard archive format\n",progname);
	printf("%s: s = next argument specifies size of archive in 512 byte blocks\n", progname);
	printf("%s: V = big verbose. Most informative information about current operation\n", progname);
	printf("%s: v = verbose mode. Provide additional information about files/operation\n", progname);
	printf("%s: w = wait mode. Ask for user confirmation before including specified file\n", progname);
	printf("%s: 0..9 select the named unit number for archive device\n\n\n\n", progname);
}/*E if HELP */

#endif /*ndef PRO*/

done(FAIL);

}/*E usage() aka useage*/

