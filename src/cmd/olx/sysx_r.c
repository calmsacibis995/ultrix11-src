
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)sysx_r.c	3.0	4/22/86";
/*
 * ULTRIX-11 system exerciser control program (sysx).
 *
 * Part III - running exerciser scripts
 *
 *
 * Fred Canter 12/18/83
 * Bill Burns 4/84
 *	added check for rx50 for rax
 * 	added eventflag usage
 *
 *
 * 1.	Start/stop execution of exercisers.
 *
 * 2.	Print exerciser log files.
 *
 */

#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <a.out.h>


char	*gs_more = "\nPress <RETURN> for more:";
char	*gs_prtc = "\nPress <RETURN> to continue:";

char *h_z[] =
{
	"",
	"The `!' is used to escape from the sysx program back to ",
	"the ULTRIX-11 command interpreter (shell), execute any of ",
	"the ULTRIX-11 commands, and then return to the sysx program. ",
	"The `!' feature may only be used when sysx is ready to ",
	"accept a command, i.e., in response to the `>' and `run>' ",
	"prompts. The `!' may be used, while the exercisers are running, ",
	"to execute ULTRIX-11 commands like ps, pstat, and iostat ",
	"to monitor system operation. The usage of the `!' is as ",
	"follows:",
	"",
	"	! command",
	"",
	"where command is any of the ULTRIX-11 commands. For a ",
	"description of the ULTRIX-11 commands refer to section ",
	"one of the \"ULTRIX-11 Programmers Manual\". For example:",
	"",
	"	! ls -l",
	"",
	"would produce a listing of all the files in the current ",
	"directory.",
	"",
	0,
};
char *h_h[] =
{
	"",
	"The `run>' prompt indicates that SYSX is running a script",
	"and is ready to accept a limited number of commands.",
	"Except for <CTRL/D> and <CTRL/C>, commands are executed ",
	"by typing the command letter followed by a <RETURN>. ",
	"The commands will ask for additional information, such as ",
	"script name, if required.  For more help type `h' followed ",
	"by the command, `h r' for help restart.",
	"",
	"Command		Description",
	"",
	"<CTRL/D>	Exit from the sysx program.",
	"<CTRL/C>	Cancel current command and return to the `run>' ",
	"			prompt.",
	"",
	"! command	Execute an ULTRIX-11 command.",
	"p		Print the status of the currently running ",
	"			script.",
	"r		Restart exerciser(s).",
	"s		Stop exerciser(s).",
	"l		Print log files on the terminal or line ",
	"			printer.",
	"",
	0,
};
char *h_l[] =
{
	"",
	"The `l' command is used to print the contents of exerciser",
	"log files on the terminal or the line printer.",
	"",
	"When asked for `which log files', respond with:",
	"1.	A <RETURN> to print `all' existing log files.",
	"	There could be very many log files in existence !",
	"",
	"2.	An exerciser name, to print all log files for that ",
	"	exerciser.  There can be many log files associated ",
	"	with an exerciser, for example there could be 20 ",
	"	copies of CPX running, so there would be 20 log files.",
	"",
	"3.	A specific log file name, obtained from the exerciser ",
	"	script via the `p' command. For example, `cpx_2.log' ",
	"	would print only the log file for the second copy of ",
	"	CPX.",
	"",
	-1,
	"",
	"When asked `Output on ', answer:",
	"1.	Yes for output on the line printer.",
	"	Answering yes when no line printer exists will cause",
	"	line printer spool files to be created and waste ",
	"	disk space.",
	"",
	"2.	No or <RETURN> for output on the terminal.",
	"",
	0,
};
char *h_p[] =
{
	"",
	"The `p' command prints the contents of the currently running ",
	"script. To execute the `p' command, type `p' followed ",
	"by a <RETURN> after the `run>' prompt. The printout consists of; ",
	"the exerciser number, the name of the exerciser, the state of the ",
	"exerciser (run/stop), and the options specified for the ",
	"exerciser. ",
	"",
	0,
};
char *h_s[] =
{
	"",
	"The `s' command is used to stop the execution of one, all, ",
	"or a group of exercisers. Answer the `Stop which exercisers' ",
	"question with one of the following three responses:",
	"",
	"1.  `a', `all', or only a <RETURN>, to stop all exercisers. ",
	"",
	"2.  The name of an exerciser, such as `cpx', to stop all ",
	"    running copies of that exerciser.",
	"",
	"3.  The number of the exerciser to stop. The exerciser",
	"    number is based upon the exerciser's position in the",
	"    script. Use the `p' command to list the script.",
	"",
	"SYSX waits for the specified exerciser(s) to stop and then returns ",
	"to either the `run>' prompt (if there are still exercisers running) ",
	"or the `>' prompt (if no exercisers are running). If the message ",
	"\"`exerciser' did not stop\" is printed; stop all exercisers, and ",
	"then restart the exerciser script.",
	"",
	0,
};
char *h_r[] =
{
	"",
	"The `r' command is used to restart the execution of one, all, ",
	"or a group of exercisers. Answer the `Restart which exercisers' ",
	"question with one of the following three responses:",
	"",
	"1.  `a', `all', or only a <RETURN>, to restart all stopped",
	"    exercisers.",
	"",
	"2.  The name of an exerciser, such as `cpx', to restart all ",
	"    stopped copies of that exerciser.",
	"",
	"3.  The number of the exerciser to restart. The exerciser",
	"    number is based upon the exerciser's position in the",
	"    script. Use the `p' command to list the script.",
	"",
	"SYSX waits for the named exerciser(s) to start then returns ",
	"to the `run>' prompt.  If the message \"`exerciser' did not start\" ",
	"is printed; stop all exercisers, then restart the exerciser script.",
	"",
	0,
};
char *h_alf[] =
{
	"",
	"Answering yes will cause any restarted exercisers to append",
	"their output to already existing logfiles. Answering no will",
	"cause the exerciser to recreate the logfile.",
	"",
	0,
};

struct hsub {
	char	*hs_name;
	char	**hs_msg;
} hsub[] = {
	"z",		&h_z,		/* help for "!" */
	"h",		&h_h,		/* general help */
	"l",		&h_l,		/* help for list log files */
	"p",		&h_p,		/* help for print script status */
	"r",		&h_r,		/* help for restart */
	"s",		&h_s,		/* help for stop */
	"alf",		&h_alf,		/* append to logfile help */
	0
};


