
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/
#
static char *sccsid = "@(#)setup.c 3.0 4/21/86";
/*
 *
 *	UNIX debugger
 *
 *	Modified for overlay text support,
 *	changes flagged by the word OVERLAY
 *
 *	Fred Canter 1/14/83
 *
 */

#include "defs.h"
#include <a.out.h>

MSG		SLOINI;
MSG		BADNAM;
MSG		ADB_BADMAG;
extern MSG	ALTMAP;	/* OVERLAY - alternate mapping */
			/* extern so setcor() knows about it - jsd */

/* OVERLAY - stack frame register offsets */
extern struct reglist reglist[];
int smlsym;
int unxkern;	/* set to one if debuging a non-overlay unix kernel */
#define	OVOFF	1
#define	NOVOFF	0
/* OVERLAY */

/* OVERLAY - added variables */
/* OVERLAY */
int 	maxoff;	/* for alternate (image) mapping */

struct map txtmap;
struct map datmap;
struct symslave *symvec;
int 	wtflag;
int 	fcor;
int 	fsym;
long 	maxfile;
long 	maxstor;
long 	txtsiz;
long 	datsiz;
long 	datbas;
long 	stksiz;
char *errflg;
int 	magic;
long 	symbas;
long 	symnum;
long 	entrypt;
/* OVERLAY - relocated here from setsym() */
TXTHDR		txthdr;
int 	ovhdr[16];
long	ovhdroff[16];

struct nlist nl[] = {
	{ "_waitloc" 	} ,
	{ "_ka6" 	} ,
	{ "_cputype" 	} ,
	{ "_rootdev" 	} ,
	{ "_swapdev" 	} ,
	{ "_pipedev" 	} ,
	{ "_el_dev" 	} ,
	{ "" 	} ,
};

/* OVERLAY */

int 	argcount;
int 	signo;
unsigned 	corhdr[ctob(USIZE)];
/*
 * The stack has been moved to below the user
 * structure in the U block, the old corhdr[]
 * value for endhdr was 512, it is now the size of
 * u_stack, see user.h
 * WARNING !, if the stack size is changed adb
 * must be recompiled.
*/
unsigned	*endhdr = &corhdr[sizeof(((struct user *)0)->u_stack)/2];

char *symfil = "a.out";
char *corfil = "core";

#define TXTHDRSIZ	(sizeof(txthdr))
struct symtab *lastsym, *lookupsym();
int	uovnum,uovpnt, numovly;
long 	ovbase, ovsize,ovsymnum;

