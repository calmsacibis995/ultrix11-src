
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)sg_main.c	3.0	4/22/86";
/*
 * ULTRIX-11 System Generation Program (sysgen)
 *
 * main() for sysgen
 * Usage:  sysgen  (sysgen -g, to allow generation of a generic kernel)
 *
 * Fred Canter
 */

#include "sysgen.h"

struct	nlist	nl[] = 
{
	{ "_cputype" },
#define			X_CPUTYPE	0
	{ "_realmem" },
#define			X_REALMEM	1
	{ "_rn_ssr3" },
#define			X_RN_SSR3	2
	{ "fpp" },
#define			XFPP		3
	{ "_hz" },
#define			X_HZ		4
	{ "_timezon" },
#define			X_TIMEZONE	5
	{ "_dstflag" },
#define			X_DSTFLAG	6
	{ "" }
};

int	cputype;	/* Current processor type */
int	fpp;		/* Set if CPU has floating point hardware */
int	hz;		/* AC line frequency */
int	tz;		/* timezone (hours west of GMT) */
int	dst;		/* daylight savings time flag */
unsigned realmem;	/* Current CPU's memory size in 64 byte clicks */
int	rn_ssr3;	/* M/M SSR3 (use 22bit mapping to tell if CPU is 23+) */
int	gkopt;		/* -g, allow generation of a generic kernel */

char	hm_buf[30];	/* help message name buffer */
char	catcmd[40] = "cat \0";
char	syslib[200];
char	*libmsg = "\n< Type: all for all files, return if no files,\
 or list (file1 file2 ... fileN) >\n? ";
char	devlib[200];
char	config[30];
char	cpf[30];
char	cfline[50];

jmp_buf	savej;