#define	SBSIZE	8192
#define	YES	1
#define	NO	0
#define HELP	1
#define	NOHELP	0

#define	XRUN	1
#define	XSTOP	0

int	sswait;		/* number of seconds to wait for exer start/stop */
			/* 23 or 24 = 450, micro/pdp-11 = 600, others = 300 */
int	cputype;

/*
 * Below needed for proper handling
 * of rax "maint area" question
 * Bill 4/20/84
 */
#define RX50	50

struct	ra_drv {		/* RA drive information */
	char	ra_dt;		/* RA drive type */
	char	ra_online;	/* RA drive on-line flag */
	union {
		daddr_t	ra_dsize;	/* RA drive size (# of LBN's) */
		struct {
			int	ra_dslo;
			int	ra_dshi;
		};
	} d_un;
} ra_drv[8];			/* size MUST be 8 */

long	evntflg();	/* type returned by event flag syscall */

struct nlist nl[] =
{
	{ "_cputype" },
	{ "_ra_drv" },
	{ "" },
};


/*
 * Exerciser control table.
 * Contains the exerciser run/stop status,
 * its name, and pointers to its kill & log file names.
 */

#ifdef EFLG
#include <sys/eflg.h>
#define	MAXEXR	64	/* maximum number of exercisers */
struct	extab
{
	char	*exn;		/* pointer to exerciser name in sbuf */
	char	*exo;		/* "	options		*/
	char	*exlf;		/* "	log filename	*/
	long	osz;		/* old logfile size - monitoring */
	long	nsz;		/* new logfile size - monitoring */
	int	exs;		/* run/stop status	*/
} exrtab[MAXEXR+1];
#else
#define	MAXEXR	150	/* maximum number of exercisers */
struct	extab
{
	char	*exn;		/* pointer to exerciser name in sbuf */
	char	*exo;		/* "	options		*/
	char	*exkf;		/* "	kill filename	*/
	char	*exlf;		/* "	log filename	*/
	long	osz;		/* old logfile size - monitoring */
	long	nsz;		/* new logfile size - monitoring */
	int	exs;		/* run/stop status	*/
} exrtab[MAXEXR+1];
#endif


char	helpcmd[] = "cat sysx_?.help\0";
char	xrn[20];	/* exerciser name */
char	logfn[20];	/* log file name */

#ifdef EFLG
int	efid[2];	/* eventflag id array */
long	eflg[2];	/* eventflags */
int	nexer;		/* number of exercisers in script */
#else
char	killfn[20];	/* kill file name */
#endif


char	line[140];
char	script[20];	/* name of current script */
char	sbuf[SBSIZE+256];	/* script buffer */

/*
 * Exerciser run flag,
 * 1 = Some or all exercisers are running
 * 0 = No exercisers are running
 *
 * When set, this flag prevents the execution of
 * certain commands and modifies the operation
 * of other commands.
 */

int	xrun;
int	stpflg;
int	logused;
int	alrmflag;
int	alrmcnt;

jmp_buf	savej;

int	buflag;	/* used to cancel a line when creating a script */

struct stat statb;	/* used by logfile montioring */

time_t	timbuf;
time_t	stime1;
time_t	stime2;

int	swiflag;	/* flag to interrupt exer startup wail loop */
char z2[3];
char z3[6];
char z4[6];
int	pgrp = 31111;

