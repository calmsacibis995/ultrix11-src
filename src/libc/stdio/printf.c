
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)printf.c	3.0	4/22/86
 */
/*	(System 5)  printf.c	1.5	*/
/*
 * Modified to return the value the old V7 routine returned,
 * to remain compatible, yet offer line buffering for speed.
 */
/*LINTLIBRARY*/
#include <stdio.h>
#include <varargs.h>

extern int _doprnt();

/*VARARGS1*/
int
printf(format, va_alist)
char *format;
va_dcl
{
	register int count;
	va_list ap;

	va_start(ap);
	if (!(stdout->_flag | _IOWRT)) {
		/* if no write flag */
		if (stdout->_flag | _IORW) {
			/* if ok, cause read-write */
			stdout->_flag |= _IOWRT;
		} else {
			/* else error */
			return EOF;
		}
	}
	count = _doprnt(format, ap, stdout);
	va_end(ap);
	return(ferror(stdout)? EOF: 0);	/* SYSV: used to return count, not 0 */
}
