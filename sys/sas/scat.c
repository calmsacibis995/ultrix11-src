/*
 * SCCSID: @(#)scat.c	3.0	4/21/86
 */
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

main()
{
	int c, i;
	char buf[50];

	do {
		printf("File: ");
		gets(buf);
		i = open(buf, 0);
	} while (i <= 0);

	while (((c = getc(i)) > 0) && (c != '$'))
		putchar(c);
	exit(0);
}
