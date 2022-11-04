
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)tar.c	3.0	4/22/86";
/*
 * ULTRIX-11 tape archiver (tar)
 *
 * Added support for fifo files(S_IFIFO).
 * George Mathew: 6/25/85
 *
 * Replaced -p magtape support with -p protection support.
 * Specifying -p now causes the file's original protection
 * mask (lowest 12 bits) to be restored as was backed up.
 * John Gemignani, Jr. 08/09/84
 * 
 * Added support for -d.
 * Option allows for the selection of RX50 diskette
 * as the archive medium.  If -b (blocking factor)
 * is not specified, -d will imply a -b 10.
 * John Gemignani, Jr. 08/08/84
 *
 * Added support for special files (S_IFCHR;S_IFBLK).
 * Special files require a new field to the preamble
 * ("rdev[]"), placed at the end, and accounted for
 * in the checksum.
 * John Gemignani, Jr. 08/08/84
 *
 * Added support for -o.
 * Directory file entries (without size) are written to
 * the tape by default; specification of -o indicates
 * tar is to be compatible with previous tars.
 * John Gemignani, Jr. 08/06/84
 *
 * Modified for new magtape device names and
 * to add tape density select option.
 * Fred Canter 5/21/83
 *
 * Modified for -b flag always specifies blocking factor
 * and fix blocking factor problems in general.
 * Fred Canter 5/4/83
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <signal.h>

char	*sprintf();
char	*strcat();
daddr_t	bsrch();
#define TBLOCK	512
#define NBLOCK	20
#define NAMSIZ	100
#define S_NO_MODE 0
union hblock {
	char dummy[TBLOCK];
	struct header {
		char name[NAMSIZ];
		char mode[8];
		char uid[8];
		char gid[8];
		char size[12];
		char mtime[12];
		char chksum[8];
		char linkflag;
		char linkname[NAMSIZ];
		char rdev[6];
	} dbuf;
} dblock, tbuf[NBLOCK];

struct linkbuf {
	ino_t	inum;
	dev_t	devnum;
	int	count;
	char	pathname[NAMSIZ];
	struct	linkbuf *nextp;
} *ihead;

struct stat stbuf;

int	rflag, xflag, vflag, tflag, mt, cflag, mflag, fflag, bflag;
int	term, chksum, wflag, recno, first, linkerrok;
int	dflag, oflag, pflag;
int	hflag;
int	freemem = 1;
int	nblock = 1;
int	pipein = 0;

daddr_t	low;
daddr_t	high;

FILE	*tfile;
char	tname[] = "/tmp/tarXXXXXX";


char	*usefile;
char	magtape[]	= "/dev/rht0";

char	*malloc();

main(argc, argv)
int	argc;
char	*argv[];
{
	char *cp;
	int onintr(), onquit(), onhup(), onterm();

	if (argc < 2)
		usage();

	tfile = NULL;
	usefile =  magtape;
	argv[argc] = 0;
	argv++;
	for (cp = *argv++; *cp; cp++) 
		switch(*cp) {
		case 'f':
			usefile = *argv++;
			fflag++;
			break;
		case 'c':
			cflag++;
			rflag++;
			break;
				/**********************************/
				/* -d: Select RX50 diskettes as   */
				/*     the backup medium.         */
				/**********************************/
		case 'd':
			dflag++;
			magtape[6] = 'r';
			magtape[7] = 'x';
			break;
				/**********************************/
				/* -o: Create an archive without  */
				/*     directory headers.         */
				/**********************************/
		case 'o':
			oflag++;
			break;
		case 'u':
			mktemp(tname);
			if ((tfile = fopen(tname, "w")) == NULL) {
				fprintf(stderr, "Tar: cannot create temporary file (%s)\n", tname);
				done(1);
			}
			fprintf(tfile, "!!!!!/!/!/!/!/!/!/! 000\n");
			/* FALL THROUGH */
		case 'r':
			rflag++;
			if (nblock != 1 && cflag == 0) {
noupdate:
				fprintf(stderr, "Tar: Blocked tapes cannot be updated (yet)\n");
				done(1);
			}
			break;
		case 'v':
			vflag++;
			break;
		case 'h':
			hflag++;
			break;
		case 'w':
			wflag++;
			break;
		case 'x':
			xflag++;
			break;
		case 't':
			tflag++;
			break;
		case 'm':
			mflag++;
			break;
		case '-':
			break;
		case 'n':
			magtape[6] = 'm';
			usefile = magtape;
			break;
		case 'p':
			if(getuid() == 0)
				pflag++;
			else
				fprintf(stderr,
				    "\n\7tar: -p ignored, must be superuser!\n");
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
			magtape[8] = *cp;
			usefile = magtape;
			break;
		case 'b':
			nblock = atoi(*argv++);
			if (nblock > NBLOCK || nblock <= 0) {
				fprintf(stderr, "Invalid blocksize. (Max %d)\n", NBLOCK);
				done(1);
			}
			if (rflag && !cflag)
				goto noupdate;
			bflag++;
			break;
		case 'l':
			linkerrok++;
			break;
		default:
			fprintf(stderr, "tar: %c: unknown option\n", *cp);
			usage();
		}

	/*****************************************************/
	/* -d option implies -b 10, if -b was not specified. */
	/*****************************************************/
	if (dflag)
		if (!bflag)
			nblock = 10;
	if (rflag) {
		if (cflag && tfile != NULL) {
			usage();
			done(1);
		}
		if (signal(SIGINT, SIG_IGN) != SIG_IGN)
			signal(SIGINT, onintr);
		if (signal(SIGHUP, SIG_IGN) != SIG_IGN)
			signal(SIGHUP, onhup);
		if (signal(SIGQUIT, SIG_IGN) != SIG_IGN)
			signal(SIGQUIT, onquit);
/*
		if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
			signal(SIGTERM, onterm);
*/
		if (strcmp(usefile, "-") == 0) {
			if (cflag == 0) {
				fprintf(stderr, "Can only create standard output archives\n");
				done(1);
			}
			setbuf(stdout, 0);	/* unbuffered output */
			mt = dup(1);
			nblock = 1;
		}
		else if ((mt = open(usefile, 2)) < 0) {
			if (cflag == 0 || (mt =  creat(usefile, 0666)) < 0) {
				fprintf(stderr, "tar: cannot open %s\n", usefile);
				done(1);
			}
		}
		if (cflag == 0 && nblock == 0)
			nblock = 1;
		dorep(argv);
	}
	else if (xflag)  {
		if (strcmp(usefile, "-") == 0) {
			pipein = 1;
			mt = dup(0);
			nblock = 1;
		}
		else if ((mt = open(usefile, 0)) < 0) {
			fprintf(stderr, "tar: cannot open %s\n", usefile);
			done(1);
		}
		done(doxtract(argv));
	}
	else if (tflag) {
		if (strcmp(usefile, "-") == 0) {
			mt = dup(0);
			nblock = 1;
		}
		else if ((mt = open(usefile, 0)) < 0) {
			fprintf(stderr, "tar: cannot open %s\n", usefile);
			done(1);
		}
		dotable();
	}
	else
		usage();
	done(0);
}

