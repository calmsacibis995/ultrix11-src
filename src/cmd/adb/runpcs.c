
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/
#
static char *sccsid = "@(#)runpcs.c 3.0 4/21/86";
/*
 *
 *	UNIX debugger
 *	Modified for overlay text support,
 *	changes flagged by the word OVERLAY.
 *
 *	Fred Canter 4/1/82
 *
 */

#include "defs.h"

MSG		NOFORK;
MSG		ENDPCS;
MSG		BADWAIT;

char 	*lp;
int 	sigint;
int 	sigqit;
int	prcstat;

/* breakpoints */
struct bkpt *bkpthead;

struct reglist 	reglist[];

char 	lastc;
unsigned 	corhdr[];
unsigned 	*endhdr;
int 	fcor;
int 	fsym;
char *errflg;
int 	errno;
int 	signo;
long 	dot;
char *symfil;
int 	wtflag;
int 	pid;
long 	expv;
int 	adrflg;
long 	loopcnt;
extern int	magic;
int 	errno,uovpnt,uovnum,unxkern;

/* service routines for sub process control */

getsig(sig)
{	
	return(expr(0) ? shorten(expv) : sig);
}

int 	userpc=1;

runpcs(runmode, execsig)
{
	int 	rc;
	register struct bkpt *bkpt;
	if(adrflg)
	{
		userpc=shorten(dot);
	}
	setbp();
	printf("%s: running\n", symfil);

	while((loopcnt--)>0){
		stty(0,&usrtty);
		ptrace(runmode,pid,userpc,execsig);
		bpwait(); 
		chkerr(); 
		readregs();

		/*look for bkpt*/
		if(signo==0 && (bkpt=scanbkpt(endhdr[pc]-2)))
		{
			/*stopped at bkpt*/
			userpc=endhdr[pc]=bkpt->loc;
			if(bkpt->flag==BKPTEXEC
			    || ((bkpt->flag=BKPTEXEC, command(bkpt->comm,':')) && --bkpt->count))
			{
				execbkpt(bkpt); 
				execsig=0; 
				loopcnt++;
				userpc=1;
			}
			else{
				bkpt->count=bkpt->initcnt;
				rc=1;
			}
		}
		else{
			rc=0; 
			execsig=signo; 
			userpc=1;
		}
	}
	return(rc);
}

endpcs()
{
	register struct bkpt *bkptr;
	if(pid)
	{
		ptrace(EXIT,pid,0,0); 
		pid=0; 
		userpc=1;
		for(bkptr=bkpthead; bkptr; bkptr=bkptr->nxtbkpt){
			if(bkptr->flag)
			{
				bkptr->flag=BKPTSET;
	
			}
		}
	}
}

setup()
{
/*
	close(fsym); 
	fsym = -1;
*/
	if((pid = fork()) == 0)
	{
		close(fsym); 
		fsym = -1;
		ptrace(SETTRC,0,0,0);
		signal(SIGINT,sigint); 
		signal(SIGQUIT,sigqit);
		doexec(); 
		exit(0);
	}
	else if(pid == -1)
	{
		error(NOFORK);
	}
	else{
		bpwait(); 
		readregs(); 
		lp[0]='\n'; 
		lp[1]=0;
/*
		if(fsym=open(symfil,wtflag) <= 0){
			perror("Cannot open symbol file");
			endpcs();
			error(0);
		}
*/
		if(errflg)
		{
			printf("adb: cannot execute %s\n",symfil);
			endpcs(); 
			error(0);
		}
	}
}

execbkpt(bkptr)
struct bkpt *bkptr;
{	
	int 	bkptloc;
	bkptloc = bkptr->loc;
	ptrace(bkptr->ovly|WIUSER,pid,bkptloc,bkptr->ins);
	stty(0,&usrtty);
	ptrace(bkptr->ovly|SINGL,pid,bkptloc,0);	/* was SINGLE, conflict with opset.c */
	bpwait(); 
	chkerr();
	ptrace(bkptr->ovly|WIUSER,pid,bkptloc,BPT);
	bkptr->flag=BKPTSET;
}

