
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#ifndef lint
static	char	*sccsid = "@(#)filenames.c	3.0	(ULTRIX)	4/21/86";
#endif	lint

/**/
/*
 *
 *	File name:
 *
 *		filenames.c
 *
 *	Source file description:
 *
 *		This module contains the various routines that
 *		deal with file name and character string handling.
 *		Define "handling" to imply string conversions,
 *		string filtration, etc..
 *		
 *		Generally, but not always, this will be done
 *		on behalf of filenames. One exception is for
 *		volume label name strings. Other cases may be
 *		added as development proceeds.
 *
 *	Functions:
 *
 *		filter_to_a()	Filter a character string to contain
 *				only "a"characters.
 *				(see ltfdefs.h -or- filter_to_a() )
 *
 *		Lookup()	Looks up a given ANSI file name
 *				among user-input file arguments.
 *
 *		make_name()	Convert a user file name to correct
 *				format for FILESTAT structure
 *
 *		rec_args()	Records file name command line arguments
 *				in FILESTAT linked list.
 *
 *		rec_file()	Records file name arguments in FILESTAT
 *				list entered from an input file,
 *				or line-by-line from stdin.
 *				
 *		term_string()	Terminate a string in the requested
 *				fashion
 *
 *	Usage:
 *
 *		n/a
 *
 *
 *	Compile:
 *
 *	    cc -O -c filenames.c	 <- For Ultrix-32/32m
 *
 *	    cc CFLAGS=-DU11-O filenames.c <- For Ultrix-11
 *
 *
 *	Modification history:
 *	~~~~~~~~~~~~~~~~~~~~
 *
 *	revision			comments
 *	--------	-----------------------------------------------
 *	  01.0		14-April-85	Ray Glaser
 *			Create original version.
 *
 *	  01.1		22-August-85	Suzanne Logcher
 *			Debug and fix rec_file.
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
 *	filter_to_a	
 *
 * Function Description:
 *
 * 	Function "filter_to_a" is called to filter a
 *	given string to "a"chracters as defined below.
 *
 *
 * Arguments:
 *
 *	char	*string		Pointer to "null-terminated"
 *				character string to be filtered
 *
 *	int	report_errors	BOOLEAN flag (TRUE/FALSE) to
 *				indicate if the caller wants
 *				to know if any non-a chctrs
 *				were found.
 *
 *				TRUE implies that the caller
 *				wants to be notified of the
 *				occurance of any non-"a"character
 *				seen in the given string..
 *
 *				FALSE implies the caller does
 *				not care if non-"a"characters were
 *				seen but desires the conversion
 *				to be made regardless.
 *
 * Return values:
 *
 *	Return values are the BOOLEAN flags TRUE or FALSE.
 *
 * Side Effects:
 *
 *	If the caller indicates that errors are to be
 *	returned, invalid non-"a"characters seen in
 *	the given string will be converted to upper case Z
 *	for subsequent user level error messages.
 *
 *	Normally, all lower case characters will be converted
 *	to upper case. Lower case letters are not allowable
 *	"a"characters. However, if the report_errors flag is
 *	set, the lower to upper case conversion is not made.
 *
 */

filter_to_a (string,report_errors)
	char	*string;
	int	report_errors;
{
/**/
/*
 *	"a"characters - Refers to the set of characters consisting of:
 *
 *		Uppercase  A-Z,  numerals  0-9, & the following 
 *		special characters:
 *
 *			space  !  "  %  &  '  (  )  *  +  ,  - _
 *			. /  :  ;  <  =  >  ?
 *
 */
/*
 * ->	Local variables
 */

short	a_flag;
short	i;
short	non_a = FALSE;	/* Default is a good string */
char	*o_string; 

/*------*\
   Code
\*------*/

o_string = string;	/* Save pointer to orginal string */

while (*string) {
	if (islower(*string))
		;
	else if (isdigit(*string))
		;
	else if (isupper(*string))
		;
	else { 
		a_flag = FALSE; /* Default to not valid chctr */	

		for (i=0; i < A_SPECIALS; i++) {
			if (*string == A_specials[i]) {
				a_flag = TRUE;
				break;
			}
		}
		if (a_flag == FALSE) {
			non_a = TRUE;
			*string = 'Z';
		}	
	}/*E if else string*/

	string++;

}/*E while *string */

/* Convert string to upper case.
 */
while (*o_string) {
    *o_string = islower(*o_string) ? *o_string-'a'+'A' : *o_string;
    o_string++;
}
/* If invalid chctrs were found, return FALSE
 */
if (non_a && report_errors)
	return(FALSE);
return(TRUE);

}/*E filter_to_a()*/
/**/
/*
 *
 * Function:
 *
 *	Lookup
 *
 * Function Description:
 *
 *	This function looks up a given ANSI volume file name
 *	entry among user-input file name arguments
 *
 * Arguments:
 *
 *	char	*name	Pointer to the name string to be found
 *
 * Return values:
 *
 *	Returns	a FILESTAT structure pointer if the name is
 *	found. Else, returns NULL if the name was not found.
 *
 *
 * Side Effects:
 *
 *	none	
 */

