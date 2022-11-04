
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#ifndef lint
static	char	*sccsid = "@(#)readtape.c	3.1	(ULTRIX)	9/5/87";
#endif	lint

/**/
/*
 *
 *	File name:
 *
 *		readtape.c
 *
 *	Source file description:
 *
 *		Mainly functions concerned with reading
 *		from the input archive by the "tar" logic.
 *
 *	Functions:
 *
 *	Usage:	n/a
 *
 *	Compile:
 *
 *	Modification history:
 *	~~~~~~~~~~~~~~~~~~~~
 *
 *	revision			comments
 *	--------	-----------------------------------------------
 *	I		19-Dec-85 rjg/
 *			Create orginal version.
 *	
 */
#include "tar.h"

/*.sbttl backtape() */

/* Function:
 *
 *	backtape
 *
 * Function Description:
 *
 *	Backspace tape (if possible). Part of the "update" function.
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
backtape()
{
/*------*\
  Locals
\*------*/

static	int	mtdev = 1;
static	struct	mtop	lmtop = {MTBSR, 1};

/*------*\
   Code
\*------*/

if (mtdev == 1)
	mtdev = ioctl(mt, MTIOCGET, &mtsts);

if (mtdev == 0) {
	if (ioctl(mt, MTIOCTOP, &lmtop) < 0) {
		fprintf(stderr, "%s: Archive backspace error\n", progname);
		done(FAIL);
	}
} else

	lseek(mt, (SIZE_L) -TBLOCK * nblock, 1);

recno--;

}/*E backtape() */

/*.sbttl bread() */

/* Function:
 *
 *	bread
 *
 * Function Description:
 *
 *	Buffer / block read
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
bread(fd, buf, size)
	FILE_D	fd;
	STRING_POINTER	buf;
	SIZE_I	size;
{
/*------*\
  Locals
\*------*/

	SIZE_I	bytes;
	SIZE_I	count;
	SIZE_I	lastread = 0;

/*------*\
   Code
\*------*/

NEWVOL:
/**/
if (!Bflag) {
	bytes = read(fd, buf, size);

	if ((bytes <= 0) && (errno == ENOSPC)) {
		if ((ioctl(mt, MTIOCGET, &mtsts) < 0) ||
			size_of_media[CARCH])
			return(bytes);
		else {
			if (!(mtsts.mt_softstat & MT_EOT))
				return(bytes);
			else {
				if (blocks >= 0)
					/* blocks < 0 implies that a
					 * file ended on an exact EOT
					 * boundry and is not continued
					 * to the next archive.
					 */
					FILE_CONTINUES++;
				EOTFLAG++;
				MULTI++;

				/* Go announce end of media & get user
				 * to mount the next tape archive.
				 */
				getdir();

				/* Now go read the requested amount of
				 * data from the newly mounted tape.
				 */
				goto NEWVOL;
			}
		}
	}
	else
		return(bytes);
}

for (count = 0; count < size; count += lastread) {
	if (lastread < 0) {
		if (count > 0)
			return (count);

		return (lastread);
	}

	lastread = read(fd, buf, size - count);

	/* Zero byte read indicates EOF.
	 * Return that or the count of last good read.
	 * ie. If we never read anything, count will be 0.
	 */
	if (!lastread) 
		return(count);

	buf += lastread;

}/*E for count = 0 ..*/

return (count);

}/*E bread() */

/*.sbttl bsrch() */

/* Function:
 *
 *	bsrch
 *
 * Function Description:
 *
 *	Part of the "update" archive function.
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
daddr_t bsrch(s, n, l, h)
	daddr_t	l, h;
	STRING_POINTER	s;
{
/*------*\
  Locals
\*------*/

	char	b[N];
	int	i, j;
	daddr_t	m, m1;

/*------*\
   Code
\*------*/

njab = 0;

loop:
/*--:
 */
if (l >= h)
	return (-1L);

m = l + (h-l)/2 - N/2;
if (m < l)
	m = l;

fseek(tfile, m, 0);
fread(b, 1, N, tfile);
njab++;

for(i=0; i<N; i++) {
	if (b[i] == '\n')
		break;
	m++;
}
if (m >= h)
	return (-1L);
m1 = m;
j = i;

for(i++; i<N; i++) {
	m1++;
	if (b[i] == '\n')
		break;
}
i = cmp(b+j, s, n);
if (i < 0) {
	h = m;
	goto loop;
}
if (i > 0) {
	l = m1;
	goto loop;
}
return (m);

}/*E bsrch() */

/*.sbttl checkdir() */

/* Function:
 *
 *	checkdir
 *
 * Function Description:
 *
 *	Checks for existance of a directory on disk. If it is
 *	not there, the function trys to create it.
 *
 * Arguments:
 *
 *	char	*name	Pointer to directory name character string
 *
 * Return values:
 *
 *
 * Side Effects:
 *
 *
 */
checkdir(name)
	STRING_POINTER	name;
{
/*------*\
  Locals
\*------*/

	STRING_POINTER	cp;
	int	i;

/*------*\
   Code
\*------*/

/*
 * Quick check for existance of directory.
 */
if (!(cp = rindex(name, '/')))
	return (0);

*cp = '\0';
if (access(name, 0) >= 0) {
	*cp = '/';
	return (cp[1] == '\0');
}

*cp = '/';
/*
 * No luck, try to make all directories in path.
 */
for (cp = name; *cp; cp++) {
	if (*cp != '/')
		continue;

	*cp = '\0';
	if (access(name, 0) < 0) {
#ifndef U11
		if (mkdir(name, 0777) < 0) {
			perror(name);
			*cp = '/';
			return (FAIL);
		}
#endif
#ifdef U11
		if (!fork()) {
			execl("/bin/mkdir","mkdir",name,0);
			fprintf(stderr,"%s: Failed to execute /bin/mkdir  for:  %s\n", progname, name);
			perror(name);
			done(FAIL);
		}
/*
 * Check returned status. If status came back (byte of zero = normal)
 * then the high byte is the exit status. Mkdir returns a high byte
 * of zero if ok, and non-zero otherwise. If zero returns, we notify the
 * user dir made ok (if vflag) otherwise mkdir would have displayed an
 * error.
 *
 */
		while(wait(&i) > 0)
			if ((i & 0377) == 0)
				if ((i & ~0377) != 0) {
					fprintf(stderr,"%s: Error making directory:  %s\n", progname, name);
					perror(name);
					done(FAIL);
				}
				
#endif
	if (VFLAG)
		fprintf(stderr,"x %s/ (directory created)\n", name);
	else
		if (vflag)
			fprintf(stderr,"x %s/\n", name);

	if (pflag)
		chown(name, stbuf.st_uid, stbuf.st_gid);

	if (pflag && cp[1] == '\0')
		chmod(name, stbuf.st_mode & 07777);

	}/*E if (access(name, 0) < 0) */

	*cp = '/';

}/*E for (cp = name; *cp; cp++) */

return (cp[-1]=='/');

}/*E checkdir() */