main(argc, argv)
int argc;
char *argv[];
{
	int	logmon();
	int	swintr();
	int	intr();
	register int i;
	int	j, k;
	register char *p, *n;
	char	*q;
	struct xtab *xp;
	int	cc, fd;
	int	mtfirst;
	int	ncopy;
	int	logfil;
	int	sst;
	int	cmxon;
	int	yn;
	int	lstall;
	int	savebp;
	int	ker;
	FILE	*fnx;
	int	ncbx, ncix, nbix;
	int	mem;
	int	dn;
#ifdef EFLG
	int	pis;
#endif
	int	lfflag;

	nlist("/unix", nl);
	if(nl[0].n_type == 0) {
		printf("\nsysx: can't access namelist in /unix\n");
		exit(0);
	}
	if((mem = open("/dev/mem", 0)) < 0) {
		printf("\nsysx: can't open /dev/mem\n");
		exit(0);
	}
	lseek(mem, (long)nl[0].n_value, 0);
	read(mem, (char *)&cputype, sizeof(cputype));
	lseek(mem, (long)nl[1].n_value, 0);
	read(mem, (char *)&ra_drv, sizeof(ra_drv));
	close(mem);
	sswait = 300;
	if((cputype == 23) || (cputype == 24))
		sswait += 150;
	if((cputype == 23) && (nl[1].n_type))
		sswait += 150;	/* if ra_drv, assume Micro/pdp-11 */
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	signal(SIGINT, SIG_IGN);
#ifndef EFLG
	system("touch junk.kill");
	system("rm -f *.kill");
#endif
	strcpy(script, argv[1]);
	p = &script;
	fd = open(p, 0);
	p = &sbuf[0];	/* clear script buffer */
	for(i=0; i<SBSIZE+256; i++)
		*p++ = 0;
	if(read(fd, (char *)&sbuf, SBSIZE+256) < 0) {
		printf("\n%s script file read error", p);
		exit(0);
	}
	close(fd);
	setext();	/* set up exerciser control table */
	cmxon = 0;
	ncbx = 0;
	ncix = 0;
	nbix = 0;
	for(i=0; exrtab[i].exn; i++) {
/*
 * Count the number of exercisers in each class.
 */
		if((strcmp("cpx", exrtab[i].exn) == 0)
		  || (strcmp("fpx", exrtab[i].exn) == 0)
		  || (strcmp("memx", exrtab[i].exn) == 0))
			ncbx++;	/* compute bound exer */
		if((strcmp("lpx", exrtab[i].exn) == 0)
		  || (strcmp("cmx", exrtab[i].exn) == 0))
			ncix++;	/* char I/O intensive exer */
		if((strcmp("rlx", exrtab[i].exn) == 0)
		  || (strcmp("rkx", exrtab[i].exn) == 0)
		  || (strcmp("rpx", exrtab[i].exn) == 0)
		  || (strcmp("hkx", exrtab[i].exn) == 0)
		  || (strcmp("hpx", exrtab[i].exn) == 0)
		  || (strcmp("rax", exrtab[i].exn) == 0)
		  || (strcmp("mtx", exrtab[i].exn) == 0))
			nbix++;	/* block I/O intensive exer */
		if(strcmp("cmx", exrtab[i].exn) == 0)
			cmxon++;	/* cmx in use */
		exrtab[i].osz = 0L;	/* initial logfile length */
		if(exrtab[i].exlf)
			logused++;
	}
/*
 * Write the number of exercisers in each class
 * out to the `sysxr.nx' file. This tells the memory
 * exerciser what the load on the system will be,
 * so that it can adjust the number of processes
 * that it runs.
 */
	fnx = fopen("sysxr.nx", "w");
	if(fnx == NULL) {
		printf("\nsysx: Can't open sysxr.nx file !\n");
		exit(0);
	}
	putw(ncbx, fnx);	/* # of compute bound exer */
	putw(ncix, fnx);	/* # of char I/O intensive exer */
	putw(nbix, fnx);	/* # of block I/O intensive exer */
	fclose(fnx);
/*
 * If the comm. mux exerciser (cmx) is to be used,
 * warn about disconnecting customer equipment from
 * dh, dz, and dzv lines !
 */
	if(cmxon) {
		printf("\n\7\7\7Disconnect any customer equipment ");
		printf("that may be affected");
		printf("\nby test data transmitted on DH, DHU, DHV,");
		printf("\nDZ, DZV, DZQ, DL output lines !");
		printf("\n\nConfirm");
		if(yes(NO, NOHELP) == NO)
			exit(0);
	}
	if(logused) {
		printf("\n\7\7\7Log files will be overwritten !");
	printf("\nThe `b' command may be used to save log files.");
		printf("\n\nProceed");
		if(yes(NO, NOHELP) == NO)
			exit(0);
	}
#ifdef EFLG
	/* request the eventflags */
	/* initial values for flags are generated by setext() */

	efid[0] = (int)evntflg(EFREQ, 0666, (long)eflg[0]);
		itos(efid[0], z3);
	if(nexer > 32) {
		efid[1] = (int)evntflg(EFREQ, 0666, (long)eflg[1]);
		itos(efid[1], z4);
	}
#else
	for(i=0; exrtab[i].exn; i++)
		close(creat(exrtab[i].exkf, 0644));
#endif
	wmsg2();

/* Run the exercisers in the script */

	for(i=0; exrtab[i].exn; i++)
			run(i,0);
	
	time(&stime1);
	for( ;; ) {
		j = 0;
		time(&stime2);
		if((stime2 - stime1) > sswait)
			break;
		if(swiflag) {	/* cancel wait loop */
			printf("\nWait loop canceled !\n");
			goto r_xit1;
		}
#ifdef EFLG
		eflg[0] = evntflg(EFRD, efid[0], (long)0);
		if(nexer > 32)
			eflg[1] = evntflg(EFRD, efid[1], (long)0);
		for(k=0; exrtab[k].exn; k++) {
			if(exrtab[k].exs == XRUN)
				continue;
			if((eflg[k>>5] & (1L << (k & 037))) == 0) {
				exrtab[k].exs = XRUN;
				xrun++;	/* say exercisers running */
				time(&timbuf);
				q = &xrn;
				p = exrtab[k].exlf;
				while(*p != '.')
					*q++ = *p++;
				*q = '\0';
				printf("\n%8s started - %s",
					&xrn, ctime(&timbuf));
			} else
				j++;
		}
#else
		for(k=0; exrtab[k].exn; k++) {
			if(exrtab[k].exs == XRUN)
				continue;
			if(access(exrtab[k].exkf, 0) != 0) {
				exrtab[k].exs = XRUN;
				xrun++;	/* say exercisers running */
				unlink(exrtab[k].exkf);
				time(&timbuf);
				q = &xrn;
				p = exrtab[k].exkf;
				while(*p != '.')
					*q++ = *p++;
				*q = '\0';
				printf("\n%8s started - %s",
					&xrn, ctime(&timbuf));
			} else
				j++;
		}
#endif
		if(j == 0)
			break;
	}
r_xit1:
#ifdef EFLG
	for(i=0; exrtab[i].exn; i++)
		if(exrtab[i].exs != XRUN) {
			q = &xrn;
			p = exrtab[i].exlf;
			while(*p != '.')
				*q++ = *p++;
			*q = '\0';
			printf("\n`%8s' did not start !", &xrn);
		}
#else
	for(i=0; exrtab[i].exn; i++)
		if(exrtab[i].exs != XRUN) {
			q = &xrn;
			p = exrtab[i].exkf;
			while(*p != '.')
				*q++ = *p++;
			*q = '\0';
			printf("\n`%8s' did not start !", &xrn);
		}
#endif
r_xit:
	signal(SIGINT, intr);
	if(xrun && logused) {	/* monitor log file growth */
		signal(SIGALRM, logmon);
		alarm(60);
	}
	setjmp(savej);
cloop:
	if(!xrun)
		exit(0);
	printf("\n\nrun> ");
cloop1:
	alrmflag = 0;
	fflush(stdout);
	cc = read(0, (char *)&line[0], 132);
	if((cc == -1) && alrmflag)
		goto cloop1;	/* alarm caused return ! */
	if(cc == 0) {	/* control d */
		if(xrun) {
			printf("\n\n\7\7\7Exercisers running !, really exit");
			if(yes(NO, NOHELP) == NO)
				goto cloop;
			printf("\nType \"sysxstop\" to stop exercisers.");
		}
		printf("\n\n");
		exit(1);
	}
	if(cc == 1)
		goto cloop;
	if(cc > 100) {
	badl:
		printf("\nBad command line !");
		goto cloop;
	}
	p = &line;
	while((*p != '\r') && (*p != '\n')) {
		if((*p >= 'A') && (*p <= 'Z'))
			*p |= 040;	/* force lower case */
		p++;
	}
	*p = 0;
	p = line;
	if((*p >= 'a') && (*p <= 'z') && (strlen(p) != 1) && (*p != 'h'))
		goto badl;
	while(*++p == ' ') ;
/*
 * Decode the command and act accordingly
 * line[0] = command
 * *p	   = rest of command line
 */
	switch(line[0]) {
	case 'h':	/* ON-LINE HELP FACILITY */
		if(!(*p)) {
			*p = 'h';
			*(p+1) = '\0';
		}
		if(*p == '!')
			*p = 'z';
		dohelp(p);
		goto cloop;
	case '!':	/* EXECUTE UNIX COMMAND */
		if(*p) {
			fflush(stdout);
			if(logused)	/* turn off alarm clock */
				alarm(0);
			system(p);
			printf("!\nrun> ");
			if(logused)	/* turn on alarm clock */
				logmon();
		} else
			printf("\nCommand missing !\n> ");
		goto cloop1;
	case 'p':	/* DISPLAY SCRIPT/STATUS */
		n = &script;
		while(*n++ != '.') ;
		*--n = 0;
		printf("\n\nCurrent running script is `%s'", script);
		*n = '.';
		printf("\n\n # EXER  STATE      LOGFILE  OPTIONS");
		for(i=0; exrtab[i].exn; i++) {
			printf("\n%2d ", i+1);
			printf("%4s  ", exrtab[i].exn);
			if(exrtab[i].exs == XRUN)
				printf("run   ");
			else
				printf("stop  ");
			if(exrtab[i].exlf)
				printf("%12s  ", exrtab[i].exlf);
			else
				printf("              ");
			if(exrtab[i].exo)
				printf("%s", exrtab[i].exo);
		}
		break;
	case 'r':	/* RESTART EXERCISERS */
		do
			printf("\nRestart which exerciser(s) <all> ? ");
		while((cc = getline(18)) < 0);

		signal(SIGINT, SIG_IGN);
		alarm(0);	/* stop monitoring log files */
		logused = 0;
		
		if(tall(cc)) {	/* restart all stopped exercisers */
			for(i = 0; exrtab[i].exn; i++)
				if(exrtab[i].exs == XSTOP)
					stpflg++;
			if(!stpflg) {
				printf("\nNo exercisers stopped !\n");
				goto cloop;
			}
#ifdef EFLG
			eflg[0] = 0L;
			eflg[1] = 0L;
			lfflag = 0;
			for(i=0; exrtab[i].exn; i++)
				if(exrtab[i].exs == XSTOP) {
					eflg[i>>5] |= (1L << (i & 037));
					if(exrtab[i].exlf)
						lfflag++;
				}
			evntflg(EFWRT, efid[0], (long)eflg[0]);
			if(eflg[1])
				evntflg(EFWRT, efid[1], (long)eflg[1]);
#else
			for(i=0; exrtab[i].exn; i++)
				if((exrtab[i].exs == XRUN) && exrtab[i].exkf)
					close(creat(exrtab[i].exkf, 0644));
#endif
		/* Run all stopped exercisers */
			if(lfflag) {
			alf:
				printf("\nAppend to log files");
				if((yn = yes(YES, HELP)) == -1) {
					dohelp("alf");
					goto alf;
				}
			}
			for(i=0; exrtab[i].exn; i++) {
				if(exrtab[i].exs == XRUN)
					continue;
				if((exrtab[i].exlf) && (!yn))
					exrtab[i].osz = exrtab[i].nsz = 0L;
				run(i, yn);
			}

			wmsg2();	/* print waiting for start message */
			time(&stime1);
			for( ;; ) {
				j = 0;
				time(&stime2);
				if((stime2 - stime1) > sswait)
					break;
				if(swiflag) {	/* cancel wait loop */
					printf("\nWait loop canceled !\n");
					goto rs_xit1;
				}
#ifdef EFLG
				eflg[0] = evntflg(EFRD, efid[0], (long)0);
				if(nexer > 32)
					eflg[1] = evntflg(EFRD, efid[1], (long)0);
				for(k=0; exrtab[k].exn; k++) {
					if(exrtab[k].exs == XSTOP) {
						if((eflg[k>>5] & (1L << (k & 037))) == 0) {
						    exrtab[k].exs = XRUN;
						    started(exrtab[k].exlf);
						} else
						    j++;
					}
				}
#else
				for(k=0; exrtab[k].exn; k++) {
					if(exrtab[k].exs == XSTOP) {
					    if(access(exrtab[k].exkf, 0) != 0) {
						    exrtab[k].exs = XRUN;
						    started(exrtab[k].exkf);
					    } else
						    j++;
					}
				}
#endif
				if(j == 0)
					break;
			}
		rs_xit1:
			for(i=0; exrtab[i].exn; i++)
				if(exrtab[i].exs == XSTOP)
					nostart(exrtab[i].exlf);
		} else if(cc < 4) {	/* start one copy of an exerciser */
#ifdef EFLG
			i = atoi(line);
			i--;
			if(exrtab[i].exn == 0) {
			    printf("\n`Exexciser #%d' - not in script !",i+1 );
			    goto rs_xit;
			}
			q = &xrn;
			p = exrtab[i].exlf;
			while(*p != '.')
				*q++ = *p++;
			*q = '\0';
#else
			q = &xrn;
			p = &line;
			while(*p != '.')
				*q++ = *p++;
			*q = '\0';
#endif
#ifdef EFLG
			eflg[0] = 0L;
			eflg[1] = 0L;
			lfflag = 0;
			if(exrtab[i].exs == XSTOP) {
				eflg[i>>5] = (1L << (i & 037));
				evntflg(EFWRT, efid[i>>5], (long)eflg[i>>5]);
				if(exrtab[i].exlf)
					lfflag++;
			} else {
				printf("\nExerciser %d - is running !", i+1);
				goto rs_xit;
			}
#else
			for(i=0; exrtab[i].exn; i++)
				if(strcmp(&line, exrtab[i].exkf) == 0)
					if(exrtab[i].exs == XRUN) {
						close(creat(exrtab[i].exkf,0644));
						break;
					} else {
					   printf("\n`%s' - not running !",&xrn);
						goto rs_xit;
					}
#endif
			if(lfflag) {
			alf1:
				printf("\nAppend to log files");
				if((yn = yes(YES, HELP)) == -1) {
					dohelp("alf");
					goto alf1;
				}
			}
		/* Run the selected exerciser */
			if((exrtab[i].exlf) && (!yn))
				exrtab[i].osz = exrtab[i].nsz = 0L;
			run(i, yn);
			wmsg2();
			time(&stime1);
			for( ;; ) {
				time(&stime2);
				if((stime2 - stime1) > sswait)
					break;
				if(swiflag) {	/* cancel wait loop */
					printf("\nWait loop canceled !\n");
					goto rs_xit2;
				}
#ifdef EFLG
				eflg[i>>5] = evntflg(EFRD, efid[i>>5], (long)0);
				if((eflg[i>>5] & (1L << (i & 037))) == 0) {
					exrtab[i].exs = XRUN;
					started(exrtab[i].exlf);
					break;
				}
#else
				if(access(exrtab[i].exkf, 0) != 0) {
					exrtab[i].exs = XSTOP;
					started(exrtab[i].exkf);
					break;
				}
#endif
			}
		rs_xit2:
			if(exrtab[i].exs != XRUN)
				nostart(exrtab[i].exlf);
		} else {	/* start all copies of an exerciser */
#ifdef EFLG
			eflg[0] = 0L;
			eflg[1] = 0L;
			lfflag = 0;
			for(i=0, j=0, k=0; exrtab[i].exn; i++)
				if(strcmp(&line, exrtab[i].exn) == 0) {
					k++;
					if(exrtab[i].exs == XSTOP) {
						j++;
						eflg[i>>5] |= (1L << (i & 037));
						if(exrtab[i].exlf)
							lfflag++;
					}
				}
			if(eflg[0])
				evntflg(EFWRT, efid[0], (long)eflg[0]);
			if(eflg[1])
				evntflg(EFWRT, efid[1], (long)eflg[1]);
#else
			for(i=0, j=0, k=0; exrtab[i].exn; i++)
				if(strcmp(&line, exrtab[i].exn) == 0) {
					k++;
					if(exrtab[i].exs == XRUN) {
						j++;
					      close(creat(exrtab[i].exkf,0644));
					}
				}
#endif
			if(k == 0) {
				printf("\n`%s' - not in script !", &line);
				goto rs_xit;
			}
			if(j == 0) {
				printf("\n`%s' - no copies stopped !", &line);
				goto rs_xit;
			}
			if(lfflag) {
			alf2:
				printf("\nAppend to log files");
				if((yn = yes(YES, HELP)) == -1) {
					dohelp("alf");
					goto alf2;
				}
			}
			for(i=0; exrtab[i].exn; i++)
				if(strcmp(&line, exrtab[i].exn) == 0) {
					if((exrtab[i].exlf) && (!yn))
						exrtab[i].osz = exrtab[i].nsz = 0L;
					if(exrtab[i].exs == XSTOP)
						run(i, yn);
				}
			wmsg2();
			time(&stime1);
			for( ;; ) {
				k = 0;
				time(&stime2);
				if((stime2 - stime1) > sswait)
					break;
				if(swiflag) {	/* cancel wait loop */
					printf("\nWait loop canceled !\n");
					goto rs_xit3;
				}
				for(i=0; exrtab[i].exn; i++) {
					if(exrtab[i].exs != XSTOP)
						continue;
					if(strcmp(&line, exrtab[i].exn) == 0) {
						k++;
#ifdef EFLG
						eflg[i>>5] = evntflg(EFRD, efid[i>>5], (long)0);
						if((eflg[i>>5] & (1L << (i & 037))) == 0) {
							exrtab[i].exs = XRUN;
							started(exrtab[i].exlf);
						}
#else
						if(access(exrtab[i].exkf, 0) != 0) {
							exrtab[i].exs = XSTOP;
							started(exrtab[i].exkf);
					 	}
#endif
				        }
				}
				if(k == 0)
					break;
			}
		rs_xit3:
			for(i=0; exrtab[i].exn; i++)
				if((strcmp(&line, exrtab[i].exn) == 0)
				  && (exrtab[i].exs != XRUN))
					nostart(exrtab[i].exlf);
		}
	rs_xit:
		for(j=0; exrtab[j].exn; j++)
			if(exrtab[j].exs == XRUN)
				if(exrtab[j].exlf)
					logused++;
		if(logused)
			alarm(60);
		signal(SIGINT, intr);
		break;
	case 's':	/* STOP EXERCISERS */

		do
			printf("\nStop which exerciser(s) <all> ? ");
		while((cc = getline(18)) < 0);

		signal(SIGINT, SIG_IGN);
		alarm(0);	/* stop monitoring log files */
		logused = 0;	/* clear log file in use flag */
		if(tall(cc)) {	/* stop all exercisers */
#ifdef EFLG
			eflg[0] = 0L;
			eflg[1] = 0L;
			for(i=0; exrtab[i].exn; i++)
				if(exrtab[i].exs == XRUN)
					eflg[i>>5] |= (1L << (i & 037));
			evntflg(EFWRT, efid[0], (long)eflg[0]);
			if(eflg[1])
				evntflg(EFWRT, efid[1], (long)eflg[1]);
#else
			for(i=0; exrtab[i].exn; i++)
				if((exrtab[i].exs == XRUN) && exrtab[i].exkf)
					close(creat(exrtab[i].exkf, 0644));
#endif
			killpg(pgrp, SIGQUIT);
			wmsg();	/* print waiting for stop message */
			time(&stime1);
			for( ;; ) {
				j = 0;
				time(&stime2);
				if((stime2 - stime1) > sswait)
					break;
				if(swiflag) {	/* cancel wait loop */
					printf("\nWait loop canceled !\n");
					goto s_xit1;
				}
#ifdef EFLG
				eflg[0] = evntflg(EFRD, efid[0], (long)0);
				if(nexer > 32)
					eflg[1] = evntflg(EFRD, efid[1], (long)0);
				for(k=0; exrtab[k].exn; k++) {
					if(exrtab[k].exs == XRUN) {
						if((eflg[k>>5] & (1L << (k & 037))) == 0) {
						    exrtab[k].exs = XSTOP;
						    stoped(exrtab[k].exlf);
					    } else
						    j++;
					}
				}
#else
				for(k=0; exrtab[k].exn; k++) {
					if(exrtab[k].exs == XRUN) {
					    if(access(exrtab[k].exkf, 0) != 0) {
						    exrtab[k].exs = XSTOP;
						    stoped(exrtab[k].exkf);
					    } else
						    j++;
					}
				}
#endif
				if(j == 0)
					break;
			}
		s_xit1:
			for(i=0; exrtab[i].exn; i++)
				if(exrtab[i].exs == XRUN)
					nostop(exrtab[i].exlf);
		} else if(cc < 4) {	/* stop one copy of an exerciser */
#ifdef EFLG
			i = atoi(line);
			i--;
			if(exrtab[i].exn == 0) {
			    printf("\n`Exexciser #%d' - not in script !",i+1 );
			    goto s_xit;
			}
			q = &xrn;
			p = exrtab[i].exlf;
			while(*p != '.')
				*q++ = *p++;
			*q = '\0';
#else
			q = &xrn;
			p = &line;
			while(*p != '.')
				*q++ = *p++;
			*q = '\0';
#endif
#ifdef EFLG
			eflg[0] = 0L;
			eflg[1] = 0L;
			if(exrtab[i].exs == XRUN) {
				eflg[i>>5] = (1L << (i & 037));
				evntflg(EFWRT, efid[i>>5], (long)eflg[i>>5]);
			} else {
				printf("\nExerciser %d - not running !", i+1);
				goto s_xit;
			}
#else
			for(i=0; exrtab[i].exn; i++)
				if(strcmp(&line, exrtab[i].exkf) == 0)
					if(exrtab[i].exs == XRUN) {
						close(creat(exrtab[i].exkf,0644));
						break;
					} else {
					   printf("\n`%s' - not running !",&xrn);
						goto s_xit;
					}
#endif
			killpg(pgrp, SIGTERM);
			wmsg();
			time(&stime1);
			for( ;; ) {
				time(&stime2);
				if((stime2 - stime1) > sswait)
					break;
				if(swiflag) {	/* cancel wait loop */
					printf("\nWait loop canceled !\n");
					goto s_xit2;
				}
#ifdef EFLG
				eflg[i>>5] = evntflg(EFRD, efid[i>>5], (long)0);
				if((eflg[i>>5] & (1L << (i & 037))) == 0) {
					exrtab[i].exs = XSTOP;
					stoped(exrtab[i].exlf);
					break;
				}
#else
				if(access(exrtab[i].exkf, 0) != 0) {
					exrtab[i].exs = XSTOP;
					stoped(exrtab[i].exkf);
					break;
				}
#endif
			}
		s_xit2:
			if(exrtab[i].exs != XSTOP)
				nostop(exrtab[i].exlf);
		} else {	/* stop all copies of an exerciser */
#ifdef EFLG
			eflg[0] = 0L;
			eflg[1] = 0L;
			for(i=0, j=0, k=0; exrtab[i].exn; i++)
				if(strcmp(&line, exrtab[i].exn) == 0) {
					k++;
					if(exrtab[i].exs == XRUN) {
						j++;
						eflg[i>>5] |= (1L << (i & 037));
					}
				}
			if(eflg[0])
				evntflg(EFWRT, efid[0], (long)eflg[0]);
			if(eflg[1])
				evntflg(EFWRT, efid[1], (long)eflg[1]);
#else
			for(i=0, j=0, k=0; exrtab[i].exn; i++)
				if(strcmp(&line, exrtab[i].exn) == 0) {
					k++;
					if(exrtab[i].exs == XRUN) {
						j++;
					      close(creat(exrtab[i].exkf,0644));
					}
				}
#endif
			if(k == 0) {
				printf("\n`%s' - not in script !", &line);
				goto s_xit;
			}
			if(j == 0) {
				printf("\n`%s' - no copies running !", &line);
				goto s_xit;
			}
			killpg(pgrp, SIGTERM);
			wmsg();
			time(&stime1);
			for( ;; ) {
				k = 0;
				time(&stime2);
				if((stime2 - stime1) > sswait)
					break;
				if(swiflag) {	/* cancel wait loop */
					printf("\nWait loop canceled !\n");
					goto s_xit3;
				}
				for(i=0; exrtab[i].exn; i++) {
					if(exrtab[i].exs != XRUN)
						continue;
					if(strcmp(&line, exrtab[i].exn) == 0) {
						k++;
#ifdef EFLG
						eflg[i>>5] = evntflg(EFRD, efid[i>>5], (long)0);
						if((eflg[i>>5] & (1L << (i & 037))) == 0) {
							exrtab[i].exs = XSTOP;
							stoped(exrtab[i].exlf);
						}
#else
						if(access(exrtab[i].exkf, 0) != 0) {
							exrtab[i].exs = XSTOP;
							stoped(exrtab[i].exkf);
					 	}
#endif
				        }
				}
				if(k == 0)
					break;
			}
		s_xit3:
			for(i=0; exrtab[i].exn; i++)
				if((strcmp(&line, exrtab[i].exn) == 0)
				  && (exrtab[i].exs != XSTOP))
					nostop(exrtab[i].exlf);
		}
	s_xit:
		for(j=0; exrtab[j].exn; j++)
			if(exrtab[j].exs == XRUN)
				if(exrtab[j].exlf)
					logused++;
		if(logused)
			alarm(60);
		for(j=0; exrtab[j].exn; j++)
			if(exrtab[j].exs == XRUN)
				break;
		if(exrtab[j].exn == 0) {
			xrun = 0;
#ifdef EFLG
			evntflg(EFREL, efid[0], (long)0);
			if(efid[1])
				evntflg(EFREL, efid[1], (long)0);
#endif
			unlink("sysxr.nx");
		}
		signal(SIGINT, intr);
		break;
	case 'l':	/* PRINT LOG FILES */
		if(logused)	/* turn off alarm clock */
			alarm(0);
		do
			printf("\nPrint which log files <all> ? ");
		while((cc = getline(19)) < 0);
		if(tall(cc))
			lstall = 1;
		else
			lstall = 0;
		printf("\nOutput on line printer");
		if(yes(NO, NOHELP) == YES) {
			if(lstall) {
				system("pr *.log |lpr");
			} else if(cc <= 5) {
				p = strcpy(&xrn, "pr ");
				p = strcat(p, &line);
				p = strcat(p, "*.log |lpr");
				system(p);
			} else {
				if(access(&line, 0) != 0) {
				lferr:
					printf("\nNo such log file !");
					break;
				}
				p = strcpy(&xrn, "pr ");
				p = strcat(p, &line);
				p = strcat(p, " |lpr");
				system(p);
			}
			if(logused)	/* turn on alarm clock */
				logmon();
		} else {
/* code to use more commented out
			printf("\nUse more for video terminal");
			if(yes(NO, NOHELP) == YES) {
				if(lstall)
					system("pr *.log | more -22");
				else if(cc <= 5) {
					p = strcpy(&xrn, "pr ");
					p = strcat(p, &line);
					p = strcat(p, "*.log");
					p = strcat(p, " | more -22");
					system(p);
				} else 	{
					if(access(&line, 0) != 0)
						goto lferr;
					p = strcpy(&xrn, "pr ");
					p = strcat(p, &line);
					p = strcat(p, " | more -22");
					system(p);
				}
			} else {
*/
			if(lstall)
				system("pr *.log");
			else if(cc <= 5) {
				p = strcpy(&xrn, "pr ");
				p = strcat(p, &line);
				p = strcat(p, "*.log");
				system(p);
			} else {
				if(access(&line, 0) != 0)
					goto lferr;
				p = strcpy(&xrn, "pr ");
				p = strcat(p, &line);
				system(p);
			}
			if(logused)	/* turn on alarm clock */
				logmon();
		/* } commented out: goes with 'more' code */
		}
		break;
	case 'b':	/* illegal option in run mode */
	case 'c':	/* illegal option in run mode */
	case 'd':	/* illegal option in run mode */
	case 'n':	/* illegal option in run mode */
	case 'x':	/* illegal option in run mode */
		printf("\nThe %s command is illegal when in run mode !\n", line);
		break;
	default:
		goto badl;
	}
	goto cloop;
}

