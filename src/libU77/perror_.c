
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)perror_.c	3.0	4/22/86
char id_perror[] = "(2.9BSD)  perror_.c  1.2";
 *
 * write a standard error message to the standard error output
 *
 * calling sequence:
 *	call perror(string)
 * where:
 *	string will be written preceeding the standard error message
 */

#include	"../libI77/fiodefs.h"

extern char *sys_errlist[];
extern int sys_nerr;
extern char *f_errlist[];
extern int f_nerr;
extern unit units[];

perror_(s, len)
char *s; ftnlen len;
{
	unit	*lu;
	char	buf[40];
	char	*mesg = s + len;

	while (len > 0 && *--mesg == ' ')
		len--;
	if (errno >=0 && errno < sys_nerr)
		mesg = sys_errlist[errno];
	else if (errno >= F_ER && errno < (F_ER + f_nerr))
		mesg = f_errlist[errno - F_ER];
	else
	{
		sprintf(buf, "%d: unknown error number", errno);
		mesg = buf;
	}
	lu = &units[STDERR];
	if (!lu->uwrt)
		nowwriting(lu);
	while (len-- > 0)
		putc(*s++, lu->ufd);
	fprintf(lu->ufd, ": %s\n", mesg);
}
