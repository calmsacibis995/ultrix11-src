
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * ULTRIX-11 initialization program - init.c
 *
 * This version of init has been modified for
 * local/remote terminal operation and to reread
 * the ttys file when a hangup signal is received.
 * This allows local terminals to operate without
 * a null modem and for terminals to be placed
 * on-line and taken off-line, and/or change their getty
 * character without rebooting the system.
 *
 * Fred Canter
 *
 * Thanks to Bill Shannon for many of the code changes.
 */
static char Sccsid[] = "@(#)init.c	3.1	10/9/87";
#include <sys/localopts.h>
#include <signal.h>
#include <sys/types.h>
#include <utmp.h>
#include <setjmp.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>

#define	LINSIZ	sizeof(wtmp.ut_line)
#define	TABSIZ	100
#define	ALL	p = &itab[0]; p < &itab[TABSIZ]; p++
#define	EVER	;;
#define SCPYN(a, b)	strncpy(a, b, sizeof(a))
#define SCMPN(a, b)	strncmp(a, b, sizeof(a))

char	shell[]	= "/bin/sh";
char	getty[]	 = "/etc/getty";
char	minus[]	= "-";
char	runc[]	= "/etc/rc";
char	ifile[]	= "/etc/ttys";
char	utmp[]	= UTMP_FILE;
char	wtmpf[]	= WTMP_FILE;
char	ctty[]	= "/dev/console";
char	dev[]	= "/dev/";
char	elc[]	= "/etc/elc";
unsigned sumcheck = 0;

struct utmp wtmp;
struct
{
	char	line[LINSIZ];
	char	comn;
	char	flag;
} line;
struct	tab
{
	char	line[LINSIZ];
	char	flag;
	char	comn;
	char	xflag;
	int	pid;
	time_t	gettytime;
	int	gettycnt;
} itab[TABSIZ];

extern	errno;

int	fi;
int	mergflag;
char	tty[20];
jmp_buf	sjbuf, shutpass;
time_t	time0;

time_t	time();
int	reset();

/*
 * default characters for single user mode.  This is
 * because ^Z and ^Y will goof you up if you aren't
 * running /bin/csh.
 */
struct	stat statb;
struct ltchars lchars =
{
/*	susp  dsusp rprntc  oflushc werasec  lnextc  iflush   status */
	0377, 0377, CRPRNT, CFLUSH, CWERASE, CLNEXT, CIFLUSH, 0
};

main(argc, argv)
int	argc;
char	*argv[];
{
	register int i, j, elpid;

	time(&time0);
	if((j = argc) == 2) {
		i = argv[1][1] << 8;
		i |= argv[1][0];
		argv[1][0] = ' ';
		argv[1][1] = ' ';
	}
	if ((elpid = fork()) == 0) {	/* Start error log copy process */
		open(ctty, 2);
		dup(0);
		dup(0);
		zaptty();
		execl(elc, elc, (char *)0);
		cmesg("init: can't exec", elc, 0);
		exit(0);
	}
	setjmp(sjbuf);
	signal(SIGINT, reset);
	signal(SIGHUP,SIG_IGN);
	for(EVER) {
		shutdown();
		single();
		if (j != 2 || cksum("/bin/login") || setlimit(i)) {
			cmesg("init: Cannot go multi-user.", 
				"Returning to single user mode", 0);
			sync();
			longjmp(sjbuf, 1);
		}
		runcom();
		merge();
		signal(SIGINT, reset);
		multiple(i);
	}
}

cksum(s)
char *s;
{
	register unsigned int i, j, k;
	int sizes[8], buf[512];
	int fd;

	fd = open(s, 0);
	if (fd < 0) {
		return(-1);
	}
	if (read(fd, sizes, 16) < 0) {
		close(fd);
		return(-1);
	}
	k = 0;
	for (j = sizes[1]/2; j > 0; ) {
		if (read(fd, buf, (j > 512) ? 1024 : (j*2)) < 0) {
			close(fd);
			return(-1);
		}
		for (i = 0; (i < 512) && (j > 0); i++, j--)
			k += buf[i];
	}
	for (j = sizes[2]/2; j > 0; ) {
		if (read(fd, buf, (j > 512) ? 1024 : (j*2)) < 0) {
			close(fd);
			return(-1);
		}
		for (i = 0; (i < 512) && (j > 0); i++, j--)
			k += buf[i];
	}
	close(fd);
	return(k);
}

int	shutreset();

