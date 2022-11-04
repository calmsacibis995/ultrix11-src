
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)shutdown.c	3.1	8/25/87";
/*
	Program to shutdown the system gracefully

	July 9, 1980

	This program should reside in the /opr directory
*/
#include <stdio.h>
#include <a.out.h>
#include <core.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/tty.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/stat.h>
#include <sys/inode.h>
#include <sys/mount.h>
#include <utmp.h>
#include <ctype.h>

#define HOURS *3600
#define MINUTES *60
#define SECONDS *1
#define NOLOGTIME	5 MINUTES	/* Time to turn off logins */
#define HUPWAIT		30 SECONDS	/* Time between sending HUP and KILL */

char	hostname[32];
int	slptime=5;	/* time to sleep between HUP and KILL passes */

struct nlist nl[] = {
	{ "_proc" },
	{ "_swapdev" },
	{ "_swplo" },
	{ "_nproc" },
	{ "_rootdev" },
	{ "_nmount" },
	{ "_sepid" },
	{ "_mount" },
	{ "_acctp" },
#define			X_ACCTP		8
	{ "" },
};

struct mount mnt;

struct mdev {
	dev_t	mntdev;
	dev_t	mnton;
	char	mntnam[32];
	char	mntonnam[32];
}mdev[20];

struct ino {
	unsigned i_flag;
	unsigned i_count;
	dev_t	i_dev;
	ino_t	i_number;
}ino;

#include	<mtab.h>
struct mtab mtab[20];
/*	m_path m_dname */

struct	proc mproc;
struct	utmp	utmp[101];

FILE	*tin;
FILE	*tout;
char	ttys[132]; /* originally 20, is now bigger to allow for comments in /etc/ttys */
int	nproc;
long	lseek();
char	*strcat();
char	*strcpy();
char	*strcmp();
char	*strncmp();
char	*ttynam;
char	*ttyname();
int	mem;
int	swmem;
int	swap;
daddr_t	swplo;
int	rootdev, nmount, nmntdev, sepid, mf;
int	fwflag=0;
int	acctp;

int	ndev;
struct devl {
	char	dname[DIRSIZ];
	dev_t	dev;
} devl[256];

char	*coref;
int	deltim;
char	cnsle[]	= {"/dev/console"};
char	lokfil[]  = {"/etc/loglock"};
char	TTYS[]	= "/etc/ttys";
char	UTTYS[] = "/etc/uttys";
char	TTTYS[] = "/etc/tttys";
FILE	*df;

int	nlflag = 1;	/* Flag for nologins, 0 = nologins, 1 = logins */
int	stogo;
int	sint;
struct interval {
	int stogo;
	int sint;
} interval[] = {
	4 HOURS,	1 HOURS,
	2 HOURS,	30 MINUTES,
	1 HOURS,	15 MINUTES,
	30 MINUTES,	10 MINUTES,
	15 MINUTES,	5 MINUTES,
	9 MINUTES,	4 MINUTES,
	8 MINUTES,	4 MINUTES,
	7 MINUTES,	4 MINUTES,
	6 MINUTES,	3 MINUTES,
	5 MINUTES,	3 MINUTES,
	4 MINUTES,	2 MINUTES,
	3 MINUTES,	2 MINUTES,
	2 MINUTES,	1 MINUTES,
	1 MINUTES,	30 SECONDS,
	0 SECONDS,	0 SECONDS
};

char *shutter;

