
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)frame.c	3.0	4/22/86
 */
frame(n)
{
	extern vti;
	n=n&0377 | 02000;
	write(vti,&n,2);
}
