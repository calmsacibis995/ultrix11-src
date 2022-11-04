
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * getty -- adapt to terminal speed on dialup, and call login
 *
 * The define FSTTY causes the DEC control characters to
 * be used as the default instead of the unix ones.
 *	erase	= delete
 *	kill	= control U
 *	intr	= control C
 *
 * Fred Canter 9/29/82
 *
 * Modified to include an external getty table.
 *
 * John Dustin 1/4/84
 * 
 * Changes for VMIN and VTIME (System V). Two new gettytab
 * entries (mn and tm) created.
 * 
 * George Mathew 7/18/85
 *
 * Fred Canter -- 1/13/88
 *
 * Changed vmin to 1 and vtime to 0 (should not be 6/1).
 * Added TIOCSETV so vmin/vtime are really set.
 * Removed "mn" and "tm" from gettytab.
 *
 */

static char Sccsid[] = "@(#)getty.c	3.2	1/13/88";
#include <stdio.h>
#include <sgtty.h>
#include <sys/ttychars.h>
#include <ctype.h>
#include <signal.h>

/*
 * Getty table initializations.
 */

extern	struct sgttyb tmode;
extern	struct tchars tchars;
extern	struct ltchars ltchars;

struct	gettystrs {
	char	*s_field;	/* name to lookup in gettytab */
	char	*s_defalt;	/* value we find by looking in defaults */
	char	*s_value;	/* value that we find in gettytab */
};

struct	gettynums {
	char	*n_field;	/* name to lookup */
	int	n_defalt;	/* number we find in defaults */
	int	n_value;	/* number we find in gettytab */
	int	n_set;		/* we got the table value */
};

struct gettyflags {
	char	*f_field;	/* name to lookup */
	char	f_invrt;	/* name existing in gettytab --> false */
	char	f_defalt;	/* true/false in defaults */
	char	f_value;	/* true/false flag that we find */
	char	f_set;		/* we found the table value */
};

extern	struct gettyflags gettyflags[];
extern	struct gettynums gettynums[];
extern	struct gettystrs gettystrs[];

/*
 * String values.
 */
#define NX	gettystrs[0].s_value
#define CL	gettystrs[1].s_value
#define IM	gettystrs[2].s_value
#define LM	gettystrs[3].s_value
#define ER	gettystrs[4].s_value
#define KL	gettystrs[5].s_value
#define ET	gettystrs[6].s_value
#define PC	gettystrs[7].s_value
#define TT	gettystrs[8].s_value
#define EV	gettystrs[9].s_value
#define LO	gettystrs[10].s_value
#define HN	gettystrs[11].s_value
#define HE	gettystrs[12].s_value
#define IN	gettystrs[13].s_value
#define QU	gettystrs[14].s_value
#define XN	gettystrs[15].s_value
#define XF	gettystrs[16].s_value
#define BK	gettystrs[17].s_value
#define SU	gettystrs[18].s_value
#define DS	gettystrs[19].s_value
#define RP	gettystrs[20].s_value
#define FL	gettystrs[21].s_value
#define WE	gettystrs[22].s_value
#define LN	gettystrs[23].s_value
#define FI	gettystrs[24].s_value

/*
 * Numeric definitions.
 */
#define IS	gettynums[0].n_value
#define OS	gettynums[1].n_value
#define SP	gettynums[2].n_value
#define ND	gettynums[3].n_value
#define CD	gettynums[4].n_value
#define TD	gettynums[5].n_value
#define FD	gettynums[6].n_value
#define BD	gettynums[7].n_value
#define TO	gettynums[8].n_value
#define F0	gettynums[9].n_value
#define F0set	gettynums[9].n_set
#define F1	gettynums[10].n_value
#define F1set	gettynums[10].n_set
#define F2	gettynums[11].n_value
#define F2set	gettynums[11].n_set
#define PF	gettynums[12].n_value

/*
 * Boolean values.
 */
