
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)derfc_.c	3.0	4/22/86
 */
double derfc_(x)
double *x;
{
double erfc();

return( erfc(*x) );
}
