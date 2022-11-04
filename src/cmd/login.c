
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * login [ name ]
 *
 * Modified to use DEC tty control characters,
 * ^U & delete.
 *
 * Fred Canter 9/29/82
 */

static char Sccsid[] = "@(#)login.c	3.1	10/12/87";
#include <sys/types.h>
#include <sgtty.h>
#include <utmp.h>
#include <signal.h>
#include <pwd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/param.h>

#define	PATH	"PATH=:/usr/ucb:/bin:/usr/bin"
#define	JCLCSH	"/bin/csh"	/* job control shell, needs new line disc. */
#define	UMASK	002

#define SCPYN(a, b)	strncpy(a, b, sizeof(a))
#define	NMAX	sizeof(utmp.ut_name)
#define FALSE	0
#define TRUE	-1
#define	CNTL(x) ('x'&037)
#define	UNDEF	'\0377'

char	maildir[30] =	"/usr/spool/mail/";
char	qlog[] = ".hushlogin";
struct	passwd nouser = {"", "nope"};
struct	sgttyb ttyb;
struct	utmp utmp;
char	minusnam[16] = "-";
/*
 * This bounds the time given to login.  We initialize it here
 * so it can be patched on machines where it's too small.
 */
int     timeout = 60;

char	homedir[64] = "HOME=";
char	term[64] = "TERM=";
char	shell[64] = "SHELL=";
char	user[NMAX+9] = "USER=";
char	*envinit[] = {homedir, PATH, term, shell, user, 0};
struct	passwd *pwd;
char	*ttyn;

struct	passwd *getpwnam();
char	*strcat();
int	setpwent();
int	timedout();
char	*ttyname();
char	*crypt();
char	*getpass();
char	*rindex(), *index();
extern	char **environ;

#define FSTTY

#ifdef FSTTY
#define ERASE	0177	
#define KILL	CNTL(U)
#else
#define ERASE	'#'
#define KILL	'@'
#endif FSTTY

#define EOT	CNTL(d)

struct	tchars	tc = {
	CNTL(c), CNTL(\\), CNTL(q), CNTL(s), CNTL(d), CBRK, CMIN, CTIME
};
struct	ltchars ltc = {
	CNTL(z), CNTL(y), CNTL(r), CNTL(o), CNTL(w), CNTL(v), UNDEF, UNDEF
};

int	intrflg;
int	rflag;
char	rusername[NMAX+1], lusername[NMAX+1];
char	rpassword[NMAX+1];
char	name[NMAX+1];
char	*rhost;
#ifdef	NEWLIMITS
unsigned sumcheck = 0;
#endif	NEWLIMITS