#define HT	gettyflags[0].f_value
#define NL	gettyflags[1].f_value
#define EP	gettyflags[2].f_value
#define EPset	gettyflags[2].f_set
#define OP	gettyflags[3].f_value
#define OPset	gettyflags[2].f_set
#define AP	gettyflags[4].f_value
#define APset	gettyflags[2].f_set
#define EC	gettyflags[5].f_value
#define CO	gettyflags[6].f_value
#define CB	gettyflags[7].f_value
#define CK	gettyflags[8].f_value
#define CE	gettyflags[9].f_value
#define PE	gettyflags[10].f_value
#define RW	gettyflags[11].f_value
#define XC	gettyflags[12].f_value
#define LC	gettyflags[13].f_value
#define UC	gettyflags[14].f_value
#define IG	gettyflags[15].f_value
#define PS	gettyflags[16].f_value
#define HC	gettyflags[17].f_value
#define UB	gettyflags[18].f_value
#define AB	gettyflags[19].f_value

/*
 * (NI) - Not Implemented  -jsd
 */
struct	gettystrs gettystrs[] = {
	{ "nx" },			/* next table */
	{ "cl" },			/* screen clear characters (NI) */
	{ "im" },			/* initial message */
	{ "lm", "login: " },		/* login message */
	{ "er", &tmode.sg_erase },	/* erase character */
	{ "kl", &tmode.sg_kill },	/* kill character */
	{ "et", &tchars.t_eofc },	/* eof character (eot) */
	{ "pc", "" },			/* pad character (NI) */
	{ "tt" },			/* terminal type (NI) */
	{ "ev" },			/* enviroment (NI) */
	{ "lo", "/bin/login" },		/* login program */
	{ "hn" },			/* host name (NI) */
	{ "he" },			/* host name edit (NI) */
	{ "in", &tchars.t_intrc },	/* interrupt char */
	{ "qu", &tchars.t_quitc },	/* quit char */
	{ "xn", &tchars.t_startc },	/* XON (start) char */
	{ "xf", &tchars.t_stopc },	/* XOFF (stop) char */
	{ "bk", &tchars.t_brkc },	/* brk char (alt \n) */
	{ "su", &ltchars.t_suspc },	/* suspend char */
	{ "ds", &ltchars.t_dsuspc },	/* delayed suspend char */
	{ "rp", &ltchars.t_rprntc },	/* reprint line char */
	{ "fl", &ltchars.t_flushc },	/* flush output char */
	{ "we", &ltchars.t_werasc },	/* word erase */
	{ "ln", &ltchars.t_lnextc },	/* literal next char */
	{ "fi", &ltchars.t_iflushc },	/* flush input queue */
	{ 0 }
};

struct	gettynums gettynums[] = {
	{ "is" },			/* input speed */
	{ "os" },			/* output speed */
	{ "sp", 1200 },			/* both speeds */
	{ "nd" },			/* newline delay */
	{ "cd" },			/* carriage-return delay */
	{ "td" },			/* tab delay */
	{ "fd" },			/* form-feed delay */
	{ "bd" },			/* backspace delay */
	{ "to" },			/* timeout (NI) */
	{ "f0" },			/* output flags (NI) */
	{ "f1" },			/* input flags (NI) */
	{ "f2" },			/* user mode flags (NI) */
	{ "pf" },			/* delay before flush at 1st prompt (NI) */
	{ 0 }
};

