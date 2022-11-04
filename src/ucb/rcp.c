
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#ifndef lint
static	char	*sccsid = "@(#)rcp.c	3.0	(ULTRIX-11)	4/22/86";
#endif lint
/*
 * rcp
 *
 * Based on "@(#)rcp.c	5.3 (Berkeley) 6/8/85";
 */

/*
 * rcp.c
 *
 *	21-Feb-86	Fred Canter and Marc Teitelbaum
 *			Fix rcp hang problem.  Rcp wasen't
 *			checking writes to remote, and if one
 *			failed because of a temporary lack of
 *			mbufs, the rcp protocol went out of sync.
 *			Now checks all writes over socket.
 *			
 *	31-DEC-85	jsd.  update with 4.3 BSD version (-p flag)
 *
 *	12-Apr-84	mah.  Fixed remote to local copy.
 *
 *	17-Apr-84	mah.  Fix remote to remote copies.
 *
 *	24-Aug-84	ma.  Correct to prevent copying into one's self.
 *
 */
#include <sys/param.h>
#include <sys/stat.h>

#ifndef pdp11
#include <sys/time.h>
#else
#include <sys/types.h>
#endif pdp11

#include <sys/ioctl.h>

#include <netinet/in.h>

#include <stdio.h>
#include <signal.h>
#include <pwd.h>
#include <ctype.h>
#include <netdb.h>
#include <errno.h>

int	rem;
char	*colon(), *index(), *rindex(), *malloc(), *strcpy(), *sprintf();
int	errs;
int	lostconn();
int	errno;
char	*sys_errlist[];
int	iamremote, targetshouldbedirectory;
int	iamrecursive;
int	pflag;
struct	passwd *pwd;
struct	passwd *getpwuid();
int	userid;
int	port;

struct buffer {
	int	cnt;
	char	*buf;
} *allocbuf();

/*VARARGS*/
int	error();

#define	ga()	 	(void) remwrite(rem, "", 1)
#define MAXHNAMLEN	255
#define MAXREMWRITES	20000  /* times to retry writes over socket */

/*
 * Define DEBUG to write error retry counts
 * to a file (/tmp/RCP.#, where #=pid).
 */
/*
#define DEBUG
*/

#ifdef DEBUG
int dfd;
#endif DEBUG

