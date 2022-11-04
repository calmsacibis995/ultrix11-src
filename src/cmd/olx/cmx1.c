
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)cmx1.c	3.0	4/22/86";
/*
 *	ULTRIX-11 communications exerciser (cmx).
 *
 *	DH11/DHU11/DHV11/DZ11/DZV11/DZQ11/DL11 
 *
 * PART 1 - (cmx1.c)
 *
 *	Part 1 does the initial setup and argument processing
 *	then writes the results into the `cmx_?#.arg' file,
 *	(? = dh, dhu, dhv, dz, dzv, dzq, dl # = unit number).
 *	The first 16 DL11 lines are treated as DL unit 0 and the
 *	second 16 are DL11 unit 1. DL11 line 0 is the console and
 *	cannot be tested !
 *	It then calls part 2 which is the actual exerciser.
 *	The program is split this way to optimize memory usage.
 *
 * Fred Canter 10/21/82
 * Bill Burns 4/84
 *	added DHU/DHV support
 *	added iostats printout (ala disk exer's)
 *	added DZQ support
 *	added check that all lines of a DH/DZ/DZV/DZQ are not
 *		enabled in ttys, when maintenance loopback mode
 *		is used.
 *	added event flag code
 *
 * This program exercises one DH11, DHU11/DHV11, DZ11/DZV11/DZQ11, 
 * or DL11 unit, (a DL11 unit is actually 16 DL11's)
 * either in maintenance loop back mode or with line
 * turnaroud connectors on the mux panel.
 *
 * Maintenance loopback mode:
 *
 * 1.	Maintenance loopback mode for DH11 and DZ11/DZV11/DZQ11
 *	automatically loops back all lines. This means that all 
 *	terminal lines on DH11 and DZ11/DZV11/DZQ11 devices must be 
 *	disabled in the `/etc/ttys' file.
 *
 * 2.	 The DHU11/DHV11 controllers allow loopback on individual lines.
 *	 When testing DHU11/DHV11 devices only the lines that are
 *	 being tested need to be disabled in the `/etc/ttys' file.
 *
 * Turnaround connectors:
 *
 *	 With turnaround connectors, only the line(s) to be
 *	 exercised must be disabled in `/etc/ttys'.
 *
 ****************************************************************
 *	WARNING WARNING WARNING WARNING WARNING WARNING		*
 *								*
 *	For the DH11, DZ11/DZV11/DZQ11 and DL11 type 		*
 *	controllers, when exercised in maintenance loop 	*
 *	back mode character output on the transmit lines is 	*
 *	NOT disabled !  Prior to running this exerciser the 	*
 *	customers equipment, other than terminals, must be 	*
 *	disconnected from the lines to be exercised or 		*
 *	disabled, if it	would be affected by the test data on 	*
 *	the lines. Also (cmx) will not function properly 	*
 *	unless the currently running ULTRIX-11 kernel is	* 
 *	named `unix'.						*
 *								*
 ****************************************************************
 *
 * USAGE:
 *
 *	cmx [-h] -unit [-m] [-l # [#]] [-u #] [-b #] [-n #] 
 *		[-e #] [-s[#]] [-i] [-z #]
 *
 *	-unit	Unit number and type - dh#, dhu#, dhv#, dz#, dzv#, dzq#,
 *			must be given.
 *		For the DL11, unit 0 is dl 1 - 15, & unit 1 is dl 16 - 31
 *		DL line 0 is the console, can't be tested !
 *
 *	-h	Print the help message.
 *
 *	-m	Suppress maintenance loop back mode.
 *		Line turnaround connector will be required.
 *
 *	-l # #	Select line(s), default is all lines.
 *		# = line, # = bit rate (optional)
 *		( bit rate must be specified for DL's, via -l or -b )
 *
 *	-u #	Do not run line #, multiple lines may be deselected.
 *
 *	-b #	Select bit rate # for all lines, overrides bit rate set by -l.
 *		( bit rate must be set for DL's, via -l or -b )
 *
 *	-n #	Limit the number of data mismatch errors to be printed to #.
 *		The default is 5 and the maximum is 132.
 *
 *	-e #	Set the data error limit to #, default = 100, maximum = 1000.
 *		If the limit is exceeded, the line is deselected.
 *		Also the line is deselected after 5 receiver timeouts.
 *
 *	-s#	Specify time intervals for periodic I/O statistics printouts.
 *		The time interval (#) is in minutes and can range
 *		from 1 to 720 (12 hours).
 *		The default time interval is 1/2 hour.
 *		If no time interval is specified (-s only),
 *		the I/O statistics are not printed.
 *
 *	-i	Inhibit the one minute delay and warning message.
 *		NOT DOCUMENTED FOR OR USED BY THE USER
 *
 *	-z #	Specifies the "exerciser number" in the script
 *		which is the event flag bit that controla the
 *		starting/stopping of the exerciser.
 *
 * 	Below option is gone due to event flag usage:
 *	-r kfn	Specify the run/stop control file name (kill filename).
 *
 */

