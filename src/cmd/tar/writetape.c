
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/


#ifndef lint
static	char	*sccsid = "@(#)writetape.c	3.0	(ULTRIX)	4/22/86";
#endif	lint

/**/
/*
 *
 *	File name:
 *
 *		writetape.c
 *
 *	Source file description:
 *
 *		Tar logic for output to tape functions.
 *
 *	Functions:
 *
 *	Usage:
 *
 *	Compile:
 *
 *
 *	Modification history:
 *	~~~~~~~~~~~~~~~~~~~~
 *
 *	revision			comments
 *	--------	-----------------------------------------------
 *	I		19-Dec-85 rjg/
 *			Create original version.
 *	
 */
#include "tar.h"

/*.sbttl checksum() */

/* Function:
 *
 *	checksum
 *
 * Function Description:
 *
 *	Perform first-level checksum for file header block.
 *
 * Arguments:
 *
 *	none
 *
 * Return values:
 *
 *	The checksum value is returned.
 *
 * Side Effects:
 *
 *	
 */
checksum()
{
/*------*\
  Locals
\*------*/

	STRING_POINTER	cp;
	int	i = 0;

/*------*\
   Code
\*------*/

for (cp = dblock.dbuf.chksum; cp < &dblock.dbuf.chksum[sizeof(dblock.dbuf.chksum)]; cp++)
	*cp = ' ';

for (cp = dblock.dummy; cp < &dblock.dummy[TBLOCK]; cp++)
	i += *cp;

return (i);

}/*E checksum() */

/*.sbttl dorep()  replete command line of args */

/* Function:
 *
 *	dorep
 *
 * Function Description:
 *
 *	Repletes the command line of arguments and places
 *	the files on the output archive.
 *
 * Arguments:
 *
 *	char	*argv	Pointer to list of path/file name arguments
 *
 * Return values:
 *
 *
 * Side Effects:
 *
 *	n/a	
 */
dorep(argv)
	char *argv[];
{
/*------*\
  Locals
\*------*/

	STRING_POINTER	chp;
	STRING_POINTER	chp2;
	COUNTER	i;

/*------*\
   Code
\*------*/

if (CARCH != start_archive && AFLAG)
	fprintf(stderr,"\n%s: Skipping to %s:  %d\n", progname, Archive, start_archive);

if (!cflag) {

	/* Here if doing an add to end or update
	 */
NEXTAU:
/**/
	/* getdir() will fill in the  stats buffer from the file
	 * header block on the input archive.
	 */
	getdir();
	/*
	 * Save last file name  &  mod time for possible
	 * archive switch compare.
	 */
	if (dblock.dbuf.name[0]) {
		strcpy(file_name,dblock.dbuf.name);
		modify_time = stbuf.st_mtime;
	}
	do {
		passtape();

		if (term)
			done(FAIL);

		/* getdir() fills in the  stats buffer from the file
	 	 * header block on the input archive.
	 	 */
		getdir();
		/*
	 	 * Save last file name  &  mod time for possible
	 	 * archive switch compare.
	 	 */
		if (dblock.dbuf.name[0]) {
			strcpy(file_name,dblock.dbuf.name);
			modify_time = stbuf.st_mtime;
		}

	} while (!endtape());

		backtape();

		if (EODFLAG && FILE_CONTINUES)
			goto NEXTAU;

		OARCH = 0;
		
		if (tfile) {

			sprintf(iobuf,
"sort +0 -1 +1nr %s -o %s; awk '$1 != prev {print; prev=$1}' %s >%sX; mv %sX %s",
				tname, tname, tname, tname, tname, tname);
			fflush(tfile);
			system(iobuf);
			freopen(tname, "r", tfile);
			fstat(fileno(tfile), &stbuf);
			high = stbuf.st_size;
		}
}/*E if !cflag */

getcwd(hdir);
strcpy(wdir,hdir);

while (*argv && !term) {
	chp2 = *argv;

	if (!strcmp(chp2,"-C") && argv[1]) {
		argv++;

		if ((chdir(*argv)) < 0) 
			perror(*argv);
		else  {
			getcwd(wdir);
			strcpy(hdir,wdir);
		}
		argv++;
		continue;

	}/*E if !strncmp */

	for (chp = *argv; *chp; chp++) 
		if (*chp == '/')
			chp2 = chp;

	if (chp2 != *argv) {
		*chp2 = '\0';

		if (chdir(*argv) < 0) {
			perror(*argv);
			continue;
		}
		getcwd(wdir);
		*chp2 = '/';
		chp2++;

	}/*E if chp2 != *argv */

	/* Go put files on the output archive.
	 */
	if (putfile(*argv++, chp2, wdir) == A_WRITE_ERR)
		return(A_WRITE_ERR);

	if (chdir(hdir) < 0) {
		fprintf(stderr,"%s: Can't change directory back ?", progname);
		perror(hdir);
		done(FAIL);
	}
}/*E while (*argv && ! term) */

PUTE++;
for (i=3; i; i--) {
	if (putempty() == A_WRITE_ERR)
		return(A_WRITE_ERR);
}
if (flushtape() == A_WRITE_ERR)
	return(A_WRITE_ERR);


if (VFLAG) {
	fprintf(stderr,"%s: links = %d/%d   directorys = %d/%d/%d\n\n",
		progname,lcount1,lcount2,dcount1,dcount2,dcount3);
}
if (!lflag)	/* -l: print link resolution errors */
	return(SUCCEED);

for (; ihead; ihead = ihead->nextp) {
	if (!ihead->count)
		continue;

	fprintf(stderr, "%s: Missing links to:  %s\n", progname, ihead->pathname);
}
return(SUCCEED);

}/*E dorep()*/