usage()
{
	fprintf(stderr, "tar: usage  tar -{txru}[cvfbdlmnop] [tapefile] [blocksize] file1 file2...\n");
	done(1);
}

dorep(argv)
char	*argv[];
{
	register char *cp, *cp2;
	char wdir[60], parent[60], tmpdir[NAMSIZ];
	int i, j;

	if (!cflag) {
		getdir();
		do {
			passtape();
			if (term)
				done(0);
			getdir();
		} while (!endtape());
		if (tfile != NULL) {
			char buf[200];

			strcat(buf, "sort +0 -1 +1nr ");
			strcat(buf, tname);
			strcat(buf, " -o ");
			strcat(buf, tname);
			sprintf(buf, "sort +0 -1 +1nr %s -o %s; awk '$1 != prev {print; prev=$1}' %s >%sX;mv %sX %s",
				tname, tname, tname, tname, tname, tname);
			fflush(tfile);
			system(buf);
			freopen(tname, "r", tfile);
			fstat(fileno(tfile), &stbuf);
			high = stbuf.st_size;
		}
	}

	getwdir(wdir);
	while (*argv && ! term) {
		strcpy(parent,wdir);
		strcpy(tmpdir,argv[0]);
		cp2 = *argv;
		for (cp = *argv; *cp; cp++) {
			if (*cp == '/') {
				cp2 = cp;
				break;
			}
		}
		if (cp2 != *argv) {
			*cp2 = '\0';
			if (*(cp2+1) == '\0')
				cp2 = *argv;
		}
		if(strcmp(argv[0],".") == 0)
			strcpy(tmpdir,wdir);
		else if(strcmp(argv[0],"..") == 0) {
			strcpy(tmpdir,wdir);
			j = strlen(tmpdir) - 1;
			for(i=j;i>=0;i--) {
				if (strcmp(tmpdir[i],'/') == 0) {
					tmpdir[i+1] = '\0';
					break;
				}
			}
		}
		if (cp2 != *argv) {
			*cp2 = '/';
			strcat(tmpdir,cp2);
		} else if (strcmp(argv[0],".") != 0 && strcmp(argv[0],"..") != 0)
			strcpy(tmpdir,argv[0]);
		cp2 = tmpdir;
		for (cp = tmpdir; *cp; cp++)
			if (*cp == '/')
				cp2 = cp;
		if (cp2 != tmpdir) {
			*cp2 = '\0';
			if (chdir(tmpdir) < 0) {
				perror(tmpdir);
				chdir(wdir);
				continue;
			}
			strcpy(parent,tmpdir);
			*cp2 = '/';
			cp2++;
		}
		putfile(*argv++, cp2, parent);
		chdir(wdir);
	}
	putempty();
	putempty();
	flushtape();
	if (linkerrok == 1)
		for (; ihead != NULL; ihead = ihead->nextp)
			if (ihead->count != 0)
				fprintf(stderr, "Missing links to %s\n", ihead->pathname);
}

