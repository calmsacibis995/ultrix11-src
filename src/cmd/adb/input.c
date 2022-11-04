
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#
static char *sccsid = "@(#)input.c 3.0 4/21/86";
/*
 *
 *	UNIX debugger
 *
 */

#include "defs.h"

int 	mkfault;
char 	line[LINSIZ];
int 	infile;
char 	*lp;
char 	lastc = '\n';
int 	eof;

/* input routines */

eol(c)
char c;
{
	return(c=='\n' || c==';');
}

rdc()
{	
	do{
		readchar();
	}
	while(	lastc==SP || lastc==TB);
	return(lastc);
}

readchar()
{
	if(eof)
	{
		lastc=0;
	}
	else{
		if(lp==0)
		{
			lp=line;
			do{
				eof = read(infile,lp,1)==0;
				if(mkfault)
				{
					error(0);
				}
			} while(eof==0 && *lp++!='\n');
			*lp=0; 
			lp=line;
		}
		if(lastc = *lp)
		{
			lp++;
		}
	}
	return(lastc);
}

nextchar()
{
	if(eol(rdc()))
	{
		lp--; 
		return(0);
	}
	else{
		return(lastc);
	}
}

quotchar()
{
	if(readchar()=='\\')
	{
		return(readchar());
	}
	else if(lastc=='\'')
	{
		return(0);
	}
	else{
		return(lastc);
	}
}

getformat(deformat)
char *deformat;
{
	register char *fptr;
	register char quote;
	fptr=deformat; 
	quote=0;
	while((quote ? readchar()!='\n' : !eol(readchar()))){
		if((*fptr++ = lastc)=='"')
		{
			quote = ~quote;
		}
	}
	lp--;
	if(fptr!=deformat)
	{
		*fptr++ = '\0';
	}
}