main(argc, argv)
	int argc;
	char **argv;
{
	char *targ, *host, *src;
	char *suser, *tuser;
	int i;
	char buf[BUFSIZ], cmd[16];
	char lhost[MAXHNAMLEN];
	struct hostent *fhost;
	struct servent *sp;

	sp = getservbyname("shell", "tcp");
	if (sp == NULL) {
		fprintf(stderr, "rcp: shell/tcp: unknown service\n");
		exit(1);
	}
	port = sp->s_port;
	pwd = getpwuid(userid = getuid());
	if (pwd == 0) {
		fprintf(stderr, "who are you?\n");
		exit(1);
	}

#ifdef DEBUG
	initdebug();
#endif DEBUG

	for (argc--, argv++; argc > 0 && **argv == '-'; argc--, argv++) {
		(*argv)++;
		while (**argv) switch (*(*argv)++) {

		    case 'r':
			iamrecursive++;
			break;

		    case 'p':		/* preserve mtimes and atimes */
			pflag++;
			break;

		    /* The rest of these are not for users. */
		    case 'd':
			targetshouldbedirectory = 1;
			break;

		    case 'f':		/* "from" */
			iamremote = 1;
			(void) response();
			(void) setuid(userid);
			source(--argc, ++argv);
			exit(errs);

		    case 't':		/* "to" */
			iamremote = 1;
			(void) setuid(userid);
			sink(--argc, ++argv);
			exit(errs);

		    default:
			fprintf(stderr,
	"Usage: rcp [-p] file1 file2\n       rcp [-rp] file... directory\n");
			exit(1);
		}
	}
	rem = -1;
	if (argc > 2)
		targetshouldbedirectory = 1;
	(void) sprintf(cmd, "rcp%s%s%s",
	    iamrecursive ? " -r" : "", pflag ? " -p" : "", 
	    targetshouldbedirectory ? " -d" : "");
	(void) signal(SIGPIPE, lostconn);
	targ = colon(argv[argc - 1]);
	if (targ) {				/* ... to remote */
		*targ++ = 0;
		if (*targ == 0)
			targ = ".";
		tuser = rindex(argv[argc - 1], '.');
		if (tuser) {
			*tuser++ = 0;
			if (!okname(tuser))
				exit(1);
		} else
			tuser = pwd->pw_name;

		for (i = 0; i < argc - 1; i++) {
			src = colon(argv[i]);
			if (src) {		/* remote to remote */
				*src++ = 0;
				if (*src == 0)
					src = ".";
				suser = rindex(argv[i], '.');
				if (suser) {
					*suser++ = 0;
					if (!okname(suser))
						continue;
		(void) sprintf(buf, "rsh %s -l %s -n %s %s '%s.%s:%s'",
					    argv[i], suser, cmd, src,
					    argv[argc - 1], tuser, targ);
				} else
		(void) sprintf(buf, "rsh %s -n %s %s '%s.%s:%s'",
					    argv[i], cmd, src,
					    argv[argc - 1], tuser, targ);
		/*full name of remote1*/
				fhost = gethostbyname(argv[i]);
				if (fhost==0) {
					printf("unknown host: %s\n", 
						argv[i]);
					exit(-1);
				}
				strcpy(lhost,fhost->h_name);
		/*full name of remote2*/
				fhost = gethostbyname(argv[argc - 1]);
				if (fhost==0) {
					printf("unknown host: %s\n", 
						argv[argc-1]);
					exit(-1);
				}
		/*check source user*/
				if(!suser)
					suser = pwd->pw_name;
		/*
		 *Comparison order is source to target.  Checking hosts, then
		 *file (either same file name or .), and last for user.
		 */
				if ( !(strcmp(lhost,fhost->h_name)) &&
			            (!(strcmp(src,targ)) ||
				     !(strcmp(targ,"."))) &&
			     	     !(strcmp(suser,tuser))) {
					printf("rcp: Cannot copy file to itself.\n");
					exit(-1);
				}
				(void) susystem(buf);
			} else {		/* local to remote */
				if (rem == -1) {
		/*get local host*/
					gethostname(lhost,MAXHNAMLEN);
		/*full name of remote*/
					fhost = gethostbyname(argv[argc - 1]);
					if (fhost==0) {
						printf("unknown host: %s\n", 
							argv[argc-1]);
						exit(-1);
					}
		/*
		 *Comparison order is source to target.  Checking hosts, then
		 *file (either same file name or .), and last for user.
		 */
					if ( !(strcmp(lhost,fhost->h_name)) &&
				            (!(strcmp(argv[i],targ)) ||
					     !(strcmp(targ,"."))) &&
				     	     !(strcmp(pwd->pw_name,tuser))) {
						printf("rcp: Cannot copy file to itself.\n");
						exit(-1);
					}
					(void) sprintf(buf, "%s -t %s",
					    cmd, targ);
					host = argv[argc - 1];
					rem = rcmd(&host, port, pwd->pw_name,
						tuser, buf, 0);
					if (rem < 0)
						exit(1);
					if (response() < 0)
						exit(1);
					(void) setuid(userid);
				}
				source(1, argv+i);
			}
		}
	} else {				/* ... to local */
		if (targetshouldbedirectory)
			verifydir(argv[argc - 1]);
		for (i = 0; i < argc - 1; i++) {
			src = colon(argv[i]);
			if (src == 0) {		/* local to local */
			/*
			 * 4.3BSD /bin/cp has -p and -r flags,
			 *
			 *	(void) sprintf(buf, "/bin/cp%s%s %s %s",
				 *
				 *  iamrecursive ? " -r" : "",
				 *  pflag ? " -p" : "",
				 */

				if (iamrecursive) {
				    error("rcp: -r flag not supported on local to local copy.\n");
				    exit(errs);
				}
				else if (pflag) {
				    error("rcp: -p flag not supported on local to local copy.\n");
				    exit(errs);
				}

				(void) sprintf(buf, "/bin/cp %s %s",
				    argv[i], argv[argc - 1]);
				(void) susystem(buf);
			} else {		/* remote to local */
				*src++ = 0;
				if (*src == 0)
					src = ".";
				suser = rindex(argv[i], '.');
				if (suser) {
					*suser++ = 0;
					if (!okname(suser))
						continue;
				} else
					host = argv[i];
					suser = pwd->pw_name;
						/*remote-to-local copy*/
		/*local host*/
				gethostname(lhost,MAXHNAMLEN);
		/*full name of remote*/
				fhost = gethostbyname(argv[i]);
				if (fhost==0) {
					printf("unknown host: %s\n", 
						argv[i]);
					exit(-1);
				}
		/*
		 *Comparison order is source to target.  Checking hosts, then
		 *file (either same file name or .), and last for user.
		 */
				if ( !(strcmp(fhost->h_name,lhost)) &&
				    (!(strcmp(src,argv[argc - 1])) ||
				     !(strcmp(argv[argc - 1],"."))) &&
				     !(strcmp(suser,pwd->pw_name))) {
					printf("rcp: Cannot copy file to itself.\n");
					exit(-1);
				}
				(void) sprintf(buf, "%s -f %s", cmd, src);
				host = argv[i];
				rem = rcmd(&host, port, pwd->pw_name,
					suser, buf, 0);
				if (rem < 0)
					continue;
				(void) setreuid(0, userid);
				sink(1, argv+argc-1);
				(void) setreuid(userid, 0);
				(void) close(rem);
				rem = -1;
			}
		}
	}
	exit(errs);
}

