
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#ifndef lint
static char *Sccsid = "@(#)ruptime.c	3.1	(ULTRIX-11)	4/6/87";
/*
 * Based on:
 *	@(#)ruptime.c	1.3	(ULTRIX-32)	4/24/85
 */
#endif lint

/*
 * ruptime.c
 *
 *	static char sccsid[] = "@(#)ruptime.c	4.14 (Berkeley) 83/07/01";
 *
 *	12-Jul-84	ma.  Correct error message and change *etc to *spdir
 *			for clarity.
 *
 *	23-Apr-85 -- jrs
 *			Add new switches to give subset listings.
 *
 */

#include <sys/param.h>
#include <ctype.h>
#include <stdio.h>
#ifndef	pdp11
#include <sys/dir.h>
#else	pdp11
#include <ndir.h>
#endif	pdp11
#include <protocols/rwhod.h>

DIR	*spdir;

#define	NHOSTS	2000
int	nhosts;
struct	hs {
	struct	whod *hs_wd;
	int	hs_nusers;
} hs[NHOSTS];
struct	whod awhod;
int	hscmp(), ucmp(), lcmp(), tcmp();

#define	WHDRSIZE	(sizeof (awhod) - sizeof (awhod.wd_we))
#define	RWHODIR		"/usr/spool/rwho"

char	*interval();
#ifndef	pdp11
int	now;
#else	pdp11
time_t	now;
#endif	pdp11
char	*malloc(), *sprintf();
int	aflg;

#define down(h)		(now - (h)->hs_wd->wd_recvtime > 5 * 60)

main(argc, argv)
	int argc;
	char **argv;
{
	struct direct *dp;
	int f, i, t;
	char buf[2*BUFSIZ]; int cc;
	register struct hs *hsp = hs;
	register struct whod *wd;
	register struct whoent *we;
#ifndef	pdp11
	int maxloadav = 0;
#else	pdp11
	long maxloadav = 0;
#endif	pdp11
	int downflag = 1;
	int upflag = 1;
	int minuse = -1;
	char *matchto = NULL;
	char *argp;
	char *myname;
	int (*cmp)() = hscmp;

	time(&t);
	myname = *argv;
	argc--, argv++;

	while (argc) {
		argp = *argv;
		if (*argp == '-') {
			argp++;
			while (*argp != '\0') {
				if (isdigit(*argp)) {
					minuse = atoi(argp);
					while (isdigit(*argp)) {
						argp++;
					}
				} else {
					switch (*argp) {

					case 'a':	/* all */
						aflg++;
						break;

					case 'd':	/* down */
						upflag = 0;
						break;

					case 'l':	/* load sort */
						cmp = lcmp;
						break;

					case 'r':	/* running */
						downflag = 0;
						break;

					case 't':	/* uptime sort */
						cmp = tcmp;
						break;

					case 'u':	/* user count sort */
						cmp = ucmp;
						break;

					default:
						fprintf(stderr, "%s: Invalid switch - %c\n",
							myname, *argp);
						exit(1);
					}
					argp++;
				}
			}
		} else {
			matchto = argp;
		}
		argc--;
		argv++;
	}
	if (chdir(RWHODIR) < 0) {
		perror(RWHODIR);
		exit(1);
	}
	spdir = opendir(".");
	if (spdir == NULL) {
		perror(RWHODIR);
		exit(1);
	}
	while (dp = readdir(spdir)) {
		if (dp->d_ino == 0)
			continue;
		if (strncmp(dp->d_name, "whod.", 5))
			continue;
		if (matchto != NULL && strcmp(matchto, &dp->d_name[5]) != 0) {
			continue;
		}
		if (nhosts == NHOSTS) {
			fprintf(stderr, "too many hosts\n");
			exit(1);
		}
		f = open(dp->d_name, 0);
		if (f > 0) {
			cc = read(f, buf, 2*BUFSIZ);
			if (cc >= WHDRSIZE) {
				hsp->hs_wd = (struct whod *)malloc(WHDRSIZE);
				wd = (struct whod *)buf;
				bcopy(buf, hsp->hs_wd, WHDRSIZE);
				hsp->hs_nusers = 0;
				for (i = 0; i < 2; i++)
					if (wd->wd_loadav[i] > maxloadav)
						maxloadav = wd->wd_loadav[i];
				we = (struct whoent *)(buf+cc);
				while (--we >= wd->wd_we)
					if (aflg || we->we_idle < 3600)
						hsp->hs_nusers++;
				nhosts++; hsp++;
			}
		}
		(void) close(f);
	}
	(void) time(&now);
	qsort((char *)hs, nhosts, sizeof (hs[0]), cmp);
	if (nhosts == 0) {
		if (matchto != NULL) {
			exit(0);
		} else {
			printf("no hosts!?!\n");
			exit(1);
		}
	}
	for (i = 0; i < nhosts; i++) {
		hsp = &hs[i];
		if (down(hsp)) {
			if (downflag != 0 && minuse < 0) {
				printf("%-12.12s%s\n", hsp->hs_wd->wd_hostname,
					interval(now - hsp->hs_wd->wd_recvtime,
					"down"));
			}
			continue;
		}
		if (upflag == 0 || (minuse >= 0 && hsp->hs_nusers < minuse)) {
			continue;
		}
		printf("%-12.12s%s,  %4d user%s  load %*.2f, %*.2f, %*.2f\n",
		    hsp->hs_wd->wd_hostname,
		    interval(hsp->hs_wd->wd_sendtime -
			hsp->hs_wd->wd_boottime, "  up"),
		    hsp->hs_nusers,
		    hsp->hs_nusers == 1 ? ", " : "s,",
		    maxloadav >= 1000 ? 5 : 4,
			hsp->hs_wd->wd_loadav[0] / 100.0,
		    maxloadav >= 1000 ? 5 : 4,
		        hsp->hs_wd->wd_loadav[1] / 100.0,
		    maxloadav >= 1000 ? 5 : 4,
		        hsp->hs_wd->wd_loadav[2] / 100.0);
		cfree(hsp->hs_wd);
	}
	exit(0);
}