endtape()
{
	if (dblock.dbuf.name[0] == '\0') {
		backtape();
		return(1);
	}
	else
		return(0);
}

getdir()
{
	register struct stat *sp;
	int i;

	readtape( (char *) &dblock);
	if (dblock.dbuf.name[0] == '\0')
		return;
	sp = &stbuf;
	sscanf(dblock.dbuf.mode, "%o", &i);
	sp->st_mode = i;
	sscanf(dblock.dbuf.uid, "%o", &i);
	sp->st_uid = i;
	sscanf(dblock.dbuf.gid, "%o", &i);
	sp->st_gid = i;
	sscanf(dblock.dbuf.rdev, "%o", &i);
	sp->st_rdev = i;
	sscanf(dblock.dbuf.size, "%lo", &sp->st_size);
	sscanf(dblock.dbuf.mtime, "%lo", &sp->st_mtime);
	sscanf(dblock.dbuf.chksum, "%o", &chksum);
	if (chksum != checksum()) {
		fprintf(stderr, "directory checksum error\n");
		done(2);
	}
	if (tfile != NULL)
		fprintf(tfile, "%s %s\n", dblock.dbuf.name, dblock.dbuf.mtime);
}

passtape()
{
	long blocks;
	char buf[TBLOCK];

	if (dblock.dbuf.linkflag == '1')
		return;
	blocks = stbuf.st_size;
	blocks += TBLOCK-1;
	blocks /= TBLOCK;

	while (blocks-- > 0)
		readtape(buf);
}

