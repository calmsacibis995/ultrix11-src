
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/
#
static char *sccsid = "@(#)opset.c	3.0	4/21/86";
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
struct symtab 	symbol;
struct symslave *symsav;
struct symslave *symvec;
long 	ovsymnum,symnum;
int 	ovitype, nodot;
/* OVERLAY */

extern int unxkern, wantov, magic;
extern long ovbase, datbas;
char *errflg;
long 	dot;
int 	dotinc;
long 	var[];

/* instruction printing */

#define	DOUBLE	0
#define DOUBLW	1
#define	SINGLE	2
#define SINGLW	3
#define	REVERS	4
#define	BRANCH	5
#define	NOADDR	6
#define	DFAULT	7
#define	TRAP	8
#define	SYS	9
#define	SOB	10
#define JMP	11
#define JSR	12

extern struct optab {
	int	mask;
	int	val;
	int	itype;
	char	*iname;
}optab[];

extern struct systab {
	int	argc;
	char	*sname;
	int	argtyp[5];
}systab[];

char *regname[] = {"r0", "r1", "r2", "r3", "r4", "r5", "sp", "pc"};

unsigned type, space, incp;

printins(f,idsp,ins)
register int 	ins;
{
	int 	byte;
	register struct optab *p;

	type=DSYM; 
	space=idsp; 
	incp=2;
	for(p=optab;; p++){
		if((ins & ~p->mask) == p->val)
		{
			break;

		}
	}
	prints(p->iname); 
	byte=ins&0100000; 
	ins &= p->mask;
	switch (p->itype) {
	case JMP:
		type=ISYM;

	case SINGLE:
		if(byte)
		{
			printc('b');
		}
	case SINGLW:
		paddr("%8t",ins);
		break;

	case REVERS:
		doubl(ins&077,(ins>>6)&07);
		break;

	case JSR:
		type=ISYM;

	case DOUBLE:
		if(byte)
		{
			printc('b');
		}
	case DOUBLW:
		doubl(ins>>6,ins);

	case NOADDR:
		break;

	case SOB:
		paddr("%8t",(ins>>6)&07);
		branch(",",-(ins&077));
		break;

	case BRANCH:
		branch("%8t",ins);
		break;

	case SYS:
		{
			int 	indir;
			register int w;
			w = ins & 0200;
			printf("%8t%s", systab[ins &= 0177].sname);
			if (ins==0 && f==0 && idsp!=NSP	) { /* indir */
				w=dot; 
				dot=chkget(inkdot(2),idsp);
				prints(" {");
				/* force looking into ?* map if split I/D */
				if (idsp&ISP &&
				    (magic==0411 || magic==0431 || magic==0451))
					idsp |= STAR;
				indir=get(dot,idsp);
				if(errflg) {
					errflg=0; 
					printc('?');
				} else{
					printins(1,idsp,indir);
				}
				printc('}');
				dot=w; 
				incp=4;
			} else if (!w) {
				w = systab[ins].argc;
				while(w-- && idsp!=NSP){
					prints("; ");
		psymoff(leng(get(inkdot(incp),idsp)), systab[ins].argtyp[w], "");
					incp += 2;
				}
			}
		}
		break;

	case TRAP:
	case DFAULT:
	default:
		printf("%8t%o", ins);
	}
	dotinc=incp;
}

doubl(a,b)
{
	paddr("%8t",a); 
	paddr(",",b);
}

branch(s,ins)
char *s;
register int 	ins;
{
	printf(s);
	if(ins&0200)
	{
		ins |= 0177400;
	}
	ins = shorten(dot) + (ins<<1) + 2;
	psymoff(leng(ins),ISYM,"");
}

paddr(s, a)
char *s;
register int 	a;
{
	register int 	r;
	/* OVERLAY - added variables */
	unsigned 	sval;
	long 	ival;
	long 	oval;
	int 	i;
	/* OVERLAY */

	var[2]=var[1];
	r = a&07; 
	a &= 070;
	printf(s);
	if(r==7 && a&020)
	{
		if(a&010)
		{
			printc('*');
		}
		if(a&040)
		{
			if(space==NSP)
			{
				printc('?');
			}
			else{
				var[1]=chkget(inkdot(incp),space);
				sval=shorten(inkdot(incp+2));
				var[1] += shorten(sval);
				var[1] &= 0177777L;
				nodot = 1;
				psymoff(var[1],(a&010?DSYM:type),"");
			}
		}
		else{
			printc('$');
			if(space==NSP)
			{
				printc('?');
			}
			else{
				var[1]=chkget(inkdot(incp), space);
				nodot = 1;
				psymoff(var[1], (a&010?type:NSYM), "");
			}
		}
		incp += 2;
		return;
	}
	r = regname[r];
	switch (a) {
		/* r */
	case 000:
		prints(r);
		return;

		/* (r) */
	case 010:
		printf("(%s)", r);
		return;

		/* *(r)+ */
	case 030:
		printc('*');

		/* (r)+ */
	case 020:
		printf("(%s)+", r);
		return;

		/* *-(r) */
	case 050:
		printc('*');

		/* -(r) */
	case 040:
		printf("-(%s)", r);
		return;

		/* *x(r) */
	case 070:
		printc('*');

		/* x(r) */
	case 060:
		if(space==NSP)
		{
			printc('?');
		}
		else{
			var[1]=chkget(inkdot(incp), space);
			nodot = 1;
			psymoff(var[1], (a==070?type:NSYM), "");
		}
		incp += 2;
		printf("(%s)", r);
		return;
	}
}
