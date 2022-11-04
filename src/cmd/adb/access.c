
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#
static char *sccsid = "@(#)access.c	3.0	4/21/86";
/*
 *
 *	UNIX debugger
 *
 */

#include "defs.h"

MSG		ODDADR;
MSG		BADDAT;
MSG		BADTXT;
struct map txtmap;
struct map datmap;
struct symtab symbol,*lastsym;
struct symslave *symsp;
int 	wtflag,unxkern,offset;
long	ovbase,ovsize,datbas, dot,symnum, calcov();
char *errflg;
int 	errno;
int 	pid,wantov,cwantov,magic;
long	ovhdroff[16];

/* file handling and access routines */

put(adr,space,value)
long adr;
{
	if(cwantov)
		wantov = cwantov;
	access(WT,adr,space,value);
}

unsigned get(adr, space)
long 	adr;
{
	if(cwantov)
		wantov = cwantov;
	return(access(RD,adr,space,0));
}

unsigned chkget(n, space)
long 	n;
{
	register int 	w;

	w = get(n, space);
	chkerr();
	return(w);
}

access(mode,adr,space,value)
long adr;
{
	int 	w, w1, pmode, rd, file;
	long tadr;

	rd = mode==RD;
	if(space == NSP)
	{
		return(0);
	}
	if(pid		/* tracing on? */)
	{
		if((adr&01) && !rd)
		{
			error(ODDADR);
		}
		pmode = (space&DSP?(rd?RDUSER:WDUSER):(rd?RIUSER:WIUSER));
		pmode |= (wantov << 8);
		w = ptrace(pmode, pid, shorten(adr&~01), value);
		if(adr&01)
		{
			w1 = ptrace(pmode, pid, shorten(adr+1), value);
			w = (w>>8)&0377 | (w1<<8);
		}
		if(errno)
		{
			perror("");
			errflg = (space&DSP ? BADDAT : BADTXT);
		}
		wantov = 0;
		return(w);
	}
	w = 0;
	if(mode==WT && wtflag==0)
	{
		error("not in write mode");
	}
	tadr = adr;
	if (inov(adr)) {
		if ((space & (STAR|ISP)) == ISP) {
			if (wantov)
				tadr = calcov(adr, wantov);
			else if (symnum > 0L)
				tadr = calcov(adr, symsp->ovnslave);
		} else if (unxkern && symnum > 0L && (space&(STAR|DSP)) == (STAR|DSP))
			tadr = (adr & sizeov) + symsp->osmslave;
	}
	wantov = 0;
	if(!chkmap(&tadr,space))
	{
		return(0);
	}
	file=(space&DSP?datmap.ufd:txtmap.ufd);
	if(longseek(file,tadr)==0 ||
	    (rd ? read(file,&w,2) : write(file,&value,2)) < 1)
	{
		errflg=(space&DSP?BADDAT:BADTXT);
	}
	return(w);
}

chkmap(adr,space)
register long *adr;
register int 	space;
{
	register struct map *amap;
	amap=((space&DSP?&datmap:&txtmap));
	if(space&STAR || !within(*adr,amap->b1,amap->e1))
	{
		if(within(*adr,amap->b2,amap->e2))
		{
			*adr += (amap->f2)-(amap->b2);
		}
		else{
			errflg=(space&DSP?BADDAT:BADTXT); 
			return(0);
		}
	}
	else{
		*adr += (amap->f1)-(amap->b1);
	}
	return(1);
}

within(adr,lbd,ubd)
long adr, lbd, ubd;
{
	return(adr>=lbd && adr<ubd);
}

inov(adr)
long adr;
{
	if ((magic == 0430 || magic == 0450) && adr >= ovbase && adr < datbas)
		return(1);
	else if ((magic == 0431 || magic == 0451) && adr >= ovbase &&
							adr < (ovbase+ovsize))
		return(1);
	else
		return(0);
}
