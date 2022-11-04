
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)findiop.c	3.0	4/22/86
 */
#include <stdio.h>

FILE *
_findiop()
{
	extern FILE *_lastbuf;
	register FILE *iop;

	for(iop = _iob; iop->_flag & (_IOREAD|_IOWRT|_IORW); iop++)
		if (iop >= _lastbuf)
			return(NULL);

	return(iop);
}
