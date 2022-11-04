
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#ifndef lint
static	char	*sccsid = "@(#)putfile.c	3.0	(ULTRIX)	4/21/86";
#endif	lint

/**/
/*
 *
 *	File name:
 *
 *		putfile.c
 *
 *	Source file description:
 *
 *		This file contains the logic to put 
 *		individual files on an output volume for
 *		the Labeled Tape Facility (LTF).	
 *		
 *
 *	Functions:
 *
 *		bflush()	Flushes Fortran Unformatted File
 *				record buffer data to output
 *				volume.
 *
 *		addrtyp()	?_? Used during Fortran Formatted
 *				File output record processing.
 *
 *		append()	Appends (writes) the current
 *				file to the output volume.
 *
 *		process()	The function that actually
 *				writes the ANSI file header
 *				and ANSI file trailer
 *				records on the output volume.
 *
 *		putfile()	Determines the "type" of file
 *				being put on the output volume
 *				and calls function "process" to
 *				cause the file to be output.
 *
 *	Usage:
 *
 *		n/a
 *
 *	Compile:
 *
 *	    cc -O -c putfile.c		<- For Ultrix-32/32m
 *
 *	    cc CFLAGS=-DU11-O putfile.c	<- For Ultrix-11
 *
 *
 *	Modification history:
 *	~~~~~~~~~~~~~~~~~~~~
 *
 *	revision			comments
 *	--------	-----------------------------------------------
 *	  01.0		2-May-85	Ray Glaser
 *			Create original version.	
 *
 *	  01.1		10-Sep-85	Suzanne Logcher
 *			Add logic to detect current century
 *
 *	  01.2		24-Sep-85	Suzanne Logcher
 *			Add logic to treat directory and subdirectory
 *			entries using ../ or ./ as relative pathnames
 *
 *	  01.3		31-Oct-85	Suzanne Logcher
 *			Add logic to create segmented files
 */

/*
 * +--> Local Includes
 */

#include	"ltfdefs.h"
char	*rindex();

/**/
/*
 *
 * Function:
 *
 *	bflush
 *
 * Function Description:
 *
 *	Flushes (writes to output volume) the Fortran
 *	Unformatted File  record buffer.
 *
 * Arguments:
 *
 *	char	*name	filename used when error in write
 *	char	*Inbuf	file writing buffer
 *
 * Return values:
 *
 *	none
 *
 * Side Effects:
 *
 *	Data is written to the output volume from the
 *	and the blocks written count is updated.
 */

bflush(name, Inbuf)
	char	*name;	/* filename */
	char	*Inbuf; /* File writing buffer */
{
if (Rb - Bb > 0) {
	while (Rb < Bb + Blocksize)
		*Rb++ = PAD;

	if (write(fileno(Magtfp), Inbuf, Blocksize) != Blocksize) {
	    PERROR "\n%s: %s %s\n", Progname, ERRWRF, name);
	    perror(Magtdev);
	    ceot();
	}
	Blocks++;
}
}/*E bflush() */
/**/
/*
 *
 * Function:
 *
 *	addrtyp
 *
 * Function Description:
 *
 *	Used during output to a volume for Fortran
 *	Unformatted File proccessing   ?_?
 *
 * Arguments:
 *
 *	char	*inch	?_?
 *	char	*typ	?_?
 *
 *
 * Return values:
 *
 *	none
 *
 * Side Effects:
 *
 *	not known at this time	
 */

addrtyp(inch, typ)
	char	*inch;
	char	*typ;
{
inch[4] = typ[0];
inch[5] = typ[1];

}/*E addrtyp() */
/**/
/*
 *
 * Function:
 *
 *	append
 *
 * Function Description:
 *
 *	Appends (writes) the given file to the output volume.
 *
 * Arguments:
 *
 *	char	*name	filename
 *	FILE	*fp	Pointer to file to output
 *	int	max	Maximum line length	
 *	int	type	Type of ANSI tape file being output
 *	int	tftype	True file type of file being output
 *	char	*Inbuf	File writing buffer
 *
 * Return values:
 *
 *	Returns a LONG count of number of blocks output.
 *
 *	Returns -1  and  -2  as error indicators.
 *
 * Side Effects:
 *
 *	
 */