struct	gettyflags gettyflags[] = {
	{ "ht", 0 },			/* has tabs */
	{ "nl", 1 },			/* has newline char */
	{ "ep", 0 },			/* even parity */
	{ "op", 0 },			/* odd parity */
	{ "ap", 0 },			/* any parity */
	{ "ec", 1 },			/* no echo */
	{ "co", 0 },			/* console special */
	{ "cb", 0 },			/* crt backspace */
	{ "ck", 0 },			/* crt kill */
	{ "ce", 0 },			/* crt erase */
	{ "pe", 0 },			/* printer erase */
	{ "rw", 1 },			/* don't use raw */
	{ "xc", 1 },			/* don't ^X ctl chars */
	{ "lc", 0 },			/* terminal has lower case */
	{ "uc", 0 },			/* terminal has no lower case */
	{ "ig", 0 },			/* ignore garbage */
	{ "ps", 0 },			/* do port selector speed select (NI) */
	{ "hc", 1 },			/* do NOT set hangup on close */
	{ "ub", 0 },			/* unbuffered output (NI) */
	{ "ab", 0 },			/* do auto-baud recognition */
	{ 0 }
};

#define FSTTY

#ifdef FSTTY
#define ERASE	0177
#define KILL	025
#else
#define ERASE	CERASE			/* defaults: #, @ */
#define KILL	CKILL
#endif FSTTY

struct sgttyb tmode = { 0, 0, ERASE, KILL, 0 };

#ifdef FSTTY
struct tchars tchars = { '\03', '\034', '\021', '\023', '\004', '\377', 1, 0 };
struct ltchars ltchars = { CSUSP, CDSUSP, CRPRNT, CFLUSH, CWERASE, CLNEXT };
#else
struct tchars tchars = { CINTR, CQUIT, CSTART, CSTOP, CEOF, CBRK, 1, 0 };
#endif FSTTY


#define EOT	04		/* EOT char */
#define TABBUFSIZ	512

char	defent[TABBUFSIZ];
char	defstrs[TABBUFSIZ];
char	tabent[TABBUFSIZ];
char	tabstrs[TABBUFSIZ];
char	name[16];
int	crmod;
int	upper;
int	lower;
int	hopcount;	/* detect infinite loops in gettytab, init 0 */
int	hardc;		/* hardcopy - use prterase */
int	slowcrt;	/* slow terminal - set CRTBS only */
int	recognized=0;	/* true if found CR and autobauding */
char	hostname[32];
static  char *tbuf;
char	*getstr();

char partab[] = {
	0001,0201,0201,0001,0201,0001,0001,0201,
	0202,0004,0003,0205,0005,0206,0201,0001,
	0201,0001,0001,0201,0001,0201,0201,0001,
	0001,0201,0201,0001,0201,0001,0001,0201,
	0200,0000,0000,0200,0000,0200,0200,0000,
	0000,0200,0200,0000,0200,0000,0000,0200,
	0000,0200,0200,0000,0200,0000,0000,0200,
	0200,0000,0000,0200,0000,0200,0200,0000,
	0200,0000,0000,0200,0000,0200,0200,0000,
	0000,0200,0200,0000,0200,0000,0000,0200,
	0000,0200,0200,0000,0200,0000,0000,0200,
	0200,0000,0000,0200,0000,0200,0200,0000,
	0000,0200,0200,0000,0200,0000,0000,0200,
	0200,0000,0000,0200,0000,0200,0200,0000,
	0200,0000,0000,0200,0000,0200,0200,0000,
	0000,0200,0200,0000,0200,0000,0000,0201
};