/*.sbttl getcwd() */

/* Function:
 *
 *	getcwd
 *
 * Function Description:
 *
 *	Get current working directory character string
 *
 * Arguments:
 *
 *
 * Return values:
 *
 *
 * Side Effects:
 *
 *	
 */
char *getcwd(buf)
	char	*buf;
{
int i;

/*------*\
   Code
\*------*/

#ifndef PRO

if (!getwd(iobuf)) {
	fprintf(stderr, "%s: %s\n", progname, iobuf);
	done(FAIL);
}
if ((i=strlen(iobuf)) >= NAMSIZ) {
	fprintf(stderr,"\n%s: File name too long: %s\n",progname,iobuf);
	done(FAIL);
}
strcpy(buf,iobuf);

#else

int	pipdes[2];

pipe(pipdes);

if (!(i=fork())) {
	close(1);
	dup(pipdes[1]);
	execl("/bin/pwd","pwd",0);
	perror(progname);
	printf("\n");
	done(FAIL);
}
while (wait((int *)NULL) != -1)
	;/*_NOP_*/

if ((read(pipdes[0],buf,MAXPATHLEN)) < 0) {
	fprintf(stderr,"%s: getcwd() can't get working directory\n",progname);
	perror(progname);
	done(FAIL);
}

while(*buf != '\n') {
	buf++;
}
*buf = '\0';
close(pipdes[0]);
close(pipdes[1]);

#endif

return (buf);

}/*E getcwd() */

/*.sbttl putdir()  Put directory files on archive */

/* Function:
 *
 *	putdir
 *
 * Function Description:
 *
 *	Writes directory file entries to the output archive.
 *
 * Arguments:
 *
 *	char	*longname	Complete pathname+filename
 *	char	*shortname	File name only
 *
 * Return values:
 *
 *	-1	if error occurs
 *	+n	if ok
 *
 * Side Effects:
 *
 */
putdir(longname,shortname)

	STRING_POINTER	longname;
	STRING_POINTER	shortname;
{
/*------*\
  Locals
\*------*/

	struct stat	Dnode;
	struct direct	dirbuf;
	SIZE_I	i;
	INDEX	j;
	FLAG	dfound = 0;
	STRING_POINTER	dpath;
	char	path[NAMSIZ];
	char	pathname[NAMSIZ];
	INDEX	sp = 0;
	int	statval;

/*------*\
   Code
\*------*/

if (!strcmp(longname,".") || !strcmp(longname,"..") ||
    !strcmp(shortname,"/"))
	return(SUCCEED);

new_file = TRUE;

if (longname[0] != '/') {

	/* For relative paths, form an absolute path name
	 * to stat.
	 */
	strcpy(pathname,hdir);
	strcat(pathname,"/");

	/* Set the Start Processing flag to skip over leading
	 * absolute path name components.
	 */
	sp = strlen(pathname);
	strcat(pathname,longname);
}
else
	strcpy(pathname,longname);

i = strlen(pathname);

for (j=0; j < i; ) {

	path[j] = pathname[j++];

	if (pathname[j] == '/' || pathname[j] == '\n' || !pathname[j]) {

	    if ( (j >= sp) && (j < NAMSIZ)) {

		path[j] = 0;

#ifndef PRO
		if (!hflag)
			statval = lstat(path, &Dnode);
		else
#endif
			statval = stat(path, &Dnode);

		if (statval < 0) {
			fprintf(stderr,"\n%s: putdir() can't stat:  %s\n",
				progname, path);

			perror(path);
			return(FAIL);
		}

		if ((Dnode.st_mode & S_IFMT) == S_IFDIR) {

			struct DIRE *direp;

			for (dfound=0,direp=Dhead; direp;
				direp = direp->dir_next) {

				if ((direp->rdev == Dnode.st_rdev) &&
			    	    (direp->inode == Dnode.st_ino)) {
					dfound++;
					break;
				}
			}
			if (!dfound) {
				/* Get some memory for our next
			 	 * directory list entry.
			 	 */
				direp = (struct DIRE *) malloc(sizeof (*direp));
				if (!direp) {
				
					/* If no mem, return what we
				 	 * have & try again ?
				 	 */
					fdlist();
					direp = (struct DIRE *) malloc(sizeof (*direp));
					if (!direp) {
						fprintf(stderr,"\n\007%s: putdir() can't get memory for directory list\n",
						progname);

						NMEM4D++;
						return(FAIL);
					}
				}/*E if !direp */

				/* Save new directory entry
			 	 */
				dcount1++;
				direp->rdev = Dnode.st_rdev;
				direp->inode = Dnode.st_ino;
				direp->dir_next = Dhead;
				Dhead = direp; /* Lists run backwards */
				Dnode.st_size = 0L;
				Dnode.st_rdev = 0;
				remaining_chctrs = written = 0L;

				if (size_of_media[CARCH]) {
	    				if ((blocks_used) >
				    	(size_of_media[CARCH] - 3L))

						OARCH = CARCH;
	    				else
						OARCH = 0;
				}
				/* Set the directory path pointer to
				 * start of absolute path name, or to
				 * the start of the relative name
				 * at the end of the absolute path.
				 */
				dpath = path + sp;
				strcat(dpath,"/");
				tomodes(&Dnode,1,dpath);
				sprintf(dblock.dbuf.chksum, "%6o", checksum());

				/* Go put directory file on the archive.
			 	 */
				if (writetape((char *)&dblock,1,1,dpath,dpath) == A_WRITE_ERR)
					return(A_WRITE_ERR);

				if (vflag && (CARCH >= start_archive))
					fprintf(stderr,"a%s %s %s\n", MFLAG ? CARCHS : NULS, dpath, VFLAG ? DIRECT : NULS);

			}/*E if !dfound*/
		}/*T if Dnode.st_mode ..*/
		else
			return(SUCCEED);

	     }/*E if j >= sp .. */
	}/*E if pathname[j] ..*/
}/*E for (j=0, d=0; j<i; ) */

return(SUCCEED);

}/*E putdir() */