long append(name, fp, type, tftype, max, Inbuf)
	char	*name;	/* filename */
	FILE	*fp;
	int	type, tftype, max;
	char	*Inbuf;	/* File writing buffer */
{
/*
 * +--> Local Variables
 */

int	done;		/* Boolean used for segmented records */
int	fillbuf;	/* Integer of amount of data to read to fill 
			 * line buffer to Blocksize */
char	*index();	/* subroutine to find first char in string */
int	length;		/* Length of line */
char	line[MAXBLKWRT+1];	/* Line read in from file to output */
int	midway;		/* Boolean used for segmented records */
int	notfini;	/* Boolean used for TEXT loop */
char	*p;		/* Pointer to the line input buffer */
char	*q;		/* Pointer to the line input buffer */
int	used = 0;	/* Number of characters used in Buffer for 
			 * segmented records */

/*------*\
   Code
\*------*/

Blocks = 0L;
p = Inbuf;

/*------*\
	TEXT FILE
\*------*/

if (type == TEXT) {
	    notfini = TRUE;
	    midway = FALSE;
	    q = line;
	    fillbuf = Blocksize;
	    while (notfini != EOF) {
		if (tftype != SYMLNK || Nosym) {
		    if ((i = read(fileno(fp), q, fillbuf)) <= 0)
			if (i < 0) {
			    PERROR "\n%s: %s %s\n", Progname, CANTRD, name);
			    perror(Progname);
			    exit(FAIL);
			}
			else
			    break;
		    q[i] = '\0';
		}
		else
		    strcpy(line, Inbuf);

		q = line;
		done = FALSE;
		while (done == FALSE) {
		    if (cp = index(q, '\n'))
			*cp = '\0';
		    else
			if (Format == VARIABLE)
			    break;

		    length = strlen(q);
		    if (length > max)
			return((long)(-1));

		    if (Format == VARIABLE) {
			if (&p[length+4] > &Inbuf[Blocksize]) {
		    	    while (p < &Inbuf[Blocksize])
				*p++ = PAD;
			    if (write(fileno(Magtfp), Inbuf, Blocksize) != Blocksize) {
	    			PERROR "\n%s: %s %s\n", Progname, ERRWRF, name);
	    			perror(Magtdev);
	    			ceot();
			    }
		    	    p = Inbuf;
		    	    Blocks++;
	        	}
			sprintf(p, "%04d%s", length+4, q);
			p = &p[length+4];
			q = ++cp;
		    }/*T if (Format == VARIABLE) */
		    else {
			if (&p[length+5] <= &Inbuf[Blocksize]) {
			    if (cp) {
				if (midway == FALSE)
				    sprintf(p, "%01d%04d%s", 0, length+5, q);
				else {
				    sprintf(p, "%01d%04d%s", 3, length+5, q);
				    midway = FALSE;
				}/*F if (midway == FALSE) */
				used += length + 5;
				p = &p[length+5];
				q = ++cp;
			    }/*T if (cp) */
			    else {
				strcpy(line, q);
				q = &line[length];
				fillbuf = Blocksize - length;
				done = TRUE;
			    }/*F if (cp) */
			}/* T if (&p[length+5] <= ... */
			else {
			    if (used + 5 < Blocksize) {
				if (midway == FALSE) {
				    sprintf(p, "%01d%04d%s", 1, Blocksize-used, q);
				    midway = TRUE;
				}/*T if (midway == FALSE) */
				else
				    sprintf(p, "%01d%04d%s", 2, Blocksize-used, q);
				q = &q[Blocksize-used-5];
				length -= Blocksize-used-5;
			    }/* T if (used + 5 < Blocksize) */ 
			    else
				while (p < &Inbuf[Blocksize])
				    *p++ = PAD;
			    used = 0;
			    if (write(fileno(Magtfp), Inbuf, Blocksize) != Blocksize) {
	    			PERROR "\n%s: %s %s\n", Progname, ERRWRF, name);
	    			perror(Magtdev);
	    			ceot();
			    }
			    p = Inbuf;
			    Blocks++;
			    if (cp)
				q[length] = '\n';
			    if (strlen(q) == 0) {
				q = line;
				fillbuf = Blocksize;
				done = TRUE;
			    }
		        }/*F if (&p[length+5] <= ... */
		    }/*F if (Format == VARIABLE) */
	    	}/*E while (done == FALSE) */
		if (tftype == SYMLNK && !Nosym)
		    notfini = EOF;
		if (Format == VARIABLE) {
		    length = strlen(q);
		    strcpy(line, q);
		    q = &line[length];
		    fillbuf = Blocksize - length;
		}
	    }/*E while (notfini != EOF */
	    if (p != Inbuf) {
		while (p < &Inbuf[Blocksize])
	    	    *p++ = PAD;
		if (write(fileno(Magtfp), Inbuf, Blocksize) != Blocksize) {
	    	    PERROR "\n%s: %s %s\n", Progname, ERRWRF, name);
	    	    perror(Magtdev);
	    	    ceot();
		}
		Blocks++;
	    }
}/*T if type == TEXT */


/*------*\
	BINARY FILE
\*------*/

if (type == BINARY) {
	while ((length = read(fileno(fp), p=Inbuf, Blocksize)) > 0) {
		if (length < Blocksize) {
			p = &p[length];
			while (p < &Inbuf[Blocksize])
				*p++ = PAD;
		}
		if (write(fileno(Magtfp), Inbuf, Blocksize) != Blocksize) {
	    	    PERROR "\n%s: %s %s\n", Progname, ERRWRF, name);
	    	    perror(Magtdev);
	    	    ceot();
		}

		p = Inbuf;
		Blocks++;

    	}/*E while length = read ..*/
	if (length < 0) {
	    PERROR "\n%s: %s %s\n", Progname, CANTRD, name);
	    perror(Progname);
	    exit(FAIL);
	}
}/*T if (type == BINARY) */


/*------*\
	COUNTED RECORD FILE 
\*------*/

if (type == COUNTED) {
    while ( fread( &length, sizeof (short), 1, fp)) {
	if ( length == -1 || p+length+4 > &Inbuf[Blocksize]) {
		while ( p < &Inbuf[Blocksize])
			*p++ = PAD;

		if (write(fileno(Magtfp), Inbuf, Blocksize) != Blocksize) {
	    	    PERROR "\n%s: %s %s\n", Progname, ERRWRF, name);
	    	    perror(Magtdev);
	    	    ceot();
		}

		p = Inbuf;
		Blocks++;
	}/*E if length == -1 ..*/

	sprintf( p, "%04d", length+4);
	fread( p+4, 2, (length+1)/2, fp);

	/*
	 * Fudge for COUNTED records always beginning
	 * on a word boundary in file
	 */
	p += length+4;
    }/*E while fread ..*/
		
    if (p != Inbuf) {
	while (p < &Inbuf[Blocksize])
		*p++ = PAD;

	if (write(fileno(Magtfp), Inbuf, Blocksize) != Blocksize) {
    	    PERROR "\n%s: %s %s\n", Progname, ERRWRF, name);
    	    perror(Magtdev);
    	    ceot();
	}
	Blocks++;

    }/*E if p != Inbuf */
}/*T if type == counted */


