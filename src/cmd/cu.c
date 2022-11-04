
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * This is a version of CU that has been modified
 * for use with the DF03 auto dialer.
 * Ported from VMUNIX 4.1 BSD to V7M-11.
 *
 * Fred Canter 6/5/83
 */
static char Sccsid[] = "@(#)cu.c 3.0 4/21/86";
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <signal.h>
#include <sgtty.h>

/*
 * defs that come from uucp.h
 */
#define NAMESIZE 15
#define FAIL -1
#define SAME 0
#define SLCKTIME 5400	/* system/device timeout (LCK.. files) in seconds */
#define ASSERT(e, f, v) if (!(e)) {\
	fprintf(stderr, "AERROR - (%s) ", "e");\
	fprintf(stderr, f, v);\
	cleanup(FAIL);\
}

/*
 *	cu telno [-t] [-n] [-m] [-s speed] [-a acu] [-l line]
 *
 *	-t is for dial-out to system.
 *	speeds are: 110, 150, 300, 1200, 2400, 4800, 9600.
 *	default for -t is 9600, 1200 otherwise.
 *
 *	Escape with `~' at beginning of line.
 *	Ordinary diversions are ~<, ~> and ~>>.
 *	Silent output diversions are ~>: and ~>>:.
 *	Terminate output diversion with ~> alone.
 *	Quit is ~. and ~! gives local command or shell.
 *	Also ~$ for canned procedure pumping remote.
 *	~%put from [to]  and  ~%take from [to] invoke builtins
 *
 *	Modified for DF02/3 by Bill Shannon - 4/22/81
 */

#define CRLF "\r\n"
#define wrc(ds) write(ds,&c,1)


char	devcua[30] = "/dev/cua0";
char	*lspeed	= "";

int	ln;	/* fd for comm line */
char	tkill, terase;	/* current input kill & erase */
int	efk;		/* process of id of listener  */
char	c;
char	oc;

char	*connmsg[] = {
	"",
	"line busy",
	"call dropped",
	"no carrier",
	"can't fork",
	"acu access",
	"tty access",
	"tty hung",
	"usage: cu telno [-t] [-n] [-m] [-s speed] [-a acu] [-l line]",
	"lock failed: line busy"
};

rdc(ds) {

	ds=read(ds,&c,1); 
	oc = c;
	c &= 0177;
	return (ds);
}

int intr;

sig2()
{
	signal(SIGINT, SIG_IGN); 
	intr = 1;
}

int set14;

xsleep(n)
{
	xalarm(n);
	pause();
	xalarm(0);
}

xalarm(n)
{
	set14=n; 
	alarm(n);
}

sig14()
{
	signal(SIGALRM, sig14); 
	if (set14) alarm(1);
}

int	dout;
int	nhup;
int	mflag;		/* forces set DTR on line (modem) */
int	dbflag;
int	nullbrk;	/* turn breaks (nulls) into dels */
struct sgttyb stbuf;

/*
 *	main: get connection, set speed for line.
 *	spawn child to invoke rd to read from line, output to fd 1
 *	main line invokes wr to read tty, write to line
 */

int	speed;