/*
 * Handle yes or no responses.
 * If only return typed take default response.
 * y or yes is the yes response anything else is NO !
 * def = 1, default answer is yes - return 1 on yes
 * def = 0, default answer is no - return 0 on no
 * hlp = 1, help available - return -1 on ?
 * hlp = 0, no help - print "type yes or no" on ?
 */

yes(def, hlp)
{
	char	resp[10];
	int	cc;

	buflag = 0;
	if(def)
		printf(" <yes> ? ");
	else
		printf(" <no> ? ");
yn:
	alrmflag = 0;
	fflush(stdout);
 	cc = read(0, (char *)&resp, 10);
	if((cc == -1) && alrmflag)
		goto yn;
	if(cc == 0) {	/* ^D - cancel script line */
		buflag++;
		return(NO);
	}
	if(cc > 4)
		return(NO);
	if(cc == 1)
		return(def);
	if((cc == 2) && (resp[0] == '?')) {
		if(hlp == HELP)
			return(-1);
		else {
			printf("\nPlease answer yes or no ! ");
			goto yn;
		}
	}
	if(resp[0] == 'y') {
		if(cc == 2)
			return(YES);
		if((cc == 4) && (resp[1] == 'e') && (resp[2] == 's'))
			return(YES);
	} else
		return(NO);
}