/*------*\
	FORTRAN UNFORMATTED FILE
\*------*/

if (type == FUF) {
	/*
	 * +--> Local Variables
	 */
	long	bytcnt;
	int	irec;
	int	lastbit;
	int	nrec;
	long	nbytes, nbytesl;
	char	rectype;
	int	res, resl;
	char	*rp;

	/*
	 * Initialize buffer pointers
	 */
	Bb = Inbuf;
	Rb = Bb;
	rp = Rb + RECOFF;

	/*
	 * Loop over all records in the file.  The unformatted
	 * format for f77 is as follows: each record is
	 * followed and preceded by a long int which contains
	 * the byte count of the record excluding the 2 long
	 * integers.
	 */
	while ((res = read(fileno(fp), &nbytes, sizeof(long))) > 0) {
		if (nbytes > 0L) {
			bytcnt = nbytes;
			nrec = ((int)nbytes-1) / MAXREC6;
			irec = nrec;

			/*
			 * if record is greater than MAXREC6,
			 * then output it in chunks of MAXREC6
			 * first.
			 */
			 while (irec--) {
				if (Rb - Bb + MAXRECFUF > Blocksize) {
					bflush(name, Inbuf);
					Rb = Bb;
					rp = Rb + RECOFF;
				}
				/*
			 	* Read record in chunks of MAXREC6
			 	*/
				res = read(fileno(fp), rp, MAXREC6);
				if (res < 0) {
			    	PERROR "\n%s: %s%c\n\n",
					Progname, EOFINM, BELL);
			    	return((long)(-1));
				}
				if (res != MAXREC6) {
				    PERROR "%\n%s: %s%c\n\n",
					Progname, WRLINM, BELL);
				return((long)(-1));
				}

				/*
				 * Set up the record type.  Note:
				 * to get here the record must
				 * be > MAXREC6 and it must be
				 * split.  Thus there must be a
				 * FIRST record and all other
				 * records must be MIDDLE records.
				 * There could be a LAST record
				 * if the record was an exact
				 * multiple of MAXREC6.
				 */
				 if (irec == nrec - 1)
					rectype = FIRST;
				else if (bytcnt > MAXREC6)
					rectype = MIDDLE;
				else
					rectype = LAST;

				/*
				 * Now output chunk of record
				 * on volume as VARIABLE format
				 */
				resl = res + RECOFF;

				sprintf(Rb, "%04d", resl);
				addrtyp(Rb, &rectype);

				bytcnt -= res;
				Rb += resl;
				rp = Rb + RECOFF;

			}/*E While irec-- */

			/*
			 * Output final part (unless exact
			 * multiple of MAXREC6) of record or
			 * all of it if record is < MAXREC6.
			 */
			 if (bytcnt > 0) {
				lastbit = bytcnt;

				if (Rb - Bb + lastbit + RECOFF > Blocksize) {
					bflush(name, Inbuf);
					Rb = Bb;
					rp = Rb + RECOFF;
				}

				res = read(fileno(fp), rp, lastbit);

				if (res < 0) {
			    	    PERROR "\n%s: %s%c\n\n",
					Progname, EOFINM, BELL);
				    return((long)(-1));
				}
				if (res != lastbit) {
				    PERROR "%\n%s: %s%c\n\n",
					Progname, WRLINM, BELL);
				    return((long)(-1));
				}

				if (lastbit == nbytes)
					rectype = ALL;
				else
					rectype = LAST;

				resl = res + RECOFF;

				sprintf(Rb, "%04d", resl);
				addrtyp(Rb, &rectype);

				Rb += resl;
				rp = Rb + RECOFF;

			}/*E if bycnt > 0 */

		}/*T if nbytes > 0L */
		else {
			if (Rb - Bb + RECOFF > Blocksize) {
				bflush(name, Inbuf);
				Rb = Bb;
				rp = Rb + RECOFF;
			}

			sprintf(Rb, "%04d", RECOFF);
			rectype = ALL;
			addrtyp(Rb, &rectype);

			Rb += RECOFF;
			rp = Rb + RECOFF;

		}/*F if nbytes > 0L */

		res = read(fileno(fp), &nbytesl, sizeof(long));

		if (res < 0) {
			PERROR "\n%s: %s%c\n\n",
				Progname, EOFINM, BELL);
			return((long)(-1));
		}
		if (nbytesl != nbytes) {
			PERROR "\n%s: %s%c\n\n",
			    Progname, BFRCNE, BELL);
			return((long)(-1));
		}
	
	}/*E while res= read ..*/
	if (res < 0) {
	    PERROR "\n%s: %s %s\n", Progname, CANTRD, name);
	    perror(Progname);
	    exit(FAIL);
	}

	bflush(name, Inbuf);

}/*E if type == FUF */

/*
 *	Return to the caller the number of blocks
 *	that were output to the volume.
 */
return(Blocks);


}/*E append()*/
/**/
/*
 *
 * Function:
 *
 *	process
 *
 * Function Description:
 *
 *	This functions writes the ANSI file header labels,
 *	file data, and ANSI file trailer labels to the
 *	output volume.
 *
 * Arguments:
 *
 *	char	*reallong;	Contains the full pathname & file name
 *	char	*longname;	Contains the pathname & file name
 *	char	*shortname;	File name only
 *	int	type;		Ultrix disk file type
 *
 * Return values:
 *
 *	none
 *
 * Side Effects:
 *
 *	If the function encounters an error condition during
 *	the output of the file, a message is output to 
 *	"stderr" and the function exits to system control.	
 */

