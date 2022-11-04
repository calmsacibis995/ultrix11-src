
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 *  ULTRIX-11 help program.
 *
 *  Locates help in the help data base by searching a
 *  a table of command names, lseek pointers, and number
 *  of lines to print.  You can still use "sccs help" or,
 *  "sccshelp" for help with sccs command error numbers.
 *
 *  John Dustin
 *  Steve Reilly
 *  5/6/85
 */
static char Sccsid[] = "@(#)help.c	3.0	4/21/86";

#include <stdio.h>

#define LINELEN		132		/* length of an input line */
#define MAXCMDLEN	200
#define OK		  1
#define HELPPATH	"/usr/lib/help/U11_help"  /* help data base */
#define HDHELPPATH	"/usr/lib/help/help_list"  /* pointers to data base */

char	line[LINELEN];		/* input line */
char	*cmd;			/* command we are finding */
FILE 	*helpfp;		/* help file data base pointer */
FILE	*headfp;		/* "lookup table" file pointer */

main(argc, argv)
int argc;
char *argv[];
{
	int i = 1;

	/*
	 *  If a help argument is not specified
	 *  then default to welcome.
	 */

	if (argc < 2)
	    cmd = "welcome";
	else
	    cmd = argv[1];

	/*
	 *  Open the header file (lookup table) that contains
	 *  the seek pointers to the master help file.
	 */
	if ( ( helpfp = fopen(HELPPATH,"r") ) == NULL ) {
	    sprintf(line, "Cannot open %s",HELPPATH);
	    perror(line); 
	    exit(1);
	}

	/*
	 *  Open the help file data base
	 */
	if ( ( headfp = fopen(HDHELPPATH,"r") ) == NULL ) {
	    sprintf(line, "Cannot open %s",HDHELPPATH);
	    perror(line);
	    exit(1);
	}

	do {
	    /*
	     *  Rewind the lookup table
	     */
	    rewind(headfp);

	    if (findcmd(cmd) != OK) {
	        /*
	         *  command not found, just continue if other args
	         */
		 printf("Sorry, there is no help available for '%s'\n",cmd);
	    }

	    cmd = argv[++i];

	} while (i < argc);

	/*
	 *  Flush the buffer before exiting
	 */
	fflush(stdout);
	exit(0);
}

/*
 * locate the help text, if not found, return NULL
 */
findcmd(cmd)
char *cmd;	/* pointer to the command the user wants */
{
	int x;		/* index variable */
	struct {
	    char command[MAXCMDLEN];	/* command string */
	    long seek_addr;	/* seek address of the command */
	    int  num_lines;	/* number of lines associated with the cmd */
	} help_list;
	
	/*
	 *  Continue reading the table file until we reach the end
	 */
	while ( fscanf(headfp, "%s%ld%d",help_list.command,
		&help_list.seek_addr, &help_list.num_lines ) != EOF ) {
	
    	    /*
	     *  Did we find a match ?
	     */
	    if ( !strcmp(help_list.command, cmd) ) {
		
		/*
		 *  We have a match so seek to start of text
		 */
		if (fseek( helpfp, help_list.seek_addr, 0) < 0) {
		    sprintf(line, "%s seek error", HELPPATH);
		    perror(line);
		    return(NULL);
		}

		/*
		 *  Print out the text associated with the command
		 */
		if ( fgets( line, LINELEN, helpfp ) == NULL ) {
		    sprintf(line, "%s read error", HELPPATH);
		    perror(line);
		    return(NULL);
		}

		/*
		 *  If command is not "welcome", print the
		 *  command name followed by colon and <CR>.
		 */
		if (strcmp(help_list.command, "welcome")) {
		    fprintf(stdout, "%s:\n",help_list.command);
		}
	
		/*
		 *  Read and print the rest of the help text
		 *  associated with the command
		 */
		for (x = help_list.num_lines - 1; x; --x) {

		    /*
		     *  Check to make sure that we can actually read
		     *  the text
		     */		
		    if ( fgets( line, LINELEN, helpfp ) == NULL ) {
			sprintf(line,"%s read error", HELPPATH);
		        perror(line);
		        return(NULL);
		    }
		
		    /*
		     *  Print the text
		     */
		    fputs(line, stdout);
		}
		return(OK);
	    }
	}
	return(NULL);
}
