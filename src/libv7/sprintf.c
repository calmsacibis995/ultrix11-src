
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)sprintf.c	3.0	4/22/86
 */
#include	<stdio.h>

char *sprintf(str, fmt, args)
char *str, *fmt;
{
	/* struct _iobuf _strbuf; */
	FILE _strbuf;

	_strbuf._flag = _IOWRT;	/* | _IOSTRG; */
	_strbuf._ptr = str;
	_strbuf._cnt = 32767;
	_doprnt(fmt, &args, &_strbuf);
	putc('\0', &_strbuf);
	return(str);
}