struct FILESTAT * Lookup(name)
	char	*name;		/* file name */
{
/*
 * +--> Local variables
 */

struct	FILESTAT *fstat;
char	*d;
char	*n;
char	*x;
char	*y;


/*------*\
   Code
\*------*/

for (fstat = F_head; fstat; fstat = fstat->f_next) {

	int	i, length;

	/* 
	 * If this file has been extracted,
	 * go to the next entry.
	 */
	if (!fstat->f_numleft)
		continue;
	/*
	 * If this is the desired file, return a pointer
	 * to its' FILESTAT structure.
	 */
	if (!mstrcmp(name, fstat->f_src))  {
		fstat->f_found++;
		return(fstat);
	}
	/* ?_?
	 * check if the volume entry is a file
	 * (recursively) under a requested directory. ?_?
	 */
	n = name;
	d = fstat->f_src;
	while (*n == *d) {
		n++;	d++;
	}
	x = n;
	y = d;
	if (*d == '.'	&& *(d+1) == 0 && (*(d-1) == '/' || *n == '/'))
	    return(fstat);
	if (*y == 0 && *x == '/' && *(x+1) == 0) {
	    fstat->f_found++;
	    return(fstat);
	}
}/*E for fstat = F_head ..*/

return(NULL);

}/*E Lookup() */
/**/
/*
 *
 * Function:
 *
 *	make_name
 *
 * Function Description:
 *
 *	Converts a file name from user input to a 
 *	file name format appropriate to the current function
 *	& inserts it into a FILESTAT structure.
 *	Case conversions are performed as required.
 *
 * Arguments:
 *
 *	char	*file
 *	char	**dest
 *
 * Return values:
 *
 *	The converted file name is stored as indicated above
 *	if successful and a non-zero value is returned,
 *	else an error message is printed to stderr  & 
 *	an exit to the system is taken.
 *
 *
 * Side Effects:
 *
 *	If an error is detected, no return is made to the caller.
 *
 */

make_name(file, dest)
	char	*file;
	char	**dest;
{

/*------*\
   Code
\*------*/

if (strlen(file) > MAXPATHLEN) {
	PERROR "\n%s: %s %s\n", Progname, FNTL, file);
	exit(FAIL);
}
#if 0
if (Func == EXTRACT || Func == TABLE)

	/* Convert destination filename to lower case.
	 */
	while(*file) {
	    *file = isupper(*file) ? *file-'A'+'a' : *file;
	    file++;
	}
#endif
*dest = (char *) malloc (strlen(file) + 1);
if (!*dest) {
	PERROR "\n%s: %s\n", Progname, TMA);
	exit(FAIL);
}
strcpy(*dest, file);
return(TRUE);

}/*E make_name() */
/**/
/*
 *
 * Function:
 *
 *	rec_args
 *
 * Function Description:
 *
 *	Records (saves) command line argument file names in the
 *	linked list "FILESTAT" structure(s) for future reference.
 *
 * Arguments:
 *
 *	int	dumpflag	Flags associated with this command line
 *	char	*longname	Path name_+_filename
 *
 * Return values:
 *
 *	none
 *
 * Side Effects:
 *
 *	If an error is dectected in a filename, a message is output
 *	to stderr and an exit to the system is taken.
 *	
 */

rec_args(longname, dumpflag)
	char	*longname;
	int	dumpflag;
{
/*
 * ->	Local variables
 */

struct	FILESTAT *fstat;

/*------*\
   Code
\*------*/

fstat = (struct FILESTAT *) malloc(sizeof(*fstat));

if (!fstat) {
	PERROR "\n%s: %s\n", Progname, TMA);
	exit(FAIL);
}
/*	Link another file name structure to our current list.
 */
fstat->f_next = F_head;

/*	Last in, first out que.
 */
F_head = fstat;

fstat->f_numleft = 1;
fstat->f_found	= 0;
fstat->f_flags = dumpflag;

/*
 * Make final version of file name.
 */
make_name(longname, &fstat->f_src);

/* Count another file desired.
 */
Numrecs++;

