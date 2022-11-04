
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/* SCCSID: @(#)arc.c	3.0	4/22/86 */
/* Plot package test program */
/* this program tasts the following plot routines: */
/* openpl, move, cont, line, arc, closepl */
box(x0,y0,x1,y1)
{
	move(x0,y0);
	cont(x0,y1);
	cont(x1,y1);
	cont(x1,y0);
	cont(x0,y0);
	move(x1,y1);
}
main()
{
openpl();
erase();
box(20,20,180,180);
line(100,20,100,180);
line(20,100,180,100);
arc(100,100,180,100,100,180);
closepl();
}
