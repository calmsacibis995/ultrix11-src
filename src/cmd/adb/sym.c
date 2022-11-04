
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/
#
static char *sccsid = "@(#)sym.c	3.0	4/21/86";
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

/* OVERLAY - added variables */
TXTHDR		txthdr;
struct map txtmap;
struct map datmap;
long 	basdot, ovbase;
int 	wantov, nodot, magic,smlsym;
/* OVERLAY */

MSG		BADFIL;

struct symtab 	symbol, *lastsym;
char 	localok;
int 	lastframe;
struct symslave 	*symvec;
unsigned 	maxoff;
long 	maxstor;

/* symbol management */
long	symseek;
long 	symbas;
long 	symcnt;
long 	symnum;
long 	localval;
char		symrqd  = -1;
struct symtab 	symbuf[SYMSIZ];
struct symtab *symnxt;
struct symtab *symend;


int 	fsym;
char *errflg;
unsigned 	findsym();


/* symbol table and file handling service routines */

longseek(f, a)
long a;
{
	return(lseek(f,a,0) != -1);
}

valpr(v,idsp)
{
	unsigned 	d;
	d = findsym(v,idsp);
	if(d < maxoff)
	{
		printf("%.8s", symbol.symc);
		if(d)
		{
			printf(OFFMODE, d);

		}

	}
}

localsym(cframe)
long cframe;
{
	int symflg;
	while(nextsym() && localok
	    && symbol.symc[0]!='~'
	    && (symflg=symbol.symf)!=037){
		if(symflg>=2 && symflg<=4)
		{
			localval=symbol.symv;
			return((-1));
		}
		else if(symflg==1)
		{
			localval=leng(shorten(cframe)+symbol.symv);
			return((-1));
		}
		else if(symflg==20 && lastframe)
		{
			localval=leng(lastframe+2*symbol.symv-(magic>=0430?12:10));
			return((-1));

		}
	}
	return(0);
}
psymoff(v,type,s)
long v; 
int type; 
char *s;
{
	unsigned 	w;

	w = findsym(shorten(v),type);
	if(!nodot && w == 0 && type == ISYM && basdot != v)
	{
		lastsym=lookupsym(&symbol);
		basdot = v;
	}
	nodot = 0;
	if(w >= maxoff)
	{
		printf(LPRMODE,v);
	}
	else{
		printf("%.8s", symbol.symc);
		if(w)
		{
			printf(OFFMODE,w);
		}

	}
	printf(s);
}

unsigned findsym(svalue,type)
unsigned svalue;
int type;
{
	long 	offset, ovlow, value, diff, symval, lstsymv;
	struct symslave *symsav, *symptr;
	int 	symtyp,inovly;
	value=svalue; 
	diff = 0377777L; 
	symsav=0;

	if(type!=NSYM && (symptr=symvec))
	{
		inovly = (type&ISYM && inov(value))?1:0;
		if(inovly && !wantov && lastsym)
			wantov = lastsym->symo;
		while(diff && (symtyp=symptr->typslave)!=ESYM){
			if(symtyp==type &&(inovly?symptr->ovnslave == wantov:1))
			{
				symval=leng(symptr->valslave);
				if(value-symval<diff
				    && value>=symval)
				{
					diff = value-symval;
					symsav=symptr;

				}

			}
			symptr++;
		}
		if(symsav)
		{
			if(smlsym)
				offset = symsav->cntslave;
			else
				offset=leng(symsav-symvec);
			symcnt=symnum-offset;
			longseek(fsym, symbas+offset*(sizeof (struct symtab)));
			read(fsym,&symbol,(sizeof (struct symtab)));
		}

	}
	wantov = 0;
	return((offset = shorten(diff)));
}

nextsym()
{
	if((--symcnt)<0)
	{
		return(0);

	}
	else{
		return(longseek(fsym, symbas+(symnum-symcnt)*(sizeof (struct symtab)))!=0 &&
		    read(fsym,&symbol,(sizeof (struct symtab)))==(sizeof (struct symtab)));

	}
}



/* sequential search through file */
symset()
{
	symcnt = symnum;
	symnxt = symbuf;
	if(symrqd)
	{
		symseek = symbas;
		longseek(fsym, symbas);
		symread(); 
		symrqd=0;

	}
	else{
		symseek = symbas+sizeof symbuf;
		longseek(fsym, symbas+sizeof symbuf);

	}
}

struct symtab *symget()
{
	register int rc;
	if(symnxt >= symend)
	{
		rc=symread(); 
		symrqd=(-1);

	}
	else{
		rc=(-1);

	}
	if(--symcnt>0 && rc==0)
	{
		errflg=BADFIL;
	}
	return((symcnt>=0 && rc) ? symnxt++ : 0);
}

symread()
{
	int 	symlen;

	longseek(fsym, symseek);
	if((symlen=read(fsym,symbuf,sizeof symbuf))>=(sizeof (struct symtab)))
	{
		symseek += leng(symlen);
		symnxt = symbuf;
		symend = &symbuf[symlen/(sizeof (struct symtab))];
		return((-1));

	}
	else{
		return(0);

	}
}