verifydir(cp)
	char *cp;
{
	struct stat stb;

	if (stat(cp, &stb) >= 0) {
		if ((stb.st_mode & S_IFMT) == S_IFDIR)
			return;
		errno = ENOTDIR;
	}
	error("rcp: %s: %s.\n", cp, sys_errlist[errno]);
	exit(1);
}

char *
colon(cp)
	char *cp;
{

	while (*cp) {
		if (*cp == ':')
			return (cp);
		if (*cp == '/')
			return (0);
		cp++;
	}
	return (0);
}

okname(cp0)
	char *cp0;
{
	register char *cp = cp0;
	register int c;

	do {
		c = *cp;
		if (c & 0200)
			goto bad1;
		if (!isalpha(c) && !isdigit(c) && c != '_' && c != '-')
			goto bad1;
		cp++;
	} while (*cp);
	return (1);
bad1:
	fprintf(stderr, "rcp: invalid user name %s\n", cp0);
	return (0);
}

susystem(s)
	char *s;
{
	int status, pid, w;
	register int (*istat)(), (*qstat)();

#ifndef pdp11
	if ((pid = vfork()) == 0) {
#else pdp11
	if ((pid = fork()) == 0) {
#endif pdp11
		(void) setuid(userid);
		execl("/bin/sh", "sh", "-c", s, (char *)0);
		_exit(127);
	}
	istat = signal(SIGINT, SIG_IGN);
	qstat = signal(SIGQUIT, SIG_IGN);
	while ((w = wait(&status)) != pid && w != -1)
		;
	if (w == -1)
		status = -1;
	(void) signal(SIGINT, istat);
	(void) signal(SIGQUIT, qstat);
	return (status);
}

source(argc, argv)
	int argc;
	char **argv;
{
	char *last, *name;
	struct stat stb;
	static struct buffer buffer;
	struct buffer *bp;
	int x, sizerr, f, amt;
	off_t i;
	char buf[BUFSIZ];

	for (x = 0; x < argc; x++) {
		name = argv[x];
		if ((f = open(name, 0)) < 0) {
			error("rcp: %s: %s\n", name, sys_errlist[errno]);
			continue;
		}
		if (fstat(f, &stb) < 0)
			goto notreg;
		switch (stb.st_mode&S_IFMT) {

		case S_IFREG:
			break;

		case S_IFDIR:
			if (iamrecursive) {
				(void) close(f);
				rsource(name, &stb);
				continue;
			}
			/* fall into ... */
		default:
notreg:
			(void) close(f);
			error("rcp: %s: not a plain file\n", name);
			continue;
		}
		last = rindex(name, '/');
		if (last == 0)
			last = name;
		else
			last++;
		if (pflag) {
			/*
			 * Make it compatible with possible future
			 * versions expecting microseconds.
			 */
			(void) sprintf(buf, "T%ld 0 %ld 0\n",
			    stb.st_mtime, stb.st_atime);
			(void) remwrite(rem, buf, strlen(buf));
			if (response() < 0) {
				(void) close(f);
				continue;
			}
		}
		(void) sprintf(buf, "C%04o %ld %s\n",
		    stb.st_mode&07777, stb.st_size, last);
		(void) remwrite(rem, buf, strlen(buf));
		if (response() < 0) {
			(void) close(f);
			continue;
		}
		if ((bp = allocbuf(&buffer, f, BUFSIZ)) < 0) {
			(void) close(f);
			continue;
		}
		sizerr = 0;
		for (i = 0; i < stb.st_size; i += bp->cnt) {
			amt = bp->cnt;
			if (i + amt > stb.st_size)
				amt = stb.st_size - i;
			if (sizerr == 0 && read(f, bp->buf, amt) != amt)
				sizerr = 1;
			(void) remwrite(rem, bp->buf, amt);
		}
		(void) close(f);
		if (sizerr == 0)
			ga();
		else
			error("rcp: %s: file changed size\n", name);
		(void) response();
	}
}

#ifndef	pdp11
#include <sys/dir.h>
#else	pdp11
#include <ndir.h>
#endif	pdp11

rsource(name, statp)
	char *name;
	struct stat *statp;
{
	DIR *d = opendir(name);
	char *last;
	struct direct *dp;
	char buf[BUFSIZ];
	char *bufv[1];

	if (d == 0) {
		error("rcp: %s: %s\n", name, sys_errlist[errno]);
		return;
	}
	last = rindex(name, '/');
	if (last == 0)
		last = name;
	else
		last++;
	if (pflag) {
		(void) sprintf(buf, "T%ld 0 %ld 0\n",
		    statp->st_mtime, statp->st_atime);
		(void) remwrite(rem, buf, strlen(buf));
		if (response() < 0) {
			closedir(d);
			return;
		}
	}
	(void) sprintf(buf, "D%04o %d %s\n", statp->st_mode&07777, 0, last);
	(void) remwrite(rem, buf, strlen(buf));
	if (response() < 0) {
		closedir(d);
		return;
	}
	while (dp = readdir(d)) {
		if (dp->d_ino == 0)
			continue;
		if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
			continue;
		if (strlen(name) + 1 + strlen(dp->d_name) >= BUFSIZ - 1) {
			error("%s/%s: Name too long.\n", name, dp->d_name);
			continue;
		}
		(void) sprintf(buf, "%s/%s", name, dp->d_name);
		bufv[0] = buf;
		source(1, bufv);
	}
	closedir(d);
	(void) remwrite(rem, "E\n", 2);
	(void) response();
}

response()
{
	char resp, c, rbuf[BUFSIZ], *cp = rbuf;

	if (read(rem, &resp, 1) != 1)
		lostconn();
	switch (resp) {

	case 0:				/* ok */
		return (0);

	default:
		*cp++ = resp;
		/* fall into... */
	case 1:				/* error, followed by err msg */
	case 2:				/* fatal error, "" */
		do {
			if (read(rem, &c, 1) != 1)
				lostconn();
			*cp++ = c;
		} while (cp < &rbuf[BUFSIZ] && c != '\n');
		if (iamremote == 0)
			(void) write(2, rbuf, cp - rbuf);
		errs++;
		if (resp == 1)
			return (-1);
		exit(1);
	}
	/*NOTREACHED*/
}

lostconn()
{

	if (iamremote == 0)
		fprintf(stderr, "rcp: lost connection\n");
	exit(1);
}

sink(argc, argv)
	int argc;
	char **argv;
{
	off_t i, j, size;
	char *targ, *whopp, *cp;
	int of, mode, wrerr, exists, first, count, amt;
	struct buffer *bp;
	static struct buffer buffer;
	struct stat stb;
	int targisdir = 0;
	int mask = umask(0);
	char *myargv[1];
	char cmdbuf[BUFSIZ], nambuf[BUFSIZ];
	int setimes = 0;

#ifndef pdp11
	struct timeval tv[2];
#define atime	tv[0]
#define mtime	tv[1]

#else pdp11
	struct utimbuf {
	    time_t atime;	/* accessed time */
	    time_t mtime;	/* modified time */
	} utimbuf;
	time_t dummy_micro;	/* dummy microseconds */
#endif pdp11

#define	SCREWUP(str)	{ whopp = str; goto screwup; }

	if (!pflag)
		(void) umask(mask);
	if (argc != 1) {
		error("rcp: ambiguous target\n");
		exit(1);
	}
	targ = *argv;
	if (targetshouldbedirectory)
		verifydir(targ);
	ga();
	if (stat(targ, &stb) == 0 && (stb.st_mode & S_IFMT) == S_IFDIR)
		targisdir = 1;
	for (first = 1; ; first = 0) {
		cp = cmdbuf;
		if (read(rem, cp, 1) <= 0)
			return;
		if (*cp++ == '\n')
			SCREWUP("unexpected '\\n'");
		do {
			if (read(rem, cp, 1) != 1)
				SCREWUP("lost connection");
		} while (*cp++ != '\n');
		*cp = 0;
		if (cmdbuf[0] == '\01' || cmdbuf[0] == '\02') {
			if (iamremote == 0)
				(void) write(2, cmdbuf+1, strlen(cmdbuf+1));
			if (cmdbuf[0] == '\02')
				exit(1);
			errs++;
			continue;
		}
		*--cp = 0;
		cp = cmdbuf;
		if (*cp == 'E') {
			ga();
			return;
		}

#define getnum(t) (t) = 0; while (isdigit(*cp)) (t) = (t) * 10 + (*cp++ - '0');
		if (*cp == 'T') {
			setimes++;
			cp++;
#ifndef pdp11
			getnum(mtime.tv_sec);
#else pdp11
			getnum(utimbuf.mtime);
#endif pdp11
			if (*cp++ != ' ')
				SCREWUP("mtime.sec not delimited");

#ifndef pdp11
			getnum(mtime.tv_usec);
#else pdp11
			getnum(dummy_micro);
#endif pdp11
			if (*cp++ != ' ')
				SCREWUP("mtime.usec not delimited");

#ifndef pdp11
			getnum(atime.tv_sec);
#else pdp11
			getnum(utimbuf.atime);
#endif pdp11
			if (*cp++ != ' ')
				SCREWUP("atime.sec not delimited");

#ifndef pdp11
			getnum(atime.tv_usec);
#else pdp11
			getnum(dummy_micro);
#endif pdp11
			if (*cp++ != '\0')
				SCREWUP("atime.usec not delimited");
			ga();
			continue;
		}
		if (*cp != 'C' && *cp != 'D') {
			/*
			 * Check for the case "rcp remote:foo\* local:bar".
			 * In this case, the line "No match." can be returned
			 * by the shell before the rcp command on the remote is
			 * executed so the ^Aerror_message convention isn't
			 * followed.
			 */
			if (first) {
				error("%s\n", cp);
				exit(1);
			}
			SCREWUP("expected control record");
		}
		cp++;
		mode = 0;
		for (; cp < cmdbuf+5; cp++) {
			if (*cp < '0' || *cp > '7')
				SCREWUP("bad mode");
			mode = (mode << 3) | (*cp - '0');
		}
		if (*cp++ != ' ')
			SCREWUP("mode not delimited");
		size = 0;
		while (isdigit(*cp))
			size = size * 10 + (*cp++ - '0');
		if (*cp++ != ' ')
			SCREWUP("size not delimited");
		if (targisdir)
			(void) sprintf(nambuf, "%s%s%s", targ,
			    *targ ? "/" : "", cp);
		else
			(void) strcpy(nambuf, targ);
		exists = stat(nambuf, &stb) == 0;
		if (cmdbuf[0] == 'D') {
			if (exists) {
				if ((stb.st_mode&S_IFMT) != S_IFDIR) {
					errno = ENOTDIR;
					goto bad2;
				}
				if (pflag)
					(void) chmod(nambuf, mode);
			} else if (mkdir(nambuf, mode) < 0)
				goto bad2;
			myargv[0] = nambuf;
			sink(1, myargv);
			if (setimes) {
				setimes = 0;
#ifndef pdp11
				if (utimes(nambuf, tv) < 0)
#else pdp11
				if (utime(nambuf, &utimbuf) < 0)
#endif pdp11
					error("rcp: can't set times on %s: %s\n",
					    nambuf, sys_errlist[errno]);
			}
			continue;
		}
		if ((of = creat(nambuf, mode)) < 0) {
	bad2:
			error("rcp: %s: %s\n", nambuf, sys_errlist[errno]);
			continue;
		}
		if (exists && pflag)
#ifndef	pdp11
			(void) fchmod(of, mode);
#else	pdp11
			(void) chmod(nambuf, mode);
#endif	pdp11
		ga();
		if ((bp = allocbuf(&buffer, of, BUFSIZ)) < 0) {
			(void) close(of);
			continue;
		}
		cp = bp->buf;
		count = 0;
		wrerr = 0;
		for (i = 0; i < size; i += BUFSIZ) {
			amt = BUFSIZ;
			if (i + amt > size)
				amt = size - i;
			count += amt;
			do {
				j = read(rem, cp, amt);
				if (j <= 0)
					exit(1);
				amt -= j;
				cp += j;
			} while (amt > 0);
			if (count == bp->cnt) {
				if (wrerr == 0 &&
				    write(of, bp->buf, count) != count)
					wrerr++;
				count = 0;
				cp = bp->buf;
			}
		}
		if (count != 0 && wrerr == 0 &&
		    write(of, bp->buf, count) != count)
			wrerr++;
		(void) close(of);
		(void) response();
		if (setimes) {
			setimes = 0;
#ifndef pdp11
			if (utimes(nambuf, tv) < 0)
#else pdp11
			if (utime(nambuf, &utimbuf) < 0)
#endif pdp11
				error("rcp: can't set times on %s: %s\n",
				    nambuf, sys_errlist[errno]);
		}				   
		if (wrerr)
			error("rcp: %s: %s\n", nambuf, sys_errlist[errno]);
		else
			ga();
	}
screwup:
	error("rcp: protocol screwup: %s\n", whopp);
	exit(1);
}

struct buffer *
allocbuf(bp, fd, blksize)
	struct buffer *bp;
	int fd, blksize;
{
	struct stat stb;
	int size=0;

	if (fstat(fd, &stb) < 0) {
		error("rcp: fstat: %s\n", sys_errlist[errno]);
		return ((struct buffer *)-1);
	}
#ifndef pdp11
	size = roundup(stb.st_blksize, blksize);
#endif pdp11
	if (size == 0)
		size = blksize;
	if (bp->cnt < size) {
		if (bp->buf != 0)
			free(bp->buf);
		bp->buf = (char *)malloc((unsigned) size);
		if (bp->buf == 0) {
			error("rcp: malloc: out of memory\n");
			return ((struct buffer *)-1);
		}
	}
	bp->cnt = size;
	return (bp);
}

/*VARARGS1*/
error(fmt, a1, a2, a3, a4, a5)
	char *fmt;
	int a1, a2, a3, a4, a5;
{
	char buf[BUFSIZ], *cp = buf;

	errs++;
	*cp++ = 1;
	(void) sprintf(cp, fmt, a1, a2, a3, a4, a5);
	(void) remwrite(rem, buf, strlen(buf));
	if (iamremote == 0)
		(void) write(2, buf+1, strlen(buf+1));
}

/* 
 * remwrite - error checking write, with retry
 *
 *	Used when writing over a socket where the write
 *	may fail do to the temporary condition of no
 *	mbufs.  After a large number of retrys give up.
 */
remwrite(fd, buff, many)
	int fd;
	char *buff; 
	int many;
{
	register int num;
	register unsigned cnt = 0;

	while ((num = write(fd, buff, many)) < 0
	      && errno == ENOBUFS
	      && cnt < MAXREMWRITES) {
		cnt++;
	}
#ifdef DEBUG
	if (cnt) {
		char message[128];
		sprintf(message,"rcp: remwrite: retried %d times.\n",
			cnt);
		write(dfd, message, strlen(message));
	}
#endif DEBUG

	if (num < 0)
		lostconn();	/* exit */

	return(num);
}

#ifdef DEBUG
initdebug() {
	int pid = getpid();
	char file1[128];

	sprintf(file1,"/tmp/RCP.%d",pid);
	close(creat(file1,0644));
	if ((dfd = open(file1,1)) < 0) {
		fprintf(stderr,"Cant open debug file %s.\n",file1);
		exit(255);
	}
}
#endif DEBUG