/*.sbttl CHECKF() */

#ifdef TAR40

/* Function:
 *
 *	CHECKF
 *
 * Function Description:
 *
 *   Routine to determine whether the specified file should
 *   be skipped or not. Implements the F, FF and FFF modifiers.
 *
 *   The set of files skipped depends upon the value of howmuch.
 *
 *   When howmuch > 0 the set of skip files consists of:
 *
 *		SCCS directories
 *		core files
 *		errs files
 *
 *    Howmuch > 1 increases the skip set to include:
 *
 *		a.out
 *		*.o
 *
 *    Howmuch > 2 causes executable files to be skipped
 *    as well. (A file is determined to be executable by
 *    looking at its "magic numbers")
 *
 *
 * Arguments:
 *
 *	char	*longname	Pointer to name string of file to
 *				to be checked
 *	int	mode		File mode bits of the file
 *	int	howmuch		Skip set modifier
 *
 * Return values:
 *
 *	1	The file is to be skipped
 *	0	The file is not to be skipped
 *
 * Side Effects:
 *
 *	The routine assumes that the file is in the current directory.
 *	The routine is not fully implemented as described above. See
 *	the #ifdef below.. 
 *	
 */
CHECKF(longname, mode, howmuch)
	STRING_POINTER	longname;
	int	mode;
	int	howmuch;
{
/*------*\
  Locals
\*------*/

	STRING_POINTER	shortname;

/*------*\
   Code
\*------*/

/* Strip off any directory and get the base filename.
 */ 
if ((shortname = rindex(longname, '/')))
	shortname++;
else
	shortname = longname;

/* Basic skip set ?
 */
if (howmuch > 0) {
	/*
	 * Skip SCCS directories on input
	 */
	if ((mode & S_IFMT) == S_IFDIR)
		return (strcmp(shortname, "SCCS") != 0);

	/*
	 * Skip SCCS directory files on extraction
	 * Check SCCS as a toplevel directory and
	 * as a subdirectory.
	 */
#ifdef TAR40
	if (STRFIND(longname,"SCCS/") == longname || STRFIND(longname,"/SCCS/"))
#else
	if (strfind(longname,"SCCS/") == longname || strfind(longname,"/SCCS/"))
#endif
		return (0);
	/*
	 * Skip core and errs files. 
	 */
	if (strcmp(shortname,"core")==0 || strcmp(shortname,"errs")==0)
		return (0);

}/*E if howmuch > 0 */

/* First level additions ?
 */
if (howmuch > 1) {
	int l;

	l = strlen(shortname);	/* get string len */

	/* Skip .o files
	 */
	if (shortname[--l] == 'o' && shortname[--l] == '.')
		return (0);

	/* Skip a.out
	 */
	if (strcmp(shortname, "a.out") == 0)
		return (0);

}/*E if howmuch > 1 */

#ifdef notdefFFF
/*
 * This routine works for -c and -r options, but is
 * not sufficent for -x option.  Since it cannot be implemented
 * fully, it is not implemented at all at this time.
 */
/*
 * Second level additions ?
 */
if (howmuch > 2) {
	/*
	 * Open the file to examine the magic numbers.
	 * If the file cannot be opened, then assume
	 * that it is not to be skipped and that another
	 * routine will perform the error handling for it.
	 * Otherwise, read from the file and check the
	 * magic numbers, indicate skipping if they are good.
	 */
	int	ifile, in;
	struct exec	hdr;

	ifile = open(shortname,O_RDONLY,0);

	if (ifile >= 0)	{
		in = read(ifile, &hdr, sizeof (struct exec));
		close(ifile);
		if (in > 0 && ! N_BADMAG(hdr))	/* executable: skip */
			return (0);
	}
}/* end if howmuch > 2 */
#endif notdefFFF

return (1);	/* Default action is not to skip */

}/*E CHECKF() */
#endif
/*.sbttl CHECKSUM() */

#ifdef TAR40

/* Function:
 *
 *	CHECKSUM
 *
 * Function Description:
 *
 *	Perform first-level CHECKSUM for file header block.
 *
 * Arguments:
 *
 *	none
 *
 * Return values:
 *
 *	The CHECKSUM value is returned.
 *
 * Side Effects:
 *
 *	
 */
CHECKSUM()
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

}/*E CHECKSUM() */
#endif
/*.sbttl checkw() */

/* Function:
 *
 *	checkw
 *
 * Function Description:
 *
 *	Checks to see if the user wants this file or not if
 *	the wait flag is set. Outputs the yes/no and name of
 *	the file, gets a response and returns.
 *
 * Arguments:
 *
 *	char	*name	Pointer to the current file name string
 *
 * Return values:
 *
 *	Non-zero	User wants this file. Default case if the
 *			wait flag is not set.
 *	zero		User does not want this file. Set only if
 *			the wait function was requested in the
 *			command line, and user types a no to our
 *			query.
 *
 * Side Effects:
 *
 *	
 */
checkw(c, name)
	char *name;
{
/*------*\
   Code
\*------*/

if (!wflag)
	return (1);

fprintf(stderr,"%c ", c);

if (vflag)
	longt(&stbuf);

fprintf(stderr,"%s: ", name);
	return (response() == 'y');

}/*E checkw() */

/*.sbttl cmp() */

/* Function:
 *
 *	cmp
 *
 * Function Description:
 *
 *	Part of the "update" archive function.
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
cmp(b, s, n)
	STRING_POINTER	b;
	STRING_POINTER	s;
{
/*------*\
  Locals
\*------*/

	INDEX	i;

/*------*\
   Code
\*------*/

if (b[0] != '\n')
	exit(2);

for (i = 0; i < n; i++) {
	if (b[i+1] > s[i])
		return (-1);

	if (b[i+1] < s[i])
		return (1);
}
return (b[i+1] == ' '? 0 : -1);

}/*E cmp() */

/*.sbttl dotable()  Top level logic for "t" function */

/* Function:
 *
 *	dotable
 *
 * Function Description:
 *
 *	Top level logic to do the Table of Contents function
 *	for an input archive.
 *
 * Arguments:
 *
 *	none
 *
 * Return values:
 *
 *
 * Side Effects:
 *
 *	
 */
