
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)swab.c	3.0	4/22/86
 */
/*
 * Swap bytes in 16-bit [half-]words
 * for going between the 11 and the interdata
 */

swab(pf, pt, n)
register short *pf, *pt;
register n;
{

	n /= 2;
	while (--n >= 0) {
		*pt++ = (*pf << 8) + ((*pf >> 8) & 0377);
		pf++;
	}
}
