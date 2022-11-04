
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/


static char Sccsid[] = "@(#)ln03of.c	3.0	4/21/86";
/*
 *	Letter Quality Printers filter for ln03 looking like lqp
 *
 * 	filter which reads the output of nroff and converts lines
 *	with ^H's to overwritten lines.  Thus this works like 'ul'
 *	but is much better: it can handle more than 2 overwrites
 *	and it is written with some style.
 *	modified by kls to use register references instead of arrays
 *	to try to gain a little speed.
 *
 * 	Passes through escape and control sequences.
 *
 *	Sends control chars to change to landscape mode for pages wider
 *	than 80 columns.  Also changes font and pitch for this case in
 *	order to get 66 lines per page in landscape mode.
 *
 */
/*************************************************************************/
#include	<stdio.h>
#include	<ctype.h>
#include	<errno.h>
#include	<sgtty.h>
#include	<signal.h>
#include	"lp.local.h"
#include	"globals.h"

#define LN03of			/* globals.h: so it won't do the putchar'\r' */
#define MAXWIDTH  132
#define MAXREP    10

/*************************************************************************/
/* added for escape sequence pass through 				 */
#define ESC	  '\033'	/* escape sequence introducer */
#define BSLH	  '\134'	/* back slash */
#define UCP	  '\120'	/* upper case P */
#define escend(x) ((x!='\120')&&(x!='\133')&&(x>='\100')&&(x<='\176'))
int   	escflg =  0;		/* escape sequence flag, 1 = in progress */
int	lstchr;		
/*************************************************************************/

main(argc, argv) 
int argc;
char *argv[];
{
	register FILE *p = stdin, *o = stdout;
	register int i, col;
	register char *cp;
	char	buf[MAXREP][MAXWIDTH];
	int	maxcol[MAXREP];
	int	lineno=0;
	int	indent=0;	/* indentation length */
	int	literal=0;	/* print control characters */
	int done, linedone, maxrep;
	char ch, *limit;

	for(i=0;i<MAXREP;maxcol[i]= -1, i++);
	width=80;
	length=66;
	init(argv);
	if (width > 80) {	/* switch to landscape mode */
			fprintf(o,"\033[15m");    /* change font */
			fprintf(o,"\033[7 J");    /* wide extended A4 page format */	
			fprintf(o,"\033[66t");    /* 66 lines/page */
			fprintf(o,"\033[8 L");    /* vertical pitch = 12 lines/30mm */
			}
	for (cp = buf[0], limit = buf[MAXREP]; cp < limit; *cp++ = ' ');
	done = 0;
	while (!done) {		/* span FFs and NULLS */
		ch=getc(p);
		if((ch != '\f' && ch != '\0') || ch == EOF)
			{
			ungetc(ch,p);
			done=1;
			}
		}
	escflg = 0;		/* is escape/control sequence in progress? */
	done = 0;
	while (!done) {
		col = indent;
		maxrep = -1;
		linedone = 0;
		while (!linedone) {
			ch = getc(p);
			if (((escflg==0)&&(ch==ESC))||escflg)
				eschdl(o,ch);	/* deal with escape character */
			else 
				switch (ch) {
				case EOF:
					linedone = done = 1;
					ch = '\n';
					break;
	
				case '\f':		/* new page on form feed */
					lineno = length;
				case '\n':		/* new line */
					if (maxrep < 0)
						maxrep = 0;
					linedone = 1;
					break;
	
				case '\b':		/* backspace */
					if (--col < indent)
						col = indent;
					break;
	
				case '\r':		/* carriage return */
					col = indent;
					break;
	
				case '\t':		/* tab */
					col = ((col - indent) | 07) + indent + 1;
					break;
	
				default:		/* everything else */
					if (col >= width || !literal && ch < ' ') {
						col++;
						break;
					}
					cp = &buf[0][col];
					for (i = 0; i < MAXREP; i++) {
						if (i > maxrep)
							maxrep = i;
						if (*cp == ' ') {
							*cp = ch;
							if (col > maxcol[i])
								maxcol[i] = col;
							break;
						}
						cp += MAXWIDTH;
					}
					col++;
					break;
				}
			}

		/* print out lines */
		for (i = 0; i <= maxrep; i++) {
			for (cp = buf[i], limit = cp+maxcol[i]; cp <= limit;) {
				putc(*cp, o);
				*cp++ = ' ';
			}
			if (i < maxrep)
				putc('\r', o);
			else
				putc(ch, o);
			if (++lineno >= length) {
				npages++;
				lineno = 0;
				if (length < 66)
					putchar('\f');  /* FF for length < 66 */
			}
			maxcol[i] = -1;
		}
	}
	if (lineno) {		/* be sure to end on a page boundary */
		putchar('\f');
		npages++;
	}
	bill(user);
	putchar('\033');	/* reset printer defaults */
	putchar('\143');
	fflush(o);
	sleep(5);		/* take five - allow printer to reset */
}
/****************************************************************/
/*								*/
/*	eschdl - escape sequence handler			*/
/*								*/
/*      This routine intercepts escape sequences for the purpose*/
/*	of pass through.					*/
/*								*/
/****************************************************************/
eschdl(o,c)
int c;
FILE  *o;
{
if(escflg==0)
	{		/* set escflg=1 => ready to receive 2nd seqchar*/
	escflg=1;
	}
else	switch(escflg)
		{
		case 1:		/* second character of escseq 		*/
			switch(c)
				{
  				case UCP:
					escflg=2; /*ctrl str pass thru mode=8 */
					lstchr=c;
					putc(ESC,o);
					putc(c,o);
					break;
				default:
					escflg=3;  /* set seq pass thru mode*/
					putc(ESC,o);
					putc(c,o);
					break;
				}
			break;
		case 2:		/* ctrl string pass through mode       	*/
			if((lstchr==ESC) && (c==BSLH))
				{
				escflg=0;
				lstchr=0;
				}
			else lstchr=c;	/* save it for next pass */
			putc(c,o);
			break;
		case 3:
			if(escend(c))
				escflg=0;/* turn off esc handler if at end  */
			putc(c,o);
			break;
		}
return(0);
}
