
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/* SCCSID: @(#)target.c	3.0	4/22/86 */
/* Plot test program */
/* BULL's Eye */
main()
{
int i;
openpl();
erase();
for (i=10;i <256; i=i+5) {
circle(256,256,i);
}
closepl();
}