/*
 * Get a line of text form the terminal,
 * replace the new line character with 0
 * and return the character count.
 * hlp = 0, print `no help available'
 * hlp > 0, print help message if `?' typed
 */

getline(hlp)
{
	register int	cc, i;

	buflag = 0;
loop:
	alrmflag = 0;
	fflush(stdout);
	while((cc = read(0, (char *)&line, 50)) >= 50)
		printf("\nToo many characters, try again !\n");
	if((cc == -1) && alrmflag)
		goto loop;
	for(i=0; i<50; i++) {
		if((line[i] >= 'A') && (line[i] <= 'Z'))
			line[i] |= 040;	/* force lower case */
		if((line[i] == '\r') || (line[i] == '\n')) {
			line[i] = 0;
			break;
		}
	}
	if(cc == 0) {	/* ^D - cancel script line */
		buflag++;
		return(cc);
	}
	if((cc == 2) && (line[0] == '?')) {
		cc = -1;
		switch(hlp) {
		case NOHELP:
			printf("\nSorry no help available !");
			break;
		case 18:
			dohelp("s");
			break;
		case 19:
			dohelp("l");
			break;
		default:
			printf("\nsysx: bad help message number\n");
			break;
		}
	}
	return(cc);
}

intr()
{
	signal(SIGINT, intr);
	longjmp(savej, 1);
}

