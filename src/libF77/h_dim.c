
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)h_dim.c	3.0	4/22/86
 */
short h_dim(a,b)
short *a, *b;
{
return( *a > *b ? *a - *b : 0);
}
