
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/* SCCSID: @(#)plotd.c	3.0	4/22/86 */
/* Plot test program */
/* openpl, erase, line, move, label, cont, closepl */
/* this program test the following plot library routines:  */
main()
{
char *message;
openpl();
erase();
line(0,0,0,239);
line(0,239,239,239);
line(239,0,239,239);
line(239,0,0,0);
move(90,118);
message = "Davids Box!";
label(message);
closepl();
}