putfile(longname, shortname, parent)
char *longname;
char *shortname;
char parent[];
{
	int infile;
	long blocks;
	char buf[TBLOCK];
	register char *cp, *cp2;
	struct direct dbuf;
	char nparent[NAMSIZ];
	int i, j;

	/********************************************************/
	/* Assume that the file is a regular file, and allow the*/
	/* stat() to fail.  This will allow open() to proceed,  */
	/* and return the standard "cannot open" message.       */
	/********************************************************/
	stbuf.st_mode = S_IFREG;
	if (!hflag)	/* -h: follow symlinks */
		lstat(shortname, &stbuf);	/* Don't follow symlinks */
	else
		stat(shortname, &stbuf);	/* stat for actual file */
	if (((stbuf.st_mode & S_IFMT) == S_IFREG) ||
	    ((stbuf.st_mode & S_IFMT) == S_IFDIR)) {
		infile = open(shortname, 0);
		if (infile < 0) {
			fprintf(stderr, "tar: %s: cannot open file\n", longname);
			return;
		}
	}

	if (tfile != NULL && checkupdate(longname) == 0) {
		close(infile);
		return;
	}
	if (checkw('r', longname) == 0) {
		close(infile);
		return;
	}

	if ((stbuf.st_mode & S_IFMT) == S_IFDIR) {
		for (i = 0, cp = buf; *cp++ = longname[i++];);
		*--cp = '/';
		*++cp = 0;
		i = 0;
	/*********************************************************/
	/* -o indicates that directory headers should NOT be     */
	/* written, thereby making current operations backwards  */
	/* with previous releases of tar.  By default, tar will  */
	/* now write these headers.  They are used at extract    */
	/* time to validate the presence of directories which    */
	/* are necessary for the extraction to succeed.          */
	/*********************************************************/
		if (!oflag) {
		    if ((cp - buf) >= NAMSIZ) {
		      fprintf(stderr,"tar: %s: file name too long\n",longname);
		      close(infile);
		      return;
		    }
	            stbuf.st_size = 0;
		    stbuf.st_rdev = 0;
		    tomodes(&stbuf);
		    strcpy(dblock.dbuf.name,buf);
		    sprintf(dblock.dbuf.chksum, "%6o", checksum());
		    writetape((char *) &dblock);
		    if (vflag)
			fprintf(stderr, "a %s directory\n", longname);
		}
		if (chdir(shortname) < 0) {
			perror(shortname);
			close(infile);
			return;
		}
		if (strcmp(parent,shortname) == 0)
			strcpy(nparent,parent);
		else
			sprintf(nparent,"%s/%s", parent, shortname);
		while (read(infile, (char *)&dbuf, sizeof(dbuf)) > 0 && !term) {
			if (dbuf.d_ino == 0) {
				i++;
				continue;
			}
			if (strcmp(".", dbuf.d_name) == 0 || strcmp("..", dbuf.d_name) == 0) {
				i++;
				continue;
			}
			cp2 = cp;
			for (j=0; j < DIRSIZ; j++)
				*cp2++ = dbuf.d_name[j];
			*cp2 = '\0';
			close(infile);
			putfile(buf, cp, nparent);
			infile = open(".", 0);
			i++;
			lseek(infile, (long) (sizeof(dbuf) * i), 0);
		}
		close(infile);
		chdir(parent);
		return;
	}
	if ((stbuf.st_mode & S_IFMT) == S_IFLNK) {
		stbuf.st_size = 0;
		tomodes(&stbuf);
		if (strlen(longname) >= NAMSIZ) {
			fprintf(stderr, "tar: file name too long -> %s\n", longname);
			return;
		}
		strcpy(dblock.dbuf.name, longname);
		if (stbuf.st_size + 1 >= NAMSIZ) {
			fprintf(stderr, "tar: symbolic link too long -> %s\n", longname);
			return;
		}
		i = readlink(shortname, dblock.dbuf.linkname, NAMSIZ - 1);
		if (i < 0) {
			perror(longname);
			return;
		}
		dblock.dbuf.linkname[i] = '\0';
		dblock.dbuf.linkflag = '2';
		if (vflag) {
			fprintf(stderr, "a %s ", longname);
			fprintf(stderr, "symbolic link to %s\n",dblock.dbuf.linkname);
		}
		sprintf(dblock.dbuf.chksum, "%6o", checksum());
		writetape((char *)&dblock);
		return;
	}
	if (((stbuf.st_mode & S_IFMT) == S_IFCHR) ||
	    ((stbuf.st_mode & S_IFMT) == S_IFBLK) ||
	    ((stbuf.st_mode & S_IFMT) == S_IFIFO)) {
		stbuf.st_size = 0;
		tomodes(&stbuf);
		strcpy(dblock.dbuf.name,longname);
		sprintf(dblock.dbuf.chksum, "%6o", checksum());
		writetape((char *) &dblock);
		if (vflag) {
			if((stbuf.st_mode & S_IFMT) == S_IFIFO)
				fprintf(stderr,"a %s fifo file\n",longname);
			else
				fprintf(stderr,"a %s special file\n",longname);
		}
		return;
	}
	if ((stbuf.st_mode & S_IFMT) != S_IFREG) {
		fprintf(stderr, "tar: %s is not a file. Not dumped\n", longname);
		return;
	}

	tomodes(&stbuf);

	cp2 = longname;
	for (cp = dblock.dbuf.name, i=0; (*cp++ = *cp2++) && i < NAMSIZ; i++);
	if (i >= NAMSIZ) {
		fprintf(stderr, "%s: file name too long\n", longname);
		close(infile);
		return;
	}

	if (stbuf.st_nlink > 1) {
		struct linkbuf *lp;
		int found = 0;

		for (lp = ihead; lp != NULL; lp = lp->nextp) {
			if (lp->inum == stbuf.st_ino && lp->devnum == stbuf.st_dev) {
				found++;
				break;
			}
		}
		if (found) {
			strcpy(dblock.dbuf.linkname, lp->pathname);
			dblock.dbuf.linkflag = '1';
			sprintf(dblock.dbuf.chksum, "%6o", checksum());
			writetape( (char *) &dblock);
			if (vflag) {
				fprintf(stderr, "a %s ", longname);
				fprintf(stderr, "link to %s\n", lp->pathname);
			}
			lp->count--;
			close(infile);
			return;
		}
		else {
			lp = (struct linkbuf *) malloc(sizeof(*lp));
			if (lp == NULL) {
				if (freemem) {
					fprintf(stderr, "Out of memory. Link information lost\n");
					freemem = 0;
				}
			}
			else {
				lp->nextp = ihead;
				ihead = lp;
				lp->inum = stbuf.st_ino;
				lp->devnum = stbuf.st_dev;
				lp->count = stbuf.st_nlink - 1;
				strcpy(lp->pathname, longname);
			}
		}
	}

	blocks = (stbuf.st_size + (TBLOCK-1)) / TBLOCK;
	if (vflag) {
		fprintf(stderr, "a %s ", longname);
		fprintf(stderr, "%ld blocks\n", blocks);
	}
	sprintf(dblock.dbuf.chksum, "%6o", checksum());
	writetape( (char *) &dblock);

	while ((i = read(infile, buf, TBLOCK)) > 0 && blocks > 0) {
		writetape(buf);
		blocks--;
	}
	close(infile);
	if (blocks != 0 || i != 0)
		fprintf(stderr, "%s: file changed size\n", longname);
	while (blocks-- >  0)
		putempty();
}