main(argc, argv)
char **argv;
{
	register char *namep;
	int pflag = 0;
	int t, f, c;
	int csole, slowt;
	int tflg;
	int ldisc, zero = 0;
	int invalid;
	int quietlog;

	signal(SIGALRM, timedout);
	alarm(timeout);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	nice(-100);
	nice(20);
	nice(0);

        /*
         * -p is used by getty to tell login not to destroy the environment
         * -r is used by rlogind to cause the autologin protocol;
         * -h is used by other servers to pass the name of the
         * remote host to login so that it may be placed in utmp and wtmp
         */
        if (argc > 1) {
                if (strcmp(argv[1], "-r") == 0) {
                        rflag = dormtlogin(argv[2]);
                        SCPYN(utmp.ut_host, argv[2]);
                        argc = 0;
                }
                if (strcmp(argv[1], "-h") == 0 && getuid() == 0) {
                        SCPYN(utmp.ut_host, argv[2]);
                        argc = 0;
                }
                if (strcmp(argv[1], "-p") == 0) {
                        argv++;
                        pflag = 1;
                        argc -= 1;
                }
	}
	if (rflag)
		ioctl(0, TIOCLSET, &zero);
	ioctl(0, TIOCNXCL, 0);
	ioctl(0, FIONBIO, &zero);
	ioctl(0, FIOASYNC, &zero);
	ioctl(0, TIOCGETP, &ttyb);
	ttyb.sg_erase = ERASE; 
	ttyb.sg_kill = KILL; 
	/*
	 * If talking to an rlogin process,
	 * propagate the terminal type and
	 * baud rate across the network.
	 */
	if (rflag) {
		doremoteterm(term, &ttyb);
		ioctl(0, TIOCSLTC, &ltc);
		ioctl(0, TIOCSETC, &tc);
	}
	ioctl(0, TIOCSETP, &ttyb);

	for (t=3; t< NOFILE; t++)
		close(t);
	ttyn = ttyname(0);
	if (ttyn==0)
		ttyn = "/dev/tty??";
	t = 0;
        do {
                ldisc = NTTYDISC;
                ioctl(0, TIOCSETD, &ldisc);
                invalid = FALSE;
                SCPYN(utmp.ut_name, "");
                /*
                 * Name specified, take it.
                 */
                if (argc > 1) {
                        SCPYN(utmp.ut_name, argv[1]);
                        argc = 0;
                }
                /*
                 * If remote login take given name,
                 * otherwise prompt user for something.
                 */
                if (rflag) {
                        SCPYN(utmp.ut_name, lusername);
                        /* autologin failed, prompt for passwd */
                        if (rflag == -1)
                                rflag = 0;
                } else
                        getloginname(&utmp);
                /*
                 * If no remote login authentication and
                 * a password exists for this user, prompt
                 * for one and verify it.
                 */
                if (!rflag && *pwd->pw_passwd != '\0') {
                        char *pp;

                        pp = getpass("Password:");
                        namep = crypt(pp, pwd->pw_passwd);
                        if (strcmp(namep, pwd->pw_passwd))
                                invalid = TRUE;
                }
		
		/* 
		 * shutdown in progress ?
                 * If user not super-user, check for logins disabled.
		 */
		if(access("/etc/sdloglock", 0) == 0 ||	
		   access("/etc/loglock",0) == 0 && pwd->pw_gid != 1 &&
			 pwd->pw_uid != 0)
			{
			printf("No Logins\n");
			exit(0);
			}

                if (invalid) {
                        printf("Login incorrect\n");
                        if (++t >= 5) {
                                ioctl(0, TIOCHPCL, (struct sgttyb *) 0);
                                close(0);
                                close(1);
                                close(2);
                                sleep(10);
                                exit(1);
                        }
                }
#ifdef	NEWLIMITS
		else if (lim(0) < 0) {
			printf("\nUser login limit exceeded,\n");
			printf("please try again later!\n");
			exit(0);
		}
#endif	NEWLIMITS
                if (*pwd->pw_shell == '\0')
                        pwd->pw_shell = "/bin/sh";
                if (!strcmp(pwd->pw_shell, JCLCSH)) {
                        ldisc = NTTYDISC;
                        ioctl(0, TIOCSETD, &ldisc);
		} else {
			ltc.t_suspc = ltc.t_dsuspc = UNDEF;
			ioctl(0, TIOCSLTC, &ltc);
		}
                if (chdir(pwd->pw_dir) < 0 && !invalid ) {
                        if (chdir("/") < 0) {
                                printf("No directory!\n");
                                invalid = TRUE;
                        } else {
                                printf("No directory! %s\n",
                                   "Logging in with home=/");
                                pwd->pw_dir = "/";
                        }
                }
                /*
                 * Remote login invalid must have been because
                 * of a restriction of some sort, no extra chances.
                 */
                if (rflag && invalid)
                        exit(1);
        } while (invalid);

	time(&utmp.ut_time);
	t = ttyslot();

	if (t>0 && (f = open(UTMP_FILE, 1)) >= 0) {
		lseek(f, (long)(t*sizeof(utmp)), 0);
		SCPYN(utmp.ut_line, index(ttyn+1, '/')+1);
		write(f, (char *)&utmp, sizeof(utmp));
		close(f);
	}
	if (t>0 && (f = open(WTMP_FILE, 1)) >= 0) {
		lseek(f, 0L, 2);
		write(f, (char *)&utmp, sizeof(utmp));
		close(f);
	}
	chown(ttyn, pwd->pw_uid, pwd->pw_gid);
	setgid(pwd->pw_gid);
	setuid(pwd->pw_uid);
	if (!term[5])
		getterm();
	environ = envinit;
	strncat(homedir, pwd->pw_dir, sizeof(homedir)-6);
	strncat(user, pwd->pw_name, sizeof(user)-6);
	if ((namep = rindex(pwd->pw_shell, '/')) == NULL)
		namep = pwd->pw_shell;
	else
		namep++;
	strncat(shell, pwd->pw_shell, sizeof(shell)-7);
	strcat(minusnam, namep);
	alarm(0);
	umask(UMASK);
	quietlog = (access(qlog,0) == 0);
	if (!quietlog)
		showmotd();
	if(access("/etc/loglock",0) == 0)
		printf("LOGINS DISABLED\n");
	strcat(maildir, pwd->pw_name);
	if(!quietlog && (access(maildir,4)==0)) {
		struct stat statb;
		stat(maildir, &statb);
		if (statb.st_size)
			printf("You have mail.\n");
	}
	signal(SIGQUIT, SIG_DFL);
	signal(SIGINT, SIG_DFL);
#ifdef	SIGTSTP
	signal(SIGTSTP, SIG_IGN);
#endif
	execlp(pwd->pw_shell, minusnam, 0);
	printf("No shell\n");
	exit(0);
}

getloginname(up)
        register struct utmp *up;
{
        register char *namep;
        char c;
	int logint();

	intrflg = 0;
	signal(SIGINT,logint);
        while (up->ut_name[0] == '\0') {
                namep = up->ut_name;
                printf("login: ");
                while ((c = getchar()) != '\n') {
			if (intrflg) {
				intrflg = 0;
				printf("\n");
				break;
			}
                        if (c == ' ')
                                c = '_';
                        if (c == EOF)
                                exit(0);
                        if (namep < up->ut_name+NMAX)
                                *namep++ = c;
                }
        }
        strncpy(lusername, up->ut_name, NMAX);
        lusername[NMAX] = 0;
	signal(SIGINT,SIG_IGN);

	setpwent();
        if ((pwd = getpwnam(lusername)) == NULL) 
                pwd = &nouser;
	endpwent();
}

