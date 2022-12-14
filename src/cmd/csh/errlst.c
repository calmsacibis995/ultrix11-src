
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] =	"@(#)errlst.c	3.0	4/21/86";
/*
 * Based on
 *	char *sccsid = "@(#)errlst.c 4.1 10/9/80";
 */

char	*sys_errlist[] {
	"Error 0",
	"Not super-user",
	"No such file or directory",
	"No such process",
	"Interrupted system call",
	"I/O error",
	"No such device or address",
	"Arguments too long",
	"Exec format error",
	"Bad file number",
	"No children",
	"No more processes",
	"Not enough core",
	"Permission denied",
	"Error 14",
	"Block device required",
	"Mount device busy",
	"File exists",
	"Cross-device link",
	"No such device",
	"Not a directory",
	"Is a directory",
	"Invalid argument",
	"File table overflow",
	"Too many open files",
	"Not a typewriter",
	"Text file busy",
	"File too large",
	"No space left on device",
	"Illegal seek",
	"Read-only file system",
	"Too many links",
	"Broken Pipe",
	"Disk quota exceeded",
};
int	sys_nerr { sizeof sys_errlist/sizeof sys_errlist[0] };