doxtract(argv)
char	*argv[];
{
	long blocks, bytes;
	char buf[TBLOCK];
	char **cp;
	int ofile;
	int ldfile;	/* any file actually loaded */

	ldfile = 1;
	for (;;) {
		getdir();
		if (endtape())
			break;

		if (*argv == 0)
			goto gotit;

		for (cp = argv; *cp; cp++)
			if (prefix(*cp, dblock.dbuf.name))
				goto gotit;
		passtape();
		continue;

gotit:
		if (checkw('x', dblock.dbuf.name) == 0) {
			passtape();
			continue;
		}

		if (checkdir(dblock.dbuf.name))
			continue;

		if (dblock.dbuf.linkflag == '2') {
			unlink(dblock.dbuf.name);
			if (symlink(dblock.dbuf.linkname, dblock.dbuf.name)<0) {
				fprintf(stderr, "tar: symbolic link failed -> %s\n", dblock.dbuf.name);
				perror("tar");
				continue;
			}
			if (vflag)
				fprintf(stderr, "x %s symbolic link to %s\n",
					dblock.dbuf.name, dblock.dbuf.linkname);
			continue;
		}
		if (dblock.dbuf.linkflag == '1') {
			unlink(dblock.dbuf.name);
			if (link(dblock.dbuf.linkname, dblock.dbuf.name) < 0) {
				fprintf(stderr, "%s: cannot link\n", dblock.dbuf.name);
				continue;
			}
			if (vflag)
				fprintf(stderr, "%s linked to %s\n", dblock.dbuf.name, dblock.dbuf.linkname);
			continue;
		}
		switch(stbuf.st_mode & S_IFMT) {
		case S_IFCHR:
		case S_IFBLK:
			if ((ofile = mknod(dblock.dbuf.name,
				stbuf.st_mode,stbuf.st_rdev)) < 0) {
				fprintf(stderr,"tar: %s - cannot create special file\n", dblock.dbuf.name);
				passtape();
				continue;
			}
			if (vflag)
				fprintf(stderr,"x %s, special file\n",dblock.dbuf.name);
			continue;
		case S_IFIFO:
			if ((ofile = mknod(dblock.dbuf.name,
				stbuf.st_mode,0)) < 0) {
				fprintf(stderr,"tar: %s - cannot create fifo file\n",dblock.dbuf.name);
				passtape();
				continue;
			}
			if (vflag)
				fprintf(stderr,"x %s, fifo file\n",dblock.dbuf.name);
			continue;
		case S_IFREG:
		case S_NO_MODE:
			if ((ofile = creat(dblock.dbuf.name,
				stbuf.st_mode & 07777)) < 0) {
				fprintf(stderr,"tar: %s - cannot create\n", dblock.dbuf.name);
				passtape();
				continue;
			}
			blocks = (((bytes = stbuf.st_size) + TBLOCK-1)/TBLOCK);
			if (vflag)
				fprintf(stderr,"x %s, %ld bytes, %ld tape blocks\n",
					 dblock.dbuf.name,bytes,blocks);
			while (blocks-- > 0) {
				readtape(buf);
				if (bytes > TBLOCK) {
					if (write(ofile, buf, TBLOCK) < 0) {
						fprintf(stderr, "tar: %s: HELP - extract write error\n", dblock.dbuf.name);
						done(2);
					}
				} else
					if (write(ofile, buf, (int) bytes) < 0) {
						fprintf(stderr, "tar: %s: HELP - extract write error\n", dblock.dbuf.name);
						done(2);
					}
				bytes -= TBLOCK;
			}
			ldfile = 0;
			close(ofile);
			break;
		default:
			fprintf(stderr,"tar: %s: cannot create\n",dblock.dbuf.name);
			passtape();
		}
		if(pflag)
			chown(dblock.dbuf.name, stbuf.st_uid, stbuf.st_gid);
		if (mflag == 0) {
			time_t timep[2];

			timep[0] = time(NULL);
			timep[1] = stbuf.st_mtime;
			utime(dblock.dbuf.name, timep);
		}
		if (pflag)
			chmod(dblock.dbuf.name,(stbuf.st_mode & 07777));
	}
	return(ldfile);
}