process(reallong, longname, shortname, type)
	char	*reallong;	/* Full path name_+_filename */
	char	*longname;	/* Relative path name_+_filename */
	char	*shortname;	/* Contains the file name only */
	int	type;		/* Type of Ultrix disk file */
{
/*
 * +--> Local Variables
 */
char	ansift;		/* Save ANSI file type character from HDR2
			 * for later verbose messge output.
			 */
int	ch;		/* Character returned from getc to check line 
			 * size for TEXT files */
char	crecent;	/* Creation date century */
char	*ctime();	/* Declare ctime routine */
char	dummy[MAXPATHLEN+1]; /* dummy variable */
int	found = 0;	/* a head link is not on tape yet */
FILE	*fp;
int	hlink = 0;	/* Flag set for field in HDR2 if hard link */
char	Inbuf[MAXBLKWRT+1]; /* Buffer for writing files */
char	interchange[18];	/* Interchange name */
int	length;		/* size of complete filename */
int	linkflag = NO;	/* Used to trigger output linked files 
			 * message. Set to YES if the current file 
			 * is linked to a file that has already 
			 * been appended to the output volume.
			 */
int	lnkseq = 0;	/* Field in HDR2 for file sequence number that
			 * a hard link points to */
struct	tm *localtime();
struct	ALINKBUF *lp;	/* Node for hard link files */
struct	tm *ltime;
int	max;		/* maximum line length */
int	nlinks;		/* number of links to file */
int	otype = 0;	/* If outputting sym link file, save SYMLNK 
			 * file type to check in append routine */
char	pathname[MAXPATHLEN+1]; /* path name */
int	version = 1;	/* version number (1 is default) */


/*------*\
   Code
\*------*/

if (strlen(longname) >= MAXPATHLEN) {
	PERROR "\n%s: %s %s%c\n\n",
		Progname, FNTL, longname, BELL);
	/*
	 * A string that long is probably fatal...
	 */
	exit(FAIL);
}
strcpy(dummy, longname);

pathname[0] = '\0';
if (cp = rindex(dummy, '/')) {
	*++cp = '\0';

/*
 * ?_? NOTE:	When we  get to putting loong file names on
 *		the output volume.. This logic will need to
 *		be updated...
 */
	if (strlen(dummy) >= MAXPATHLEN) {
		PERROR "\n%s: %s %s%c\n\n",
			Progname, FNTL, longname, BELL);
		/*
	 	* A string that long is probably fatal...
	 	*/
		exit(FAIL);
	}
	strcpy(pathname, dummy);
}
/*
 * Make the ANSI interchange file name
 */ 
strcpy(dummy,shortname);
j = 0;
if (!(filter_to_a(dummy,REPORT_ERRORS)))
    if (Warning) {
	PERROR "\n%s: %s %s", Progname, NONAFN, shortname);
	PERROR "\n%s: %s %s", Progname, INVVID2, dummy);
	j++;
    }
if (strlen(dummy) > 17) {
    /*
     * If user has requested warnings, tell user file name
     * is too long for HDR1 and that it will be truncated.
     */
    dummy[17] = '\0';
    if (Warning) {
	PROMPT "\n%s: %s %s%c", Progname, FNTL, shortname, BELL);
	j++;
    }
}
if ((j > 0) && Warning)
    PERROR "\n%s: %s\n\n", Progname, FILENNG);

strcpy(interchange, dummy);
interchange[17] = '\0';
if ((fp = fopen(reallong, "r")) == NULL) {
	PERROR "\n%s: %s %s%c\n\n", Progname, CANTOPEN, reallong, BELL);
	perror(Progname);
	exit(FAIL);
}
nlinks = Inode.st_nlink;

/*
 * Get file owner's name string for HDR3.
 */
if (getpw(Inode.st_uid,Name)) {
	PERROR "\n%s: %s %d%c\n", Progname, CANTFPW, Inode.st_uid, BELL);
	strcpy(Name, "          ");
}
/*
 * Owner id field in HDR3 label is maximum of 10 characters
 */
for (j=0; j < 11 && Name[j] && Name[j] != ':'; j++)
	Owner[j] = Name[j];

Owner[j] = NULL;

cp = ctime(&Inode.st_mtime);
if (cp[20] == '2' && cp [21] == '0')
    crecent = '0';
else
    crecent = ' ';
ltime = localtime(&Inode.st_mtime);
Blocks = 0L;
max = 0;

if (type == TEXT || type == SYMLNK && Nosym) {
	/* 
	 * If we find a really long line in the text,
	 * we should output it as a spanned/segmented 
	 * record rather variable length record.
	 *
	 * Find length of longest line.
	 */
	Format = 0;
	ch = TRUE;
	while (ch != EOF) {
	    length = 0;
	    while ((ch = getc(fp)) != EOF && ch != '\n')
		length++;
	    if (length > max)
		max = length;
	}/*E while (ch) */
	fclose(fp);
	if ((fp = fopen(reallong, "r")) == NULL) {
	    PERROR "\n%s: %s %s%c\n\n", Progname, CANTOPEN, reallong, BELL);
	    exit(FAIL);
	}
	if (max + 5 > Reclength && Reclength != MAXRECSIZE) {
	    PERROR "\n%s: %s %s%c\n\n", Progname, RECLTS, longname, BELL);
	    return;
	}/*T if (max + 5 > Reclength ... */
	else
	    if (max + 5 > Reclength || max + 5 > Blocksize) {
		Format = SEGMENT;
		Maxrec = max;
	    }/*T if (max + 5 ... */
	    else {
		Format = VARIABLE;
		Maxrec = max + 4;
	    }/*F if (max + 5 ... */
}/*E if (type == TEXT || type == SYMLNK && Nosym) */

fseek(fp, 0L, 0);

/****\
 *	Write  HDR1  on volume..
 ****/