main(argc, argv)
char **argv;
{
	int	onintr(), earlyintr();
	int i;
	char *ap, c;
	int puid, pid, pppid, ppid;
	int incnt;
	char inbuf[80];
	extern int deltim;
	int notdone;

	signal(SIGINT, earlyintr);
	shutter = getlogin();
	gethostname(hostname, sizeof (hostname));
	printf("\nULTRIX-11 Shutdown\n");
	ttynam = ttyname(0);
	if (strcmp(cnsle, ttynam))
	{
		printf("\nShutdown should only be run from the console device\n");
		printf("Do you want to run from this terminal <y or n> ? ");
		gets(inbuf);
		if(inbuf[0] != 'y')
			exit(1);
	}
	printf("The following users are logged into the System\n\n");
	system("who");
	for(;;)
	{
		int bad = 0;
		printf("\nHow many minutes until shutdown [1-99] ? ");
		for (incnt = 0; (c = getchar()) != '\n' ; ++incnt) {
			if ( isdigit(c) )
				inbuf[incnt] = c;
			else if (! isdigit(c) )
				bad++, incnt--;
		}
		inbuf[incnt] = '\0';
		deltim = atoi(inbuf);
		if (bad)
			deltim = -1;
		if (incnt == 0 || deltim > 99 || deltim < 0)
		{
			printf("\nImproper time specified\n");
			continue;
		}
		else
			break;
	}
	signal(SIGINT, onintr);
	pid = getpid();
	if(chdir("/dev") < 0) {
		fprintf(stderr, "Can't change to /dev\n");
	errxit:
		unlink(&lokfil);
		exit(1);
	}
	nlist(argc>2? argv[2]:"/unix", nl);
	if (nl[0].n_type==0) {
		fprintf(stderr, "No namelist\n");
		goto errxit;
	}
	coref = "/dev/mem";
	if ((mem = open(coref, 0)) < 0) {
		fprintf(stderr, "No mem\n");
		goto errxit;
	}
	printf("\nWarning Phase\n\n");

	stogo = deltim * 60;
	for (;;)
	{
		deltim = stogo;
		if (stogo <= NOLOGTIME && nlflag ) {
			nlflag = 0;
			creat(&lokfil, 0); /* Turn off Logins */
		}
		sndall();
/*new*/
		for ( i = 0; stogo <= interval[i].stogo && interval[i].sint; i++) {
		    sint = interval[i].sint;
		}

		if (stogo <= 0) break;
		if (stogo > 0 && sint > 0) sleep(sint<stogo ? sint : stogo);
		stogo -= sint;
	}
	fwflag++;
	swmem = open(coref, 0);
	/*
	 * read mem to find swap dev.
	 */
	lseek(mem, (long)nl[1].n_value, 0);
	read(mem, (char *)&nl[1].n_value, sizeof(nl[1].n_value));
	/*
	 * Find base of swap
	 */
	lseek(mem, (long)nl[2].n_value, 0);
	read(mem, (char *)&swplo, sizeof(swplo));
	/*
	 * Locate proc table
	 */
	lseek(mem, (long)nl[0].n_value, 0);
	getdev();
	printf("\nKill Process Phase\n\n");
	while(mproc.p_pid != pid)	/* find this process id */
	{
		read(mem, (char *)&mproc, sizeof mproc);
	}
	ppid = mproc.p_ppid;		/* get parent pid */
	lseek(mem, (long)nl[0].n_value, 0);
	while(mproc.p_pid != ppid)	/* find this process id */
	{
		read(mem, (char *)&mproc, sizeof mproc);
	}
	pppid = mproc.p_ppid;
/* turn off all terminals and kill all user processes */
	printf("\tKilling User Processes\n");
	if ((tin = fopen(TTYS, "r")) == NULL) {
		printf("cannot open %s for reading!\n", TTYS);
	} else if ((tout = fopen(TTTYS, "w")) == NULL) {
		printf("cannot open %s for writing!\n", TTTYS);
	} else {
		while(fscanf(tin, "%s", &ttys[0]) != EOF) {
			if(strcmp(&ttys[2], cnsle+5)
			  && strcmp(&ttys[2], ttynam+5))
				ttys[0] = '0';
			fprintf(tout, "%s\n", &ttys[0]);
		}
		fclose(tin);
		fclose(tout);
		unlink(UTTYS);
		if (link(TTYS, UTTYS) == -1)
			printf("Can't link %s to %s\n", TTYS, UTTYS);
		else if (unlink(TTYS) == -1)
			printf("Can't unlink %s\n", TTYS);
		else if (link(TTTYS, TTYS) == -1) {
			printf("Can't link %s to %s\n", TTTYS, TTYS);
			if (link(UTTYS, TTYS) == -1)
			    printf("Can't link %s back to %s!\n",UTTYS, TTYS);
		} else if (unlink(TTTYS) == -1)
			printf("Can't unlink %s\n", TTTYS);
	}
	kill(1, 1);	/* signal init to re-read the /etc/ttys file */

	/*
	 * Make two passes through the table,
	 * send HUP first, wait 5 seconds, then send KILL.
	 */
	printf("\tKilling System Processes\n");
	notdone = SIGHUP;
	lseek(mem, (long)nl[3].n_value, 0);
	read(mem, &nproc, sizeof(nproc));
	while (notdone) {	/* make two passes through the table */
	    lseek(mem, (long)nl[0].n_value, 0);
	    for (i=0; i<nproc; i++)
	    {
		read(mem, (char *)&mproc, sizeof mproc);
		if (mproc.p_stat==0 || mproc.p_pid <= 2)
			continue;
		if (mproc.p_pgrp==0 && mproc.p_uid==0 && mproc.p_ppid==0)
			continue;
		if(mproc.p_pid==ppid || mproc.p_pid==pid || mproc.p_pid==pppid)
			continue;
		puid = mproc.p_pid;
		kill(puid, notdone);	/* notdone = SIGHUP or SIGKILL */
	    }
	    if (notdone == SIGHUP) {	/* just did HUP pass */
		sleep(slptime);
	        notdone = SIGKILL;
	    }
	    else if (notdone == SIGKILL)/* just did last pass (SIGKILL) */
		notdone = 0;		/* quit */
	    else
		notdone = SIGKILL;  /* shouldn't happen */
	}

	printf("\tDisabling Error Logging\n");
	system("/etc/eli -d");

/* Turn off accounting if it is configured and enabled. */
	if (nl[X_ACCTP].n_value) {
		lseek(mem, (long)nl[X_ACCTP].n_value, 0);
		read(mem, &acctp, sizeof(acctp));
		if (acctp) {
			printf("\nTurning off System Accounting\n\n");
			acct(0);
		}
	}

/* Dismount mounted filesystems */
	printf("\nDismounting Mounted File Systems\n\n");
	dodismount();
/* shut it down */
	if (unlink(TTYS) == -1)
		printf("Can't unlink temporary %s\n", TTYS);
	else if (link(UTTYS, TTYS) == -1)
		printf("Can't link %s to %s\n", UTTYS, TTYS);
	else if (unlink(UTTYS) == -1)
		printf("Can't unlink %s\n", UTTYS);
	printf("\nSystem Time-sharing Stopped\n");
	sync();
	exit(0);
}