main(argc, argv)
char **argv;
{
	char *tname;
	int retval;

	signal(SIGINT, SIG_IGN);
	ghostname(hostname,sizeof(hostname));
	if (hostname[0] == '\0')
		strcpy(hostname, "Amnesiac");
	gettable("default",defent, defstrs);
	gendefaults();
	tname = "default";
	if (argc > 1)
		tname = argv[1];

	switch (tname) {
	case '3':			/* adapt to connect speed (212) */
		ioctl(0, TIOCGETP, &tmode);
		if (tmode.sg_ispeed==B300)
			tname = '0';
		else
			tname = '3';
		break;
	}

	for (;;) {
		int ldisp = DFLT_LDISC;		/* default line discipline */
		gettable(tname,tabent,tabstrs);
		if(OPset || EPset || APset)
			OPset++, EPset++, APset++; 
		setdefaults();
		ioctl(0, TIOCFLUSH, 0);

		if (IS)
			tmode.sg_ispeed = speed(IS);
		else if (SP)
			tmode.sg_ispeed = speed(SP);
		if (OS)
			tmode.sg_ospeed = speed(OS);
		else if (SP)
			tmode.sg_ospeed = speed(SP);

		tmode.sg_flags = setflags(1);	/* set raw... */
		ioctl(0, TIOCSETP, &tmode);
		setchars();
		ioctl(0,TIOCSETC,&tchars);
		ioctl(0,TIOCSETV,&tchars);	/* set vmin & vtime */
		ioctl(0, TIOCSETD, &ldisp);
		if (HC)
			ioctl(0,TIOCHPCL,0);
		do { 
			prompt();
		} while ((retval = getname()) == 2);

		if (retval) {
			tmode.sg_flags = setflags(2);
			if(crmod || NL)
				tmode.sg_flags |= CRMOD;
			if(upper || UC)
				tmode.sg_flags |= LCASE;
			if(lower || LC)
				tmode.sg_flags &= ~LCASE;
			ioctl(0, TIOCSETP, &tmode);
			ioctl(0, TIOCSETC, &tchars);
			ioctl(0, TIOCSETV, &tchars);	/* set vmin & vtime */
			ioctl(0, TIOCSLTC, &ltchars);
			putchr('\n');
			signal(SIGINT, SIG_DFL);
			execl(LO, "login", name, 0);
			exit(1);
		}
		if (NX && *NX)
			tname = NX;
	}
}

getname()
{
	register char *np;
	register c;
	char cs;
	int dflg;		/* delete flag */
	int abc=1;		/* auto-baud count */

	signal(SIGINT, SIG_IGN);
	crmod = upper = lower = 0;
	dflg = 0;

	tmode.sg_flags = setflags(0);
	ioctl(0,TIOCSETP,&tmode);
	tmode.sg_flags = setflags(1);
	ioctl(0,TIOCSETP,&tmode);
	np = name;

	/*
	 * if vt100 or vt52, check for slow speeds
	 */
	hardc = slowcrt = 0;
	if ((hardc = gettype()) == 0) {			/* found a crt */
		if ((tmode.sg_ispeed <= B1200) || (tmode.sg_ospeed <= B1200))
			slowcrt++;			/* use crtbs only */
	} else
		hardc++;		/* use prterase if not a crt */
	for (;;) {
		if (read(0, &cs, 1) <= 0)
			exit(0);
		if ((c = cs&0177) == 0)
			return(0);
		if (c==EOT)
			exit(1);

		if (AB && !recognized && abc++ >= 3)
			return(0);
		if (c==tchars.t_intrc)
			return(0);
		if (c=='\r' || c=='\n' || np >= &name[16])
			break;
		if ((c == 17) || (c == 19))	/* XON=17; XOFF=19 */
			continue;		/* flush CTRL/Q and CTRL/S */
		if (c>='a' && c<='z')
			lower++;
		else if (c>='A' && c<='Z')
			upper++;
		else if (c == ERASE) {
			if (np > name) {
				np--;
				if (hardc) {
					if(!dflg) {
						putchr('\\');
						dflg++;
					}
					putchr(*np);
				}			/* is a crt*/
				else if (slowcrt)
					puts("\b");	/* just backup */
				else	
					puts("\b \b");	/* backup and erase */
			}
			continue;
		}
		else if (c == KILL)			/* just prompt over */
			return(2);
		else if(c == ' ')
			c = '_';
		if (IG && (c < ' ' || c > 0176))
			continue;
		if ((dflg) && (hardc)) { 
			putchr('\/');		/* mark end of deleted text */
			dflg=0;
		}
		putchr(cs);
		*np++ = c;
	}
	*np = 0;				/* null terminate */
	if (np==name) {
		recognized++;
		return(2);			/* only got <cr> */
	}
	if (c == '\r')
		crmod++;

	if (upper && !lower && !LC || UC)	/* map to lower case */
		for (np=name; *np; np++)
			if (isupper(*np))
				*np=tolower(*np);
	return(1);
}
puts(as)
char *as;
{
	register char *s;

	s = as;
	while (*s)
		putchr(*s++);
}
putchr(cc)
{
	char c;

	c = cc;
	c |= partab[c&0177] & 0200;
	if (OP)
		c ^= 200;	/* odd parity */
	write(1, &c, 1);
}
/*
 * Get terminal type from /etc/ttytype.
 * If terminal type is vt100 or vt52, return 0, else return 1
 * Pretty much like getterm() in login(1).
 */
