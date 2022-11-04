
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/* SCCSID: @(#)orgin.c	3.0	4/22/86 */
main()
{
openpl();
erase();
linemod("dotted");
line(0,0,400,400);
closepl();
}
