
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)r_tanh.c	3.0	4/22/86
 */
double r_tanh(x)
float *x;
{
double tanh();
return( tanh(*x) );
}
