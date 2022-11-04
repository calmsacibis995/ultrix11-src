
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#
static char *sccsid = "@(#)findfn.c 3.0 4/21/86";
/*
 *
 *	UNIX debugger
 *
 */

#include "defs.h"

MSG		NOCFN;

int callpc,uovnum,numovly,wantov,cwantov,unxkern,magic;
long ovbase;
char 	localok;
struct symtab 	symbol;
char *errflg;


findroutine(cframe)
long 	cframe;
{
	int 	narg, lastpc, wantpc, back2, inst;
	int	isov1, ovnum, isov2, ovrtn, rtncnt;
	char 	v;

	v=0; 
	localok=0; 
	lastpc=callpc;
	callpc=get(cframe+2, DSP); 
	if(magic >= 0430 && !unxkern){
		inst = get(cframe-2,DSP);
		if((inst > 7 || inst < 0) && lastpc < shorten(ovbase))
			cwantov = uovnum;
		else
			cwantov = inst;
	}
	back2=get(leng(callpc-2), ISP);
	if((inst=get(leng(callpc-4), ISP)) == 04737 /* jsr pc,*$... */)
	{
		narg = 1;
	}
	else if((inst&~077)==04700		/* jsr pc,... */)
	{
		narg=0; 
		v=(inst!=04767);
	}
	else if((back2&~077)==04700)
	{
		narg=0; 
		v=(-1);
	}
	else{
		goto finderr;
	}
	wantpc = (v ? lastpc : ((inst==04767?callpc:0)+back2));
	if(magic >= 0430 && !unxkern)
	{
		isov1 = get(leng(wantpc), ISP);
		ovnum = get(leng(wantpc+2), ISP);
		isov2 = get(leng(wantpc+4), ISP);
		ovrtn = get(leng(wantpc+016), ISP);
		if(isov1 == 012700 && isov2 == 020037
		  && ovnum <= numovly)
		{
			wantpc = ovrtn;
			wantov = ovnum;
		}
		else
		{
			wantov = cwantov;
		}
	}
	if(findsym(wantpc ,ISYM) == -1 && !v)
	{
		symbol.symc[0] = '?';
		symbol.symc[1] = 0;
		symbol.symv = 0;
	}
	else{
		localok=(-1);
	}
	inst = get(leng(callpc), ISP);
	if(inst == 05726		/* tst (sp)+ */)
	{
/* 
		cwantov = 0;
 */
		return(narg+1);
	}
	if(inst == 022626		/* cmp (sp)+,(sp)+ */)
	{
/*
		cwantov = 0;
 */
		return(narg+2);
	}
	if(inst == 062706		/* add $n,sp */)
	{
		rtncnt = get(leng(callpc+2), ISP);
/*
		cwantov = 0;
 */
		return(narg+rtncnt/2);
	}
/*
	cwantov = 0;
 */
	return(narg);
finderr:
/*
	cwantov = 0;
 */
	errflg=NOCFN;
	return(0);
}

