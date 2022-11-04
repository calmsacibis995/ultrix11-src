
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#
static char *sccsid = "@(#)expr.c	3.0	4/21/86";
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
extern char line[];
struct symslave *symsp;
struct symslave *symvec;
long 	ovsymnum,symnum, basdot;
int 	cwantov;
struct symtab *lastsym;
/* OVERLAY */
MSG		BADSYM;
MSG		BADVAR;
MSG		BADKET;
MSG		BADSYN;
MSG		NOCFN;
MSG		NOADR;
MSG		BADLOC;

struct symtab 	symbol;
int 	lastframe;
int 	savlastf;
long 	savframe;
int 	savpc;
int 	callpc;
char 	*lp;
int 	octal;
char *errflg;
long 	localval;
char 	isymbol[8];
char 	lastc;
unsigned 	*endhdr;
long 	dot;
long 	ditto;
int 	dotinc;
long 	var[];
long 	expv;

expr(a)
{	/* term | term dyadic expr |	*/
	int 	rc;
	long 	lhs;

	rdc(); 
	lp--; 
	rc=term(a);
	while(rc){
		lhs = expv;
		switch (readchar()) {

		case '+':
			/* OVERLAY - symbol+offset address mode */
			term(a|1);
			expv += lhs;
			break;
			/* OVERLAY */

		case '-':
			/* OVERLAY - symbol-offset address mode */
			term(a|1);
			expv = lhs - expv;
			break;
			/* OVERLAY */

		case '#':
			term(a|1); 
			expv = round(lhs,expv); 
			break;

		case '*':
			term(a|1); 
			expv *= lhs; 
			break;

		case '%':
			term(a|1); 
			expv = lhs/expv; 
			break;

		case '&':
			term(a|1); 
			expv &= lhs; 
			break;

		case '|':
			term(a|1); 
			expv |= lhs; 
			break;

		case ')':
			if((a&2)==0)
			{
				error(BADKET);
			}

		default:
			lp--;
			return(rc);
		}
	}
	return(rc);
}

term(a)
{	/* item | monadic item | (expr) | */

	switch (readchar()) {
	case '*':
		term(a|1); 
		expv=chkget(expv,DSP); 
		return(1);

	case '@':
		term(a|1); 
		expv=chkget(expv,ISP); 
		return(1);

	case '-':
		term(a|1); 
		expv = -expv; 
		return(1);

	case '~':
		if((lp - 1) == line)
		{
			lp--;
			return(item(a));
		}
		term(a|1); 
		expv = ~expv; 
		return(1);

	case '(':
		expr(2);
		if(*lp!=')')
		{
			error(BADSYN);
		}
		else{
			lp++; 
			return(1);
		}

	default:
		lp--;
		return(item(a));
	}
}

