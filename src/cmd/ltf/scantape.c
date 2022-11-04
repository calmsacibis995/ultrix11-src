
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#ifndef lint
static	char	*sccsid = "@(#)scantape.c	3.1	(ULTRIX)	12/3/87";
#endif	lint

/**/
/*
 *
 *	File name:
 *
 *		scantape.c
 *
 *	Source file description:
 *
 *		This module "scans" ( ie.  READS )  the input volume
 *		when an  eXtract or Table of contents function has
 *		been requested of the LTF.
 *
 *	Functions:
 *
 *		date_time()	Fills a given 'sdate' string with
 *				the appropriate month, day, and
 *				modification time.
 *
 *		date_year()	Fills a given 'sdate' string with
 *				the appropriate month, day, and
 *				year.
 *
 *		expand_mode()	Expands the uid and gid mode fields
 *		    expandm()	for print out on TABLE or EXTRACT
 *				functions.
 *
 *		scantape()	Top level logic to read the Volume
 *
 *		wbadlab()	Writes out "bad" label data encountered
 *
 *	Usage:
 *
 *		n/a
 *
 *	Compile:
 *
 *	    cc -O -c scantape.c		 <- For Ultrix-32/32m
 *
 *	    cc CFLAGS=-DU11-O scantape.c <- For Ultrix-11
 *
 *
 *	Modification history:
 *	~~~~~~~~~~~~~~~~~~~~
 *
 *	revision			comments
 *	--------	-----------------------------------------------
 *	 01.0		20-Apr-85	Ray Glaser
 *			Create orginal version
 *
 *	 01.1		4-Sep-85	Suzanne Logcher
 *			Add interchange name to TABLE w/o v or V
 *			Change use of position number to table or 
 *			   extract files
 *			Correct use of Noheader3	
 */

/*
 * ->	Local Includes
 */

#include "ltfdefs.h"

/**/
/*
 *
 * Function:
 *
 *	date_time
 *
 * Function Description:
 *
 *	Fills the string 'sdate' with the appropriate
 *	month, day, and modification time.
 *
 * Arguments:
 *
 *	char	*sdate		The string to be filled
 *	long	*sec		Source of the time..
 *
 * Return values:
 *
 *	The string given is date/time filled.
 *
 * Side Effects:
 *
 *	none	
 */
date_time(sdate, sec)
	char	*sdate;
	long	*sec;
{
/*
 * +--> Local Variables
 */

int	century;
long	clock;
char	*ctime();
long	rettime;
int	thisyear;
struct	tm *localtime();
struct	tm *ltime;
long	time();

/*------*\
   Code
\*------*/

rettime = time(&clock);
ltime =	localtime(&rettime);
thisyear = ltime->tm_year;

ltime = localtime(sec);

if (ltime->tm_year == thisyear)
	sprintf(sdate, "%s %2d %02d:%02d", Months[ltime->tm_mon + 1],
		ltime->tm_mday, ltime->tm_hour, ltime->tm_min);
else {
	cp = ctime(sec);
	if (cp[20] == '2' && cp [21] == '0')
	    century = 2000;
	else {
	    if (cp[20] != '1' || cp[21] != '9')
		PERROR "\n%s: %s\n", Progname, BADCENT);
	    century = 1900;
	}
	sprintf(sdate, "%s %2d  %4d", Months[ltime->tm_mon + 1],
		ltime->tm_mday, ltime->tm_year + century);
}
}/*E date_time() */
/**/
/*
 *
 * Function:
 *
 *	date_year
 *
 * Function Description:
 *
 *	Fills the given string with the appropriate month,
 *	day, and year.
 *
 * Arguments:
 *
 *	char	*sdate
 *	char	*century
 *	char	*date
 *
 * Return values:
 *
 *	String is filled with .. etc.. 
 *
 *
 * Side Effects:
 *
 *	none
 *	
 */

date_year(sdate, century, date)
	char	*sdate, *century, *date;
{
/*
 * +--> Local Variables
 */

int	leap;
int	year;
int	yearday;
int	realyear;

/*------*\
   Code
\*------*/

sscanf(date, "%2d%3d", &year, &yearday);
if (*century == '0')
    i = 2000 + year;
else
    if (*century != ' ')
	PERROR "\n%s: %s\n", Progname, BADCENT);
    i = 1900 + year;
	
realyear = i;
leap = i%4 == 0 && i%100 != 0 || i%400 == 0;

for (i= 0; yearday > Days[leap][i]; i++)
	yearday -= Days[leap][i];

sprintf(sdate, "%s %2d  %4d", Months[i], yearday, realyear);

}/*E date_year();
/**/
/*
 *
 * Function:
 *
 *	expand_mode, expandm
 *
 * Function Description:
 *
 *	These siamese functions expand and print the 9 mode
 *	fields of an Ultrix  uid / gid  file status.
 *
 * Arguments:
 *
 *	int	mode	Word containing the  st_mode value
 *
 * Return values:
 *
 *	The ASCII representation of the uid/gid is printed
 *	on stdout.
 *
 * Side Effects:
 *
 *	none	
 */