getdev()
{
	struct stat sbuf;
	struct direct dbuf;

	if ((df = fopen("/dev", "r")) == NULL) {
		fprintf(stderr, "Can't open /dev\n");
		unlink(&lokfil);
		exit(1);
	}
	ndev = 0;
	while (fread((char *)&dbuf, sizeof(dbuf), 1, df) == 1) {
		if(dbuf.d_ino == 0)
			continue;
		if(stat(dbuf.d_name, &sbuf) < 0)
			continue;
		if ((sbuf.st_mode&S_IFMT) != S_IFCHR)
			continue;
		strcpy(devl[ndev].dname, dbuf.d_name);
		devl[ndev].dev = sbuf.st_rdev;
		ndev++;
	}
	fclose(df);
	if ((swap = open("/dev/swap", 0)) < 0) {
		fprintf(stderr, "Can't open /dev/swap\n");
		unlink(&lokfil);
		exit(1);
	}
}

getdevnam(dev, pnt)
dev_t dev;
int pnt;
{
	struct stat sbuf;
	struct direct dbuf;
	char *npnt;
	int cnt;

	fseek(df, 0L, 0);
	npnt = &mdev[pnt].mntnam;
	while (fread((char *)&dbuf, sizeof(dbuf), 1, df) == 1) {
		if(dbuf.d_ino == 0)
			continue;
		if(stat(dbuf.d_name, &sbuf) < 0)
			continue;
		if(sbuf.st_rdev != dev)
			continue;
		if((sbuf.st_mode & S_IFMT) != S_IFBLK)
			continue;  /* don't unmount non-block devices */
		strcpy(npnt, "/dev/");
		strcat(npnt, dbuf.d_name);
		for(cnt = 0; cnt < 20; cnt++){
			if(strcmp(dbuf.d_name, mtab[cnt].m_dname) == 0){
				strcpy(mdev[pnt].mntonnam, mtab[cnt].m_path);
				break;
			}
		}
	}
}

sndall()
{
	register i;
	register struct utmp *p;
	int hour, min;
	FILE *f;
	extern int deltim;

	if((f = fopen(UTMP_FILE, "r")) == NULL) {
		fprintf(stderr, "Cannot open %s\n",UTMP_FILE);
		unlink(&lokfil);
		exit(1);
	}
	fread((char *)utmp, sizeof(struct utmp), 101, f);
	fclose(f);
	for(i=0; i<101; i++) {
		p = &utmp[i];
		if(p->ut_name[0] == 0)
			continue;
		sleep(1);
		if(strcmp(ttynam+5, p->ut_line))
			sendmes(p->ut_line);
		else if(deltim == 0)
			printf("\tFINAL WARNING SENT\n");
		else if (deltim >=1 HOURS )
		{
			hour = ((deltim +20)/60)/60;
			min = (deltim / 60) - (hour * 60);
			if (min > 0)
			    printf("\t%d hour %d minute warning sent\n",hour,min);
			else
			    printf("\t%d hour warning sent\n",hour);
		}
		else if(deltim >= 1 MINUTES)
			printf("\t%d minute warning sent\n", deltim/60);
		else if (deltim >  0 SECONDS )
			printf("\t%d second warning sent\n",deltim);
		else
			printf("\n\7\7\7SHUTDOWN CANCELED\n");
	}
}