main(ac,av)
char *av[];
{
	int fk;
	char *telno;
	int cleanup();

	signal(SIGALRM, sig14);
	nhup = (int)signal(SIGINT, cleanup);
	signal(SIGHUP, cleanup);
	signal(SIGQUIT, cleanup);
	if (ac < 2) {
		prf(connmsg[8]);
		exit(8);
	}
	for (; ac > 1; av++,ac--) {
		if (av[1][0] != '-')
			telno = av[1];
		else switch(av[1][1]) {
		case 't':
			dout = 1;
			strcpy(devcua, "/dev/cul0");
			continue;
	/* DEFUNCT */
		case 'b':
/*			nullbrk++;	*/
			continue;
		case 'd':	/* DEBUG FLAG */
			dbflag++;
			continue;
		case 's':
			lspeed = av[2]; ++av; --ac;
			break;
		case 'a':
		case 'l':
			strncpy(devcua, av[2], 30); ++av; --ac;
			break;
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			devcua[strlen(devcua)-1] = av[1][1];
			break;
		case 'm':
			mflag++;
			break;
		default:
			prf("Bad flag %s", av[1]);
			break;
		}
	}
	/*** TEMPORARY ACCOUNTING CODE ***/
	{
		FILE *f;
		extern char *getenv();

		if ((f = fopen("/usr/adm/culog", "a")) != NULL) {
			fprintf(f, "%-8.8s %-20.20s %4.4s\n", getenv("USER"),
				telno, lspeed);
			fclose(f);
		}
	}
	/*** END TEMPORARY ACCOUNTING CODE ***/
	if (!exists(devcua))
		exit(9);
	switch(atoi(lspeed)) {
	case 110:
		speed = B110;break;
	case 150:
		speed = B150;break;
	default:
		if(dout)
			speed = B9600;
		else
			speed = B1200;
		break;
	case 300:
		speed = B300;break;
	case 1200:
		speed = B1200;break;
	case 2400:
		speed = B2400;break;
	case 4800:
		speed = B4800;break;
	case 9600:
		speed = B9600;break;
	}
/*
 * Can't tell if ACU is DF02 or DF03, SO
 * we always dial at 300 BPS, then switch to 1200 BPS
 * if necessary. This means that the DF02/3 ACU board
 * must be setup for 300 BPS ACU communications speed !
 * SEE ACU switch options in DF02/3 modem user's guide.
 */
	if(!dout && (speed != B300)) {
		stbuf.sg_ispeed = B300;
		stbuf.sg_ospeed = B300;
	} else {
		stbuf.sg_ispeed = speed;
		stbuf.sg_ospeed = speed;
	}
	stbuf.sg_flags = EVENP|ODDP;
	stbuf.sg_flags |= RAW;
	stbuf.sg_flags |= TANDEM;
	stbuf.sg_flags &= ~ECHO;
	if(!dout)
		prf("\nDialing...");
	ln = conn(devcua, telno);
	if (ln < 0) {
		prf("Connect failed: %s",connmsg[-ln]);
		cleanup(-ln);
	}
	prf("\n\7Connected");
	fk = fork();
	signal(SIGINT, SIG_IGN);
	if (fk == 0) {
		chwrsig();
		rd();
		prf("\007Lost carrier");
		cleanup(3);
	}
	mode(1);
	efk = fk;
	wr();
	mode(0);
	kill(fk, SIGKILL);
	wait((int *)NULL);
	stbuf.sg_ispeed = 0;
	stbuf.sg_ospeed = 0;
	ioctl(ln, TIOCSETP, &stbuf);
	ioctl(ln, TIOCCDTR, (struct sgttyb *)NULL);
	prf("\nDisconnected");
	cleanup(0);
}

/*
 *	conn: establish dial-out connection.
 *	Example:  fd = conn("/dev/cua0","4500");
 *	Returns descriptor open to tty for reading and writing.
 *	Negative values (-1...-7) denote errors in connmsg.
 *	Uses alarm and fork/wait; requires sig14 handler.
 *	Be sure to disconnect tty when done, via HUPCL or stty 0.
 */

conn(acu,telno)
char *acu, *telno;
{
	extern errno;
	char *atail;
	char *rindex();
	char c = 0;
	int er, dn, t;
	er=0; 

	atail = rindex(acu, '/')+1;
	if (mlock(atail) == FAIL) {
		er = 9;
		goto X;
	}
	xalarm(40);
	if ((dn=open(acu,2))<0) {
		er=(errno == 6? 1:5); 
		goto X;
	}
	if(!dout || mflag)
		ioctl(dn, TIOCSDTR, (struct sgttyb *)NULL);
	ioctl(dn, TIOCSETP, &stbuf);
	ioctl(dn, TIOCEXCL, (struct sgttyb *)NULL);
	ioctl(dn, TIOCHPCL, (struct sgttyb *)NULL);
	ioctl(dn, TIOCFLUSH, (struct sgttyb *)NULL);
	xalarm(0);
	if(dout)
		goto X;		/* direct connect, NO ACU ! */
	t = strlen(telno);
	if (t < 30)
		t = 30;
	xalarm(5*t);
	t = write(dn, "\001", 1);
	sleep(1);		/* waste a little time */
	t = write(dn, "\002", 1);
	t = write(dn, telno, strlen(telno));
	t = read(dn, &c, 1);
	if (c != 'A')
		er = 3;
	xalarm(0);
	if (t<0)
		er=2; 
/*
 * Always dial at 300 BPS,
 * may need to adjust speed !
 */
	if(speed != B300) {
		stbuf.sg_ispeed = speed;
		stbuf.sg_ospeed = speed;
		ioctl(dn, TIOCSETP, &stbuf);
		ioctl(dn, TIOCEXCL, (struct sgttyb *)NULL);
		ioctl(dn, TIOCHPCL, (struct sgttyb *)NULL);
		ioctl(dn, TIOCFLUSH, (struct sgttyb *)NULL);
	}
X: 
	if (er) {
		delock(atail);
		ioctl(dn, TIOCCDTR, (struct sgttyb *)NULL);
		close(dn);
	}
	return (er? -er:dn);
}

