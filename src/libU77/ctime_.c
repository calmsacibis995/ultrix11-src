
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)ctime_.c	3.0	4/22/86
char id_ctime[] = "(2.9BSD)  ctime_.c  1.1";
 *
 * convert system time to ascii string
 *
 * calling sequence:
 *	character*24 string, ctime
 *	integer clock
 *	string = ctime (clock)
 * where:
 *	string will receive the ascii equivalent of the integer clock time.
 */

#include	"../libI77/fiodefs.h"
#include	<sys/types.h>

char *ctime();

ctime_(str, len, clock)
char *str; ftnlen len; time_t *clock;
{
	char *s = ctime(clock);
	s[24] = '\0';
	b_char(s, str, len);
}
