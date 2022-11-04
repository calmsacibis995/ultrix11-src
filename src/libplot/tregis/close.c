/*
 * SCCSID: @(#)close.c	3.0	4/22/86 
 */

/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*

 Facility:	Part of the plot package for the REGIS terminal

 Called by:	driver()

 Calls: 	Nothing	

 Abstract:	Turn off REGIS mode on terminal	

 Author:	Kevin J. Dunlap

 Creation:	March 1985

*/

#include <stdio.h>
extern ggi;
closepl(){
	fprintf(ggi, "\033\\\n"); /* Turn off REGIS */
	fflush(ggi);
}
