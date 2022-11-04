
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/
#
static char *sccsid = "@(#)pcs.c 3.0 4/21/86";
/*
 *
 *	UNIX debugger
 *
 */

#include "defs.h"

MSG		NOBKPT;
MSG		SZBKPT;
MSG		EXBKPT;
MSG		NOPCS;
MSG		BADMOD;

/* breakpoints */
struct bkpt *bkpthead;

char 	*lp;
char 	lastc;
unsigned 	corhdr[ctob(USIZE)];
unsigned 	*endhdr;
struct symtab *lastsym;
int magic;

int 	signo;
long 	dot;
int 	pid;
long 	cntval;
long 	loopcnt;


/* sub process control */

subpcs(modif)
{
	register int 	check;
	int 	execsig;
	int 	runmode;
	register struct bkpt *bkptr;
	char *comptr;
	execsig=0; 
	loopcnt=cntval;

	switch(modif){

		/* delete breakpoint */
	case 'd': 
	case 'D':
		if((bkptr=scanbkpt(shorten(dot))))
		{
			bkptr->flag=0; 
			return;
		}
		else{
			error(NOBKPT);
		}

		/* set breakpoint */
	case 'b': 
	case 'B':
		if((bkptr=scanbkpt(shorten(dot))))
		{
			bkptr->flag=0;
		}
		for(bkptr=bkpthead; bkptr; bkptr=bkptr->nxtbkpt){
			if(bkptr->flag == 0)
			{
				break;
			}
		}
		if(bkptr==0)
		{
			if((bkptr=sbrk(sizeof *bkptr)) == -1)
			{
				error(SZBKPT);
			}
			else{
				bkptr->nxtbkpt=bkpthead;
				bkpthead=bkptr;
			}
		}
		bkptr->loc = dot;
		bkptr->ovly = (magic >= 0430?lastsym->symo:0) << 8;
		bkptr->initcnt = bkptr->count = cntval;
		bkptr->flag = BKPTSET;
		check=MAXCOM-1; 
		comptr=bkptr->comm; 
		rdc(); 
		lp--;
		do{
			*comptr++ = readchar();
		}
		while(check-- && lastc!='\n');

		*comptr=0; 
		lp--;
		if(check)
		{
			return;
		}
		else{
			error(EXBKPT);
		}
		/* exit */
	case 'k' :
	case 'K':
		if(pid)
		{
			printf("%d: killed", pid); 
			endpcs(); 
			return;
		}
		error(NOPCS);

		/* run program */
	case 'r': 
	case 'R':
		endpcs();
		setup();
		runmode=CONTIN;
		break;

		/* single step */
	case 's': 
	case 'S':
		runmode=SINGL;	/* was SINGLE, conflict with opset.c */
		if(pid)
		{
			execsig=getsig(signo);
		}
		else{
			setup(); 
			loopcnt--;
		}
		break;

		/* continue with optional signal */
	case 'c': 
	case 'C': 
	case 0:
		if(pid==0)
		{
			error(NOPCS);
		}
		runmode=CONTIN;
		execsig=getsig(signo);
		break;

	default: 
		error(BADMOD);
	}

	if(loopcnt>0 && runpcs(runmode, execsig))
	{
		printf("breakpoint%16t");
	}
	else{
		printf("stopped at%16t");
	}
	delbp();
	printpc();
}

