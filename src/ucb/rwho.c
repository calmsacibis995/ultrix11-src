
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#ifndef lint
static char sccsid[] = "@(#)rwho.c	3.1	8/18/87";
#endif
/*
 * Based on
 *	"@(#)rwho.c	4.7 (Berkeley) 83/07/01";
 */

#include <sys/param.h>
#include <stdio.h>
#ifndef	pdp11
#include <sys/dir.h>
#else	pdp11
#include <ndir.h>
#endif	pdp11
#include <protocols/rwhod.h>

DIR	*etc;

struct	whod wd;
int	utmpcmp();
#ifndef	pdp11
#define	NUSERS	1000
#else	pdp11
#define	NUSERS	700
#endif	pdp11
struct	myutmp {
	char	myhost[32];
	int	myidle;
#ifndef	pdp11
	struct	outmp myutmp;
#else	pdp11
	struct	outmp Myutmp;
#endif	pdp11
} myutmp[NUSERS];
int	nusers;

#define	WHDRSIZE	(sizeof (wd) - sizeof (wd.wd_we))
#define	RWHODIR		"/usr/spool/rwho"

char	*ctime(), *strcpy();
long	now;
int	aflg;

main(argc, argv)
	int argc;
	char **argv;
{
	struct direct *dp;
	int cc, width;
	register struct whod *w = &wd;
	register struct whoent *we;
	register struct myutmp *mp;
	int f, n, i;

	argc--, argv++;
again:
	if (argc > 0 && !strcmp(argv[0], "-a")) {
		argc--, argv++;
		aflg++;
		goto again;
	}
	(void) time(&now);
	if (chdir(RWHODIR) < 0) {
		perror(RWHODIR);
		exit(1);
	}
	etc = opendir(".");
	if (etc == NULL) {
		perror("/etc");
		exit(1);
	}
	mp = myutmp;
	while (dp = readdir(etc)) {
		if (dp->d_ino == 0)
			continue;
		if (strncmp(dp->d_name, "whod.", 5))
			continue;
		f = open(dp->d_name, 0);
		if (f < 0)
			continue;
		cc = read(f, (char *)&wd, sizeof (struct whod));
		if (cc < WHDRSIZE) {
			(void) close(f);
			continue;
		}
		if (now - w->wd_recvtime > 5 * 60) {
			(void) close(f);
			continue;
		}
		cc -= WHDRSIZE;
		we = w->wd_we;
		for (n = cc / sizeof (struct whoent); n > 0; n--) {
			if (aflg == 0 && we->we_idle >= 60*60) {
				we++;
				continue;
			}
			if (nusers >= NUSERS) {
				printf("too many users\n");
				exit(1);
			}
#ifndef	pdp11
			mp->myutmp = we->we_utmp; mp->myidle = we->we_idle;
#else	pdp11
			mp->Myutmp = we->we_utmp; mp->myidle = we->we_idle;
#endif	pdp11
			(void) strcpy(mp->myhost, w->wd_hostname);
			nusers++; we++; mp++;
		}
		(void) close(f);
	}
	qsort((char *)myutmp, nusers, sizeof (struct myutmp), utmpcmp);
	mp = myutmp;
	width = 0;
	for (i = 0; i < nusers; i++) {
#ifndef	pdp11
		int j = strlen(mp->myhost) + 1 + strlen(mp->myutmp.out_line);
#else	pdp11
		int j = strlen(mp->myhost) + 1 + strlen(mp->Myutmp.out_line);
#endif	pdp11
		if (j > width)
			width = j;
		mp++;
	}
	mp = myutmp;
	for (i = 0; i < nusers; i++) {
		char buf[128];
#ifndef	pdp11
		sprintf(buf, "%s:%s", mp->myhost, mp->myutmp.out_line);
		printf("%-8.8s %-*s %.12s",
		   mp->myutmp.out_name,
		   width,
		   buf,
		   ctime((time_t *)&mp->myutmp.out_time)+4);
#else	pdp11
		sprintf(buf, "%s:%s", mp->myhost, mp->Myutmp.out_line);
		printf("%-8.8s %-*s %.12s",
		   mp->Myutmp.out_name,
		   width,
		   buf,
		   ctime((time_t *)&mp->Myutmp.out_time)+4);
#endif	pdp11
		mp->myidle /= 60;
		if (mp->myidle) {
			if (aflg) {
				if (mp->myidle >= 100*60)
					mp->myidle = 100*60 - 1;
				if (mp->myidle >= 60)
					printf(" %2d", mp->myidle / 60);
				else
					printf("   ");
			} else
				printf(" ");
			printf(":%02d", mp->myidle % 60);
		}
		printf("\n");
		mp++;
	}
	exit(0);
}

utmpcmp(u1, u2)
	struct myutmp *u1, *u2;
{
	int rc;

#ifndef	pdp11
	rc = strncmp(u1->myutmp.out_name, u2->myutmp.out_name, 8);
#else	pdp11
	rc = strncmp(u1->Myutmp.out_name, u2->Myutmp.out_name, 8);
#endif	pdp11
	if (rc)
		return (rc);
	rc = strncmp(u1->myhost, u2->myhost, 8);
	if (rc)
		return (rc);
#ifndef	pdp11
	return (strncmp(u1->myutmp.out_line, u2->myutmp.out_line, 8));
#else	pdp11
	return (strncmp(u1->Myutmp.out_line, u2->Myutmp.out_line, 8));
#endif	pdp11
}