#include <stdio.h>
#include <a.out.h>
#include <signal.h>
#include <sys/param.h>	/* Don't matter which one */
#include <sys/stat.h>
#include <sys/devmaj.h>

#define	R	0
#define	W	1
#define	RW	2

char	afn[20];	/* argument passing file name */
char	argbuf[512];	/* argument passing buffer */

struct nlist nl[] =
{
	{ "_usermem" },
	{ "_sepid" },
	{ "" },
};

char	dn[4];		/* DH/DHU/DHV/DZ/DZV/DZQ/DL device name */
char	tdn[12] = "/dev/";	/* tty node name `/dev/tty??' */
char	ttys[20];	/* one line ffrom `/etc/ttys' - ##tty## */

int	unit;		/* DH11/DHU11/DHV11/DZ11/DZV11/DZQ11/DL11 unit number */
int	nline;		/* number of lines per unit */

/*
 * Line activity table,
 * -1 = line not selected
 *  0 = line deselected via -u
 *  1 = line selected via -l (without bit rate)
 * >1 = line selected via -l (with bit rate)
 * for active lines, set to bit rate (sgtty) number.
 */

int	lnact[] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};

char	*help[] =
{
	"\n\n(cmx) - ULTRIX-11 DH11/DHU11/DHV11/DZ11/DZV11/DL11 exerciser.",
	"\nUsage:",
	"\tcmx [-h] -unit [-m] [-l # [#]] [-u #] [-b #] [-n #] [-e #]",
	"\n\t-h\tPrint this help message.",
	"\n\t-unit\tUnit number and type - dh#, dhu#, dhv# dz#, dzv#, dzq# or dl#.",
	"\n\t-m\tSuppress maintenance loop back mode.",
	"\n\t-l # #\tSelect line(s) - # = line, # = bit rate.",
	"\n\t-u #\tDo not run line #, multiple lines may be deselected.",
	"\n\t-b #\tSelect bit rate # for all lines.",
	"\n\t-n #\tLimit the number of data mismatch errors printed to #.",
	"\t\tThe default is 5 and the maximum is 132.",
	"\n\t-e #\tSet the data error limit to #, default = 100, maximum = 1000.",
	"\t\tIf the limit is exceeded, the line is deselected.",
	"\t\tAlso the line is deselected after 5 receiver timeouts.",
	"",
	0
};

struct stat statb;

int	dhflag, dhuflag, dhvflag, dzflag, dzvflag, dzqflag, dlflag;
int	nmflag, brflag, lsflag;
int	iflag, sflag;
int	ndep = 5;
int	ndel = 100;
int	istime = 30;

#ifdef EFLG
char	*efbit;
char	*efids;
int	zflag;
#else
char	*killfn = "cmx.kill";
#endif

