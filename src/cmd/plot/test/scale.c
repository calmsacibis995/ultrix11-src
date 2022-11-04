
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/* SCCSID: @(#)scale.c	3.0	4/22/86 */

main()
{
openpl();
erase();
space(0,0, 479, 479);
line(0,0,0,479);
line(0,479,479,479);
line(479,0,479,479);
line(479,0,0,0);
closepl();
}
