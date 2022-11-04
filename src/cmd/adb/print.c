
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/
#
static char *sccsid = "@(#)print.c 3.0 4/21/86";
/*
 *
 *	UNIX debugger
 *
 *	Modified for the overlay text support,
 *	changes flagged by the word OVERLAY.
 *
 *	Fred Canter 4/1/82
 *
 */

#include "defs.h"

/* OVERLAY */
long pargp, pframe,tdot,symnum,ovsymnum;
struct symslave *symvec, *symsp;

int uovnum;
int unxkern;
int wantov,cwantov;
extern int magic;
extern long datbas, datsiz;
/* OVERLAY */

MSG		LONGFIL;
MSG		NOTOPEN;
MSG		A68BAD;
MSG		A68LNK;
MSG		BADMOD;
MSG		version;

struct map txtmap;
struct map datmap;

struct symtab 	symbol, *lastsym;
int 	lastframe;
int 	callpc;
int 	infile;
int 	outfile;
char 	*lp;
int 	maxoff;
int 	maxpos;
int 	octal;

/* symbol management */
long 	localval;

/* breakpoints */
struct bkpt *bkpthead;

/*
 * The reglist array contains the name and offset
 * for each element of data on the stack in a core
 * dump of a user process. The offset values are
 * automatically set for an overlay or non-overlay
 * stack frame based on the value of the top
 * location on the stack. This value will be
 * a saved PSW if the core dump occurred while
 * the non-overlay kernel was running, or an overlay
 * number (1-7) if the dump was done with the overlay
 * kernal running.
 * The offset values may also be set with the $i
 * modifier for I & D space kernel or the $n
 * modifier for the non I & D (overlay) kernel.
 */

struct reglist reglist [] = {
	"dev", dev,	/* trap type */
	"ps", ps,
	"pc", pc,
	"sp", sp,
	"r5", r5,
	"r4", r4,
	"r3", r3,
	"r2", r2,
	"r1", r1,
	"r0", r0,
};

/*
 * The order of the frnames array has been changed
 * so that the FP registers will be printed correctly,
 * the origional order was{ 0, 3, 4, 5, 1, 2 }.
 * The frnames array may not be necessary at all,
 * but I am not optimizing, just fixing !
 * See the file print.c.orig for the way it was !
 *	Fred Canter 7/19/81
 */
int 	frnames[] = { 0, 1, 2, 3, 4, 5 };

char		lastc;
unsigned 	corhdr[];
unsigned 	*endhdr;
int 	fcor;
char *errflg;
int 	signo;
long 	dot;
long 	var[];
char *symfil;
char *corfil;
int 	pid;
long 	adrval;
int 	adrflg;
long 	cntval;
int 	cntflg;

/*
 * Text used to print the cause of a user
 * process being core dumpped.
 */
extern char *traptypes[];

extern char *signals[];

/* general printing routines ($) */


