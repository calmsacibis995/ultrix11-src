
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/


static char Sccsid[] = "@(#)ulf.c	3.0	4/21/86";
/*
 *
 * ulf -- Universal Lineprinter Filter
 *	  Written by Cliff Matthews, on April 7th, 1983 at the
 *	  University of New Mexico
 *
 * Supports:  diablo and dumb (LA180, LA120) "lineprinters"
 *
 * This program uses a "line_structure" that is a linked list of arrays to
 * hold enough characters for one physical line on the lineprinter.  The
 * first array contains the characters of the line, and all subsequent arrays
 * hold overprinting characters.  Blanks are left as zero's, so that they
 * don't have to be printed, just move the printhead whatever way is easiest.
 *
 */

#include	<stdio.h>
#include	<ctype.h>
#include	<errno.h>
#include	<sgtty.h>
#include	"lp.local.h"
#include	"globals.h"

#define	CONTROL		20	/* How many tolerable control characters */
#define	RPRINT		"\0336"	/* Make diablo print backword esc 6 */
#define	FPRINT		"\0335"	/* Make diablo print forward  esc 5 */

struct	line_s {
	char	*line;
	struct	line_s *next;
};

struct	line_s *makeline();

main(argc,argv)
int argc;
char *argv[];
{

	char c;				/* last character read */
	struct line_s *linepointer;	/* chars to print later */
	int loc;			/* current printing location */

	init(argv);
	linepointer = makeline();

	while ( (c = getchar()) != EOF ) {
		switch (c) {
		case ' ':			/* Space */
			loc++;
			break;

		case '\b':			/* Backspace */
			loc--;
			break;

		case '\t':			/* Tab */
			loc += 8 - (loc % 8);
			break;

		case '\n':			/* New Line */
			dumplines( linepointer );
			loc = 0;
			break;

		case '\r':			/* Carriage Return */
			loc = 0;
			break;

		case '\f':			/* Form Feed */
			dumplines( linepointer );
			loc = 0;
			throwpage();
			break;

		default:
			c &= 0177;			/* Turn off parity */
			if ((!isprint(c)) && NC )
				tossout();		/* Not Printable */
			else
				if (( loc >= 0) && ( loc <= width - 1))
					place( c, loc++, linepointer );
				else
					loc++;			/* off page */
		}
	}
	bill(user);
}

struct line_s *
makeline()	/* malloc up some memory for a line_structure */
{
	struct line_s *s;	/* memory allocated for the new line */
	char *cp;		/* scratch char pointer */
	char *end;		/* last legal array position for while loop */

	s = (struct line_s *) malloc(sizeof(struct line_s));
	if ( s == 0 )
		outofmemory();
	else {
		s->next = NULL;
		cp = s->line = malloc(width + 1);
		if ( s->line == 0)
			outofmemory();
		else {
			end = s->line + width;
			while ( cp < end )
				*cp++ = ' ';
			*cp = 0;
		}
	}
	return (s);
}

/* tossout: used to keep count of how many control
 * characters were being printed, and would puke if
 * too many were attempted, now just returns without
 * printing.  Doesn't keep count anymore.
 */
tossout()
{
	return;
}

place(c, location, p)	/* add a new character to a line_structure */
char c;			/* character to place */
struct line_s *p;	/* structure to put character in */
int location;		/* where in the line the character should go */
{
	char *cp;

	if (*(cp = (p->line + location)) != ' ') {
		if (p->next == NULL)
			p->next = makeline();
		place(c, location, p->next);
	} else
		*cp = c;
}

dumplines(p)		/* dump the top-level line_structure */
struct line_s *p;
{
	char *cp, *end;	/* scratch, and last legit array element */

	if (p->next != NULL)
		rdumpl(p->next);
	qprint(p->line);
	printf("\n");

	linenum++;
	if (linenum == length) {
		npages++;
		linenum = 0;
	}

	cp = p->line;
	end = cp + width;
	while ( cp < end )
		*cp++ = ' ';

	p->next = NULL;
}


rdumpl(p)		/* recursively dump the overprints */
struct line_s *p;
{
	if (p->next != NULL)
		rdumpl(p->next);
	qprint(p->line);
	free(p->line);
	free(p);
}

outofmemory()
{
	printf("%s: Out of Memory\n",invoke);
	exit(ENOMEM);
}

qprint(line)		/* print a line efficiently */
char *line;
{
	char *cp;		/* scratch character pointer */
/*	char *end; NOTUSED	/* last legitimate array position */
	int r,l;		/* right most character, leftmost */
	int i,j;		/* temporary values */

	cp = line + width - 1;
	while ( cp >= line && *cp == ' ')
		--cp;

	if ( cp < line )
		return;

	if (diablo) {
		r = cp - line;
/* NOTUSED	end = line + width;	*/
		cp = line;
		while ( *cp++ == ' ' );
		l = cp - line - 1;
		i = abs(printhead - l) + (2 * ((direction == FORWARD) ? 1 : -1));
		j = abs(printhead - r);
		if ( (printhead < l) || (i < j) ) {
			spaceto(l,FORWARD);
			pr(line,l,r);
		} else {
			spaceto(r,BACKWARD);
			pr(line,r,l);
		}

	} else {
		*++cp = '\0';
		printf("%s\r", line);
	}
}

spaceto( where, dir)
int where;			/* which column */
int dir;			/* which way to go afterward */
{
	if (where == 0) {
		putchar('\r');
		direction = FORWARD;	/* where else should we go */
	} else {
		if ( direction != dir ) {
			printf("%s", ( dir == FORWARD ) ? FPRINT : RPRINT );
			direction = dir;
		}
		if (printhead != where) {
			putchar('\033');	/* <esc><tab># gets you to # */
			putchar('\t');
			putchar((char)where+1);
		}
	}
	printhead = where;
}

pr(line,start,stop)	/* prints the characters from start to stop */
int start;		/* array index of first character */
int stop;		/* array index of last character */
char *line;		/* line to play with */
{
	char *end;		/* last character location to access */
	char *cp;		/* scratch character position */

	end = line + stop;
	cp = line + start;
	if (start < stop) {
		while ( cp <= end )
			putchar( *cp++ );
		printhead = stop+1;
	} else {
		while ( cp >= end )
			putchar( *cp-- );
		if ( printhead < 0 )
			printhead = 0;
		printhead = stop-1;
	}
}

throwpage()		/* toss a page */
{
	printf("%s",ff);
	npages++;
	linenum = 0;
}
