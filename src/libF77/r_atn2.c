
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)r_atn2.c	3.0	4/22/86
 */
double r_atn2(x,y)
float *x, *y;
{
double atan2();
return( atan2(*x,*y) );
}
