
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#ifndef lint
static	char	*sccsid = "@(#)xtractf.c	3.0	(ULTRIX)	4/21/86";
#endif	lint

/**/
/*
 *
 *	File name:
 *
 *		xtractf.c
 *
 *	Source file description:
 *
 *		Contains the functions that read data from the
 *		input volume and places it in the resultant
 *		output disk file.
 *
 *	Functions:
 *
 *		checkdir()	Check to see if a directory exists
 *				& create missing directories
 *
 *		fufcnv()	Converts a Fortran Unformatted File
 *				(from input volume) to correct Ultrix
 *				disk file format
 *
 *		getlen()	Get the length of the next variable-
 *				length record from the input volume
 *
 *		xtractf()	Top  level logic to extract files
 *				from the input volume
 *
 *	Usage:
 *
 *		n/a
 *
 *	Compile:
 *
 *	    cc -O -c xtractf.c		<- For Ultrix-32/32m
 *
 *	    cc CFLAGS=-DU11-O xtractf.c	<- For Ultrix-11
 *
 *
 *	Modification history:
 *	~~~~~~~~~~~~~~~~~~~~
 *
 *	revision			comments
 *	--------	-----------------------------------------------
 *	 01.0		25-April-85	Ray Glaser
 * 			Create original version
 *
 *	 01.1		4-Sep-85	Suzanne Logcher
 *			Correct use of Noheader3 in xtractf()
 *
 *	 01.2		4-Oct-85	Suzanne Logcher
 *			Add logic to extract a segmented file
 */

/*
 * +--> Include File(s)
 */

#include "ltfdefs.h"

/**/
/*
 *
 * Function:
 *
 *	checkdir
 *
 * Function Description:
 *
 *	This function checks to see if a directory exists.
 *	It creates those directories that are not found to exist.	
 *
 * Arguments:
 *
 *	char	*path	Pointer to the string containing the
 *			directory path structure to be checked.
 *
 * Return values:
 *
 *	TRUE	if the directory existed
 *	FALSE	if the directory had to be created
 *
 * Side Effects:
 *
 *	This functions "forks" a child process to create a
 *	needed directory.
 *
 *	If the function fails to create the required directory,
 *	an error message is output to  stderr  and the function
 *	exits to system control.
 *	
 */
checkdir(path)
	char	*path;
{
/*
 * +--> Local variables
 */

char	dummy[MAXPATHLEN+1];
char	dummy2[MAXPATHLEN+1];
int	k;

/*------*\
   Code
\*------*/

/* Remove any trailing slash from directory name, else
 * the mkdir call will fail.  dummy is used to preserve the
 * orginal form of path given to us.
 */
strcpy(dummy2,path);
j = 0;
for (cp = dummy2; *cp; cp++)
    if (*cp == '/')
	j++;
i = strlen(dummy2);
if (dummy2[i-1] == '/')
    dummy2[i-1] = 0;
for (i = 1; i <= j; i++) {
    strcpy(dummy, dummy2);
    cp = dummy;
    for (k = 1; *cp && k <= i; cp++)
	if (*cp == '/')
	    k++; 
    if (*(--cp) == '/')
	*cp = 0;

    if (stat(dummy, &Inode) < 0) {
	register int pid, rp;

	if (!(pid = fork())) {
		execl("/bin/mkdir", "mkdir", dummy, 0);
		execl("/usr/bin/mkdir", "mkdir", dummy, 0);

		/* If the execl's ever return to here, ERROR */
		PERROR "\n%s: %s %s  %c\n", Progname, CANTMKD, dummy,BELL);
		perror(Progname);
		exit(FAIL);
	}/*E if pid = fork ..*/

	while ((rp = wait(&i)) >= 0 && rp != pid)
	    	;
	return(TRUE);

    }/*E if stat dummy &Inode ..*/
}/*E for */
return(FALSE);

}/*E checkdir() */
/**/
/*
 *
 * Function:
 *
 *	fufcnv
 *
 * Function Description:
 *
 *	Converts a Fortran Unformatted File (from input volume)
 *	to the correct form for an Ultrix disk file.
 *
 * Arguments:
 *
 *	int	count;
 *	FILE	*fp;
 *	char	*p;
 *
 *
 * Return values:
 *
 *
 * Side Effects:
 *
 *	
 */

fufcnv(name, p, count, fp)
	char	*name;		/* filename used for error */
	char	*p;
	int	count;
	FILE	*fp;
{
/*
 * +--> Local Variables
 */
int	bincnt = 0;

/*------*\
   Code
\*------*/

i = *p++;
j = *p++;

if (j != 0)
	PERROR "\n%s: %s %d%c\n", Progname, SCNDCB, j, BELL);