shutdown()
{
	register i, f;
	register struct tab *p;

	signal(SIGINT, SIG_IGN);
	for(ALL) {
		term(p);
		p->line[0] = 0;
	}
	close(creat(utmp, 0644));
	signal(SIGALRM, shutreset);
	if (setjmp(shutpass) == 0) {
		alarm(30);
		for(i=0; i<5; i++)
			kill(-1, SIGKILL);
		while(wait((int *)0) != -1)
			;
		alarm(0);
	}
	acct(0);
	signal(SIGALRM, SIG_DFL);
	for(i=0; i<10; i++)
		close(i);
	f = open(wtmpf, 1);
	if (f >= 0) {
		lseek(f, 0L, 2);
		SCPYN(wtmp.ut_line, "~");
		SCPYN(wtmp.ut_name, "shutdown");
		time(&wtmp.ut_time);
		write(f, (char *)&wtmp, sizeof(wtmp));
		close(f);
	}
}

shutreset()
{
	cmesg("WARNING: Something is hung (won't die); ps axl advised", 0);
	longjmp(shutpass, 1);
}

single()
{
	register pid;
	register xpid;
	extern	errno;

   do {
	pid = fork();
	if(pid == 0) {
		signal(SIGHUP, SIG_DFL);
		signal(SIGINT, SIG_DFL);
		signal(SIGALRM, SIG_DFL);
		open(ctty, 2);
		dup(0);
		dup(0);
		ioctl(0, TIOCSLTC, &lchars);
		execl(shell, minus, (char *)0);
		cmesg("init: can't exec ", shell, 0);
		exit(0);
	}
	sync();
	while((xpid = wait((int *)0)) != pid)
		if (xpid == -1 && errno == ECHILD)
			break;
   } while (xpid == -1);
}

runcom()
{
	register pid, f;

	pid = fork();
	if(pid == 0) {
		open("/", 0);
		dup(0);
		dup(0);
		execl(shell, shell, runc, (char *)0);
		exit(1);
	}
	while(wait((int *)0) != pid)
		;
	f = open(wtmpf, 1);
	if (f >= 0) {
		lseek(f, 0L, 2);
		SCPYN(wtmp.ut_line, "~");
		SCPYN(wtmp.ut_name, "reboot");
		if (time0) {
			wtmp.ut_time = time0;
			time0 = 0;
		} else
			time(&wtmp.ut_time);
		write(f, (char *)&wtmp, sizeof(wtmp));
		close(f);
	}
}

setmerge()
{
	signal(SIGHUP, SIG_IGN);
	mergflag = 1;
}

multiple(lim)
register lim;
{
	register struct tab *p;
	register pid;

loop:
	mergflag = 0;
	signal(SIGHUP, setmerge);
	for(EVER) {
		/*
		 * Keep resetting the limit to keep someone from
		 * adb'ing the kernel.  If we can't set the limit,
		 * then we go back to single user mode.
		 */
		if(setlimit(lim)) {
			cmesg("init: returning to single user mode", 0);
			return;
		}
		pid = wait((int *)0);
		if(mergflag) {
			merge();
			goto loop;
		}
		if(pid == -1) {
			if (errno == ECHILD) {
				cmesg("init: no children left", 0);
				return;
			}
			goto loop;
		}
		for(ALL)
			if(p->pid == pid || p->pid == -1) {
				rmut(p);
				dfork(p);
			}
	}
}

term(p)
register struct tab *p;
{

	if(p->pid != 0 && p->pid != -1) {
		rmut(p);
		kill(p->pid, SIGKILL);
	}
	p->pid = 0;
}

rline()
{
	register c, i;

loop:
	c = get();
	if(c < 0)
		return(0);
	if(c == 0)
		goto loop;
	line.flag = c;
	c = get();
	if(c <= 0)
		goto loop;
	line.comn = c;
	SCPYN(line.line, "");
	for (i=0; i<LINSIZ; i++) {
		c = get();
		if(c <= 0 || c == ' ' || c == '\t')
			break;
		line.line[i] = c;
	}

	while(c > 0)
		c = get();
	if (i == 0 || line.flag == '#')	/* skip blank lines and comments */
		goto loop;
	if(line.line[0] == 0)
		goto loop;
	strcpy(tty, dev);
	strncat(tty, line.line, LINSIZ);
	if(access(tty, 06) < 0)
		goto loop;
	return(1);
}

get()
{
	char b;

	if(read(fi, &b, 1) != 1)
		return(-1);
	if(b == '\n')
		return(0);
	return(b);
}

#define	FOUND	1
#define	CHANGE	2

merge()
{
	register struct tab *p;

	fi = open(ifile, 0);
	if(fi < 0)
		return;
	for(ALL)
		p->xflag = 0;
	while(rline()) {
		for(ALL) {
			if (SCMPN(p->line, line.line))
				continue;
			p->xflag |= FOUND;
			if((line.comn != p->comn) || (line.flag != p->flag)) {
				p->xflag |= CHANGE;
				p->flag = line.flag;
				p->comn = line.comn;
			}
			goto contin1;
		}
		if(line.flag == '0')
			goto contin1;
		for(ALL) {
			if(p->line[0] != 0)
				continue;
			SCPYN(p->line, line.line);
			p->xflag |= FOUND|CHANGE;
			p->flag = line.flag;
			p->comn = line.comn;
			goto contin1;
		}
	contin1:
		;
	}
	close(fi);
	for(ALL) {
		if((p->xflag&FOUND) == 0) {
			term(p);
			clrtty(p);
			p->line[0] = 0;
		}
		if((p->xflag&CHANGE) != 0) {
			term(p);
			clrtty(p);
			if((p->flag == '1') || (p->flag == '2'))
				dfork(p);
		}
	}
}

