
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)erfc_.c	3.0	4/22/86
 */
float erfc_(x)
float *x;
{
double erfc();

return( erfc(*x) );
}
