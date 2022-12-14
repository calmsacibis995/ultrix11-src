
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*	@(#)c10.c	1.2	*/
#
/*

	    	C compiler, part 2


*/

char release[] = "@(#)C rel 2.3; UTS rel 1.3";
static char *Sccsid = "@(#)c10.c	3.0	(ULTRIX)	4/21/86";
#include "c1.h"

#define	dbprint(op)	/* */
#ifdef	DEBUG
#define	dbprint(op)	printf("	/ %s", opntab[op])
#endif

char	maprel[] = {	EQUAL, NEQUAL, GREATEQ, GREAT, LESSEQ,
			LESS, GREATQP, GREATP, LESSEQP, LESSP
};

char	notrel[] = {	NEQUAL, EQUAL, GREAT, GREATEQ, LESS,
			LESSEQ, GREATP, GREATQP, LESSP, LESSEQP
};

struct tconst czero = { CON, INT, 0};
struct tconst cone  = { CON, INT, 1};

struct tname sfuncr = { NAME, STRUCT, STATIC, 0, 0, 0 };

struct	table	*cregtab;

int	nreg	= 3;
int	isn	= 10000;
#ifndef	SYSTEM_V
int	Nflag;
#endif	SYSTEM_V

main(argc, argv)
char *argv[];
{

	if (argc<4) {
		error("Arg count");
		exit(1);
	}
#ifndef	SYSTEM_V
	if (argc>4 && strcmp(argv[4], "-N") == 0)
		Nflag++;
#endif	SYSTEM_V
	if (freopen(argv[1], "r", stdin)==NULL) {
		error("Missing temp file");
		exit(1);
	}
	if ((freopen(argv[3], "w", stdout)) == NULL) {
		error("Can't create %s", argv[3]);
		exit(1);
	}
	funcbase = curbase = coremax = sbrk(0);
	getree();
	/*
	 * If any floating-point instructions
	 * were used, generate a reference that
	 * pulls in the floating-point part of printf.
	 */
	if (nfloat)
		printf(".globl	fltused\n");
	/*
	 * tack on the string file.
	 */
	printf(".globl\n.data\n");
	if (*argv[2] != '-') {
		if (freopen(argv[2], "r", stdin)==NULL) {
			error("Missing temp file");
			exit(1);
		}
		getree();
	}
	if (totspace >= (unsigned)56000) {
		error("Warning: possibly too much data");
		nerror--;
	}
	exit(nerror!=0);
}

/*
 * Given a tree, a code table, and a
 * count of available registers, find the code table
 * for the appropriate operator such that the operands
 * are of the right type and the number of registers
 * required is not too large.
 * Return a ptr to the table entry or 0 if none found.
 */
struct optab *
match(tree, table, nrleft, nocvt)
union tree *tree;
struct table *table;
{
#define	NOCVL	1
#define	NOCVR	2
	int op, d1, d2, dope;
	union tree *p2;
	register union tree *p1;
	register struct optab *opt;

	if (tree==NULL)
		return(NULL);
	if (table==lsptab)
		table = sptab;
	if ((op = tree->t.op)==0)
		return(0);
	dope = opdope[op];
	if ((dope&LEAF) == 0)
		p1 = tree->t.tr1;
	else
		p1 = tree;
	d1 = dcalc(p1, nrleft);
	if ((dope&BINARY)!=0) {
		p2 = tree->t.tr2;
		/*
		 * If a subtree starts off with a conversion operator,
		 * try for a match with the conversion eliminated.
		 * E.g. int = double can be done without generating
		 * the converted int in a register by
		 * movf double,fr0; movfi fr0,int .
		 */
		if (opdope[p2->t.op]&CNVRT && (nocvt&NOCVR)==0
			 && (opdope[p2->t.tr1->t.op]&CNVRT)==0) {
			tree->t.tr2 = p2->t.tr1;
			if (opt = match(tree, table, nrleft, NOCVL))
				return(opt);
			tree->t.tr2 = p2;
		} else if (opdope[p1->t.op]&CNVRT && (nocvt&NOCVL)==0
		 && (opdope[p1->t.tr1->t.op]&CNVRT)==0) {
			tree->t.tr1 = p1->t.tr1;
			if (opt = match(tree, table, nrleft, NOCVR))
				return(opt);
			tree->t.tr1 = p1;
		}
		d2 = dcalc(p2, nrleft);
	}
	for (; table->tabop!=op; table++)
		if (table->tabop==0)
			return(0);
	for (opt = table->tabp; opt->tabdeg1!=0; opt++) {
		if (d1 > (opt->tabdeg1&077)
		 || (opt->tabdeg1 >= 0100 && (p1->t.op != STAR)))
			continue;
		if (notcompat(p1, opt->tabtyp1, opt->tabdeg1, op))
			continue;
		if ((opdope[op]&BINARY)!=0 && p2!=0) {
			if (d2 > (opt->tabdeg2&077)
			 || (opt->tabdeg2 >= 0100) && (p2->t.op != STAR) )
				continue;
			if (notcompat(p2,opt->tabtyp2, opt->tabdeg2, 0))
				continue;
			if ((opt->tabdeg2&077)==20 && xdcalc(p2,nrleft)>20)
				continue;
		}
		return(opt);
	}
	return(0);
}

/*
 * Given a tree, a code table, and a register,
 * produce code to evaluate the tree with the appropriate table.
 * Registers reg and upcan be used.
 * If there is a value, it is desired that it appear in reg.
 * The routine returns the register in which the value actually appears.
 * This routine must work or there is an error.
 * If the table called for is cctab, sptab, or efftab,
 * and tree can't be done using the called-for table,
 * another try is made.
 * If the tree can't be compiled using cctab, regtab is
 * used and a "tst" instruction is produced.
 * If the tree can't be compiled using sptab,
 * regtab is used and the register is pushed on the stack.
 * If the tree can't be compiled using efftab,
 * just use regtab.
 * Regtab must succeed or an "op not found" error results.
 *
 * A number of special cases are recognized, and
 * there is an interaction with the optimizer routines.
 */
rcexpr(atree, atable, reg)
union tree *atree;
struct table *atable;
{
	register r;
	int modf, nargs, recurf;
	register union tree *tree;
	register struct table *table;

	table = atable;
	recurf = 0;
	if (reg<0) {
		recurf++;
		reg = ~reg;
		if (reg>=020) {
			reg -= 020;
			recurf++;
		}
	}
again:
	if((tree=atree)==0)
		return(0);
	if (opdope[tree->t.op]&RELAT && tree->t.tr2->t.op==CON
	    && tree->t.tr2->c.value==0
	    && table==cctab)
		tree = atree = tree->t.tr1;
	/*
	 * fieldselect(...) : in efftab mode,
	 * ignore the select, otherwise
	 * do the shift and mask.
	 */
	if (tree->t.op == FSELT) {
		if (table==efftab)
			atree = tree = tree->t.tr1;
		else {
			tree->t.op = FSEL;
			atree = tree = optim(tree);
		}
	}
	switch (tree->t.op)  {

	/*
	 * Structure assignments
	 */
	case STRASG:
		strasg(tree);
		return(0);

	/*
	 * An initializing expression
	 */
	case INIT:
		tree = optim(tree);
		doinit(tree->t.type, tree->t.tr1);
		return(0);

	/*
	 * Put the value of an expression in r0,
	 * for a switch or a return
	 */
	case RFORCE:
		tree = tree->t.tr1;
		if((r=rcexpr(tree, regtab, reg)) != 0)
			movreg(r, 0, tree);
		return(0);

	/*
	 * sequential execution
	 */
	case SEQNC:
		r = nstack;
		rcexpr(tree->t.tr1, efftab, reg);
		nstack = r;
		atree = tree = tree->t.tr2;
		goto again;

	/*
	 * In the generated &~ operator,
	 * fiddle things so a PDP-11 "bit"
	 * instruction will be produced when cctab is used.
	 */
	case ANDN:
		if (table==cctab) {
			tree->t.op = TAND;
			tree->t.tr2 = optim(tnode(COMPL, tree->t.type, tree->t.tr2, TNULL));
		}
		break;

	/*
	 * Handle a subroutine call. It has to be done
	 * here because if cexpr got called twice, the
	 * arguments might be compiled twice.
	 * There is also some fiddling so the
	 * first argument, in favorable circumstances,
	 * goes to (sp) instead of -(sp), reducing
	 * the amount of stack-popping.
	 */
	case CALL:
		r = 0;
		nargs = 0;
		modf = 0;
		if (tree->t.tr1->t.op!=NAME || tree->t.tr1->n.class!=EXTERN) {
			nargs++;
			nstack++;
		}
		tree = tree->t.tr2;
		if(tree->t.op) {
			while (tree->t.op==COMMA) {
				r += comarg(tree->t.tr2, &modf);
				tree = tree->t.tr1;
				nargs++;
			}
			r += comarg(tree, &modf);
			nargs++;
		}
		tree = atree;
		tree->t.op = CALL2;
		if (modf && tree->t.tr1->t.op==NAME
		   && tree->t.tr1->n.class==EXTERN)
			tree->t.op = CALL1;
		if (cexpr(tree, regtab, reg)<0)
			error("compiler botch: call");
		popstk(r);
		nstack -= nargs;
		if (table==efftab || table==regtab)
			return(0);
		r = 0;
		goto fixup;

	/*
	 * Longs need special treatment.
	 */
	case ASLSH:
	case LSHIFT:
		if (tree->t.type==LONG) {
			if (tree->t.tr2->t.op==ITOL)
				tree->t.tr2 = tree->t.tr2->t.tr1;
			else
				tree->t.tr2 = optim(tnode(LTOI,INT,tree->t.tr2,TNULL));
			if (tree->t.op==ASLSH)
				tree->t.op = ASLSHL;
			else
				tree->t.op = LLSHIFT;
		}
		break;

	/*
	 * Try to change * to shift.
	 */
	case TIMES:
	case ASTIMES:
		tree = pow2(tree);
	}
	/*
	 * Try to find postfix ++ and -- operators that can be
	 * pulled out and done after the rest of the expression
	 */
	if (table!=cctab && table!=cregtab && recurf<2
	 && (opdope[tree->t.op]&LEAF)==0) {
		if (r=delay(&atree, table, reg)) {
			tree = atree;
			table = efftab;
			reg = r-1;
		}
	}
	/*
	 * Basically, try to reorder the computation
	 * so  reg = x+y  is done as  reg = x; reg += y
	 */
	if (recurf==0 && reorder(&atree, table, reg)) {
		if (table==cctab && atree->t.op==NAME)
			return(reg);
	}
	tree = atree;
	if (table==efftab && tree->t.op==NAME)
		return(reg);
	if ((r=cexpr(tree, table, reg))>=0) {
		if (table==cregtab && (tree->t.op==INCAFT
		    || tree->t.op==DECAFT || tree->t.op==TIMES))
			goto fixup;
		return(r);
	}
	if (table!=regtab && (table!=cctab||(opdope[tree->t.op]&RELAT)==0)) {
		if((r=cexpr(tree, regtab, reg))>=0) {
	fixup:
			modf = isfloat(tree);
			dbprint(tree->t.op);
			if (table==sptab || table==lsptab) {
				if (tree->t.type==LONG) {
					printf("mov\tr%d,-(sp)\n",r+1);
					nstack++;
				}
				printf("mov%s	r%d,%s(sp)\n", modf=='f'?"f":"", r,
					table==sptab? "-":"");
				nstack++;
			}
			if (table==cctab || table==cregtab)
				printf("tst%s	r%d\n", modf=='f'?"f":"", r);
			return(r);
		}
	}
	/*
	 * Special grace for unsigned chars as right operands
	 */
	if (opdope[tree->t.op]&BINARY && tree->t.tr2->t.type==UNCHAR) {
		tree->t.tr2 = tnode(LOAD, UNSIGN, tree->t.tr2, TNULL);
		return(rcexpr(tree, table, reg));
	}
	/*
	 * There's a last chance for this operator
	 */
	if (tree->t.op==LTOI) {
		r = rcexpr(tree->t.tr1, regtab, reg);
		if (r >= 0) {
			r++;
			goto fixup;
		}
	}
	if (tree->t.type == STRUCT)
		error("Illegal operation on structure");
	else if (tree->t.op>0 && tree->t.op<RFORCE && opntab[tree->t.op])
		error("No code table for op: %s", opntab[tree->t.op]);
	else
		error("No code table for op %d", tree->t.op);
	return(reg);
}

/*
 * Try to compile the tree with the code table using
 * registers areg and up.  If successful,
 * return the register where the value actually ended up.
 * If unsuccessful, return -1.
 *
 * Most of the work is the macro-expansion of the
 * code table.
 */
cexpr(tree, table, areg)
register union tree *tree;
struct table *table;
{
	int c, r;
	register union tree *p, *p1;
	struct table *ctable;
	union tree *p2;
	char *string;
	int reg, reg1, rreg, flag, opd;
	struct optab *opt;

	reg = areg;
	p1 = tree->t.tr2;
	c = tree->t.op;
	opd = opdope[c];
	/*
	 * When the value of a relational or a logical expression is
	 * desired, more work must be done.
	 */
	if ((opd&RELAT||c==LOGAND||c==LOGOR||c==EXCLA) && table!=cctab) {
		cbranch(tree, c=isn++, 1, reg);
		rcexpr((union tree *)&czero, table, reg);
		branch(isn, 0, 0);
		label(c);
		rcexpr((union tree *)&cone, table, reg);
		label(isn++);
		return(reg);
	}
	if(c==QUEST) {
		if (table==cctab)
			return(-1);
		cbranch(tree->t.tr1, c=isn++, 0, reg);
		flag = nstack;
		rreg = rcexpr(p1->t.tr1, table, reg);
		nstack = flag;
		branch(r=isn++, 0, 0);
		label(c);
		reg = rcexpr(p1->t.tr2, table, rreg);
		if (rreg!=reg)
			movreg(reg, rreg, tree->t.tr2);
		label(r);
		return(rreg);
	}
	reg = oddreg(tree, reg);
	reg1 = reg+1;
	/*
	 * long values take 2 registers.
	 */
	if ((tree->t.type==LONG||opd&RELAT&&tree->t.tr1->t.type==LONG)
	   && tree->t.op!=ITOL)
		reg1++;
	/*
	 * Leaves of the expression tree
	 */
	if ((r = chkleaf(tree, table, reg)) >= 0)
		return(r);
	/*
	 * x + (-1) is better done as x-1.
	 */
	if (tree->t.op==PLUS||tree->t.op==ASPLUS) {
		if ((p1=tree->t.tr2)->t.op==CON && p1->c.value==-1) {
			p1->c.value = -p1->c.value;
			tree->t.op += (MINUS-PLUS);
		}
	}
	/*
	 * Because of a peculiarity of the PDP11 table
	 * char = *intreg++ and *--intreg cannot go through.
 	 */
	if ((tree->t.tr2->t.op==AUTOI||tree->t.tr2->t.op==AUTOD)
	 && (tree->t.tr1->t.type==CHAR || tree->t.tr1->t.type==UNCHAR)
	 && tree->t.tr2->t.type!=CHAR && tree->t.tr2->t.type!=UNCHAR)
		tree->t.tr2 = tnode(LOAD, tree->t.tr2->t.type, tree->t.tr2, TNULL);
	if (table==cregtab)
		table = regtab;
	/*
	 * The following peculiar code depends on the fact that
	 * if you just want the codition codes set, efftab
	 * will generate the right code unless the operator is
	 * a shift or
	 * postfix ++ or --. Unravelled, if the table is
	 * cctab and the operator is not special, try first
	 * for efftab;  if the table isn't, if the operator is,
	 * or the first match fails, try to match
	 * with the table actually asked for.
	 */
	/*
	 * Account for longs and oddregs; below is really
	 * r = nreg - reg - (reg-areg) - (reg1-reg-1);
	 */
	r = nreg - reg + areg - reg1 + 1;
	if (table!=cctab || c==INCAFT || c==DECAFT || tree->t.type==LONG
	 || c==ASRSH || c==ASLSH || c==ASULSH
	 || (opt = match(tree, efftab, r, 0)) == 0)
		if ((opt=match(tree, table, r, 0))==0)
			return(-1);
	string = opt->tabstring;
	p1 = tree->t.tr1;
	if (p1->t.op==FCON && p1->f.value>0) {
		printf(".data\nL%d:%o;%o;%o;%o\n.text\n", p1->f.value, p1->f.fvalue);
		p1->c.value = -p1->c.value;
	}
	p2 = 0;
	if (opdope[tree->t.op]&BINARY) {
		p2 = tree->t.tr2;
		if (p2->t.op==FCON && p2->f.value>0) {
			printf(".data\nL%d:%o;%o;%o;%o\n.text\n",
			   p2->f.value, p2->f.fvalue);
			p2->f.value = -p2->f.value;
		}
	}
loop:
	/*
	 * The 0200 bit asks for a tab.
	 */
	if ((c = *string++) & 0200) {
		c &= 0177;
		putchar('\t');
	}
	switch (c) {

	case '\n':
		dbprint(tree->t.op);
		break;

	case '\0':
		if (!isfloat(tree))
			if (tree->t.op==DIVIDE||tree->t.op==ASDIV)
				reg--;
		if (table==regtab && (tree->t.op==ASSIGN || opdope[tree->t.op]&ASSGOP)) {
			if (tree->t.tr1->t.type==CHAR)
				printf("movb	r%d,r%d\n", reg, reg);
			else if (tree->t.tr1->t.type==UNCHAR)
				printf("bic	$!0377,r%d\n", reg);
		}
		return(reg);

	/* A1 */
	case 'A':
		p = p1;
		goto adr;

	/* A2 */
	case 'B':
		p = p2;
		goto adr;

	adr:
		c = 0;
		while (*string=='\'') {
			c++;
			string++;
		}
		if (*string=='+') {
			c = 100;
			string++;
		}
		pname(p, c);
		goto loop;

	/* I */
	case 'M':
		if ((c = *string)=='\'')
			string++;
		else
			c = 0;
		prins(tree->t.op, c, instab);
		goto loop;

	/* B1 */
	case 'C':
		if ((opd&LEAF) != 0)
			p = tree;
		else
			p = p1;
		goto pbyte;

	/* BF */
	case 'P':
		p = tree;
		goto pb1;

	/* B2 */
	case 'D':
		p = p2;
	pbyte:
		if (p->t.type==CHAR || p->t.type==UNCHAR)
			putchar('b');
	pb1:
		if (isfloat(p))
			putchar('f');
		goto loop;

	/* BE */
	case 'L':
		if (p1->t.type==CHAR || p2->t.type==CHAR
		 || p1->t.type==UNCHAR || p2->t.type==UNCHAR)
			putchar('b');
		p = tree;
		goto pb1;

	/* F */
	case 'G':
		p = p1;
		flag = 01;
		goto subtre;

	/* S */
	case 'K':
		p = p2;
		flag = 02;
		goto subtre;

	/* H */
	case 'H':
		p = tree;
		flag = 04;

	subtre:
		ctable = regtab;
		if (flag&04)
			ctable = cregtab;
		c = *string++ - 'A';
		if (*string=='!') {
			string++;
			c |= 020;	/* force right register */
		}
		if (*string=='?') {
			string++;
			c |= 040;	/* force condition codes */
		}
		if ((c&02)!=0)
			ctable = sptab;
		if ((c&04)!=0)
			ctable = cctab;
		if ((flag&01) && ctable==regtab && (c&01)==0
		  && ((c&040)||tree->t.op==DIVIDE||tree->t.op==MOD
		   || tree->t.op==ASDIV||tree->t.op==ASMOD||tree->t.op==ITOL))
			ctable = cregtab;
		if ((c&01)!=0) {
			p = p->t.tr1;
			if(collcon(p) && ctable!=sptab) {
				if (p->t.op==STAR)
					p = p->t.tr1;
				p = p->t.tr1;
			}
		}
		if (table==lsptab && ctable==sptab)
			ctable = lsptab;
		if (c&010)
			r = reg1;
		else
			if (opdope[p->t.op]&LEAF || p->t.degree < 2)
				r = reg;
			else
				r = areg;
		rreg = rcexpr(p, ctable, r);
		if (ctable!=regtab && ctable!=cregtab)
			goto loop;
		if (c&010) {
			if (c&020 && rreg!=reg1)
				movreg(rreg, reg1, p);
			else
				reg1 = rreg;
		} else if (rreg!=reg)
			if ((c&020)==0 && oddreg(tree, 0)==0 && tree->t.type!=LONG
			&& (flag&04
			  || flag&01&&xdcalc(p2,nreg-rreg-1)<=(opt->tabdeg2&077)
			  || flag&02&&xdcalc(p1,nreg-rreg-1)<=(opt->tabdeg1&077))) {
				reg = rreg;
				reg1 = rreg+1;
			} else
				movreg(rreg, reg, p);
		goto loop;

	/* R */
	case 'I':
		r = reg;
		if (*string=='-') {
			string++;
			r--;
		}
		goto preg;

	/* R1 */
	case 'J':
		r = reg1;
	preg:
		if (*string=='+') {
			string++;
			r++;
		}
		if (r>nreg || r>=4 && tree->t.type==DOUBLE) {
			if (regpanic)
				error("Register overflow: simplify expression");
			else
				longjmp(jmpbuf, 1);
		}
		printf("r%d", r);
		goto loop;

	case '-':		/* check -(sp) */
		if (*string=='(') {
			nstack++;
			if (table!=lsptab)
				putchar('-');
			goto loop;
		}
		break;

	case ')':		/* check (sp)+ */
		putchar(')');
		if (*string=='+')
			nstack--;
		goto loop;

	/* #1 */
	case '#':
		p = p1->t.tr1;
		goto nmbr;

	/* #2 */
	case '"':
		p = p2->t.tr1;

	nmbr:
		if(collcon(p)) {
			if (p->t.op==STAR) {
				printf("*");
				p = p->t.tr1;
			}
			if ((p = p->t.tr2)->t.op == CON) {
				if (p->c.value)
					psoct(p->c.value);
			} else if (p->t.op==AMPER)
				pname(p->t.tr1, 0);
		}
		goto loop;

	/*
	 * Certain adjustments for / %
	 */
	case 'T':
		c = reg-1;
		if (uns(p1) || uns(p2)) {
			printf("clr	r%d\n", c);
			goto loop;
		}
		if (dcalc(p1, 5)>12 && !match(p1, cctab, 10, 0))
			printf("tst	r%d\n", reg);
		printf("sxt	r%d\n", c);
		goto loop;

	case 'V':	/* adc sbc, clr, or sxt as required for longs */
		switch(tree->t.op) {
		case PLUS:
		case ASPLUS:
		case INCBEF:
		case INCAFT:
			printf("adc");
			break;

		case MINUS:
		case ASMINUS:
		case NEG:
		case DECBEF:
		case DECAFT:
			printf("sbc");
			break;

		case ASSIGN:
			p = tree->t.tr2;
			goto lcasev;

		case ASDIV:
		case ASMOD:
		case ASULSH:
			p = tree->t.tr1;
		lcasev:
			if (p->t.type!=LONG) {
				if (uns(p) || uns(tree->t.tr2))
					printf("clr");
				else
					printf("sxt");
				goto loop;
			}
		default:
			while ((c = *string++)!='\n' && c!='\0');
			break;
		}
		goto loop;

	/*
	 * Mask used in field assignments
	 */
	case 'Z':
		printf("$%o", tree->F.mask);
		goto loop;

	/*
	 * Relational on long values.
	 * Might bug out early. E.g.,
	 * (long<0) can be determined with only 1 test.
	 */
	case 'X':
		if (xlongrel(*string++ - '0'))
			return(reg);
		goto loop;
	}
	putchar(c);
	goto loop;
}

/*
 * This routine just calls sreorder (below)
 * on the subtrees and then on the tree itself.
 * It returns non-zero if anything changed.
 */
reorder(treep, table, reg)
union tree **treep;
struct table *table;
{
	register r, o;
	register union tree *p;

	p = *treep;
	o = p->t.op;
	if (opdope[o]&LEAF||o==LOGOR||o==LOGAND||o==SEQNC||o==QUEST||o==COLON)
		return(0);
	while(sreorder(&p->t.tr1, regtab, reg, 1))
		;
	if (opdope[o]&BINARY) 
		while(sreorder(&p->t.tr2, regtab, reg, 1))
			;
	r = 0;
	if (table!=cctab)
	while (sreorder(treep, table, reg, 0))
		r++;
	*treep = optim(*treep);
	return(r);
}

/*
 * Basically this routine carries out two kinds of optimization.
 * First, it observes that "x + (reg = y)" where actually
 * the = is any assignment op is better done as "reg=y; x+reg".
 * In this case rcexpr is called to do the first part and the
 * tree is modified so the name of the register
 * replaces the assignment.
 * Moreover, expressions like "reg = x+y" are best done as
 * "reg = x; reg += y" (so long as "reg" and "y" are not the same!).
 */
sreorder(treep, table, reg, recurf)
union tree **treep;
struct table *table;
{
	register union tree *p, *p1;

	p = *treep;
	if (opdope[p->t.op]&LEAF)
		return(0);
	if (p->t.op==PLUS && recurf)
		if (reorder(&p->t.tr2, table, reg))
			*treep = p = optim(p);
	p1 = p->t.tr1;
	if (p->t.op==STAR || p->t.op==PLUS) {
		if (recurf && reorder(&p->t.tr1, table, reg))
			*treep = p = optim(p);
		p1 = p->t.tr1;
	}
	if (p1->t.op==NAME) switch(p->t.op) {
		case ASLSH:
		case ASRSH:
		case ASSIGN:
			if (p1->n.class != REG || p1->n.type==CHAR
			  || isfloat(p->t.tr2))
				return(0);
			if (p->t.op==ASSIGN) switch (p->t.tr2->t.op) {
			case TIMES:
				if (!ispow2(p->t.tr2))
					break;
				p->t.tr2 = pow2(p->t.tr2);
			case PLUS:
			case MINUS:
			case AND:
			case ANDN:
			case OR:
			case EXOR:
			case LSHIFT:
			case RSHIFT:
				p1 = p->t.tr2->t.tr2;
				if (xdcalc(p1, 16) > 12
				 || p1->t.op==NAME
				 &&(p1->n.nloc==p->t.tr1->n.nloc
				  || p1->n.regno==p->t.tr1->n.nloc))
					return(0);
				p1 = p->t.tr2;
				p->t.tr2 = p1->t.tr1;
				if (p1->t.tr1->t.op!=NAME
				 || p1->t.tr1->n.class!=REG
				 || p1->t.tr1->n.nloc!=p->t.tr1->n.nloc)
					rcexpr(p, efftab, reg);
				p->t.tr2 = p1->t.tr2;
				p->t.op = p1->t.op + ASPLUS - PLUS;
				*treep = p;
				return(1);
			}
			goto OK;

		case ASTIMES:
			if (!ispow2(p))
				return(0);
		case ASPLUS:
		case ASMINUS:
		case ASAND:
		case ASANDN:
		case ASOR:
		case ASXOR:
		case INCBEF:
		case DECBEF:
		OK:
			if (table==cctab||table==cregtab)
				reg += 020;
			rcexpr(optim(p), efftab, ~reg);
			*treep = p1;
			return(1);
	}
	return(0);
}

/*
 * Delay handles postfix ++ and -- 
 * It observes that "x + y++" is better
 * treated as "x + y; y++".
 * If the operator is ++ or -- itself,
 * it calls rcexpr to load the operand, letting
 * the calling instance of rcexpr to do the
 * ++ using efftab.
 * Otherwise it uses sdelay to search for inc/dec
 * among the operands.
 */
delay(treep, table, reg)
union tree **treep;
struct table *table;
{
	register union tree *p, *p1;
	register r;

	p = *treep;
	if ((p->t.op==INCAFT||p->t.op==DECAFT)
	 && p->t.tr1->t.op==NAME) {
		return(1+rcexpr(paint(p->t.tr1, p->t.type), table, reg));
	}
	p1 = 0;
	if (opdope[p->t.op]&BINARY) {
		if (p->t.op==LOGAND || p->t.op==LOGOR
		 || p->t.op==QUEST || p->t.op==COLON || p->t.op==SEQNC)
			return(0);
		}
		p1 = sdelay(&p->t.tr2);
	if (p1==0)
		p1 = sdelay(&p->t.tr1);
	if (p1) {
		r = rcexpr(optim(p), table, reg);
		*treep = p1;
		return(r+1);
	}
	return(0);
}

union tree *
sdelay(ap)
union tree **ap;
{
	register union tree *p, *p1;

	p = *ap;
	if ((p->t.op==INCAFT||p->t.op==DECAFT) && p->t.tr1->t.op==NAME) {
		*ap = paint(ncopy(p->t.tr1), p->t.type);
		return(p);
	}
	if (p->t.op==STAR || p->t.op==PLUS)
		if (p1=sdelay(&p->t.tr1))
			return(p1);
	if (p->t.op==PLUS)
		return(sdelay(&p->t.tr2));
	return(0);
}

/*
 * Propagate possible implicit type-changing operation
 */
union tree *
paint(tp, type)
register union tree *tp;
register type;
{

	if (tp->t.type==type)
		return(tp);
	if (tp->t.type==CHAR && type==INT)
		return(tp);
#ifndef	fix1
	/* casting a pointer to a pointer is easy... -Dave Borman 11/19/85 */
	if (tp->t.op == AMPER &&
	    !(tp->t.type & ~(XTYPE|TYPE)) && (tp->t.type&XTYPE) == PTR &&
	    !(type & ~(XTYPE|TYPE)) && (type&XTYPE) == PTR) {
		tp->t.type = type;
		return(tp);
	}
	if (tp->t.type==CHAR || tp->t.type==UNCHAR)
#else	fix1
	if ((tp->t.type==CHAR && type != UNCHAR) ||
	    (tp->t.type==UNCHAR && type != CHAR))
#endif	fix1
		return(optim(tnode(LOAD, type, tp, TNULL)));
	tp->t.type = type;
	if (tp->t.op==AMPER && type&XTYPE)
		tp->t.tr1 = paint(tp->t.tr1, decref(type));
	else if (tp->t.op==STAR)
		tp->t.tr1 = paint(tp->t.tr1, incref(type));
	else if (tp->t.op==ASSIGN) {
		paint(tp->t.tr1, type);
		paint(tp->t.tr2, type);
	}
	return(tp);
}

/*
 * Copy a tree node for a register variable.
 * Used by sdelay because if *reg-- is turned
 * into *reg; reg-- the *reg will in turn
 * be changed to some offset class, accidentally
 * modifying the reg--.
 */
union tree *
ncopy(p)
register union tree *p;
{
	register union tree *q;

	q = getblk(sizeof(struct xtname));
	q->n.op = p->n.op;
	q->n.type = p->n.type;
	q->n.class = p->n.class;
	q->n.regno = p->n.regno;
	q->n.offset = p->n.offset;
	if (q->n.class==EXTERN)
		strncpy(q->x.name, p->x.name, NCPS);
	else
		q->n.nloc = p->n.nloc;
	return(q);
}

/*
 * If the tree can be immediately loaded into a register,
 * produce code to do so and return success.
 */
chkleaf(tree, table, reg)
register union tree *tree;
struct table *table;
{
	struct tnode lbuf;

	if (tree->t.op!=STAR && dcalc(tree, nreg-reg) > 12)
		return(-1);
	lbuf.op = LOAD;
	lbuf.type = tree->t.type;
	lbuf.degree = tree->t.degree;
	lbuf.tr1 = tree;
	return(rcexpr((union tree *)&lbuf, table, reg));
}

/*
 * Compile a function argument.
 * If the stack is currently empty, put it in (sp)
 * rather than -(sp); this will save a pop.
 * Return the number of bytes pushed,
 * for future popping.
 */
comarg(tree, flagp)
register union tree *tree;
int *flagp;
{
	register retval;
	int i;
	int size;

	if (tree->t.op==STRASG) {
		size = tree->F.mask;
		tree = tree->t.tr1;
		tree = strfunc(tree);
		if (size <= sizeof(int)) {
			paint(tree, INT);
			goto normal;
		}
		if (size <= sizeof(long)) {
			paint(tree, LONG);
			goto normal;
		}
		if (tree->t.op!=NAME && tree->t.op!=STAR) {
			error("Unimplemented structure assignment");
			return(0);
		}
		tree = tnode(AMPER, STRUCT+PTR, tree, TNULL);
		tree = tnode(PLUS, STRUCT+PTR, tree, tconst(size, INT));
		tree = optim(tree);
		retval = rcexpr(tree, regtab, 0);
		size >>= 1;
		if (size <= 5) {
			for (i=0; i<size; i++)
				printf("mov	-(r%d),-(sp)\n", retval);
		} else {
			if (retval!=0)
				printf("mov	r%d,r0\n", retval);
			printf("mov	$%o,r1\n", size);
			printf("L%d:mov	-(r0),-(sp)\ndec\tr1\njne\tL%d\n", isn, isn);
			isn++;
		}
		nstack++;
		return(size*2);
	}
normal:
	if (nstack || isfloat(tree) || tree->t.type==LONG) {
		rcexpr(tree, sptab, 0);
		retval = arlength(tree->t.type);
	} else {
		(*flagp)++;
		rcexpr(tree, lsptab, 0);
		retval = 0;
	}
	return(retval);
}

union tree *
strfunc(tp)
register union tree *tp;
{
	if (tp->t.op != CALL)
		return(tp);
	paint(tp, STRUCT+PTR);
	return(tnode(STAR, STRUCT, tp, TNULL));
}

/*
 * Compile an initializing expression
 */
doinit(type, tree)
register type;
register union tree *tree;
{
	float sfval;
	double fval;
	long lval;

	if (type==CHAR || type==UNCHAR) {
		printf(".byte ");
		if (tree->t.type&XTYPE)
			goto illinit;
		type = INT;
	}
	if (type&XTYPE)
		type = INT;
	switch (type) {
	case INT:
	case UNSIGN:
		if (tree->t.op==FTOI) {
			if (tree->t.tr1->t.op!=FCON && tree->t.tr1->t.op!=SFCON)
				goto illinit;
			tree = tree->t.tr1;
			tree->c.value = tree->f.fvalue;
			tree->t.op = CON;
		} else if (tree->t.op==LTOI) {
			if (tree->t.tr1->t.op!=LCON)
				goto illinit;
			tree = tree->t.tr1;
			lval = tree->l.lvalue;
			tree->t.op = CON;
			tree->c.value = lval;
		}
		if (tree->t.op == CON)
			printf("%o\n", tree->c.value);
		else if (tree->t.op==AMPER) {
			pname(tree->t.tr1, 0);
			putchar('\n');
		} else
			goto illinit;
		return;

	case DOUBLE:
	case FLOAT:
		if (tree->t.op==ITOF) {
			if (tree->t.tr1->t.op==CON) {
				fval = tree->t.tr1->c.value;
			} else
				goto illinit;
		} else if (tree->t.op==FCON || tree->t.op==SFCON)
			fval = tree->f.fvalue;
		else if (tree->t.op==LTOF) {
			if (tree->t.tr1->t.op!=LCON)
				goto illinit;
			fval = tree->t.tr1->l.lvalue;
		} else
			goto illinit;
		if (type==FLOAT) {
			sfval = fval;
			printf("%o; %o\n", sfval);
		} else
			printf("%o; %o; %o; %o\n", fval);
		return;

	case LONG:
		if (tree->t.op==FTOL) {
			tree = tree->t.tr1;
			if (tree->t.op==SFCON)
				tree->t.op = FCON;
			if (tree->t.op!= FCON)
				goto illinit;
			lval = tree->f.fvalue;
		} else if (tree->t.op==ITOL) {
			if (tree->t.tr1->t.op != CON)
				goto illinit;
			if (uns(tree->t.tr1))
				lval = (unsigned)tree->t.tr1->c.value;
			else
				lval = tree->t.tr1->c.value;
		} else if (tree->t.op==LCON)
			lval = tree->l.lvalue;
		else
			goto illinit;
		printf("%o; %o\n", lval);
		return;
	}
illinit:
	error("Illegal initialization");
}

movreg(r0, r1, tree)
union tree *tree;
{
	register char *s;
	char c;

	if (r0==r1)
		return;
	if (tree->t.type==LONG) {
		s = "mov	r%d,r%d\nmov	r%d,r%d\n";
		if (r0 < r1)
			printf(s, r0+1,r1+1,r0,r1);
		else
			printf(s, r0,r1,r0+1,r1+1);
		return;
	}
	c = isfloat(tree);
	printf("mov%.1s	r%d,r%d\n", &c, r0, r1);
}
