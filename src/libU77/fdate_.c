
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)fdate_.c	3.0	4/22/86
char id_fdate[] = "(2.9BSD)  fdate_.c  1.1";
 *
 * Return date and time in an ASCII string.
 *
 * calling sequence:
 *	character*24 string
 * 	call fdate(string)
 * where:
 *	the 24 character string will be filled with the date & time in
 *	ascii form as described under ctime(3).
 *	No 'newline' or NULL will be included.
 */

#include	"../libI77/fiodefs.h"
#include	<sys/types.h>

fdate_(s, strlen)
char *s; ftnlen strlen;
{
	char *ctime(), *c;
	time_t time(), t;

	t = time((time_t) 0);
	c = ctime(&t);
	c[24] = '\0';
	b_char(c, s, strlen);
}