setsym()
{
	long 	aval, taval, ovoff;
	int 	indx, relflg, symval, symflg,tsymcnt;
	struct symslave *symptr;
	struct symtab *symp;
	register int 	i;
	long t;

	/* Lets try to find out if this is a Unix kernel */
	tsymcnt = 0;
	fsym=getfile(symfil,1);
	txtmap.ufd=fsym;
	if(read(fsym, txthdr, TXTHDRSIZ)==TXTHDRSIZ)
	{
		magic=txthdr[0];
		if(magic == SA_MAGIC)	/* (0401) - stand alone program */
			magic = 0407;	/* treat as 0407 (really is) */
		if(magic!=0411 && magic!=0410 && magic!=0407 && magic!=0405
		    && magic!=0430 && magic!=0431 && magic!=0450 && magic!=0451)
		{
			magic=0;
		}
		else{
			symnum=txthdr[4]/(sizeof (struct symtab));
			txtsiz=txthdr[1];
			datsiz=txthdr[2];
			if (magic>=0430)
			{
				read(fsym, ovhdr, magic >= 0450 ? 32 : 16);
				ovsize=0;
				if (magic < 0450)
					for (i = 8; i < 16; i++)
						ovhdr[i] = 0;
				for(numovly=0,i=1; i<16; i++){
					ovsize += ovhdr[i]; 
					if(ovhdr[i])
						numovly++;
				}
				ovhdroff[1] = txtsiz;
				for (i=1; i < 15; i++)
					ovhdroff[i+1] = ovhdroff[i] + ovhdr[i];
				symbas=txtsiz+datsiz+ovsize;
				txtmap.b1=0;
				/* OVERLAY txtmap.e1=txtsiz;	*/
				txtmap.f1=TXTHDRSIZ + (magic < 0450 ? 16 : 32);
				txtmap.b2=datbas=((magic==0430||magic==0450)?
				round(txtsiz,TXTRNDSIZ) +
				    round((L_INT)ovhdr[0],TXTRNDSIZ) : 0);
				txtmap.e2=txtmap.b2+datsiz;
				txtmap.f2=txtmap.f1+txtsiz+ovsize;
				/* OVERLAY alternate mapping */
				txtmap.e1=txtmap.f2;
				ovbase = round(txtsiz, TXTRNDSIZ);
			}
			else{
				symbas=txtsiz+datsiz;
				txtmap.b1=0;
				txtmap.e1=(magic==0407?symbas:txtsiz);
				txtmap.f1 = TXTHDRSIZ;
				txtmap.b2=magic==0410?round(txtsiz,TXTRNDSIZ):0;
				switch(magic)
				{
					case 0407:
						datbas = txtsiz + 2;
						break;
					case 0410:
						datbas=round(txtsiz,TXTRNDSIZ);
						break;
					case 0411:
						datbas = 0;
						break;
				}
				txtmap.e2=txtmap.b2+(magic==0407?symbas:datsiz);
				txtmap.f2 = TXTHDRSIZ+(magic==0407?0:txtmap.e1);
			}
			if (magic >= 0450)
				unxkern = 1;
			else if (symnum) {
				nlist(symfil, nl);
				for (i = unxkern = 1; i < 7 ; i++)
					if (nl[i].n_type == 0)
						unxkern = 0;
			} else
				printf("No symbol table for %s\n",symfil);
			entrypt=txthdr[5];
			relflg=txthdr[7];
			if(relflg!=1)
			{
				symbas <<= 1;
			}
			symbas += TXTHDRSIZ;
			if (magic >= 0430)
			{
				symbas += (magic < 0450) ? 16 : 32;
			}
			sizeov = (datbas - ovbase) -1;
			/* set up symvec */
			smlsym = 0;
			ovsymnum = symnum;
			/*
			 * We check for overflow before doing the
			 * sbrk() to make sure we don't get blown away.
			 */
			t = (long)shorten((1+symnum))*sizeof (SYMSLAVE);
			if (t + (unsigned)sbrk(0) > 8192*7L)
				symvec = -1;
			else
				symvec=sbrk((short)t);
			if((symptr=symvec)==-1)
			{
				printf("%s, %s\n",BADNAM,SLOINI);
				symset();
				for(ovsymnum = 0;(symp=symget()) && errflg==0;){
					switch(symp->symf ){
						case 02:
						case 03:
						case 04:
						case 042:
						case 043:
						case 044:
							ovsymnum++;
							break;
						default:
							break;
					}
				}
				symvec=sbrk(shorten((1+ovsymnum))*sizeof (SYMSLAVE));
				if((symptr=symvec) == -1){
					printf("Still %s\n",BADNAM);
					symptr=symvec=sbrk(sizeof (SYMSLAVE));
					smlsym = -1;
				}
				else
					smlsym = 1;
			}
			if(smlsym >= 0 && symnum){
				symset();
				while((symp=symget()) && errflg==0){
					if(smlsym){
						switch(symp->symf ){
							case 02:
							case 03:
							case 04:
							case 042:
							case 043:
							case 044:
								break;
							default:
								tsymcnt++;
								continue;
								break;
						}
					}
					symval=symp->symv; 
					symflg=symp->symf;
					if(smlsym)
						symptr->cntslave = tsymcnt++;
					symptr->valslave=symval;
					symptr->typslave=SYMTYPE(symflg);
					/* OVERLAY - set up slave symbol table with overlay values */
					if(symp->symv == 0140000 &&
					    strncmp("_u",symp->symc,2)==0 &&
					    symp->symf == 041)
					{
						symptr->typslave=DSYM;
					}
					if(symp->symo == 0)
						symptr->osmslave=0L;
					else{
						aval=symp->symv;
						aval &= 0177777;
						if(aval >= ovbase)
							symptr->ovnslave = symp->symo;
						if(unxkern && aval >= ovbase)
						{
							symptr->osmslave = aval;
							if(symp->symo > 1)
							{
								ovoff = 020000;
						/*  MAPPED BUFFER OFFSET */
								ovoff += 020000;
								ovoff += txthdr[2];
								ovoff += txthdr[3];
								ovoff += 077;
								ovoff &= ~077;
								for(indx=2; indx<symp->symo; indx++){
									ovoff += ovhdr[indx]; 
								}
								symptr->osmslave += ovoff;
							}
						}else if(!unxkern && (symp->symf&07)==02){
							aval &= sizeov;
							for(indx = 1; indx < symp->symo; indx++)
								aval += ovhdr[indx];
							symptr->osmslave = txtmap.b2 + aval;
						}
					}
					if(unxkern && symptr->valslave==0140000)
						symptr->osmslave=0140000L;
					/* OVERLAY */
					symptr++;
				}
			}
			symptr->typslave=ESYM;
		}
	}
	if(magic==0)
	{
		txtmap.e1=maxfile;
	}
	if(magic >= 0430 && symnum){
		if(lastsym = lookupsym("__ovno"))
			uovpnt = lastsym->symv;
		else{
			printf("Fatal: Cannot find __ovno\n");
			exit(0);
		}
		lastsym = 0;
	}
}