count -= 2;

switch(i) {
	case ALL:
		bincnt = count;
		if (write(fileno(fp), &bincnt, sizeof(bincnt)) != sizeof(bincnt)) {
		    PERROR "\n:%s %s %s\n", Progname, ERRWRF, name);
		    exit(FAIL);
		}
		if (write(fileno(fp), p, bincnt) != bincnt) {
		    PERROR "\n:%s %s %s\n", Progname, ERRWRF, name);
		    exit(FAIL);
		}
		if (write(fileno(fp), &bincnt, sizeof(bincnt)) != sizeof(bincnt)) {
		    PERROR "\n:%s %s %s\n", Progname, ERRWRF, name);
		    exit(FAIL);
		}
		break;

	case FIRST:
		bincnt = 0;
		if (write(fileno(fp), Spaces, sizeof(bincnt)) != sizeof(bincnt)) {
		    PERROR "\n:%s %s %s\n", Progname, ERRWRF, name);
		    exit(FAIL);
		}
		if (write(fileno(fp), p, count) != count) {
		    PERROR "\n:%s %s %s\n", Progname, ERRWRF, name);
		    exit(FAIL);
		}
		bincnt += count;
		break;

	case MIDDLE:
		if (write(fileno(fp), p, count) != count) {
		    PERROR "\n:%s %s %s\n", Progname, ERRWRF, name);
		    exit(FAIL);
		}
		bincnt += count;
		break;

	case LAST:
		if (write(fileno(fp), p, count) != count) {
		    PERROR "\n:%s %s %s\n", Progname, ERRWRF, name);
		    exit(FAIL);
		}
		bincnt += count;
		fseek(fp, (long)(-1 * (bincnt+sizeof(bincnt))), 1);
		if (write(fileno(fp), &bincnt, sizeof(bincnt)) != sizeof(bincnt)) {
		    PERROR "\n:%s %s %s\n", Progname, ERRWRF, name);
		    exit(FAIL);
		}
		fseek(fp, (long)bincnt, 1);
		if (write(fileno(fp), &bincnt, sizeof(bincnt)) != sizeof(bincnt)) {
		    PERROR "\n:%s %s %s\n", Progname, ERRWRF, name);
		    exit(FAIL);
		}
		break;

	default:
		PERROR "\n%s: %s %d%c\n", Progname, FSTCB, i, BELL);
		break;
	}
return(1);

}/*E fufcnv() */
/**/
/*
 *
 * Function:
 *
 *	getlen
 *
 * Function Description:
 *
 *	Get and return the length of the next variable-length
 *	record from the input volume.
 *
 * Arguments:
 *
 *	char	*s	Pointer to the string containing ?_?
 *
 * Return values:
 *
 *	The integer length of the record is returned.
 *
 *	A zero length is a valid return value. It indicates
 *	an empty record.
 *
 *	If *s points to the padding character (^),
 *	-1 is returned to signify the end of available data
 *	in the current block.
 *
 *
 * Side Effects:
 *
 *	none	
 */

getlen(s)
	char	*s;
{

/*------*\
   Code
\*------*/

if (*s == PAD)
	return(-1);	/* Record padding character seen, signal
			 * end of data to the caller.
			 */
/* Increment pointer if record is segmented
*/
if (L_recformat == SEGMENT)
    s++;

/* Default is an empty record
 */
j = 0;

for (i=0; i < 4; i++)
	j = 10 * j + (*s++ - '0');
/* If VARIABLE, -4.  If SEGMENT, -5
*/
if (L_recformat == VARIABLE)
    return(j-4);
return(j-5);

}/*E getlen() */
/**/
/*
 *
 * Function:
 *
 *	xtractf
 *
 * Function Description:
 *
 *	Extract a given file from the input Volume.
 *
 * Arguments:
 *
 *	char	*path;		 Pathname_+_filename from input vol
 *	char	*s;		 User entered path/file name
 *	long	charcnt;	 Character count (if applicable) 
 *	char	*name;		 Alternate name given to disk file if
 *				 different from that on the input
 *				 volume (user driven). Used when -W
 *				 switch is given and the input volume
 *				 file is found to exist on disk. 
 *
 * Return values:
 *
 *	1. The character count of the extracted file.
 *	2. -or-   -1  if an error was encountered that can
 *		  be recovered from without undue hardship.
 *
 *		 (xtractf must return -1 upon error because a
 *		  zero-length file can have a valid returned
 *		  length of 0 bytes)
 *
 * Side Effects:
 *
 *	If the function cannot write the file on the output
 *	disk for whatever ... An error message is output to
 *	stderr and the function exits to system control.
 *
 *	If the user has requested that warnings be issued when
 *	a file being extracted is already on disk (-W qualifier)
 *	the logic will pause and give the user the option(s) of:
 *
 *		1. Cancel the extraction of the file from 
 *		   the input volume.
 *
 *		2. Extract the file and overwrite the existing
 *		   copy on the output medium.
 *
 *		3. Extract the file, but give it a new name as
 *		   supplied by the user.
 */
 