sprintf(Dummy,
	"HDR1%-17.17s%-6.6s%04d%04d%04d%02d%c%02d%03d %02d%03d %06d%-13.13s%7.7s",
	interchange, Volid, Fsecno, Fseqno,
	(version-1) / 100 + 1, (version-1) % 100,
	crecent, ltime->tm_year, ltime->tm_yday+1, 99, 366, 0,
	type == FUF ? "DECFILE11A" : IMPID, Spaces);
if (Ansiv != '4') 
	filter_to_a(Dummy,IGNORE_ERRORS);

if (write(fileno(Magtfp), Dummy, BUFSIZE) != BUFSIZE) {
    PERROR "\n%s: %s %s\n", Progname, ERRWRF, longname);
    perror(Magtdev);
    ceot();
}

if (type == FUF)
	max = MAXREC4;


/****\
 *	Write  HDR2  on volume..
 ****/

/*
 * Directories always have a link count > 1 ...
 * So, we don't bother to enter them in the linked files list.
 * Also, can't hard link directories ...
 */
if (type != DIRECT && nlinks > 1 && !Nosym) {
    hlink = 1;
    /*	If this file is linked to another,
     *	find the ANSI file sequence number that it
     *	is linked to.
     */
    for (lp = A_head; lp; lp = lp->a_next)
	if (lp->a_inum == Inode.st_ino && lp->a_dev == Inode.st_dev) {
	    found++;
	    break;
	}

    /* If we found the file that this file is linked to in our list,
     * get it's file sequence number and set linkflag.
     */
    if (found) {
	lnkseq = lp->a_fseqno;
	linkflag = YES;
    }
    else {
	/* Else, enter info into a node for a possible reference
	 */
	lp = (struct ALINKBUF *) malloc(sizeof(*lp));
	if (lp == NULL) {
	    PERROR "\n%s: %s%c%c\n\n", Progname, NOMEM, BELL, BELL);
	    exit(FAIL);
	}/*T if lp == NULL */
	else {
	    lp->a_next = A_head;
	    A_head = lp;
	    lp->a_inum = Inode.st_ino;
	    lp->a_dev = Inode.st_dev;
	    lp->a_fsecno = Fsecno;
	    lp->a_fseqno = Fseqno;
	    lp->a_pathname = (char *) malloc(strlen(longname) + 1);
	    if (lp == NULL) {
	        PERROR "\n%s: %s%c%c\n\n", Progname, NOMEM, BELL, BELL);
	        exit(FAIL);
	    }/*T if lp == NULL */
	    else {
		strcpy(lp->a_pathname, longname);
	    }
	    lnkseq = 0;
	}/*F if lp == NULL */
    }/*F if (found) */
}/*E if (type != DIRECT && nlinks > 1) */

/*
 * Perform any special functions required for symbolic links
 */
if (type == SYMLNK) {
	otype = type;
	type = TEXT;
	if (!Nosym) {
	    /*
	     * Read the link to find out what
	     * it points to.
	     */
	    dummy[0] = 0;
	    if ((ch = readlink(reallong, dummy, MAXNAMLEN)) < 0) {
		PERROR "\n%s: %s %s\n",Progname, CANTRSL, reallong);
		perror(Progname);
	    }
	    else
		dummy[ch] = '\0';
	    linkflag = YES;
	    /*
 	     * For Symbolic links, the file data is the full 
 	     * path name of the file pointed to by the 
	     * symbolic link.
 	     */
	    strcpy(Inbuf,dummy);
	    max = strlen(Inbuf);
	    Inbuf[max] = '\n';
	    Inbuf[max+1] = '\0';
	    if (max + 5 > Reclength || max + 5 > Blocksize || max + 5 > MAXRECSIZE) {
		Format = SEGMENT;
		Maxrec = max;
	    }
	    else {
		Format = VARIABLE;
		Maxrec = max + 4;
	    }
	}/*T if (!Nosym) */ 
}/*T if (type == SYMLNK) */

/******
 *
 * Calculate the number of HDR / EOF  labels that will be needed
 * to contain the full path/file name.
 *
 */

length = (strlen(longname)) -1;

Lhdrl = 3;
Leofl = 0;

length -= 36;
/*
 * Determine the number of HDR labels needed.
 */
while ((length > 0) && (Lhdrl !=9)) {
	length -= 76;
	Lhdrl++;
}
/*
 * Determine the number of EOF labels needed.
 */
if (length > 0) {
	Leofl = 2;
	while ((length >0) && (Leofl != 9)) {
		length -= 76;
		Leofl++;
	}
}
/* Set the format of the record if it is not a TEXT file.  If it is,
 * it has already been set. 
 */
if (type == FUF || type == COUNTED)
    Format = VARIABLE;
else
    if (type != TEXT)
	Format = FIXED;
if (type == FUF)
    Maxrec = max + 4;
else
    if (type != TEXT)
	Maxrec = Reclength;

sprintf(Dummy,
    "HDR2%c%05d%05d%06o%04d%04d%04d%3.3s%c%010ld%1.1d%1.1d%1.1d00%-28.28s",
	Format, Blocksize, Maxrec,
	Inode.st_mode & 0177777,
	Inode.st_uid, Inode.st_gid,
	lnkseq, Tftypes,
	(type == BINARY) ? 'M' : Carriage,
	(type == SYMLNK && !Nosym) ? max : Inode.st_size, 
	Lhdrl, Leofl, hlink, Spaces);
/*
 * Save ANSI file type for verbose messgae use.
 */
ansift = Dummy[4];

if (write(fileno(Magtfp), Dummy, BUFSIZE) != BUFSIZE) {
    PERROR "\n%s: %s %s\n", Progname, ERRWRF, longname);
    perror(Magtdev);
    ceot();
}