item(a)
{	/* name [ . local ] | number | . | ^ | <var | <register | 'x | | */
	int 	base, d, frpt, regptr, indx;
	char 	savc;
	char 	hex;
	long 	frame;
	union{
		float r; 
		long i;
	} 
	real;

	hex=0;
	readchar();
	if(symchar(0))
	{
		readsym();
		if(lastc=='.')
		{
			frame=endhdr[r5]&EVEN; 
			lastframe=0; 
			callpc=endhdr[pc];
			while(errflg==0){
				savpc=callpc;
				findroutine(frame);
				if(symnum)		/* XXX */
					cwantov = 0;
				if(eqsym(symbol.symc,isymbol,'~'))
				{
					break;
				}
				lastframe=frame;
				frame=get(frame,DSP)&EVEN;
				if(frame==0)
				{
					error(NOCFN);
				}
			}
			savlastf=lastframe; 
			savframe=frame;
			readchar();
			if(symchar(0))
			{
				chkloc(expv=frame);
			}
		}
		else if((lastsym=lookupsym(isymbol))==0)
		{
			error(BADSYM);
		}
		else{
			expv = lastsym->symv;
			basdot = expv;
			/* OVERLAY - set alternate expv for overlay symbols */
			symsp = symvec;
			for(indx=0; indx<(symnum==ovsymnum?symnum:ovsymnum); indx++){
				if(lastsym->symv == symsp->valslave
				  && lastsym->symo == symsp->ovnslave)
				{
					break;
				}
				symsp++;
			}
			/* OVERLAY */
		}
		lp--;

	}
	else if(digit(lastc) || (hex=(-1), lastc=='#' && hexdigit(readchar())))
	{
		expv = 0;
		if((lp - 1) == &line)
			lastsym = 0;
		base = (lastc == '0' || octal ? 8 : (hex ? 16 : 10));
		while((hex ? hexdigit(lastc) : digit(lastc))){
			expv *= base;
			if((d=convdig(lastc))>=base)
			{
				error(BADSYN);
			}
			expv += d; 
			readchar();
			if(expv==0 && (lastc=='x' || lastc=='X'))
			{
				hex=(-1); 
				base=16; 
				readchar();
			}
		}
		if(lastc=='.' && (base==10 || expv==0) && !hex)
		{
			real.r=expv; 
			frpt=0; 
			base=10;
			while(digit(readchar())){
				real.r *= base; 
				frpt++;
				real.r += lastc-'0';
			}
			while(frpt--){
				real.r /= base; 
			}
			expv = real.i;
		}
		lp--;
		if(symnum)
			cwantov = 0;
	}
	else if(lastc=='.')
	{
		readchar();
		if(symchar(0))
		{
			lastframe=savlastf; 
			callpc=savpc; 
			findroutine(savframe);
			if(symnum)		/* XXX */
				cwantov = 0;
			chkloc(savframe);
		}
		else{
			expv=dot;
		}
		lp--;
	}
	else if(lastc=='"')
	{
		expv=ditto;
	}
	else if(lastc=='+')
	{
		expv=inkdot(dotinc);
	}
	else if(lastc=='^')
	{
		expv=inkdot(-dotinc);
	}
	else if(lastc=='<')
	{
		savc=rdc();
		if(regptr=getreg(savc))
		{
			expv=endhdr[regptr];
		}
		else if((base=varchk(savc)) != -1)
		{
			expv=var[base];
		}
		else{
			error(BADVAR);
		}
	}
	else if(lastc=='\'')
	{
		d=4; 
		expv=0;
		while(quotchar()){
			if(d--)
			{
				if(d==1)
				{
					expv <<= 16;
				}
				expv |= ((d&1)?lastc:lastc<<8);
			}
			else{
				error(BADSYN);
			}
		}
	}
	else if(a)
	{
		error(NOADR);
	}
	else{
		lp--; 
		return(0);
	}
	return(1);
}

/* service routines for expression reading */

readsym()
{
	register char	*p;

	p = isymbol;
	do{
		if(p < &isymbol[8])
		{
			*p++ = lastc;
		}
		readchar();
	}
	while(symchar(1));

	while(p < &isymbol[8]){
		*p++ = 0; 
	}
}

struct symtab *lookupsym(symstr)
char *symstr;
{
	struct symtab *symp;
	char *opnt;

	if(opnt = index(line, '&')){
		if(*(opnt-1) != '?' && *(opnt-1) != '*')
			opnt = 0;
	}
	symset();
	while((symp=symget())){
		if((symp->symf&SYMCHK)==symp->symf)
		   if(eqsym(symp->symc,symstr,opnt?'~':'_')){
			return(symp);
		   }
	}
	return(0);
}

hexdigit(c)
char c;
{	
	return((c>='0' && c<='9') || (c>='a' && c<='f'));
}

convdig(c)
char c;
{
	if(digit(c))
	{
		return(c-'0');
	}
	else if(hexdigit(c))
	{
		return(c-'a'+10);
	}
	else{
		return(17);
	}
}

digit(c) char c;	
{
	return(c>='0' && c<='9');
}

letter(c) char c;	
{
	return(c>='a' && c<='z' || c>='A' && c<='Z');
}

symchar(dig)
{
	if(lastc=='\\')
	{
		readchar(); 
		return((-1));
	}
	return(letter(lastc)||lastc=='_'||((lp-1)==line&&lastc=='~')||dig&&digit(lastc));
}

varchk(name)
{
	if(digit(name))
	{
		return(name-'0');
	}
	if(letter(name))
	{
		return((name&037)-1+10);
	}
	return(-1);
}

chkloc(frame)
long 	frame;
{
	readsym();
	do{
		if(localsym(frame)==0)
		{
			error(BADLOC);
		}
		expv=localval;
	}
	while(!eqsym(symbol.symc,isymbol,'~'));
}

eqsym(s1, s2, c)
register char *s1, *s2;
char 	c;
{
	if(eqstr(s1,s2))
	{
		return((-1));
	}
	else if(*s1==c)
	{
		char 	s3[8];
		register int 	i;

		s3[0]=c;
		for(i=1; i<8; i++){
			s3[i] = *s2++; 
		}
		return(eqstr(s1,s3));
	}
}