printtrace(modif)
{
	int 	narg, i, stat, name, limit,lastov,lastpc,callov;
	unsigned 	dynam;
	register struct bkpt *bkptr;
	char 	hi, lo;
	int 	word;
	char *comptr;
	long 	link;
	struct symtab *symp;

	if(cntflg==0)
	{
		cntval = -1;
	}

	switch (modif){

	case '<':
	case '>':
		{
			char 	file[64];
			int 	index;

			index=0;
			if(modif=='<')
			{
				iclose();
			}
			else{
				oclose();
			}
			if(rdc()!='\n')
			{
				do{
					file[index++]=lastc;
					if(index>=63)
					{
						error(LONGFIL);
					}
				}
				while(readchar()!='\n');

				file[index]=0;
				if(modif=='<')
				{
					infile=open(file,0);
					if(infile<0)
					{
						infile=0; 
						error(NOTOPEN);
					}
				}
				else{
					outfile=open(file,1);
					if(outfile<0)
					{
						outfile=creat(file,0644);
					}
					else{
						lseek(outfile,0L,2);
					}
				}
			}
			lp--;
		}
		break;

	case 'o':
		octal = (-1); 
		break;

	case 'd':
		octal = 0; 
		break;

	case 'q': 
	case 'Q': 
	case '%':
		done();

	case 'w': 
	case 'W':
		maxpos=(adrflg?adrval:MAXPOS);
		break;

	case 's': 
	case 'S':
		maxoff=(adrflg?adrval:MAXOFF);
		break;

	case 'v': 
	case 'V':
		prints(version);
		prints("variables\n");
		for(i=0;i<=35;i++){
			if(var[i])
			{
				printc((i<=9 ? '0' : 'a'-10) + i);
				printf(" = %Q\n",var[i]);
			}
		}
		break;

	case 'm': 
	case 'M':
		printmap("? map",&txtmap);
		printmap("/ map",&datmap);
		break;

	case 0: 
	case '?':
		if(pid)
		{
			printf("pcs id = %d\n",pid);
		}
		else if(fcor <= 0){
			error("no process or core file");
		}
		sigprint(); 
/*
		flushbuf();
*/

	case 'r': 
	case 'R':
		if(!pid && fcor <= 0)
			error("no process or core file");
		printregs();
		return;

	case 'f': 
	case 'F':
		if(!pid && fcor <= 0)
			error("no process or core file");
		printfregs(modif=='F');
		return;

	case 'c': 
	case 'C':
		if(!pid && fcor <= 0)
			error("no process or core file");
		pframe=(adrflg?adrval:endhdr[r5])&EVEN; 
		lastframe=0;
/* 
		lastsym = 0;
 */
		callpc=(adrflg?get(pframe+2,DSP):endhdr[pc]);
		while(cntval--){
			chkerr();
			narg = findroutine(pframe);
			callov = cwantov;
			cwantov = 0;
			lastpc = symbol.symv;
			lastov = symbol.symo;
			printf("%.8s(", symbol.symc);
			pargp = pframe+4;
			if(--narg >= 0)
			{
				printf("%o", get(pargp, DSP));
			}
			while(--narg >= 0){
				pargp += 2;
				printf(",%o", get(pargp, DSP));
			}
			if(symbol.symo)
				printf(") Overlay %o",symbol.symo);
			else
				printf(")");
			printf(" from ");
			wantov = callov;
			psymoff(leng(callpc),ISYM," ");
			printf("\n");
			wantov = lastov;
			findsym(lastpc,ISYM);
			if(modif=='C')
			{
				while(localsym(pframe)){
					word=get(localval,DSP);
					printf("%8t%.8s:%10t", symbol.symc);
					if(errflg)
					{
						prints("?\n"); 
						errflg=0; 
					}
					else{
						printf("%o\n",word);
					}
				}
			}
			lastframe=pframe;
			pframe=get(pframe, DSP)&EVEN;
			if(pframe==0)
			{
				break;
			}
		}
		break;

		/*print externals*/
	case 'e': 
	case 'E':
		symset();
		while((symp=symget())){
			chkerr();
			switch(symp->symf){
				case 04:
				case 044:
					if(fcor <= 0)
						continue;
					break;
				case 03:
				case 043:
					if(fcor <= 0
					  && leng(symp->symv)>=(datbas+datsiz))
						continue;
					break;
				default:
					continue;
					break;
			}
			printf("%.8s:%12t%o\n", symp->symc
			       ,get(leng(symp->symv),fcor>0?DSP:(ISP|STAR)));
		}
		break;

	case 'a': 
	case 'A':
		pframe=(adrflg ? adrval : endhdr[r4]);
		while(cntval--){
			chkerr();
			stat=get(pframe,DSP); 
			dynam=get(pframe+2,DSP); 
			link=get(pframe+4,DSP);
			if(modif=='A')
			{
				printf("%8O:%8t%-8o,%-8o,%-8o",pframe,stat,dynam,link);
			}
			if(stat==1)
			{
				break;
			}
			if(errflg)
			{
				error(A68BAD);
			}
			if(get(link-4,ISP)!=04767)
			{
				if(get(link-2,ISP)!=04775)
				{
					error(A68LNK);
				}
				else{
					/*compute entry point of routine*/
					prints(" ? ");
				}
			}
			else{
				printf("%8t");
				valpr(name=shorten(link)+get(link-2,ISP),ISYM);
				name=get(leng(name-2),ISP);
				printf("%8t\""); 
				limit=8;
				do{
					word=get(leng(name),DSP); 
					name += 2;
					lo=word&0377; 
					hi=(word>>8)&0377;
					printc(lo); 
					printc(hi);
				}
				while(lo && hi && limit--);

				printc('"');
			}
			limit=4; 
			i=6; 
			printf("%24targs:%8t");
			while(limit--){
				printf("%8t%o",get(pframe+i,DSP)); 
				i += 2; 
			}
			printc('\n');
			pframe=dynam;
		}
		errflg=0;
		flushbuf();
		break;

		/*set default c frame*/
		/*print breakpoints*/
	case 'b': 
	case 'B':
		printf("breakpoints\ncount%8tbkpt%24tcommand\n");
		for(bkptr=bkpthead; bkptr; bkptr=bkptr->nxtbkpt){
			if(bkptr->flag)
			{
				printf("%-8.8d",bkptr->count);
				psymoff(leng(bkptr->loc),ISYM,"%24t");
				comptr=bkptr->comm;
				while(*comptr){
					printc(*comptr++); 
				}
			}
		}
		break;

	default: 
		error(BADMOD);
	}
}