sendmes(tty)
char *tty;
{
	register i;
	char t[50], buf[BUFSIZ];
	FILE *f;
	char *ts;
	time_t sdt;
	extern int deltim;

	i = fork();
	if(i == -1) {
		fprintf(stderr, "Try again\n");
		return;
	}
	if(i)
		return;
	strcpy(t, "/dev/");
	strcat(t, tty);
	if((f = fopen(t, "w")) == NULL) {
		fprintf(stderr,"cannot open %s\n", t);
		exit(1);
	}
	setbuf(f, buf);
	fprintf(f, "Broadcast Message ...\n\n");
		fprintf(f,
		"\007\007\t*** System shutdown message ");
	if (shutter)		/*specifies who is doing the shutdown*/
		fprintf(f,"from %s@%s ***\r\n\n", shutter, hostname);
	else			/*regular shutdown*/
		fprintf(f,"(%s) ***\r\n\n", hostname);
	time(&sdt);
	sdt = sdt + deltim;
	ts = ctime(&sdt);	/*convert time to ascii string*/
	if (deltim > 10 MINUTES)		/*shutdown at xx:xx*/
		fprintf(f, "System going down at %5.5s\r\n", ts+11);
	else if (deltim > 95 SECONDS) {	/*shutdown in xx minutes*/
		fprintf(f, "System going down in %d minute%s\r\n",
		(deltim+30)/60, (deltim+30)/60 != 1 ? "s" : "");
	} 
	else if (deltim > 0) {		/*shutdown in xx seconds*/
		fprintf(f, "System going down in %d second%s\r\n",
		deltim, deltim != 1 ? "s" : "");
	} 
	else if (deltim == 0) 		/*shutdown NOW*/
		fprintf(f, "System going down IMMEDIATELY\r\n");
	else				/*shutdown Canceled*/
		fprintf(f, "System Shutdown CANCELED\007\007\r\n");
	fprintf(f, "\r\n");
	exit(0);

}

dismount(pnt)
{
	short dev, cnt;

	if(mdev[pnt].mnton == -1)
		return;
	dev = mdev[pnt].mntdev;
	for(cnt = 0; cnt < nmntdev; cnt++){
		if(cnt == pnt)
			continue;
		if(mdev[cnt].mnton == dev)
			dismount(cnt);
	}
	printf("Dismounting %s", mdev[pnt].mntnam);
	if(mdev[pnt].mntonnam[0])
		printf("\tfrom %s", mdev[pnt].mntonnam);
	printf("\n");
	if(umount(mdev[pnt].mntnam) != 0){
		printf("\07\07Dismount failed for %s\n", mdev[pnt].mntnam);
		return;
	}
	mdev[pnt].mnton = -1;
	return;
}

dodismount()
{
	int i;

	if ((df = fopen("/dev", "r")) == NULL) {
		fprintf(stderr, "Can't open /dev\n");
		exit(1);
	}
	lseek(mem, (long)nl[4].n_value, 0);
	read(mem, &rootdev, sizeof(rootdev));
	lseek(mem, (long)nl[5].n_value, 0);
	read(mem, &nmount, sizeof(nmount));
	lseek(mem, (long)nl[6].n_value, 0);
	read(mem, &sepid, sizeof(sepid));
	if((mf = open("/etc/mtab", 0)) > 0){
		read(mf, &mtab, sizeof mtab);
		close(mf);
	}
	else
		mf = 0;
	for(nmntdev = i = 0; i < nmount; i++){
		lseek(mem,(long)nl[7].n_value+(i*sizeof(struct mount)),0);
		read(mem, &mnt, sizeof(struct mount));
		if(mnt.m_bufp == 0 || mnt.m_dev == rootdev)
			continue;
		lseek(mem, (long)mnt.m_inodp, 0);
		read(mem, &ino, sizeof(struct ino));
		mdev[nmntdev].mntdev = mnt.m_dev;
		mdev[nmntdev].mnton  = ino.i_dev;
		getdevnam(mnt.m_dev, nmntdev);
		nmntdev++;
	}
	fclose(df);
	sync();
	for(i = 0; i < nmntdev; i++)
		dismount(i);
	close(creat("/etc/mtab", 644));
	sync();
}

earlyintr()
{
	printf("\n\nSHUTDOWN CANCELED\n");
	exit(1);
}

onintr()
{
	if(fwflag) {
		signal(SIGINT, onintr);
		return;
	} else {
		signal(SIGINT, SIG_IGN); /* ignore any subsequent ^C's */
		unlink(&lokfil);
		deltim = -100;
		sndall();
		exit(0);
	}
}