gettype()
{
	char	*devname;
	char	*t, *tname, *type;
	FILE	*tp;
	char	ttline[64];

	if ((tp = fopen("/etc/ttytype", "r")) == NULL) {
unknown:
		fclose(tp);
		return(1);
	}
	devname = ttyname(0);
	for (tname = devname; *tname++; )
		;
	while (*--tname != '/')
		;
	tname++;
	while (fgets(ttline, sizeof(ttline), tp) != NULL) {
		ttline[strlen(ttline) - 1] = '\0';
		type = ttline;
		for (t = ttline; *t && *t != ' ' && *t != '\t'; t++) ;
		*t++ = '\0';
		if (strcmp(t,tname) == 0) {
			if ((strcmp(type,"vt100") == 0) ||
				(strcmp(type, "vt52") == 0)) {
				fclose(tp);
				return(0);
			}
		}
	}
	goto unknown;
}

gendefaults()
{
	register struct gettystrs *sp;
	register struct gettynums *np;
	register struct gettyflags *fp;

	for (sp = gettystrs; sp->s_field; sp++)
		if (sp->s_value)
			sp->s_defalt = sp->s_value;

	for (np = gettynums; np->n_field; np++)
		if (np->n_set)
			np->n_defalt = np->n_value;

	for (fp = gettyflags; fp->f_field; fp++)
		if (fp->f_set)
			fp->f_defalt = fp->f_value;
		else
			fp->f_defalt = fp->f_invrt;
}
setdefaults()
{
	register struct gettystrs *sp;
	register struct gettynums *np;
	register struct gettyflags *fp;
	
	for (sp = gettystrs; sp->s_field; sp++)
		if (!sp->s_value)
			sp->s_value = sp->s_defalt;

	for (np = gettynums; np->n_field; np++)
		if (!np->n_set)
			np->n_value = np->n_defalt;

	for (fp = gettyflags; fp->f_field; fp++)
		if (!fp->f_set)
			fp->f_value = fp->f_defalt;

}
prompt()
{
	if (IM && *IM)
		putf(IM);
	puts(LM);
	if (CO)
		putchr('\n');
}

putf(cp)
register char *cp;
{
	while (*cp) {
		if (*cp != '%') {
			putchr(*cp++);
			continue;
		}
		switch (*++cp) {
			case 'h':
				puts(hostname);
				break;
			case '%':
				putchr('%');
				break;
		}
		cp++;
	}
}

gettable(name, buf, area)
	char *name, *buf, *area;
{
	register struct gettystrs *sp;
	register struct gettynums *np;
	register struct gettyflags *fp;
	int n;

	hopcount = 0;	/* new lookup, start fresh */
	if (getent(buf, name) != 1)
		return;

	for (sp = gettystrs; sp->s_field; sp++)
		sp->s_value = getstr(sp->s_field, &area);
	for (np = gettynums; np->n_field; np++) {
		n = getnum(np->n_field);
		if (n == -1)
			np->n_set = 0;
		else {
			np->n_set = 1;
			np->n_value = n;
		}
	}
	for (fp = gettyflags; fp->f_field; fp++) {
		n = getflag(fp->f_field);
		if (n == -1)
			fp->f_set = 0;
		else {
			fp->f_set = 1;
			fp->f_value = n ^ fp->f_invrt;
		}
	}
}

