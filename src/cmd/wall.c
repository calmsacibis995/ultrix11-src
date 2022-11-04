
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)wall.c	3.1	6/11/87";
#include <stdio.h>
#include <utmp.h>
#include <signal.h>
#include <time.h>
#define	USERS	101
#define IGNOREUSER	"sleeper"
#ifdef	pdp11
#define	gethostname	ghostname
#endif	pdp11

char	hostname[32];
char	mesg[3000];
int	msize,sline;
struct	utmp utmp[USERS];
char	*strcpy();
char	*strcat();
char who[9] = "???";
long	clock, time();
struct tm *localtime();
struct tm *localclock;

main(argc, argv)
char *argv[];
{
	register i;
	register char c;
	register struct utmp *p;
	FILE *f;
	FILE *mf;

	gethostname(hostname, sizeof (hostname));
	if((f = fopen(UTMP_FILE, "r")) == NULL) {
		fprintf(stderr, "Cannot open %s\n",UTMP_FILE);
		exit(1);
	}
	clock = time( 0 );
	localclock = localtime( &clock );
	mf = stdin;
	if(argc >= 2) {
		if((mf = fopen(argv[1], "r")) == NULL) {
			fprintf(stderr,"Cannot open %s\n", argv[1]);
			exit(1);
		}
	}
	while((i = getc(mf)) != EOF) {
		if (msize >= sizeof mesg) {
			fprintf(stderr, "Message too long\n");
			exit(1);
		}
		mesg[msize++] = i;
	}
	fclose(mf);
	sline = ttyslot(2); /* 'utmp' slot no. of sender */
	fread((char *)utmp, sizeof(struct utmp), USERS, f);
	fclose(f);
	if (sline)
		strncpy(who, utmp[sline].ut_name, sizeof(utmp[sline].ut_name));
	for(i=0; i<USERS; i++) {
		p = &utmp[i];
		if ((p->ut_name[0] == 0) ||
		    (strncmp (p->ut_name, IGNOREUSER, sizeof(p->ut_name)) == 0))
			continue;
		sleep(1);
		sendmes(p->ut_line);
	}
	exit(0);
}

sendmes(tty)
char *tty;
{
	register i;
	char t[50], buf[BUFSIZ];
	register char *cp;
	register int c, ch;
	FILE *f;

	while ((i = fork()) == -1)
		if (wait((int *)0) == -1) {
			fprintf(stderr, "Try again\n");
			return;
		}
	if(i)
		return;
	strcpy(t, "/dev/");
	strcat(t, tty);

	signal(SIGALRM, SIG_DFL);	/* blow away if open hangs */
	alarm(10);

	if((f = fopen(t, "w")) == NULL) {
		fprintf(stderr,"cannot open %s\n", t);
		exit(1);
	}
	setbuf(f, buf);
	fprintf(f,
	    "\nBroadcast Message from %s!%s (%.*s) at %d:%02d ...\r\n\n"
		, hostname
		, who
		, sizeof(utmp[sline].ut_line)
		, utmp[sline].ut_line
		, localclock -> tm_hour
		, localclock -> tm_min
	);
	for (cp = mesg, c = msize; c-- > 0; cp++) {
		ch = *cp;
		if (ch == '\n')
			putc('\r', f);
		putc(ch, f);
	}
	exit(0);
}
