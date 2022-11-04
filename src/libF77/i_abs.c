
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)i_abs.c	3.0	4/22/86
 */
long int i_abs(x)
long int *x;
{
if(*x >= 0)
	return(*x);
return(- *x);
}
