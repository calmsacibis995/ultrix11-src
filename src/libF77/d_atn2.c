
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)d_atn2.c	3.0	4/22/86
 */
double d_atn2(x,y)
double *x, *y;
{
double atan2();
return( atan2(*x,*y) );
}
