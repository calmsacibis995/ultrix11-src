
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/


/*
 * SCSCID: @(#)globals.h	3.0	4/21/86
 */
/*
 * Global definitions and variables, etc.
 * init(), log() and bill() routines are here.
 */

char	*malloc();
char	*rindex();
char	*pgetstr();

short	BR;		/* baud rate if lp is a tty */
short	NC;		/* don't send control characters to printer */
int	FC;		/* flags to clear if lp is a tty */
int	FS;		/* flags to set if lp is a tty */
int	XC;		/* flags to clear for local mode */
int	XS;		/* flags to set for local mode */
char	*LF;		/* logfile */
char	*lpac;		/* where the lineprinter accounting is kept */
char	*invoke;	/* name this filter was invoked as */
char	*user;		/* name the user that this should be billed to */
char	*ff;		/* what to use for a form feed */
int	width;		/* width of a page for current line_printer */
int	length;		/* number of lines on a page */
int	linenum;	/* number of lines printed so far on this page */
unsigned npages;	/* number of pages printed */

#define	FORWARD		1	/* directions for diablo */
#define	BACKWARD	0	/* directions for diablo */
int	direction;		/* 1 means we are printing forward */
int	printhead;		/* location of the printhead */
int	diablo;			/* are we a diablo or not */

static struct bauds {
	int baud;
	int speed;
} bauds[] = {
	50,	B50,
	75,	B75,
	110,	B110,
	134,	B134,
	150,	B150,
	200,	B200,
	300,	B300,
	600,	B600,
	1200,	B1200,
	1800,	B1800,
	2400,	B2400,
	4800,	B4800,
	9600,	B9600,
	19200,	EXTA,
	38400,	EXTB,
	0,		0
};

init(argv)			/* initialize global variables */
char *argv[];
{
	static char buf[BUFSIZ/2];
	char b[BUFSIZ];
	char *bp = buf;
	char *cp1;		/* temp character pointer */
	register struct bauds *baudp;
	static struct sgttyb sbuf;
	int ldisp = NTTYDISC;	/* Set new line discipline to take care of
				 * character overflow when the clist structure
				 * is set to less than a reasonable value,
				 * like < 50.
				 */
	printhead = 0;
	linenum = 0;
	npages = 1;
	invoke = ((cp1 = rindex(*argv,'/')) ? cp1 + 1 : *argv);
	user = argv[1];

	if (strncmp("diablo",invoke,sizeof("diablo")-sizeof(char)) == 0)
		diablo = 1;
	else
		diablo = 0;

	if (pgetent(b, invoke) <= 0) {
		fprintf(stderr,"%s: can't find printer description\n",invoke);
		exit(3);
	} else {

	/* The printcap capability routines returns the following codes:
	 *		pgetnum -1 if not found.
	 *		pgetstr  0 if not found.
	 *		pgetflag 0 if not found.
	 *
	 *	John Dustin 6/28/84
	 */
		if ((width = pgetnum("pw", &bp)) < 0)
			width = DEFLINELEN;
		if ((length = pgetnum("pl", &bp)) < 0)
			length = DEFPAGESIZE;
		BR = pgetnum("br", &bp);
		if ((lpac = pgetstr("af", &bp)) == NULL)
			lpac = DEFACCOUNTFILE;
		if ((ff = pgetstr("ff", &bp)) == NULL)
			ff = "\f";
		if ((FC = pgetnum("fc")) < 0)
			FC = 0;
		if ((FS = pgetnum("fs")) < 0)
			FS = 0;
		if ((XC = pgetnum("xc")) < 0)
			XC = 0;
		if ((XS = pgetnum("xs")) < 0)
			XS = 0;
		if ((LF = pgetstr("lf", &bp)) == NULL)
			LF = DEFLOGF;
		NC = pgetflag("nc");
	}

	/*
	 * The following attempt fails on LN01
	 * because it uses /dev/lp, so let it go.
	 */
	if (ioctl(fileno(stdout), TIOCEXCL, (char *)0) < 0) {
		log("Warning: cannot set exclusive-use");
		/* log("cannot set exclusive-use"); */
		/* exit(1);	*/
	}

	/*
	 * get tty parameters must NOT fail
	 */
	if (ioctl(fileno(stdout), TIOCGETP, (char *) &sbuf) < 0) {
		log("cannot get tty parameters");
		exit(1);
	}

	if (ioctl(fileno(stdout), TIOCSETD, &ldisp) < 0) {
		log("Warning: cannot set new line discipline");
		/* log("cannot set new line discipline"); */
		/* exit(1); */
	}

	if (BR > 0) {
		for (baudp = bauds; baudp->baud; baudp++)
			if (BR == baudp->baud)
				break;
			if (!baudp->baud) {
				log("illegal baud rate %d", BR);
				exit(1);
			}
			sbuf.sg_ispeed = sbuf.sg_ospeed = baudp->speed;
	}
	sbuf.sg_flags &= ~FC;
	sbuf.sg_flags |= FS;

	/*
	 * Only attempt to TIOCSETP if BR was specified, else
	 * assume a fixed speed line and don't worry about it.
	 */
	if (BR > 0) {
		if (ioctl(fileno(stdout), TIOCSETP, (char *)&sbuf) < 0) {
			log("cannot set tty parameters (BR)");
			exit(1);
		}
	}
	if (XC) {
		if (ioctl(fileno(stdout), TIOCLBIC, (char *) &XC) < 0) {
			log("cannot set local tty parameters (XC)");
			exit(1);
		}
	}
	if (XS) {
		if (ioctl(fileno(stdout), TIOCLBIS, (char *) &XS) < 0) {
			log("cannot set local tty parameters (XS)");
			exit(1);
		}
	}

#ifndef LN03of
	putchar ('\r');
#endif
	direction = FORWARD;
}

bill(account)
char *account;
{
	FILE *fopen(), *ac;

	if ((ac = fopen(lpac,"a")) != NULL) {
		fprintf(ac,"%s\t%u\n",account,npages);
		fclose(ac);
	}
}

log(message, a1, a2, a3)
char *message;
{
	short	console;
	FILE	*lfd;
	long	time();
	char	*ctime();	/* for recording the date in log file */
	long	clock;		/* holds total seconds since Jan, 1970 */
	char	*ap;		/* pointer to ascii date/time */

	lfd = fopen(LF,"a");
	console = isatty(fileno(lfd));

	fprintf(lfd, console ? "\r\n%s: " : "%s: ", invoke);
	fprintf(lfd, message, a1, a2, a3);
	clock = time(0);
	ap = ctime(&clock);
	fprintf(lfd, "  %s", ap);	/* `date` at end adds the '\n' */
	if (console) {
		putc('\r', lfd);
		putc('\n', lfd);
	}
	fclose(lfd);
}