long xtractf(path, s, charcnt, name)
	char	*path;		/* pathname_+_filename from input vol */
	char	*s;		/* user entered path/file name */
	long	charcnt;	/* character count (if applicable) */
	char	*name;		/* alternate extracted filename
				 * (if different from user entered
				 *  name) */
{
/*
 * +--> Local Variables
 */

int	count;
int	done = FALSE;		/* Boolean to make sure new file names
				 * get "stat" -ed too */
char	dummy2[MAXPATHLEN+1];	/* Temporary variable */
char	Inbuf[MAXBLKSIZE+1];	/* File writing buffer */
int	nbytes;
long	num = 0L;
char	*p;
char	*q;
FILE	*xfp;

/*------*\
   Code
\*------*/

/* If user didn't specify path/file names, use the
 * name from the input volume as default.
 */
if (!*s) {
	strcpy(dummy2,path);
#if 0
	sprintf(dummy2, "%s%s", path, L_filename);
	term_string(dummy2,DELNL,TRUNCATE);
#endif
}
else
	strcpy(dummy2, s);

if (Wildc) {
	/*
	 * User entered a wild card name string that matched
	 * the current file on the input volume.
	 * Use the actual name found on the
	 * input volume.
	 */
#if 0
	term_string(path,DELNL,TRUNCATE);
#endif
	strcpy(name,path);
	strcpy(dummy2,path);
}
if (!strcmp(Tftypes,"dir")) {
    Dircre = checkdir(dummy2);
    fsf(1);
    return((long)(0));
}/*T if (!strcmp(Tftypes,"dir")) */ 

while (stat(dummy2, &Inode) >= 0 && !done) {
    if (Warning) {
	PROMPT "\n%s: %s %s\n", Progname, EXISTS, dummy2);
	PROMPT "%s: %s %c", Progname, OVRWRT, BELL);

	gets(Labelbuf);
	PROMPT "\n");

	Labelbuf[0] = Labelbuf[0] == 'Y' ? 'y' : Labelbuf[0];

	if (Labelbuf[0] != 'y') {
		/*
		 * User does not want to overwrite existing file.
		 */
		char	dummy[MAXPATHLEN+1];

		PROMPT "%s: %s ", Progname, ALTERN);
		gets(dummy);
		PERROR "\n");
		if (!dummy[0]) {
			/*
			 * User wants to skip extract of this file.
			 */
			fsf(2);
			return((long)(-1));
		}/*T if !dummy[0] */
		else {
			if (strlen(dummy) > MAXPATHLEN) {
				PERROR "\n%s: %s %s%c", Progname,
				    FNTL, dummy, BELL);
				fsf(2);
				return((long)(-1));

			}/*T if strlen dummy > MAXPATHLEN */
			else {
#if 0
				term_string(dummy,DELNL,TRUNCATE);
#endif
				strcpy(dummy2, dummy);
				strcpy(name, dummy2);
				Wildc = FALSE;

			}/*F if strlen dummy > MAXPATHLEN */
		}/*F if !dummy[0] */
	}/*T if Labelbuf[0] != 'y') */
	else
	    done = TRUE;
    }/*E if Warning */
    else
	done = TRUE;
}/*while stat dummy2 ..*/


/* File being extracted ...
 */
if (!(xfp = fopen(dummy2, "w")))
    if ((Dircre = checkdir(dummy2)) == FALSE) {
	PERROR "\n%s: %s %s %c\n", Progname,
	  CANTCRE, dummy2, BELL);
	perror(Progname);
	fsf(2);
	return((long)(-1));
    }
    else {
	if (!(xfp = fopen(dummy2, "w"))) {
	    PERROR "\n%s: %s %s %c\n", Progname, CANTCRE, dummy2, BELL);
	    perror(Progname);
	    fsf(2);
	    return((long)(-1));
	}/*if !xfp */
    }