expand_mode(mode)
int	mode;
{
/*
 * +--> Local Variables
 */

register int **mp;

/*------*\
   Code
\*------*/

for (mp = &M[0]; mp < &M[9]; )
	expandm(*mp++, mode);

}/*E expand_mode() */
/*
 * Sub-function:
 *
 *  expandm		Determines the status of the next mode field.
 *			This function is a sub-function used only for
 *			the above expand_mode function. It was done
 *			this way for ease of implementation.
 */

expandm(choices, mode)
	int	*choices; /* Number of choices for this field */
	int	mode;
{
/*
 * +--> Local Variables
 */
register int	*ch;

/*------*\
   Code
\*------*/

ch = choices;
i = *ch++;
while (--i >= 0 && (mode & *ch++) == 0)
	ch++;

printf("%c", *ch);

}/*E expandm()
/**/
/*
 * Function:
 *
 *	scantape
 *
 * Function Description:
 *
 *	This function is the top level logic to read a volume
 *	for the  TABLE  and  EXTRACT  functions.
 *
 * Arguments:
 *
 *	No direct arguments are passed. However the logic depends
 *	on flag variables set by the main-line logic to take the 
 *	correct actions.
 *
 * Return values:
 *
 *	This function does not return to the caller.  Wether or not
 *	the operation was successful, the function exits to system
 *	control. If an error occurs, a message is output to stderr.
 *
 * Side Effects:
 *
 *	none
 */

scantape()
{
/*
 * ->	Local variables
 */

int	Bufoff;		/* Buffer offset read from HDR2 (51-52) */
char	cat_misc[20];	/* misc message buf */
long	charcnt;	/* Character count of binary files.
			 * A precaution  in case the last character
			 * of a binary file is the same as the
			 * the padding character. */
struct	FILESTAT *fstat;
int	getlen();	/* Declare getlen proc. to get length of F 
			 * or S record */
int	gid;		/* Group id */
int	hlink;		/* Field in HDR2, if set, file has hard link */
int	k;		/* Temporary variable */
int	linkflag;	/* Set if link() is successful */
int	lnk_fseqno;	/* File sequence no. of head link file */
char	lnk_msg[30];	/* Used to output Hard/Symbolic link msgs */
char	lnk_name[MAXPATHLEN+1];/* File name of head link file */
struct	XLINKBUF *lp;
char	pathname[MAXPATHLEN+1]; /* temporary variable */
int	mode;		/* File mode */
long	modtime;	/* Modification time of file. */
int	nbytes;
long	ret;
long	save;
char	sdate[13];	/* Creation-date string */
int	skip;		/* Position number skip counter */
char	sysvol[14];	/* Holds copy of VOL1 L_systemid */
int	tfiletype;	/* True Ultrix file type */
int	uid;		/* User id */
int	wildc = NO;
long	xtractf();

/*------*\
   Code
\*------*/

/***
 *	READ / PROCESS  +-->  VOL1
 ***/

/* Initialize lp just in case */
lp = (struct XLINKBUF *) NULL;

if ((i = read(fileno(Magtfp), Labelbuf, BUFSIZE)) <= 0) {
	PERROR "\n%s: %s VOL1 (%s)%c\n", Progname, CANTRL, Magtdev, BELL);
	perror(Magtdev);
	if (i < 0)
	    ceot();
	exit(FAIL);
}
	
L_labid[0] = 0;
sscanf(Labelbuf, "%3c%1d%6c%14c%13c%14c",
	L_labid, &L_labno, L_volid, Dummy, L_systemid, L_ownrid);
strcpy(sysvol, L_systemid);

if (strncmp(L_labid, "VOL",3)) {
	PERROR "\n%s: %s VOL1%c\n", Progname, INVLF, BELL); 
	/*
	 * Go display the bad data and exit.
	 */
	wbadlab();
}
/*
 * First VOLume label number should always be 1.
 */
if (L_labno != 1) {
	PERROR "\n%s: %s VOL1 %s%c\n", Progname, INVLF, INVLNO, BELL);
	wbadlab();
}
/*
 * Pick up, save, & verify the ANSI version number of this volume.
 */
Ansiv = Labelbuf[79];

if (Ansiv != '3' && Ansiv != '4') {
	PERROR "\n%s: %s VOL1 %s-> %c%c\n", Progname, INVLF,
	  UNSAV, Ansiv, BELL);
	wbadlab();
}
/* Save volume id.
 */
strcpy(Volid, L_volid);

/*
 * Display the volume label information if requested.
 *
 */