dotable()
{
	for (;;) {
		getdir();
		if (endtape())
			break;
		if (vflag)
			longt(&stbuf);
		printf("%s", dblock.dbuf.name);
		if (dblock.dbuf.linkflag == '1')
			printf(" linked to %s", dblock.dbuf.linkname);
		printf("\n");
		passtape();
	}
}

putempty()
{
	char buf[TBLOCK];
	char *cp;

	for (cp = buf; cp < &buf[TBLOCK]; )
		*cp++ = '\0';
	writetape(buf);
}

longt(st)
register struct stat *st;
{
	register char *cp;
	char *ctime();
	int majordev;
	int minordev;

	pmode(st);
	if (((st->st_mode & S_IFMT) == S_IFCHR) ||
	    ((st->st_mode & S_IFMT) == S_IFBLK)) {
		majordev = st->st_rdev>>8;
		minordev = st->st_rdev&0377;
		printf("     ");
		printf("%3d,%3d",majordev,minordev);
	} else {
		printf("%3d/%1d", st->st_uid, st->st_gid);
		printf("%7D", st->st_size);
	}
	cp = ctime(&st->st_mtime);
	printf(" %-12.12s %-4.4s ", cp+4, cp+20);
}

#define	SUID	04000
#define	SGID	02000
#define	ROWN	0400
#define	WOWN	0200
#define	XOWN	0100
#define	RGRP	040
#define	WGRP	020
#define	XGRP	010
#define	ROTH	04
#define	WOTH	02
#define	XOTH	01
#define	STXT	01000
int	m1[] = { 1, ROWN, 'r', '-' };
int	m2[] = { 1, WOWN, 'w', '-' };
int	m3[] = { 2, SUID, 's', XOWN, 'x', '-' };
int	m4[] = { 1, RGRP, 'r', '-' };
int	m5[] = { 1, WGRP, 'w', '-' };
int	m6[] = { 2, SGID, 's', XGRP, 'x', '-' };
int	m7[] = { 1, ROTH, 'r', '-' };
int	m8[] = { 1, WOTH, 'w', '-' };
int	m9[] = { 2, STXT, 't', XOTH, 'x', '-' };

int	*m[] = { m1, m2, m3, m4, m5, m6, m7, m8, m9};

pmode(st)
register struct stat *st;
{
	register int **mp;

	switch(st->st_mode & S_IFMT) {
	case S_IFCHR:
			printf("c");
			break;
	case S_IFBLK:
			printf("b");
			break;
	case S_IFIFO:
			printf("p");
			break;
	case S_IFDIR:
			printf("d");
			break;
	default:
			printf("-");
	}
	for (mp = &m[0]; mp < &m[9];)
		select(*mp++, st);
}