/*.sbttl putfile()  Put file(s) on archive recursively */

/* Function:
 *
 *	putfile
 *
 * Function Description:
 *
 *	This function  RECURSIVELY  puts files on the output archive.
 *
 * Arguments:
 *
 *
 * Return values:
 *
 *
 * Side Effects:
 *
 *	
 */

putfile(longname, shortname, parent)
	STRING_POINTER longname;
	STRING_POINTER shortname;
	STRING_POINTER parent;
{
/*------*\
  Locals
\*------*/

	STRING_POINTER	cp;
#ifdef U11
	STRING_POINTER	cp2;
#endif
	struct	direct	*dp;
	struct	direct	dirbuf;

#ifndef U11
	DIR	*dirp;
#endif
	SIZE_I	i;
	FILE_D	infile = 0;
	SIZE_I	j;

/*------*\
   Code
\*------*/

new_file = TRUE;
 
if (strlen(longname) >= NAMSIZ) {

NAMTL:
/**/
	fprintf(stderr,"\n%s: Path name too long: %s\n",progname,longname);
	return(FAIL);
}

/* -h: Follow symbolic links. Test of user flag.
 */
#ifndef PRO
if (!hflag)
	i = lstat(shortname, &stbuf);	/* Don't follow symlinks  -
					 * stat link itself. */
else
#endif
	i = stat(shortname, &stbuf);	/* Follow - stat actual file */

if (i < 0) {
	fprintf(stderr,"\n%s: putfile() can't stat: %s\n",
		progname, shortname);

	perror(shortname);
	return(FAIL);

}/*E if i < 0 */

/* (-u) Latest version already in archive? Yes: Skip.
 */
if (tfile) {
	if (!update(longname, stbuf.st_mtime))
		return(SUCCEED);
}
/* (-w) Passed user confirmation? No: skip.
 */
if (wflag) {
	if (!checkw('r', longname))
		return(SUCCEED);
}
/* (-F[FF]) Recreatable file? Yes: skip for speed.
 */
if (Fflag) {
	if (checkf(shortname, stbuf.st_mode, Fflag) == 0)
		return(SUCCEED);
}
/* Process directories in path..
 */
if (!oflag && !NMEM4D && !DFLAG && !NFLAG)
	if (putdir(longname,shortname) == A_WRITE_ERR)
		return(A_WRITE_ERR);
		
switch (stbuf.st_mode & S_IFMT) {

	case S_IFDIR: {
	
			char	curd[NAMSIZ];
			char	newparent[NAMSIZ];
			char	pfnbuf[NAMSIZ];

		getcwd(curd);
		/*
 		 * Copy the long file name to a work buffer and
 		 * append a "slash null".
 		 * ie.. ->  test/0  ...
 		 * Set up in order to tack on a file name to a
 		 * potential directory name.
 		 */
		for (i = 0, cp = pfnbuf; *cp++ = longname[i++];)
			; /*-NOP-*/

		*--cp = '/';
		*++cp = 0;


		/* If no memory for directory lists, or user
		 * doesn't want us to keep them, put out the
		 * directory as in the past.
		 */

		if ((NFLAG || DFLAG || NMEM4D) && !oflag) {

			if (size_of_media[CARCH]) {
    				if ((blocks_used) >
					(size_of_media[CARCH] - 3L))

					OARCH = CARCH;
    				else
					OARCH = 0;
			}
			dcount2++;
			stbuf.st_size = 0L;
			stbuf.st_rdev = 0;
			remaining_chctrs = written = 0L;
			tomodes(&stbuf,1,pfnbuf);
			sprintf(dblock.dbuf.chksum, "%6o", checksum());

			/* Go put directory file on the archive.
		 	*/
			if (writetape((char *)&dblock,1,1,pfnbuf,pfnbuf) == A_WRITE_ERR)
				return(A_WRITE_ERR);

			if (vflag && (CARCH >= start_archive))
				fprintf(stderr,"a%s %s %s\n", MFLAG ? CARCHS : NULS, pfnbuf, VFLAG ? DIRECT : NULS);
		}/*E if NFLAG .. */ 

		i = 0;
#ifndef U11

		if (chdir(shortname) < 0) {
			fprintf(stderr, "%s: Can't change directory to:  %s\n", progname, shortname);
 			perror(shortname);
			return(FAIL);
 		}
 		if ((dirp = opendir(".")) == NULL) {
 			fprintf(stderr,"%s: Directory read error:  %s\n", progname, longname);
			perror(longname);

			if (chdir(curd) < 0) {
				fprintf(stderr, "%s: Can't change directory back to %s ?\n", progname, curd);
				perror(curd);
			}
			return(FAIL);
		}
		getcwd(newparent);

		while ((dp = readdir(dirp)) && !term) {
			if (!dp->d_ino)
				continue;

			if (!strcmp(".", dp->d_name) ||
				!strcmp("..", dp->d_name))
				continue;

			strcpy(cp, dp->d_name);
			i = telldir(dirp);
			closedir(dirp);

			if (putfile(pfnbuf, cp, newparent) == A_WRITE_ERR)
				return(A_WRITE_ERR);

			dirp = opendir(".");
			seekdir(dirp, i);

		}/*E while dp = readdir ..*/

		closedir(dirp);

#endif

#ifdef U11
		/* Read the directory content.
 		*/
		if (chdir(shortname) < 0) {
			fprintf(stderr, "%s: Can't change directory to:  %s\n", progname, shortname);
			perror(shortname);
			return(FAIL);
		}
		infile = open(".",O_RDONLY);

		if (infile < 0) {
			fprintf(stderr, "%s: Can't open directory file:  %s\n", progname, shortname);
			perror(shortname);
			return(FAIL);
		}
		getcwd(newparent);

		while (read(infile, (char *)&dirbuf, sizeof(dirbuf)) > 0 && !term) {
			if (!dirbuf.d_ino) {
				i++;
				continue;
			}
			if (strcmp(".", dirbuf.d_name) == 0  ||
				strcmp("..", dirbuf.d_name) == 0) {
				i++;
				continue;
			}
			cp2 = cp;
			/* Tack a file name on to a directory name.
	 		*/
			for (j=0; j < DIRSIZ; j++)
				*cp2++ = dirbuf.d_name[j];

			*cp2 = '\0';
			close(infile);
			/*
	 		* Buf now should look like -
	 		*
	 		*  test/foo
	 		*       ^______ and "cp" should point here....
	 		*/
			if (putfile(pfnbuf,cp,newparent) == A_WRITE_ERR)
				return(A_WRITE_ERR);

			infile = open(".",O_RDONLY);
			i++;
			lseek(infile, (SIZE_L)(sizeof(dirbuf)*i),0);
		
		}/*E while read.. etc*/

		close(infile);

#endif

		if (chdir(curd) < 0) {
			fprintf(stderr, "%s: Can't change directory back to:  %s\n", progname, curd);
			perror(curd);
			return(FAIL);
		}
		break;

	}/*E case S_IFDIR */
/*
 */
#ifndef PRO
	case S_IFLNK:

		remaining_chctrs = written = 0L;

		if (size_of_media[CARCH]) {
		    if ((blocks_used + (SIZE_L)blocks) >
				(size_of_media[CARCH] - 3L))

			OARCH = CARCH;
		    else
			OARCH = 0;
		}

		if (stbuf.st_size + 1L  >= NAMSIZ)
			goto NAMTL;

		/* Should be safe to insert dblock stats.
		 */
		stbuf.st_size = 0L;
		tomodes(&stbuf,1,longname);


		/* Warning: 
		 *	The string returned by "readlink" is not null
		 *	terminated. The code relies on the fact that
		 *	tomodes() zeroes the entire dblock. ergo: by
		 *	definition, the linkname in dblock will be
		 *	null terminated.
		 */
		i = readlink(shortname, dblock.dbuf.linkname, (NAMSIZ - 1));
		if (i < 0) {
			perror(longname);
			return(FAIL);
		}
		dblock.dbuf.typeflag = SYMTYPE;
		sprintf(dblock.dbuf.chksum, "%6o", checksum());

		if (vflag && (CARCH >= start_archive)) {
			fprintf(stderr,"a%s %s  symbolic link to %s\n",
				MFLAG ? CARCHS : NULS, longname, dblock.dbuf.linkname);
		}

		if (writetape((char *)&dblock,1,1, shortname, longname) == A_WRITE_ERR)
			return(A_WRITE_ERR);

		break;
#endif
/*
 */
	case S_IFREG:

		if ((infile = open(shortname, O_RDONLY)) < 0) {
			fprintf(stderr, "%s: Can't open file:  %s\n", progname, longname);
			perror(longname);
			return(FAIL);
		}
		if (stbuf.st_nlink > 1) {
			found = 0;
			tomodes(&stbuf,1,longname);

			for (lp = ihead; lp; lp = lp->nextp)
				if (lp->inum == stbuf.st_ino && lp->devnum == stbuf.st_dev) {
					found++;
					break;
				}
			/* 
			 * Fix for IPR-00006.
			 * If the linked file was already output,
			 * don't output subsequent copies.
			 */
			if (found && (!strcmp(dblock.dbuf.name, lp->pathname))) {
				if (CARCH >= start_archive)
					fprintf(stderr,"%s: Linked file has already been output. Skipping:  %s\n",
						progname, lp->pathname);
				close(infile);
				return(FAIL);
			}
			if (found) {
				strcpy(dblock.dbuf.linkname, lp->pathname);
				dblock.dbuf.typeflag = LNKTYPE;

				sprintf(dblock.dbuf.chksum, "%6o", checksum());
				if (writetape((char *) &dblock,1,1,shortname,longname) == A_WRITE_ERR) {
					close(infile);
					return(A_WRITE_ERR);
				}
				if (vflag && CARCH >= start_archive) 
					fprintf(stderr,"a%s %s  link to %s\n",
					 MFLAG ? CARCHS : NULS, longname, lp->pathname);
				lp->count--;

				close(infile);
				return(SUCCEED);
			}

			lp = (struct linkbuf *) malloc(sizeof(*lp));

			if (!lp) {

				fdlist();
				lp = (struct linkbuf *) malloc(sizeof(*lp));
				if (!lp && !NMEM4L) {
					fprintf(stderr,"\n\007%s: Out of memory, link information lost\n",
					progname);

					NMEM4L++;
				}
			} 
			if (lp) {
				lcount1++;
				lp->nextp = ihead;
				ihead = lp;
				lp->inum = stbuf.st_ino;
				lp->devnum = stbuf.st_dev;
				lp->count = stbuf.st_nlink - 1;

				strcpy(lp->pathname, longname);
			} else
				lcount2++;

		}/*E if stbuf.st_nlink > 1 */

		blocks = (stbuf.st_size + (SIZE_L)(TBLOCK-1L)) / (SIZE_L)TBLOCK;

		written = 0L;
		remaining_chctrs = stbuf.st_size;

		if (size_of_media[CARCH]) {
		    if ((blocks_used + (SIZE_L)blocks) >=
				(size_of_media[CARCH] - 3L))

			OARCH = CARCH;
		    else
			OARCH = 0;
		}
		tomodes(&stbuf,blocks,longname);
		sprintf(dblock.dbuf.chksum, "%6o", checksum());

		/*
		 * Write directory header block for this file.
	 	 */
		if (writetape((char *)&dblock,1,blocks,shortname,longname) == A_WRITE_ERR) {
			close(infile);
			return(A_WRITE_ERR);
		}

		if (vflag && (CARCH >= start_archive))
			fprintf(stderr,"a%s %s %d blocks \n",
			 MFLAG ? CARCHS : NULS, longname, blocks);

		if ((start_archive - 1) > CARCH) {
			/*
			 * If skipping archives, try to avoid
			 * reading the file.
			 */
			close(infile);

			for (; blocks > 0; blocks--) {	
				if (writetape(iobuf,1,blocks,shortname,longname) == A_WRITE_ERR)
					return(A_WRITE_ERR);	

			}
		}
		else {

			while ((i = read(infile, iobuf, TBLOCK)) > 0
				&& blocks > 0) {

				written += (SIZE_L)i;

				if (writetape(iobuf,1,blocks,shortname,longname) == A_WRITE_ERR) {
					close(infile);
					return(A_WRITE_ERR);
				}

				blocks--;

			}/*E while i = ..*/

			close(infile);

			if (blocks != 0 || i != 0)
				fprintf(stderr, "%s: File changed size:  %s\n", progname, longname);
			while (--blocks >=  0)
				if (putempty()==A_WRITE_ERR)
					return(A_WRITE_ERR);

		}
		break;

/*	Special files !?!
 */
	case S_IFCHR:
	case S_IFBLK:

		remaining_chctrs = written = 0L;

		if (size_of_media[CARCH]) {
		    if ((blocks_used + (SIZE_L)blocks) >
				(size_of_media[CARCH] - 3L))

			OARCH = CARCH;
		    else
			OARCH = 0;
		}
		stbuf.st_size = 0L;
		tomodes(&stbuf,1,longname);
		sprintf(dblock.dbuf.chksum, "%6o", checksum());

		if (writetape((char *) &dblock,1,1,shortname,longname) == A_WRITE_ERR)
			return(A_WRITE_ERR);

		if (vflag && (CARCH >= start_archive))
			fprintf(stderr,"a%s %s (special file)\n",
			 MFLAG ? CARCHS : NULS, longname);

		break;

	default:
		fprintf(stderr, "%s: %s <- Is not a file. Not dumped\n", progname, longname);
	
		break;

}/*E switch (stbuf.st_mode & S_IFMT) */
return(SUCCEED);

}/*E putfile()*/