if (Verbose) {
	PROMPT "\n%s: %s %s %s %c\n",
		Progname, VOLIS, Volid, ANSIV, Ansiv); 
	PROMPT "%s: %s %s\n", Progname, OWNRID, L_ownrid);
}
	/*	*-*  NOTE  *-*
	 *	     ----
	 * If we see an ANSI version 4 tape, we may have to:
	 ***
	 *	READ +-->  VOL2 - VOL9		(if present)
	 *
	 * For both 3/4 we may have:
	 *
	 *	READ +-->  UVL1 - UVL9		(if present)
	 ***/

/**/
/*
 *	SCAN the TAPE -- and do what is required for an
 *
 *		EXTRACT or TABLE  function..
 *
 */
skip = 1;
FOREVER {
    charcnt		= 0L;
    linkflag	= FALSE;
    modtime		= 0L;
    pathname[0]	= 0;

    /*******
     *	READ / PROCESS	+-->  HDR1
     ******/
	if ((i = read(fileno(Magtfp), Labelbuf, BUFSIZE)) <= 0) {
	    /*
	     * This is the normal way that the LTF currently exits
	     * from the  forever loop we are inside of. When it 
	     * exhausts the usable data on the input volume, the
	     * above read returns a  <= 0 condition. Normally this
	     * should be due to the fact that it has reached the
	     * set of "double" end of tape marks that signify the
 	     * end of data. Empty files may pose a premature exit,
 	     * but at this time, enough is not yet known about empty
 	     * file handling to determine if this is in fact a
 	     * real possiblity.
 	     */
	    if (i < 0)
		ceot();
    	    if (Seqno && skip <= Seqno) {
	    	PERROR "\n%s: %s %s\n", Progname, CANTFSF, Magtdev);
	    	perror(Magtdev);
	        printf("\n");
	    	exit(FAIL);
	    }
	    printf("\n");
	    exit(SUCCEED);
	}
	sscanf(Labelbuf, "%3s%1d%17s%6s%4d%4d%4d%2d%c%5s %5s%*7c%13s", 
		L_labid, &L_labno, L_filename, L_volid, &L_fsecno, 
		&L_fseqno, &L_gen, &L_genver, &L_crecent, L_credate, 
		L_expirdate, L_systemid);
	if (Ansiv == '3')
	    strcpy(sysvol, L_systemid);

	/* Abort if multi-volume indicator is seen !
	 */
	if (!strcmp(L_labid, "EOV")) {
	/*
	 * As the LTF is not designed to process multi-volume
	 * sets, we must abandon the operation at this point.
	 * ie. There is no more data on the tape. A partial
	 * file may have been written to the output device.
	 * If we were to try to deal with "multi-volume" sets,
	 * logic would be added at this point to switch to
	 * the next volume and continue ...
	 */
		PERROR "\n%s: %s\n\n", Progname, MULTIV1);
		exit(FAIL);
	}
	/*
	 * Set the flag that tells us whether or not this volume
	 * was created by an Ultrix system. Volmo makes sure we
	 * do the announcement and flag set only wonce.
	 */
	if (!Volmo) {
	    if (Verbose)
		PROMPT "%s: %s  %s\n", Progname, IMPIDM, L_systemid);
	}
	if (!strncmp(L_systemid,IMPID,13) && !strncmp(L_systemid, sysvol, 13))
	    Ultrixvol = TRUE;
	else {
	    Ultrixvol = FALSE;
	    Noheader3 = TRUE;
	}
	if (strncmp(L_systemid, sysvol, 13))
	    PERROR "%s: %s %s\n", Progname, IMPIDC, L_systemid);

	if (strcmp(L_labid, "HDR") || L_labno != 1) {
		if (Tape) {
hdr1err:
		    PERROR "\n%s: %s HDR1%c\n", Progname, INVLF, BELL); 
		    wbadlab();
		}/*T if Tape */
		else {
			/*
			 * If i/o device is not a tape AND
			 * we see a dummy tape_mark when we
			 * are looking for HDR1, assume it is
			 * the double tape mark - end of data
			 * on volume condition.
			 */
			if (!tape_mark(Labelbuf))
				goto hdr1err;
			else {
				cat_misc[0] = 0;	
				printf("\n");

				/* Complain about files that were not
				 * found on the volume.
				 */
				for (fstat = F_head; fstat; fstat = fstat->f_next) {
					if (!fstat->f_found) {
					    if (!Seqno)
					    	PERROR "%s: %s %s\n", Progname,NOTONV, fstat->f_src);
					    else
					    	PERROR "%s: %s %s\n", Progname,NOTONP, fstat->f_src);
					    cat_misc[0] = '\n';
					}
				}/*E for fstat ..*/

				fprintf(stderr,"%c",cat_misc[0]);
    	    			if (Seqno && skip <= Seqno) {
	    			    PERROR "\n%s: %s %s\n", Progname, CANTFSF, Magtdev);
	    			    perror(Magtdev);
				    printf("\n");
	    			    exit(FAIL);
	    			}
				exit(SUCCEED);	
			}
		}/*F if tape */
	}/*E if HDR1 */
	/*
 	 *	Lower case the HDR1 File ID string.
 	 *	IF: We  MUST  use the HDR1 string for this files' name.
 	 */
	if (!Ultrixvol || Noheader3 || Ansiv != '4') {
		cp = L_filename;
		while (*cp) {
			*cp = isupper(*cp) ? *cp-'A'+'a' : *cp;
			cp++;
		}
	}
	/**/
	/*******
 	*	READ / PROCESS	+-->  HDR2
 	******/
	if ((i = read(fileno(Magtfp), Labelbuf, BUFSIZE)) <= 0) {
		PERROR "\n%s: %s HDR2%c\n", Progname, CANTRL, BELL);
		perror(Progname);
		if (i < 0)
		    ceot();
		exit(FAIL);
	}

	if (Ultrixvol) {
	    sscanf(Labelbuf, "%3s%1d%c%5d%5d%6o%4d%4d%4d%3c%1c%10ld%1d%1d%1d%2d",
		L_labid, &L_labno, &L_recformat, &L_blklen,
		    &L_reclen, &mode, &uid, &gid, &lnk_fseqno,
			Tftypes,Dummy,&charcnt,&Lhdrl, &Leofl, 
			   &hlink, &Bufoff);
	}
	else {
		sscanf(Labelbuf, "%3s%1d%c%5d%5d%35c%2d", L_labid,
		    &L_labno, &L_recformat, &L_blklen, &L_reclen,Dummy, &Bufoff);
		strcpy(Tftypes, "???");
	}
	/*
	 * Make sure we are seeing the correct label at this point.
	 */
	if (strcmp(L_labid, "HDR") || L_labno != 2) {
		PERROR "\n%s: %s HDR2%c\n", Progname, INVLF, BELL); 
		wbadlab();
	}

	if (Func == EXTRACT) {
	    if (L_blklen < MINBLKSIZE || L_blklen > MAXBLKSIZE) {
		PERROR "\n%s: %s %d%s\n", Progname, INVBS, MAXBLKSIZE, BYTES);
		exit(FAIL);
	    }
	    if ((strncmp(Tftypes, "dir", 3)) && (L_recformat == FIXED) && (L_reclen < 1 || L_reclen > MAXRECSIZE)) {
		PERROR "\n%s: %s %d%s\n", Progname, INVRS, MAXRECSIZE, BYTES);
		exit(FAIL);
	    }
	}
	/**/
	/*******
 	*	READ / PROCESS	+-->  HDR3
 	******/
	
	/*	If user doesn't want HDR3, etc. data to be used,
 	*	skip this stuff that gets name from HDR3 - EOF9.
 	*/
	if (!Noheader3 && skip >= Seqno) {
	    if ((i = read(fileno(Magtfp), Labelbuf, BUFSIZE)) <= 0) {
		PERROR "\n%s: %s HDR3%c\n", Progname, CANTRL, BELL);
		PERROR "\n%s: %s\n", Progname, TRYNH3);
		perror(Progname);
		if (i < 0)
	    	    ceot();
		exit(FAIL);
	    }

	    Labelbuf[BUFSIZE] = 0;

	    /* For one of our volumes, get the
	     * data we have on this file from HDR3.
	     */

	    sscanf(Labelbuf, "%3s%1d%10ld%10c%20c%36s", L_labid, &L_labno, &modtime, Owner, Hostname, pathname);
	    if (strcmp(L_labid, "HDR") || L_labno != 3) {
		PERROR "\n%s: %s HDR3%c", Progname, INVLF, BELL);
		PERROR "\n%s: %s\n\n", Progname, TRYNH3);
	        wbadlab();		
	    }
	    /* 
	     * If not an ANSI version 4 volume, lower case the
	     * pathname string component by default.
	     */
	    if (Ansiv != '4') {
		cp = pathname;
		while (*cp) {
			*cp = isupper(*cp) ? *cp-'A'+'a' : *cp;
			cp++;
		}
	    }
	    /**/
	    /*******
	    *	READ / PROCESS	+-->  HDR4 thru HDRn
	    ******/

	    if (Lhdrl > 3) {
		char epathname[77];
		int labelno=4;

		for (;labelno <= Lhdrl; labelno++) {
			if ((i = read(fileno(Magtfp), Labelbuf, BUFSIZE)) <= 0) {
				PERROR "\n%s: %s HDR%d%c\n", Progname, CANTRL, labelno, BELL);
				perror(Progname);
				if (i < 0)
				    ceot();
				exit(FAIL);
			}
			/*
			 * Extract the label number and extended
			 * pathname characters from this HDRn.
			 */
			sscanf(Labelbuf, "%3s%1d%76s", L_labid, &L_labno, epathname);
			/*
			 * Make sure we are seeing something
			 * that looks like the correct
			 * HDRn label.
			 */
			if (strcmp(L_labid, "HDR") || L_labno != labelno) {
					PERROR "\n%s: %s HDR%d%c", Progname, INVLF, labelno, BELL);
					PERROR "\n%s: %s\n\n", Progname, TRYNH3);
    					wbadlab();		
			}
			strncat(pathname,epathname,76);
		}/*E for ;labelno ..*/
		/*
		 * If there are extended path/file name
		 * characters tucked away in the EOF labels,
		 * extract them and tack them on to the
		 * real path name of the file before
		 * we read the file data. A tedious, but
		 * necessary step.
		 */
		if (Leofl) {
			if (Tape) {
			    rew(1);
			    /*
			     * This should place us at the EOF1
			     * label of the file.
			     */
			    fsf(((L_fseqno - 1) *3) + 2);
			}/*T if (Tape) */
			else {
			    save = ftell(Magtfp);
			    fsf(1);
			}/*F if (Tape) */
			for (labelno =1; labelno <= Leofl; labelno++) {
				if ((i = read(fileno(Magtfp), Labelbuf, BUFSIZE)) <= 0) {
					PERROR "\n%s: %s EOF%d%c\n", Progname, CANTRL, labelno, BELL);
					perror(Progname);
					if (i < 0)
				    	    ceot();
					exit(FAIL);
				}
				/*
			 	 * Extract the label number &
				 * extended pathname characters
				 * from this EOFn.
			 	 */
				sscanf(Labelbuf, "%3s%1d%76s", L_labid, &L_labno, epathname);
				/*
			 	 * Make sure we are seeing
				 * something that looks like
			 	 * an EOFn label.
			 	 */
				if (strcmp(L_labid, "EOF") || L_labno != labelno) {
						PERROR "\n%s: %s EOF%d%c", Progname, INVLF, labelno, BELL);
						PERROR "\n%s: %s\n\n", Progname, TRYNH3);
	    					wbadlab();		
				}
				/* First extended path/file
				 * name characters appear in
				 * EOF3 and can continue thru
				 * the EOF9 label.
				 */
				if(L_labno > 2){
					strncat(pathname,epathname,76);
				}
			}/*E for labelno=1;labelno ..*/
			/*
			 * Now re-position the volume for
			 * reading of the file data after
			 * extracting the extended path/file
			 * name characters from the EOF
			 * label set. We will end up at HDR1
			 * of the file and the xtractf() 
			 * function will take us forward to
			 * the start of the data with a fsf().
			 */
			if (Tape) {
			    rew(1);
			    /*
			     * This should place us at the HDR1
			     * label of the file.
			     */
			    fsf((L_fseqno - 1) *3);
			}/*T if (Tape) */
			else {
			    fseek(Magtfp, save, 0);
			}/*F if (Tape) */
		}/*E if (Leofl) */
	    }/*E if Lhdrl > 3 */
	}/*F if Noheader3 */

	/**/
	if (!Volmo) {
	    if (Verbose && Ultrixvol && !Noheader3) 
		PERROR "%s: %s  %s\n", Progname, VOLCRE, Hostname);
	    Volmo++;
	    printf("\n");
	}
	if (!Noheader3)
    	    strcpy(Name,pathname);
	else {
    	    modtime = 0l;
    	    Owner[0] = NULL;
    	    Hostname[0] = NULL;
    	    /*
    	    * L_filename is 17 characters of the "interchange"
    	    * file name from HDR1.
    	    */
    	    strcpy(Name,L_filename);
    	    strcpy(pathname,L_filename);
	}
	/* Go position to file data, then check if buffer offset
	 * is zero.  If not, read past specified bytes 
	 */
	fsf(1);
	if (Bufoff != 0)
	    if ((i = read(fileno(Magtfp), Labelbuf, Bufoff)) <= 0) {
		PERROR "\n%s: %s %2d %c\n", Progname, CANTBUF, Bufoff, BELL);
		perror(Progname);
		if (i < 0)
		    ceot();
		exit(FAIL);
	    }
	/* If positioning by sequence number and not there yet,
	 * increment skipper and go to start of forever loop
	 */ 
	if (Seqno && skip < Seqno) {
	    fsf(2);
	    skip++;
	    continue;
	}/*T if (Seqno ... */
	skip++;
	/*
	* Numrecs equals 0 if no file arguments are specified.
	* Thus the entire tape must be processed.
	*/
	if (! Numrecs || (fstat = Lookup(Name))) {
		/* Is this a symbolic link ?
		 */
		if (!strcmp(Tftypes,"sym") && Ultrixvol) {

			char	Inbuf[MAXBLKSIZE+1];

			strcpy(lnk_msg,SLINKTO);
			linkflag = YES;

			/* Read the link from input volume
			 * and save it in  lnk_name for later.
			 */
			if ((nbytes = read(fileno(Magtfp), Inbuf, L_blklen)) <= 0) {
				PERROR "\n%s: %s %s%c\n", Progname, CANTRD, Name, BELL);
				if (nbytes < 0)
		    		    ceot();
				exit(FAIL);
			}
			i = getlen(Inbuf);
			if (L_recformat == VARIABLE)
			    strncpy(lnk_name,&Inbuf[4],i);
			else {
			    strncpy(lnk_name,&Inbuf[5],i);
			    while (Inbuf[0] != '3') {
				if ((nbytes = read(fileno(Magtfp), Inbuf, L_blklen)) <= 0) {
				    PERROR "\n%s: %s %s%c\n", Progname, CANTRD, Name, BELL);
				    if (nbytes < 0)
		    			ceot();
				    exit(FAIL);
				}
				j = getlen(Inbuf);
				i += j;
				strncat(lnk_name,&Inbuf[5],j);
			    }/*E while (&Inbuf[0] != '3') */
			}
			lnk_name[i] = 0;
			if (Func != TABLE) {
				unlink(Name);
				if (symlink(lnk_name,Name) < 0) {
			    		PERROR "\n%s: %s -> %s\n    to -> %s%c\n",
					Progname, CANTLF, Name,
					lnk_name, BELL);
			    		perror(Progname);
					printf("\n");
				}
			}/*E if Func != TABLE */
			/* Skip to EOF1
			*/
			fsf(1);
		}/*E if !strcmp(Tfypes, "sym") && Ultrixvol) */
		if ((Func == TABLE) && !linkflag) {
			/* Skip to start of EOF labels
	 	 	 */
			fsf(1);
		}
		/*
		 * Is this file hard linked to another file ?
		 */
		if (lnk_fseqno && Ultrixvol) {
			int found = 0;
			strcpy(lnk_msg,HLINKTO);
			for (lp = X_head; lp; lp = lp->x_next) {
				if (lp->x_fseqno == lnk_fseqno) {
					found++;
					linkflag = YES;
					strcpy(lnk_name,lp->x_pathname);
					break;
				}
			}/*E for lp = X_head ..*/
			if (!found)
				PERROR "\n%s:  %s %s\n%s%c\n", Progname,CANTL1,Name,MHL,BELL);
			if (found && (Func == EXTRACT)) {
				unlink(Name);
				if (link(lnk_name, Name) < 0) {
			    		PERROR "\n%s: %s -> %s\n    to -> %s%c\n", Progname, CANTLF, Name, lnk_name, BELL);
					perror(Progname);
					printf("\n");
			    		linkflag = NO;
				}
				else {
					/* If link found, skip to next
					 * EOF1?
					 */
					fsf(1);
				}
			}/*E if found */
		}/*E if lnk_fseqno ..*/
		if (!lnk_fseqno && hlink && Ultrixvol) {
		    lp = (struct XLINKBUF *) malloc(sizeof(*lp));
		    if (!lp) {
			PERROR "\n%s: %s%c\n", Progname,NOMEM,BELL);
		    	exit(FAIL);
		    }/*E if !lp */

		    /* Save enough information about this file
	 	     * in order to identify it in case there are
	 	     * other files linked to it on the input volume.
	 	     */
		    lp->x_next = X_head; /* Pointers run backward ! */
		    X_head = lp;
		    lp->x_fseqno = L_fseqno;
		    lp->x_fsecno = L_fsecno;
		    lp->x_pathname = (char *) malloc(strlen(Name) + 1);
		    if (!lp) {
			PERROR "\n%s: %s%c\n", Progname,NOMEM,BELL);
		    	exit(FAIL);
		    }/*E if !lp */
		    else
			strcpy(lp->x_pathname, Name);
		}/*T if (!lnk_fseqno && hlink %% Ultrixvol) */