main(argc, argv)
int	argc;
char	*argv[];
{
	long	atol();
	int	intr();
	register char *p;
	register int i;
	int	cc;
	int	sidlib, ovlib;
	int	sysall, devall;
	int	sysnone, devnone;
	int	allsidf, allovf;
	int	mem;

	signal(SIGINT, intr);
	nlist("/unix", nl);	/* get current CPU and REALMEM and RN_SSR3 */
	if((nl[X_CPUTYPE].n_value == 0) ||
	   (nl[X_REALMEM].n_value == 0) ||
	   (nl[X_HZ].n_value == 0) ||
	   (nl[X_TIMEZONE].n_value == 0) ||
	   (nl[X_DSTFLAG].n_value == 0) ||
	   (nl[X_RN_SSR3].n_value == 0)) {
	    printf("\nOne or more symbols missing from /unix namelist:\n");
	    printf("\n\tcputype realmem rn_ssr3 hz timezone dstflag!\n");
	    exit(1);
	}
	if((mem = open("/dev/mem", 0)) < 0) {
		printf("\nsysgen: cannot open /dev/mem for reading!\n");
		exit(1);
	}
	lseek(mem, (long)nl[X_CPUTYPE].n_value, 0);
	read(mem, (char *)&cputype, sizeof(cputype));
	lseek(mem, (long)nl[X_REALMEM].n_value, 0);
	read(mem, (char *)&realmem, sizeof(realmem));
	lseek(mem, (long)nl[X_RN_SSR3].n_value, 0);
	read(mem, (char *)&rn_ssr3, sizeof(rn_ssr3));
	if(nl[XFPP].n_value) {
		lseek(mem, (long)nl[XFPP].n_value, 0);
		read(mem, (char *)&fpp, sizeof(fpp));
	} else
		fpp = -1;	/* No FP support code (can't happen, but...) */
	lseek(mem, (long)nl[X_HZ].n_value, 0);
	read(mem, (char *)&hz, sizeof(hz));
	if((hz < 45) || (hz > 65))
		hz = 60;
	lseek(mem, (long)nl[X_TIMEZONE].n_value, 0);
	read(mem, (char *)&tz, sizeof(tz));
	tz /= 60;
	if((tz < 0) || (tz > 23))
		tz = 5;
	lseek(mem, (long)nl[X_DSTFLAG].n_value, 0);
	read(mem, (char *)&dst, sizeof(dst));
	if((dst < 0) || (dst > 1))
		dst = 1;
	/* NO SYSGEN if memory size less than 248KB */
	if((cputype <= 0)||(cputype > 84)||(realmem == 0)||(realmem < 3968)) {
		printf("\nsysgen: bad values read from current kernel!\n");
		printf("\n\tcputype = %d realmem = %D\n",
			cputype, (long)(realmem*64L));
		exit(1);
	}
	if(strcmp("-g", argv[1]) == 0)
		gkopt = 1;	/* allow gen of generic kernel */
	else
		gkopt = 0;
	printf("\n\nULTRIX-11 System Generation Program");
	printf("\n\nFor help, type h then press <RETURN>\n");
	setjmp(savej);
cloop:
	printf("\n");
cloop1:
	printf("\nsysgen> ");
	fflush(stdout);
	cc = read(0, (char *)&line[0], 132);
	if(cc == 0) {	/* control d */
		printf("\n\n");
		exit(0);
	}
	if(cc == 1)
		goto cloop;
	if(cc > 100) {
	badl:
		printf("\nBad command line!");
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
	case 'h':	/* on-line help facility */
		switch(*p) {
		case '!':
			sprintf(hm_buf, "sg_sh");
			phelp(hm_buf);
			break;
		case 'c':
		case 'r':
		case 'l':
		case 'p':
		case 'm':
		case 'i':
		case 'd':
		case 's':
			sprintf(hm_buf, "sg_%c", *p);
			phelp(&hm_buf);
			break;
		default:
			printf("\nNo help available for (%c)\n", *p);
		case 'e':
		case 0:
			pmsg(sg_help);
			break;
		}
		goto cloop1;
	case '!':	/* execute unix command */
		if(*p)
			system(p);
		else
			printf("\nCommand missing!\n");
		goto cloop1;
	case 'd':	/* print device list */
		pmsg(devlist);
		goto cloop;
	case 'c':	/* create configuration file */
		ccf();
/*
 * Load a fresh copy of sysgen after each create,
 * so that the standard device address and vectors
 * will be reloaded into the device type tables.
 */
		execl(argv[0], argv[0], "fresh", "copy", (char *)0);
		printf("\nsysgen: Can't exec fresh copy!\n");
		exit(1);
	case 'm':	/* make unix */
		mkunix();
		break;
	case 'l':	/* list existing config files */
		printf("\nList of configuration files, ignore the `.cf'!\n\n");
		system("ls *.cf");
		goto cloop1;
	case 'r':	/* remove a config file */
		p = cname();
		if (p == 0) {
			printf("\nNo config file removed");
			goto cloop1;
		}
		if (unlink(p) < 0)
			printf("\n%s nonexistent!", p);
		p = strcpy(&cpf, p);
		p = strcat(p, "_p");
		unlink(p);
		goto cloop1;
	case 'i':	/* install new unix */
		pmsg(install);
		break;
	case 'p':	/* print config file */
			/* depends on ?cf_p file generated by `c' command */
		p = cname();
		if (p == 0)
			goto cloop1;
		catcmd[4] = '\0';
		cfline[0] = '\0';
		p = strcat(&cfline, &config);
		p = strcat(&cfline, "_p");
		if(access(p, 0) != 0) {
	 printf("\n`%s' configuration print file nonexistent!\n",cfline);
			break;
		}
		strcat(&catcmd, &cfline);
		system(&catcmd);
		goto cloop1;
	case 's':	/* recompile & archive source files */
		printf("\n\7\7\7RECOMPILE AND ARCHIVE SYSTEM SOURCES!");
		printf("\n(Maximum line length is 80 characters!)\n");
	s1:
		printf("\nLibraries to be rearchived ?");
      printf("\n< Type: s for separate I & D, o for overlay, b for both > ? ");
		cc = getline(NOHELP);
		if(cc != 2)
			goto s1;
		sidlib = ovlib = 0;
		switch(line[0]) {
		case 's':
			sidlib++;
			break;
		case 'o':
			ovlib++;
			break;
		case 'b':
			sidlib++;
			ovlib++;
			break;
		default:
			goto s1;
		}
		sysall = devall = 0;
		line[0] = 0;
	slib1:
		printf("\nLIB1 - system library source files to remake ?");
		printf("%s", libmsg);
		if ((cc = getline(NOHELP)) == 0)
			goto slib1;
		if(cc > 1) {
			if(strcmp("all", &line) == 0)
				sysall = 1;
			else
				sysall = 0;
		}
		if(cc == 1)
			sysnone = 1;
		else
			sysnone = 0;
		strcpy(&syslib, &line);
		line[0] = 0;
	slib2:
	    printf("\nLIB2 - device driver library source files to remake ?");
		printf("%s", libmsg);
		if ((cc = getline(NOHELP)) == 0)
			goto slib2;
		if(cc > 1) {
			if(strcmp("all", &line) == 0)
				devall = 1;
			else
				devall = 0;
		}
		if(cc == 1)
			devnone = 1;
		else
			devnone = 0;
		strcpy(&devlib, &line);
		printf("\n");
		if(sidlib && ovlib && sysall && devall) {
			system("make all");
			break;
		}
		allsidf = allovf = 0;
		if(sidlib && sysall && devall) {
			system("make all70");
			allsidf++;
		}
		if(ovlib && sysall && devall) {
			system("make all23");
			allovf++;
		}
		if(sidlib) {
			if(sysall && !allsidf)
				system("make allsys_id");
			else if(!sysnone && !sysall) {
				sprintf(&line, "mksys_id %s", &syslib);
				system(&line);
			}
			if(devall && !allsidf)
				system("make alldev_id");
			else if(!devnone && !devall) {
				sprintf(&line, "mkdev_id %s", &devlib);
				system(&line);
			}
		}
		if(ovlib) {
			if(sysall && !allovf)
				system("make allsys_ov");
			else if(!sysnone && !sysall) {
				sprintf(&line, "mksys_ov %s", &syslib);
				system(&line);
			}
			if(devall && !allovf)
				system("make alldev_ov");
			else if(!devnone && !devall) {
				sprintf(&line, "mkdev_ov %s", &devlib);
				system(&line);
			}
		}
		break;
	default:
		goto badl;
	}
	goto cloop;
}
