
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)ef1cmc_.c	3.0	4/22/86
 */
/* EFL support routine to compare two character strings */

long int ef1cmc_(a, la, b, lb)
int *a, *b;
long int *la, *lb;
{
return( s_cmp( (char *)a, (char *)b, *la, *lb) );
}