swintr()
{
	signal(SIGINT, SIG_IGN);
	swiflag++;
}

tall(cc)
{

	if((cc == 1) ||
	  (strcmp("a", line) == 0) ||
	  (strcmp("all", line) == 0))
		return(1);
	else
		return(0);
}

/*
 * Monitor the growth of log files every 15 minutes,
 * to warn the user that errors are occurring and
 * that disk space could become tight !
 */

logmon()
{
	register int i, ft;
	long sd;

	if(!xrun || !logused)
		return;
	alrmflag++;	/* tell the world alarm occurred */
	alrmcnt++;
	ft = 0;
	for(i=0; exrtab[i].exn; i++) {
	    if(exrtab[i].exlf == 0)
	    	continue;	/* no logfile */
	    if(stat(exrtab[i].exlf, &statb) < 0) {
	        printf("\nsysx: Can't stat %s file\n", exrtab[i].exlf);
	    	continue;
	    }
	    exrtab[i].nsz = statb.st_size;
	    sd = exrtab[i].nsz - exrtab[i].osz;
	    if((sd && (alrmcnt >= 15)) || (sd > 500)) {
	    	if(ft++ == 0) {
		    time(&timbuf);
		    printf("\n\n\t  Log file status - %s ", ctime(&timbuf));
		}
		printf("\nLog file `%13s' size changed", exrtab[i].exlf);
		printf(" from %06lu to %06lu bytes", exrtab[i].osz, exrtab[i].nsz);
		exrtab[i].osz = exrtab[i].nsz;
	    }
	}
	if(ft)
	    printf("\n\nrun> ");
	fflush(stdout);
	if(alrmcnt >= 15)
	    alrmcnt = 0;
	signal(SIGALRM, logmon);
	alarm(60);
}