setcor()
{
	register int 	i;	/* OVERLAY - added variable */
	fcor=getfile(corfil,2);
	datmap.ufd=fcor;
	if(read(fcor, corhdr, ctob(USIZE))==ctob(USIZE))
	{
		txtsiz = ((struct user *)corhdr)->u_tsize << 6;
		datsiz = ((struct user *)corhdr)->u_dsize << 6;
		stksiz = ((struct user *)corhdr)->u_ssize << 6;
		/*
		 * core dump starts at 0 for regular(407) and
		 * split I/D(411|431) files, shared text starts
		 * at the data offset.
		 */
		if (magic == 0407 || magic == 0411 ||
		    magic == 0431 || magic == 0451)
			datmap.b1 = 0;
		else
			datmap.b1 = datbas;
		datmap.e1=(magic==0407?txtsiz:datmap.b1)+datsiz;
		datmap.f1 = ctob(USIZE);
		datmap.b2 = maxstor-stksiz;
		datmap.e2 = maxstor;
		/*
			datmap.f2 = ctob(USIZE)+(magic==0410?datsiz:datmap.e1);
		*/
		datmap.f2 = ctob(USIZE);
		if(magic >= 0410)
			datmap.f2 += datsiz;
		else
			datmap.f2 += datmap.e1;
		if(magic && magic!=((struct user *)corhdr)->u_exdata.ux_mag)
		{
			printf("%s\n",ADB_BADMAG);
			printf("%s\n",ALTMAP);
			/*
			 * OVERLAY
			 * If the core file is not in core(5)
			 * format, then map it as an image.
			 * Possibly physical memory or a core dump.
			 */
			maxoff=02000;	/* for _u symbols */
			datmap.b1=0;
			datmap.e1=maxfile;
			datmap.f1=0;
			datmap.b2=0;
			if (magic == 0430 || magic == 0450) {
				/*
				 * Set / * map to map to 3 text
				 * segments + data + bss + overlays.
				 */
				datmap.e2=060000;
				datmap.e2 += (long)txthdr[2] + txthdr[3]; 
				for(i=2; i<16; i++) {
					datmap.e2 += ovhdr[i]; 
				}
				datmap.f2=0;
			} else if (magic == 0431 || magic == 0451) {
				/*
				 * set the / * map to map to the
				 * text and overlays.
				 */
				datmap.e2 = txthdr[1];
				for(i=2; i<16; i++)
					datmap.e2 += ovhdr[i];
				datmap.f2=0;
				datmap.f2 = (077L+txthdr[2]+txthdr[3]) & ~077L;
			} else {
				datmap.e2 = txthdr[1];
				datmap.f2 = txthdr[2] + txthdr[3];
				datmap.f2 += 077;
				datmap.f2 &= ~077;
			}
		}
		else if(magic >= 0430 && !unxkern)
			uovnum = ((struct user *)corhdr)->u_ovdata.uo_curov;
		/* OVERLAY */
	}
	else{
		datmap.e1 = maxfile;
	}
}

create(f)
char *f;
{	
	int fd;
	if((fd=creat(f,0644))>=0)
	{
		close(fd); 
		return(open(f,wtflag));
	}
	else{
		return(-1);
	}
}

getfile(filnam,cnt)
char *filnam;
{
	register int 	tfsym;

	if(!eqstr("-",filnam))
	{
		tfsym=open(filnam,wtflag);
		if(tfsym<0 && argcount>cnt)
		{
			if(wtflag)
			{
				tfsym=create(filnam);
			}
			if(tfsym<0)
			{
				printf("cannot open `%s'\n", filnam);
			}
		}
	}
	else{
		tfsym = -1;
	}
	return(tfsym);
}

long calcov(adr, wov)
long adr;
{
	return((adr & sizeov) + ovhdroff[wov]);
}
