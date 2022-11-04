
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char sccsid[] = "@(#)vipw.c	3.0	4/22/86";
/*
 * Changes for ULTRIX-11:
 *     restore original mode of passwd file before leaving.
 * John Dustin
 */
/* Based on: vipw.c  4.2  (Berkeley) 9/7/83 */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>

#include <stdio.h>
#include <errno.h>
#include <signal.h>

/*
 * Password file editor with locking.
 */
char	*temp = "/etc/ptmp";
char	*temp2 = "/etc/ptmp2";
char	*passwd = "/etc/passwd";
char	buf[BUFSIZ];
char	*getenv();
char	*index();
extern	int errno;

main(argc, argv)
	char *argv[];
{
	int fd;
	unsigned short origmode=0644;	/* mode of orig passwd file */
	FILE *ft, *fp;
	char *editor;
	struct stat statbuf;

	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
	setbuf(stderr, NULL);
	umask(0);
	/* 
	 * save old passwd file mode so can restore before leaving
	 */
	if (stat(passwd, &statbuf) == 0)
		origmode = statbuf.st_mode;

	if (stat(temp, &statbuf) < 0) {
		if (creat(temp, 0644) < 0) {
			fprintf(stderr, "vipw: "); perror(temp);
			exit(1);
		}
	}
	else {	/* stat suceeded... passwd file busy */
		fprintf(stderr, "vipw: password file busy, try again later.\n");
		exit(1);
	}
	fd = open(temp, 2);
	if (fd < 0) {
		fprintf(stderr, "vipw: "); perror(temp);
		exit(1);
	}
		
	ft = fdopen(fd, "w");
	if (ft == NULL) {
		fprintf(stderr, "vipw: "); perror(temp);
		goto bad;
	}
	fp = fopen(passwd, "r");
	if (fp == NULL) {
		fprintf(stderr, "vipw: "); perror(passwd);
		goto bad;
	}
	while (fgets(buf, sizeof (buf) - 1, fp) != NULL)
		fputs(buf, ft);
	fclose(ft); fclose(fp);
	editor = getenv("EDITOR");
	if (editor == 0)
		editor = "vi";
	sprintf(buf, "%s %s", editor, temp);
	if (system(buf) == 0) {
		struct stat sbuf;
		int ok;

		/* sanity checks */
		if (stat(temp, &sbuf) < 0) {
			fprintf(stderr,
			    "vipw: can't stat temp file, %s unchanged\n",
			    passwd);
			goto bad;
		}
		if (sbuf.st_size == 0) {
			fprintf(stderr, "vipw: bad temp file, %s unchanged\n",
			    passwd);
			goto bad;
		}
		ft = fopen(temp, "r");
		if (ft == NULL) {
			fprintf(stderr,
			    "vipw: can't reopen temp file, %s unchanged\n",
			    passwd);
			goto bad;
		}
		ok = 0;
		while (fgets(buf, sizeof (buf) - 1, ft) != NULL) {
			register char *cp;

			cp = index(buf, '\n');
			if (cp == 0)
				continue;
			*cp = '\0';
			cp = index(buf, ':');
			if (cp == 0)
				continue;
			*cp = '\0';
			if (strcmp(buf, "root"))
				continue;
			/* password */
			cp = index(cp + 1, ':');
			if (cp == 0)
				break;
			/* uid */
			if (atoi(cp + 1) != 0)
				break;
			cp = index(cp + 1, ':');
			if (cp == 0)
				break;
			/* gid */
			cp = index(cp + 1, ':');
			if (cp == 0)
				break;
			/* gecos */
			cp = index(cp + 1, ':');
			if (cp == 0)
				break;
			/* login directory */
			if (strncmp(++cp, "/:"))
				break;
			cp += 2;
			if (*cp && strcmp(cp, "/bin/sh") &&
			    strcmp(cp, "/bin/csh"))
				break;
			ok++;
		}
		fclose(ft);
		if (ok) {
			/* temp2 holds the orig passwd in case of trouble */
			if (link(passwd, temp2) < 0) {
				fprintf(stderr, "vipw: ");
				perror(temp2);
			} else {
				if (unlink(passwd) < 0) {
					fprintf(stderr, "vipw: ");
					perror(passwd);
				} else {
					if (link(temp, passwd) < 0) {
						fprintf(stderr, "vipw: ");
						perror(passwd);
						if (link(temp2, passwd) < 0) {
							fprintf(stderr, "vipw: ");
							perror(passwd);
						}
					}
				}
			/* restore original passwd file mode */
			chmod(passwd, origmode);
			}
		} else {
		/*	fprintf(stderr,
			    "vipw: you mangled the temp file, %s unchanged\n",
			    passwd);
		 */
			fprintf(stderr,"vipw: you mangled the root entry, %s unchanged\n",passwd);
		}
	}
bad:
	unlink(temp);
	unlink(temp2);
}
