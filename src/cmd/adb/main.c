
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#
static char *sccsid = "@(#)main.c 3.0 4/21/86";
/*
 *
 *	UNIX debugger
 *
 */

#include "defs.h"

MSG		NLERR;

int 	mkfault;
int 	executing;
int 	infile;
char 	*lp;
int 	maxoff;
int 	maxpos;
int 	sigint, fault();
int 	sigqit;
int 	wtflag;
long 	maxfile;
long 	maxstor;
long 	txtsiz;
long 	datsiz;
long 	datbas;
long 	stksiz;
char *errflg;
int 	exitflg;
int 	magic,wantov,cwantov;
long 	entrypt;
char 	lastc;
int 	eof;
int 	lastcom;
long 	var[36];
char *symfil;
char *corfil;
char 	printbuf[];
char 	*printptr;

long round(a,b)
long 	a, b;
{
	long 	w;
	w = ((a+b-1)/b)*b;
	return(w);
}

/* error handling */

chkerr()
{
	if(errflg || mkfault)
	{
		error(errflg);
	}
}

error(n)
char *n;
{
	wantov = cwantov = 0;
	errflg=n;
	iclose(); 
	oclose();
	longjmp(erradb,1);
}

fault(a)
{
	signal(a,fault);
	lseek(infile,0L,2);
	mkfault++;
}

/* set up files and initial address mappings */
int argcount;

main(argc, argv)
register char **argv;
register int 	argc;
{
	int ttytype;
	char prompt[16];

	sprintf(prompt,"%s> ",argv[0]);
	ttytype = isatty(0);
	maxfile=1L<<24; 
	maxstor=1L<<16;
	gtty(0,&adbtty);
	gtty(0,&usrtty);
	while(argc>1){
		if(eqstr("-w",argv[1]))
		{
			wtflag=2; 
			argc--; 
			argv++;
		}
		else{
			break;
		}
	}
	if(argc>1)
	{
		symfil = argv[1];
	}
	if(argc>2)
	{
		corfil = argv[2];
	}
	argcount=argc;
	/*
	 * maxoff set to 02000 in setcor() if alternate (image)
	 * mapping used, so that adb will correctly relate addresses
	 * in the user structure to (_u + something).
	 */
	maxoff=07777;
	maxpos=MAXPOS;
	setsym(); 
	setcor();

	/* set up variables for user */
	/*	maxoff=MAXOFF; maxpos=MAXPOS;	*/
	var[VARB] = datbas;
	var[VARD] = datsiz;
	var[VARE] = entrypt;
	var[VARM] = magic;
	var[VARS] = stksiz;
	var[VART] = txtsiz;

	if((sigint=signal(SIGINT,01))!=01)
	{
		sigint=fault; 
		signal(SIGINT,fault);
	}
	sigqit=signal(SIGQUIT,1);
	setjmp(erradb);
	if(executing)
	{
		delbp();
	}
	executing=0;


	for(;;){
		flushbuf();
		if(errflg)
		{
			printf("%s\n",errflg);
			exitflg=errflg;
			errflg=0;
		}
		if(mkfault)
		{
			mkfault=0; 
			printc('\n'); 
			prints(DBNAME);
		}
		if (ttytype && !infile) {
			write(0,prompt,strlen(prompt));
		}
		lp=0; 
		rdc(); 
		lp--;
		if(eof)
		{
			if(infile)
			{
				iclose(); 
				eof=0; 
				longjmp(erradb,1);
			}
			else{
				done();
			}
		}
		else{
			exitflg=0;
		}
		command(0,lastcom);
		if(lp && lastc!='\n')
		{
			error(NLERR);
		}
	}
}

done()
{
	endpcs();
	exit(exitflg);
}