/**/

		if (Func == EXTRACT && Numrecs && fstat->f_flags) {

			if (fstat->f_flags & FUF && fstat->f_flags & DD) {
				PERROR "\n%s: %s\n", Progname, MS1);
				exit(FAIL);
			}
					
			if (fstat->f_flags & FUF) {
				if (L_recformat == VARIABLE)
	    				L_recformat = FUF;
				else {
	    				PERROR "\n%s: %s\n", Progname, MS2);
					exit(FAIL);
				}
			}/*E if fstat->f_flags & FUF */

			if (fstat->f_flags & DD) {
				if (L_recformat == VARIABLE)
	    				L_recformat = DD;
				else {
	    				PERROR "\n%s: %s\n", Progname, MS3);
					exit(FAIL);
				}
			}/*E if fstat->f_flags & DD */
	
		}/*E if (Func == EXTRACT && Numrecs && fstat->f_flags) */
/**/
		if (Func == EXTRACT && !linkflag) {
			Xname[0] = 0;
		 	ret = xtractf(pathname, ! Numrecs ? "" : fstat->f_src, charcnt, Xname);
			if (Xname[0] != 0)
			    strcpy(Name, Xname);
			if (ret >= 0L && !strcmp(L_systemid, IMPID)) {
				if (permission) {
				    chmod(Name, mode);
				    chown(Name, uid, gid);
				}
				if (modtime > 0L) {
					time_t	timep[2];
					timep[0] = time(NULL);
					timep[1] = modtime;
					utime(Name, timep);
				}/*E if modtime > 0L */
			}/*E if (ret >= 0L && ! strcmp(L_systemid, IMPID)) */
			if ( !linkflag  && lp ) {
				if (ret < 0L) {
#if 0
					X_head = lp->x_next;
					free((char*)lp);
#endif
					continue;
				}
				else if (Xname[0] != 0 && !Wildc)
					strcpy(lp->x_pathname, Xname);

			}/*E if !linkflag && lp */

			/* Stop processing any files with
	 	 	 * ret < 0L that haven't been caught
	 	 	 * before this.  e.g., non-head link
	 	 	 * files that were not extracted.
	 	 	 */
			if (ret < 0L)
				continue;
	
		}/*E if (Func == EXTRACT && ! linkflag) */
