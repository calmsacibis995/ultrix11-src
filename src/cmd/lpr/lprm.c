
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)lprm.c	3.0	4/21/86";
/* Based on: (2.9BSD)	lprm.c	4.3	81/08/06	*/
/*
 * lprm - dequeue the current user's spool entry
 *
 * lprm [-] [pid] [user]
 *
 * Using information in the lock file, lprm will kill the
 * currently active daemon (if necessary), remove the associated files,
 * startup a new daemon.  Priviledged users may remove anyone's spool
 * entries, otherwise one can only remove their own.
 *
 */

#include	<stdio.h>
#include	<signal.h>
#include	<sys/types.h>
#include	<sys/dir.h>
#include	<pwd.h>
#include	<sys/stat.h>
#include	<ctype.h>
#include	<errno.h>
#include	"lp.local.h"

#define	MAXUSERS	50
#define	MAXREQUESTS	30

/*
 * Stuff for handling lprm specifications
 */
struct	passwd	user[MAXUSERS];		/* users to process */
struct	direct	*pdir;			/* used in perusing SPOOLDIR */
int		users;			/* # of users in user array */
int		cur_daemon;		/* daemon's pid */
int		cur_pid;		/* active daemon file pid */
int		requ[MAXREQUESTS];	/* pid of spool entries */
int		requests;		/* # of spool requests */
int		all = 0;		/* eliminate everything ? */

/*
 * Stuff for printcap (a la termcap) description
 */
char	*SD;
char	*LO;
char	*DN;
char	*AF;
char	*printer;
char	*pgetstr();

struct passwd *getpwnam(), *getpwuid();
char	*malloc();
char	*realloc();
char	*getenv();
char	*malloc();
char	*rindex();

main(argc, argv)
	char *argv[];
{
	register int nitems, garbage = 0;
	register struct passwd *p;
	register int nargs = argc;
	int assasinated = 0;

	argv++;
	while (argc > 1) {
		if (argv[0][0] == '-') {
			if (argv[0][1] == 'P') {
				if (!chkprinter(&argv[0][2]))
					fatal("%s: unknown printer", &argv[0][2]);
				printer = &argv[0][2];
			} else {
				if (users)
					usage();
				users = -1;
			}
		} else {
			if (users < 0)
				usage();
			if (isdigit(argv[0][0])) {
				if (requests >= MAXREQUESTS)
					fatal("too many requests to lprm");
				requ[requests++] = atoi(*argv);
			} else {
				if (users >= MAXUSERS)
					fatal("too many users");
				p = getpwnam(*argv);
				if (p)
					user[users++] = *p;
				else
					fatal("%s: unknown user", *argv);
			}
		}
		argc--;
		argv++;
	}
	/*
	 * If the format was `lprm -' and the user isn't super-user,
	 *  then fake things to look like he said `lprm user'.
	 */
	if (users < 0) {
		int uid;

		if ((uid = getuid()) != 0) {
			if ((p = getpwuid(uid)) == NULL)
			/*	fatal("who are you?");	*/
				fatal("cannot get your user ID");
			user[0] = *p;
			users = 1;
		} else
			all = 1;
	}
	/*
	 * Build a picture of the spool q
	 */
	if (printer == NULL) {
		if ((printer = getenv("PRINTER")) == NULL)
			printer = DEFLP;
		if (!chkprinter(printer))
			fatal("%s: unknown printer", printer);
	}
	if (chdir(SD) < 0)
		fatal("cannot chdir to spooling area");
	if ((nitems = collect()) == 0) {
		printf("no entries\n");
		exit(0);
	}
	garbage = lockchk(LO);
	/*
	 * If we have garbage, then just remove the files,
	 *  otherwise check first for an active daemon
	 *  (in which case we kill it off it is reading our file)
	 *  then remove stuff (after which we have to restart the
	 *  daemon).
	 */
	if (!garbage) {
		char file[15];

		sprintf(file, "dfA%05d", cur_pid);
		if ((nargs == 1 && access(file, 4) == 0) || chk(file, cur_pid))
		{
			requ[requests++] = cur_pid;
			assasinated = (kill(cur_daemon, SIGTERM) == 0);
			if (!assasinated)
				fatal("cannot kill daemon");
		}
	}
	for (nargs = 0; nargs < nitems; nargs++)
		process(&pdir[nargs]);
	if (!garbage && assasinated) {
		unlink(LO);
		execl(DN, rindex(DN, '/') ? rindex(DN, '/')+1 : DN, printer, 0);
		fatal("cannot restart daemon");
	}
}

