
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#
static char *sccsid = "@(#)format.c 3.0 4/21/86";
/*
 *
 *	UNIX debugger
 *
 *	Modified for overlay text support,
 *	changes flagged by the word OVERLAY
 *
 *	Fred Canter 3/23/82
 *
 */

#include "defs.h"

MSG		BADMOD;
MSG		NOFORK;
MSG		ADWRAP;

struct symtab 	symbol;

/* OVERLAY - added variables */
extern int magic, cwantov, maxpos;
extern long symnum;
/* OVERLAY */
int 	mkfault;
char 	*lp;
int 	maxoff;
int 	sigint;
int 	sigqit;
char *errflg;
char 	lastc;
long 	dot;
int 	dotinc;
long 	var[];
int	savdotinc;

scanform(icount,ifp,itype,ptype)
long 	icount;
char *ifp;
{
	char *fp;
	char 	modifier;
	int 	fcount, init=1;
	long 	savdot;

	while(icount){
		fp=ifp;
		if(init==0 && findsym(shorten(dot),ptype)==0 && maxoff)
		{
			printf("\n%.8s:%16t",symbol.symc);
		}
		savdot=dot; 
		savdotinc = 1;
		init=0;

		/*now loop over format*/
		while(*fp && errflg==0){
			if(digit(modifier = *fp))
			{
				fcount=0;
				while(digit(modifier = *fp++)){
					fcount *= 10;
					fcount += modifier-'0';
				}
				fp--;
			}
			else{
				fcount=1;
			}
			if(*fp==0)
			{
				break;
			}
			fp=exform(fcount,fp,itype,ptype);
		}
		dotinc=dot-savdot;
		dot=savdot;
		if(errflg)
		{
			if(icount<0)
			{
				errflg=0; 
				break;
			}
			else{
				error(errflg);
			}
		}
		if(--icount)
		{
			dot=inkdot(dotinc);
		}
		if(mkfault)
		{
			error(0);
		}
	}
}