main(argc, argv)
char *argv[];
int argc;
{
	int	stop(), intr();
	register int i, j, k;
	register char *p, *n;
	int	*ap;
	int	fi, fo, ln, opnerr;
	int	maj;
	int	burst, maxcc, bufsiz;
	int	mem, usermem, sepid;
	int	lineon;

	setpgrp(0, 31111);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	signal(SIGQUIT, stop);
	if(argc < 2)
		goto argerr;
	for(i=1; i<argc; i++) {
		p = argv[i];
		if(*p++ != '-') {
		argerr:
			fprintf(stderr, "\ncmx: bad arg\n");
			goto usage;
		}
		switch(*p) {
		case 'h':	/* Print help message */
		usage:
			for(j=0; help[j]; j++)
				fprintf(stderr, "\n%s", help[j]);
			exit(0);
#ifdef EFLG
		case 'z':
			zflag++;
			i++;
			efbit = argv[i++];
			efids = argv[i];
			break;
#else
		case 'r':
			i++;
			killfn = argv[i];
			break;
#endif
		case 'i':
			iflag++;
			break;
		case 'd':	/* DH/DHU/DHV/DZ/DZV/DZQ/DL unit select */
			dn[0] = *p++;
			if(*p == 'h') {
				if(*(p+1) == 'v')
					dhvflag++;
				else if(*(p+1) == 'u')
					dhuflag++;
				else
					dhflag++;
			} else if(*p == 'l')
				dlflag++;
			else if(*p == 'z') {
				if(*(p+1) == 'v')
					dzvflag++;
				else if(*(p+1) == 'q')
					dzqflag++;
				else
					dzflag++;
			} else {
			badu:
				fprintf(stderr, "\ncmx: bad unit\n");
				goto usage;
			}
			dn[1] = *p++;
			if(dzvflag || dzqflag || dhuflag || dhvflag)
				p++;
			if(dhuflag || dhvflag)
				dn[0] = 'u';
			if(*p < '0' || *p > '7')
				goto badu;
			unit = *p - '0';
			dn[2] = *p++;
			dn[3] = 0;
			if(*p != 0)
				goto badu;
			break;
		case 'm':	/* Suppress maintenance loop back mode */
			nmflag++;
			break;
		case 'b':	/* Set bit rate on all lines */
			i++;
			p = argv[i];
			if(*p < '0' || *p > '9')
				goto argerr;
			if((brflag = brcon(p)) < 0) {
			badbr:
				fprintf(stderr, "\ncmx: bad bit rate\n");
				goto usage;
			}
			break;
		case 'l':	/* line select */
			i++;
			j = atoi(argv[i]);
			if(j < 0 || j > 15) {
			badln:
				fprintf(stderr, "\ncmx: bad line number\n");
				goto usage;
			}
			lnact[j] = 1;
			lsflag++;
			if((argv[i+1] [0] == '-') || ((i+1) >= argc))
				break;
			i++;
			if((k = brcon(argv[i])) < 0)
				goto badbr;
			lnact[j] = k;	/* save bit rate */
			break;
		case 'n':	/* limit data error printouts */
			i++;
			p = argv[i];
			if(*p < '0' || *p > '9')
				goto argerr;
			ndep = atoi(p);
			if(ndep < 0 || ndep > 132) {
	fprintf(stderr, "\ncmx: bad error print limit, using default (5)\n");
				ndep = 5;
			}
			break;
		case 'u':	/* deselect a line */
			i++;
			j = atoi(argv[i]);
			if(j < 0 || j > 15)
				goto badln;
			lnact[j] = 0;
			break;
		case 'e':	/* set data error limit */
			i++;
			p = argv[i];
			if(*p < '0' || *p > '9')
				goto argerr;
			ndel = atoi(p);
			if (ndel < 0 || ndel > 1000) {
				fprintf(stderr, "\ncmx: bad data error limit, using default (100)\n");
				ndel = 100;
			}
			break;
		case 's':
			sflag++;
			p++;
			if(*p < '0' || *p >'9')
				break;
			sflag = 0;
			istime = atoi(p);
			if(istime <= 0)
				istime = 30;
			if(istime > 720)
				istime = 720;
			break;
		default:
			goto argerr;
		}
	}
	if(!zflag) {
		if(isatty(2)) {
			fprintf(stderr,"cmx: detaching... type \"sysxstop\" to stop\n");
			fflush(stderr);
		}
		if((i = fork()) == -1) {
			printf("cmx: Can't fork new copy !\n");
			exit(1);
		}
		if(i != 0)
			exit(0);
	}
	setpgrp(0, 31111);
	if((dhflag + dhuflag + dhvflag + dzflag + dzvflag + dzqflag + dlflag) != 1)
		goto badu;
	if(dzflag) {
		nline = 8;
		maj = DZ_RMAJ << 8;
	} else if(dzvflag || dzqflag) {
		nline = 4;
		maj = DZ_RMAJ << 8;
	} else if(dlflag) {
		nline = 16;
		maj = CO_RMAJ << 8;	/* NOT DL_RMAJ !!! */
	} else if(dhvflag) {
		nline = 8;
		maj = UH_RMAJ << 8;
	} else if(dhuflag) {
		nline = 16;
		maj = UH_RMAJ << 8;
	} else {
		nline = 16;
		maj = DH_RMAJ << 8;
	}

/*
 * Set the character count burst size and maximum
 * character count based on the amount of free memory
 * after the ULTRIX-11 kernel.
 * The symbol `usermem' is read from the kernel
 * to obtain the free memory value.
 * If free memory exceeds 256 kb then the burst is 16
 * and the maximum count is 128, otherwise the burst
 * is 8 and the maximum count is 64.
 */

	nlist("/unix", nl);
	if(nl[0].n_type == 0) {
		fprintf(stderr, "\ncmx: Can't access /unix namelist\n");
		exit(1);
	}
	if((mem = open("/dev/mem", R)) < 0) {
		fprintf(stderr, "\ncmx: Can't open /dev/mem\n");
		exit(1);
	}
	lseek(mem, (long)nl[0].n_value, 0);
	read(mem, (char *)&usermem, sizeof(usermem));
	lseek(mem, (long)nl[1].n_value, 0);
	read(mem, (char *)&sepid, sizeof(sepid));
	close(mem);
	if((usermem >= 4096) && sepid) {
		burst = 32;
		maxcc = 0177;
		bufsiz = 132;
	} else {
		burst = 16;
		maxcc = 077;
		bufsiz = 68;
	}
/*
 * In order to prevent overrun errors, the burst size
 * is reduced in some cases.
 */
	if(dlflag)
		burst = 1;
	if(!dlflag && !dzflag && !dzvflag && !dzqflag)
		burst = burst/4;

/*
 * Set up line activity table.
 * If -l was specified, turn on selected lines.
 * If not turn on all lines,
 * except those deselected by [-u #].
 * If -b specified, set all lines to selected
 * bit rate.
 */

	for(i=0; i<16; i++) {
		if(lnact[i] > 0) {	/* line selected */
			if((i >= 4) && dzvflag) {
				fprintf(stderr,"\ncmx: DZV has only 4 lines\n");
				exit(1);
			}
			if((i >= 4) && dzqflag) {
				fprintf(stderr,"\ncmx: DZQ has only 4 lines\n");
				exit(1);
			}
			if((i >= 8) && dzflag) {
				fprintf(stderr, "\ncmx: DZ has only 8 lines\n");
				exit(1);
			}
			if((i >= 8) && dhvflag) {
				fprintf(stderr,"\ncmx: DHV has only 8 lines\n");
				exit(1);
			}
			if(dlflag && (i == 0) && (unit == 0)) {
				fprintf(stderr, "\ncmx: DL 0 is console, can't exercise !\n");
				exit(1);
			}
			if(brflag)
				lnact[i] = brflag;	/* set bit rate */
		} else if(lnact[i] < 0) {
			if(!lsflag) {
				if(brflag)
					lnact[i] = brflag;
				else
					lnact[i] = 1;
			}
		} else
			lnact[i] = -1;
	}
/*
 * For the DL11 only, make sure that all selected lines
 * also have the bit rate specified. The randum bit rate
 * pattern cannot be used with the DL11 because the bit
 * rate is not programmable.
 */
	if(dlflag) {
		for(i=0; i<16; i++)
			if(lnact[i] == 1) {
				fprintf(stderr,"\ncmx: DL line %d selected,",i);
				fprintf(stderr," without specifing bit rate !\n");
				exit(1);
			}
	}
/*
 * Read the `/etc/ttys' and print an error message
 * if any of the tty lines on this DH/DZ/DZV/DL are enabled.
 * This is necessary to prevent the test data outputted on
 * the line from being seen as input from a terminal
 * by ULTRIX-11.
 */

	if(freopen("/etc/ttys", "r", stdin) == NULL) {
		fprintf(stderr, "\ncmx: Can't open `/etc/ttys' file\n");
		exit(1);
	}
	opnerr = 0;
	fgets(ttys, 20, stdin);		/* skip first line in /etc/ttys */
	while(fgets(ttys, 20, stdin) != NULL) {
		if((ttys[0] > '0') && (ttys[0] < '4')) {    /* line enabled */
		/*	ln = atoi(&ttys[5]); */
/* new */
			p = &ttys[2];
			n = &tdn[5];
			for(;*p != '\n'; p++, n++)
				*n = *p;
			*n = '\0';
/* new */
			/* sprintf(&tdn, "/dev/tty%02d", ln); */
			if(stat(&tdn, &statb) < 0) {
				fprintf(stderr, "\ncmx: Can't stat %s\n", tdn);
				exit(1);
			}
			if(maj != (statb.st_rdev & 0177400))
				continue;	/* not this device */
			for(j=0; j<nline; j++) {
				if((statb.st_rdev & 0377) != (j + (unit*nline)))
					continue;
				lineon++;
				if(lnact[j] >= 0) {	/* line selected */
		    			/*fprintf(stderr,"\ncmx: tty%02d on %s line %02d",ln,dn,j); */
					fprintf(stderr,"\ncmx: %s on %s line %02d",&tdn,dn,j);
					fprintf(stderr,", must be disabled in `/etc/ttys' file.");
					opnerr++;
				}
			}
		}
	}
	if(opnerr) {
		fprintf(stderr, "\n");
		exit(1);
	}
	if(lineon && (dhflag||dzflag||dzvflag||dzqflag) && (nmflag == 0)) {
		fprintf(stderr, "\ncmx: All lines on %s must be disabled in the \n", dn);
		fprintf(stderr, "`/etc/ttys' file, when using maintenance loopback mode !\n");
		exit(1);
	}
	fclose(stdin);
/*
 * Create the DH/DHU/DHV/DZ/DZV/DL nodes: 
 *
 * 	`/dev/dh#??', `/dev/uh#??', `/dev/dz#??', or `/dev/dl#??'
 *
 */
	for(i=0; i<nline; i++) {
		sprintf(&tdn, "/dev/%s%02d", dn, i);
		unlink(tdn);
		if(lnact[i] >= 0)
			if(mknod(tdn, 020600, (maj | (i+(unit*nline)))) < 0) {
				fprintf(stderr, "\ncmx: Can't create %s", tdn);
				fprintf(stderr, ", must be super-user !\n");
				exit(1);
			}
	}

/*
 * Write needed data into the `cmx_??#.arg' file,
 * and call part 2 of the exerciser (cmxr).
 */

	ap = &argbuf;
	*ap++ = maj;
	*ap++ = unit;
	*ap++ = dzflag;
	*ap++ = dzvflag;
	*ap++ = dzqflag;
	*ap++ = dhuflag;
	*ap++ = dhvflag;
	*ap++ = dlflag;
	*ap++ = sflag;
	*ap++ = istime;
	*ap++ = ndep;
	*ap++ = ndel;
	*ap++ = bufsiz;
	*ap++ = burst;
	*ap++ = maxcc;
	*ap++ = brflag;
	*ap++ = nmflag;
#ifdef EFLG
	*ap++ = zflag;
#endif
	for(i=0; i<16; i++)
		*ap++ = lnact[i];
	sprintf(&afn, "cmx_%s.arg", dn);
	if((fo = creat(afn, 0644)) < 0) {
		fprintf(stderr, "\ncmx: Can't create %s file\n", afn);
		exit(1);
	}
	if(write(fo, (char *)&argbuf, 512) != 512) {
		fprintf(stderr, "\ncmx: %s write error\n", afn);
		exit(1);
	}
	close(fo);
	fflush(stdout);
	if(iflag == 0)
		iprint(dn);
	signal(SIGQUIT, SIG_IGN);
#ifdef EFLG
	if(zflag)
		execl("cmxr", "cmxr", dn, efbit, efids, (char *)0);
	else
		execl("cmxr", "cmxr", dn, (char *)0);
#else
	execl("cmxr", "cmxr", dn, killfn, (char *)0);
#endif
	fprintf(stderr, "\ncmx: Can't exec cmxr\n");
	exit(1);
}

/*
 * Convert a bit rate, i.e., 9600 to
 * the bit rate number used by sgtty.
 */

brcon(brp)
char	*brp;
{
	switch(atoi(brp)) {
	case 110:
		return(3);
		break;
	case 300:
		return(7);
		break;
	case 1200:
		return(9);
		break;
	case 2400:
		return(11);
		break;
	case 4800:
		return(12);
		break;
	case 9600:
		return(13);
		break;
	default:
		return(-1);
	}
}

stop()
{
	exit(0);
}

iprint(dn)
char *dn;
{
    fprintf(stderr, "\n\n\7\7\7****** WARNING WARNING WARNING ******\n");
    fprintf(stderr, "\nTest data will be transmitted on %s output lines.", dn);
    fprintf(stderr, "\nCustomer equipment which may be affected by");
    fprintf(stderr, "\nthe test data must be disconnected or disabled.");
    fprintf(stderr, "\nData transmission will begin in one minute !");
    fprintf(stderr, "\nTo abort transmission type control \\ \n");
    sleep(60);
}