/*
 * Process a lock file: collect the pid of the active
 *  daemon and the pid of the active spool entry.
 * Return boolean indicating non-existence of a lock file.
 */
lockchk(s)
	char *s;
{
	register int fd;
	register int i;

	if ((fd = open(s, 0)) < 0)
		if (errno == EACCES)
			fatal("cannot access lock file");
		else
			return(1);

	if (read(fd, (char *)&cur_daemon, sizeof(int)) != sizeof(int))
	{
		fatal("bad lock file (cannot read current daemon pid)");
	}

	/*
	 * We used to wait for upto (1+2+3+...+17+18+19+20 = 3.5mins)
	 * before deciding that the current pid does not exist.
	 * We now hang around only 2 seconds before returning with
	 * a dead cur_pid. (-1).
	 */
	for (i = 1; read(fd, (char *)&cur_pid, sizeof(int)) != sizeof(int); i++)
	{
		if (i > 2) {		/* was 20 */
			cur_pid = -1;	/* choose impossible pid */
			break;
		}
		sleep(1);		/* was sleep(i) */
	}
	close(fd);
	return(0);
}

/*
 * Process the spool q: build an in-core table of the directory
 *  for use in deciding what gets removed.
 */
collect()
{
	register int fd, items = 0;
	register char c1, c2;

	if ((fd = open(".", 0)) < 0)
		fatal("cannot access spooling directory");
	/*
	 * skip . and ..
	 */
	lseek(fd, (long)(2*sizeof(struct direct)), 0);
	pdir = (struct direct *)malloc(sizeof(struct direct));
	while(1) {
		register struct direct *proto;

		proto = &pdir[items];
		if (read(fd, (char *)proto, sizeof(*proto)) != sizeof(*proto))
			break;
		if (proto->d_ino == 0)
			continue;
		c1 = proto->d_name[0], c2 = proto->d_name[1];
		if (c1 != 't' && c1 != 'c' && c1 != 'd' && c1 != 'l')
			continue;
		if (c2 != 'f')
			continue;
		items++;
		proto = (struct direct *)realloc((char *)pdir, (items+1)*sizeof(struct direct));
		if (proto == NULL) {
			printf("out of memory, only handling %d items\n", items);
			break;
		}
		pdir = proto;
	}
	return(items);
}

/*
 * Check through the requests and users to see if this
 * directory entry should be removed.
 */
process(p)
	register struct direct *p;
{
	if (all || chk(p->d_name, atoi(p->d_name+3)))
		printf(unlink(p->d_name) ? "cannot dequeue %s\n" : "%s dequeued\n",
			p->d_name);
}
/*
 * Do the dirty work in checking
 */
chk(file, pid)
	char *file;
	int pid;
{
	struct stat buf;
	register struct passwd *q;
	register int *r;

	/*
	 * Check the request list
	 */
	for (r = requ; r < &requ[requests]; r++)
		if (*r == pid && access(file, 4) == 0)
			return(1);
	if (stat(file, &buf) < 0)
		return(0);
	/*
	 * Check to see if it's in the user list
	 */
	for (q = user; q < &user[users]; q++)
		if (q->pw_uid == buf.st_uid && access(file, 4) == 0)
			return(1);
	return(0);
}

fatal(s, a)
	char *s, *a;
{
	fprintf(stderr, "lprm: fatal error: ");
	fprintf(stderr, s, a);
	fputc('\n', stderr);
	exit(1);
}

usage()
{
	printf("usage: lprm [-] [-Pprinter] [[job #] [user] ...]\n");
	exit(2);
}

chkprinter(s)
char *s;
{
	static char buf[BUFSIZ/2];
	char b[BUFSIZ];
	int stat;
	char *bp = buf;

	if ((stat = pgetent(b, s)) < 0)
		fatal("cannot open printer description file");
	else if (stat == 0)
		return(0);
	if ((DN = pgetstr("dn", &bp)) == NULL)
		DN = DEFDAEMON;
	if ((SD = pgetstr("sd", &bp)) == NULL)
		SD = DEFSPOOL;
	if ((LO = pgetstr("lo", &bp)) == NULL)
		LO = DEFLOCK;
	AF = pgetstr("af", &bp);
	return(1);
}
