
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)setline.c	3.0	4/22/86";

/***
 *	setline - optimize line setting for sending or receiving files
 *
 *	return code - none
 */

#include "uucp.h"
#ifdef	SYSIII
#include <termio.h>
#endif

#define PACKSIZE	64
#define SNDFILE	'S'
#define RCVFILE 'R'
#define RESET	'X'

setline(type)
char type;
{
#ifdef	SYSIII
	static struct termio tbuf, sbuf;
	static int set = 0;

	DEBUG(2, "setline - %c\n", type);
	switch(type) {
	case SNDFILE:
		break;
	case RCVFILE:
		ioctl(Ifn, TCGETA, &tbuf);
		sbuf = tbuf;
		tbuf.c_cc[VMIN] = PACKSIZE;
		ioctl(Ifn, TCSETAW, &tbuf);
		set++;
		break;
	case RESET:
		if (set == 0) break;
		set=0;
		ioctl(Ifn, TCSETAW, &sbuf);
/* Probable bugs: sbuf should be static, set should be reset to 0. rti!trt */
/* fixed 10/3/83 decvax!larry */
		break;
	}
#endif
}