/*
 *	wr: write to remote: 0 -> line.
 *	~.	terminate
 *	~<file	send file
 *	~!	local login-style shell
 *	~!cmd	execute cmd locally
 *	~$proc	execute proc locally, send output to line
 *	~%cmd	execute builtin cmd (put and take)
 *	~#	send 1-sec break
 *	~^Z	suspend cu process.
 */

wr()
{
	int ds,fk,lcl,x;
	char *p,b[600];
	for (;;) {
		p=b;
		while (rdc(0) == 1) {
			if (p == b) lcl=(c == '~');
			if (p == b+1 && b[0] == '~') lcl=(c!='~');
			if (nullbrk && c == 0) oc=c=0177; /* fake break char */
			if (!lcl) {
				c = oc;
				if (wrc(ln) == 0) {
					prf("line gone"); return;
				}
				c &= 0177;
			}
			if (lcl) {
/*				if (c == 0177) c=tkill;	*/
				if (c == '\r' || c == '\n') goto A;
				wrc(0);
			}
			*p++=c;
			if (c == terase) {
				p=p-2; 
				if (p<b) p=b;
			}
				/* || c == 0177 REMOVED */
			if (c == tkill || c == '\4' || c == '\r' || c == '\n') p=b;
		}
		return;
A: 
		echo("");
		*p=0;
		switch (b[1]) {
		case '.':
		case '\004':
			return;
		case '#':
			ioctl(ln, TIOCSBRK, 0);
			sleep(1);
			ioctl(ln, TIOCCBRK, 0);
			continue;
		case '!':
		case '$':
			fk = fork();
			if (fk == 0) {
				char *getenv();
				char *shell = getenv("SHELL");
				if (shell == 0) shell = "/bin/sh";
				close(1);
				dup(b[1] == '$'? ln:2);
				close(ln);
				mode(0);
				if (!nhup) signal(SIGINT, SIG_DFL);
				if (b[2] == 0) execl(shell,shell,0);
				/* if (b[2] == 0) execl(shell,"-",0); */
				else execl(shell,"sh","-c",b+2,0);
				prf("Can't execute shell");
				exit(~0);
			}
			if (fk!=(-1)) {
				while (wait(&x)!=fk);
			}
			mode(1);
			if (b[1] == '!') echo("!");
			else {
				echo("$");
			}
			break;
		case '<':
			if (b[2] == 0) break;
			if ((ds=open(b+2,0))<0) {
				prf("Can't divert %s",b+1); 
				break;
			}
			intr=x=0;
			mode(2);
			if (!nhup) signal(SIGINT, sig2);
			while (!intr && rdc(ds) == 1) {
				if (wrc(ln) == 0) {
					x=1; 
					break;
				}
			}
			signal(SIGINT, SIG_IGN);
			close(ds);
			mode(1);
			if (x) return;
			echo("<");
			break;
		case '>':
		case ':':
			{
			FILE *fp; char tbuff[128]; register char *q;
			sprintf(tbuff,"/tmp/cu%d",efk);
			if(NULL==(fp = fopen(tbuff,"w"))) {
				prf("Can't tell other demon to divert");
				break;
			}
			fprintf(fp,"%s\n",(b[1]=='>'?&b[2]: &b[1] ));
			if(dbflag) prf("name to be written in temporary:"),prf(&b[2]);
			fclose(fp);
			kill(efk,SIGEMT);
			}
			break;
#ifdef SIGTSTP
#define CTRLZ	26
		case CTRLZ:
			mode(0);
			kill(getpid(), SIGTSTP);
			mode(1);
			break;
#endif
		case '%':
			dopercen(&b[2]);
			break;
		default:
			prf("Use `~~' to start line with `~'");
		}
		continue;
	}
}
#define	XOFF	023
#define	XON	021