/*
 * Attempt to force the terminal to a known state
 * by doing a non blocking open then a close.
 * This forces DTR to drop when the state of a line
 * is changed from modem to any other state.
 * Also, set the line to local or remote.
 *
 * Added 9/12/87 -- Fred Canter
 */

clrtty(p)
register struct tab *p;
{
	register int fd;

	/*
	 * Check for bogus tty name.
	 * This should not happen.
	 */
	if(p->line[0] == 0)
		return;
	strcpy(tty, dev);
	strncat(tty, p->line, LINSIZ);
	fd = open(tty, O_RDONLY|O_NDELAY);
	if(fd >= 0)
		close(fd);
	zaptty();
	if(stat(&tty, &statb) < 0)
		return;
	/*
	 * Set line to local or remote.
	 */
	ttlocl(statb.st_rdev, (((p->flag - '0') & 02) >> 1));
}

setlimit(n)
{
	switch(n) {
	case '!&':
		lim(8*17);
		break;
	case '.:':
		lim(16*17);
		break;
	case '%$':
		lim(32*17);
		break;
	case ';#':
		lim(100*17);
		break;
	default:
		lim(-1);
		return(-1);
	}
	return(0);
}

dfork(p)
struct tab *p;
{
	register pid;
	time_t	t;
	int dowait = 0;

	time(&t);
	p->gettycnt++;
	if ((t - p->gettytime) >= 60) {
		p->gettytime = t;
		p->gettycnt = 1;
	} else if (p->gettycnt >= 5) {
		dowait = 1;
		p->gettytime = t;
		p->gettycnt = 1;
	}
	pid = fork();
	if(pid == 0) {
		signal(SIGHUP, SIG_DFL);
		signal(SIGINT, SIG_DFL);
		strcpy(tty, dev);
		strncat(tty, p->line, LINSIZ);
		if (dowait) {
			cmesg("init: ", getty, tty," failing, sleeping");
			sleep(30);
		}
		chown(tty, 0, 0);
		chmod(tty, 0622);
		if (open(tty, 2) < 0) {
			int repcnt = 0;
			do {
				if (repcnt % 10 == 0)
				    cmesg("init: cannot open", tty, 0);
				repcnt++;
				sleep(60);
			} while (open(tty, 2) < 0);
		}
		dup(0);
		dup(0);
		tty[0] = p->comn;
		tty[1] = 0;
		execl(getty, minus, tty, (char *)0);
		cmesg("init: can't exec ", getty, tty, 0);
		exit(0);
	}
	p->pid = pid;
}

rmut(p)
register struct tab *p;
{
	register f;
	int found = 0;

	f = open(utmp, 2);
	if(f >= 0) {
		while(read(f, (char *)&wtmp, sizeof(wtmp)) == sizeof(wtmp)) {
			if (SCMPN(wtmp.ut_line, p->line) || wtmp.ut_name[0]==0)
				continue;
			lseek(f, -(long)sizeof(wtmp), 1);
			SCPYN(wtmp.ut_name, "");
			time(&wtmp.ut_time);
			write(f, (char *)&wtmp, sizeof(wtmp));
			found++;
		}
		close(f);
	}
	if (found) {
		f = open(wtmpf, 1);
		if (f >= 0) {
			SCPYN(wtmp.ut_line, p->line);
			SCPYN(wtmp.ut_name, "");
			time(&wtmp.ut_time);
			lseek(f, (long)0, 2);
			write(f, (char *)&wtmp, sizeof(wtmp));
			close(f);
		}
		/*
		 * After a proper login force reset
		 * of error detection code in dfork.
		 */
		p->gettytime = 0;
	}
}

reset()
{
	longjmp(sjbuf, 1);
}

cmesg(s1, s2, s3, s4)
char *s1, *s2, *s3, *s4;
{
	register int pid;
	char c = ' ';

	pid = fork();
	if (pid == 0) {
		int fd = open(ctty, 2);
		write(fd, s1, strlen(s1));
		if (s2) {
			write(fd, &c, 1);
			write(fd, s2, strlen(s2));
			if (s3) {
				write(fd, &c, 1);
				write(fd, s3, strlen(s3));
				if (s4) {
					write(fd, &c, 1);
					write(fd, s4, strlen(s4));
				}
			}
		}
		write(fd, "\r\n", 2);
		close(fd);
		exit(0);
	}
	while (wait((int *)0) != pid)
		;
}