static char **
charnames[] = {
	&ER, &KL, &IN, &QU, &XN, &XF, &ET, &BK,
	&SU, &DS, &RP, &FL, &WE, &LN, &FI, 0
};

static char *
charvars[] = {
	&tmode.sg_erase, &tmode.sg_kill, &tchars.t_intrc,
	&tchars.t_quitc, &tchars.t_startc, &tchars.t_stopc,
	&tchars.t_eofc, &tchars.t_brkc, &ltchars.t_suspc,
	&ltchars.t_dsuspc, &ltchars.t_rprntc, &ltchars.t_flushc,
	&ltchars.t_werasc, &ltchars.t_lnextc, &ltchars.t_iflushc, 0

};
setchars()
{
	register int i;
	register char *p;

	for (i=0; charnames[i]; i++) {
		p = *charnames[i];
		if (p && *p)
			*charvars[i] = *p;
		else
			*charvars[i] = '\0377';
	}
}

int
setflags(n)
{
	int f, slflg;
	f = 0;
	slflg = LDECCTQ;		/* only ^Q start after ^S */

	if (AP)
		f |= ANYP;
	else if (OP)
		f |= ODDP;
	else if (EP)
		f |= EVENP;
	if (UC)
		f |= LCASE;
	if (NL)
		f |= CRMOD;

	f |= delaybits();

	if (n==1) {
		if (RW)
			f |= RAW;
		else
			f |= CBREAK;
		return(f);
	}
	if (!HT)
		f |= XTABS;
	if (n == 0)
		return(f);

	/*
	 * set defaults according to hardc and slowcrt,
	 * but any gettytab entry will still override.
	 */
	if (!hardc) {
		if (!slowcrt) {		/* set these two if faster crt */
			slflg |= LCRTERA;
			slflg |= LCRTKIL;
			f &= ~FF1;	/* zap the ff1 that always creeps in */
		}
		slflg |= LCRTBS;	/* do this for all crt's */
		f &= ~XTABS;		/* don't expand tabs - extra overhead */
	}
	else		/* isahardcopyterminal */
		slflg |= LPRTERA;

	if (CB)
		slflg |= LCRTBS;
	if (CE)
		slflg |= LCRTERA;
	if (CK)
		slflg |= LCRTKIL;
	if (PE)
		slflg |= LPRTERA;
	if (XC)
		slflg |= LCTLECH;
	if (EC)
		f |= ECHO;
	ioctl(0,TIOCLSET, &slflg);

	return(f);
}
struct delayval {
	unsigned	delay;	/* delay in ms */
	int		bits;
};

struct delayval crdelay[] = {
	1,		CR1,
	2,		CR2,
	3,		CR3,
	0,		CR3,
};

struct delayval nldelay[] = {
	1,		NL1,
	2,		NL2,
	3,		NL3,
	0,		NL3,
};

struct delayval bsdelay[] = {
	1,		BS1,
	0,		0,
};

struct delayval ffdelay[] = {
	1,		FF1,
	0,		FF0,
};