/**/
		if ((nbytes = read(fileno(Magtfp), Labelbuf, BUFSIZE)) < 0){
			PERROR "\n%s: %s EOF1\n", Progname, CANTRL);
		        ceot();
		}
		sscanf(Labelbuf, "%3s%1d%*50c%6ld", L_labid,
			&L_labno, &L_nblocks);

		if (strcmp(L_labid, "EOF") || L_labno != 1) {
			PERROR "\n%s: %s EOF1\n", Progname, INVLF);
			wbadlab();
		}
		/*
	 	 *	Skip over the rest of the "EOF" label
	 	 *	set ...  for the TIME BEING !!
	 	 */
		fsf(1);	

		if (Func == TABLE) {
			if (Verbose) {
			    /*
			     * If this file is not a directory file,
			     * list it. If it is a directory file and
			     * the user really wants to see it (them),
			     * list the name. Else, directory files
			     * are not listed.
			     */
			    if (strcmp(Tftypes,"dir") || Dverbose) {

				sprintf(cat_misc, "t(%d,%d)",
					L_fseqno, L_fsecno);
				printf("%-7s", cat_misc);
	
				/* If this is not an Ultrix volume.
			 	 */
				if (strcmp(L_systemid, IMPID))
					printf("---------   -/-   ");
				else {
					expand_mode(mode);
					printf("%4d/%-4d%s", uid, gid, Owner);
				}
				if (modtime > 0L)
					date_time(sdate, &modtime);
				else
					date_year(sdate, &L_crecent, L_credate);
		
				printf("%12s", sdate);

				if (!Ultrixvol) {
					sprintf(cat_misc, "%ld(%d)%c", L_nblocks,
			    		L_blklen, L_recformat);
					printf("%11s", cat_misc);

				}/*T if !Ultrixvol */
				else {
					if (strcmp(Tftypes,"dir"))
					    sprintf(cat_misc,"  %04ld bytes  <%s>%c",
				    	    charcnt,Tftypes,L_recformat);

					else 
					     /* Directory files are
					      * always 0000 bytes long,
					      * so don't bother to list
					      * the size.
					      */
					    sprintf(cat_misc,"              <%s>%c",
					    Tftypes,L_recformat);

					printf("%s", cat_misc);

				}/*F if !Ultrixvol */

				/* For loong path names..
			 	 */
				if (strlen(Name) > 12)
					printf("\n       %s\n", Name);
				else
					printf(" %s", Name);
		
			   }/*E if strcmp(Tftypes .. */
			}/*E if (Verbose) */
			else {
			    /*
			     * If this file is not a directory file,
			     * list it. If it is a directory file and
			     * the user really wants to see it (them),
			     * list the name. Else, directory files
			     * are not listed.
			     */
			    if (strcmp(Tftypes,"dir") || Dverbose) {
				printf("t  %s", Name);
				if (!Noheader3)
				    printf("  (%s %s)", INTERCH, L_filename);
			    }
			}
			if (linkflag)
				if (Verbose) 
					printf("\n      %s %s\n", lnk_msg, lnk_name);
				else
					printf ("  %s %s\n", lnk_msg, lnk_name);
			else
			    if (strcmp(Tftypes,"dir") || Dverbose) 
				printf ("\n");
	
		}/*T if (Func == TABLE) */
		else {
			/*
		 	 * Func == EXTRACT
		 	 */
			if (Verbose) {
			    /*
			     * If this file is not a directory file,
			     * list it. If it is a directory file and
			     * the user really wants to see it (them),
			     * list the name. Else, directory files
			     * are not listed.
			     */
			    if (strcmp(Tftypes,"dir") || Dverbose || Dircre) {
				if (linkflag)
					printf("x<%s>%c %04ld byte%c,  %s  %s %s", 
					Tftypes, L_recformat, charcnt, 
					charcnt == 1 ? ' ' : 's', Name, 
					lnk_msg, lnk_name);
				else {
#if 0
	/* If we want to see how many tape blocks, put this back in.
	 */
				printf("x<%s>%c %04ld byte%c, %03ld %d-byte tape block%c  %s\n",
				Tftypes,L_recformat,
				ret, ret == 1 ? ' ' : 's',
				L_nblocks, L_blklen,
				L_nblocks == 1L ? ' ' : 's',
				Name);
#endif
					/* If not a directory file, list
					 * its size in bytes, else not.
					 * Directory files are always
					 * 0000 bytes long and when
					 * using the Dverbose mode, they
					 * tend to cloud the output.
					 */
					if (strcmp(Tftypes,"dir"))
					    printf("x<%s>%c %04ld byte%c,  %s",
					    Tftypes,L_recformat,
					    ret, ret == 1 ? ' ' : 's', Name);
					else
					    printf("x<%s>%c              %s",Tftypes,L_recformat,Name);

				}

				if (Dircre) {
					printf(" %s",DIRCRE);
					Dircre = FALSE;
				}	
				printf("\n");

			    }/*E if strcmp(Tftypes .. */
			}/*T if Verbose (Func = EXTRACT) */

			else {
				/* Func == EXTRACT
			 	 * not verbose..
			 	 */
			    if (strcmp(Tftypes,"dir") || Dverbose || Dircre) {
				printf("x  %s", Name);

				if (linkflag)
					printf("  %s %s\n", lnk_msg, lnk_name);
				else
					printf("\n");
			    }
			}/*F if Verbose (Func = Extract) */

			if ((charcnt != ret) && Ultrixvol && !linkflag) 
				PERROR "\n%s: %s %s\n%s %ld %s %ld%c\n\n",
			  	Progname, BADCNT1, Name, BADCNT2, charcnt, BADCNT3, ret, BELL);

		}/*F if Func == TABLE */
