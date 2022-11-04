
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)lpq.c	3.0	4/21/86";
/* 2.9BSD   ??? (est ~5/81) 	lpq.c	3.0	86/04/21	*/
/*
 * Spool Queue examination program
 *
 * lpq [+[n]] [-Pprinter] [users...]
 *
 * + =>'s continually scan q until empty
 * -P used to identify printer as per lpr/lpd
 */
#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/dir.h>
#include	<pwd.h>
#include	<sgtty.h>
#include	<sys/stat.h>
#include	<ctype.h>
#include	"lp.local.h"

#define	MAXUSERS	50
#define	MAXJOBS		50
#define	READ		0
#define	DEFTIME		10		/* default sleep interval */
#define	JOBCOL		40		/* column for job # in -l format */
#define	OWNCOL		7		/* start of Owner column in normal */

struct	passwd user[MAXUSERS];		/* when users are specified */
struct	direct *pdir;			/* for sorting stuff */

int	jobs[MAXJOBS];			/* jobs specified on command line */
int	njobs = 0;			/* # of entries in jobs */
int	users;				/* # of users in user or < 0 */
int	current_pid;			/* current daemon file pid */
int	garbage = 0;			/* # of garbage df files */
int	rank = 0;			/* order to be printed */
int	slptime = DEFTIME;		/* pause between screen refereshes */
int	repeat = 0;			/* + flag indicator */
int	col;				/* column on screen */
int	lflag = 0;			/* long output option */
int	first;				/* first file in ``files'' column ? */
int	SIZCOL = 62;			/* start of Size column in normal */

char	line[132];			/* input line from daemon file */
char	file[132];			/* print file name guess */
char	ufile[132];			/* unlink file */
char	*head0 = "Rank   Owner      Job #  Files";
char	*head1 = "Total Size\n";

long	totsize = 0;			/* total print job size in bytes */

/*
 * Printcap (a la termcap) stuff for mutiple printers
 */
char	*SD;				/* spooling directory */
char	*LO;				/* name of lock file */
char	*LP;				/* line printer name */
char	*pgetstr();

struct passwd *getpwnam(), *getpwuid();
char	*index();
char	*getenv();

main(argc, argv)
	char *argv[];
{
	register struct passwd *p;
	register int n;

	argv++;
	while (argc > 1) {
		if (argv[0][0] == '+') {
			if (argv[0][1] != '\0')
				if ((slptime = atoi(&argv[0][1])) < 0)
					slptime = DEFTIME;
			repeat++;
		} else if (argv[0][0] == '-')
			switch(argv[0][1]) {

			case 'P':		/* printer name */
				if (!chkprinter(&argv[0][2]))
					fatal("%s: unknown printer", &argv[0][2]);
				break;

			case 'l':		/* long output */
				lflag++;
				break;

			default:
				usage();
		} else {
			if (isdigit(argv[0][0])) {
				if (njobs >= MAXJOBS)
					fatal("too many jobs requested");
				jobs[njobs++] = atoi(argv[0]);
			} else {
				if (users >= MAXUSERS)
					fatal("too many users");
				p = getpwnam(*argv);
				if (p)
					user[users++] = *p;
				else
					printf("unknown user %s\n", *argv);
			}
		}
		argc--;
		argv++;
	}
	if (!users && !njobs)
		users = -1;
	if (SD == NULL) {
		char *pr;

		if ((pr = getenv("PRINTER")) == NULL)
			pr = DEFLP;
		if (!chkprinter(pr))
			fatal("%s: unknown printer", pr);
	}
	if (chdir(SD) < 0)
		fatal("can't chdir to spooling area");

	if (repeat)
		do {
			if ((n = display()) > 0) {
				sleep(slptime);
				rank = 0;
			}
		} while (n > 0);
	else
		display();
}

/*
 * Display the current state of the q
 */