if (!Noheader3) {
    /****\
     *	Write  HDR3  on volume..
     ****/
    sprintf(Dummy, "HDR3%010ld%-10.10s%-20.20s%-36.36s",
	Inode.st_mtime,
	Owner,
	Hostname,
	longname);

    if (Ansiv != '4') 
	filter_to_a(Dummy,IGNORE_ERRORS);

    if (write(fileno(Magtfp), Dummy, BUFSIZE) != BUFSIZE) {
        PERROR "\n%s: %s %s\n", Progname, ERRWRF, longname);
        perror(Magtdev);
        ceot();
    }

    /****\
     *	Write additional file header labels (HDR4 - HDRn)
     *	to contain the remaining portions of very long
     *	path/file names.
     ****/

    if (Lhdrl > 3) {
	int i;

	for (i=4,j=36; i<=Lhdrl; i++,j += 76) {

		sprintf(Dummy, "HDR%1.1d%-76.76s", i,&longname[j]);	

		if (Ansiv != '4') 
			filter_to_a(Dummy,IGNORE_ERRORS);

		if (write(fileno(Magtfp), Dummy, BUFSIZE) != BUFSIZE) {
		    PERROR "\n%s: %s %s\n", Progname, ERRWRF, longname);
		    perror(Magtdev);
		    ceot();
		}
	}	
    }
}/* T !Noheader3 */

/****\
 *	Write an end-of-file mark on tape that separates 
 *	the file header labels from the data ....
 ****/

weof();

/****\
 *	Write the actual file data on the output volume.
 ****/

if ((Blocks = append(longname, fp, type, otype, max, Inbuf)) < 0L)
    if (Blocks == (long)(-1)) {
	PERROR "\n%s: %s %s%c\n\n", Progname, ERRWRF, longname, BELL);
	ceot();
    }

if (fclose(fp) < 0) {
    PERROR "\n%s: %s %s\n", Progname, CANTCLS, longname);
    perror(longname);
    exit(FAIL);
}
/* If outputting symlnk, change type from TEST back to SYMLNK
 */
if (otype)
    type = otype;
/****\
 *	Write an end-of-file mark on tape
 *	Separates the data from the file trailer labels...
 ****/

weof();

/****\
 *	Write  EOF1  to output volume..
 ****/

sprintf(Dummy,
	"EOF1%-17.17s%-6.6s%04d%04d%04d%02d%c%02d%03d %02d%03d %06ld%-13.13s%7.7s",
	interchange, Volid, Fsecno, Fseqno,
	(version-1) / 100 + 1, (version-1) % 100,
	crecent, ltime->tm_year, ltime->tm_yday+1, 99, 366, Blocks,
	type == FUF ? "DECFILE11A" : IMPID, Spaces);
if (Ansiv != '4') 
	filter_to_a(Dummy,IGNORE_ERRORS);

if (write(fileno(Magtfp), Dummy, BUFSIZE) != BUFSIZE) {
    PERROR "\n%s: %s %s\n", Progname, ERRWRF, longname);
    perror(Magtdev);
    ceot();
}

/****\
 *	Write  EOF2  to output volume..
 ****/

sprintf(Dummy,
    "EOF2%c%05d%05d%06o%04d%04d%04d%3.3s%c%010ld%1.1d%1.1d%1.1d00%-28.28s",
	Format, Blocksize, Maxrec,
	Inode.st_mode & 0177777,
	Inode.st_uid, Inode.st_gid,
	lnkseq, Tftypes,
	(type == BINARY) ? 'M' : Carriage,
	(type == SYMLNK && !Nosym) ? max : Inode.st_size, 
	Lhdrl, Leofl, hlink, Spaces);

if (write(fileno(Magtfp), Dummy, BUFSIZE) != BUFSIZE) {
    PERROR "\n%s: %s %s\n", Progname, ERRWRF, longname);
    perror(Magtdev);
    ceot();
}
if (!Noheader3) {
    /*
     *	Write  EOF3 - EOFn  to output volume.
     *
     *	EOF3 thru EOF'n'  contain the remaining characters of a
     *	very long path/file name. If no characters need be stored
     *	in the EOF labels for a path/file name, at a minimum we
     *	output the same number of trailer lables as header labels
     *	in order to keep things tidy.
     */
    if (Leofl) {
	int i;

	for (i=3,j=492; i<= Leofl; i++,j += 76) {
		sprintf(Dummy, "EOF%1.1d%-76.76s", i,&longname[j]);	
		if (Ansiv != '4') 
			filter_to_a(Dummy,IGNORE_ERRORS);
		if (write(fileno(Magtfp), Dummy, BUFSIZE) != BUFSIZE) {
    		    PERROR "\n%s: %s %s\n", Progname, ERRWRF, longname);
    		    perror(Magtdev);
    		    ceot();
		}
	}
    }/* if (Leofl) */
    /* 
     * If Leofl = 0, no EOF lables are required to hold path/file
     * name characters. Therefore, we output as many space filled EOFx
     * lables as we need to in order to have the same number of EOF
     * trailer labesl as HDR header labels to keep ANSI happy.  If some 
     * number of EOF labels were required for path/file characters,
     * they will have been output by the code above. If this does not
     * result in the same number of EOF labels as HDR labels, write out
     * the required number of EOF labels.
     */

    if (Leofl < Lhdrl) {
	/*       \----> By accident, all HDR and all EOF labels may
 	 *		have been needed, and have been output, to
	 *		contain path/file name characters. Therefor,
	 *		the following code has no function. The case
	 *		referred to as an accident is one where the
	 *		path/file name is in the order of 949 to
	 *		1024 characters in length.
	 */ 
	if (!Leofl) /* If no EOF labels were required for path/file
		     * characters, begin with EOF3, and continue
		     * with blank EOF lables until we output as many
		     * EOF labesl as HDR labels.
		     */
		j = 3;	
	else
		j = Leofl + 1; /* If padding out EOF labels to the
				* same number of HDR labels  and
				* some EOF labels have been used for
				* path/file name characters, start with
				* the next sequential label number.
				*/
	while (j <= Lhdrl) {
		sprintf(Dummy, "EOF%1.1d%-76.76s", j++,Spaces);	
		if (write(fileno(Magtfp), Dummy, BUFSIZE) != BUFSIZE) {
    		    PERROR "\n%s: %s %s\n", Progname, ERRWRF, longname);
    		    perror(Magtdev);
    		    ceot();
		}
	}
    }/* if (Leofl < Lhdrl)*/
}/* T !Noheader3 */

