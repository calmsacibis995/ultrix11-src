
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/* SCCSID="@(#)ioprim.c	3.0	4/22/86"	*/
/* "(Berkeley 2.9)  ioprim.c  1.1"
 *
 * FORTRAN i/o interface.
 * For use with UNIX version 7 f77 compiler.
 */
#include <stdio.h>
#include <ctype.h>
#include "ioprim.h"

static FILE *fd[MAXUNIT] = {
		stdin, stdout, stderr,
		NULL,  NULL,   NULL,
		NULL,  NULL,   NULL,
		NULL,  NULL,   NULL,
		NULL,  NULL,   NULL,
		NULL,  NULL,   NULL,
		NULL,  NULL,   NULL,
		NULL,  NULL};

/* Read a character from the standard input */
getc_(result, lr, c, lc)
FORTCHAR *result;
STRLARG lr;
register FORTCHAR *c;
STRLARG lc;
{
	*c = (FORTCHAR)getchar();
	*result = *c;
}

/* Read a character from a file */
getch_(result, lr, c, f, lc)
FORTCHAR *result;
STRLARG lr;
register FORTCHAR *c;
register FILEID *f;
STRLARG lc;
{
	*c = (FORTCHAR)getc(fd[*f]);
	*result = *c;
}

/* Push a character back into input file */
putbak_(result, lr, c, f, lc)
char *result;
STRLARG lr;
char *c;
FILEID *f;
STRLARG lc;
{
	*result = ungetc(*c, fd[*f]);
	*c = *result;
}

/* Binary read from a file */
FORTINT readb_(ptr, size, nitems, f)
char *ptr;
FORTINT *size, *nitems;
FILEID *f;
{
	return((FORTINT)fread(ptr, (int)*size,
		(int)*nitems, fd[*f]));
}

/* Write a character to the standard output */
putc_(c, lc)
register FORTCHAR *c;
STRLARG lc;
{
	putchar(*c);
}

/* Write a character to a file */
putch_(c, f, lc)
register FORTCHAR *c;
register FILEID *f;
STRLARG lc;
{
	putc(*c, fd[*f]);
}

/* Binary write to a file */
FORTINT writeb_(ptr, size, nitems, f)
char *ptr;
FORTINT *size, *nitems;
FILEID *f;
{
	return((FORTINT)fwrite(ptr, (int)*size,
		(int)*nitems, fd[*f]));
}

/* Open a file, return its fileid */
FILEID open_(name, mode, ln, lm)
FORTCHAR *name;
FORTCHAR *mode;
STRLARG ln, lm;
{
	register int i;
	char c;
	int ic;
	FILE *fopen();

	for (i=ln-1; i>0; i--)
		if (!isspace(c=name[i]) && c!=NULL)
			break;
	c = name[ic=i+1];
	name[ic] = NULL;
	for (i=0; i<MAXUNIT; i++)	/* Find a free fileid */
		if (fd[i] == NULL)
			break;
	if (i >= MAXUNIT)		/* All are in use */
		i = FORTERR;
	else if ((fd[i] = fopen(name, mode)) == NULL)
		i = FORTERR;
	name[ic] = c;
	return((FILEID)i);
}

/* Create a new file, deleting old one, if any */
FILEID create_(name, mode, ln, lm)
FORTCHAR *name;
FORTCHAR *mode;
STRLARG ln, lm;
{
	unlink(name);
	return(open_(name, mode, ln, lm));
}

/* Close an open file */
close_(f)
register FILEID *f;
{
	if (fd[*f] != NULL)
		fclose(fd[*f]);
	fd[*f] = NULL;
}

/* Flush output to a file */
flush(f)
register FILEID *f;
{
	if (fd[*f] != NULL)
		fflush(fd[*f]);
}

/* Position a file for next i/o operation */
seek_(f, offset, whence)
FILEID *f;
OFFSET *offset;
FORTINT *whence;
{
	fseek(fd[*f], (OFFSET)*offset, (int)*whence);
}

/* Return current offset on a file */
OFFSET tell_(f)
FILEID *f;
{
	return((OFFSET)ftell(fd[*f]));
}

/* Get FILE (stream) for a file */
FILE *getfile(fid)
FILEID fid;
{
	return(fd[fid]);
}
