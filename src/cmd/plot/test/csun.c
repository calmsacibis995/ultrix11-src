
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/* SCCSID: @(#)csun.c	3.0	4/22/86 */
main()
{
openpl();
erase();
box(60,60,140,140);
box(20,20,180,180);
line(100,60,100,140);
line(60,100,140,100);
circle(100,100,40);
circle(60,60,40);
circle(60,140,40);
circle(140,140,40);
circle(140,60,40);
circle(60,100,40);
circle(100,140,40);
circle(140,100,40);
circle(100,60,40);
closepl();
}