/*
 * Scan the script in sbuf and load the needed
 * pointers to names and the like into the exerciser
 * control table (exrtab).
 */

setext()
{
	register int i;
	register char *n;

	eflg[0] = 0L;
	eflg[1] = 0L;
	for(i=0, n = &sbuf; *n; i++) {
		exrtab[i].exn = n;	/* exer name */
		while(*++n != ' ') ;
		*n++ = '\0';		/* terminate name string */
		if(*n == '-')
			exrtab[i].exo = n;	/* options */
		else
			exrtab[i].exo = 0;	/* no options, can't happen */
#ifdef EFLG
		eflg[i>>5] |= (1L << (i & 037));
		nexer++;
		while((*n != '>') && (*n != '&') && (*n != '\n'))
			n++;
		if(exrtab[i].exo)
			*(n-1) = '\0';
#else
		exrtab[i].exkf = 0;
		while((*n != '>') && (*n != '&') && (*n != '\n')) {
			if(*n++ == '-')
				if(*n++ == 'r') {	/* kill file */
					exrtab[i].exkf = ++n;
					while(*++n != ' ') ;
					*n++ = '\0';
				}
		}
		if(exrtab[i].exo && (exrtab[i].exkf == 0))
			*(n-1) = '\0';
#endif
		exrtab[i].exlf = 0;
		if(*n == '>') {
			n += 2;
			exrtab[i].exlf = n;	/* log file */
			while(*++n != ' ') ;
			*n++ = '\0';
		}
		while(*n++ != '\n') ;
		if((*n == 0) && (*(n+1) != 0))
			n++;	/* ed strips nulls from end of line */
		exrtab[i].exs = XSTOP;
		exrtab[i+1].exn = 0;	/* terminate table */
		if(i == (MAXEXR - 1)) {
			printf("\nMaximum of %d exercisers reached !", MAXEXR);
			break;
		}
	}
}

