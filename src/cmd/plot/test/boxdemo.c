
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/* SCCSID: @(#)boxdemo.c	3.0	4/22/86 */
main()
{
openpl();
line(0,0,239,0);
line(239,0,239,239);
line(239,239,0,239);
line(0,239,0,0);
closepl();
}