printmap(s,amap)
char *s; 
struct map *amap;
{
	int file;
	file=amap->ufd;
	printf("%s%12t`%s'\n",s,(file<0 ? "-" : (file==fcor ? corfil : symfil)));
	printf("b1 = %-16Q",amap->b1);
	printf("e1 = %-16Q",amap->e1);
	printf("f1 = %-16Q",amap->f1);
	printf("\nb2 = %-16Q",amap->b2);
	printf("e2 = %-16Q",amap->e2);
	printf("f2 = %-16Q",amap->f2);
	printc('\n');
}

printfregs(longpr)
{
	register i;
	double f;

	/*
	 * Added code to print the floating error code
	 * and floating error address registers.
	 * The two lines of code "ELSE" & "THEN" are the
	 * same now, because the fpsave routine always saves
	 * the floating registers in double format.
	 * Here again, fixing not optimizing !
	 *	Fred Canter 7/19/81
	 */
	printf("fpsr	%o\n", ((struct user *)corhdr)->u_fps.u_fpsr);
	printf("fpfec	%o\n", ((struct user *)corhdr)->u_fperr.f_fec);
	printf("fpfea	%o\n", ((struct user *)corhdr)->u_fperr.f_fea);
	for(i=0; i<FRMAX; i++){
		if (((struct user *)corhdr)->u_fps.u_fpsr&FD || longpr) {
			/* long mode */
			f = ((struct user *)corhdr)->u_fps.u_fpregs[frnames[i]];
		} else {
			f = ((struct user *)corhdr)->u_fps.u_fpregs[frnames[i]];
		}
		printf("fr%-8d%-32.18f\n", i, f);
	}
}

/*
 * printregs() has been modified to print the trap type
 * along with the registers for core dumped processes.
 */
printregs()
{
	register struct reglist *p;
	int 	v;
	long	tv;

	for(p=reglist; p < &reglist[10]; p++){
		printf("%s%8t%o%8t", p->rname, v=endhdr[p->roffs]);
		tv = (unsigned)v;
		if(p == &reglist[0])
		{
			printf("%s", traptypes[v & 017]);
		}
		else if(p->roffs==pc)
		{
			if(uovnum && inov(leng(tv)))
				wantov = uovnum;
			valpr(v,ISYM);
			if(uovnum && inov(leng(tv)))
				printf(" Overlay #%d", uovnum);
		}
		else
		{
			valpr(v,DSYM);
		}
		printc('\n');
	}
	printpc();
}

getreg(regnam)
{
	register struct reglist *p;
	register char *regptr;
	char 	regnxt;
	regnxt=readchar();
	for(p=reglist; p<=&reglist[9]; p++){
		regptr=p->rname;
		if((regnam == *regptr++) && (regnxt == *regptr))
		{
			return(p->roffs);
		}
	}
	lp--;
	return(0);
}

printpc()
{
	int i;

	tdot=dot=endhdr[pc];
	if(uovnum)
		cwantov = wantov = uovnum;
	if(findsym(shorten(dot),ISYM) <= maxoff){
		lastsym = lookupsym(symbol.symc);
		for(symsp=symvec,i=0;i<(symnum==ovsymnum?symnum:ovsymnum);i++){
			if(lastsym->symv == symsp->valslave
			    && lastsym->symo == symsp->ovnslave)
				break;
			symsp++;
		}
	}
	else{
		lastsym = 0;
		symsp = 0;
	}
	wantov = uovnum;
	psymoff(dot,ISYM,":%16t");
	dot = tdot;
	if(pid && magic >= 0430)
		cwantov = wantov = uovnum;
	printins(0,ISP,chkget(tdot,ISP));
	printc('\n');
	cwantov = 0;
}

sigprint()
{
	prints(signals[signo]);
}