display()
{
	register int i, nitems;
	int spfd;
	struct stat statb;
	int dcomp();

	/*
	 * Find all the spool files in the spooling directory
	 */
	if ((spfd = open(".", READ)) < 0)
		fatal("can't examine spooling area");
	lseek(spfd, (long)(2*sizeof(struct direct)), 0);
	pdir = (struct direct *)malloc(sizeof(struct direct));
	nitems = 0;
	while (1) {
		struct direct *proto;

		proto = &pdir[nitems];
		if (read(spfd, (char *)proto, sizeof(*proto)) != sizeof(*proto))
			break;
		if (proto->d_ino == 0 || proto->d_name[0] != 'd' ||
		    proto->d_name[1] != 'f')
			continue;	/* just daemon files */
		nitems++;
		proto = (struct direct *)realloc((char *)pdir,
				(nitems+1)*sizeof(struct direct));
		if (proto == NULL) {
			fprintf(stdout, "out of memory, only showing %d jobs\n", nitems);
			break;
		}
		pdir = proto;
	}
	if (nitems == 0) {
		printf("no entries\n");
		return(0);
	}
	close(spfd);
	if ((spfd = open(LO, READ)) < 0)
		garbage = nitems;
	else {
		lseek(spfd, (long)sizeof(int), 0);	/* skip daemon id */
		if (read(spfd, (char *)&current_pid, sizeof(int)) != sizeof(int))
			current_pid = -1;		/* should be invalid */
		close(spfd);
	}
	qsort(pdir, nitems, sizeof(struct direct), dcomp);
	/*
	 * Now, examine the daemon files and print out the jobs to
	 * be done for each user
	 */
	if (!lflag && garbage < nitems)
		header();
	for (i = garbage; i < nitems; i++)
		inform(pdir[i].d_name, 0);

	/*
	 * What's left is garbage, inform the user
	 */
	if (garbage > 0) {
		register short down = stat(LP, &statb) >= 0 &&
					(statb.st_mode&0777) == 0;
		fprintf(stdout, down ? "Warning: printer down" :
				"Warning: no daemon present");
		putchar('\n');
		if (!lflag)
			header();
		for (i = 0; i < garbage; i++)
			inform(pdir[i].d_name, 1);
	}
	return(nitems-garbage);
}

/*
 * Print the header for the short listing format
 */
header()
{
	printf(head0);
	col = strlen(head0)+1;
	blankfill(SIZCOL);
	printf(head1);
}

dcomp(d1, d2)
	register struct direct *d1, *d2;
{
	return(strncmp(d1->d_name, d2->d_name, DIRSIZ));
}

inform(df, garb)
	char *df;
{
	register int j, k;
	register struct passwd *p;
	FILE *fd;
	int spfd, dfpid = atoi(df+3);
	char *owner;
	struct stat buf;

	/*
	 * There's a chance the daemon file has gone away
	 * in the meantime; if this is the case just keep going
	 */
	if ((spfd = open(df, READ)) < 0)
		return;

	fstat(spfd, &buf);
	/*
	 * Was this file specified in the user's list
	 */
	for (j = 0; j < users; j++)
		if (user[j].pw_uid == buf.st_uid)
			break;
	if (j >= users) {			/* scan jobs list */
		for (k = 0; k < njobs; k++)
			if (dfpid == jobs[k])
				break;
	} else
		k = njobs;
	if (users < 0 || j < users || k < njobs) {	/* found one */
		if (lflag)
			putchar('\n');
		col = 0;
		if (users < 0 || k < njobs)
			owner = (p = getpwuid(buf.st_uid)) == NULL ? "???" :
					p->pw_name;
		else if ((owner = user[j].pw_name) == NULL)
			owner = "???";
		if (lflag)
			col += strlen(owner);
		if (!garb && dfpid == current_pid)
			rank = 0;
		else
			rank++;
		if (lflag) {
			printf("%s: ", owner), col += 2;
			prank(rank);
			blankfill(JOBCOL);
			printf(" [job #%d]\n", dfpid);
		} else {
			prank(rank);
			blankfill(OWNCOL);
			printf("%-10s %-5d  ", owner, dfpid), col += 18;
			first = 1;
		}
			
		/*
		 * Now list the files associated with the
		 * print job
		 */
		fd = fdopen(spfd, "r");
		j = 0;			/* # of copies of a file */
		*file = *ufile = '\0';

		while (fgets(line, sizeof(line), fd) != NULL) {

			switch(line[0]) {

			default:
				continue;
			case 'N':
				if (*file) {
					/*
					 * Could miss a file which has been
					 *   linked...
					 */
					if (strcmp(file, line+1)) {
						show(file, file, j);
						j = 0;
						if (strcmp(line+1, " \n"))
							strcpy(file, line+1);
						continue;
					}
					j++;
				} else if (strcmp(line+1, " \n"))
					strcpy(file, line+1);
				k++;
				continue;
			case 'U':
				strcpy(ufile, line+1);
				break;
				
			}
			show(file, ufile, j);
			j = 0;
		}
		fclose(fd);
		if (*file)			/* there's a file left over */
			show(file, ufile, j);
		if (!lflag) {
			blankfill(SIZCOL);
			printf("%D bytes\n", totsize);
			totsize = 0;
		}
	} else
		rank++;
	close(spfd);
}

