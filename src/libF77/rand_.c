
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/* SCCSID="@(#)rand_.c	3.0	4/22/86"	*/
/* "(Berkeley 2.9)  rand_.c  1.1"
 *
 * Routines to return random values
 *
 * calling sequence:
 *	double precision d, drand
 *	i = irand(iflag)
 *	x = rand(iflag)
 *	d = drand(iflag)
 * where:
 *	If arg is 1, generator is restarted. If arg is 0, next value
 *	is returned. Any other arg is a new seed for the generator.
 *	Integer values will range from 0 thru 2147483647.
 *	Real values will range from 0.0 thru 1.0
 *	(see rand(3))
 */

#define	RANDMAX		32767		/* pdp11 */

long irand_(iarg)
long *iarg;
{
	if (*iarg) srand((int)*iarg);
	return(( ((long)rand()) << 16) | rand());
}

float rand_(iarg)
long *iarg;
{
	if (*iarg) srand((int)*iarg);
	return( (float)(rand())/(float)RANDMAX );
}

double drand_(iarg)
long *iarg;
{
	if (*iarg) srand((int)*iarg);
	return( (double)(rand())/(double)RANDMAX );
}