return;

}/*E rec_args() */
/**/
/*
 *
 * Function:
 *
 *	rec_file
 *
 * Function Description:
 *
 *	If the Func = EXTRACT or TABLE, records (saves) argument 
 *	file names in the linked list FILESTAT structure(s) which 
 *	were entered from "stdin" or from a specified file.
 *	If the Func = CREATE, saves one filename in the argument
 *	crname, and returns to the calling routine.
 *
 * Arguments:
 *
 *	FILE	*fp		Pointer to an input file, or stdin
 *	int	iflag		Input flag, -1 = stdin, 1 = file
 *	char	*crname		Will contain a filename if Func = 
 *				CREATE
 *
 * Return values:
 *
 *	EOF	- unsuccessful = 0
 *	TRUE	- successful = 1
 *
 * Side Effects:
 *
 * 	If errors are encountered, the routine will output a message
 *	to stderr and exit to system control. ie. No return to caller.
 *	
 */

rec_file(fp, iflag, crname)
	FILE	*fp;	/* Pointer to file of file names or stdin */
	int	iflag;	/* Input flag, -1 = stdin, 1 = file */
	char	*crname;/* CREATE filename */
{
/*
 * ->	Local variables
 */

int	dumpflag = 0;
struct	FILESTAT *fstat;
char	*l, line[MAXPATHLEN];

/*------*\
   Code
\*------*/

if (Func == CREATE || Func == WRITE) {

    /* Test if iflag = -1 for stdin.
     */
    if (iflag == -1) {
	/*
	 * Process a list of file names entered from "stdin".
	 */
	PROMPT "\n%s: %s ", Progname, ENFNAM);
		
	if (!(fgets(line, sizeof(line), fp)) ||	line[0] == '\n') {
	    return(EOF); 
	}

	/* Remove newline and truncate the string after
	 * the first non-white space character.
	 */
	term_string(line,DELNL,TRUNCATE);

	if (strlen(line) < MAXPATHLEN-1)
		strcpy(crname, line);
	else {
		PERROR "\n%s: %s %s", Progname, FNTL, line);
		exit(FAIL);
		return(EOF);
	}
	return(TRUE);

   }/*F if iflag */
   else {
	if (fgets((l=line), sizeof(line), fp) != NULL) {
	    Dfiletype = 0;
	    term_string(l,NULL,TRUNCATE);
	    if (strlen(l) < MAXPATHLEN-1)
		strcpy(crname,l);
	    else {
		PERROR "\n%s: %s %s", Progname, FNTL, line);
		exit(FAIL);
	    }
	    if (*crname == 0)
		return(EOF);
	    else
		return(TRUE);

	}/* if fgets l=line etc ..*/
	else
	    return(EOF);
    }/* else */
}/*E if Func == CREATE || WRITE */

/* Func should be either TABLE or EXTRACT to get here.
 */
fstat = (struct FILESTAT *) malloc(sizeof(*fstat));

if (!fstat) {
	PERROR "\n%s: %s\n", Progname, TMA);
	exit(FAIL);
}

/*
 * Link another file name arg into the list
 */
fstat->f_next = F_head;
F_head = fstat;
for (;;) {
    if (iflag == -1) {
	PROMPT "\n%s: %s ", Progname, ENFNAM);
	if (!fgets(line, sizeof(line), fp) || line[0] == '\n')	{
		F_head = fstat->f_next;
		free((char *)fstat);
		return(EOF);
	}
	term_string(line,DELNL,TRUNCATE);
	fstat->f_numleft = 1;
	make_name(line, &fstat->f_src);
    	Numrecs++;
    	fstat++;
    }/*T if iflag */
    else
	if (fgets(l=line, sizeof(line), fp) != NULL) {
	    term_string(l,DELNL,NULL);
	    make_name(l, &fstat->f_src);
	    fstat->f_numleft = 1;
	    fstat->f_flags = dumpflag;
	    Numrecs++;
	    fstat++;
        }/*T if !fgets ..*/
	else	{
		F_head = fstat->f_next;
		free((char *)fstat);
		return(TRUE);
	}
	fstat = (struct FILESTAT *) malloc(sizeof(*fstat));
	if (!fstat) {
	    PERROR "\n%s: %s\n", Progname, TMA);
	    exit(FAIL);
	}
	fstat->f_next = F_head;
	F_head = fstat;
}/*E for ;; */

}/*E rec_file() */
/**/
/*
 *
 * Function:
 *
 *	term_string
 *
 * Function Description:
 *
 *	This function terminates a given input string according
 *	to the calling parameters.
 *
 * Arguments:
 *
 *	char	*string		Pointer to input string
 *	int	delnl		True is trailing new line to be replaced
 *				with a NULL
 *	int	truncate	If true - truncate string after the 1st
 *				non-white space character
 *
 * Return values:
 *
 *	The altered in place string.
 *
 * Side Effects:
 *
 *	none
 *
 */

