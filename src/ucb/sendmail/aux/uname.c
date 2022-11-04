
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

# include <stdio.h>
# include <sysexits.h>

/*
**  UNAME -- print UNIX system name (fake version)
**
**	For UNIX 3.0 compatiblity.
*/

main(argc, argv)
	int argc;
	char **argv;
{
	char buf[40];

	gethostname(buf, sizeof buf);
	printf("%s\n", buf);
	exit(EX_OK);
}
