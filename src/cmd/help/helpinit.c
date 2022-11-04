
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/* 
 *
 * ULTRIX-11 Help Initialization Program, helpinit.c
 *
 *  Opens a master database and produces a table containing
 *  a command name, a seek_address and the number of lines
 *  of help text for that command.  This table is then used
 *  by the "help" program to access the help text from the
 *  database.
 * 
 *  Need to run this program every time the help database
 *  (/usr/lib/help/U11_help) is changed!
 *
 *  John Dustin
 *  Steve Reilly
 *  6-MAY-85
 */
static char Sccsid[]= "@(#)helpinit.c	3.0	4/21/86";
#include <stdio.h>

#define MAXLINELEN	132		/* max length of a buffer */
char line[MAXLINELEN];			/* the read buffer */

main(argc, argv)
int  argc;
char *argv[];
{
	FILE *masterfp,		/* file pt of the master file */
	     *headerfp;		/* file pt of the header file */
	long seek_addr;		/* the seek address of the master file */
	char *tmp_line;		/* tmp pointer in the line buffer */
	int line_count;		/* no. of lines associated with the command */
	int found_first = 0;	/* have we found our first command yet */
	
	if ( argc < 2 ) {
	    printf("Usage: helpinit helpfile\n");
	    exit(1);
	}
	/* 
	 *  Open the master database.
	 */
	if ( (masterfp = fopen(argv[1],"r")) == NULL ) {
	    sprintf(line,"Cannot open %s",argv[1]);
	    perror(line);
	    exit(1);
	}

	/*
	 *  If the user specified a file then open it.  If not
	 *  specified then set the file pt to standard out.
	 */
	if ( argc > 2 ) {
	    if ( (headerfp = fopen(argv[2],"w")) == NULL ) {
		sprintf(line,"Cannot open %s",argv[2]);
		perror(line);
		exit(1);
	    }
	}
	else
	    headerfp = stdout;

	/*
	 *  Get beginning seek address of the master file
	 */
	seek_addr = (long )ftell( masterfp );

	/*
	 *  Initialize the tmp_line pointer
	 */
	tmp_line = line;

	/*
	 *  Look for "-" in the first column of the master
	 *  file, when found, print command, seek address,
	 *  and number of lines associated with the command
	 *  to standard output.
	 */
	while ( ( fgets( tmp_line, MAXLINELEN, masterfp ) ) != NULL ) 
		{
		if ( !strncmp("-",tmp_line++,1) ) 
			{
			/*
			 *  If we have found a command, line_count
			 *  is the number of lines for the previous command
			 */
			if ( found_first )
				fprintf(headerfp," %d\n",line_count);

			/*
			 *  Print the command name (without the \n)
			 */
			fprintf(headerfp,"%c", *tmp_line++ );
			while ( strncmp("\n",tmp_line,1) ) {
				fprintf(headerfp,"%c",*tmp_line++);
			}

			/*
			 *  Print seek address
			 */
			fprintf(headerfp," %ld", seek_addr);

			/*
			 *  Initialize the line_count and indicate that
			 *  we found at least our first command
			 */
			line_count = 0;
			found_first = 1;				
			}
		/*
		 *  Set tmp_line to beginning of line buffer
		 */
		tmp_line = line;

		/*
		 *  Where are we in the master file and keep
		 *  line count for possible use later
		 */
		seek_addr = ftell( masterfp );
		line_count++;
		}

		/*
		 *  Close of the structure initialization
		 */
		fprintf(headerfp," %d\n",line_count);
	exit(0);
}