char *exform(fcount,ifp,itype,ptype)
int 	fcount;
char *ifp;
{
	/* execute single format item `fcount' times
		 * sets `dotinc' and moves `dot'
		 * returns address of next format item
		 */
	unsigned 	w;
	long 	savdot, wx;
	char *fp;
	char 	c, modifier, longpr;
	double 	fw;
	struct x {
		long sa;
		int sb,sc;
	};

	while(fcount>0){
		fp = ifp; 
		c = *fp;
		longpr=(c>='A')&(c<='Z')|(c=='f');
		if(itype==NSP || *fp=='a')
		{
			wx=dot; 
			w=dot;
		}
		else{
			w=get(dot,itype);
			if(longpr)
			{
				wx=itol(w,get(inkdot(2),itype));
			}
			else{
				wx=w;
			}
		}
		if(c=='F')
		{
			((struct x *)&fw)->sb=get(inkdot(4),itype);
			((struct x *)&fw)->sc=get(inkdot(6),itype);
		}
		if(errflg)
		{
			return(fp);
		}
		if(mkfault)
		{
			error(0);
		}
		var[0]=wx;
		modifier = *fp++;
		if(magic>=0430 && wx==dot && modifier=='o' && wx!=leng(w))
			modifier = 'y';
		dotinc=(longpr?4:2);
		if(charpos()==0 && modifier!='a')
		{
			printf("%16m");
		}
		if(symnum)
			cwantov = 0;
		switch(modifier) {

		case' ': 
		case TB:
			break;

		case 't': 
		case 'T':
			printf("%T",fcount); 
			return(fp);

		case 'r': 
		case 'R':
			printf("%M",fcount); 
			return(fp);

		case 'a':
			psymoff(dot,ptype,":%16t");
			dotinc=0; 
			break;

		case 'p':
			psymoff(var[0],ptype,"%16t");
			break;

		case 'u':
			printf("%-8u",w); 
			break;

		case 'U':
			printf("%-16U",wx); 
			break;

		case 'c': 
		case 'C':
			if(modifier=='C')
			{
				printesc(w&0377);
			}
			else{
				printc(w&0377);
			}
			dotinc=1; 
			break;

		case 'b': 
		case 'B':
			printf("%-8o", w&0377); 
			dotinc=1; 
			break;

		case 's': 
		case 'S':
			savdot=dot; 
			dotinc=1;
			while((c=get(dot,itype)&0377) && errflg==0){
				dot=inkdot(1);
				if(modifier == 'S')
				{
					printesc(c);
				}
				else{
					printc(c);
				}
				endline();
			}
			dotinc=dot-savdot+1; 
			dot=savdot; 
			break;

		case 'x':
			printf("%-8x",w); 
			break;

		case 'X':
			printf("%-16X", wx); 
			break;

		case 'Y':
			printf("%-24Y", wx); 
			break;

		case 'q':
			printf("%-8q", w); 
			break;

		case 'Q':
			printf("%-16Q", wx); 
			break;

		case 'z':
		case 'Z':
			printbin(w, c=='Z'?16:8);
			dotinc = c=='Z'?2:1;
			break;

		case 'y':
			printf("%-8o ovly = %o", wx,symbol.symo); 
			break;

		case 'o':
		case 'w':
			printf("%-8o", w); 
			break;

		case 'O':
		case 'W':
			printf("%-16O", wx); 
			break;

		case 'i':
			printins(0,itype,w); 
			printc('\n'); 
			break;

		case 'd':
			printf("%-8d", w); 
			break;

		case 'D':
			printf("%-16D", wx); 
			break;

		case 'f':
			fw = 0;
			((struct x *)&fw)->sa = wx;
			printf("%-16.9f", fw);
			dotinc=4; 
			break;

		case 'F':
			((struct x *)&fw)->sa = wx;
			printf("%-32.18F", fw);
			dotinc=8; 
			break;

		case 'n': 
		case 'N':
			printc('\n'); 
			dotinc=0; 
			break;

		case '"':
			dotinc=0;
			while(*fp != '"' && *fp){
				printc(*fp++); 
			}
			if(*fp)
			{
				fp++;
			}
			break;

		case '^':
			dot=inkdot(-savdotinc*fcount); 
			return(fp);

		case '+':
			/* OVERLAY - .+offset addressing */
			dot=inkdot(fcount);
			return(fp);
			/* OVERLAY */

		case '-':
			/* OVERLAY - .-offset addressing */
			dot=inkdot(-fcount);
			return(fp);
			/* OVERLAY */

		default: 
			error(BADMOD);
		}
		if(itype!=NSP)
		{
			dot=inkdot(dotinc);
			savdotinc = dotinc;
		}
		fcount--; 
		endline();
	}
	return(fp);
}

unox()
{
	int 	rc, status, unixpid;
	char *argp = lp;

	while(lastc!='\n'){
		rdc(); 
	}
	if((unixpid=fork())==0)
	{
		signal(SIGINT,sigint); 
		signal(SIGQUIT,sigqit);
		*lp=0; 
		execl("/bin/sh", "sh", "-c", argp, 0);
		exit(16);
	}
	else if(unixpid == -1)
	{
		error(NOFORK);
	}
	else{
		signal(SIGINT,1);
		while((rc = wait(&status)) != unixpid && rc != -1);

		signal(SIGINT,sigint);
		prints("!"); 
		lp--;
	}
}

printesc(c)
{
	c &= 0177;
	if(c<SP || c>'~' || c=='@')
	{
		printf("@%c",(c=='@' ? '@' : c^0140));
	}
	else{
		printc(c);
	}
}

long inkdot(incr)
{
	long 	newdot;

	newdot=dot+incr;
	if((dot ^ newdot) >> 24)
	{
		error(ADWRAP);
	}
	return(newdot);
}

printbin(v, n)
register int v, n;
{
	if(charpos() >= maxpos-n)
		printf("\n%16m");
	while(n--)
		printc((v&(1<<n)) ?'1':'0');
	prints("  ");
}
