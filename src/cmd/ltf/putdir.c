
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#ifndef lint
static	char	*sccsid = "@(#)putdir.c	3.0	(ULTRIX)	4/21/86";
#endif	lint

/**/
/*
 *
 *	File name:
 *
 *		putdir.c
 *
 *	Source file description:
 *
 *		This file contains the logic to put 
 *		individual directory files on an output
 *		volume for the Labeled Tape Facility (LTF).
 *		
 *
 *	Functions:
 *
 *		fdlist()	Returns directory list to memory
 *
 *		dprocess()	Outputs a directory file to the volume.
 *
 *		putdir()	Examines the current long path
 *				name for directory entries and
 *				outputs those which have not yet
 *				been placed on the output volume.
 *
 *	Usage:
 *
 *		n/a
 *
 *	Compile:
 *
 *	    cc -O -c putdir.c		<- For Ultrix-32/32m
 *
 *	    cc CFLAGS=-DU11-O putdir.c	<- For Ultrix-11
 *
 *
 *	Modification history:
 *	~~~~~~~~~~~~~~~~~~~~
 *
 *	revision			comments
 *	--------	-----------------------------------------------
 *	  01.0		3-Jun-85	Ray Glaser
 *			Create original version.	
 *
 *	  01.1		10-SEP-85	Suzanne Logcher
 *			Add logic to check current century 
 *
 *	  01.2		24-SEP-85	Suzanne Logcher
 *			Correct the representation of directories and
 *			subdirectories when creating a volume
 */

/*
 * +--> Local Includes
 */

#include	"ltfdefs.h"
/**/
/*
 *
 * Function:
 *
 *	fdlist
 *
 * Function Description:
 *
 *	Returns directory list to memory
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

/* Return malloc'd directory list to freemem
 */
for (lDhead = Dhead; lDhead;) {
	dlinkp = lDhead->dir_next;	
	free ((char *)lDhead);
	lDhead = dlinkp;
}
Dhead = 0;	/* The list is now empty */
}/*E fdlist() */
/**/
/*
 *
 * Function:
 *
 *	dprocess
 *
 * Function Description:
 *
 *	This functions writes the ANSI file header labels
 *	and ANSI file trailer labels to the output volume
 *	for a directory file.
 *
 * Arguments:
 *
 *	char	*longname;	Contains the complete directory name
 *	char	*shortname;	Current/last sub-directory name
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

dprocess(longname, shortname)
	char	*longname;
	char	*shortname;
{
/*
 * +--> Local Variables
 */
char	ansift;		/* Saves ANSI file type character from HDR2
			 * for later verbose messge output.
			 */
char	crecent;	/* Creation date century */
char	*ctime();	/* Declare ctime routine */
char	interchange[18];	/* Interchange name */
int	length;
struct	tm *localtime();
struct	tm *ltime;
int	version = 1;		/* version number (1 is default) */


/*------*\
   Code
\*------*/


if ((strlen(longname)) >= MAXPATHLEN) {
	PERROR "\n%s: %s %s%c\n\n",
		Progname, FNTL, longname, BELL);
	/*
	 * A string that long is fatal...
	 */
	exit(FAIL);
}
/*
 * Make the ANSI interchange file name
 */ 
i = 0;
strcpy(Dummy,shortname);
if (!(filter_to_a(Dummy,REPORT_ERRORS))) {
	if (Warning) {
		PERROR "\n%s: %s %s", Progname, NONAFN, shortname);
		PERROR "\n%s: %s %s", Progname, INVVID2, Dummy);
		i++;
	}
}

if (strlen(Dummy) > 17) {
	/*
	 * If user has requested warnings, tell user file name
	 * is too long for HDR1 and that it will be truncated.
	 */
	Dummy[17] = '\0';
	if (Warning) {
	    PROMPT "\n%s: %s %s%c", Progname, FNTL, shortname, BELL);
	    i++;
	}
}
if ((i > 0) && Warning)
    PERROR "\n%s: %s\n\n", Progname, FILENNG);

strcpy(interchange, Dummy);
strcpy(Tftypes,"dir");
Inode.st_size = 0L;

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
if (cp[20] == '2' && cp[21] == '0')
    crecent = '0';
else
    crecent = ' ';

ltime = localtime(&Inode.st_mtime);
Blocks = 0L;

/****\
 *	Write  HDR1  on volume..
 ****/

sprintf(Dummy,
	"HDR1%-17.17s%-6.6s%04d%04d%04d%02d%c%02d%03d %02d%03d %06d%-13.13s%7.7s",
	interchange, Volid, Fsecno, Fseqno, (version-1) / 100 + 1, 
	(version-1) % 100, crecent, ltime->tm_year, ltime->tm_yday+1, 
	99, 366, 0, IMPID, Spaces);

if (Ansiv != '4') 
	filter_to_a(Dummy,IGNORE_ERRORS);

if (write(fileno(Magtfp), Dummy, BUFSIZE) != BUFSIZE) {
    PERROR "\n%s: %s %s\n", Progname, ERRWRF, longname);
    perror(Magtdev);
    ceot();
} 


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