select(pairp, st)
int *pairp;
struct stat *st;
{
	register int n, *ap;

	ap = pairp;
	n = *ap++;
	while (--n>=0 && (st->st_mode&*ap++)==0)
		ap++;
	printf("%c", *ap);
}

checkdir(name)
register char *name;
{
	register char *cp;
	int i;
	for (cp = name; *cp; cp++) {
		if (*cp == '/') {
			*cp = '\0';
			if (access(name, 01) < 0) {
				if (fork() == 0) {
					execl("/bin/mkdir", "mkdir", name, 0);
					execl("/usr/bin/mkdir", "mkdir", name, 0);
					fprintf(stderr, "tar: cannot find mkdir!\n");
					done(0);
				}
			/***************************************************/
			/* Check returned status: If status came back (the */
			/* byte of zero indicates normal termination) then */
			/* the high byte is the exit status. mkdir returns */
			/* a high byte of zero if successful, and non-zero */
			/* otherwise.  If zero returns, we notify the user */
			/* otherwise mkdir would have displayed an error.  */
			/***************************************************/
				while (wait(&i) > 0)
				    if ((i & 0377) == 0)
					if ((i & ~0377) == 0) {
					    if(vflag)
						fprintf(stderr,"x %s, directory\n",name);
					    if(pflag) {
						chown(name, stbuf.st_uid, stbuf.st_gid);
						chmod(name, stbuf.st_mode&07777);
					    }
					}
			}
			*cp = '/';
		}
	}
	return(cp[-1] == '/');
}

onintr()
{
	signal(SIGINT, SIG_IGN);
	term++;
}

onquit()
{
	signal(SIGQUIT, SIG_IGN);
	term++;
}

onhup()
{
	signal(SIGHUP, SIG_IGN);
	term++;
}

onterm()
{
	signal(SIGTERM, SIG_IGN);
	term++;
}

tomodes(sp)
register struct stat *sp;
{
	register char *cp;

	for (cp = dblock.dummy; cp < &dblock.dummy[TBLOCK]; cp++)
		*cp = '\0';
	sprintf(dblock.dbuf.mode, "%6o ", sp->st_mode);
	sprintf(dblock.dbuf.uid, "%6o ", sp->st_uid);
	sprintf(dblock.dbuf.gid, "%6o ", sp->st_gid);
	sprintf(dblock.dbuf.rdev, "%6o ", sp->st_rdev);
	sprintf(dblock.dbuf.size, "%11lo ", sp->st_size);
	sprintf(dblock.dbuf.mtime, "%11lo ", sp->st_mtime);
}

checksum()
{
	register i;
	register char *cp;

	for (cp = dblock.dbuf.chksum; cp < &dblock.dbuf.chksum[sizeof(dblock.dbuf.chksum)]; cp++)
		*cp = ' ';
	i = 0;
	for (cp = dblock.dummy; cp < &dblock.dummy[TBLOCK]; cp++)
		i += *cp;
	return(i);
}

checkw(c, name)
char *name;
{
	if (wflag) {
		printf("%c ", c);
		if (vflag)
			longt(&stbuf);
		printf("%s: ", name);
		if (response() == 'y'){
			return(1);
		}
		return(0);
	}
	return(1);
}

response()
{
	char c;

	c = getchar();
	if (c != '\n')
		while (getchar() != '\n');
	else c = 'n';
	return(c);
}

checkupdate(arg)
char	*arg;
{
	char name[100];
	long	mtime;
	daddr_t seekp;
	daddr_t	lookup();

	rewind(tfile);
	for (;;) {
		if ((seekp = lookup(arg)) < 0)
			return(1);
		fseek(tfile, seekp, 0);
		fscanf(tfile, "%s %lo", name, &mtime);
		if (stbuf.st_mtime > mtime)
			return(1);
		else
			return(0);
	}
}

done(n)
{
	unlink(tname);
	exit(n);
}

prefix(s1, s2)
register char *s1, *s2;
{
	while (*s1)
		if (*s1++ != *s2++)
			return(0);
	if (*s2)
		return(*s2 == '/');
	return(1);
}