char *
interval(time, updown)
#ifndef	pdp11
	int time;
#else	pdp11
	time_t time;
#endif	pdp11
	char *updown;
{
	static char resbuf[32];
#ifndef	pdp11
	int days, hours, minutes;
#else	pdp11
	time_t days, hours, minutes;
#endif	pdp11

#ifndef	pdp11
	if (time < 0 || time > 3*30*24*60*60) {
#else	pdp11
	if (time < 0 || time > 365L*24L*60L*60L) {
#endif	pdp11
		(void) sprintf(resbuf, "   %s ??:??", updown);
		return (resbuf);
	}
	minutes = (time + 59) / 60;		/* round to minutes */
	hours = minutes / 60; minutes %= 60;
	days = hours / 24; hours %= 24;
#ifndef	pdp11
	if (days)
		(void) sprintf(resbuf, "%s %2d+%02d:%02d",
		    updown, days, hours, minutes);
	else
		(void) sprintf(resbuf, "%s    %2d:%02d",
		    updown, hours, minutes);
#else	pdp11
	if (days)
		(void) sprintf(resbuf, "%s %2D+%02D:%02D",
		    updown, days, hours, minutes);
	else
		(void) sprintf(resbuf, "%s    %2D:%02D",
		    updown, hours, minutes);
#endif	pdp11
	return (resbuf);
}

hscmp(h1, h2)
	struct hs *h1, *h2;
{

	return (strcmp(h1->hs_wd->wd_hostname, h2->hs_wd->wd_hostname));
}

/*
 * Compare according to load average.
 */
lcmp(h1, h2)
	struct hs *h1, *h2;
{

	if (down(h1))
		if (down(h2))
			return (tcmp(h1, h2));
		else
			return (1);
	else if (down(h2))
		return (-1);
#ifndef	pdp11
	else
		return (h2->hs_wd->wd_loadav[0] - h1->hs_wd->wd_loadav[0]);
#else	pdp11
	else if (h2->hs_wd->wd_loadav[0] > h1->hs_wd->wd_loadav[0])	
		return(1);
	else if (h2->hs_wd->wd_loadav[0] == h1->hs_wd->wd_loadav[0])
		return(0);
	else
		return(-1);
#endif	pdp11
}

/*
 * Compare according to number of users.
 */
ucmp(h1, h2)
	struct hs *h1, *h2;
{

	if (down(h1))
		if (down(h2))
			return (tcmp(h1, h2));
		else
			return (1);
	else if (down(h2))
		return (-1);
	else
		return (h2->hs_nusers - h1->hs_nusers);
}

/*
 * Compare according to uptime.
 */
tcmp(h1, h2)
	struct hs *h1, *h2;
{
#ifndef	pdp11
	long t1, t2;

	return (
		(down(h2) ? h2->hs_wd->wd_recvtime - now
			  : h2->hs_wd->wd_sendtime - h2->hs_wd->wd_boottime)
		-
		(down(h1) ? h1->hs_wd->wd_recvtime - now
			  : h1->hs_wd->wd_sendtime - h1->hs_wd->wd_boottime)
	);
#else	pdp11
	long t;
	t =
		(down(h2) ? h2->hs_wd->wd_recvtime - now
			  : h2->hs_wd->wd_sendtime - h2->hs_wd->wd_boottime)
		-
		(down(h1) ? h1->hs_wd->wd_recvtime - now
			  : h1->hs_wd->wd_sendtime - h1->hs_wd->wd_boottime);
	return((t < 0L) ? -1 : ((t == 0L) ? 0 : 1));
#endif	pdp11
}
