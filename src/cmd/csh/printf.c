
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char *Sccsid =	"@(#)printf.c	3.0	4/21/86";
/* Copyright (c) 1979 Regents of the University of California */
/*
 * Hacked "printf" which prints through putchar.
 * DONT USE WITH STDIO!
 */
printf(fmt, args)
char *fmt;
{
	_doprnt(fmt, &args, 0);
}

_strout(string, count, adjust, foo, fillch)
register char *string;
register int count;
int adjust;
register struct { int a[6]; } *foo;
{

	if (foo != 0)
		abort();
	while (adjust < 0) {
		if (*string=='-' && fillch=='0') {
			putchar(*string++);
			count--;
		}
		putchar(fillch);
		adjust++;
	}
	while (--count>=0)
		putchar(*string++);
	while (adjust) {
		putchar(fillch);
		adjust--;
	}
}