dopercen(line)
register char *line;
{
	char *args[10];
	register narg, f;
	int rcount;
	for (narg = 0; narg < 10;) {
		while(*line == ' ' || *line == '\t')
			line++;
		if (*line == '\0')
			break;
		args[narg++] = line;
		while(*line != '\0' && *line != ' ' && *line != '\t')
			line++;
		if (*line == '\0')
			break;
		*line++ = '\0';
	}
	if (equal(args[0], "take")) {
		if (narg < 2) {
			prf("usage: ~%%take from [to]");
			return;
		}
		if (narg < 3)
			args[2] = args[1];
/*		wrln("stty tabs;");	*/
		wrln("echo '~>:'");
		wrln(args[2]);
		wrln(";tee /dev/null <");
		wrln(args[1]);
		wrln(";echo '~>'\n");
		return;
	} else if (equal(args[0], "put")) {
		if (narg < 2) {
			prf("usage: ~%%put from [to]");
			return;
		}
		if (narg < 3)
			args[2] = args[1];
		if ((f = open(args[1], 0)) < 0) {
			prf("cannot open: %s", args[1]);
			return;
		}
		wrln("stty -echo;cat >");
		wrln(args[2]);
		wrln(";stty echo\n");
		xsleep(5);
		intr = 0;
		if (!nhup)
			signal(SIGINT, sig2);
		mode(2);
		rcount = 0;
		while(!intr && rdc(f) == 1) {
			rcount++;
			if (c == tkill || c == terase)
				wrln("\\");
			if (wrc(ln) != 1) {
				xsleep(2);
				if (wrc(ln) != 1) {
					prf("character missed");
					intr = 1;
					break;
				}
			}
		}
		signal(SIGINT, SIG_IGN);
		close(f);
		if (intr) {
			wrln("\n");
			prf("stopped after %d bytes", rcount);
		}
		wrln("\004");
		xsleep(5);
		mode(1);
		return;
	}
	prf("~%%%s unknown\n", args[0]);
}

equal(s1, s2)
register char *s1, *s2;
{
	while (*s1++ == *s2)
		if (*s2++ == '\0')
			return(1);
	return(0);
}

wrln(s)
register char *s;
{
	while (*s)
		write(ln, s++, 1);
}
/*	chwrsig:  Catch orders from wr process 
 *	to instigate diversion
 */
int whoami;
chwrsig(){
	int dodiver(); 
	whoami = getpid();
	signal(SIGEMT,dodiver);
}
int ds,slnt;
int justrung;
dodiver(){
	static char dobuff[128], morejunk[256]; register char *cp; 
	FILE *fp;
	justrung = 1;
	signal(SIGEMT,dodiver);
	sprintf(dobuff,"/tmp/cu%d",whoami);
	fp = fopen(dobuff,"r");
	if(fp==NULL) prf("Couldn't open temporary");
	unlink(dobuff);
	if(dbflag) {
		prf("Name of temporary:");
		prf(dobuff);
	}
	fgets(dobuff,128,fp); fclose(fp);
	if(dbflag) {
		prf("Name of target file:");
		prf(dobuff);
	}
	for(cp = dobuff-1; *++cp; ) /* squash newline */
		if(*cp=='\n') *cp=0;
	cp = dobuff;
	if (*cp=='>') cp++;
	if (*cp==':') {
		cp++;
		if(*cp==0) {
			slnt ^= 1;
			return;
		} else  {
			slnt = 1;
		}
	}
	if (ds >= 0) close(ds);
	if (*cp==0) {
		slnt = 0;
		ds = -1;
		return;
	}
	if (*dobuff!='>' || (ds=open(cp,1))<0) ds=creat(cp,0644);
	lseek(ds, (long)0, 2);
	if(ds < 0) prf("Creat failed:"), prf(cp);
	if (ds<0) prf("Can't divert %s",cp+1);
}