nostop(kfn)
char	*kfn;
{
	register char *q, *p;
	char	xn[20];

	q = &xn;
	p = kfn;
	while(*p != '.')
		*q++ = *p++;
	*q = '\0';
	printf("\n`%8s' did not stop !", &xn);
}

nostart(kfn)
char	*kfn;
{
	register char *q, *p;
	char	xn[20];

	q = &xn;
	p = kfn;
	while(*p != '.')
		*q++ = *p++;
	*q = '\0';
	printf("\n`%8s' did not start !", &xn);
}

stoped(kfn)
char	*kfn;
{
	register char *q, *p;
	char	xn[20];

	q = &xn;
	p = kfn;
	while(*p != '.')
		*q++ = *p++;
	*q = '\0';
	time(&timbuf);
	printf("\n%8s stopped - %s", &xn, ctime(&timbuf));
}

started(kfn)
char	*kfn;
{
	register char *q, *p;
	char	xn[20];

	q = &xn;
	p = kfn;
	while(*p != '.')
		*q++ = *p++;
	*q = '\0';
	time(&timbuf);
	printf("\n%8s started - %s", &xn, ctime(&timbuf));
}

wmsg()
{
	printf("\nWaiting for shutdown confirmation from exercisers.");
	printf("\nTyping <CTRL/C> will cancel wait loop !\n");
	swiflag = 0;
	signal(SIGINT, swintr);
}

wmsg2()
{
	printf("\nWaiting for startup confirmation from exercisers.");
	printf("\nTyping <CTRL/C> will cancel wait loop !\n");
	swiflag = 0;
	signal(SIGINT, swintr);
}

/*
 * Convert an int to an ascii string
 * and place it in "str".
 * Strip off leading zero's
 */

itos(num, str)
int num;
char *str;
{
	char *n;
	int i, x, div, frst;

	frst = 0;
	x = 0;
	n = str;
	i = num;
	div = 10000;
	while(div) {
		x = (i / div);
		if((x != 0) && (!frst))
			frst++;
		if(frst)
			*n++ = (x + '0');
		i -= (x * div);
		div /= 10;
	}
	*n++ = '\0';
	if(*str == '\0') {
		*str++ = '0';
		*str = '\0';
	}
}

/*
 * Run an exerciser
 * For each exer in script
 * do a fork/exec dynamically adding "-z efpis efid" to each exer
 * note: "i" is the "position in script"
 */

run(pis,appflg)
int pis;
int appflg;
{
	int i, j;
	char *n;
	char *execbuf[25];
	static char z1[] = "-z";
	char parse[256];
	
	for(i = 0; i < 256; i++)
		parse[i] = 0;
	i = pis;
	j = 0;
	execbuf[j++] = exrtab[i].exn;
	if(exrtab[i].exo) {
		strcpy(parse, exrtab[i].exo);
		n = &parse[0];
		execbuf[j++] = n;
		while(*n++ != '\0') {
			if(*n == ' ') {
				execbuf[j++] = n+1;
				*n++ = '\0';
			}
		}
	}
	execbuf[j++] = &z1;
	n = &z2;
	if(i > 31)
		itos((i-32), z2);
	else
		itos(i, z2);
	execbuf[j++] = &z2;
	if(i > 31)
		execbuf[j++] = &z4;
	else
		execbuf[j++] = &z3;
	execbuf[j++] = 0;
	if(exrtab[i].exlf != 0)
		if(appflg)
			freopen(exrtab[i].exlf, "a", stdout);
		else
			freopen(exrtab[i].exlf, "w", stdout);
	if(fork() == 0) {
		execv(exrtab[i].exn, execbuf);
		printf("exec failed\n");
		exit(0);
	}
	if(exrtab[i].exlf != 0)
		freopen("/dev/tty", "w", stdout);
}

dohelp(s)
char *s;
{
	register int i, j;

/*
	if(args[0] == 0)
		args[0] = "help";
*/
	for(i=0; hsub[i].hs_name; i++)
		if(strcmp(s, hsub[i].hs_name) == 0)
			break;
	if(hsub[i].hs_name == 0) {
		printf("\nNo help available for `%s' subject.\n", s);
		return(1);
	}
	for(j=0; hsub[i].hs_msg[j]; j++) {
		if(hsub[i].hs_msg[j] == -1) {
			printf("%s", gs_more);
			while(getchar() != '\n') ;
			continue;
		}
		printf("\n%s", hsub[i].hs_msg[j]);
	}
}