getwdir(s)
char *s;
{
	int i;
	int	pipdes[2];

	pipe(pipdes);
	if ((i = fork()) == 0) {
		close(1);
		dup(pipdes[1]);
		execl("/bin/pwd", "pwd", 0);
		execl("/usr/bin/pwd", "pwd", 0);
		fprintf(stderr, "pwd failed!\n");
		printf("/\n");
		exit(1);
	}
	while (wait((int *)NULL) != -1)
			;
	read(pipdes[0], s, 50);
	while(*s != '\n')
		s++;
	*s = '\0';
	close(pipdes[0]);
	close(pipdes[1]);
}

#define	N	200
int	njab;
daddr_t
lookup(s)
char *s;
{
	register i;
	daddr_t a;

	for(i=0; s[i]; i++)
		if(s[i] == ' ')
			break;
	a = bsrch(s, i, low, high);
	return(a);
}

daddr_t
bsrch(s, n, l, h)
daddr_t l, h;
char *s;
{
	register i, j;
	char b[N];
	daddr_t m, m1;

	njab = 0;

loop:
	if(l >= h)
		return(-1L);
	m = l + (h-l)/2 - N/2;
	if(m < l)
		m = l;
	fseek(tfile, m, 0);
	fread(b, 1, N, tfile);
	njab++;
	for(i=0; i<N; i++) {
		if(b[i] == '\n')
			break;
		m++;
	}
	if(m >= h)
		return(-1L);
	m1 = m;
	j = i;
	for(i++; i<N; i++) {
		m1++;
		if(b[i] == '\n')
			break;
	}
	i = cmp(b+j, s, n);
	if(i < 0) {
		h = m;
		goto loop;
	}
	if(i > 0) {
		l = m1;
		goto loop;
	}
	return(m);
}

cmp(b, s, n)
char *b, *s;
{
	register i;

	if(b[0] != '\n')
		exit(2);
	for(i=0; i<n; i++) {
		if(b[i+1] > s[i])
			return(-1);
		if(b[i+1] < s[i])
			return(1);
	}
	return(b[i+1] == ' '? 0 : -1);
}

readtape(buffer)
char *buffer;
{
	int i, j;

	if (recno >= nblock || first == 0) {
		if((pipein==0) && (first==0) && ((fflag==0) || (bflag==0)))
			j = NBLOCK;
		else
			j = nblock;
		if ((i = read(mt, tbuf, TBLOCK*j)) < 0) {
			fprintf(stderr, "Tar: tape read error\n");
			done(3);
		}
		if (first == 0) {
			if ((i % TBLOCK) != 0) {
				fprintf(stderr, "Tar: tape blocksize error\n");
				done(3);
			}
			i /= TBLOCK;
			if (rflag && i != 1) {
				fprintf(stderr, "Tar: Cannot update blocked tapes (yet)\n");
				done(4);
			}
			if (i != nblock) {
				fprintf(stderr, "Tar: blocksize = %d\n", i);
				nblock = i;
			}
		}
		recno = 0;
	}
	first = 1;
	copy(buffer, &tbuf[recno++]);
	return(TBLOCK);
}

writetape(buffer)
char *buffer;
{
	first = 1;
	if (nblock == 0)
		nblock = 1;
	if (recno >= nblock) {
		if (write(mt, tbuf, TBLOCK*nblock) < 0) {
			fprintf(stderr, "Tar: tape write error\n");
			done(2);
		}
		recno = 0;
	}
	copy(&tbuf[recno++], buffer);
	if (recno >= nblock) {
		if (write(mt, tbuf, TBLOCK*nblock) < 0) {
			fprintf(stderr, "Tar: tape write error\n");
			done(2);
		}
		recno = 0;
	}
	return(TBLOCK);
}

backtape()
{
	lseek(mt, (long) -TBLOCK, 1);
	if (recno >= nblock) {
		recno = nblock - 1;
		if (read(mt, tbuf, TBLOCK*nblock) < 0) {
			fprintf(stderr, "Tar: tape read error after seek\n");
			done(4);
		}
		lseek(mt, (long) -TBLOCK, 1);
	}
}

flushtape()
{
	if(write(mt, tbuf, TBLOCK*nblock) < 0) {
		fprintf(stderr, "Tar: tape write error (last record)\n");
		done(2);
	}
}

copy(to, from)
register char *to, *from;
{
	register i;

	i = TBLOCK;
	do {
		*to++ = *from++;
	} while (--i);
}