/*
 *	rd: read from remote: line -> 1
 *	catch:
 *	~>[>][:][file]
 *	stuff from file...
 *	~>	(ends diversion)
 */

rd()
{
	extern int ds,slnt;
	char *p,*q,b[600];
	p=b;
	ds=(-1);
agin:
	while (rdc(ln) == 1) {
		if (!slnt) wrc(1);
		if (p < &b[600])
			*p++=c;
		if (c!='\n') continue;
		q=p; 
		p=b;
		if (b[0]!='~' || b[1]!='>') {
			if (*(q-2) == '\r') {
				q--; 
				*(q-1)=(*q);
			}
			if (ds>=0) write(ds,b,q-b);
			continue;
		}
		if (ds>=0) close(ds);
		if (slnt) {
			write(1, b, q - b);
			write(1, CRLF, sizeof(CRLF));
		}
		if (*(q-2) == '\r') q--;
		*(q-1)=0;
		slnt=0;
		q=b+2;
		if (*q == '>') q++;
		if (*q == ':') {
			slnt=1; 
			q++;
		}
		if (*q == 0) {
			ds=(-1); 
			continue;
		}
		if (b[2]!='>' || (ds=open(q,1))<0) ds=creat(q,0644);
		lseek(ds, (long)0, 2);
		if (ds<0) prf("Can't divert %s",b+1);
	}
	if(justrung) {
		justrung = 0;
		goto agin;
	}
}

mode(f)
{
	struct sgttyb stbuf;
	ioctl(0, TIOCGETP, &stbuf);
	tkill = stbuf.sg_kill;
	terase = stbuf.sg_erase;
	if (f == 0) {
		stbuf.sg_flags &= ~RAW;
		stbuf.sg_flags |= ECHO|CRMOD;
	}
	if (f == 1) {
		stbuf.sg_flags |= RAW;
		stbuf.sg_flags &= ~(ECHO|CRMOD);
	}
	if (f == 2) {
		stbuf.sg_flags &= ~RAW;
		stbuf.sg_flags &= ~(ECHO|CRMOD);
	}
	ioctl(0, TIOCSETP, &stbuf);
}

echo(s)
char *s;
{
	char *p;
	for (p=s;*p;p++);
	if (p>s) write(0,s,p-s);
	write(0,CRLF, sizeof(CRLF));
}

prf(f, s)
char *f;
char *s;
{
	fprintf(stderr, f, s);
	fprintf(stderr, CRLF);
}

exists(devname)
char *devname;
{
	if (access(devname, 0)==0)
		return(1);
	prf("%s does not exist", devname);
	return(0);
}

cleanup(code)
{
	rmlock(NULL);
	exit(code);
}

/*
 * This code is taken directly from uucp and follows the same
 * conventions.  This is important since uucp and cu should
 * respect each others locks.
 */

	/*  ulockf 3.2  10/26/79  11:40:29  */
/* #include "uucp.h" */



/*******
 *	ulockf(file, atime)
 *	char *file;
 *	time_t atime;
 *
 *	ulockf  -  this routine will create a lock file (file).
 *	If one already exists, the create time is checked for
 *	older than the age time (atime).
 *	If it is older, an attempt will be made to unlink it
 *	and create a new one.
 *
 *	return codes:  0  |  FAIL
 */

ulockf(file, atime)
char *file;
time_t atime;
{
	struct stat stbuf;
	time_t ptime;
	int ret;
	static int pid = -1;
	static char tempfile[NAMESIZE];

	if (pid < 0) {
		pid = getpid();
		sprintf(tempfile, "/usr/spool/uucp/LTMP.%d", pid);
	}
	if (onelock(pid, tempfile, file) == -1) {
		/* lock file exists */
		/* get status to check age of the lock file */
		ret = stat(file, &stbuf);
		if (ret != -1) {
			time(&ptime);
			if ((ptime - stbuf.st_ctime) < atime) {
				/* file not old enough to delete */
				return(FAIL);
			}
		}
		ret = unlink(file);
		ret = onelock(pid, tempfile, file);
		if (ret != 0)
			return(FAIL);
	}
	stlock(file);
	return(0);
}