/*****\
 *	Write tape mark to separate us from next label group
 *	if we are going to output more files.
 *****/

weof();

term_string(shortname,DELNL,TRUNCATE);

if (Verbose) {
	printf("c<%s>%c  %s%s",Tftypes, ansift, pathname, shortname);
#if 0
	/* If we decide we care about tape blocks, put
	 * this logic back in. -else- Just announce the
	 * byte count.
	 */
	if (Blocks == 1L)
		printf(",\t001%s   ", TAPEB);
	else
		printf(",\t%03ld%s  ", Blocks, TAPEBS);
#endif
	if (Inode.st_size == 1L || (type == SYMLNK && !Nosym && max == 1))
		printf(",\t0001%s   ", BYTE);
	else
	    if (type == SYMLNK && !Nosym)
		printf(",\t%04d%s  ", max, BYTES);
	    else
		printf(",\t%04ld%s  ", Inode.st_size, BYTES);
	if (linkflag || type == SYMLNK) {
		if (strlen(longname) > 19 || strlen(dummy) > 19) 
			printf("\n       ");
		if (type == SYMLNK)
			printf("%s %s", SLINKTO, dummy);
		else 
			printf("%s %s", HLINKTO, lp->a_pathname);
	}
	printf("\n");
}/*T if Verbose */
else
	printf("c  %s%s\n", pathname, shortname);

/*	Increment the ANSI file sequence number for the next
 *	file to go out (if any).
 */
Fseqno++;

}/*E process() */
/**/
/*
 *
 * Function:
 *
 *	putfile
 *
 * Function Description:
 *
 *	This function determines the Ultrix disk file type
 *	(if required) and calls function "process" to output
 *	a file to the volume.
 *
 * Arguments:
 *
 *	char	*longname	Full pathname_+_filename
 *	char	*shortname	File name only
 *--
 * Accesses Global variable:
 *--
 *	int	Dfiletype	Default/designated Ultrix
 *				disk file type.
 *
 * Return values:
 *
 *	none
 *
 * Side Effects:
 *
 *	If the user has specified a filename that is in fact
 *	a directory -AND- for that argument the user has
 *	specified a default/designated Ultrix disk file
 *	type, then ALL files under that heirarchy will take
 *	on the attributes appropriate for the given default
 *	file type. The Dfiletype variable should only be
 *	applied to singular file names, applying it to
 *	a directory may or may not be what the user intends.
 *
 *	If errors occur during the file type determination
 *	process, the function outputs a message to "stderr"
 *	and exits to system control.	
 */

DIR	*dirfile;
struct	stat inodes;
char	dirent[14+1];
int	tfiletype;		/* Result from Filetype */
struct	direct	*dirp;
char	sbuf[MAXPATHLEN+1];
char	fullpath[MAXPATHLEN+1];	/* Full pathname */
char	dummy4[MAXPATHLEN+1];	/* The current working file/directory */

/* #define DEBUG 	/* turn on debug output */

putfile(longname, shortname, iflag, workdir)
	char	*longname;	/* Pathname_+_filename */
	char	*shortname;	/* File name only */
	int	iflag;		/* If iflag = 1, then stdin */
	char	*workdir;	/* Working directory */
{
/*
 * +--> Local Variables
 */

long	p;		/* WAS AN int, MUST BE long FOR PDP */
int	savindex;	/* index into sbuf; the place to terminate on return */

/*------*\
   Code
\*------*/

/*
 * We build dummy4[] for the fstat call, instead of adding the file
 * name to the end of fullpath.  This leaves the last component of
 * fullpath always being a directory, which later turns into sbuf.
 * Then, new directories are added to the end of fullpath, just after
 * we have descended the new directory.  NOTE: in this manner, fullpath
 * always points to the current directory!  fullpath components are
 * popped off just before we chdir up to the new level.  Search for
 * keyword "HERE".
 */

#ifdef DEBUG
  printf("\n\nIn putfile:\n");
  printf("	longname: %s\n", longname);
  printf("	shortname: %s\n", shortname);
  printf("	workdir: %s\n\n", workdir);
#endif

if (longname[0] == '/' || !(strcmp(shortname, "/"))) {
    if (strlen(longname) == 0)
	strcpy(dummy4, shortname);
    else
	strcpy(dummy4, longname);
}
else {
    strcpy(dummy4, workdir);
    strcat(dummy4, "/");
    strcat(dummy4, longname);
}

#ifdef DEBUG
	printf("fullpath is Currently: %s\n",
		strlen(fullpath) > 0 ? fullpath : "Unknown");
	printf("about to stat(%s)\n",dummy4);
#endif

/* Do the correct "stat" call
 */
i = stat(dummy4, &inodes);
j = lstat(dummy4, &Inode);
if (i < 0 || j < 0)
    if (skip && iflag == 1)
	return(EOF);
    else {
	if ((Inode.st_mode & S_IFMT) == S_IFLNK && i < 0)
	    PERROR "\n%s: %s %s%c\n", Progname, CANTSTS, dummy4, BELL);
	else
	    PERROR "\n%s: %s %s%c\n", Progname, CANTSTW, dummy4, BELL);
	perror(Progname);
	return(EOF);
    }
