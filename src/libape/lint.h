
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/


/*	SCCSID: @(#)lint.h	3.0	4/22/86	*/
/* lint definitions */

#ifdef lint		/* Here a is an integer, s a char pointer */
#define ignore(a)	Ignore(a)
#define forget(s)	Forget(s)
#define ignors(s)	Ignors(s)
#else
#define ignore(a)	a
#define forget(s)
#define ignors(s)	s
#endif lint