/*.sbttl tomodes()  Put file status in archive header block */

/* Function:
 *
 *	tomodes
 *
 * Function Description:
 *
 *	Put the file status modes in the tar header block
 *	for this file.
 *
 * Arguments:
 *
 *	struct stat	*sp	Pointer to filestat structure.
 *	SIZE_I	rblocks		Number of real blocks to be used by
 *				this file on the archive excluding
 *				the header block.
 *	STRING_POINTER	name	Pointer to file name string
 *
 * Return values:
 *
 *
 * Side Effects:
 *
 *	
 */
tomodes(sp,rblocks,name)
	struct stat	*sp;
	SIZE_I	rblocks;
	STRING_POINTER	name;
{
/*------*\
  Locals
\*------*/

	STRING_POINTER	cp;
	INDEX	j;
	int	majordev, minordev;
	char	pwfdat[256];

/*------*\
   Code
\*------*/

/* Zero out directory header block.
 */
for (cp = dblock.dummy; cp < &dblock.dummy[TBLOCK]; )
	*cp++ = '\0';

/* Insert file stats into directory/header block.
 */
strcpy(dblock.dbuf.name, name);
sprintf(dblock.dbuf.mode, "%6o ", sp->st_mode);
sprintf(dblock.dbuf.uid, "%6o ", sp->st_uid);
sprintf(dblock.dbuf.gid, "%6o ", sp->st_gid);
sprintf(dblock.dbuf.magic, "%6o ",sp->st_rdev);
sprintf(dblock.dbuf.size, "%11lo ", sp->st_size);
sprintf(dblock.dbuf.mtime, "%11lo ", sp->st_mtime);
dblock.dbuf.org_size[0] = ' ';

if (!NFLAG) {

	/* Insert User Group standard format indicator.
	 */
	strcpy(dblock.dbuf.magic,TMAGIC);

	if (getpw(sp->st_uid,pwfdat) && OFLAG) {
		fprintf(stderr,"\n%s: Can't find user name in password file. UID = %d\n%s: File name = %s\n\n",
			progname, sp->st_uid, progname, name);
	}
	else {
		/* Insert users' name from password file into
		 * the header block.
		 */
		for (j=0;(j<TUNMLEN && pwfdat[j] && pwfdat[j] != ':'); j++) 
			dblock.dbuf.uname[j] = pwfdat[j];

		dblock.dbuf.uname[j] = 0;
	}
	if (gp=getgrgid(sp->st_gid)) {
		char	*cp;
		cp = gp->gr_name;
		for (j=0;(j<TGNMLEN && *cp && *cp != ':'); j++, cp++)
			dblock.dbuf.gname[j] = *cp;
		dblock.dbuf.gname[j] = 0;
	}
	else {
		if (OFLAG)
			fprintf(stderr,"\n%s: Can't find group name in /etc/group file. GID = %d\n%s: File name = %s\n\n",
				progname, sp->st_gid, progname, name);

	}

	switch (sp->st_mode & S_IFMT) {

		case S_IFDIR:
			dblock.dbuf.typeflag = DIRTYPE;
			break;

		case S_IFCHR:
			dblock.dbuf.typeflag = CHRTYPE;
			goto comsd;

		case S_IFBLK:
			dblock.dbuf.typeflag = BLKTYPE;
comsd:
			majordev = sp->st_rdev >> 8;
			minordev = sp->st_rdev & 0377;
			sprintf(dblock.dbuf.devmajor, "%6o ", majordev);
			sprintf(dblock.dbuf.devminor, "%6o ", minordev);

			break;

		default:
			dblock.dbuf.typeflag = REGTYPE;
			break;

	}/*E switch sp->st_mode & S_IFMT */

	/* Put Ultrix archive numbers in the header
	 * unless a User-Group-Standard archive is desired.
	 * ie. no multi-archive extensions.
	 */
	if (!SFLAG) {
		sprintf(dblock.dbuf.carch, "%2d ",CARCH);
		sprintf(dblock.dbuf.oarch, "%2d ",OARCH);

		if (size_of_media[CARCH] || (CARCH <= start_archive)) {

		    SIZE_L  available_blocks;

		    if (OARCH) {
			/*
			 * File is being split across an archive.
			 */
			sprintf(dblock.dbuf.org_size, "%11lo ",sp->st_size);
			/*
			 * Determine how much space is left on
			 * this archive and assume one block as a
			 * minimum for any output.
			 */
			available_blocks =
			    size_of_media[CARCH] - (blocks_used + 4L);

			/* Is there at least one block for the header ?
			 */
			if (available_blocks >= 0L) {
				/*
				 * Set continuation header flag.
				 */
				header_flag = 1;

				if (rblocks > available_blocks)
				    chctrs_in_this_chunk =
				      (SIZE_L)TBLOCK * available_blocks;
				else
				    chctrs_in_this_chunk =
					(SIZE_L)TBLOCK * rblocks;

				if (remaining_chctrs >= chctrs_in_this_chunk)
					remaining_chctrs -= chctrs_in_this_chunk;
				else
					/* This will be last chunk. 
					 */
					chctrs_in_this_chunk = remaining_chctrs;

		    	    }/*T if available_blocks >= 0 */
			    else {
				/* Default chunk size to remaining
				 * chctrs because this file will
				 * start fresh on the next archive.
				 */
				chctrs_in_this_chunk = remaining_chctrs;

				header_flag = 0;

			    }/*F if available_blocks >= 0 */

			    sprintf(dblock.dbuf.size, "%11lo ", chctrs_in_this_chunk);

			    /* Re-insert mod time because previous
			     * sprintf overwrites p/o mtime field.
			     */
			    sprintf(dblock.dbuf.mtime, "%11lo ",sp->st_mtime);
			}/*E if OARCH */
		}
	}
}/*E if !NFLAG */

}/*E tomodes() */

