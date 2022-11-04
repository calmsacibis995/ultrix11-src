
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)signal_.c	3.0	4/22/86
 */
signal_(sigp, procp)
int *sigp, (**procp)();
{
int sig, proc;
sig = *sigp;
proc = *procp;

return( signal(sig, proc) );
}