#define MAXLOCKS 10	/* maximum number of lock files */
char *Lockfile[MAXLOCKS];
int Nlocks = 0;

/***
 *	stlock(name)	put name in list of lock files
 *	char *name;
 *
 *	return codes:  none
 */

stlock(name)
char *name;
{
	char *p;
	extern char *calloc();
	int i;

	for (i = 0; i < Nlocks; i++) {
		if (Lockfile[i] == NULL)
			break;
	}
	ASSERT(i < MAXLOCKS, "TOO MANY LOCKS %d", i);
	if (i >= Nlocks)
		i = Nlocks++;
	p = calloc(strlen(name) + 1, sizeof (char));
	ASSERT(p != NULL, "CAN NOT ALLOCATE FOR %s", name);
	strcpy(p, name);
	Lockfile[i] = p;
	return;
}


/***
 *	rmlock(name)	remove all lock files in list
 *	char *name;	or name
 *
 *	return codes: none
 */

rmlock(name)
char *name;
{
	int i;

	for (i = 0; i < Nlocks; i++) {
		if (Lockfile[i] == NULL)
			continue;
		if (name == NULL
		|| strcmp(name, Lockfile[i]) == SAME) {
			unlink(Lockfile[i]);
			free(Lockfile[i]);
			Lockfile[i] = NULL;
		}
	}
	return;
}


/*  this stuff from pjw  */
/*  /usr/pjw/bin/recover - check pids to remove unnecessary locks */
/*	isalock(name) returns 0 if the name is a lock */
/*	unlock(name)  unlocks name if it is a lock*/
/*	onelock(pid,tempfile,name) makes lock a name
	on behalf of pid.  Tempfile must be in the same
	file system as name. */
/*	lock(pid,tempfile,names) either locks all the
	names or none of them */
isalock(name) char *name;
{
	struct stat xstat;
	if(stat(name,&xstat)<0) return(0);
	if(xstat.st_size!=sizeof(int)) return(0);
	return(1);
}
unlock(name) char *name;
{
	if(isalock(name)) return(unlink(name));
	else return(-1);
}
onelock(pid,tempfile,name) char *tempfile,*name;
{	int fd;
	fd=creat(tempfile,0444);
	if(fd<0) return(-1);
	write(fd,(char *) &pid,sizeof(int));
	close(fd);
	if(link(tempfile,name)<0)
	{	unlink(tempfile);
		return(-1);
	}
	unlink(tempfile);
	return(0);
}
lock(pid,tempfile,names) char *tempfile,**names;
{	int i,j;
	for(i=0;names[i]!=0;i++)
	{	if(onelock(pid,tempfile,names[i])==0) continue;
		for(j=0;j<i;j++) unlink(names[j]);
		return(-1);
	}
	return(0);
}

#define LOCKPRE "/usr/spool/uucp/LCK."

/***
 *	delock(s)	remove a lock file
 *	char *s;
 *
 *	return codes:  0  |  FAIL
 */

delock(s)
char *s;
{
	char ln[30];

	sprintf(ln, "%s.%s", LOCKPRE, s);
	rmlock(ln);
}


/***
 *	mlock(sys)	create system lock
 *	char *sys;
 *
 *	return codes:  0  |  FAIL
 */

mlock(sys)
char *sys;
{
	char lname[30];
	sprintf(lname, "%s.%s", LOCKPRE, sys);
	return(ulockf(lname, (time_t) SLCKTIME ) < 0 ? FAIL : 0);
}



/***
 *	ultouch()	update access and modify times for lock files
 *
 *	return code - none
 */

ultouch()
{
	time_t time();
	int i;
	struct ut {
		time_t actime;
		time_t modtime;
	} ut;

	ut.actime = time(&ut.modtime);
	for (i = 0; i < Nlocks; i++) {
		if (Lockfile[i] == NULL)
			continue;
		utime(Lockfile[i], &ut);
	}
	return;
}