struct delayval tbdelay[] = {
	1,		TAB1,
	2,		TAB2,
	3,		XTABS,		/* this is expand tabs */
	100,		TAB1,
	0,		TAB0,
};
delaybits()
{
	register f;

	f  = adelay(CD, crdelay);
	f |= adelay(ND, nldelay);
	f |= adelay(FD, ffdelay);
	f |= adelay(TD, tbdelay);
	f |= adelay(BD, bsdelay);
	return (f);
}
adelay(ms, dp)
	register ms;
	register struct delayval *dp;
{
	if (ms == 0)
		return (0);
	while (dp->delay && ms > dp->delay)
		dp++;
	return (dp->bits);
}
struct speedtab {
	int	speed;
	int	uxname;
} speedtab[] = {
	50,		B50,
	75,		B75,
	110,		B110,
	134,		B134,
	150,		B150,
	200,		B200,
	300,		B300,
	600,		B600,
	1200,		B1200,
	1800,		B1800,
	2400,		B2400,
	4800,		B4800,
	9600,		B9600,
	19200,		EXTA,
	19,		EXTA,		/* for people who say 19.2K */
	38400,		EXTB,
	38,		EXTB,
	7200,		EXTB,		/* alternative */
	0
};
speed(val)
{
	register struct speedtab *sp;

	if (val <= 15)
		return(val);

	for (sp = speedtab; sp->speed; sp++)
		if (sp->speed == val)
			return (sp->uxname);

	return (B300);		/* default in impossible cases */
}

/*
 * Get an entry for terminal name in buffer bp,
 * from the termcap file.  Parse is very rudimentary;
 * we just notice escaped newlines.
 */
getent(bp, name)
	char *bp, *name;
{
	register char *cp;
	register int c;
	register int i = 0, cnt = 0;
	char ibuf[TABBUFSIZ];
	int tf;

	tbuf = bp;
	tf = open("/etc/gettytab", 0);
	if (tf < 0) {
		system("echo getty: Cannot open /etc/gettytab >> /dev/console");
		return (-1);
	}
	for (;;) {
		cp = bp;
		for (;;) {
			if (i == cnt) {
				cnt = read(tf, ibuf, TABBUFSIZ);
				if (cnt <= 0) {
					close(tf);
					return (0);
				}
				i = 0;
			}
			c = ibuf[i++];
			if (c == '\n') {
				if (cp > bp && cp[-1] == '\\'){
					cp--;
					continue;
				}
				break;
			}
			if (cp >= bp+TABBUFSIZ) {
				write(2,"Gettytab entry too long\n", 24);
				break;
			} else
				*cp++ = c;
		}
		*cp = 0;

		/*
		 * The real work for the match.
		 */
		if (namatch(name)) {
			close(tf);
			return(nchktc());
		}
	}
}

/*
 * nchktc: check the last entry, see if it is tc=xxx. If so,
 * recursively find xxx and append that entry (minus the names)
 * to take the place of the tc=xxx entry. This allows termcap
 * entries to say "like an HP2621 but doesn't turn on the labels".
 * Note that this works because of the left to right scan.
 */
#define MAXHOP	32
nchktc()
{
	register char *p, *q;
	char tcname[16];	/* name of similar terminal */
	char tcbuf[TABBUFSIZ];
	char *holdtbuf = tbuf;
	int l;

	p = tbuf + strlen(tbuf) - 2;	/* before the last colon */
	while (*--p != ':')
		if (p<tbuf) {
			write(2, "Bad gettytab entry\n", 19);
			return (0);
		}
	p++;
	/* p now points to beginning of last field */
	if (p[0] != 't' || p[1] != 'c')
		return(1);
	strcpy(tcname,p+3);
	q = tcname;
	while (q && *q != ':')
		q++;
	*q = 0;
	if (++hopcount > MAXHOP) {
		write(2, "Getty: infinite tc= loop\n", 25);
		return (0);
	}
	if (getent(tcbuf, tcname) != 1)
		return(0);
	for (q=tcbuf; *q != ':'; q++)
		;
	l = p - holdtbuf + strlen(q);
	if (l > TABBUFSIZ) {
		write(2, "Gettytab entry too long\n", 24);
		q[TABBUFSIZ - (p-tbuf)] = 0;
	}
	strcpy(p, q+1);
	tbuf = holdtbuf;
	return(1);
}

/*
 * namatch deals with name matching.  The first field of the termcap
 * entry is a sequence of names separated by |'s, so we compare
 * against each such name.  The normal : terminator after the last
 * name (before the first field) stops us.
 */