while ((nbytes = read(fileno(Magtfp), p=Inbuf, L_blklen)) > 0 ) {
	if (!Tape) {
		if (tape_mark(Inbuf))
			goto eofdata;
	}/*E if !Tape */
	
	if (L_recformat == FIXED || L_recformat == UFORMAT || (L_recformat != VARIABLE && L_recformat != SEGMENT)) {
		/*
		 * Test if charcnt == 0L
		 * foreign tapes won't have a character count !
		 */
		if (charcnt == 0L) {

			cp = p;
			cp += nbytes;

			while (*--cp == PAD)
			    ;
			cp++;
			num += (long)(cp - p);

			if (write(fileno(xfp), p, cp - p) != (cp - p)) {
			    PERROR "\n%s: %s %s\n", Progname, ERRWRF, dummy2);
			    exit(FAIL);
			}/*E if write < 0 */

		}/*T if charcnt == 0L */

		else if (charcnt >= (long)nbytes) {
			if (write(fileno(xfp), Inbuf, nbytes) != nbytes) {
			    PERROR "\n%s: %s %s\n", Progname, ERRWRF, dummy2);
			    exit(FAIL);

			}/* if write < 0 */

			charcnt -= (long)nbytes;
			num += (long)nbytes;

		}/*T if charcnt >= nbytes */

		else if (charcnt > 0L) {
		        if (write(fileno(xfp), Inbuf,(int)charcnt) != (int)charcnt){
			    PERROR "\n%s: %s %s\n", Progname, ERRWRF, dummy2);
			    exit(FAIL);

			}/* if write < 0 */

			num += charcnt;

			/* Set charcnt equal to -1 */

			charcnt = (long)(-1);
	
		}/* if charcnt > 0L */
	
	}/*T if (L_recformat == FIXED) */
/**/
/*
 *	Volume file was not FIXED LENGTH  record  format.
 */
	else {
	    j = 0;
	    while (p < &Inbuf[nbytes] && (count = getlen(p)) >= 0) {

		if (L_recformat == VARIABLE) {
			p += 4;

			if (!count) {
				putc('\n', xfp);
				fflush(xfp);
				num++;
				continue;

			}/*E if !count */
		}/*T if L_recformat == VARIABLE */

		else if (L_recformat == FUF) {
			p += 4;

			if (! fufcnv(dummy2, p, count, xfp)) {
			    PERROR "\n%s: %s\n", Progname, FUFTL);
			    PERROR "%s: %s -> %s%c\n", Progname,
				NOTEX, dummy2, BELL);
			    fsf(2);
			    return((long)(-1));

			}/*E if ! fufcnv ..*/

			p += count;
			continue;

		}/*T if L_recformat == FUF */
		else
		    if (L_recformat == SEGMENT) {
			q = p;
			p += 5;
			if (write(fileno(xfp), p, count) != count) {
			    PERROR "\n%s: %s %s\n", Progname, ERRWRF, dummy2);
			    exit(FAIL);
			}/*E if write < 0 */
			if (*q == '0' || *q == '3') {
			    putc('\n', xfp);
			    fflush(xfp);
			    num++;
			}/*T if (*q == '0' || *q == '3') */ 
			p += count;
			num += (long)count;
			continue;
		    }/*T if L_recformat == SEGMENT */
		else /* L_recformat == DD */
			count += 4;

		if (write(fileno(xfp), p, count) != count) {
		    PERROR "\n%s: %s %s\n", Progname, ERRWRF, dummy2);
		    exit(FAIL);
		}/*E if write < 0 */

		p += count;
		num += (long)count;

/* ?_?
 * ?_?	Checking for VARIABLE type   AGAIN ???    ?_?
 */
		if (L_recformat == VARIABLE) {
			putc('\n', xfp);
			fflush(xfp);
			num++;

		}/*E if L_recformat == VARIABLE */
	    }/*E while p < &Inbuf[nbytes]  .. */
	}/*F if (L_recformat == FIXED) */
}/*E while ((nbytes = read(fileno(Magtfp), p=Inbuf, L_blklen))) */
if (nbytes < 0) {
    PERROR "\n%s: %s %s\n", Progname, CANTRD, Magtdev);
    ceot();
}
/*
*/
eofdata:

fclose(xfp);

return(num);

}/*E xtractf() */

/**\\**\\**\\**\\**\\**  EOM  xtractf.c  **\\**\\**\\**\\**\\*/
/**\\**\\**\\**\\**\\**  EOM  xtractf.c  **\\**\\**\\**\\**\\*/