/****\
 *	Write  HDR2  on volume..
 ****/

sprintf(Dummy,
    "HDR2%c%05d%05d%06o%04d%04d%04d%3.3s%c%010ld%1.1d%1.1d 00%28.28s",
	FIXED, Blocksize, 0,
	Inode.st_mode & 0177777,
	Inode.st_uid, Inode.st_gid,
	0, Tftypes,
	Carriage, Inode.st_size, Lhdrl, Leofl, Spaces);
/*
 * Save ANSI file type for verbose messgae use.
 */
ansift = Dummy[4];

if (write(fileno(Magtfp), Dummy, BUFSIZE) != BUFSIZE) {
    PERROR "\n%s: %s %s\n", Progname, ERRWRF, longname);
    perror(Magtdev);
    ceot();
} 


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

/****\
 *	Write an end-of-file mark on tape that separates 
 *	the file header labels from the data ....
 ****/

weof();

/****\
 *	An empty file is written for a directory.
 *	(write an end-of-file mark on tape)
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
	IMPID, Spaces);

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
    "EOF2%c%05d%05d%06o%04d%04d%04d%3.3s%c%010ld%1.1d%1.1d 00%28.28s",
	FIXED, Blocksize, 0,
	Inode.st_mode & 0177777,
	Inode.st_uid, Inode.st_gid,
	0, Tftypes,
	Carriage, Inode.st_size, Lhdrl, Leofl, Spaces);

if (write(fileno(Magtfp), Dummy, BUFSIZE) != BUFSIZE) {
    PERROR "\n%s: %s %s\n", Progname, ERRWRF, longname);
    perror(Magtdev);
    ceot();
} 


/****\
 *	Write  EOF3 - EOFn  to output volume.
 *
 *	EOF3 thru EOF'n'  contain the remaining characters of a
 *	very long path/file name. If no characters need be stored
 *	in the EOF labels for a path/file name, at a minimum we
 *	output the same number of trailer lables as header labels
 *	in order to keep things tidy.
 ****/
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
}
/* 
 * If Leofl = 0, no EOF lables are required to hold path/file
 * name characters. Therefore, we output as many space filled EOFx
 * lables as we need to in order to have the same number of EOF
 * trailer labesl as HDR header labels to keep ANSI happy.
 *
 * If some number of EOF labels were required for path/file characters,
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
}

/*****\
 *	Write tape mark to separate us from next label group
 *	if we are going to output more files.
 *****/

weof();

if (Dverbose) {
	if (Verbose) {

		printf("c<%s>%c  %s\n",Tftypes, ansift, longname);

	}/*T if Verbose */
	else
		printf("c  %s\n", longname);

}/*E if Dverbose */

/*	Increment the ANSI file sequence number for the next
 *	file to go out (if any).
 */
Fseqno++;

}/*E dprocess() */
/**/
/*
 *
 * Function:
 *
 *	putdir
 *
 * Function Description:
 *
 *	This function determines if there are directory components
 *	in the given path name that have not yet been placed on
 *	the output volume and places them in the list of directories
 *	output and then calls "dprocess" to write a directory file
 *	entry on the output volume.
 *
 * Arguments:
 *
 *	char	*longname	Full pathname_+_filename
 *	char	*shortname	File name only
 *
 * Return values:
 *
 *	none
 *
 * Side Effects:
 *
 *	none
 */