doexec()
{
	char *argl[MAXARG];
	char 	args[LINSIZ];
	char *p, **ap, *filnam;
	ap=argl; 
	p=args;
	*ap++=symfil;
	do{
		if(rdc()=='\n')
		{
			break;
		}
		*ap = p;
		while(lastc!='\n' && lastc!=SP && lastc!=TB){
			*p++=lastc; 
			readchar(); 
		}
		*p++=0; 
		filnam = *ap+1;
		if(**ap=='<')
		{
			close(0);
			if(open(filnam,0)<0)
			{
				printf("%s: cannot open\n",filnam); 
				exit(0);
			}
		}
		else if(**ap=='>')
		{
			close(1);
			if(creat(filnam,0666)<0)
			{
				printf("%s: cannot create\n",filnam); 
				exit(0);
			}
		}
		else{
			ap++;
		}
	}
	while(lastc!='\n');

	*ap++=0;
	if(prcstat = execv(symfil, argl)){
		printf("adb: execute failed for %s\n", symfil);
		perror("adb: return stat = ");
	}
}

struct bkpt *scanbkpt(adr)
{
	register struct bkpt *bkptr;
	for(bkptr=bkpthead; bkptr; bkptr=bkptr->nxtbkpt){
		if(bkptr->flag && bkptr->loc==adr)
		{
			break;
		}
	}
	return(bkptr);
}

delbp()
{
	register int 	a;
	register struct bkpt *bkptr;
	for(bkptr=bkpthead; bkptr; bkptr=bkptr->nxtbkpt){
		if(bkptr->flag)
		{
			a=bkptr->loc;
			ptrace(bkptr->ovly|WIUSER,pid,a,bkptr->ins);
		}
	}
}

setbp()
{
	register int 	a;
	register struct bkpt *bkptr;

	for(bkptr=bkpthead; bkptr; bkptr=bkptr->nxtbkpt){
		if(bkptr->flag)
		{
			a = bkptr->loc;
			bkptr->ins = ptrace(bkptr->ovly|RIUSER, pid, a, 0);
			ptrace(bkptr->ovly|WIUSER, pid, a, BPT);
			if(errno)
			{
				perror("cannot set breakpoint: ");
				psymoff(leng(bkptr->loc),ISYM,"\n");
			}
		}
	}
}

bpwait()
{
	register int w;
	int stat;

	signal(SIGINT, 1);
	while((w = wait(&stat))!=pid && w != -1);

	prcstat = stat;
	signal(SIGINT,sigint);
	gtty(0,&usrtty);
	stty(0,&adbtty);
	if(w == -1)
	{
		pid=0;
		errflg=BADWAIT;
	}
	else if((stat & 0177) != 0177)
	{
		if(signo = stat&0177)
		{
			sigprint();
		}
		if(stat&0200)
		{
			prints(" - core dumped");
			close(fcor);
			setcor();
		}
		pid=0;
		errflg=ENDPCS;
	}
	else{
		signo = stat>>8;
		if(signo!=SIGTRAP)
		{
			sigprint();
		}
		else{
			signo=0;
		}
		flushbuf();
	}
}

readregs()
{
	/*get register values from pcs*/
	register i;
	for(i=0; i<10; i++){
		endhdr[reglist[i].roffs] =
			ptrace(RUREGS, pid, UROFF+2*reglist[i].roffs, 0);
	}

	/* floating point		*/
	for(i=(int)FROFF/2; i<(FRLEN+(int)FROFF)/2; i++){
		corhdr[i] = ptrace(RUREGS,pid,2*i,0); 
	}
	/* grab the floating point error registers */
	for (i=(int)FERROFF/2; i < ((int)FERROFF+FERRLEN)/2; i++)
		corhdr[i] = ptrace(RUREGS, pid, 2*i, 0);
	if(!unxkern && pid > 0 && magic >= 0430 && uovpnt)
		uovnum = ptrace(02,pid,uovpnt,0);
}