/**/
/* The following logic was added as a result of a QPR on "rdt". 
 * It complained that when only 1 distinct file was given for
 * an extract, the entire tape was searched for all copies.
 * This was true. VMS tape routines & tar do the same thing however.
 * But, this loses if you have a large number of files on tape. 
 * ie. It takes a long time to read 1300 files looking for all copies. 
 * As the user may specify, via wildcards (* and ?), that all
 * copies are desired, the following logic was added to stop
 * the extract on the first instance of the requested file.
 */
	    if (Numrecs) {
		cp = fstat->f_src;
		while (*cp && *cp != '\n') {
	    		if ((*cp == '*') || (*cp == '?')) {
				wildc = YES;
				break;
	    		}
	    		cp++;

		}/*E while *cp ..*/
		if (!wildc) {
			free((char*)fstat->f_src);
			fstat->f_src = (char *) malloc (12);
			if (!fstat->f_src) {
			    PERROR "\n%s: %s%c\n", Progname,NOMEM,BELL);
		    	    exit(FAIL);
			}
	    		strcpy(fstat->f_src,"1extracted1");
	    		fstat->f_numleft =0;
			Numrecs--;
	    		if (!Numrecs) {
    	    			if (Seqno && skip <= Seqno) {
	    			    PERROR "\n%s: %s %s\n", Progname, CANTFSF, Magtdev);
	    			    perror(Magtdev);
				    printf("\n");
	    			    exit(FAIL);
	    			}
				printf("\n");
				exit(SUCCEED);
			}
		}/*E if !wildc */
	    }/*T if (Numrecs) */
	}/*T if (! Numrecs || (!(fstat = Lookup(Name))))*/ 
	else {
	    /* Getting here implies that the user has specified
	     * a list of names to be extracted, or tabled &
	     * the current file on the input volume is not one
	     * of the ones about which the user is concerned.
	     * The fsf (Forward Space File) will skip over the
	     * file and its' ANSI label sets (header & trailer)
	     * and place a pointer at the next HDR1 or the end.
	     */
	    fsf(2);
	}
}/*E FOREVER loop */

}/*E scantape() */
/**/
/*
 *
 * Function:
 *
 *	wbadlab
 *
 * Function Description:
 *
 *	This function saves a lot of repetative code by
 *	outputting the common information for all bad
 *	labels encountered.
 *
 * Arguments:
 *
 *	None
 *
 * Return values:
 *
 *	None, the function always exits to system control.
 *
 * Side Effects:
 *
 *	None
 *	
 */

wbadlab()
{
/*
 * +--> Local Variables
 */

int i;

/*------*\
   Code
\*------*/
	
PERROR "\n%s: %s\n", Progname, INVLD);

for (i=0; i < BUFSIZE+1; i++) {
	if (Labelbuf[i] < ' ' || Labelbuf[i] > '~') Labelbuf[i] = ' ';
}
filter_to_a(Labelbuf, i);
PERROR "%s", Labelbuf);
PERROR "\n%s: %s\n", Progname, EINVLD);
exit(FAIL);

}/*E wbadlabd() */

/**\\**\\**\\**\\**\\**  EOM  scantape.c  **\\**\\**\\**\\**\\*/
/**\\**\\**\\**\\**\\**  EOM  scantape.c  **\\**\\**\\**\\**\\*/