if (Nosym)
    Inode = inodes;
/*
 * Go put any required directory entries on the output volume.
 */

if (Ansiv != '3' && !Noheader3)
    if ((i = putdir(longname, shortname, workdir)) == EOF)
	return;

/* Check if current file (dummy4) is a directory - start recursive volume creation
 */

tfiletype = Filetype(dummy4, Progname);

if ((Inode.st_mode & S_IFMT) == S_IFDIR) {
	if (chdir(dummy4) < 0) {
	    /* Check if error is permission denied
	     */
	    if (errno != EACCES) {
	        PERROR "\n%s: %s %s%c\n", Progname, CANTCHD, dummy4, BELL);
	        perror(Progname);
		exit(FAIL);
	    }
	    else {
	        PERROR "\n%s: %s %s%c\n", Progname, CANTCHW, dummy4, BELL);
	        perror(Progname);
		return;
	    }
	}
#ifdef DEBUG
	printf("just chdir'd down to %s\n",dummy4);
#endif
	/*
	 * HERE, we update fullpath to reflect where we are currently,
	 * so we can later get back by popping the last component,
	 * search for keyword "HERE".
	 */
	strcpy(fullpath, dummy4);

#ifdef DEBUG
	printf("new fullpath is: %s\n", fullpath);
#endif

	/* Read through current directory
	 */
	if (!( dirfile = opendir( "."))) {
		PERROR "\n%s: %s %s%c\n\n", Progname, CANTOD, longname, BELL);
		perror(Progname);
		exit(FAIL);
	}
	while (dirp = readdir(dirfile)) {

		/* Make sure we don't read empty slots in the
		 * directory ...
		 */	
		if (!dirp->d_ino)
			continue;

		/* Likewise, don't try to process the current
		 * and previous directory entries which are
		 * present in all directories.
		 */
		if (!strcmp(".", dirp->d_name) ||
		    !strcmp("..", dirp->d_name))
			continue;

		/* Get new contents of sbuf 
		 */
    		strcpy(sbuf, longname);
		savindex = strlen(sbuf);
		strcat(sbuf, "/");
		strcat(sbuf, dirp->d_name);
		
		/*
		 * RECURSIVE UPON THINESELF !
		 *
		 * But, first save a pointer to where we are in the
		 * current directory and close it, lest we have too
		 * many open files dangling about the premises. 
		 */

		p = telldir(dirfile);

		/* Save the directory name because as soon as we
		 * issue the closedir we will lose the "dirp"
		 * pointer !
		 */
		strcpy(dirent, dirp->d_name);
		closedir(dirfile);

		/* RECURSE !*@#$!
		 */
/**************
printf("push sbuf = %s, dirent = %s, p = %ld\n", sbuf, dirent, p);
printf("push fullpath = %s\n", fullpath);
***************/

		putfile(sbuf, dirent, iflag, workdir);
		sbuf[savindex] = '\0';

		/* When we return, re-open and position the
		 * current directory.
		 */
		dirfile = opendir(".");
		seekdir(dirfile, p);
#ifdef DEBUG
		printf("Continuing with opendir...");
#endif
	}/*E while dirp ..*/

#ifdef DEBUG
	printf("DONE with opendir...\n\n");
#endif
	closedir(dirfile);

	/*
	 * HERE: pop 1 off end of fullpath to get back up 1 level,
	 *       check if going up to / 
	 */
	if ((strlen(fullpath) > 1) && (cp = rindex(fullpath, '/'))) {
	    if (cp == fullpath)
		cp++;
	    *cp = '\0';
	}
#ifdef DEBUG
	printf("\nNew working directory will be: %s\n\n", fullpath);
#endif

	if (chdir(fullpath) < 0) {
	    PERROR "\n%s: %s %s%c\n", Progname, CANTCHD, fullpath, BELL);
	    exit(FAIL);
	}

#ifdef DEBUG
	printf("just chdir'd back up to %s\n", fullpath);
#endif
    	return; 

}/*E if ((Inode.st_mode & S_IFMT) == S_IFDIR) */

if ((Inode.st_mode & S_IFMT) == S_IFCHR) {
special:
	PERROR "\n%s: %s %s%c", Progname, SPCLDF, longname, BELL);
	return;
}
	
if ((Inode.st_mode & S_IFMT) == S_IFBLK)
	goto special;

if ((Inode.st_mode & S_IFMT) != S_IFDIR) {
    /*	Has the user given us a default Ultrix disk file
     *	type for this file ?  If yes, use it and bypass our
     *	file determination logic.
     */
    if (Dfiletype) {
	/*
	 * Go put a file on the output volume,
	 * making sure user is aware s/he is overriding our
	 * disk file type determination logic.
	 */
	PROMPT "\n%s: %s %s%c\n", Progname, USEDF, shortname, BELL);
	process(dummy4, longname, shortname, Dfiletype);
    }/*T if Dfiletype */
    else {
	/* Go put a file on the output volume.
	*/
	if (tfiletype != EOF) {
	    process(dummy4, longname, shortname, tfiletype);
	}
    }/*F if Dfiletype */
    return;

}/* if ((Inode.st_mode & S_IFMT) != S_IFDIR) */
else
    return;
}/*E putfile() */

/**\\**\\**\\**\\**\\**  EOM  putfile.c  **\\**\\**\\**\\**\\*/
/**\\**\\**\\**\\**\\**  EOM  putfile.c  **\\**\\**\\**\\**\\*/
