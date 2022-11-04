
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#
static char *sccsid = "@(#)command.c 3.0 4/21/86";
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

MSG		BADEQ;
MSG		NOMATCH;
MSG		BADVAR;
MSG		BADCOM;
MSG		NOTOV;
MSG		NOCORE;
MSG		BADOVN;

struct map txtmap;
struct map datmap;
int 	executing;
char 	*lp;
int 	fcor;
int 	fsym;
int 	mkfault;
char *errflg;
char 	lastc, eqcom;
char 	eqformat[1024] = "o";
char 	stformat[1024] = "o^\"= \"i";
unsigned 	*endhdr;
long 	dot,tdot;
long 	ditto;
int 	dotinc;
int 	lastcom = '=';
long 	var[];
long 	locval;
long 	locmsk;
int 	pid;
long 	expv;
long 	adrval;
int 	adrflg;
long 	cntval;
int 	cntflg;

/* OVERLAY - added variables */
int	magic,wantov, numovly,cwantov;
long	symnum;
struct symtab symbol, *lastsym;
/* OVERLAY */

/* command decoding */

command(buf,defcom)
char *buf;
int 	defcom;		/* OVERLAY - was char */
{
	int 	itype, ptype, modifier, regptr;
	char 	longpr, ovrqst;
	char 	wformat[1];
	char 	savc;
	long 	w, savdot;
	char *savlp = lp;

	if(buf)
	{
		if(*buf=='\n')
		{
			return(0);
		}
		else{
			lp=buf;
		}
	}
	do{
		ovrqst = 0;
		if(adrflg=expr(0))
		{
			dot=expv; 
			ditto=dot;
		}
		adrval=dot;
		if(rdc()==',' && expr(0))
		{
			cntflg=(-1); 
			cntval=expv;
		}
		else{
			cntflg=0; 
			cntval=1; 
			lp--;
		}
		if(adrflg && lastc == '!'){
			if(magic < 0430)
				error(NOTOV);
			lp++;
			/*
			 * Overlay number can be
			 *	A single hex digit
			 *	one or two decimal digits,
			 *	two or three octal digits (first is a '0')
			 */
			lastc = rdc();
			if (lastc >= 'a' && lastc <= 'f') {
				ovrqst = lastc - 'a' + 10;
				lastc = rdc();
			} else {
				int base = 10;
				if (lastc == '0') {
					base = 8;
					lastc = rdc();
				}
				ovrqst = lastc - '0';
				lastc = rdc();
				if (lastc >= '0' && lastc < '0' + base) {
					ovrqst *= base;
					ovrqst += lastc - '0';
					lastc = rdc();
				}
			}
			if(ovrqst > 15 || ovrqst <= 0 || ovrqst > numovly)
				error(BADOVN);
			wantov = ovrqst;
			lp--;
		}
		if(!eol(rdc()))
		{
			lastcom=lastc;
		}
		else{
			if(adrflg==0)
			{
				dot=inkdot(dotinc);
			}
			lp--; 
			lastcom=defcom;
		}
		switch(lastcom&0177) {
		case '/':
			if(fcor <= 0 && pid == 0)
				error(NOCORE);
			itype=DSP; 
			ptype=DSYM;
			goto trystar;

		case '=':
			itype=NSP; 
			ptype=ASYM;
			goto trypr;

		case '?':
			itype=ISP; 
			ptype=ISYM;
			goto trystar;
trystar:
			if(rdc()=='*')
			{
				lastcom |= QUOTE; 
			}
			else{
				lp--;
			}
			if(lastcom&QUOTE)
			{
				itype |= STAR; 
				ptype = (DSYM+ISYM)-ptype;
			}
			/* OVERLAY - & address mode */
			if(rdc()=='&' )
			{
				if(magic < 0430)
				{
					error(NOTOV);
				}
				else{
					lastcom |= 0400;
				}
			}
			else{
				lp--;
			}
			if(lastcom&0400)
			{
				itype |= OVRLAY;
			}
trypr:
			/* OVERLAY */
			longpr=0; 
			eqcom=lastcom=='=';
			switch (rdc()) {
			case 'm':
				{/*reset map data*/
					int 	fcount;
					struct map 	*smap;
					union{
						struct map *m; 
						long *mp;
					}
					amap;

					if(eqcom)
					{
						error(BADEQ);
					}
					smap=(itype&DSP?&datmap:&txtmap);
					amap.m=smap; 
					fcount=3;
					if(itype&STAR)
					{
						amap.mp += 3;
					}
					while(fcount-- && expr(0)){
						*(amap.mp)++ = expv; 
					}
					if(rdc()=='?')
					{
						smap->ufd=fsym;
					}
					else if(lastc == '/')
					{
						smap->ufd=fcor;
					}
					else{
						lp--;
					}
				}
				break;

			case 'L':
				longpr=(-1);
			case 'l':
				/*search for exp*/
				if(eqcom)
				{
					error(BADEQ);
				}
				dotinc=2; 
				savdot=dot;
				expr(1); 
				locval=expv;
				if(expr(0))
				{
					locmsk=expv; 
				}
				else{
					locmsk = -1L;
				}
				printf("Searching\n");
				for(;;){
					w=leng(get(dot,itype));
					if(longpr)
					{
						w=itol(w,get(inkdot(2),itype));
					}
					if(errflg || mkfault || (w&locmsk)==locval)
					{
						break;
					}
					dot=inkdot(dotinc);
				}
				if(errflg)
				{
printf("%8O ",dot);
					dot=savdot; 
					errflg=NOMATCH;
				}
				psymoff(dot,ptype,"");
				break;

			case 'W':
				longpr=(-1);
			case 'w':
				if(eqcom)
				{
					error(BADEQ);
				}
				wformat[0]=lastc; 
				expr(1);
				do{
					savdot=dot; 
					psymoff(dot,ptype,":%16t"); 
					exform(1,wformat,itype,ptype);
					errflg=0; 
					dot=savdot;
					if(longpr)
					{
						put(dot,itype,expv);
					}
					put((longpr?inkdot(2):dot),itype,shorten(expv));
					savdot=dot;
					printf("=%8t"); 
					exform(1,wformat,itype,ptype);
					newline();
				}
				while(	expr(0) && errflg==0);
				dot=savdot;
				chkerr();
				break;
			default:
				lp--;
				getformat(eqcom ? eqformat : stformat);
				if(!eqcom)
				{
					if(!ovrqst && symnum && lastsym && inov(dot))
						wantov = lastsym->symo;
					else if(ovrqst && lastsym == 0){
						if(symnum){
							findsym(shorten(dot),ptype);
							lastsym = lookupsym(&symbol);
						}
						cwantov = wantov = ovrqst;
						itype |= OVRLAY;
						lastcom |= 0400;
					}
					psymoff(dot,ptype,":%16t");
				}
				scanform(cntval,(eqcom?eqformat:stformat),itype,ptype);
			}
			break;
		case '>':
			lastcom=0; 
			savc=rdc();
			if(regptr=getreg(savc))
			{
				endhdr[regptr]=shorten(dot);
				ptrace(WUREGS,pid,UROFF+2*regptr,endhdr[regptr]);
			}
			else if((modifier=varchk(savc)) != -1)
			{
				var[modifier]=dot;
			}
			else{
				error(BADVAR);
			}
			break;

		case '!':
			lastcom=0;
			unox(); 
			break;

		case '$':
			lastcom=0;
			printtrace(nextchar()); 
			break;

		case ':':
			if(!executing)
			{
				executing=(-1);
				subpcs(nextchar());
				executing=0;
				lastcom=0;
			}
			break;

		case 0:
			prints(DBNAME);
			break;

		default: 
			error(BADCOM);
		}
		flushbuf();
	}
	while(rdc()==';');
	if(buf)
	{
		lp=savlp; 
	}
	else{
		lp--;
	}
	return(adrflg && dot!=0);
}