term_string(string,delnl,truncate)
	char	*string;
	int	delnl;
	int	truncate;
{

/*------*\
   Code
\*------*/

if (delnl) {
	i = strlen(string) -1;

	if (string[i] == '\n')
		string[i] = NULL;

}/*E if delnl */

if (truncate) {
	while (*string && !isspace(*string))
		string++;

	if (isspace(*string))
		*string = NULL;

}/*E if truncate */

}/*E term_string() */


/* expnum() routine was moved here from ltf.c */
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
 */
expnum(numstring)
	char	*numstring;
{

/*------*\
   Code
\*------*/

j = 0;
error_status = FALSE;
if (*numstring == '-') {
    j++;
    numstring++;
}
for (i = 0; isdigit(*numstring); numstring++)
	i = (10 * i) + (*numstring - '0');
switch (*numstring) {
	case '\0':
		if (!j)
		    return(i);
		else
		    return(0 - i);
	case 'b':
		return(i * 512);
	case 'k':
		return(i * 1024);
	default:
		error_status = TRUE;
		return(0);
}/*E switch *numstring */
}/*E expnum() */

/* showhelp() routine was moved here from ltf.c module */
/**/
/*
 *
 * Function:
 *
 *	showhelp()
 *
 * Function Description:
 *
 *	Prints a help message with a description of all functions,
 *	switches, and qualifiers and their definitions, then exits 
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
 *	This function never returns to the caller. 
 *	It always exits to the system.
 *	
 */

showhelp()
{
#ifndef U11
PERROR "\n%s: %s\n", Progname, USE1);
PERROR "%s: %s\n", Progname, USE2);
PERROR "%s: %s\n", Progname, HELP1);
PERROR "%s: %s\n", Progname, HELP2);
PERROR "%s: %s\n", Progname, HELP3);
PERROR "%s: %s\n", Progname, HELP4);
PERROR "%s: %s\n", Progname, HELP5);
PERROR "%s: %s\n", Progname, HELP6);
PERROR "%s: %s\n", Progname, HELP7);
PERROR "%s: %s\n", Progname, HELP9);
PERROR "%s: %s\n", Progname, HELP10);
PERROR "%s: %s\n", Progname, HELP11);
PERROR "%s: %s\n", Progname, HELP12);
PERROR "%s: %s\n", Progname, HELP13);
PERROR "%s: %s\n", Progname, HELP14);
PERROR "%s: %s\n\n", Progname, HELP30);
PERROR "%s: %s ", Progname, HELP20);
response();
PERROR "\n%s: %s\n", Progname, USE1);
PERROR "%s: %s\n", Progname, USE2);
PERROR "%s: %s\n", Progname, HELP31);
PERROR "%s: %s\n", Progname, HELP15);
PERROR "%s: %s\n", Progname, HELP16);
PERROR "%s: %s\n", Progname, HELP17);
PERROR "%s: %s\n", Progname, HELP18);
PERROR "%s: %s\n", Progname, HELP19);
PERROR "%s: %s\n", Progname, HELP21);
PERROR "%s: %s\n", Progname, HELP22);
PERROR "%s: %s\n", Progname, HELP23);
PERROR "%s: %s\n", Progname, HELP24);
PERROR "%s: %s\n", Progname, HELP25);
PERROR "%s: %s\n", Progname, HELP26);
PERROR "%s: %s\n", Progname, HELP27);
PERROR "%s: %s\n", Progname, HELP28);
PERROR "%s: %s\n", Progname, HELP29);
#else U11

PERROR "\n%s: %s\n", Progname, USE1);
PERROR "%s: %s\n\n", Progname, USE2);
PERROR "%s: %s\n\n", Progname, TRYHELP);

#endif U11

exit(FAIL);
}/*E showhelp() */

/* usage() routine was moved here from ltf.c */
/**/
/*
 *
 * Function:
 *
 *	usage()
 *
 * Function Description:
 *
 *	Prints a summary of the command line format,	
 *	allowable functions, qualifiers, etc..
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
 *	This function never returns to the caller. 
 *	It always exits to the system.
 *	
 */

usage()
{

PERROR "\n%s: %s%c", Progname, USE1, BELL);
PERROR "\n%s: %s", Progname, USE2);
PERROR "\n%s: %s\n\n", Progname, TRYHELP);
exit(FAIL);

}/*E usage() */

/**\\**\\**\\**\\**\\**  EOM  filenames.c  **\\**\\**\\**\\**\\*/
/**\\**\\**\\**\\**\\**  EOM  filenames.c  **\\**\\**\\**\\**\\*/
