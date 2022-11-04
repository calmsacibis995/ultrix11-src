
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/


static char Sccsid[] = "@(#)ln01of.c	3.0	4/21/86";
/*  	LN01 NROFF FILTER
 * 	filter which reads the output of nroff and converts it to
 *	LN01 compatible image.  Since nroff only deals with monospaced
 *	fonts this limits the number of fonts that will work effectively.
 *	The fonts and font selection files are in /usr/lib/font/devln01/
 *	This filter will pass through any escape or control sequences.
 *	Underlining has been implemented using the escape sequences so
 *	that if at some time proportional fonts are used (ever so ragged)
 *	they will be properly underlined.  
 */
#include	<stdio.h>
#include	<ctype.h>
#include	<errno.h>
#include	<sgtty.h>
#include	<signal.h>
#include	"lp.local.h"
#include	"globals.h"

#define psiz 50		/* 50 pixels per line in portrait mode */
#define lsiz 35		/* 35 pixels per line in landscape mode   */
#define fontdir "/usr/lib/font/devln01/" /* font directory */

int	indent = 0;	/* indentation length */

/********************************************************/
/*	added for escape sequence ignoring for ln01	*/
/********************************************************/
#define ESC		'\033'  /* escape sequence introducer */
#define BSLH		'\134'  /* back slash */
#define UCP		'\120'  /* uppercase P */
#define UCF		'\106'  /* uppercase F for internal font loads */
#define escend(x)	((x!='\120')&&(x!='\133')&&(x>='\100')&&(x<='\176'))
int	escflg=0;		/* escape sequence flag = 1 in progress */
/********************************************************/

main(argc, argv) 
	int argc;
	char *argv[];
{
	register int i, col;
	FILE *p = stdin, *o = stdout;
	int nlines=0,npages=0;
	int lund,ch,undflg,last,done;

	width=80;
	length=66;
	init(argv);
/*								      */
/*	attempt load of default fonts if not acting as ditroff filter */
/*								      */
	fprintf(o,"\033\143");	/* reset printer */
	fflush(o);
	sleep(2);		/*allow some time*/
	setsiz(o,length,width); /* set page size */
	col = indent;
	last=0;	 	/* last character processed	*/
	lund=0;		/* flag was last char underlined (needed for tbl bug)*/
	undflg=0;	/* is underlining in progress   */
	escflg=0;	/* is escape/control sequence in progress */
	done=0;
	while(!done)
		{
		ch=getc(p);
		if((ch!='\f' && ch != '\0') || ch == EOF) 
			{
			done=1;
			ungetc(ch,p);
			}
		}
	while((ch=getc(p)) != EOF)
		{
		/*	Escape sequence pass through code     */
		if (((escflg==0) && (ch == ESC)) || escflg)
			{
			if(last=='_')
				{
				putc('_',o);
				last=0;
				}
			if(undflg)
				{
				undflg=0;
				lund=0;
				fprintf(o,"\033\1330m");
				}
			col=indent; /* assume ditroff in control of chrs/line */
			eschdl(o,ch);
			}
		else
			{
			if((last=='_')&&(ch!='\b'))
				{
				putc('_',o);
				undflg=0;
				col++;
				}
			switch (ch) 
			{
			case '\f':	/* new page on form feed */
				npages++;
			case '\n':	/* new line */
				if(ch=='\f')
					nlines=0;
				else
					nlines++;
				col=0;
				if(undflg)
					{
					undflg=0;
					lund=0;
					fprintf(o,"\033\1330m");
					}
				putc(ch,o);
				break;
			case '\b':	/* backspace */
				if(last=='_')
					{
					undflg=1;
					fprintf(o,"\177\033\1334m");
					}
				else	if(lund)
						lund++;
					else	putc(ch,o);
				break;
			case '\r':	/* carriage return */
				col = indent;
				if(undflg)
					{
					undflg=0;
					lund=0;
					fprintf(o,"\033\1330m");
					}
				putc(ch,o);
				for(i=0;i != indent;i++,putc(' ',o));
				break;
			case '_':
				break;
			case '\t':	/* tab it default is tab/8char*/
				if(undflg)
					{
					undflg=0;
					lund=0;
					fprintf(o,"\033\1330m");
					}
				for(col++;(col % 8) != 0;col++);
				putc(ch,o);
				break;
			default:	/* all else */
				if (col < width)
					col++;
				else
					{
					undflg=0;
					fprintf(o,"\033\1330m");
					col=indent;
					putc('\r',o);
					putc('\n',o);
					for(i=0;i != indent;i++,putc(' ',o));
					nlines++;
					}
				if(lund<=1)
					putc(ch,o);
				lund=0;
				if(undflg)
					{
					undflg=0;
					fprintf(o,"\033\1330m");
					lund=1;
					}
			}
			last=ch;
			}
		}
	bill(user);
	fprintf(o,"\014\033\143");	/* send FF + reset printer */
	fflush(o);
	sleep(8);			/* allow some time */
	exit(0);
}
/****************************************************************/
/*								*/
/*	eschdl - escape sequence handler			*/
/*								*/
/*      This routine intercepts escape sequences for the purpose*/
/*	of pass through.					*/
/*								*/
/*	This routine also intercepts inyternal font load sequences */
/*	the sequence format is as follows:			*/
/*	ESC UCF fontfile;fontfile;...fontfile BSLH		*/
/*      fonts are assumed to be in loadable form in fontdir	*/
/****************************************************************/
eschdl(o,c)
int c;
FILE  *o;
{
FILE  *fnt;
char	fntfil[50];	/* string for font filename*/
char	filnam[20];
int	lstchr,filcnt,fnmind;
int chr;
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
				case UCF:
					escflg=4;  /* set load font case */
					fnmind=0;
					filcnt=0;
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
		case 4:
			switch(c)
				{
				case BSLH:
				case ';':
					if(filcnt==0)
						{
						fprintf(o,"\033P1;1y");
						filcnt++;
						}
					filnam[fnmind]='\0';
					fnmind=0;
					sprintf(fntfil,"%s%s",fontdir,filnam);
					if((fnt=fopen(fntfil,"r")) == NULL)
						break;
					while((chr=getc(fnt)) != EOF)
						putc(chr,o);
					fclose(fnt);
					if(c==BSLH)
						{
						escflg=0;
						fprintf(o,"; \033\134");
						}
					break;
				default:
					filnam[fnmind++]=c;
				}
		}
return(0);
}
/************************************************************************/
/*									*/
/*	setsiz - set page size						*/
/*									*/
/*	assume constant width characters				*/
/************************************************************************/
setsiz(o,len,wid)
FILE	*o;
int len,wid;
{
int plen,pstr;
if(wid>80)
	{			/* landscape mode	*/
	plen=(71*lsiz)+1;
	pstr=(5*lsiz)+1;
	}
else	
	{			/* portrait mode	*/
	plen=(len*psiz)+1;
	pstr=1;
	}
fprintf(o,"\033\1333300t");	/* set full size page	*/
fprintf(o,"\033\133%d;%dr",pstr,plen);/* set margins within page*/
fflush(o);
return(0);
}
