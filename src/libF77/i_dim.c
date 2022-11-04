
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)i_dim.c	3.0	4/22/86
 */
long int i_dim(a,b)
long int *a, *b;
{
return( *a > *b ? *a - *b : 0);
}