logint()
{
	int logint(); signal(SIGINT, logint);
	intrflg++;
}

timedout()
{

        printf("\nLogin timed out after %d seconds\n", timeout);
        exit(0);
}

int	stopmotd;
catch()
{
	signal(SIGINT, SIG_IGN);
	stopmotd++;
}

showmotd()
{
	FILE *mf;
	register c;

	signal(SIGINT, catch);
	if((mf = fopen("/etc/motd","r")) != NULL) {
		while((c = getc(mf)) != EOF && stopmotd == 0)
			putchar(c);
		fclose(mf);
	}
	signal(SIGINT, SIG_IGN);
}
/*
 * doremotelogin
 */

dormtlogin(host)
        char *host;
{
        FILE *hostf;
        int first = 1;
	char temp_term[64];

        getstr(rusername, sizeof (rusername), "remuser");
        getstr(lusername, sizeof (lusername), "locuser");
        getstr(temp_term, sizeof(temp_term), "Terminal type");
	
	strcat(term,temp_term);
        if (getuid()) {
                pwd = &nouser;
                goto bad;
        }
        pwd = getpwnam(lusername);
        if (pwd == NULL) {
                pwd = &nouser;
                goto bad;
        }
        hostf = pwd->pw_uid ? fopen("/etc/hosts.equiv", "r") : 0;
again:
        if (hostf) {
                char ahost[32];

                while (fgets(ahost, sizeof (ahost), hostf)) {
                        char *user;

                        if ((user = index(ahost, '\n')) != 0)
                                *user++ = '\0';
                        if ((user = index(ahost, ' ')) != 0)
                                *user++ = '\0';
                        if (!strcmp(host, ahost) &&
                            !strcmp(rusername, user ? user : lusername)) {
                                fclose(hostf);
                                return (1);
                        }
                }
                fclose(hostf);
        }
        if (first == 1) {
                char *rhosts = ".rhosts";
                struct stat sbuf;

                first = 0;
                if (chdir(pwd->pw_dir) < 0)
                        goto again;
                if (stat(rhosts, &sbuf) < 0)
                        goto again;
                hostf = fopen(rhosts, "r");
                fstat(fileno(hostf), &sbuf);
                if (sbuf.st_uid && sbuf.st_uid != pwd->pw_uid) {
                        printf("login: Bad .rhosts ownership.\r\n");
                        fclose(hostf);
                        goto bad;
                }
                goto again;
        }
bad:
        return (-1);
	
}
getstr(buf, cnt, err)
        char *buf;
        int cnt;
        char *err;
{
        char c;

        do {
                if (read(0, &c, 1) != 1)
                        exit(1);
                if (--cnt < 0) {
                        printf("%s too long\r\n", err);
                        exit(1);
                }
                *buf++ = c;
        } while (c != 0);
}
char    *speeds[] =
    { "0", "50", "75", "110", "134", "150", "200", "300",
      "600", "1200", "1800", "2400", "4800", "9600", "19200", "38400" };
#define NSPEEDS (sizeof (speeds) / sizeof (speeds[0]))

doremoteterm(term, tp)
        char *term;
        struct sgttyb *tp;
{
        char *cp = index(term, '/');
        register int i;

        if (cp) {
                *cp++ = 0;
                for (i = 0; i < NSPEEDS; i++)
                        if (!strcmp(speeds[i], cp)) {
                                tp->sg_ispeed = tp->sg_ospeed = i;
                                break;
                        }
        }
        tp->sg_flags = ECHO|CRMOD|ANYP|XTABS;
}
/*
 * make a reasonable guess as to the kind of terminal the user is on.
 * We look in /etc/ttytype for this info (format: each line has two
 * words, first word is a term type, second is a tty name), and default
 * to "unknown" if we can't find any better.  In the case of dialups we get
 * names like "dialup" which is a lousy guess but tset can
 * take it from there.
 */
getterm()
{

	register char	*sp, *tname;
	register int	i;
	register FILE	*fdes;
	char		*type, *t;
	char		ttline[64];

	if ((fdes = fopen("/etc/ttytype", "r")) == NULL) {
unknown:
		strcat(term, "unknown");
		fclose(fdes);
		return;
	}
	for (tname = ttyn; *tname++; )
		;
	while (*--tname != '/')
		;
	tname++;
	while (fgets(ttline, sizeof(ttline), fdes) != NULL) {
		ttline[strlen(ttline)-1] = 0;	/* zap \n on end */
		type = ttline;
		for (t=ttline; *t && *t!=' ' && *t != '\t'; t++)
			;
		*t++ = 0;
		/* Now have term and type pointing to the right guys */
		if (strcmp(t, tname) == 0) {
			strcat(term, type);
			fclose(fdes);
			return;
		}
	}
	goto unknown;
}