show(file, ufile, copies)
	register char *ufile, *file;
{
	register char *t;

	if ((t = index(file, '\n')) == NULL)
		strcpy(file, "(standard input)");
	else
		*t = '\0';
	if ((t = index(ufile, '\n')) != NULL)
		*t = '\0';
	if (lflag)
		ldump(file, *ufile ? ufile : file, copies);
	else
		dump(file, *ufile ? ufile : file, copies);
	*ufile = *file = '\0';
}

/*
 * Fill the line with blanks to the specified column
 */
blankfill(n)
	register int n;
{
	while (col++ < n)
		putchar(' ');
}

/*
 * Give the abbreviated dump of the file names
 */
dump(file, ufile, copies)
	char *file, *ufile;
{
	register short n, fill;
	struct stat lbuf;

	/*
	 * Print as many files as will fit
	 *  (leaving room for the total size)
	 */
	 fill = first ? 0 : 2;	/* fill space for ``, '' */
	 if (((n = strlen(file)) + col + fill) >= SIZCOL-4) {
		if (col < SIZCOL) {
			printf(" ..."), col += 4;
			blankfill(SIZCOL);
		}
	} else {
		if (first)
			first = 0;
		else
			printf(", ");
		printf("%s", file);
		col += n+fill;
	}
	if (*ufile && !stat(ufile, &lbuf))
		totsize += copies ? (copies+1)*lbuf.st_size : lbuf.st_size;
}

/*
 * Print the long info about the file
 */
ldump(file, ufile, copies)
	char *file, *ufile;
{
	struct stat lbuf;

	putchar('\t');
	if (copies)
		printf("%-2d copies of %-19s", copies+1, file);
	else
		printf("%-32s", file);
	if (*ufile) {
		if (!stat(ufile, &lbuf))
			printf(" %D bytes", lbuf.st_size);
		else
			printf(" ??? bytes");
	} else
		printf(" ??? bytes");
	putchar('\n');
}

/*
 * Print the job's rank in the queue,
 *   update col for screen management
 */
prank(n)
{
	char line[100];
	static char *r[] = {
		"th", "st", "nd", "rd", "th", "th", "th", "th", "th", "th"
	};

	if (n == 0) {
		printf("active");
		col += 6;
		return;
	}
	if ((n/10) == 1)
		sprintf(line, "%dth", n);
	else
		sprintf(line, "%d%s", n, r[n%10]);
	col += strlen(line);
	printf("%s", line);
}

usage()
{
	printf("usage: lpq [-l] [+[n]] [-Pprinter] [users...]\n");
	exit(1);
}

fatal(s, a)
	char *s;
{
	fprintf(stderr, "lpq: ");
	fprintf(stderr, s, a);
	putc('\n', stderr);
	exit(2);
}

/*
 * Interrogate the printer data base
 */
chkprinter(s)
	char *s;
{
	static char buf[BUFSIZ/2];
	char b[BUFSIZ];
	int stat;
	char *bp = buf;

	if ((stat = pgetent(b, s)) < 0)
		fatal("can't open description file");
	else if (stat == 0)
		return(0);
	if ((LP = pgetstr("lp", &bp)) == NULL)
		LP = DEFDEVLP;
	if ((SD = pgetstr("sd", &bp)) == NULL)
		SD = DEFSPOOL;
	if ((LO = pgetstr("lo", &bp)) == NULL)
		LO = DEFLOCK;
	return(1);
}