namatch(np)
	char *np;
{
	register char *Np, *Bp;

	Bp = tbuf;
	if (*Bp == '#')
		return(0);
	for (;;) {
		for (Np = np; *Np && *Bp == *Np; Bp++, Np++)
			continue;
		if (*Np == 0 && (*Bp == '|' || *Bp == ':' || *Bp == 0))
			return (1);
		while (*Bp && *Bp != ':' && *Bp != '|')
			Bp++;
		if (*Bp == 0 || *Bp == ':')
			return (0);
		Bp++;
	}
}

/*
 * Skip to the next field.  Notice that this is very dumb, not
 * knowing about \: escapes or any such.  If necessary, :'s can be put
 * into the termcap file in octal.
 */
static char *
skip(bp)
	register char *bp;
{
	while (*bp && *bp != ':')
		bp++;
	if (*bp == ':')
		bp++;
	return (bp);
}

/*
 * Return the (numeric) option id.
 * Numeric options look like
 *	li#80
 * i.e. the option string is separated from the numeric value by
 * a # character.  If the option is not found we return -1.
 * Note that we handle octal numbers beginning with 0.
 */
int
getnum(id)
	char *id;
{
	register int i, base;
	register char *bp = tbuf;

	for (;;) {
		bp = skip(bp);
		if (*bp == 0)
			return (-1);
		if (*bp++ != id[0] || *bp == 0 || *bp++ != id[1])
			continue;
		if (*bp == '@')
			return(-1);
		if (*bp != '#')
			continue;
		bp++;
		base = 10;
		if (*bp == '0')
			base = 8;
		i = 0;
		while (isdigit(*bp))
			i *= base, i += *bp++ - '0';
		return (i);
	}
}

/*
 * Handle a flag option.
 * Flag options are given "naked", i.e. followed by a : or the end
 * of the buffer.  Return 1 if we find the option, or 0 if it is
 * not given.
 */
getflag(id)
	char *id;
{
	register char *bp = tbuf;

	for (;;) {
		bp = skip(bp);
		if (!*bp)
			return (-1);
		if (*bp++ == id[0] && *bp != 0 && *bp++ == id[1]) {
			if (!*bp || *bp == ':')
				return (1);
			else if (*bp == '!')
				return (0);
			else if (*bp == '@')
				return(-1);
		}
	}
}

/*
 * Get a string valued option.
 * These are given as
 *	cl=^Z
 * Much decoding is done on the strings, and the strings are
 * placed in area, which is a ref parameter which is updated.
 * No checking on area overflow.
 */
char *
getstr(id, area)
	char *id, **area;
{
	register char *bp = tbuf;

	for (;;) {
		bp = skip(bp);
		if (!*bp)
			return (0);
		if (*bp++ != id[0] || *bp == 0 || *bp++ != id[1])
			continue;
		if (*bp == '@')
			return(0);
		if (*bp != '=')
			continue;
		bp++;
		return (decode(bp, area));
	}
}

/*
 * decode does the grung work to decode the
 * string capability escapes.
 */
static char *
decode(str, area)
	char *str;
	char **area;
{
	register char *cp;
	register int c;
	register char *dp;
	int i;

	cp = *area;
	while ((c = *str++) && c != ':') {
		switch (c) {

		case '^':
			c = *str++ & 037;
			break;

		case '\\':
			dp = "E\033^^\\\\::n\nr\rt\tb\bf\f";
			c = *str++;
nextc:
			if (*dp++ == c) {
				c = *dp++;
				break;
			}
			dp++;
			if (*dp)
				goto nextc;
			if (isdigit(c)) {
				c -= '0', i = 2;
				do
					c <<= 3, c |= *str++ - '0';
				while (--i && isdigit(*str));
			}
			break;
		}
		*cp++ = c;
	}
	*cp++ = 0;
	str = *area;
	*area = cp;
	return (str);
}