putdir(longname, shortname, workdir)
	char	*longname;	/* Pathname_+_filename */
	char	*shortname;	/* File name only */
	char	*workdir;	/* Working directory */
{
/*
 * +--> Local Variables
 */

int	d;
char	dir[MAXNAMLEN+1];
int	i, j, k;
struct	stat inode;
char	path[MAXPATHLEN+1];
int	relpath;
char	relpaths[MAXPATHLEN+1];
int	ret;
int	ret1;
int	s;
struct	stat tnode;

/*------*\
   Code
\*------*/

if (!strcmp(longname, ".") || !strcmp(longname, "..") || 
    !strcmp(shortname, "/"))
    return(TRUE);

relpaths[0] = '\0';
if (longname[0] == '/')
	relpath = FALSE;
else {
	relpath = TRUE;
	strcpy(relpaths, workdir);
	strcat(relpaths,"/");
}/*F longname[0] = '/' */
i = strlen(longname);
path[0] = '\0';
dir[0] = '\0';

for (s = 0, j = 0, d = 0; j < i; ) {

	/* Add to the current full directory tree name.
	 */
	path[j] = longname[j];

	/* Add to the current sub-directory name used for
	 * the interchange file name by dprocess().
	 */
	if (longname[s] == '/')
		/*
		 * Relative subdirectories do not begin their
		 * names with a slash.
		 * This "if" is needed in the event a relative
		 * name without a leading slash is given.
		 * An absolute path name does begin with a / ,
		 * but the following name string is really a
		 * relative name to the root directory (/) and
		 * so we always want to strip the leading slash
		 * so that the HDR1 name output by dprocess
		 * is always  "name/" .. The correct/complete
		 * path (absolute or relative) is contained
		 * in our path string and goes into HDR3, etc.
		 */
		dir[d++] = longname[j+1];
	else 
		dir[d++] = longname[j];
	j++;

	if (longname[j] == '/' || longname[j] == '\n' ||
	    !longname[j] || d >= MAXNAMLEN) {

		dir[d] = path[j] = 0;
		s = j;

		ret1 = 0;
		if (relpath) {
			strcat(relpaths, dir);
			k = strlen(relpaths);
			relpaths[k] = '\0';
			ret = stat(relpaths, &Inode);
			ret1 = lstat(relpaths, &inode);
			if (ret < 0 || ret1 < 0) {
			    if (inode.st_mode & S_IFMT == S_IFLNK && ret < 0)
			        PERROR "\n%s: %s %s%c\n", Progname, CANTSTS, relpaths, BELL);
			    else
			        PERROR "\n%s: %s %s%c\n", Progname, BADST, relpaths, BELL);
			    system("pwd");
			    perror(Progname);
			    return(EOF);
			}
			strcat(relpaths,"/");
		}
		else {
			ret = stat(path, &Inode);
			ret1 = lstat(path, &inode);
			if (ret < 0 || ret1 < 0) {
			    if ((inode.st_mode & S_IFMT) == S_IFLNK && ret < 0)
			        PERROR "\n%s: %s %s%c\n", Progname, CANTSTS, path, BELL);
			    else
			        PERROR "\n%s: %s %s%c\n", Progname, BADST, path, BELL);
			    system("pwd");
			    perror(Progname);
			    return(EOF);
			}
		}
		if ((Inode.st_mode & S_IFMT) == S_IFDIR) {

			int	found = 0;
			struct	DIRE *dp;

			if ((inode.st_mode & S_IFMT) == S_IFLNK) {
			    tnode = Inode;
			    Inode = inode;
			    if (!Nosym)
				return(TRUE);
			}
			if (dir[d-1] != '/')
				/*
				 * Sub-directory names are always
				 * terminated by a / character.
				 */
				strcat(dir,"/");

			/* If this is a partial tree name, ensure
			 * that we include the trailing / .
			 */
			if (longname[j] == '/') {
				path[j] = longname[j++];
				path[j] = 0;
				s = j;
			}
			else
				/* It is the end of the tree name &
				 * we have to add a slash for
				 * ourselves.
				 */
				strcat(path,"/");
			for (dp = Dhead; dp; dp = dp->dir_next)
				if (dp->rdev == Inode.st_rdev &&
				    dp->inode == Inode.st_ino) {
					found = TRUE;
					break;
				}
			if (!found && !NMEM4D) {
    			    /* Get some memory for our next
     			     * directory list entry.
     			     */
			    dp=(struct DIRE *) malloc(sizeof(*dp));
			    if (!dp) {
				/* If no mem, return what we
 	 		 	 * have & try again ?
 	 			 */
				fdlist();
			        dp=(struct DIRE *) malloc(sizeof(*dp));
			        if (!dp) {
				    PERROR "\n%s: %s%c\n", Progname, NOMDIR, BELL);
				    NMEM4D++;
	    		        }/*T if !dp */
				else {
				    dp->dir_next = Dhead;
				    Dhead = dp; /*Lists run backwards*/
				    dp->rdev = Inode.st_rdev;
				    dp->inode = Inode.st_ino;
				}/*F if !dp */
			    }/*T if !dp */
			    else {
				dp->dir_next = Dhead;
				Dhead = dp; /*Lists run backwards*/
				dp->rdev = Inode.st_rdev;
				dp->inode = Inode.st_ino;
			    }/*F if !dp */
			    /*
			     * Go put a new directory entry on
			     * the output volume if Nodir switch
			     * is FALSE.
			     */
			    if (!Nodir)
				dprocess(path, dir);
			}/*T if (!found && !NMEM4D) */
			d = 0;
			if ((inode.st_mode & S_IFMT) == S_IFLNK)
			    Inode = tnode;
		}/*T if ((Inode.st_mode &S_IFMT) == S_IFDIR) */
		else {
			/* If something other than a directory was
			 * detected in the given path, exit back
			 * to caller for ordinary file processing.
			 */
			if (!Nosym)
			    Inode = inode;
			return(TRUE);
		}
	}/*T if (longname[j] == '/' || ... */
}/*E for (s=0, j=0, d=0; j<i; ) */
for (cp = longname, i = 0; *cp; cp++)
    if (*cp == '/')
	i++; 
if (!relpath) {
    for (cp = workdir, j = 0; *cp; cp++)
	if (*cp == '/')
	    j++; 
    numdir = i - j;
}
else
    numdir = i + 1; 
if (!Nosym)
    Inode = inode;
    
}/*E putdir() */

/**\\**\\**\\**\\**\\**  EOM  putdir.c  **\\**\\**\\**\\**\\*/
/**\\**\\**\\**\\**\\**  EOM  putdir.c  **\\**\\**\\**\\**\\*/