/*.sbttl writetape() */

/* Function:
 *
 *	writetape
 *
 * Function Description:
 *
 *	Top level logic to write an output archive.
 *
 * Arguments:
 *
 *	char	*buffer		Pointer to data buffer to write from
 *	long	blocks		Number of blocks to add to those
 *				on the current archive in this call.
 *	long	total_blocks	Total number of blocks caller would
 *				like to place on the device eventually.
 *	char	*longname	Files' name. Long and short..
 *	char	*shortname
 *
 * Return values:
 *
 *	TRUE	If the number of blocks written to the device
 *		successfully.
 *
 * Side Effects:
 *
 *	When writting data, the number of blocks used is
 *	updated by the number of blocks written.
 *	If an error is detected, the routine exits via the "done"
 *	subroutine.
 */

writetape(buffer, blocks, total_blocks, shortname, longname)
	STRING_POINTER	buffer;
	SIZE_I	blocks;
	SIZE_I	total_blocks;
	STRING_POINTER	shortname;
	STRING_POINTER	longname;
{
/*------*\
  Locals
\*------*/

	STRING_POINTER	from;
	STRING_POINTER	to;
	COUNTER	i;
	FLAG	write_header;

/*------*\
   Code
\*------*/

if (FEOT)
	goto WEOA;

if (size_of_media[CARCH]) {

	if (((SIZE_L)blocks + blocks_used) >
		(size_of_media[CARCH] - 3L)) {
	    /*
	     * PUTE  is set by the main-line logic when all
	     * arg files have been output. Its' purpose is
	     * to avoid creating another archive by the coincidence
	     * of file size and media size resulting in tar end
	     * of archive (zero filled) blocks occuring within
	     * the last few blocks of the device. ie. We don't
	     * want to create another archive to simply contain
	     * an end of data (zero filled) block .. 
	     */
	    if (!PUTE) {
		blocks_used += 3L;
		MULTI++;

		/* Continuing to next vol, tack on the EOA blocks.
		 */
		if (puteoa() == A_WRITE_ERR)
			return(A_WRITE_ERR);
WEOA:
/**/
		if (VFLAG && EOTFLAG) 
			fprintf(stderr,"\n%s: %ld Blocks used on %s for %s %d\n",
				progname,
				FEOT ? (blocks_used - (SIZE_L)blocks)
				     : (blocks_used - (SIZE_L)nblock),
				usefile, Archive, CARCH);
		
		/* Remember how big this object was for possible
		 * error recovery. For tapes encountering an EOT, 3
		 * is added to the count because the pseudo writes
		 * on retry "assume" that we will need 3 blocks for
		 * end-of-archive info that really isn't present when
		 * an EOT is seen.
		 */
		if (FEOT)
			size_of_media[CARCH++] =
				(blocks_used - (SIZE_L)blocks) + 3L;
		else

			size_of_media[CARCH++] =
		    	   EOTFLAG ? (blocks_used - (SIZE_L)nblock) + 3L
			           : blocks_used;

		close(mt);
		sprintf(CARCHS, "%d", CARCH);

		/* If we have encountered a REAL end of tape, there
		 * are nblocks of data waiting to go out.
		 * If the EOT occured during flushtape(), there are
		 * "blocks" worth of data in tbuf to go out.
		 */
		if (FEOT)
			blocks_used = (SIZE_L)blocks;
		else
			blocks_used = EOTFLAG ? (SIZE_L)nblock : 0L;

		if (CARCH > start_archive) {

			if (CARCH > MAXAR) {
				fprintf(stderr,"\n%s: Reached maximum %s limit: EXITING\n",progname, Archive);
				done(FAIL);
			}
			/*
			 * Reset media size to default when
			 * resuming real writting on error recovery.
			 */
			size_of_media[CARCH] = size_of_media[0];

			fprintf(stderr,"\n\007%s: Please change %s media on  %s  & press RETURN. ",
				progname, Archive, usefile);

			response();

		}
OPENA:
/**/
		if ((mt = open(usefile, O_RDWR)) < 0) {

			fprintf(stderr, "\n\007%s: Can't open:  %s\n",
				progname, usefile);
			perror(usefile);
			fprintf(stderr,"%s: Please press  RETURN  to retry ", progname);
			response();
			goto OPENA;
		}
		if (OARCH) {
		    write_header = header_flag;
		    recno = 0;

		    if (!EOTFLAG) {
			/*
			 * __must be end of disk__
			 */
			tomodes(&stbuf, total_blocks,longname);
			sprintf(dblock.dbuf.chksum, "%6o", checksum());

			if (vflag && (CARCH >= start_archive))
			    fprintf(stderr,"%s: Continuing %D bytes of  %s  to  %s %d\n\n",
				 progname, chctrs_in_this_chunk, longname, Archive, CARCH); 

			MULTI = 0;

			if (write_header) { 
				if (writetape((char *)&dblock,1,1,shortname,longname) == A_WRITE_ERR) {
					done(A_WRITE_ERR);
					return(A_WRITE_ERR);
				}
			}
		    }/*E if !EOTFLAG */

		    if (EOTFLAG) {
			char	lastblock[TBLOCK];
			STRING_POINTER	cp;
			int	i;

			/* cremain will be zero in a rare case of a
			 * file ending on an exact EOT boundry.
			 * It does not get split across tapes.
			 */
			if (cremain) {
				if (vflag && (CARCH >= start_archive))
			    		fprintf(stderr,"%s: Continuing %D bytes of  %s  to  %s %d\n\n",
					progname, cremain, cblock.dbuf.name, Archive, CARCH); 

				/* Save the last "buffered" block.
				 */
#ifdef PRO
				for (from = (char *)&tbuf[nblock-1],
				     to = (char *)lastblock,
				     i = TBLOCK; i; i--)

					*to++ = *from++;
#else
				bcopy((char *)&tbuf[nblock-1],
					lastblock, TBLOCK);
#endif
				/* Shuffle the buffer content down
				 * one block.
				 */
				for (from = (char *)&tbuf[nblock-2],
				     to = (char *)&tbuf[nblock-1],
				     i = nblock-1; i; i--) {

					COUNTER j;

					for (j = TBLOCK; j; j--)

						*to++ = *from++;

					/* Back up the pointers for
					 * the next block.
					 */
					to = from - TBLOCK;
					from = to - TBLOCK;
				}

				/* Initialize continuation header block.
				 */
				sprintf(cblock.dbuf.carch,"%2d ",CARCH);
				sprintf(cblock.dbuf.oarch,"%2d ",OARCH);
			    	sprintf(cblock.dbuf.size, "%11lo ",cremain);
			    	sprintf(cblock.dbuf.mtime, "%11lo ",cmtime);
				sprintf(cblock.dbuf.org_size, "%11lo ",corgsize);

				/* Compute new checksum
				 */
				for (cp = cblock.dbuf.chksum;
				      cp < &cblock.dbuf.chksum[sizeof(cblock.dbuf.chksum)];
					cp++)

					*cp = ' ';

				for (cp = cblock.dummy;
				     cp < &cblock.dummy[TBLOCK]; cp++)

					i += *cp;

				sprintf(cblock.dbuf.chksum, "%6o", i);
				
				/* Copy in continuation header block
				 */
#ifdef PRO
				for (from = (char *)&cblock,
				     to = (char *)&tbuf[0],
				     i = TBLOCK; i; i--)

					*to++ = *from++;
#else
				bcopy((char *)&cblock,
					(char *)&tbuf[0],TBLOCK);
#endif
			}/*E if cremain */

			/* Write out the new buffer
			 */
			if ((write(mt,tbuf, TBLOCK*nblock)) < 0)

				goto WERR;

			if (cremain) {
				/* Copy the saved lastblock to the start
				 * of the buffer for the next write.
				 */
#ifdef PRO
				for (from = (char *)lastblock,
				     to = (char *)&tbuf[recno++],
				     i = TBLOCK; i; i--)

					*to++ = *from++;
#else
				bcopy(lastblock,(char *)&tbuf[recno++],TBLOCK);
#endif
				blocks_used++;

			}/*E if cremain */

			MULTI = EOTFLAG = OARCH = 0;

			return(SUCCEED);

		    }/*E if EOTFLAG */
		}/*if OARCH */
	    }/*E if !PUTE */
	}/*E if (SIZE_L)blocks + blocks_used ..*/
}/*E if size_of_media[CARCH] */

blocks_used += (SIZE_L)blocks;

#ifdef PRO
for (from = buffer, to = (char *)&tbuf[recno++], i=TBLOCK; i; i--)
	*to++ = *from++;
#else
/*
 * Use the system subroutine call for much better speed !
 */
bcopy(buffer, (char *)&tbuf[recno++], TBLOCK);

#endif

if (recno >= nblock) {
	if (CARCH >= start_archive) {

		if ((write(mt, tbuf, TBLOCK*nblock)) < 0 ) {

			if ((ioctl(mt, MTIOCGET, &mtsts)<0) ||
				size_of_media[CARCH] ||
				(errno != ENOSPC) || NFLAG) {

WERR:
/**/
				fprintf(stderr,"\n");
				perror(usefile);
				fprintf(stderr, "\007%s: Archive  %d  write error\n", progname, CARCH);
				done(A_WRITE_ERR);
				return(A_WRITE_ERR);
			}
			else {
				if (!(mtsts.mt_softstat & MT_EOT))
					goto WERR;
				else {
					mtops.mt_op = MTCSE;

					if (ioctl(mt, MTIOCTOP, &mtops) < 0) 
						goto WERR;
					else {
	        				fprintf(stderr,"\n%s: End of %s media", progname, Archive);
						OARCH = CARCH;
						EOTFLAG++;
						MULTI++;
						goto WEOA;
					}
				}
			}
		}
		/* Save possible continuation block when a
		 * new/different file is started.
		 */
		if (new_file) {
#ifdef PRO
			for (from = (char *)&dblock,
			       to = (char *)&cblock,
			        i = TBLOCK; i; i--)

				*to++ = *from++;
#else
			bcopy((char *)&dblock,(char *)&cblock,TBLOCK);
#endif
			new_file = FALSE;
		}

		cremain = remaining_chctrs - written;
		corgsize = stbuf.st_size;
		cmtime = stbuf.st_mtime;

	}/*E if (CARCH >= start_archive) */

	recno = 0;
}
return (SUCCEED);

}/*E writetape() */