dotable()
{

for (;;) {
	extracted_size = 0L;
TNEXTA:
/**/
	getdir();

	if (endtape()) {
		if (FILE_CONTINUES)
			goto TNEXTA;

		if (VFLAG)
			printf("\n%s: %ld Blocks used on %s for archive  %d\n\n",
				progname, blocks_used, usefile, CARCH);
		break;
	}
	dostats();
	
	/* Check for F[FF] operation, skipping SCCS stuff, etc..
 	 */
#ifdef TAR40
	if (Fflag && !CHECKF(dblock.dbuf.name, stbuf.st_mode, Fflag)) {
#else
	if (Fflag && !checkf(dblock.dbuf.name, stbuf.st_mode, Fflag)) {

#endif
		passtape();
		continue;
	}
	
	passtape();

}/*E for ;; */

}/*E dotable() */

dostats()
{
	/*
 	 * Save last file name  &  mod time for possible
 	 * archive switch compare.
 	 */

	strcpy(file_name,dblock.dbuf.name);
	modify_time = stbuf.st_mtime;

	if (dblock.dbuf.typeflag == DIRTYPE) {
		if (vflag)
			longt(&stbuf);

		printf("%s", dblock.dbuf.name);
	}
	else {
		if (vflag)
			longt(&stbuf);

		printf("%s", dblock.dbuf.name);

		if (!vflag && OARCH)
			printf("  <- continued from archive  %d,  Current archive is  %d, Original archive is  %d",
				(CARCH-1), CARCH, OARCH);
	}
	if (dblock.dbuf.typeflag == LNKTYPE)
		printf(" linked to %s", dblock.dbuf.linkname);

	if (dblock.dbuf.typeflag == SYMTYPE)
		printf(" symbolic link to %s", dblock.dbuf.linkname);

	if (VFLAG && dblock.dbuf.typeflag == CHRTYPE)
		printf(" (character special)");

	if (VFLAG && dblock.dbuf.typeflag == BLKTYPE)
		printf(" (block special)");
	
	printf("\n");

}/*E dostats() */

/*.sbttl doxtract() */

/* Function:
 *
 *	doxtract
 *
 * Function Description:
 *
 *	eXtracts (reads) files from an input archive and places
 *	them into disk files.
 *
 * Arguments:
 *
 *	char	*argv	Pointer to list of input file names
 *
 * Return values:
 *
 *
 * Side Effects:
 *
 *	
 */
doxtract(argv)
	char *argv[];
{
/*------*\
  Locals
\*------*/

	char	**cp;
	char	buf[TBLOCK];
	FILE_D	ofile;

/*------*\
   Code
\*------*/

for (;;) {
	extracted_size = 0L;
	getdir();
	if (endtape()) {

		if (VFLAG)
			fprintf(stderr,"\n%s: %ld Blocks used on %s for archive  %d\n\n",
				progname, blocks_used, usefile, CARCH);
			break;
	}
NEW_FILE:
/**/
	/* Save last file name  &  modify time for possbile
	 * archive switch ck.
	 */
	strcpy(file_name,dblock.dbuf.name);
	modify_time = stbuf.st_mtime;
	
	if (!*argv)
		goto gotit;

	for (cp = argv; *cp; cp++)
		if (prefix(*cp, dblock.dbuf.name))
			goto gotit;

	passtape();
	continue;

gotit:
/*---:
 */
	/* (w) Pass user conf.? No: skip file.
	 */
	if (wflag) {
		if (!checkw('x', dblock.dbuf.name)) {
			passtape();
			continue;
		}
	}

	/* (F[FF]) Fast? Check to see if the file is in the skip set.
	 */
	if (Fflag) {
#ifdef TAR40
		if (!CHECKF(dblock.dbuf.name, stbuf.st_mode, Fflag)) {
#else
		if (!checkf(dblock.dbuf.name, stbuf.st_mode, Fflag)) {

#endif
			passtape();
			continue;
		}
	}
	if (checkdir(dblock.dbuf.name))
		continue;

	/* See if this is a symbolic link.
	 */
	if (dblock.dbuf.typeflag == SYMTYPE) {
		/*
		 * Fix for IPR-00014. Prevent tar from
		 * unlinking non-empty directories.
		 *
		 * Only unlink non-directories or
		 * empty directories.
		 */
#ifndef U11
		if (rmdir(dblock.dbuf.name) < 0) {
			if (errno == ENOTDIR)
				unlink(dblock.dbuf.name);
			else
				if (errno == ENOTEMPTY)
					perror(progname);
		}

		if (symlink(dblock.dbuf.linkname, dblock.dbuf.name)<0) {
			fprintf(stderr, "%s: Symbolic link failed:  %s\n",
				progname, dblock.dbuf.name);

			perror(dblock.dbuf.name);
			continue;
		}
#else
		if (stat(dblock.dbuf.name, &stbuf) != ENOENT) {
			if ((stbuf.st_mode & S_IFMT) == S_IFDIR) {
				if (rmdir(dblock.dbuf.name) < 0) {
					fprintf(stderr,"%s: Can't remove directory symbolically linked to by  %s\n",
					progname,dblock.dbuf.name);

					perror(dblock.dbuf.name);
				}
			}
		}
		unlink(dblock.dbuf.name);

#ifndef PRO
		if (symlink(dblock.dbuf.linkname, dblock.dbuf.name) < 0) {
#else
		if (link(dblock.dbuf.linkname, dblock.dbuf.name) < 0) {
#endif
			fprintf(stderr, "%s: Can't link:  %s\n", progname, dblock.dbuf.name);
			perror(dblock.dbuf.name);
			continue;
		}
#endif
		if (vflag)
			fprintf(stderr,"x %s symbolic link to %s\n", dblock.dbuf.name, dblock.dbuf.linkname);

		if (pflag)
			chown(dblock.dbuf.name, stbuf.st_uid, stbuf.st_gid);

		continue;
	}/*E if (dblock.dbuf.typeflag == SYMTYPE) */

	/* See if this is a hard link.
	 */
	if (dblock.dbuf.typeflag == LNKTYPE) {

		/* Fix for IPR-00014. Prevent tar from
		 * unlinking non-empty directories.
		 *
		 * Only unlink non-directories, or
		 * empty directories.
		 */
#ifndef U11
		if (rmdir(dblock.dbuf.name) < 0) {

			if (errno == ENOTDIR)
				unlink(dblock.dbuf.name);
			else
				if (errno == ENOTEMPTY)
					perror(progname);
		}
#else
		if (stat(dblock.dbuf.name, &stbuf) != ENOENT) {
			if ((stbuf.st_mode & S_IFMT) == S_IFDIR) {
				if (rmdir(dblock.dbuf.name) < 0) {
					fprintf(stderr,"%s: Can't remove directory linked to by  %s\n",progname,dblock.dbuf.name);
					perror(dblock.dbuf.name);
				}
			}
			else 
				unlink(dblock.dbuf.name);
		}
#endif
		if (link(dblock.dbuf.linkname, dblock.dbuf.name) < 0) {
			fprintf(stderr, "%s: Can't link:  %s\n", progname, dblock.dbuf.name);
			perror(dblock.dbuf.name);
			continue;
		}
		if (vflag)
			fprintf(stderr,"%s linked to %s\n", dblock.dbuf.name, dblock.dbuf.linkname);
		if (pflag)
			chown(dblock.dbuf.linkname, stbuf.st_uid, stbuf.st_gid);


		continue;

	}/*E if dblock.dbuf.typeflag == LNKTYPE */

	/*
	 * Are we extracting a special file ?
	 */
	if (((stbuf.st_mode & S_IFMT) == S_IFCHR) ||
	      ((stbuf.st_mode & S_IFMT) == S_IFBLK)) {	

		if ((ofile = mknod(dblock.dbuf.name,
		     stbuf.st_mode,stbuf.st_rdev)) < 0 ) {

			fprintf(stderr,"%s: Can't create special file:  %s\n", progname,dblock.dbuf.name);
			perror(dblock.dbuf.name);
			passtape();
			continue;
		}
		if (vflag)
			fprintf(stderr,"x %s, (special file)\n", dblock.dbuf.name);
		continue;	

	}/*E if st->st_mode etc..*/

	/*
	 * Assume a regular type of file.
	 */
	if ((ofile = creat(dblock.dbuf.name,stbuf.st_mode&0xfff)) < 0) {
		fprintf(stderr, "%s: Can't create:  %s\n", progname, dblock.dbuf.name);
		perror(dblock.dbuf.name);
		passtape();
		continue;
	}

NEXT_CHUNK:
/**/
	bytes = stbuf.st_size;
	blocks = (bytes + (SIZE_L)TBLOCK - 1L) / (SIZE_L)TBLOCK;

	if (vflag) {
		if (OARCH  && (original_size > chctrs_in_this_chunk)) 
#ifndef U11
			fprintf(stderr,"x %s, %ld bytes of %ld, %d blocks\n",
				dblock.dbuf.name, chctrs_in_this_chunk,
				original_size, blocks);
		else
			fprintf(stderr,"x %s, %ld bytes, %d blocks\n",
				dblock.dbuf.name, bytes, blocks);

#else
			fprintf(stderr,"x %s, %ld bytes of %ld, %d tape blocks\n",
				dblock.dbuf.name, chctrs_in_this_chunk,
				original_size, blocks);
		else
			fprintf(stderr,"x %s, %ld bytes, %d tape blocks\n",
				dblock.dbuf.name, bytes, blocks);
#endif
	}

	for (; blocks-- > 0; bytes -= (SIZE_L)TBLOCK) {
		readtape(buf);

		if (bytes > (SIZE_L)TBLOCK) {
			if (write(ofile, buf, TBLOCK) < 0) {
				fprintf(stderr, "%s: Write error on extract:  %s\n",
					progname, dblock.dbuf.name);

				perror(dblock.dbuf.name);
				done(FAIL);
			}
			continue;
		}
		if (write(ofile, buf, (int) bytes) < 0) {
			fprintf(stderr,"%s: Write error on extract:  %s\n",
				progname, dblock.dbuf.name);

			perror(dblock.dbuf.name);
			done(FAIL);
		}
	}/*E for ; blocks-- > 0 ..*/
	
	/* Have we extracted all of the file ?
	 * Non UMA files can't be continued, by definition.
	 */
	if ((hdrtype != UMA) || (extracted_size == original_size)) {
		close(ofile);
	}
	else {
		/* Come here when we need to process another non-tape
		 * archive to get more of the file. For tapes, the
		 * archive switching is transparant to this logic.
		 * It is delt with by the readtape(), bread(), and
		 * getdir() routines.
		 */
		char last_file[NAMSIZ+1];

		strcpy(last_file,dblock.dbuf.name);
		modify_time = stbuf.st_mtime;
		getdir();
		if (endtape()) {

			/* If this is the last archive, we will never 
			 * see the rest of the file. Give up.
			 */
			if (!FILE_CONTINUES) {
				fprintf(stderr,"\n%s: File  %s  not correctly extracted \n\n",
				  progname, last_file);

				close(ofile);

				if (VFLAG)
					fprintf(stderr,"\n%s: %ld Blocks used on %s for archive  %d\n\n",
						progname, blocks_used,
						usefile, CARCH);
					done(FAIL);
			}
			else {
				/*
		 	 	* Else, next archive should be inserted.
		 	 	*/
				getdir();
				goto NEXT_CHUNK;
			}
		}
		else {
			fprintf(stderr,"\n%s: File  %s  not correctly extracted \n\n",
			  progname, last_file);

			close(ofile);
			goto NEW_FILE;
		}
	}
	if (pflag)
		chown(dblock.dbuf.name, stbuf.st_uid, stbuf.st_gid);

#ifndef U11 
	if (!mflag) {
		struct timeval tv[2];

		tv[0].tv_sec = time(0);
		tv[0].tv_usec = 0;
		tv[1].tv_sec = stbuf.st_mtime;
		tv[1].tv_usec = 0;
		utimes(dblock.dbuf.name, tv);
	}
#endif

#ifdef U11
	if (!mflag) {
		time_t timep[2];

		timep[0] = time(NULL);
		timep[1] = stbuf.st_mtime;
		utime(dblock.dbuf.name, timep);
	}
#endif

	if (pflag)
		chmod(dblock.dbuf.name, stbuf.st_mode & 07777);

}/*E for ;; */

}/*E doxtract()*/

/*.sbttl endtape()  Determine if end of archive reached */

/* Function:
 *
 *	endtape
 *
 * Function Description:
 *
 *	Decide if the end of archive has been reached.
 *
 * Arguments:
 *
 *	None explictly stated. However, the routine expects that
 *	"dblock" has been filled with the contents of a file
 *	header block from the input archive.
 *
 * Return values:
 *
 *	The content of  dblock.dbuf.name  is returned.
 *	If zero, tar has seen its' end of archive indicator.
 *	Else, there is yet more data on this archive.
 *
 * Side Effects:
 *
 *	none
 */
endtape()
{

EODFLAG = FILE_CONTINUES = 0;

/* If this is an empty block, read one more block so we don't break
 * our pipe by closing down before reading ALL the blocks ! 
 * From a tape, it doesn't matter, but thru a pipe...
 */ 

if (pipein) {
	if (dblock.dbuf.name[0] == '\0') {
		sleep(2);
		readtape((char *)&dblock);
		return(TRUE);
	}
}

/* For multi_archive operations, normally read thru the 2 zero-filled
 * blocks and return status from next one. This block will tell
 * us if the file is continued to another archive - but only if
 * the last file on the archive was written by this version of tar.
 * For appends and updates, we need to NOT read thru the zero-blocks
 * in order to maintain correct physical postion.
 */

if (OARCH && rflag && !dblock.dbuf.name[0] && size_of_media[CARCH]) {
	/*
	 * The current file either is the last one on the media
	 * or the first and is continued from a previous archive.
	 */
	if (blocks_used == (size_of_media[CARCH] - 2L)) {

		/* The file must be the last one and is continued
		 * to next archive.
		 */
		blocks_used += 2L;
		EODFLAG = FILE_CONTINUES = TRUE;
		return(TRUE);
	}
	else {
		/* There is more room on the archive.
		 */
		blocks_used -= 1;
		return(TRUE);
	}
}

/* Deal with an append to an archive not beginning or ending
 * with a continued file.
 */
if (!dblock.dbuf.name[0] && rflag) {
	blocks_used--;
	return(TRUE);
}

/* Now, if NOT dealing with an update or rappend  AND
 * the last file was an Ultrix Multi Archive format.
 */
if ((dblock.dbuf.name[0] == '\0') && (hdrtype == UMA)) {

	EODFLAG = TRUE;
	readtape((char *)&dblock);
	readtape((char *)&dblock);

	if (dblock.dbuf.name[0]) {

		FILE_CONTINUES++;

		strcpy(file_name,dblock.dbuf.name);
		dblock.dbuf.name[0] = '\0';
	}

	return (dblock.dbuf.name[0]=='\0');
}

/* Adjust the block count for archives not ending with a file
 * written in "UMA" format.
 */

if (dblock.dbuf.name[0] == '\0') {
	if (!rflag)
		blocks_used++;
	else
		blocks_used--;
}
return (dblock.dbuf.name[0]=='\0');

}/*E endtape() */

/*.sbttl getdir() */

/* Function:
 *
 *	getdir
 *
 * Function Description:
 *
 *	Reads a file header block from the input archive.
 *	Extract desired information from it and place in
 *	appropriate variables.
 *
 *	This routine also determines the "type" of tar header.
 *	and sets control variable "hdrtype"; 
 *
 * Arguments:
 *
 *	none
 *
 * Return values:
 *
 *	none
 *
 * Side Effects:
 *
 *	Routine may exit to the system if a checksum error
 *	is detected and the -i switch was not specified.
 */

getdir()
{
/*------*\
  Locals
\*------*/

	FLAG	retrying = 0;
	int	i;
	int	majordev, minordev;
	int	openm;
	struct	passwd	*pwp;
	FLAG	reload;
	struct	stat	*sp;
	SIZE_I	tcarch;
	SIZE_I  toarch;


/*------*\
   Code
\*------*/

sp = &stbuf;
	
/*
 * Determine how much space is left on
 * this archive and assume one block as a
 * minimum for any output.
 */
if(size_of_media[0]){
	SIZE_L available_blocks;

	available_blocks = size_of_media[CARCH] - (nblock + blocks_used +3L);
	if(available_blocks < 0L && !FILE_CONTINUES)
		goto EOTD;
}

/* If the condition  EOTFLAG && !VOLCHK  is true,
 * bread() has detected a real tape EOT marker & is calling us
 * to announce the end of media & to request the user to mount
 * the next archive.
 */
if ((EOTFLAG && !VOLCHK) || (FILE_CONTINUES && EODFLAG))
	goto EOTD;

/* The following test implies that  readtape()  has been told of a
 * real tape EOT by bread() & is calling us to verify that the correct
 * (next) archive was mounted.
 */
if (EOTFLAG && VOLCHK)
	goto CHKVOL;

top:
NEXTVOL:
/**/

readtape((char *)&dblock);

if (!dblock.dbuf.name[0]) {
	
	if (EODFLAG || EOTFLAG)
		goto CHKVOL;
	else
		return;
}

if (FILE_CONTINUES) {

EOTD:
/**/
	nextvol = CARCH+1;

	if (VFLAG) {
		if (MULTI)
			fprintf(stderr,"\n%s: End of archive media\n",progname);
		else
			fprintf(stderr,"\n");

		fprintf(stderr,"%s: %ld Blocks used on %s for archive  %d\n",
			progname, blocks_used, usefile, CARCH);
	}
	if (FILE_CONTINUES)
		fprintf(stderr,"\n%s: File  %s  is continued on archive  %d",
	 	progname, file_name, nextvol);

	FILE_CONTINUES = 0;

RELOAD:
/**/
	close(mt);
	blocks_used = 0L;
	recno = 0;
	first = 0;
	if (retrying)
		fprintf(stderr,"\n%s: Please load correct archive on  %s  & press RETURN ",
			progname, usefile);
	else
		fprintf(stderr,"\n\007%s: Please load archive  %d  on  %s  & press RETURN ",
			progname, nextvol, usefile);

	response();
	fprintf(stderr,"\n");

	if (tflag || xflag)
		openm = O_RDONLY;
	else
		openm = O_RDWR;
OPENT:
/**/
	if ((mt = open(usefile, openm)) < 0) {
		fprintf(stderr, "%s: Can't open:  %s\n",
			progname, usefile);

		perror(usefile);
		fprintf(stderr,"\n%s: Please press  RETURN  to retry ",progname);
		response();
		goto OPENT;
	}
	MULTI = 0;
	if (!EOTFLAG)
		goto NEXTVOL;
	else
		return; /*<- go back to bread() or readtape() */
}
sscanf(dblock.dbuf.mode, "%o", &i);
sp->st_mode = i;
sscanf(dblock.dbuf.uid, "%o", &i);
sp->st_uid = i;
sscanf(dblock.dbuf.gid, "%o", &i);
sp->st_gid = i;
/*
 * If this is an older version of Ultrix tar archive, this
 * will extract (if any) the major/minor device number of
 * a "special" file.
 */
sscanf(dblock.dbuf.magic,"%o", &i);
sp->st_rdev = i;
sscanf(dblock.dbuf.size, "%lo", &sp->st_size);
sscanf(dblock.dbuf.mtime, "%lo", &sp->st_mtime);
sscanf(dblock.dbuf.chksum, "%o", &chksum);

/*
 * Determine  true  tar  header format type  and 
 * announce result if requested.
 */
OARCH = 0;
volann++;
/*
 * Default to Original Tar Archive
 */
hdrtype = OTA;

if (!dblock.dbuf.magic[0]) {
	if (VFLAG && !volann)
	    printf("%s: Original tar archive format \n\n", progname);
}
else {	/* Further determination of tar header type.
	 * There is something in the former  rdev[6] field.
	 */
	hdrtype = OUA;	/* Set type to older Ultrix format  -
			 * ie. dblock.dbuf.magic MAY contain
			 * major/minor device numbers.
			 */ 
	if (!strcmp(dblock.dbuf.magic,TMAGIC)) {

		hdrtype = UGS;	/* Set type to plain User Group
				 * standard format.
				 * Do final check of format.
				 */
		if (dblock.dbuf.carch[0]) {

			/* Is User Group standard, PLUS -> Ultrix
			 * multi-archive extensions.
			 */
			hdrtype = UMA;
			/*
			 * Extract mod time & archive #'s.
			 */
CHKVOL:
/**/
			sscanf(dblock.dbuf.mtime, "%lo", &sp->st_mtime);
			sscanf(dblock.dbuf.carch, "%d", &tcarch);
			sscanf(dblock.dbuf.oarch, "%d", &toarch);

			if (tcarch >= CARCH) {
				CARCH = tcarch;
				OARCH = toarch;
			}

			if (EODFLAG || VOLCHK) {
				reload = 0;


				if (CARCH != nextvol) {
					reload++;
					fprintf(stderr,"%s: Incorrect archive (%d) loaded on %s\n",
						progname,CARCH,usefile);

					fprintf(stderr,"%s: Archive  %d  expected\n",
						progname, nextvol);
				}

			    if (blocks >= 0) {
				/*
				 * If blocks is less than 0, an EOT
				 * and end of file occured at the
				 * same time. In this case, the file
				 * is not continued to the next archive
				 * and therefore the following checks
				 * do not apply. A new and different
				 * file will have been started.
				 */
				if (strcmp(file_name,dblock.dbuf.name)){
					if (!reload) {
						fprintf(stderr,"%s: Archive  %d  does not begin with correct file.\n",
							progname,CARCH);

					fprintf(stderr,"%s: Continued file name:  %s\n",
						progname, file_name);

					fprintf(stderr,"%s: File name on current archive  %s\n",
					     progname,dblock.dbuf.name);

					reload++;
					}
				}
				if (!reload && (sp->st_mtime != modify_time)) {

					fprintf(stderr,"\n%s: Continued file:  %s\n%s: modification time does not match previous archive.\n",
					   progname,file_name,progname);

					reload++;
				}
			    }/*E if blocks > 0 */

				if (reload) {
					retrying = reload;
					reload = VOLCHK = 0;
					goto RELOAD;
				}
			}
			sscanf(dblock.dbuf.size, "%lo", &sp->st_size);

			/* dblock.dbuf.size  contains the size of the
			 * file, or a file chunk.
			 * For "real" tapes, the portion of a file 
			 * written when we encounter an  EOT  has the
			 * true file size as it is not possible to
			 * know in advance when we'll run out of tape.
			 * For disks,  dblock.dbuf.size  contains the
			 * the amount of data we could put on the
			 * current archive. This would be either at the
			 * end of an archive, or at the beginning of
			 * a continuation archive. This is possible
			 * because we know the media size in advance
			 * and can calculate the amount of data we
			 * can place on it without running out of
			 * available space.
			 */

			chctrs_in_this_chunk = sp->st_size;

			/* The next test is seeking to determine if
			 * this is a "continuation" header. Either
			 * continued to next archive (disks only) or
			 * continued from a previous archive.
			 * Real tapes have only continued "from" headers
			 * and the  EOT  detection is used in lieu of
			 * a "continued to next archive" header.
			 */
			if (OARCH) {
				sscanf(dblock.dbuf.org_size, "%lo", &original_size);

				/* Don't add up the size when switching
				 * real tape archives. The initial
				 * archive contained the size info
				 * required to extract the file.
				 * For non-tapes, the various portion
				 * sizes of the file are contained in
				 * each header. For tapes, you cannot
				 * know when you are going to expire
				 * the media - ie. run out of tape.
				 */
				if (!EOTFLAG)
					extracted_size += sp->st_size;
			}
			else  {
				/* When processing a "real" tape, the
				 * header of a continued file on the
				 * ORIGINAL archive will contain the
				 * true size of the file. It is this
				 * amount in blocks that the logic
				 * will attempt to read.
				 */
				extracted_size = original_size = sp->st_size;
			}
			if (VOLCHK && EOTFLAG) {
				/*
				 * Return to  readtape()  indicating
				 * that the correct continuation
				 * archive was verified as having
				 * been mounted on the input device.
				 */
				return;
			}
			if (VFLAG && !volann) {
				printf("%s: Ultrix multi-archive tar format\n", progname);
				printf("%s: Current archive  %d\n", progname, CARCH);
				if (OARCH && (OARCH != CARCH))
					printf("%s: Original archive  %d\n\n", progname, OARCH);
				else
					printf("\n");
			}
		}/*T if (dblock.dbuf.carch[0]) */
		else {
			/* Archive is User Group Standard 
			 * format.
			 */
			if (VFLAG && !volann)
				printf("%s: User Group Standard tar archive format\n\n", progname);

		}/*F if dblock.dbuf.carch[0] */
	}/*T if (strcmp(dblock.dbuf.magic,TMAGIC)==0) */

	else {
		/* Archive is older Ultrix version ..
		 * ie. May have a device major/minor number
		 * the  dblock.dbuf.magic  (formally rdev[6]).
		 */
		if (VFLAG && !volann)
			printf("%s: Older Ultrix tar archive format\n\n", progname);

	}/*F if strcmp(dblock.dbuf.magic, TMAGIC)==0 */
}

/* If this file is in User Group format, with or without Ultrix
 * extensions, convert fields to the internal format used by the
 * original  tar  code for ease of implementation.
 */
if (hdrtype >= UGS) {

	/* Put possible major/minor device numbers where tar
	 * usually expects to find them.
	 */
	sscanf(dblock.dbuf.devmajor, "%o", &majordev);
	sscanf(dblock.dbuf.devminor, "%o", &minordev);
	sp->st_rdev = ((majordev << 8) | minordev);

	/* Scan /etc/passwd and /etc/group to get user id
	 * and group id numbers.
	 */
	if (!NFLAG) {
		if (pwp=getpwnam(dblock.dbuf.uname))
			sp->st_uid = pwp->pw_uid;

		if (gp=getgrnam(dblock.dbuf.gname))
			sp->st_gid = gp->gr_gid;
	}
	/*
	 * Take further special action regarding "file types"
	 * as defined by the User Group standard.
	 */
	switch(dblock.dbuf.typeflag) {

		/* A regular file ?
		 */
		case AREGTYPE:
		case REGTYPE:
			sp->st_mode |= S_IFREG;
			break;

		/* Symbolic link ?
		 *
		case SYMTYPE:
			sp->st_mode |= S_IFLNK;
			break;

		/* Character special ?
		 */
		case CHRTYPE:
			sp->st_mode |= S_IFCHR;
			break;

		/* Block special ?
		 */
		case BLKTYPE:	
			sp->st_mode |= S_IFBLK;		
			break;

		/* Directory ?
		 */
		case DIRTYPE:
			sp->st_mode |= S_IFDIR;
			break;

		/* FIFO special
		 *  and/or  Contiguous ?
		 *
 		 * (for want of anything better at the moment,
		 *  classify these as -regular- files.)
		 */
		case FIFOTYPE:
		case CONTTYPE:
		default:
			sp->st_mode |= S_IFREG;
			break;

	}/*E switch(typeflag) */

}/* if hdrtype >= UGS */


EODFLAG = 0;

#ifdef TAR40
if (chksum != (i = CHECKSUM())) {
#else
if (chksum != (i = checksum())) {
#endif

	fprintf(stderr, "\n\n\007%s: Directory checksum error, possible file name:\n", progname);

	for (i=0; i < NAMSIZ; i++) 
		fprintf(stderr,"%c",dblock.dbuf.name[i]);

	fprintf(stderr,"\n");

	if (iflag)
		goto top;

	done(FAIL);
}
if (tfile)
	fprintf(tfile, "%-s %-12.12s\n", dblock.dbuf.name, dblock.dbuf.mtime);

}/*E getdir() */

/*.sbttl longt() */

/* Function:
 *
 *	longt
 *
 * Function Description:
 *
 *	Prints out the long statistics about the current file.
 *
 * Arguments:
 *
 *	struct stat	*st	Pointer to filestat data
 *
 * Return values:
 *
 *
 * Side Effects:
 *
 *	
 */
longt(st)
	struct stat	*st;
{
/*------*\
  Locals
\*------*/

	STRING_POINTER	cp;
	STRING_POINTER	ctime();
	SIZE_I	tblocks;
	int	majordev, minordev;

/*------*\
   Code
\*------*/

pmode(st);

if (((st->st_mode & S_IFMT) == S_IFCHR) ||
      ((st->st_mode & S_IFMT) == S_IFBLK)) {

	majordev = st->st_rdev >> 8;
	minordev = st->st_rdev & 0377;
	if (OFLAG && vflag) {
		if (hdrtype >= UGS) 
			printf(" %s/%s\t",dblock.dbuf.uname,dblock.dbuf.gname);
	}

	printf("%3d/%d    %3d,%3d ", st->st_uid, st->st_gid, majordev,minordev);
}
else {
	if (OFLAG && vflag) {
		if (hdrtype >= UGS) 
			printf(" %s/%s  ",dblock.dbuf.uname,dblock.dbuf.gname);
	}
	printf("%3d/%d ", st->st_uid, st->st_gid);

	if (OARCH && (original_size > chctrs_in_this_chunk)) {
		printf(" (%ld bytes of %ld)", chctrs_in_this_chunk, original_size);
	}
	else
		printf("%7D", st->st_size);

	if (!OARCH)
		tblocks = (st->st_size + (SIZE_L)(TBLOCK-1L)) / (SIZE_L)TBLOCK;
	else
		tblocks = (chctrs_in_this_chunk + (SIZE_L)(TBLOCK-1L)) / (SIZE_L)TBLOCK;

	if (VFLAG)
		printf("/%03d ", tblocks);
}
cp = ctime(&st->st_mtime);
printf(" %-12.12s %-4.4s ", cp+4, cp+20);

}/*E longt() */

/*.sbttl lookup() */

/* Function:
 *
 *	lookup
 *
 * Function Description:
 *
 *	Used during an archive "update" function.
 *
 * Arguments:
 *
 *	char	*s
 *
 * Return values:
 *
 *
 * Side Effects:
 *
 *	
 */
daddr_t lookup(s)
	STRING_POINTER	s;
{
/*------*\
  Locals
\*------*/

	daddr_t	a;
	INDEX	i;

/*------*\
   Code
\*------*/

for (i=0; s[i]; i++)
	if (s[i] == ' ')
		break;

a = bsrch(s, i, low, high);
	return (a);

}/*E lookup() */

/*.sbttl passtape() */

/* Function:
 *
 *	passtape
 *
 * Function Description:
 *
 *	Skips over input file content when desired.
 *
 * Arguments:
 *
 *	none
 *
 * Return values:
 *
 *	none
 *
 * Side Effects:
 *
 */
passtape()
{
/*------*\
  Locals
\*------*/

	SIZE_L	pblocks;
	char	buf[TBLOCK];

/*------*\
   Code
\*------*/

if (dblock.dbuf.typeflag) {
	if ((dblock.dbuf.typeflag != REGTYPE) &&
    	    (dblock.dbuf.typeflag != AREGTYPE))

		return;
}
pblocks = stbuf.st_size;
pblocks += (SIZE_L)TBLOCK-1L;
pblocks /= (SIZE_L)TBLOCK;

/* Number of blocks in a file better not ever exceed an "int"
 * size, or this will fail. 
 */
blocks = (SIZE_I)pblocks;

/* 		*-*  WARNING  *-*
 *
 * readtape() will make appropriate adjustments to the global "blocks"
 * variable if a real "tape" archive switch is performed because
 * of an encountered  EOT. This must be done in order to prevent
 * this logic from reading an incorrect number of blocks when
 * switching tape archives.
 */
while (blocks-- > 0)
	readtape(buf);

}/*E passtape() */

/*.sbttl pmode() */

/* Function:
 *
 *	pmode
 *
 * Function Description:
 *
 *	Prints out the Unix file protection modes in
 *	symbolic (rwx) format. Also outputs the special
 *	file indication - for directorys (d), character
 *	special (c), and block special (b). If none of these,
 *	first character on line is given as "-".
 *
 * Arguments:
 *
 *	struct stat	*st	Pointer to filestat buffer.
 *
 * Return values:
 *
 *
 * Side Effects:
 *
 *	
 */

static int	m1[]	= { 1, ROWN, 'r', '-' };
static int	m2[]	= { 1, WOWN, 'w', '-' };
static int	m3[]	= { 2, SUID, 's', XOWN, 'x', '-' };
static int	m4[]	= { 1, RGRP, 'r', '-' };
static int	m5[]	= { 1, WGRP, 'w', '-' };
static int	m6[]	= { 2, SGID, 's', XGRP, 'x', '-' };
static int	m7[]	= { 1, ROTH, 'r', '-' };
static int	m8[]	= { 1, WOTH, 'w', '-' };
static int	m9[]	= { 2, STXT, 't', XOTH, 'x', '-' };

/* Next variable must come after the above to supply addresses.
 */
static int	*m[]	= { m1, m2, m3, m4, m5, m6, m7, m8, m9};

pmode(st)
	struct stat *st;
{

	char *cp;
	register int **mp;

#ifndef U11
if (VFLAG) {
#endif
	switch(st->st_mode & S_IFMT) {

		case S_IFCHR:
			printf("c");
			break;

		case S_IFBLK:
			printf("b");
			break;

		case S_IFDIR:
			printf("d");
			break;

		default: {
			if ((cp = rindex(dblock.dbuf.name,'/'))==0)
				printf("-");
			else {
				cp++;
				if (!*cp)
					printf("d");
				else
					printf("-");
			}
		}
	}
#ifndef U11
}/*E if VFLAG */
#endif
	for (mp = &m[0]; mp < &m[9];)
		select(*mp++, st);

}/*E pmode() */

/*.sbttl prefix() */

/* Function:
 *
 *	prefix
 *
 * Function Description:
 *
 *	Part of the extract logic.
 *
 * Arguments:
 *
 *	char	*s1
 *	char	*s2
 *
 * Return values:
 *
 *
 * Side Effects:
 *
 *	
 */
prefix(s1, s2)
	register char *s1, *s2;
{
/*------*\
   Code
\*------*/

while (*s1)
	if (*s1++ != *s2++)
		return (0);
if (*s2)
	return (*s2 == '/');

return (1);

}/*E prefix() */

/*.sbttl readtape()  Read data from input archive */

/* Function:
 *
 *	readtape
 *
 * Function Description:
 *
 *	Logic to read from an input archive.
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
readtape(buffer)
	STRING_POINTER	buffer;
{
/*------*\
  Locals
\*------*/

	SIZE_I	i;
	STRING_POINTER	from, to;

/*------*\
   Code
\*------*/

if (recno >= nblock || first == 0) {

REREAD:
/**/
	if ((i = bread(mt, tbuf, TBLOCK*nblock)) < 0) {
		fprintf(stderr, "%s: Archive read error at block %ld\n",
			progname, blocks_used);
		perror(usefile);
		done(FAIL);
	}
	if (!first) {

		if ((i % TBLOCK) != 0) {
			fprintf(stderr, "%s: Archive blocksize error\n",
				progname);
			done(FAIL);
		}
		i /= TBLOCK;
		if (i != nblock) {
			fprintf(stderr,"%s: blocksize = %d\n", progname, i);
			nblock = i;
		}
	}/*E if !first */

	recno = 0;

	if (EOTFLAG) {
	/*
	 * bread()  has detected an  EOT  on a "real" tape.
	 * It has called  getdir()  to announce the end of media
	 * & has asked the user to mount the next archive. This
	 * has been done & bread() has returned to us some amount of
	 * data it has read from the new archive.
	 */
#ifdef PRO
		for (from = (char *)&tbuf[recno], to = (char *)&dblock, i=TBLOCK; i; i--)
			*to++ = *from++;
#else
		bcopy((char *)&tbuf[recno], (char *)&dblock, TBLOCK);
#endif
	    if (blocks >= 0) {
		/*
		 * If blocks is < 0 at this point, an EOT and the
		 * end of file have occured at the same time.
		 * The logic will actually be in getdir() when the
		 * EOT is detected. When that is the case, then a
		 * file is not actually split across a volume.
		 * A new and different file will be started on the
		 * next archive in the set.
		 *
		 * Account for the continuation header block.
		 */
		blocks_used++;
		recno++;

	    }/*E if blocks >= 0 */

		/* Go ask  getdir()  to verify that the correct
		 * continuation archive has been mounted by the user.
		 */
		VOLCHK++;

		getdir();

		if (!VOLCHK) 
			/* 
			 * getdir()  will negate the  VOLCHK  flag if
			 * it feels that the mounted archive was not
			 * the correct archive. We'll re-cycle thru
			 * the logic & try to goad the user into
			 * putting up the correct archive.
			 */
			goto REREAD;
		
		if (blocks >= 0) {

			/* Get the number of blocks in this continued
			 * chunk of a file.
			 */
			blocks =
			    (chctrs_in_this_chunk + (SIZE_L)TBLOCK - 1L) / (SIZE_L)TBLOCK;


			if (vflag && xflag) {

				if (OARCH && (original_size > chctrs_in_this_chunk)) 
#ifndef U11
					fprintf(stderr,"x %s, %ld bytes of %ld, %d blocks\n",
					  dblock.dbuf.name, chctrs_in_this_chunk,
					  original_size, blocks);
				else
					fprintf(stderr,"x %s, %ld bytes, %d blocks\n",
					  dblock.dbuf.name, bytes, blocks);
#else
					fprintf(stderr,"x %s, %ld bytes of %ld, %d tape blocks\n",
					  dblock.dbuf.name, chctrs_in_this_chunk,
					  original_size, blocks);
				else
					fprintf(stderr,"x %s, %ld bytes, %l tape blocks\n",
					  dblock.dbuf.name, bytes, blocks);
#endif
			}/*E if vflag && xflag */

			if (tflag)
				dostats();	

			/*
			 * Because of the loop counters used by
			 * passtape() &  doxtract(), one is needed
			 * to be subtracted from the count of blocks
			 * left to read. Their loops have already
			 * counted the block we are going to return
			 * to them from the new archive. If we don't
			 * dec the count by one, they'll end up reading
			 * a block too many.
			 */
		 	blocks--;

		}/*E if blocks >= 0 */
			
		EOTFLAG = VOLCHK = MULTI = FILE_CONTINUES = 0;

	}/*E if EOTFLAG */
}/*E if recno >= nblock ..*/

first = 1;

#ifdef PRO
for (from = (char *)&tbuf[recno++], to = buffer, i=TBLOCK; i; i--)
	*to++ = *from++;
#else
bcopy((char *)&tbuf[recno++],buffer,TBLOCK);
#endif

blocks_used++;
return (TBLOCK);

}/*E readtape() */

/*.sbttl select() */

/* Function:
 *
 *	select
 *
 * Function Description:
 *
 *	Part of the "table" function.
 *
 * Arguments:
 *
 *	int		*pairp	?
 *	struct stat	*st	Pointer to filestat structure
 *
 * Return values:
 *
 *
 * Side Effects:
 *
 *	
 */
select(pairp, st)
	int	*pairp;
	struct stat	*st;
{
/*------*\
  Locals
\*------*/

	int	*ap;
	int	n;

/*------*\
   Code
\*------*/

ap = pairp;
n = *ap++;

while (--n >= 0 && (st->st_mode & *ap++) == 0)
	ap++;

printf("%c", *ap);

}/*E select() */

/*.sbttl STRFIND() */

#ifdef TAR40
/* Function:
 *
 *	STRFIND
 *
 * Function Description:
 *
 *	Searches for  substr  in text.
 *	
 * Arguments:
 *
 *	char	*text		Text to search
 *	char	*substr		String to locate
 *
 * Return values:
 *
 *	Pointer to  substr  starting position in text if found.
 *	NULL if not found.
 *
 * Side Effects:
 *
 *	
 */
STRING_POINTER	STRFIND(text,substr)
		STRING_POINTER text, substr;
{
/*------*\
  Locals
\*------*/

	COUNTER	i;	/* counter for possible fits */
	SIZE_I	substrlen;/* len of substr--to avoid recalculating */

/*------*\
   Code
\*------*/

substrlen = strlen(substr);

/* Loop through text until not found or match.
 */
for (i = strlen(text) - substrlen; i >= 0 &&
     strncmp(text, substr, substrlen); i--)

	text++;

/* Return NULL if not found, ptr else NULL.
 */ 
return ((i < 0 ? NULL : text));

}/*E STRFIND() */
#endif
/*.sbttl update()  Top level logic to UPDATE an archive */

/* Function:
 *
 *	update
 *
 * Function Description:
 *
 *
 * Arguments:
 *
 *
 *
 * Return values:
 *
 *
 * Side Effects:
 *
 *	
 */
update(arg,ftime)
	char	*arg;
	time_t	ftime;
{
/*------*\
  Locals
\*------*/

	daddr_t	lookup();
	long	mtime;
	char	name[MAXPATHLEN];
	daddr_t	seekp;

/*------*\
   Code
\*------*/

rewind(tfile);

for (;;) {
	if ((seekp = lookup(arg)) < 0)
		return (1);

	fseek(tfile, seekp, 0);
	fscanf(tfile, "%s %lo", name, &mtime);
	return (ftime > mtime);
}
}/*E update() */

